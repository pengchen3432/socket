/****************************************************************************
*
* FILENAME:        upnpd.c
*
* DESCRIPTION:     Description of this source file's contents
*
* Copyright (c) 2017 by Grandstream Networks, Inc.
* All rights reserved.
*
* This material is proprietary to Grandstream Networks, Inc. and,
* in addition to the above mentioned Copyright, may be
* subject to protection under other intellectual property
* regimes, including patents, trade secrets, designs and/or
* trademarks.
*
* Any use of this material for any purpose, except with an
* express license from Grandstream Networks, Inc. is strictly
* prohibited.
*
***************************************************************************/

//===========================
// Includes
//===========================
#include <libubox/vlist.h>
#include <uci.h>
#include "global.h"
#include "track.h"
#include "initd.h"
#include "utils.h"
#include "upnpd.h"
#include "apply.h"
#include "time.h"
#include "cfparse.h"
#include "check.h"
#include "sgrequest.h"
#include "cfmanager.h"
#include "config.h"

//===========================
// Defines
//===========================

//===========================
// Typedefs
//===========================
enum {
    UPNPD_VLTREE_ALL,
    UPNPD_VLTREE_UPNPD,
    UPNPD_VLTREE_PERM_RULE,

    __UPNPD_VLTREE_MAX
};

enum {
    UPNPD_HOOKER_ALL,
    UPNPD_HOOKER_UPNPD,
    UPNPD_HOOKER_PERM_RULE,

    __UPNPD_HOOKER_MAX
};


enum {
    UPNPD_PERM_RULE_ACTION,
    UPNPD_PERM_RULE_EXT_PORTS,
    UPNPD_PERM_RULE_INT_ADDR,
    UPNPD_PERM_RULE_INT_PORTS,
    UPNPD_PERM_RULE_COMMENT,

    __UPNPD_PERM_RULE_MAX,
};

//===========================
// Locals
//===========================
/* Variables */
struct vlist_tree upnpd_vltree;
struct vlist_tree perm_rule_vltree;

extern const struct blobmsg_policy upnp_policy[__UPNP_MAX];
/* Functions */

/*
 * Private Functions
 */
static int
cfparse_upnpd_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_upnpd_perm_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

/*
 * Private Data
 */

const struct blobmsg_policy upnpd_policy[__UPNPD_MAX] = {
    [UPNPD_ENABLE] = { .name = "enabled", .type = BLOBMSG_TYPE_BOOL },
    [UPNPD_ENABLE_NATPMP] = { .name = "enable_natpmp", .type = BLOBMSG_TYPE_BOOL },
    [UPNPD_ENABLE_UPNP] = { .name = "enable_upnp", .type = BLOBMSG_TYPE_BOOL },
    [UPNPD_SECURE_MODE] = { .name = "secure_mode", .type = BLOBMSG_TYPE_STRING },
    [UPNPD_LOG_OUTPUT] = { .name = "log_output", .type = BLOBMSG_TYPE_STRING },
    [UPNPD_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING },
    [UPNPD_UPNP_LEASE_FILE] = { .name = "upnp_lease_file", .type = BLOBMSG_TYPE_STRING },
    [UPNPD_EXTERNAL_IFACE] = { .name = "external_iface", .type = BLOBMSG_TYPE_STRING },
    [UPNPD_INTERNAL_IFACE] = { .name = "internal_iface", .type = BLOBMSG_TYPE_ARRAY }
};

