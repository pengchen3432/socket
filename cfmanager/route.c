/****************************************************************************
*
* FILENAME:        route.c
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
#include "global.h"
#include "utils.h"
#include "config.h"
#include "sgrequest.h"
#include "cfmanager.h"
#include "cfparse.h"
#include "route.h"

//===========================
// Defines
//===========================

//===========================
// Typedefs
//===========================

//===========================
// Locals
//===========================
/* Variables */
extern struct blobmsg_policy static_route_ipv4_policy[__STATIC_ROUTE_IPV4_MAX];
extern struct blobmsg_policy static_route_ipv6_policy[__STATIC_ROUTE_IPV6_MAX];

/* Functions */
//=============================================================================
int
route_delete_ipv4_static_route_by_wan(
    char *wan
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    int should_commit = 0;

    vlist_for_each_element( &cm_static_route_ipv4_vltree, cm_cfg, node ) {
        struct blob_attr *tb[__STATIC_ROUTE_IPV4_MAX];
        char *id;
        char *wan_intf;
        char section_nm[BUF_LEN_64] = { 0 };

        blobmsg_parse( static_route_ipv4_policy,
            __STATIC_ROUTE_IPV4_MAX,
            tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        id = blobmsg_get_string( tb[STATIC_ROUTE_IPV4_ID] );
        snprintf( section_nm, sizeof(section_nm), "ipv4_static_route%s", id );
        wan_intf = blobmsg_get_string( tb[STATIC_ROUTE_IPV4_OUT_INTF] );
        if ( !strcmp(wan_intf, wan) ) {
            cfmanager_log_message( L_WARNING, "Delete ipv4 static route %s, because %s disabled!\n",
                section_nm, wan );
            config_del_named_section( CFMANAGER_CONFIG_NAME, "ipv4_static_route", section_nm );
            should_commit = 1;
        }
    }

    if ( should_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_STATIC_ROUTE_IPV4, false );
    }

    return 0;
}

