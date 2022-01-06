/****************************************************************************
* *
* * FILENAME:        $RCSfile: schedule.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/04
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
#include <libubox/vlist.h>
#include <uci.h>
#include "global.h"
#include "track.h"
#include "initd.h"
#include "utils.h"
#include "schedule.h"
#include "cfparse.h"
#include "time.h"
#include "apply.h"
#include "config.h"


//=================
//  Defines
//=================
#define SCHEDULE_SCRIPT_NAME SPLICE_STR(CM_FOLDER,/schedule_apply.sh)
//=================
//  Typedefs
//=================

//=================
//  Globals
//=================

const struct blobmsg_policy schedule_policy[__SCH_MAX] = {
    [SCH_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
    [SCH_CLIENT_ACCESS] = { .name = "client_access", .type = BLOBMSG_TYPE_STRING },
    [SCH_BWCTRL] = { .name = "bwctrl", .type = BLOBMSG_TYPE_STRING },
    [SCH_FIREWALL] = { .name = "firewall", .type = BLOBMSG_TYPE_STRING },
    [SCH_REBOOT] = { .name = "reboot", .type = BLOBMSG_TYPE_STRING },
    [SCH_UPGRADE] = { .name = "upgrade", .type = BLOBMSG_TYPE_STRING },
    [SCH_WTIME1] = { .name = "wtime1", .type = BLOBMSG_TYPE_STRING },
    [SCH_WTIME2] = { .name = "wtime2", .type = BLOBMSG_TYPE_STRING },
    [SCH_WTIME3] = { .name = "wtime3", .type = BLOBMSG_TYPE_STRING },
    [SCH_WTIME4] = { .name = "wtime4", .type = BLOBMSG_TYPE_STRING },
    [SCH_WTIME5] = { .name = "wtime5", .type = BLOBMSG_TYPE_STRING },
    [SCH_WTIME6] = { .name = "wtime6", .type = BLOBMSG_TYPE_STRING },
    [SCH_WTIME7] = { .name = "wtime7", .type = BLOBMSG_TYPE_STRING },
    [SCH_ABTIME_LISTS] = { .name = "abtime", .type = BLOBMSG_TYPE_ARRAY },
};

struct vlist_tree sch_vltree;

//=================
//  Locals
//=================

/*
 * Private Functions
 */

/*
 * Private Data
 */
static const struct uci_blob_param_list schedule_policy_list = {
    .n_params = __SCH_MAX,
    .params = schedule_policy,
};

static struct blob_buf b;
//=================
//  Functions
//=================

//=============================================================================
static void
cfparse_schedule_node_free(
    struct schedule_config_parse *schcfpar
)
//=============================================================================
{
    SAFE_FREE( schcfpar->cf_section.config );
    SAFE_FREE( schcfpar );
}

//=============================================================================
static void
cfparse_schedule_tree_free(
    struct schedule_config_parse *schcfpar
)
//=============================================================================
{
    avl_delete( &sch_vltree.avl, &schcfpar->node.avl );

    cfparse_schedule_node_free( schcfpar );
}

//=============================================================================
static int
cfparse_schedule_hooker(
    FILE *fp,
    struct schedule_config_parse *schcfpar,
    struct blob_attr *new_config,
    int index
)
//=============================================================================
{
    int rc = 0;

    switch ( index ) {
        case SCH_SSID:
            break;
        case SCH_CLIENT_ACCESS:
            break;
        case SCH_BWCTRL:
            break;
        case SCH_REBOOT:
            break;
        case SCH_UPGRADE:
            break;
        case SCH_WTIME1:
            break;
        case SCH_WTIME2:
            break;
        case SCH_WTIME3:
            break;
        case SCH_WTIME4:
            break;
        case SCH_WTIME5:
            break;
        case SCH_WTIME6:
            break;
        case SCH_WTIME7:
            break;
        case SCH_ABTIME_LISTS:
            break;
        default:
            break;
    }

    rc |= OPTION_FLAGS_CONFIG_CHANGED;
    return rc;
}

//=============================================================================
static void
schedule_update(
    struct schedule_config_parse *schcfpar_old,
    struct schedule_config_parse *schcfpar_new
)
//=============================================================================
{
    struct blob_attr *tb_old[__SCH_MAX];
    struct blob_attr *tb_new[__SCH_MAX];
    char command[COMMAND_LEN] = { 0 };
    FILE *fp = NULL;
    int options = 0, i = 0;

    struct blob_attr *config_old = schcfpar_old->cf_section.config;
    struct blob_attr *config_new = schcfpar_new->cf_section.config;

    blobmsg_parse( schedule_policy,
            __SCH_MAX,
            tb_old,
            blob_data( config_old ),
            blob_len( config_old ) );

    blobmsg_parse( schedule_policy,
            __SCH_MAX,
            tb_new,
            blob_data( config_new ),
            blob_len( config_new ) );

    fp = fopen( SCHEDULE_SCRIPT_NAME, "wb" );
    if( !fp ) {
        printf( "create file: %s failed\n", SCHEDULE_SCRIPT_NAME );
        cfmanager_log_message( L_WARNING,
            "create file: %s failed\n", SCHEDULE_SCRIPT_NAME );
        options |= OPTION_FLAGS_NEED_RELOAD;
        goto end;
    }

    for( i = 0; i < __SCH_MAX; i++ ) {
        if( !blob_attr_equal( tb_new[i], tb_old[i] ) ) {
            options |= cfparse_schedule_hooker(fp, schcfpar_new, tb_new[i], i);

            //It's not right here, but I don't know how to deal with the validation of each value
            options |= OPTION_FLAGS_NEED_RELOAD;
        }
    }

end:
    if( options & OPTION_FLAGS_NEED_RELOAD ) {
        apply_set_reload_flag( CONFIG_TRACK_SCHEDULE );
    }
    else if( options & OPTION_FLAGS_CONFIG_CHANGED ) {
        snprintf( command, sizeof( command ),"sh -x %s", SCHEDULE_SCRIPT_NAME );
        cfmanager_log_message( L_DEBUG, "system( %s )\n",  command );
        system( command );
    }
}

