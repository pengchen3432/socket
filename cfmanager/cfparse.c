/****************************************************************************
* *
* * FILENAME:        $RCSfile: cfparse.c,v $
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
#include "track.h"
#include "initd.h"
#include "wireless.h"
#include "cfparse.h"
#include "network.h"
#include "dhcp.h"
#include "firewall.h"
#include "schedule.h"
#include "qos.h"
#include "global_access.h"
#include "cfmanager.h"
#include "config.h"
#include "mwan3.h"
#include "check.h"
#include "system.h"
#include "email.h"
#include "notification.h"
#include "grandstream.h"
#include "controller.h"
#include "upnpd.h"
#include "bwctrl.h"
#include "time.h"
#include "utils.h"
#include "smb.h"
#include "tr069.h"
#include "gsportalcfg.h"
#include "ecm.h"

//=================
//  Defines
//=================
#define CFG_PARSE_STATUS_PATH   SPLICE_STR(CM_FOLDER,/cfg_parse_status)
//=================
//  Typedefs
//=================

//=================
//  Globals
//=================

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
struct uci_package *
cfparse_init_package(
    const char *config
)
//=============================================================================
{
    struct uci_context *ctx = uci_ctx;
    struct uci_package *p = NULL;

    if ( !ctx ) {
        ctx = uci_alloc_context();
        uci_ctx = ctx;

        ctx->flags &= ~UCI_FLAG_STRICT;
        uci_set_confdir( uci_ctx, CM_CONFIG_PATH );
    }
    else {
        p = uci_lookup_package(ctx, config);
        if (p) {
            uci_unload(ctx, p);
        }
    }

    if ( uci_load( ctx, config, &p ) ) {
        cfmanager_log_message( L_DEBUG, "uci_load failed\n" );
        return NULL;
    }

    return p;
}

/*
 * The section is only used for "cfmanager" now,
 * if your parameter 'config' is not "cfmanager,
 * use LOAD_ALL_SECTION as section please.
 * If force_load is true, no memory differences are compared.
  */
//=============================================================================
int
cfparse_load_file(
    const char *config,
    int section,
    bool force_load
)
//=============================================================================
{
    cfmanager_log_message( L_DEBUG, "load %s force_load:%d\n", config, force_load );

    if ( strcmp( config, "tracks" ) == 0 ) {
        tracks_load();
    }
    else if ( strcmp( config, "wireless" ) == 0 ) {
        cfparse_load_wireless();
    }
    else if ( strcmp( config, "network" ) == 0 ) {
        cfparse_load_network();
    }
    else if ( strcmp( config, "dhcp" ) == 0 ) {
        cfparse_load_dhcp();
    }
    else if ( strcmp( config, "schedule" ) == 0 ) {
        cfparse_load_schedule();
    }
    else if ( strcmp( config, "qos" ) == 0 ) {
        cfparse_load_qos();
    }
    else if ( strcmp( config, "blackhole" ) == 0 ) {
        cfparse_load_blackhole();
    }
    else if ( strcmp( config, CFMANAGER_CONFIG_NAME ) == 0 ) {
        cm_load_cfmanager( section, force_load );
        cfmanager_resync_timer_start();
        cfubus_event_config_change();
    }
    else if ( strcmp( config, "mwan3" ) == 0 ) {
        cfparse_load_mwan3();
    }
    else if ( strcmp( config, "firewall" ) == 0 ) {
        cfparse_load_firewall();
    }
    else if ( strcmp( config, "system" ) == 0 ) {
        cfparse_load_sys();
    }
    else if ( strcmp( config, "email" ) == 0 ) {
        cfparse_load_email();
    }
    else if ( strcmp( config, "notification" ) == 0 ) {
        cfparse_load_notification();
    }
    else if ( strcmp( config, CF_CONFIG_NAME_GRANDSTREAM ) == 0 ) {
        cfparse_load_grandstream();
    }
    else if ( strcmp( config, "controller" ) == 0 ) {
        cfparse_load_controller();
    }
    else if ( strcmp( config, "upnpd" ) == 0 ) {
        cfparse_load_upnpd();
    }
    else if ( strcmp( config, "bwctrl" ) == 0 ) {
        cfparse_load_bwctrl();
    }
    else if ( strcmp( config, "samba" ) == 0 ) {
        cfparse_load_smb();
    }
    else if( strcmp( config, "tr069" ) == 0 ) {
        cfparse_load_tr069();
    }
    else if ( strcmp( config, "gsportalcfg" ) == 0 ) {
        cfparse_load_gsportalcfg();
    }
    else if ( strcmp( config, "ecm" ) == 0 ) {
        cfparse_load_ecm();
    }
    else {
    }
    return 0;
}

