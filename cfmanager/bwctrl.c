/****************************************************************************
* *
* * FILENAME:        $RCSfile: bwctrl.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/04/22
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
#include "global.h"
#include "track.h"
#include "apply.h"
#include "time.h"
#include "bwctrl.h"
#include "utils.h"
#include "cfparse.h"

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
enum {
    BWCTRL_HOOKER_RULE,

    __BWCTRL_HOOKER_MAX
};

enum {
    BWCTRL_RULE_ENABLED,
    BWCTRL_RULE_ID,
    BWCTRL_RULE_SSID_ID,
    BWCTRL_RULE_TYPE,
    BWCTRL_RULE_DEV,
    BWCTRL_RULE_URATE,
    BWCTRL_RULE_DRATE,

    __BWCTRL_RULE_MAX
};

enum {
    BWCTRL_MIN,
    BWCTRL_VLTREE_RULE,

    __BWCTRL_VLTREE_MAX
};
//=================
//  Globals
//=================
struct vlist_tree bwctrl_rule_vltree;

static const struct blobmsg_policy bwctrl_rule_policy[__BWCTRL_RULE_MAX] = {
    [BWCTRL_RULE_ENABLED] = { .name = "enabled", .type = BLOBMSG_TYPE_BOOL },
    [BWCTRL_RULE_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [BWCTRL_RULE_SSID_ID] = { .name = "ssid_id", .type = BLOBMSG_TYPE_STRING },
    [BWCTRL_RULE_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [BWCTRL_RULE_DEV] = { .name = "dev", .type = BLOBMSG_TYPE_ARRAY },
    [BWCTRL_RULE_URATE] = { .name = "urate", .type = BLOBMSG_TYPE_STRING },
    [BWCTRL_RULE_DRATE] = { .name = "drate", .type = BLOBMSG_TYPE_STRING },
};
//=================
//  Locals
//=================

/*
 * Private Functions
 */
static int
cfparse_bwctrl_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);
/*
 * Private Data
 */

static const struct uci_blob_param_list bwctrl_rule_policy_list = {
    .n_params = __BWCTRL_RULE_MAX,
    .params = bwctrl_rule_policy,
};

static const struct cm_hooker_policy cm_hp[__BWCTRL_HOOKER_MAX] = {
    [BWCTRL_HOOKER_RULE] = { .cb = cfparse_bwctrl_rule_hooker },
};

static const struct cm_vltree_info bwctrl_vltree_info[__BWCTRL_VLTREE_MAX] = {
    [BWCTRL_VLTREE_RULE] = {
        .key = "bwctrl-rule",
        .vltree = &bwctrl_rule_vltree,
        .policy_list = &bwctrl_rule_policy_list,
        .hooker = BWCTRL_HOOKER_RULE
    }
};

static struct blob_buf b;
//=================
//  Functions
//=================

