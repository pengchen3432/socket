/****************************************************************************
* *
* * FILENAME:        $RCSfile: main.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2020/12/22
* *
* * DESCRIPTION:     xxxx feature:
* *
* *
* * Copyright (c) 2020 by Grandstream Networks, Inc.
* * All rights reserved.
* *
* * This material is proprietary to Grandstream Networks, Inc. and,
* * in addition to the above mentioned Copyright, may be
* * subject to protection under other intellectual property
* * regimes, including patents, trade secrets, designs and/or
* *
* * Any use of this material for any purpose, except with an
* * express license from Grandstream Networks, Inc. is strictly
* * prohibited.
* *
* ***************************************************************************/
//=================
//  Includes
//=================
#include "global.h"
#include "cfparse.h"
#include "ubus.h"
#include "utils.h"
#include "ipc.h"
#include "config.h"
#include "time.h"
#include "track.h"
#include "apply.h"
#include "sgrequest.h"

//=================
//  Defines
//=================
/* firmware header mask bits that allow downgrade and factory reset */
#define FACTORY_RESET_MASK  ( 0xF << 0 )
//=================
//  Typedefs
//=================

//=================
//  Globals
//=================
const char *app_name = "cfmanager";
struct uci_context *uci_ctx;
struct cm_service_init cm_sve_init = {
    .first_load_cfg = false,
    .boot_start = false
};

extern bool use_syslog;
struct device_info device_info;
//=================
//  Locals
//=================

/*
 * Private Functions
 */

/*
 * Private Data
 */




//=================
//  Functions
//=================


//=============================================================================
uint32_t
cfmanager_calculate_version(
    char *version
)
//=============================================================================
{
    int a = 0, b = 0, c = 0;
    uint32_t vernum = 0;

    sscanf( version, "%d.%d.%d", &a, &b, &c );
    vernum = VERSIONNUM( a, b, c );

    return vernum;
}

//=============================================================================
static int
cfmanager_usage(
    const char *progname
)
//=============================================================================
{
    fprintf(stderr, "Usage: %s(%s) [options]\n"
        "Options:\n"
        " -l <level>:       Log output level (default: %d)\n"
        " -s:               Use stderr instead of syslog for log messages\n"
        " -i <start/boot>:  The way of init\n"
        "\n", progname, VERSIONSTR, DEFAULT_LOG_LEVEL);

    return 1;
}

//=============================================================================
static void
cfmanager_handle_signal(
    int signo
)
//=============================================================================
{
    uloop_end();
}

//=============================================================================
static void
cfmanager_setup_signals(
    void
)
//=============================================================================
{
    struct sigaction s;

    memset(&s, 0, sizeof(s));
    s.sa_handler = cfmanager_handle_signal;
    s.sa_flags = 0;
    sigaction(SIGINT, &s, NULL);
    sigaction(SIGTERM, &s, NULL);
    sigaction(SIGUSR1, &s, NULL);
    sigaction(SIGUSR2, &s, NULL);

    s.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &s, NULL);
}

//=============================================================================
static int
cfmanager_device_info_init(
    void
)
//=============================================================================
{
    char value[BUF_LEN_128];
    int ret = 0;
    char path[LOOKUP_STR_SIZE];

    memset( &device_info, 0, sizeof( struct device_info ) );

    ret = util_read_file_content( "/proc/gxp/dev_info/dev_mac",
            device_info.mac,sizeof( device_info.mac ) );
    if( !ret ) {
        sscanf( device_info.mac, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
            &device_info.mac_raw[0],
            &device_info.mac_raw[1],
            &device_info.mac_raw[2],
            &device_info.mac_raw[3],
            &device_info.mac_raw[4],
            &device_info.mac_raw[5] );
    }

    util_read_file_content( "/proc/gxp/dev_info/PN",
            device_info.part_number, sizeof( device_info.part_number ) );

    util_read_file_content( "/proc/gxp/dev_info/dev_alias",
            device_info.product_model, sizeof( device_info.product_model ) );

    ret = util_read_file_content( "/tmp/gs_version",
            device_info.version_firmware,sizeof( device_info.version_firmware ) );
    if ( !ret ) {
        char *newline = strchr( device_info.version_firmware, '\n' );
        if ( newline != NULL ) {
            *newline = '\0';
        }
    }

    memset( value, 0, sizeof( value ) );
    ret = util_read_file_content( "/tmp/gs_vermask", value, sizeof( value ) );
    if ( !ret ) {
        unsigned short vermask = 0;
        sscanf( value, "0x%hx", &vermask );
        device_info.version_mask = ( vermask & FACTORY_RESET_MASK );
    }

    snprintf( path, sizeof( path ), "%s.mode.workMode", CFMANAGER_CONFIG_NAME );
    config_uci_get_option( path, value, sizeof( value ) );
    if( '1' == value[0] ) {
        device_info.work_mode = MODE_AP;
    }
    else {
        device_info.work_mode = MODE_ROUTER;
    }

    return 0;
}

//=============================================================================
int main(int argc, char **argv)
{
    int ch = 0;
    FILE *fp = NULL;
    const char *path = NULL;
    int log_level = DEFAULT_LOG_LEVEL;

    while ( ( ch = getopt(argc, argv, "l:si:" ) ) != -1 ) {
        switch(ch) {
            case 'l':
                log_level = atoi(optarg);
                cfmanager_set_log_level( log_level );
                break;
            case 's':
                use_syslog = false;
                break;
            case 'i':
                if ( 0 == strcmp( optarg, "boot" ) ) {
                    cm_sve_init.boot_start = true;
                }
                else if ( 0 == strcmp( optarg, "start" ) ) {
                    cm_sve_init.boot_start = false;
                }
                else {
                    cm_sve_init.boot_start = false;
                }
                break;
            default:
                return cfmanager_usage(argv[0]);
        }
    }

    if ( use_syslog )
        openlog( app_name, 0, LOG_DAEMON );

    fp = fopen( "/var/run/cfmanager.pid", "w" );
    fprintf( fp, "%u", (uint32_t)getpid() );
    fclose( fp );

    cfmanager_setup_signals();

    uloop_init();

    uci_ctx = uci_alloc_context();
    if ( NULL == uci_ctx ) {
        cfmanager_log_message( L_ERR, "Failed to connect to config\n" );
        if ( use_syslog ) {
            closelog();
        }
        return -1;
    }

    config_init();
    uci_ctx->flags &= ~UCI_FLAG_STRICT;
    //Set temporary save directory
    uci_set_confdir( uci_ctx, CM_CONFIG_PATH );

    if ( cfmanager_ipc_init() < 0 ) {
        cfmanager_log_message( L_ERR, "Failed to connect to ipc\n" );
        if ( use_syslog ) {
            closelog();
        }
        return -1;
    }

    if ( cfubus_init( path ) < 0 ) {
        cfmanager_log_message( L_ERR, "Failed to connect to ubus\n" );
        if ( use_syslog ) {
            closelog();
        }
        return -1;
    }

    time_init();

    cfmanager_device_info_init();
    cfparse_init();

    uloop_run();

    cfubus_done();
    cfparse_deinit();

    apply_deinit();

    uci_free_context( uci_ctx );

    uloop_done();

    cfmanager_log_message( L_WARNING, "cfmanager exiting finish\n" );
    if ( use_syslog )
        closelog();

    return 0;
}