const struct blobmsg_policy upnp_perm_rule_policy[__UPNPD_PERM_RULE_MAX] = {
    [UPNPD_PERM_RULE_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
    [UPNPD_PERM_RULE_EXT_PORTS] = { .name = "ext_ports", .type = BLOBMSG_TYPE_STRING },
    [UPNPD_PERM_RULE_INT_ADDR] = { .name = "int_addr", .type = BLOBMSG_TYPE_STRING },
    [UPNPD_PERM_RULE_INT_PORTS] = { .name = "int_ports", .type = BLOBMSG_TYPE_STRING },
    [UPNPD_PERM_RULE_COMMENT] = { .name = "comment", .type = BLOBMSG_TYPE_STRING },
};


static const struct uci_blob_param_list upndp_policy_list = {
    .n_params = __UPNPD_MAX,
    .params = upnpd_policy,
};

static const struct uci_blob_param_list perm_rule_policy_list = {
    .n_params = __UPNPD_PERM_RULE_MAX,
    .params = upnp_perm_rule_policy,
};

static const struct cm_hooker_policy cm_hp[__UPNPD_HOOKER_MAX] = {
    [UPNPD_HOOKER_UPNPD] = { .cb = cfparse_upnpd_hooker },
    [UPNPD_HOOKER_PERM_RULE] = { .cb = cfparse_upnpd_perm_rule_hooker },
};

static const struct cm_vltree_info upnpd_vltree_info[__UPNPD_VLTREE_MAX] = {
    [UPNPD_VLTREE_UPNPD] = {
        .key = "upnpd",
        .vltree = &upnpd_vltree,
        .policy_list = &upndp_policy_list,
        .hooker = UPNPD_HOOKER_UPNPD
    },

    [UPNPD_VLTREE_PERM_RULE] = {
        .key = "perm_rule",
        .vltree = &perm_rule_vltree,
        .policy_list = &perm_rule_policy_list,
        .hooker = UPNPD_HOOKER_PERM_RULE
    },

};

static struct blob_buf b;

//=================
//  Functions
//=================

//=============================================================================
static struct vlist_tree*
cfparse_upnpd_find_tree(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __UPNPD_VLTREE_MAX; i++ ) {

        if( !upnpd_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( upnpd_vltree_info[i].key, section_type ) ) {
            return upnpd_vltree_info[i].vltree;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",  section_type );

    return NULL;
}

//=============================================================================
static const struct uci_blob_param_list*
cfparse_upnpd_find_blob_param_list(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __UPNPD_VLTREE_MAX; i++ ) {
        if( !upnpd_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( upnpd_vltree_info[i].key, section_type ) ) {
            return upnpd_vltree_info[i].policy_list;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n", section_type );

    return NULL;
}

//=============================================================================
static void
cfparse_upnpd_node_free(
    struct upnpd_config_parse *upnp_cfpar
)
//=============================================================================
{
    SAFE_FREE( upnp_cfpar->cf_section.config );
    SAFE_FREE( upnp_cfpar );
}

//=============================================================================
static void
cfparse_upnpd_tree_free(
    struct vlist_tree *vltree,
    struct upnpd_config_parse *upnp_cfpar
)
//=============================================================================
{
    avl_delete( &vltree->avl, &upnp_cfpar->node.avl );

    cfparse_upnpd_node_free( upnp_cfpar );
}

//=============================================================================
static int
cfparse_upnpd_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch ( index ) {
        case UPNPD_ENABLE:
            break;
        case UPNPD_ENABLE_NATPMP:
            break;
        case UPNPD_ENABLE_UPNP:
            break;
        case UPNPD_SECURE_MODE:
            break;
        case UPNPD_LOG_OUTPUT:
            break;
        case UPNPD_PORT:
            break;
        case UPNPD_UPNP_LEASE_FILE:
            break;
        case UPNPD_EXTERNAL_IFACE:
            break;
        case UPNPD_INTERNAL_IFACE:
            break;
        default:
            break;
    }

    //rc |= OPTION_FLAGS_CONFIG_CHANGED;
    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
static int
cfparse_upnpd_perm_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch ( index ) {
        case UPNPD_PERM_RULE_ACTION:
            break;
        case UPNPD_PERM_RULE_EXT_PORTS:
            break;
        case UPNPD_PERM_RULE_INT_ADDR:
            break;
        case UPNPD_PERM_RULE_INT_PORTS:
            break;
        case UPNPD_PERM_RULE_COMMENT:
            break;

        default:
            break;
    }

    //rc |= OPTION_FLAGS_CONFIG_CHANGED;
    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
void
cfparse_upnpd_update_cfg(
    const struct blobmsg_policy *policy,
    struct upnpd_config_parse *upnpd_cfpar_old,
    struct upnpd_config_parse *upnpd_cfpar_new,
    int policy_size,
    int hooker
)
//=============================================================================
{
    struct blob_attr *tb_old[policy_size];
    struct blob_attr *tb_new[policy_size];
    struct cm_vltree_extend extend;
    int i = 0;
    int option = 0;

    struct blob_attr *config_old = upnpd_cfpar_old->cf_section.config;
    struct blob_attr *config_new = upnpd_cfpar_new->cf_section.config;

    blobmsg_parse( policy,
            policy_size,
            tb_old,
            blob_data( config_old ),
            blob_len( config_old ) );

    blobmsg_parse( policy,
            policy_size,
            tb_new,
            blob_data( config_new ),
            blob_len( config_new ) );

    memset( &extend, 0, sizeof( extend ) );

    for( i = 0; i < policy_size; i++ ) {
        if( !blob_attr_equal( tb_new[i], tb_old[i] ) ||
            extend.need_set_option & BIT(i) ) {

            if ( cm_hp[hooker].cb ) {
                option |= cm_hp[hooker].cb( tb_new, i, &extend );
            }
        }
    }

    if ( option & OPTION_FLAGS_NEED_RELOAD ) {
        apply_set_reload_flag( CONFIG_TRACK_UPNPD );
    }
}

//=============================================================================
static void
cfparse_upnpd_call_update_func(
    const char *section_type,
    struct upnpd_config_parse *upnpd_cfpar_old,
    struct upnpd_config_parse *upnpd_cfpar_new
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type || NULL == upnpd_cfpar_old || NULL == upnpd_cfpar_new ) {
        return;
    }

    for( i = 1; i < __UPNPD_VLTREE_MAX; i++ ) {
        if( 0 == strcmp( upnpd_vltree_info[i].key, section_type ) ) {
            cfparse_upnpd_update_cfg( upnpd_vltree_info[i].policy_list->params,
                upnpd_cfpar_old,
                upnpd_cfpar_new,
                upnpd_vltree_info[i].policy_list->n_params,
                upnpd_vltree_info[i].hooker );
        }
    }
}

//=============================================================================
static void
cfparse_upnpd_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct upnpd_config_parse *upnpd_cfpar_old = NULL;
    struct upnpd_config_parse *upnpd_cfpar_new = NULL;

    if ( node_old ) {
        upnpd_cfpar_old =
            container_of(node_old, struct upnpd_config_parse, node);
    }

    if ( node_new ) {
        upnpd_cfpar_new =
            container_of(node_new, struct upnpd_config_parse, node);
    }

    if ( upnpd_cfpar_old && upnpd_cfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "update upnp section type '%s', name '%s'\n",
            upnpd_cfpar_old->cf_section.type, upnpd_cfpar_old->cf_section.name );

        if ( blob_attr_equal( upnpd_cfpar_new->cf_section.config, upnpd_cfpar_old->cf_section.config ) ) {
            cfparse_upnpd_node_free( upnpd_cfpar_new );
            return;
        }

        cfparse_upnpd_call_update_func( upnpd_cfpar_old->cf_section.type, upnpd_cfpar_old, upnpd_cfpar_new );

        SAFE_FREE( upnpd_cfpar_old->cf_section.config );
        upnpd_cfpar_old->cf_section.config = blob_memdup( upnpd_cfpar_new->cf_section.config );
        cfparse_upnpd_node_free( upnpd_cfpar_new );
        apply_set_reload_flag( CONFIG_TRACK_UPNPD );
    }
    else if ( upnpd_cfpar_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete upnp section type '%s', name '%s'\n",
            upnpd_cfpar_old->cf_section.type, upnpd_cfpar_old->cf_section.name );

        vltree = cfparse_upnpd_find_tree( upnpd_cfpar_old->cf_section.type );
        if( NULL == vltree ) {
            return;
        }
        cfparse_upnpd_tree_free( vltree, upnpd_cfpar_old );
        apply_set_reload_flag( CONFIG_TRACK_UPNPD );
    }
    else if ( upnpd_cfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "New upnp section type '%s', name '%s'\n",
            upnpd_cfpar_new->cf_section.type, upnpd_cfpar_new->cf_section.name );
        apply_set_reload_flag( CONFIG_TRACK_UPNPD );
    }
}

