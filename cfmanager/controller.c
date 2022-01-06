/****************************************************************************
* *
* * FILENAME:        $RCSfile: controller.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/15
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
#include <unistd.h>
#include "global.h"
#include "track.h"
#include "initd.h"
#include "utils.h"
#include "cfparse.h"
#include "controller.h"
#include "apply.h"
#include "time.h"
#include "grandstream.h"
#include "cfmanager.h"

#include "config.h"
#include "check.h"


//=================
//  Defines
//=================
#define MAC_LEN 8

//=================
//  Typedefs
//=================


//=================
//  Globals
//=================
struct vlist_tree gs_controller_vltree;

//=================
//  Locals
//=================
/*
 * Private Date
 */
static struct blob_buf buf;
const struct blobmsg_policy controller_policy[__CONTROLLER_MAX] = {
    [CONTROLLER_ROLE] = {  .name = "role", .type = BLOBMSG_TYPE_STRING },
    [CONTROLLER_WORK_MODE] = {  .name = "workMode", .type = BLOBMSG_TYPE_STRING },
    [CONTROLLER_SET_FROM] = { .name = "from", .type = BLOBMSG_TYPE_INT32 },
};

static const struct uci_blob_param_list controller_list = {
    .n_params = __CONTROLLER_MAX,
    .params = controller_policy,
};

//=================
//  Functions
//=================

/*
 * Private Functions
 */
static int
cfparse_controller_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

//=============================================================================
static void
cfparse_controller_node_free(
    struct controller_config_parse *cfpar
)
//=============================================================================
{
    SAFE_FREE( cfpar->cf_section.config );
    SAFE_FREE( cfpar );
}

//=============================================================================
static void
cfparse_controller_tree_free(
    struct controller_config_parse *cfpar
)
//=============================================================================
{
    avl_delete( &gs_controller_vltree.avl, &cfpar->node.avl );
    cfparse_controller_node_free( cfpar );
}

//=============================================================================
int
cfparse_controller_del_paired_devices(
    void
)
//=============================================================================
{
    struct ubus_context * ctx = NULL;
    uint32_t search_id;
    static struct blob_buf b;
    char devices_mac[MAC_LEN] = { 0 };

    ctx = cfubus_get_ubus_context();
    if ( NULL == ctx ) {
        cfmanager_log_message( L_ERR, "get ubus_context failed\n" );
        return -1;
    }

    if ( ubus_lookup_id( ctx, "controller.discovery", &search_id ) ) {
        cfmanager_log_message( L_ERR, "look controller.discovery failed\n" );
        return -1;
    }

    //del all pairead devices
    blob_buf_init( &b,0 );
    snprintf( devices_mac, MAC_LEN, "all" );
    blobmsg_add_string( &b, "mac", devices_mac );
    ubus_invoke( ctx, search_id, "delete_device", b.head, NULL, NULL, 1000 );
    blob_buf_free( &b );

    return 0;
}

//=============================================================================
void
cfparse_controller_update_cfg(
    struct controller_config_parse *cfpar_old,
    struct controller_config_parse *cfpar_new
)
//=============================================================================
{
    int i = 0;
    int option = 0;
    struct blob_attr *tb_old[__CONTROLLER_MAX];
    struct blob_attr *tb_new[__CONTROLLER_MAX];

    struct blob_attr *config_old = cfpar_old->cf_section.config;
    struct blob_attr *config_new = cfpar_new->cf_section.config;

    blobmsg_parse( controller_policy,
            __CONTROLLER_MAX,
            tb_old,
            blob_data( config_old ),
            blob_len( config_old ) );

    blobmsg_parse( controller_policy,
            __CONTROLLER_MAX,
            tb_new,
            blob_data( config_new ),
            blob_len( config_new ) );

    for( i = 0; i < __CONTROLLER_MAX; i++ ) {
        if( !blob_attr_equal( tb_new[i], tb_old[i] ) ) {
            option |= cfparse_controller_hooker( tb_new, i, NULL );
        }
    }

    if ( option & OPTION_FLAGS_NEED_RELOAD ) {
        apply_set_reload_flag( CONFIG_TRACK_CONTROLLER );
    }
}