//=============================================================================
static struct vlist_tree*
cfparse_bwctrl_find_tree(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __BWCTRL_VLTREE_MAX; i++ ) {

        if( !bwctrl_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( bwctrl_vltree_info[i].key, section_type ) ) {
            return bwctrl_vltree_info[i].vltree;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",  section_type );

    return NULL;
}

//=============================================================================
static const struct uci_blob_param_list*
cfparse_bwctrl_find_blob_param_list(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __BWCTRL_VLTREE_MAX; i++ ) {
        if( !bwctrl_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( bwctrl_vltree_info[i].key, section_type ) ) {
            return bwctrl_vltree_info[i].policy_list;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n", section_type );

    return NULL;
}

//=============================================================================
static void
cfparse_bwctrl_node_free(
    struct bwctrl_config_parse *bwctrl_cfpar
)
//=============================================================================
{
    SAFE_FREE( bwctrl_cfpar->cf_section.config );
    SAFE_FREE( bwctrl_cfpar );
}

//=============================================================================
static void
cfparse_bwctrl_tree_free(
    struct vlist_tree *vltree,
    struct bwctrl_config_parse *bwctrl_cfpar
)
//=============================================================================
{
    avl_delete( &vltree->avl, &bwctrl_cfpar->node.avl );

    cfparse_bwctrl_node_free( bwctrl_cfpar );
}

//=============================================================================
static int
cfparse_bwctrl_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch ( index ) {
        case BWCTRL_RULE_ENABLED:
            break;
        case BWCTRL_RULE_ID:
            break;
        case BWCTRL_RULE_SSID_ID:
            break;
        case BWCTRL_RULE_TYPE:
            break;
        case BWCTRL_RULE_DEV:
            break;
        case BWCTRL_RULE_URATE:
            break;
        case BWCTRL_RULE_DRATE:
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
cfparse_bwctrl_update_cfg(
    const struct blobmsg_policy *policy,
    struct bwctrl_config_parse *bwctrl_cfpar_old,
    struct bwctrl_config_parse *bwctrl_cfpar_new,
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

    struct blob_attr *config_old = bwctrl_cfpar_old->cf_section.config;
    struct blob_attr *config_new = bwctrl_cfpar_new->cf_section.config;

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
        apply_set_reload_flag( CONFIG_TRACK_BWCTRL );
    }
}

//=============================================================================
static void
cfparse_bwctrl_call_update_func(
    const char *section_type,
    struct bwctrl_config_parse *bwctrl_cfpar_old,
    struct bwctrl_config_parse *bwctrl_cfpar_new
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type || NULL == bwctrl_cfpar_old || NULL == bwctrl_cfpar_new ) {
        return;
    }

    for( i = 1; i < __BWCTRL_VLTREE_MAX; i++ ) {
        if( 0 == strcmp( bwctrl_vltree_info[i].key, section_type ) ) {
            cfparse_bwctrl_update_cfg( bwctrl_vltree_info[i].policy_list->params,
                bwctrl_cfpar_old,
                bwctrl_cfpar_new,
                bwctrl_vltree_info[i].policy_list->n_params,
                bwctrl_vltree_info[i].hooker );
        }
    }
}

//=============================================================================
static void
cfparse_bwctrl_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct bwctrl_config_parse *bwctrl_cfpar_old = NULL;
    struct bwctrl_config_parse *bwctrl_cfpar_new = NULL;

    if ( node_old ) {
        bwctrl_cfpar_old =
            container_of(node_old, struct bwctrl_config_parse, node);
    }

    if ( node_new ) {
        bwctrl_cfpar_new =
            container_of(node_new, struct bwctrl_config_parse, node);
    }

    if ( bwctrl_cfpar_old && bwctrl_cfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "update bwctrl section type '%s', name '%s'\n",
            bwctrl_cfpar_old->cf_section.type, bwctrl_cfpar_old->cf_section.name );

        if ( blob_attr_equal( bwctrl_cfpar_new->cf_section.config,
                bwctrl_cfpar_old->cf_section.config ) ) {
            cfparse_bwctrl_node_free( bwctrl_cfpar_new );
            return;
        }

        cfparse_bwctrl_call_update_func( bwctrl_cfpar_old->cf_section.type,
            bwctrl_cfpar_old, bwctrl_cfpar_new );

        SAFE_FREE( bwctrl_cfpar_old->cf_section.config );
        bwctrl_cfpar_old->cf_section.config = blob_memdup( bwctrl_cfpar_new->cf_section.config );
        strncpy( bwctrl_cfpar_old->id, bwctrl_cfpar_new->id, sizeof( bwctrl_cfpar_old->id )-1 );
        cfparse_bwctrl_node_free( bwctrl_cfpar_new );
        apply_set_reload_flag( CONFIG_TRACK_BWCTRL );
    }
    else if ( bwctrl_cfpar_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete bwctrl section type '%s', name '%s'\n",
            bwctrl_cfpar_old->cf_section.type, bwctrl_cfpar_old->cf_section.name );

        vltree = cfparse_bwctrl_find_tree( bwctrl_cfpar_old->cf_section.type );
        if( NULL == vltree ) {
            return;
        }
        cfparse_bwctrl_tree_free( vltree, bwctrl_cfpar_old );
        apply_set_reload_flag( CONFIG_TRACK_BWCTRL );
    }
    else if ( bwctrl_cfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "New bwctrl section type '%s', name '%s'\n",
            bwctrl_cfpar_new->cf_section.type, bwctrl_cfpar_new->cf_section.name );

        apply_set_reload_flag( CONFIG_TRACK_BWCTRL );
    }
}

