/****************************************************************************
* *
* * FILENAME:        $RCSfile: track.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/12
* *
* * DESCRIPTION:     xxxx feature:
* *
* *
* * Copyright (c) 2021 by Grandstream Networks, Inc.
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
#include "track.h"
#include "initd.h"
#include "global.h"
#include "utils.h"
#include "cfparse.h"
#include "time.h"
#include "cfmanager.h"
//=================
//  Defines
//=================

//=================
//  Typedefs
//=================

//=================
//  Globals
//=================
int apply_status = APPLY_CMPL;

struct track_exec_param tk_ec_pm[__CONFIG_TRACK_MAX] = {
    [CONFIG_TRACK_WIRELESS] = { .name = "wireless" },
    [CONFIG_TRACK_NETWORK] = { .name = "network" },
    [CONFIG_TRACK_DHCP] = { .name = "dhcp" },
    [CONFIG_TRACK_FIREWALL] = { .name = "firewall" },
    [CONFIG_TRACK_SCHEDULE] = { .name = "schedule" },
    [CONFIG_TRACK_QOS] = { .name = "qos" },
    [CONFIG_TRACK_BLACKHOLE] = { .name = "blackhole" },
    [CONFIG_TRACK_SYSTEM] = { .name = "system" },
    [CONFIG_TRACK_EMAIL] = { .name = "email" },
    [CONFIG_TRACK_NOTIFY] = { .name = "notification" },
    [CONFIG_TRACK_CONTROLLER] = { .name = "controller" },
    [CONFIG_TRACK_UPNPD] = { .name = "upnpd" },
    [CONFIG_TRACK_DDNS] = { .name = "ddns" },
    [CONFIG_TRACK_GRANDSTREAM] = { .name = "grandstream" },
    [CONFIG_TRACK_MWAN3] = { .name = "mwan3" },
    [CONFIG_TRACK_BWCTRL] = { .name = "bwctrl" },
    [CONFIG_TRACK_WIFIISOLATE] = { .name = "wifi_isolate" },
    [CONFIG_TRACK_SAMBA] = { .name = "samba" },
    [CONFIG_TRACK_GSPORTAL] = { .name = "gsportalcfg" },
    [CONFIG_TRACK_PORTALCGI] = { .name = "portalfcgi" }, 
    [CONFIG_TRACK_TR069] = { .name = "tr069" },
    [CONFIG_TRACK_ECM] = { .name = "ecm" },

};
//=================
//  Locals
//=================

/*
 * Private Functions
 */

/*
 * Private Data
 */

struct track_process {
    struct uloop_process uloop;
    void (*cb)( struct track_process*, int ret );
};

static struct avl_tree ucitracks;
static struct track_process tk_pro;

//=================
//  Functions
//=================
/*
config network
    option init network
    list affects dhcp

config wireless
    list affects network

config firewall
    option init firewall
    list affects qos

config dhcp
    option init dnsmasq

config system
    option init log
    option exec 'etc/init.d/sysntpd restart'

config qos
    option init qos
 */

//=============================================================================
int
track_create(
    struct uci_context *uci_ctx,
    struct uci_section *s
)
//=============================================================================
{
    struct track *t = NULL;

    const char *value = NULL;
    struct uci_option *affects;
    struct uci_element *e;

    t = calloc( 1, sizeof(struct track) );
    if ( !t )
        return -1;

    t->name = strdup( s->type );
    if( !t->name ) {
        SAFE_FREE( t );
        return -1;
    }

    cfmanager_log_message( L_DEBUG, "t->name %s\n", t->name );

    value = uci_lookup_option_string( uci_ctx, s, "init" );
    if( value ) {
        t->init = strdup( value );
    }

    value = uci_lookup_option_string( uci_ctx, s, "exec" );
    if ( value ) {
        t->exec = strdup( value );
    }

    affects = uci_lookup_option( uci_ctx, s, "affects" );
    if ( affects && ( affects->type != UCI_TYPE_LIST ) ) {
        SAFE_FREE( t->name );
        SAFE_FREE( t->init );
        SAFE_FREE( t->exec );
        SAFE_FREE( t );
        return -1;
    }

    if ( affects ) {
        uci_foreach_element( &affects->v.list, e ) {
            if ( strcmp( e->name, "wireless" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_WIRELESS);
            }
            else if ( strcmp( e->name, "network" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_NETWORK);
            }
            else if ( strcmp( e->name, "dhcp" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_DHCP);
            }
            else if ( strcmp( e->name, "firewall" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_FIREWALL);
            }
            else if ( strcmp( e->name, "schedule" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_SCHEDULE);
            }
            else if ( strcmp( e->name, "qos" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_QOS);
            }
            else if ( strcmp( e->name, "email" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_EMAIL);
            }
            else if ( strcmp( e->name, "notification" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_NOTIFY);
            }
            else if ( strcmp( e->name, "blackhole" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_BLACKHOLE);
            }
            else if ( strcmp( e->name, "controller" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_CONTROLLER);
            }
            else if ( strcmp( e->name, "grandstream" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_GRANDSTREAM);
            }
            else if ( strcmp( e->name, "mwan3" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_MWAN3);
            }
            else if ( strcmp( e->name, "bwctrl" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_BWCTRL);
            }
            else if ( strcmp( e->name, "wifi_isolate" ) == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_WIFIISOLATE);
            }
            else if (strcmp(e->name, "samba") == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_SAMBA);
            }
            else if (strcmp(e->name, "gsportalcfg") == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_GSPORTAL);
            }
            else if (strcmp(e->name, "portalfcgi") == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_PORTALCGI);
            }
            else if (strcmp(e->name, "ecm") == 0 ) {
                t->affects |= BIT(CONFIG_TRACK_ECM);
            }
        }
    }

    t->avl.key = t->name;
    avl_insert( &ucitracks, &t->avl );

    return 0;
}