//=============================================================================
static void
cfparse_controller_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct controller_config_parse *cfpar_old = NULL;
    struct controller_config_parse *cfpar_new = NULL;

    if ( node_old ) {
        cfpar_old =
            container_of(node_old, struct controller_config_parse, node);
    }

    if ( node_new ) {
        cfpar_new =
            container_of(node_new, struct controller_config_parse, node);
    }

    if ( cfpar_old && cfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "update system section '%s'\n", cfpar_old->cf_section.name );

        if ( blob_attr_equal( cfpar_new->cf_section.config, cfpar_old->cf_section.config ) ) {
            cfparse_controller_node_free( cfpar_new );

            return;
        }

        cfparse_controller_update_cfg( cfpar_old, cfpar_new);

        SAFE_FREE( cfpar_old->cf_section.config );
        cfpar_old->cf_section.config = blob_memdup( cfpar_new->cf_section.config );
        cfparse_controller_node_free( cfpar_new );
    }
    else if ( cfpar_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete section '%s'\n", cfpar_old->cf_section.name );

        cfparse_controller_tree_free( cfpar_old );
        apply_set_reload_flag( CONFIG_TRACK_CONTROLLER );
    }
    else if ( cfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "New section '%s'\n", cfpar_new->cf_section.name );

        apply_set_reload_flag( CONFIG_TRACK_CONTROLLER );
    }
}

//=============================================================================
static int
cfparse_controller_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case CONTROLLER_WORK_MODE:
            rc |= OPTION_FLAGS_NEED_RELOAD;
            cfparse_controller_del_paired_devices();
            break;

        default:
            break;
     }

     return rc;
}

//=============================================================================
static void
cfparse_controller_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct controller_config_parse *cfpar = NULL;
    char *name;
    char *type;

    cfpar = calloc_a( sizeof( *cfpar ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type ) +1 );

    if ( !cfpar ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc network_config_parse '%s'\n", section_name );

        return;
    }

    cfpar->cf_section.name = strcpy( name, section_name );
    cfpar->cf_section.type = strcpy( type, section_type );
    cfpar->cf_section.config = blob_memdup( data );

    vlist_add( &gs_controller_vltree, &cfpar->node, cfpar->cf_section.name );
}


//=============================================================================
static void
cfparse_controller_uci_to_blob(
     struct uci_section *s
)
//=============================================================================
{
    blob_buf_init( &buf, 0 );
    uci_to_blob( &buf, s, &controller_list );
    cfparse_controller_add_blob_to_tree( buf.head, s->e.name, s->type );
    blob_buf_free( &buf );
}

//=============================================================================
int
cfparse_load_controller(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct uci_package *package = NULL;

    package = cfparse_init_package( CF_CONFIG_NAME_CONTROLLER );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load controller package failed\n" );
        return -1;
    }

    vlist_update( &gs_controller_vltree );

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );
        if( 0 == strcmp( s->type, "controller" ) ) {
            cfparse_controller_uci_to_blob( s );
        }
    }

    vlist_flush( &gs_controller_vltree );

    if( apply_get_reload_flag( CONFIG_TRACK_CONTROLLER ) ) {
        apply_add( "controller" );
        apply_flush_reload_flag( CONFIG_TRACK_CONTROLLER );
        apply_timer_start();
    }

    return 0;
}

//=============================================================================
void
cfparse_controller_init(
    void
)
//=============================================================================
{
    vlist_init( &gs_controller_vltree, avl_strcmp, cfparse_controller_vltree_update );
    gs_controller_vltree.keep_old = true;
    gs_controller_vltree.no_delete = true;
}

//=============================================================================
void
cfparse_controller_deinit(
     void
)
//=============================================================================
{
    vlist_flush_all( &gs_controller_vltree );
}