//=============================================================================
static void
cfparse_bwctrl_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct bwctrl_config_parse *bwctrl_cfpar = NULL;
    struct vlist_tree *vltree = NULL;
    struct blob_attr *tb[__BWCTRL_RULE_MAX];
    char *name;
    char *type;

    vltree = cfparse_bwctrl_find_tree( section_type );
    if( NULL == vltree ) {
        cfmanager_log_message( L_DEBUG, "No corresponding binary tree was found according to %s\n",
            section_type );
        return;
    }

    bwctrl_cfpar = calloc_a( sizeof( *bwctrl_cfpar ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type ) +1 );

    if ( !bwctrl_cfpar ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc bwctrl_config_parse '%s'\n", section_name );

        return;
    }

    blobmsg_parse( bwctrl_rule_policy,
        __BWCTRL_RULE_MAX,
        tb,
        blob_data( data ),
        blob_len( data ) );

    if( tb[BWCTRL_RULE_ID] ) {
        strncpy( bwctrl_cfpar->id, blobmsg_get_string( tb[BWCTRL_RULE_ID] ),
            sizeof( bwctrl_cfpar->id ) -1 );
    }

    bwctrl_cfpar->cf_section.name = strcpy( name, section_name );
    bwctrl_cfpar->cf_section.type = strcpy( type, section_type );
    bwctrl_cfpar->cf_section.config = blob_memdup( data );

    vlist_add( vltree, &bwctrl_cfpar->node, bwctrl_cfpar->cf_section.name );
}

//=============================================================================
static void
cfparse_bwctrl_uci_to_blob(
    struct uci_section *s
)
//=============================================================================
{
    const struct uci_blob_param_list* uci_blob_list = NULL;

    blob_buf_init( &b, 0 );

    uci_blob_list = cfparse_bwctrl_find_blob_param_list( s->type );
    if( NULL == uci_blob_list ) {
        cfmanager_log_message( L_DEBUG,
            "No corresponding uci_blob_param_list was found according to %s\n",
            s->type );

        return;
    }
    uci_to_blob( &b, s, uci_blob_list );

    cfparse_bwctrl_add_blob_to_tree( b.head, s->e.name, s->type );

    blob_buf_free( &b );
}

//=============================================================================
int
cfparse_load_bwctrl(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct vlist_tree *vltree = NULL;
    struct uci_package *package = NULL;
    int i = 0;

    package = cfparse_init_package( "bwctrl" );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load bwctrl package failed\n" );
        return -1;
    }

    for( i = 1; i < __BWCTRL_VLTREE_MAX; i++ ) {

        vltree = bwctrl_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_update( vltree );
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        cfparse_bwctrl_uci_to_blob( s );
    }

    for( i = 1; i < __BWCTRL_VLTREE_MAX; i++ ) {
        vltree = bwctrl_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush( vltree );
    }

    if( apply_get_reload_flag( CONFIG_TRACK_BWCTRL ) ) {
        apply_add( "bwctrl" );
        apply_flush_reload_flag( CONFIG_TRACK_BWCTRL );
        apply_timer_start();
    }

    return 0;
}

//=============================================================================
void
cfparse_bwctrl_init(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __BWCTRL_VLTREE_MAX; i++ ) {

        vltree = bwctrl_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_init( vltree, avl_strcmp, cfparse_bwctrl_vltree_update );
        vltree->keep_old = true;
        vltree->no_delete = true;
    }
}

//=============================================================================
void
cfparse_bwctrl_deinit(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __BWCTRL_VLTREE_MAX; i++ ) {

        vltree = bwctrl_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush_all( vltree );
    }
}