//==============================================================================
int
route_parse_ipv4_static_route(
    struct blob_attr *attr
)
//==============================================================================
{
    struct blob_attr *new_tb[__STATIC_ROUTE_IPV4_MAX];
    struct blob_attr *old_tb[__STATIC_ROUTE_IPV4_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int i = 0;
    int same_cfg = 1;
    int ret = ERRCODE_SUCCESS;
    char section_nm[BUF_LEN_64] = { 0 };
    int action;

    blobmsg_parse( static_route_ipv4_policy,
        __STATIC_ROUTE_IPV4_MAX,
        new_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    if ( !new_tb[STATIC_ROUTE_IPV4_ACTION] ) {
        cfmanager_log_message( L_ERR,
            "Missing %s in json!\n", static_route_ipv4_policy[STATIC_ROUTE_IPV4_ACTION].name );
        return ERRCODE_PARAMETER_ERROR;
    }

    action = atoi( blobmsg_get_string(new_tb[STATIC_ROUTE_IPV4_ACTION]) );
    if ( action >= __STATIC_ROUTE_IPV4_ACTION_MAX ) {
        cfmanager_log_message( L_ERR, "Invalid action %d!\n", action );
        return ERRCODE_PARAMETER_ERROR;
    }

    if ( action != STATIC_ROUTE_IPV4_ACTION_ADD && !new_tb[STATIC_ROUTE_IPV4_ID] ) {
        cfmanager_log_message( L_ERR,
            "Missing %s in json!\n", static_route_ipv4_policy[STATIC_ROUTE_IPV4_ID].name );
        return ERRCODE_PARAMETER_ERROR;
    }
    else if ( action == STATIC_ROUTE_IPV4_ACTION_ADD ) {
        for ( i = 0; i < __STATIC_ROUTE_IPV4_MAX; i++ ) {
            if ( i != STATIC_ROUTE_IPV4_NEXTHOP && !new_tb[i] ) {
                cfmanager_log_message( L_ERR,
                    "Missing %s in json!\n", static_route_ipv4_policy[i].name );
                return ERRCODE_PARAMETER_ERROR;
            }
        }
    }

    snprintf( section_nm, sizeof(section_nm), "ipv4_static_route%s",
        blobmsg_get_string(new_tb[STATIC_ROUTE_IPV4_ID]) );
    cm_cfg = util_get_vltree_node( &cm_static_route_ipv4_vltree, VLTREE_CM_TREE, section_nm );
    if ( !cm_cfg && action == STATIC_ROUTE_IPV4_ACTION_ADD ) {
        cfmanager_log_message( L_WARNING, "NOT found ipv4 static route %s information in %s, create it...\n", 
            section_nm, CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "ipv4_static_route", section_nm );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section %s in %s\n", section_nm, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __STATIC_ROUTE_IPV4_MAX; i++ ) {
            if ( i != STATIC_ROUTE_IPV4_ACTION ) {
                snprintf( path, sizeof( path ), "%s.%s.%s",
                    CFMANAGER_CONFIG_NAME, section_nm, static_route_ipv4_policy[i].name );
                config_set_by_blob( new_tb[i], path, static_route_ipv4_policy[i].type );
            }
        }

        same_cfg = 0;
    }
    else if ( action == STATIC_ROUTE_IPV4_ACTION_ADD ) {
        cfmanager_log_message( L_WARNING, "Update ipv4 static route %s in %s\n", 
            section_nm, CFMANAGER_CONFIG_NAME );

        blobmsg_parse( static_route_ipv4_policy,
            __STATIC_ROUTE_IPV4_MAX,
            old_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __STATIC_ROUTE_IPV4_MAX; i++ ) {
            if ( !new_tb[i] || i == STATIC_ROUTE_IPV4_ACTION ) {
                continue;
            }

            if ( sgreq_compar_attr( new_tb[i], old_tb[i], static_route_ipv4_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, section_nm, static_route_ipv4_policy[i].name );
            config_set_by_blob( new_tb[i], path, static_route_ipv4_policy[i].type );

            same_cfg = 0;
        }
    }
    else if ( cm_cfg ) {
        cfmanager_log_message( L_WARNING, "Delete ipv4 static route %s!\n", section_nm );
        config_del_named_section( CFMANAGER_CONFIG_NAME, "ipv4_static_route", section_nm );
        same_cfg = 0;
    }
    else {
        cfmanager_log_message( L_ERR, "Try to delete ipv4 static route %s, but not found!\n", section_nm );
        ret = ERRCODE_PARAMETER_ERROR;
    }

    if ( same_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "The ipv4 static route %s config is same\n", section_nm );
    }

    return ret;
}

//==============================================================================
int
route_parse_ipv6_static_route(
    struct blob_attr *attr
)
//==============================================================================
{
    struct blob_attr *tb[__STATIC_ROUTE_IPV6_MAX];
    struct blob_attr *tb_cm[__STATIC_ROUTE_IPV6_MAX];
    struct cm_config *cm_cfg = NULL;
    char section_name[BUF_LEN_16] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    const char *value = NULL;
    int i = 0;
    int ret = 0;

    blobmsg_parse( static_route_ipv6_policy,
        __STATIC_ROUTE_IPV6_MAX,
        tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    if( !tb[STATIC_ROUTE_IPV6_ACTION] || !tb[STATIC_ROUTE_IPV6_ID] ) {
        cfmanager_log_message( L_ERR, "Missing required parameter 'action' or 'id' \n" );
        return ERRCODE_INTERNAL_ERROR;
    }

    snprintf( section_name, sizeof( section_name ), "%s%s",
        STATIC_ROUTEV6_NAME_PREFIX, blobmsg_get_string( tb[STATIC_ROUTE_IPV6_ID] ) );
    value = blobmsg_get_string( tb[STATIC_ROUTE_IPV6_ACTION] );
    if( 0 == strcmp( "1", value ) ) {
        ret = config_del_named_section( CFMANAGER_CONFIG_NAME, "ipv6_static_route", section_name );
        if( ret < 0) {
            cfmanager_log_message( L_ERR, "delete %s failed\n", section_name );
            return ERRCODE_INTERNAL_ERROR;
        }

        return ERRCODE_SUCCESS;
    }

    cm_cfg = util_get_vltree_node( &cm_static_route_ipv6_vltree, VLTREE_CM_TREE, section_name );
    if( !cm_cfg ) {
        ret = config_set_section( CFMANAGER_CONFIG_NAME, CM_STATIC_ROUTEV6_SECTION, section_name );
        if( ret != 0 ) {
            cfmanager_log_message( L_ERR, "create section '%s' failed\n", CM_STATIC_ROUTEV6_SECTION );
            return ERRCODE_INTERNAL_ERROR;
        }

        memset( tb_cm, 0, __STATIC_ROUTE_IPV6_MAX * sizeof(*tb_cm) );
    }
    else {
        blobmsg_parse( static_route_ipv6_policy,
            __STATIC_ROUTE_IPV6_MAX,
            tb_cm,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );
    }

    for( i = 0; i < __STATIC_ROUTE_IPV6_MAX; i++ ) {
        if( !tb[i] || STATIC_ROUTE_IPV6_ACTION == i ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], tb_cm[i], static_route_ipv6_policy[i].type ) ) {
            continue;
        }

        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, section_name, static_route_ipv6_policy[i].name );
        config_set_by_blob( tb[i], path, static_route_ipv6_policy[i].type );
    }

    return ERRCODE_SUCCESS;

}

/* EOF */