//=============================================================================
static void
cfparse_schedule_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct schedule_config_parse *schcfpar_old = NULL;
    struct schedule_config_parse *schcfpar_new = NULL;

    if ( node_old )
        schcfpar_old =
            container_of(node_old, struct schedule_config_parse, node);

    if ( node_new )
        schcfpar_new =
            container_of(node_new, struct schedule_config_parse, node);

    if ( schcfpar_old && schcfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "update schedule config section '%s'\n", schcfpar_old->cf_section.name );

        if ( blob_attr_equal( schcfpar_old->cf_section.config, schcfpar_new->cf_section.config ) ) {
            cfparse_schedule_node_free( schcfpar_new );
            return;
        }

        schedule_update( schcfpar_old, schcfpar_new );

        SAFE_FREE( schcfpar_old->cf_section.config );
        schcfpar_old->cf_section.config = blob_memdup( schcfpar_new->cf_section.config );
        schcfpar_old->sch_option = schcfpar_new->sch_option;
        cfparse_schedule_node_free( schcfpar_new );
    }
    else if ( schcfpar_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete schedule config section '%s'\n", schcfpar_old->cf_section.name );

        cfparse_schedule_tree_free( schcfpar_old );

        apply_set_reload_flag( CONFIG_TRACK_SCHEDULE );
    }
    else if ( schcfpar_new ) {
        cfmanager_log_message( L_WARNING,
                "New schedule schedule section '%s'\n", schcfpar_new->cf_section.name );

        apply_set_reload_flag( CONFIG_TRACK_SCHEDULE );
    }
}

//=============================================================================
static void
cfparse_schedule_parse(
    struct blob_attr *data,
    const char *section_name
)
//=============================================================================
{
    struct blob_attr *tb[__SCH_MAX];
    struct schedule_config_parse *schcfpar = NULL;
    char *name;

    blobmsg_parse( schedule_policy,
        __SCH_MAX,
        tb,
        blob_data( data ),
        blob_len( data ) );

    schcfpar = calloc_a( sizeof( *schcfpar ), &name, strlen( section_name ) +1 );
    if ( !schcfpar ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc schedule_config_parse '%s'\n", section_name );

        return;
    }

    if( tb[SCH_SSID] ) {
        schcfpar->sch_option |= BIT( SCH_SSID );
    }
    if( tb[SCH_CLIENT_ACCESS] ) {
        schcfpar->sch_option |= BIT( SCH_CLIENT_ACCESS );
    }
    if( tb[SCH_BWCTRL] ) {
        schcfpar->sch_option |= BIT( SCH_BWCTRL );
    }
    if( tb[SCH_FIREWALL] ) {
        schcfpar->sch_option |= BIT( SCH_FIREWALL );
    }
    if( tb[SCH_REBOOT] ) {
        schcfpar->sch_option |= BIT( SCH_REBOOT );
    }
    if( tb[SCH_UPGRADE] ) {
        schcfpar->sch_option |= BIT( SCH_UPGRADE );
    }

    schcfpar->cf_section.name = strcpy( name, section_name );
    schcfpar->cf_section.config = blob_memdup( data );

    vlist_add( &sch_vltree, &schcfpar->node, schcfpar->cf_section.name );
}

//=============================================================================
static void
cfparse_schedule(
    struct uci_section *s
)
//=============================================================================
{
    blob_buf_init( &b, 0 );
    uci_to_blob( &b, s, &schedule_policy_list );
    cfparse_schedule_parse( b.head, s->e.name );
    blob_buf_free( &b );
}

//=============================================================================
int
cfparse_load_schedule(
    void
)
//=============================================================================
{
    struct uci_element *e;
    struct uci_package *package = NULL;

    package = cfparse_init_package( "schedule" );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load schedule package failed\n" );
        return -1;
    }

    vlist_update( &sch_vltree );

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        if( 0 == strcmp( s->type, "schedule" ) ) {
            cfparse_schedule( s );
        }
    }

    vlist_flush( &sch_vltree );

    if( apply_get_reload_flag( CONFIG_TRACK_SCHEDULE ) ) {
        apply_add( "schedule" );
        apply_flush_reload_flag( CONFIG_TRACK_SCHEDULE );
        apply_timer_start();
    }

    return 0;
}

//=============================================================================
void
cfparse_schedule_init(
    void
)
//=============================================================================
{
    vlist_init( &sch_vltree, avl_strcmp, cfparse_schedule_update );
    sch_vltree.keep_old = true;
    sch_vltree.no_delete = true;
}

//=============================================================================
void
cfparse_schedule_deinit(
    void
)
//=============================================================================
{
    vlist_flush_all( &sch_vltree );
}