//=============================================================================
static void
cfparse_upnpd_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct upnpd_config_parse *upnpd_cfpar = NULL;
    struct vlist_tree *vltree = NULL;
    char *name;
    char *type;

    vltree = cfparse_upnpd_find_tree( section_type );
    if( NULL == vltree ) {
        cfmanager_log_message( L_DEBUG, "No corresponding binary tree was found according to %s\n",
            section_type );
        return;
    }

    upnpd_cfpar = calloc_a( sizeof( *upnpd_cfpar ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type ) +1 );

    if ( !upnpd_cfpar ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc upnpd_config_parse '%s'\n", section_name );

        return;
    }

    upnpd_cfpar->cf_section.name = strcpy( name, section_name );
    upnpd_cfpar->cf_section.type = strcpy( type, section_type );
    upnpd_cfpar->cf_section.config = blob_memdup( data );

    vlist_add( vltree, &upnpd_cfpar->node, upnpd_cfpar->cf_section.name );
}

//=============================================================================
static void
cfparse_upnpd_uci_to_blob(
    struct uci_section *s
)
//=============================================================================
{
    const struct uci_blob_param_list* uci_blob_list = NULL;

    blob_buf_init( &b, 0 );

    uci_blob_list = cfparse_upnpd_find_blob_param_list( s->type );
    if( NULL == uci_blob_list ) {
        cfmanager_log_message( L_DEBUG, 
            "No corresponding uci_blob_param_list was found according to %s\n",
            s->type );

        return;
    }
    uci_to_blob( &b, s, uci_blob_list );

    cfparse_upnpd_add_blob_to_tree( b.head, s->e.name, s->type );

    blob_buf_free( &b );
}

