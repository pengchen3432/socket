/****************************************************************************
* *
* * FILENAME:        $RCSfile: gsportalcfg.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/12/06
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
#include "apply.h"
#include "time.h"
#include "initd.h"
#include "utils.h"
#include "cfparse.h"
#include "config.h"
#include "cfmanager.h"
#include "gsportalcfg.h"


//=================
//  Defines
//=================

//=================
//  Typedefs
//=================

//=================
//  Globals
//=================
struct vlist_tree gsportalcfg_wifi_portal_vltree;
struct vlist_tree gsportalcfg_portal_policy_vltree;

const struct uci_blob_param_list gsportalcfg_wifi_portal_list = {
    .n_params = __WIFI_PORTAL_MAX,
    .params = gsportalcfg_wifi_portal_policy,
};

const struct uci_blob_param_list gsportalcfg_portal_policy_list = {
    .n_params = __PORTAL_POLICY_MAX,
    .params = gsportalcfg_portal_policy,
};

const struct blobmsg_policy gsportalcfg_wifi_portal_policy[__WIFI_PORTAL_MAX] = {
    [WIFI_PORTAL_ENABLE] = { .name = "portal_enable", .type = BLOBMSG_TYPE_STRING },
    [WIFI_PORTAL_POLICY] = { .name = "portal_policy", .type = BLOBMSG_TYPE_STRING },
    [WIFI_PORTAL_SSID] = { .name = "ssid_name", .type = BLOBMSG_TYPE_STRING },
    [WIFI_PORTAL_LAN_INTERFACE] = { .name = "lan_interface", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy gsportalcfg_portal_policy[__PORTAL_POLICY_MAX] = {
    [PORTAL_POLICY_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_AUTH_TYPE] = { .name = "auth_type", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_EXPIRATION] = { .name = "expiration", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_IDLE_TIMEOUT] = { .name = "idle_timeout", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_PAGE_PATH] = { .name = "portal_page_path", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_LANDING_PAGE] = { .name = "landing_page", .type = BLOBMSG_TYPE_STRING },
    /* 'external_page' is actually 'landing_page_url', This is a mistake left by history. */
    [PORTAL_POLICY_LANDING_PAGE_URL] = { .name = "external_page", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_ENABLE_HTTPS] = { .name = "enable_https", .type = BLOBMSG_TYPE_BOOL },
    [PORTAL_POLICY_ENABLE_HTTPS_REDIRECT] = { .name = "enable_https_redirect", .type = BLOBMSG_TYPE_BOOL },
    [PORTAL_POLICY_ENABLE_FAIL_SAFE] = { .name = "enable_failsafe", .type = BLOBMSG_TYPE_BOOL },
    [PORTAL_POLICY_ENABLE_DALY_LIMIT] = { .name = "enable_daily_limit", .type = BLOBMSG_TYPE_BOOL },

    [PORTAL_POLICY_RADIUS_SERVER] = { .name = "radius_server", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_PORT] = { .name = "radius_port", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_SECRET] = { .name = "radius_secret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_METHOD] = { .name = "radius_method", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_DYNAMIC_VLAN] = { .name = "radius_dynamic_vlan", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_ACCT_SERVER] = { .name = "radius_acct_server", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_ACCT_PORT] = { .name = "radius_acct_port", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_ACCT_SECRET] = { .name = "radius_acct_secret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_ACCT_UPDATE_INTERVAL] = { .name = "acct_update_interval", .type = BLOBMSG_TYPE_STRING },
    /* 'ly_vc_name' is actually 'rad_nas_id', This is a mistake left by history(linkyfi). */
    [PORTAL_POLICY_RADIUS_NAS_ID] = { .name = "rad_nas_id", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_ENABLE_THIRD] = { .name = "enable_third", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_ENABLE_BYTE_LIMIT] = { .name = "enable_byte_limit", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_WECHAT_SHOP_ID] = { .name = "ShopId", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_WECHAT_APP_ID] = { .name = "AppId", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_WECHAT_SECRET_KEY] = { .name = "SecretKey", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_FB_APP_ID] = { .name = "fb_app_id", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_FB_APP_SECRET] = { .name = "fb_app_secret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_FB_OAUTH_DOMAIN] = { .name = "fb_app_oauth_domain", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_FB_OAUTH_PORT] = { .name = "fb_app_oauth_port", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_FB_REAUTH] = { .name = "fb_re_auth", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_GG_CLIENT_ID] = { .name = "gg_clientid", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_GG_CLIENT_SECRET] = { .name = "gg_clientsecret", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_TWITER_ID] = { .name = "twitter_id", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_TWITER_CONSUMER_KEY] = { .name = "twitter_consumer_key", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_TWITER_CONSUMER_SECRET] = { .name = "twitter_consumer_secret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_TWITER_FORCE_TO_FOLLOW] = { .name = "twitter_force_to_follow", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_SIMPLE_PASSWD] = { .name = "simple_passwd", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_OM_SPLASH_PAGE_URL] = { .name = "om_splash_page_url", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_OM_SPLASH_PAGE_SECRET] = { .name = "om_splash_page_secret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_OM_SPLASH_PAGE_AUTH_URL] = { .name = "om_splash_page_auth_url", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_OM_SPLASH_PAGE_AUTH_SECRET] = { .name = "om_splash_page_auth_secret", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_PRE_AUTH] = { .name = "pre_auth", .type = BLOBMSG_TYPE_ARRAY},
    [PORTAL_POLICY_POST_AUTH] = { .name = "post_auth", .type = BLOBMSG_TYPE_ARRAY },
};