//=============================================================================
void
track_affects(
    struct track *t,
    char *command,
    int   size
)
//=============================================================================
{
    struct track *tk = NULL;
    struct initd *node;
    uint32_t handle = 0;;
    int    remain = 0;
    int    i = 0;

    remain = size - strlen( command );

    if ( t->init ) {
        node = initd_get( t->init );
        if ( node ) {
            node->reload = 1;
        }
        else {
            cfmanager_log_message( L_WARNING, "node is NULL\n" );
        }
    }
    if ( t->exec ) {
        snprintf( command + strlen( command ), remain, "%s&&", t->exec );
    }

    for ( i = 0; i < __CONFIG_TRACK_MAX; i++ ) {

        if ( t->affects & BIT(i) ) {
            if ( handle & BIT(i) ) {
                continue;
            }

            switch( i ) {
                case CONFIG_TRACK_WIRELESS:
                    tk = track_get( "wireless" );
                    break;
                case CONFIG_TRACK_NETWORK:
                    tk = track_get( "network" );
                    break;
                case CONFIG_TRACK_DHCP:
                    tk = track_get( "dhcp" );
                    break;
                case CONFIG_TRACK_FIREWALL:
                    tk = track_get( "firewall" );
                    break;
                case CONFIG_TRACK_SCHEDULE:
                    tk = track_get( "schedule" );
                    break;
                case CONFIG_TRACK_QOS:
                    tk = track_get( "qos" );
                    break;
                case CONFIG_TRACK_BLACKHOLE:
                    tk = track_get( "blackhole" );
                    break;
                case CONFIG_TRACK_SYSTEM:
                    tk = track_get( "system" );
                    break;
                case CONFIG_TRACK_EMAIL:
                    tk = track_get( "email" );
                    break;
                case CONFIG_TRACK_NOTIFY:
                    tk = track_get( "notification" );
                    break;
                case CONFIG_TRACK_CONTROLLER:
                    tk = track_get( "controller" );
                    break;
                case CONFIG_TRACK_UPNPD:
                    tk = track_get( "upnpd" );
                    break;
                case CONFIG_TRACK_DDNS:
                    tk = track_get( "ddns" );
                    break;
                case CONFIG_TRACK_GRANDSTREAM:
                    tk = track_get( "grandstream" );
                    break;
                case CONFIG_TRACK_MWAN3:
                    tk = track_get( "mwan3" );
                    break;
                case CONFIG_TRACK_BWCTRL:
                    tk = track_get( "bwctrl" );
                    break;
                case CONFIG_TRACK_WIFIISOLATE:
                    tk = track_get( "wifi_isolate" );
                    break;
                case CONFIG_TRACK_SAMBA:
                    tk = track_get("samba");
                    break;
                case CONFIG_TRACK_GSPORTAL:
                    tk = track_get("gsportalcfg");
                    break;
                case CONFIG_TRACK_PORTALCGI:
                    tk = track_get("portalfcgi");
                    break;
                case CONFIG_TRACK_ECM:
                    tk = track_get("ecm");
                    break;
            }

            handle |= BIT(i);

            if ( tk ) {
                track_affects( tk, command, remain );
            }
        }
    }
}

