/****************************************************************************
* *
* * FILENAME:        $RCSfile: tr069.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/10/13
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
#include "network.h"
#include "time.h"
#include "apply.h"
#include "sgrequest.h"
#include "cfparse.h"
#include "check.h"
#include "tr069.h"


//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
enum {
    __TR069_VLTREE_ALL,
    TR069_VLTREE,

    __TR069_VLTREE_MAX
};

enum {
    TR069_HOOKER_TR069,

    __TR069_HOOKER_MAX
};

enum {
    TR069_CFG_ENABLE,
    TR069_CFG_ACS_URL,
    TR069_CFG_ACS_NAME,
    TR069_CFG_ACS_PASSWORD,
    TR069_CFG_PERIODIC_INFORM_ENABLE,
    TR069_CFG_PERIODIC_INFORM_INTERVAL,
    TR069_CFG_CPE_CERT_FILE,
    TR069_CFG_CPE_CERT_KEY,

    __TR069_CFG_MAX
};
//=================
//  Globals
//=================
struct vlist_tree tr069_vltree;
//=================
//  Locals
//=================

/*
 * Private Functions
 */
static int
cfparse_tr069_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);
/*
 * Private Data
 */
static struct blob_buf b;

static const struct blobmsg_policy tr069_cfg_policy[__TR069_CFG_MAX] = {
    [TR069_CFG_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_STRING },
    [TR069_CFG_ACS_URL] = { .name = "acs_url", .type = BLOBMSG_TYPE_STRING },
    [TR069_CFG_ACS_NAME] = { .name = "acs_name", .type = BLOBMSG_TYPE_STRING },
    [TR069_CFG_ACS_PASSWORD] = { .name = "acs_password", .type = BLOBMSG_TYPE_STRING },
    [TR069_CFG_PERIODIC_INFORM_ENABLE] = { .name = "periodic_inform_enable", .type = BLOBMSG_TYPE_STRING },
    [TR069_CFG_PERIODIC_INFORM_INTERVAL] = { .name = "periodic_inform_interval", .type = BLOBMSG_TYPE_STRING },
    [TR069_CFG_CPE_CERT_FILE] = { .name = "cpe_cert_file", .type = BLOBMSG_TYPE_STRING },
    [TR069_CFG_CPE_CERT_KEY] = { .name = "cpe_cert_key", .type = BLOBMSG_TYPE_STRING },
};

static const struct cm_hooker_policy tr069_hp[__TR069_HOOKER_MAX] ={
    [TR069_HOOKER_TR069] = { .cb = cfparse_tr069_hooker },
};

static const struct uci_blob_param_list tr069_list = {
    .n_params = __TR069_CFG_MAX,
    .params = tr069_cfg_policy,
};

static const struct cm_vltree_info tr069_vltree_info[__TR069_VLTREE_MAX] = {
    [TR069_VLTREE] = {
        .key = "tr069",
        .vltree = &tr069_vltree,
        .policy_list = &tr069_list,
        .hooker = TR069_HOOKER_TR069
    },
};
//=================
//  Functions
//=================