//=============================================================================
void
cfparse_scan_dir(
    const char *base
)
//=============================================================================
{
    DIR *dir;
    struct dirent *dent;

    dir = opendir( base );
    if ( dir == NULL ) {
        return;
    }

    for ( dent = readdir(dir); dent != NULL; dent = readdir(dir) ) {
        if ( dent->d_name[0] == '.' ) {
            continue;
        }
        if ( dent->d_type != DT_REG ) {
            continue;
        }
        if ( strcmp( dent->d_name, CF_CONFIG_NAME_GRANDSTREAM) == 0 || strcmp( dent->d_name, "tracks") == 0 ||
            strcmp( dent->d_name, CFMANAGER_CONFIG_NAME ) == 0 ||
            strcmp( dent->d_name, "firewall") == 0 ||
            strcmp( dent->d_name, "mwan3") == 0 ) {
            continue;
        }

        cfparse_load_file( dent->d_name, LOAD_ALL_SECTION, false );
    }

    closedir( dir );
}

//=============================================================================
void
cfparse_init(
    void
)
//=============================================================================
{
    FILE *fp = NULL;

    check_create_all_cfg();
    track_init();
    initd_load();
    cfparse_load_file( "tracks", LOAD_ALL_SECTION, false );
    cfparse_wireless_init();
    cfparse_network_init();
    cfparse_dhcp_init();
    cfparse_schedule_init();
    cfparse_sys_init();
    cfparse_grandstream_init();
    cfparse_controller_init();
    cfparse_upnpd_init();
    cfparse_bwctrl_init();
    cfparse_smb_init();
    cfparse_tr069_init();
    cfparse_gsportalcfg_init();
    cm_init();

    /* Loading the configuration file for
     * the first time does not require restarting
     * the relevant services
     */
    cm_sve_init.first_load_cfg = true;
    cfparse_scan_dir( CM_CONFIG_PATH );
    cm_sve_init.first_load_cfg = false;

    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_ALL, false );
    cfparse_load_firewall();
    cfparse_load_mwan3();

    if( cm_sve_init.boot_start ) {
        config_sync();

        fp = fopen( CFG_PARSE_STATUS_PATH, "w" );
        if( !fp ) {
            cfmanager_log_message( L_ERR, "create %s failed\n", CFG_PARSE_STATUS_PATH );
            return;
        }
    }

    if( fp ) {
        fclose( fp );
    }

    //Reset flag boot_start bit
    cm_sve_init.boot_start = false;
}

//=============================================================================
void
cfparse_deinit(
    void
)
//=============================================================================
{
    track_deinit();
    cfparse_wireless_deinit();
    cfparse_network_deinit();
    cfparse_dhcp_deinit();
    cfparse_schedule_deinit();
    cfparse_sys_deinit();
    cfparse_grandstream_deinit();
    cfparse_controller_deinit();
    cfparse_upnpd_deinit();
    cfparse_smb_deinit();
    cfparse_tr069_deinit();
    cfparse_gsportalcfg_deinit();
    cm_deinit();

    initd_unload();
}