//=============================================================================
static void
track_process_cb(
    struct uloop_process *proc,
    int ret
)
//=============================================================================
{
    apply_status = APPLY_CMPL;
    apply_execl_timer_stop();

    cfmanager_log_message( L_DEBUG, "Command execution complete\n" );
}

//=============================================================================
static int
track_execl(
    const char *cmd
)
//=============================================================================
{
    int pid = 0;

    if( !cmd ) {
        return -1;
    }

    if ( ( pid = fork() ) < 0 ) {
        // TODO:Continue with the command?
        cfmanager_log_message( L_ERR, "fork failed\n" );
        return -1;
    }

    if ( !pid ) {
        execl("/bin/sh", "sh", "-c", cmd, (char*)0);
        cfmanager_log_message( L_ERR, "Command execution failed\n" );
        exit(127);
    }

    tk_pro.uloop.cb = track_process_cb;
    tk_pro.uloop.pid = pid;
    uloop_process_add( &tk_pro.uloop );

    //Set the timeout for command execution
    apply_status = APPLY_IN_PROCESS;
    apply_execl_timer_start();

    cfmanager_log_message( L_DEBUG, "Executing command\n" );
    return 0;
}

//=============================================================================
void
track_apply(
    void
)
//=============================================================================
{
    struct track *t;
    char command_exec[512];
    char command_init[512];
    char command_total[1024];
    int  len = 0;

    if( cm_sve_init.boot_start || cm_sve_init.first_load_cfg ) {
        cfmanager_log_message( L_ERR, "Wait for configuration loading to complete\n" );
        return;
    }

    memset( command_exec, 0, sizeof( command_exec ) );
    memset( command_init, 0, sizeof( command_init ) );
    memset( command_total, 0, sizeof( command_total ) );

    avl_for_each_element( &ucitracks, t, avl ) {
        if ( t->flags & OPTION_FLAGS_NEED_RELOAD ) {
            track_affects( t, command_exec, sizeof( command_exec ) );
            t->flags &= ~OPTION_FLAGS_NEED_RELOAD;
        }
    }

    //first run init
    initd_apply( command_init, sizeof( command_init ) );

    //next run exec
    util_del_dup_cmd( command_exec, sizeof( command_exec ) );

    snprintf( command_total, sizeof( command_total ), "%s%s", command_init, command_exec );
    len = strlen( command_total );
    if ( len && command_total[len - 2] == '&' ) {
        command_total[len - 2] = '\0';
        cfmanager_log_message( L_DEBUG, "run exec %s \n", command_total );

        track_execl( command_total );
    }
}

//=============================================================================
struct track *
track_get(
    const char *name
)
//=============================================================================
{
    struct track *t;

    if ( !name )
        return NULL;

    t = avl_find_element( &ucitracks, name, t, avl );
    if ( !t )
        return NULL;

    return t;
}

//=============================================================================
int
tracks_load(
    void
)
//=============================================================================
{
    struct uci_element *e;
    struct uci_package *package = NULL;

    package = cfparse_init_package( "tracks" );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load tracks package failed\n" );
        return -1;
    }

    cfmanager_log_message( L_DEBUG, "tracks_load ...\n" );
    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section(e);
        if ( track_create( uci_ctx, s ) != 0 ) {
            cfmanager_log_message( L_ERR,
                "create track %s failed\n", s->type );
        }
    }

    return 0;
}

//=============================================================================
void
track_init(
    void
)
//=============================================================================
{
    avl_init( &ucitracks, avl_strcmp, false, NULL );
}

//=============================================================================
void
track_deinit(
    void
)
//=============================================================================
{
    struct track *t, *temp;


    avl_for_each_element_safe(&ucitracks, t, avl, temp) {
        avl_delete( &ucitracks, &t->avl );
        SAFE_FREE( t->name );
        SAFE_FREE( t->init );
        SAFE_FREE( t->exec );
        SAFE_FREE( t );
    }
}

#if 0
const char *app_name = "tracks";
//=============================================================================
int
main(
    int argc,
    char **argv
)
//=============================================================================
{
    struct uci_context *ctx = NULL;
    struct uci_package *p = NULL;
    struct track *t;

    openlog( app_name, 0, LOG_DAEMON );

    ctx = uci_alloc_context();

    ctx->flags &= ~UCI_FLAG_STRICT;

    uci_load( ctx, "tracks", &p );

    initd_load();
    track_init();
    tracks_load( ctx, p );
    t = track_get( "network" );
    if ( t )
        t->flags |= OPTION_FLAGS_NEED_RELOAD;
    track_apply();

    track_deinit();
    initd_unload();
    closelog();
    return 0;
}
#endif