const struct cm_vltree_info gsportalcfg_vltree_info[__GSPORTALCFG_VLTREE_MAX] = {
    [GSPORTALCFG_VLTREE_WIFI_PORTAL] = {
        .key = "wifi_portal",
        .vltree = &gsportalcfg_wifi_portal_vltree,
        .policy_list = &gsportalcfg_wifi_portal_list,
        .hooker = GSPORTALCFG_HOOKER_WIFI_PORTAL
    },
    [GSPORTALCFG_VLTREE_PORTAL_POLICY] = {
        .key = "portal_policy",
        .vltree = &gsportalcfg_portal_policy_vltree,
        .policy_list = &gsportalcfg_portal_policy_list,
        .hooker = GSPORTALCFG_HOOKER_PORTAL_POLICY
    }
};

enum {
    __GS_MIN = __OPTION_FLAG_MAX,

    GS_PORTALCFG,
    GS_PORTALCGI,

    __GS_MAX
};

//=================
//  Locals
//=================

/*
 * Private Functions
 */
static int
cfparse_gsportalcfg_wifi_portal_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_gsportalcfg_portal_policy_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);
/*
 * Private Data
 */
static const struct cm_hooker_policy gsportalcfg_hp[__GSPORTALCFG_HOOKER_MAX] = {
    [GSPORTALCFG_HOOKER_WIFI_PORTAL] = { .cb = cfparse_gsportalcfg_wifi_portal_hooker },
    [GSPORTALCFG_HOOKER_PORTAL_POLICY] = { .cb = cfparse_gsportalcfg_portal_policy_hooker },
};

static struct blob_buf b;

//=================
//  Functions
//=================

//=============================================================================
static int
cfparse_gsportalcfg_wifi_portal_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;
    int action = extend->action;

    switch( action ) {
        case VLTREE_ACTION_DEL:
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case WIFI_PORTAL_ENABLE:
                case WIFI_PORTAL_SSID:
                    rc |= BIT( GS_PORTALCGI );
                    rc |= BIT( GS_PORTALCFG );
                    break;

                default:
                    rc |= BIT( GS_PORTALCFG );
                    break;
            }
        default:
            break;
    }

    return rc;
}

//=============================================================================
static int
cfparse_gsportalcfg_portal_policy_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;
    rc |= BIT( GS_PORTALCFG );

    return rc;
}