//=============================================================================
int
upnpd_reset_by_wan(
    char *wan
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    int should_commit = 0;

    vlist_for_each_element( &cm_upnp_vltree, cm_cfg, node ) {
        struct blob_attr *tb[__UPNP_MAX];
        char *wan_intf;
        char path[LOOKUP_STR_SIZE] = { 0 };

        blobmsg_parse( upnp_policy,
            __UPNP_MAX,
            tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        wan_intf = blobmsg_get_string( tb[UPNP_INTF] );
        if ( !strcmp(wan_intf, wan) ) {
            cfmanager_log_message( L_WARNING,
                "Disable upnp and reset interface to wan0, because %s disabled!\n",
                wan );
            snprintf( path, sizeof( path ), "%s.upnp.%s",
                CFMANAGER_CONFIG_NAME, upnp_policy[UPNP_ENABLE].name );
            config_uci_set( path, "0", 0 );
            snprintf( path, sizeof( path ), "%s.upnp.%s",
                CFMANAGER_CONFIG_NAME, upnp_policy[UPNP_INTF].name );
            config_uci_set( path, "wan0", 0 );

            should_commit = 1;
        }
    }

    if ( should_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_UPNP, false );
    }

    return 0;
}

//=============================================================================
int
upnpd_parse_upnp(
    struct blob_attr *attr,
    int *fw_need_load
)
//=============================================================================
{
    struct blob_attr *new_tb[__UPNP_MAX];
    struct blob_attr *old_tb[__UPNP_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int i = 0;
    int same_cfg = 1;
    int ret = ERRCODE_SUCCESS;

    blobmsg_parse( upnp_policy,
        __UPNP_MAX,
        new_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    for ( i = 0; i < __UPNP_MAX; i++ ) {
        if ( i != UPNP_MAPPING_TABLE && !new_tb[i] ) {
            cfmanager_log_message( L_ERR,
                "Missing %s in json!\n", upnp_policy[i].name );
            return ERRCODE_PARAMETER_ERROR;
        }
    }

    *fw_need_load = 0;
    cm_cfg = util_get_vltree_node( &cm_upnp_vltree, VLTREE_CM_TREE, "upnp" );
    if ( !cm_cfg ) {
        cfmanager_log_message( L_WARNING, "NOT found upnp in %s, create it...\n", 
            CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "upnp", "upnp" );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section upnp in %s\n", CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __UPNP_MAX; i++ ) {
            if ( i == UPNP_MAPPING_TABLE ) {
                continue;
            }

            snprintf( path, sizeof( path ), "%s.upnp.%s",
                CFMANAGER_CONFIG_NAME, upnp_policy[i].name );
            config_set_by_blob( new_tb[i], path, upnp_policy[i].type );
        }

        same_cfg = 0;
        *fw_need_load = 1;
    }
    else {
        cfmanager_log_message( L_WARNING, "Update upnp in %s\n", CFMANAGER_CONFIG_NAME );

        blobmsg_parse( upnp_policy,
            __UPNP_MAX,
            old_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __UPNP_MAX; i++ ) {
            if ( !new_tb[i] ) {
                continue;
            }

            if ( sgreq_compar_attr( new_tb[i], old_tb[i], upnp_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ), "%s.upnp.%s",
                CFMANAGER_CONFIG_NAME, upnp_policy[i].name );
            config_set_by_blob( new_tb[i], path, upnp_policy[i].type );

            same_cfg = 0;
            *fw_need_load = 1;
        }
    }

    if ( same_cfg ) {
        cfmanager_log_message( L_DEBUG, "The upnp config is same\n" );
    }

    return ret;
}

//=============================================================================
int
cfparse_load_upnpd(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct vlist_tree *vltree = NULL;
    struct uci_package *package = NULL;
    int i = 0;

    //check_set_defvalue( CHECK_UPNPD );

    package = cfparse_init_package( "upnpd" );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load upnpd package failed\n" );
        return -1;
    }

    for( i = 1; i < __UPNPD_VLTREE_MAX; i++ ) {

        vltree = upnpd_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_update( vltree );
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        cfparse_upnpd_uci_to_blob( s );
    }

    for( i = 1; i < __UPNPD_VLTREE_MAX; i++ ) {
        vltree = upnpd_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush( vltree );
    }

    if( apply_get_reload_flag( CONFIG_TRACK_UPNPD ) ) {
        apply_add( "upnpd" );
        apply_flush_reload_flag( CONFIG_TRACK_UPNPD );
        apply_timer_start();
    }

    return 0;
}

//=============================================================================
void
cfparse_upnpd_init(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __UPNPD_VLTREE_MAX; i++ ) {

        vltree = upnpd_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_init( vltree, avl_strcmp, cfparse_upnpd_vltree_update );
        vltree->keep_old = true;
        vltree->no_delete = true;
    }

    // Load upnpd configure when cfmanager re-start.
    cfparse_load_upnpd();
}

//=============================================================================
void
cfparse_upnpd_deinit(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __UPNPD_VLTREE_MAX; i++ ) {

        vltree = upnpd_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush_all( vltree );
    }
}

/* EOF */