//=============================================================================
static const struct uci_blob_param_list*
cfparse_tr069_find_blob_param_list(
    const char *section_type
)
//=============================================================================
{
    int i = 0;
    const char *key = NULL;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __TR069_VLTREE_MAX; i++ ) {
        key = tr069_vltree_info[i].key;

        if( !key ) {
            continue;
        }

        if( 0 == strcmp( key, section_type ) ) {
            return tr069_vltree_info[i].policy_list;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static struct vlist_tree*
cfparse_tr069_find_tree(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( !section_type ) {
        return NULL;
    }

    for( i = 1; i < __TR069_VLTREE_MAX; i++ ) {

        if( !tr069_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( tr069_vltree_info[i].key, section_type ) ) {
            return tr069_vltree_info[i].vltree;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static void
cfparse_tr069_node_free(
    struct tr069_config_parse *node
)
//=============================================================================
{
    SAFE_FREE( node->cf_section.config );
    SAFE_FREE( node );
}

//=============================================================================
static void
cfparse_tr069_tree_free(
    struct vlist_tree *vltree,
    struct tr069_config_parse *node
)
//=============================================================================
{
    avl_delete( &vltree->avl, &node->node.avl );

    cfparse_tr069_node_free( node );
}

//=============================================================================
static int
cfparse_tr069_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;

    switch( index ) {
        case TR069_CFG_ENABLE:
            break;
        case TR069_CFG_ACS_URL:
            break;
        case TR069_CFG_ACS_NAME:
            break;
        case TR069_CFG_ACS_PASSWORD:
            break;
        case TR069_CFG_PERIODIC_INFORM_ENABLE:
            break;
        case TR069_CFG_PERIODIC_INFORM_INTERVAL:
            break;
        case TR069_CFG_CPE_CERT_FILE:
            break;
        case TR069_CFG_CPE_CERT_KEY:
            break;
        default:
            break;
    }

    option |= OPTION_FLAGS_NEED_RELOAD;

    return option;
}

//=============================================================================
void
cfparse_tr069_update_cfg(
    const struct blobmsg_policy *policy,
    struct tr069_config_parse *tr069_cfg_old,
    struct tr069_config_parse *tr069_cfg_new,
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

    struct blob_attr *config_old = tr069_cfg_old->cf_section.config;
    struct blob_attr *config_new = tr069_cfg_new->cf_section.config;

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

    extend.section_name = tr069_cfg_old->cf_section.name;

    for( i = 0; i < policy_size; i++ ) {
        if( !blob_attr_equal( tb_new[i], tb_old[i] ) ) {

            if ( tr069_hp[hooker].cb ) {
                option |= tr069_hp[hooker].cb( tb_new, i, &extend );
            }
        }
    }

    if ( option & OPTION_FLAGS_NEED_RELOAD ) {
        apply_set_reload_flag( CONFIG_TRACK_TR069 );
    }
}

//=============================================================================
static void
cfparse_tr069_call_update_func(
    const char *section_type,
    struct tr069_config_parse *tr069_cfg_old,
    struct tr069_config_parse *tr069_cfg_new
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type || NULL == tr069_cfg_old || NULL == tr069_cfg_new ) {
        return;
    }

    for( i = 1; i < __TR069_VLTREE_MAX; i++ ) {

        if( 0 == strcmp( tr069_vltree_info[i].key, section_type ) ) {

            cfparse_tr069_update_cfg( tr069_vltree_info[i].policy_list->params,
                tr069_cfg_old,
                tr069_cfg_new,
                tr069_vltree_info[i].policy_list->n_params,
                tr069_vltree_info[i].hooker );
        }
    }
}

//=============================================================================
static void
cfparse_tr069_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct tr069_config_parse *tr069_cfg_new = NULL;
    struct tr069_config_parse *tr069_cfg_old = NULL;

    if( node_old ) {
        tr069_cfg_old =
            container_of( node_old, struct tr069_config_parse, node );
    }

    if( node_new ) {
        tr069_cfg_new =
            container_of( node_new, struct tr069_config_parse, node );
    }

    if( node_old && node_new ) {
        cfmanager_log_message( L_WARNING,
            "update tr069 section '%s'\n", tr069_cfg_old->cf_section.name );

        if ( blob_attr_equal( tr069_cfg_new->cf_section.config, tr069_cfg_old->cf_section.config ) ) {
            cfparse_tr069_node_free( tr069_cfg_new );
            cfmanager_log_message( L_DEBUG,
                "the config section '%s' is same\n", tr069_cfg_old->cf_section.name );

            return;
        }

        cfparse_tr069_call_update_func( tr069_cfg_old->cf_section.type, tr069_cfg_old, tr069_cfg_new );

        SAFE_FREE( tr069_cfg_old->cf_section.config );
        tr069_cfg_old->cf_section.config = blob_memdup( tr069_cfg_new->cf_section.config );
        cfparse_tr069_node_free( tr069_cfg_new );
    }
    else if( node_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete tr069 section '%s'\n", tr069_cfg_old->cf_section.name );

        vltree = cfparse_tr069_find_tree( tr069_cfg_old->cf_section.type );
        if( NULL == vltree ) {
            cfmanager_log_message( L_DEBUG, "no tree corresponding to %s was found\n",
                tr069_cfg_old->cf_section.type );

            return;
        }
        cfparse_tr069_tree_free( vltree, tr069_cfg_old );

        apply_set_reload_flag( CONFIG_TRACK_TR069 );
    }
    else if( node_new ) {
        cfmanager_log_message( L_WARNING,
            "New tr069 section '%s'\n", tr069_cfg_new->cf_section.name );

        apply_set_reload_flag( CONFIG_TRACK_TR069 );
    }
}

//=============================================================================
static void
cfparse_tr069_add_blob_to_tree(
    struct blob_attr *data,
    char *section_name,
    char *section_type
)
//=============================================================================
{
    struct tr069_config_parse *tr069_cfg = NULL;
    struct vlist_tree *vltree = NULL;
    char *name = NULL;
    char *type = NULL;

    vltree = cfparse_tr069_find_tree( section_type );
    if( NULL == vltree ) {
        cfmanager_log_message( L_DEBUG, "No corresponding binary tree was found according to %s\n",
            section_type );

        return;
    }

    tr069_cfg = calloc_a( sizeof( *tr069_cfg ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type ) +1 );

    if( NULL == tr069_cfg ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc tr069_config_parse '%s'\n", section_name );

        return;
    }

    tr069_cfg->cf_section.name = strcpy( name, section_name );
    tr069_cfg->cf_section.type = strcpy( type, section_type );
    tr069_cfg->cf_section.config = blob_memdup( data );

    vlist_add( vltree, &tr069_cfg->node, tr069_cfg->cf_section.name );
}

//=============================================================================
static void
cfparse_tr069_uci_to_blob(
    struct uci_section *s
)
//=============================================================================
{
    const struct uci_blob_param_list *uci_blob_list = NULL;

    blob_buf_init( &b, 0 );

    uci_blob_list = cfparse_tr069_find_blob_param_list( s->type );
    if( NULL == uci_blob_list ) {
        cfmanager_log_message( L_ERR, "No corresponding uci_blob_param_list was found according to %s\n",
            s->type );

        return;
    }
    uci_to_blob( &b, s, uci_blob_list );

    cfparse_tr069_add_blob_to_tree( b.head, s->e.name, s->type );

    blob_buf_free( &b );
}

//=============================================================================
int
cfparse_load_tr069(
 void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct vlist_tree *vltree = NULL;
    struct uci_package *package = NULL;
    int i = 0;

    check_set_defvalue( CHECK_TR069 );

    package = cfparse_init_package( "tr069" );
    if( !package ) {
        cfmanager_log_message( L_ERR, "load package tr069 failed\n" );
        return -1;
    }

    for( i = 1; i < __TR069_VLTREE_MAX; i++ ) {

        vltree = tr069_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_update( vltree );
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        cfparse_tr069_uci_to_blob( s );
    }

    for( i = 1; i < __TR069_VLTREE_MAX; i++ ) {

        vltree = tr069_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush( vltree );
    }

    apply_handle_all_reload_flag();

    return 0;
}

//=============================================================================
void
cfparse_tr069_init(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __TR069_VLTREE_MAX; i++ ) {

        vltree = tr069_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_init( vltree, avl_strcmp, cfparse_tr069_vltree_update );
        vltree->keep_old = true;
        vltree->no_delete = true;
    }
}

//=============================================================================
void
cfparse_tr069_deinit(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __TR069_VLTREE_MAX; i++ ) {

        vltree = tr069_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush_all( vltree );
    }
}