//=============================================================================
static void
cfparse_gsportalcfg_node_free(
    struct gsportalcfg_config_parse *gscfgpar
)
//=============================================================================
{
    SAFE_FREE( gscfgpar->cf_section.config );
    SAFE_FREE( gscfgpar );
}

//=============================================================================
static void
cfparse_gsportalcfg_tree_free(
    struct vlist_tree *vltree,
    struct gsportalcfg_config_parse *gscfgpar
)
//=============================================================================
{
    avl_delete( &vltree->avl, &gscfgpar->node.avl );

    cfparse_gsportalcfg_node_free( gscfgpar );
}

//=============================================================================
static int
cfparse_gsportalcfg_find_hooker(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return -1;
    }

    for( i = 1; i < __GSPORTALCFG_VLTREE_MAX; i++ ) {

        if( !gsportalcfg_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( gsportalcfg_vltree_info[i].key, section_type ) ) {

            return gsportalcfg_vltree_info[i].hooker;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n", section_type );
    return -1;
}

//=============================================================================
static const struct uci_blob_param_list*
cfparse_gsportalcfg_find_blob_param_list(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __GSPORTALCFG_VLTREE_MAX; i++ ) {

        if( !gsportalcfg_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( gsportalcfg_vltree_info[i].key, section_type ) ) {
            return gsportalcfg_vltree_info[i].policy_list;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static struct vlist_tree*
cfparse_gsportalcfg_find_tree(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __GSPORTALCFG_VLTREE_MAX; i++ ) {

        if( !gsportalcfg_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( gsportalcfg_vltree_info[i].key, section_type ) ) {
            return gsportalcfg_vltree_info[i].vltree;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static void
cfparse_gsportalcfg_config_change_effective(
    int option
)
//=============================================================================
{
    if ( option & BIT( GS_PORTALCFG ) ) {
        apply_set_reload_flag( CONFIG_TRACK_GSPORTAL );
    }
    if ( option & BIT( GS_PORTALCGI ) ) {
        apply_set_reload_flag( CONFIG_TRACK_PORTALCGI );
    }
}

//=============================================================================
void
cfparse_gsportalcfg_update_cfg(
    const struct blobmsg_policy *policy,
    struct gsportalcfg_config_parse *gscfgpar_old,
    struct gsportalcfg_config_parse *gscfgpar_new,
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

    struct blob_attr *config_old = gscfgpar_old->cf_section.config;
    struct blob_attr *config_new = gscfgpar_new->cf_section.config;

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
    extend.action = VLTREE_ACTION_UPDATE;

    for( i = 0; i < policy_size; i++ ) {
        if( !blob_attr_equal( tb_new[i], tb_old[i] ) ||
            extend.need_set_option & BIT(i) ) {

            if ( gsportalcfg_hp[hooker].cb ) {
                option |= gsportalcfg_hp[hooker].cb( tb_new, i, &extend );
            }
        }
    }

    cfparse_gsportalcfg_config_change_effective( option );
}

//=============================================================================
static void
cfparse_gsportalcfg_call_update_func(
    const char *section_type,
    struct gsportalcfg_config_parse *gscfgpar_old,
    struct gsportalcfg_config_parse *gscfgpar_new
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type
        || NULL == gscfgpar_old
        || NULL == gscfgpar_new ) {
        return;
    }

    for( i = 1; i < __GSPORTALCFG_VLTREE_MAX; i++ ) {

        if( 0 == strcmp( gsportalcfg_vltree_info[i].key, section_type ) ) {

            cfparse_gsportalcfg_update_cfg( gsportalcfg_vltree_info[i].policy_list->params,
                gscfgpar_old,
                gscfgpar_new,
                gsportalcfg_vltree_info[i].policy_list->n_params,
                gsportalcfg_vltree_info[i].hooker );
        }
    }
}

//=============================================================================
static int
cfparse_gsportalcfg_vltree_handle_add_or_del(
    struct gsportalcfg_config_parse *gscfgpar,
    int action
)
//=============================================================================
{
    int policy_size = 0;
    int hooker = 0;
    int i = 0;
    int option = 0;
    const struct blobmsg_policy *policy = NULL;
    const struct uci_blob_param_list *blob_policy = NULL;
    struct cm_vltree_extend extend;

    if( !gscfgpar ) {
        return -1;
    }

    hooker = cfparse_gsportalcfg_find_hooker( gscfgpar->cf_section.type );
    if( 0 > hooker ) {
        cfmanager_log_message( L_WARNING,
            "There is no corresponding callback processing for %s\n",
            gscfgpar->cf_section.type );

        return -1;
    }

    blob_policy = cfparse_gsportalcfg_find_blob_param_list( gscfgpar->cf_section.type );
    if( NULL == blob_policy ) {
        cfmanager_log_message( L_WARNING,
            "There is no corresponding blob param list for %s\n",
            gscfgpar->cf_section.type );

        return -1;
    }

    policy = blob_policy->params;
    policy_size = blob_policy->n_params;

    struct blob_attr *tb[policy_size];

    blobmsg_parse( policy,
        policy_size,
        tb,
        blob_data( gscfgpar->cf_section.config ),
        blob_len( gscfgpar->cf_section.config ) );

    memset( &extend, 0, sizeof( extend ) );
    extend.action = action;

    for( i = 0; i < policy_size; i++ ) {

        //If the highest position is 1, all items will be skipped after that
        if( extend.set_compl_option & BIT( BIT_MAX ) ) {
            break;
        }
        else if( extend.set_compl_option & BIT( i ) ) {
            continue;
        }

        if ( gsportalcfg_hp[hooker].cb ) {
            option |= gsportalcfg_hp[hooker].cb( tb, i, &extend );
        }
    }

    cfparse_gsportalcfg_config_change_effective( option );

    return option;
}


//=============================================================================
static void
cfparse_gsportalcfg_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct gsportalcfg_config_parse *gscfgpar_old = NULL;
    struct gsportalcfg_config_parse *gscfgpar_new = NULL;
    int ret = 0;

    if ( node_old ) {
        gscfgpar_old =
            container_of(node_old, struct gsportalcfg_config_parse, node);
    }

    if ( node_new ) {
        gscfgpar_new =
            container_of(node_new, struct gsportalcfg_config_parse, node);
    }

    if ( gscfgpar_old && gscfgpar_new ) {
        cfmanager_log_message( L_WARNING,
            "update gsportalcfg section '%s'\n", gscfgpar_old->cf_section.name );

        if ( blob_attr_equal( gscfgpar_new->cf_section.config,
                gscfgpar_old->cf_section.config ) ) {
            cfparse_gsportalcfg_node_free( gscfgpar_new );
            return;
        }

        cfparse_gsportalcfg_call_update_func(
            gscfgpar_old->cf_section.type, gscfgpar_old, gscfgpar_new );

        SAFE_FREE( gscfgpar_old->cf_section.config );
        gscfgpar_old->cf_section.config = blob_memdup( gscfgpar_new->cf_section.config );
        cfparse_gsportalcfg_node_free( gscfgpar_new );
    }
    else if ( gscfgpar_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete gsportalcfg section '%s'\n", gscfgpar_old->cf_section.name );

        vltree = cfparse_gsportalcfg_find_tree( gscfgpar_old->cf_section.type );
        if( NULL == vltree ) {
            cfmanager_log_message( L_DEBUG,
                "no tree corresponding to %s was found\n",
                    gscfgpar_old->cf_section.type );

            return;
        }

        ret = cfparse_gsportalcfg_vltree_handle_add_or_del( gscfgpar_old, VLTREE_ACTION_DEL );
        if( ret < 0 ) {
            apply_set_reload_flag( CONFIG_TRACK_GSPORTAL );
        }

        cfparse_gsportalcfg_tree_free( vltree, gscfgpar_old );
    }
    else if ( gscfgpar_new ) {
        cfmanager_log_message( L_WARNING,
            "New gsportalcfg section '%s'\n", gscfgpar_new->cf_section.name );

        ret = cfparse_gsportalcfg_vltree_handle_add_or_del( gscfgpar_new, VLTREE_ACTION_ADD );
        if( ret < 0 ) {
            apply_set_reload_flag( CONFIG_TRACK_GSPORTAL );
        }
    }
}

//=============================================================================
static void
cfparse_gsportalcfg_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct gsportalcfg_config_parse *gscfgpar = NULL;
    struct vlist_tree *vltree = NULL;
    char *name;
    char *type;

    vltree = cfparse_gsportalcfg_find_tree( section_type );
    if( NULL == vltree ) {
        cfmanager_log_message( L_DEBUG,
            "No corresponding binary tree was found according to %s\n",
            section_type );

        return;
    }

    gscfgpar = calloc_a( sizeof( *gscfgpar ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type ) +1 );

    if ( !gscfgpar ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc gsportalcfg_config_parse '%s'\n", section_name );

        return;
    }

    gscfgpar->cf_section.name = strcpy( name, section_name );
    gscfgpar->cf_section.type = strcpy( type, section_type );
    gscfgpar->cf_section.config = blob_memdup( data );

    vlist_add( vltree, &gscfgpar->node, gscfgpar->cf_section.name );
}

//=============================================================================
static void
cfparse_gsportalcfg_uci_to_blob(
    struct uci_section *s
)
//=============================================================================
{
    const struct uci_blob_param_list* uci_blob_list = NULL;

    blob_buf_init( &b, 0 );

    uci_blob_list = cfparse_gsportalcfg_find_blob_param_list( s->type );
    if( NULL == uci_blob_list ) {
        cfmanager_log_message( L_DEBUG,
            "No corresponding uci_blob_param_list was found according to %s\n",
            s->type );

        return;
    }
    uci_to_blob( &b, s, uci_blob_list );

    cfparse_gsportalcfg_add_blob_to_tree( b.head, s->e.name, s->type );

    blob_buf_free( &b );
}

//=============================================================================
int
cfparse_load_gsportalcfg(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct uci_package *package = NULL;
    struct vlist_tree *vltree = NULL;
    int i = 0;

    package = cfparse_init_package( "gsportalcfg" );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load gsportalcfg package failed\n" );
        return -1;
    }

    for( i = 1; i < __GSPORTALCFG_VLTREE_MAX; i++ ) {
        vltree = gsportalcfg_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_update( vltree );
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        cfparse_gsportalcfg_uci_to_blob( s );
    }

    for( i = 1; i < __GSPORTALCFG_VLTREE_MAX; i++ ) {

        vltree = gsportalcfg_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush( vltree );
    }

    if( apply_get_reload_flag( CONFIG_TRACK_GSPORTAL ) ) {
        apply_add( "gsportalcfg" );
        apply_flush_reload_flag( CONFIG_TRACK_GSPORTAL );
        apply_timer_start();
    }

    if( apply_get_reload_flag( CONFIG_TRACK_PORTALCGI ) ) {
        apply_add( "portalfcgi" );
        apply_flush_reload_flag( CONFIG_TRACK_PORTALCGI );
        apply_timer_start();
    }

    return 0;
}


//=============================================================================
void
cfparse_gsportalcfg_init(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __GSPORTALCFG_VLTREE_MAX; i++ ) {

        vltree = gsportalcfg_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_init( vltree, avl_strcmp, cfparse_gsportalcfg_vltree_update );
        vltree->keep_old = true;
        vltree->no_delete = true;
    }
}

//=============================================================================
void
cfparse_gsportalcfg_deinit(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __GSPORTALCFG_VLTREE_MAX; i++ ) {

        vltree = gsportalcfg_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush_all( vltree );
    }
}