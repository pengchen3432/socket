/****************************************************************************
*
* FILENAME:        ddns.c
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
#include "track.h"
#include "time.h"
#include "apply.h"
#include "ddns.h"

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
extern const struct blobmsg_policy ddns_policy[__DDNS_MAX];

/* Functions */
//=============================================================================
int
ddns_delete_by_wan(
    char *wan
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    int should_commit = 0;

    vlist_for_each_element( &cm_ddns_vltree, cm_cfg, node ) {
        struct blob_attr *tb[__DDNS_MAX];
        char *id;
        char *wan_intf;
        char section_nm[BUF_LEN_64] = { 0 };

        blobmsg_parse( ddns_policy,
            __DDNS_MAX,
            tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        id = blobmsg_get_string( tb[DDNS_ID] );
        snprintf( section_nm, sizeof(section_nm), "ddns%s", id );
        wan_intf = blobmsg_get_string( tb[DDNS_INTF] );
        if ( !strcmp(wan_intf, wan) ) {
            cfmanager_log_message( L_WARNING, "Delete ddns %s, because %s disabled!\n",
                section_nm, wan );
            config_del_named_section( CFMANAGER_CONFIG_NAME, "ddns", section_nm );
            should_commit = 1;
        }
    }

    if ( should_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_DDNS, false );
    }

    return 0;
}

//=============================================================================
int
ddns_parse(
    struct blob_attr *attr
)
//=============================================================================
{
    struct blob_attr *new_tb[__DDNS_MAX];
    struct blob_attr *old_tb[__DDNS_MAX];
    int i;
    int ret = ERRCODE_SUCCESS;
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int same_cfg = 1;
    char section_nm[BUF_LEN_64] = { 0 };
    int action;

    blobmsg_parse( ddns_policy,
        __DDNS_MAX,
        new_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    if ( !new_tb[DDNS_ACTION] ) {
        cfmanager_log_message( L_ERR,
            "Missing %s in json!\n", ddns_policy[DDNS_ACTION].name );
        return ERRCODE_PARAMETER_ERROR;
    }

    action = atoi( blobmsg_get_string(new_tb[DDNS_ACTION]) );
    if ( action >= __DDNS_ACTION_MAX ) {
        cfmanager_log_message( L_ERR, "Invalid action %d!\n", action );
        return ERRCODE_PARAMETER_ERROR;
    }

    if ( action != DDNS_ACTION_ADD && !new_tb[DDNS_ID] ) {
        cfmanager_log_message( L_ERR,
            "Missing %s in json!\n", ddns_policy[DDNS_ID].name );
        return ERRCODE_PARAMETER_ERROR;
    }
    else if ( action == DDNS_ACTION_ADD ) {
        for ( i = 0; i < __DDNS_MAX; i++ ) {
            if ( i != DDNS_STATUS && !new_tb[i] ) {
                cfmanager_log_message( L_ERR,
                    "Missing %s in json!\n", ddns_policy[i].name );
                return ERRCODE_PARAMETER_ERROR;
            }
        }
    }

    snprintf( section_nm, sizeof(section_nm), "ddns%s", blobmsg_get_string( new_tb[DDNS_ID] ) );
    cm_cfg = util_get_vltree_node( &cm_ddns_vltree, VLTREE_CM_TREE, section_nm );
    if ( !cm_cfg && action == DDNS_ACTION_ADD ) {
        cfmanager_log_message( L_WARNING, "NOT found ddns %s information in %s, create it...\n", 
            section_nm, CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "ddns", section_nm );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section %s in %s\n", section_nm, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __DDNS_MAX; i++ ) {
            if ( i != DDNS_ACTION && i != DDNS_STATUS ) {
                snprintf( path, sizeof( path ), "%s.%s.%s",
                    CFMANAGER_CONFIG_NAME, section_nm, ddns_policy[i].name );
                config_set_by_blob( new_tb[i], path, ddns_policy[i].type );
            }
            else if ( i == DDNS_STATUS ) {
                // Set disable when add a new ddns config.
                snprintf( path, sizeof(path), "%s.%s.status",
                    CFMANAGER_CONFIG_NAME, section_nm );
                config_uci_set( path, "0", 0 );
            }
        }

        same_cfg = 0;
    }
    else if ( cm_cfg ) {
        if ( action == DDNS_ACTION_ENABLE ) {
            cfmanager_log_message( L_WARNING,
                "Enable ddns %s, and disable others!\n", section_nm );
            snprintf( path, sizeof( path ), "%s.%s.status",
                CFMANAGER_CONFIG_NAME, section_nm );
            config_uci_set( path, "1", 0 );

            // Disable other services.
            vlist_for_each_element( &cm_ddns_vltree, cm_cfg, node ) {
                if ( strcmp( section_nm, cm_cfg->cf_section.name ) ) {
                    cfmanager_log_message( L_WARNING, "Disable ddns %s!\n",
                        cm_cfg->cf_section.name );
                    snprintf( path, sizeof( path ), "%s.%s.status",
                        CFMANAGER_CONFIG_NAME, cm_cfg->cf_section.name  );
                    config_uci_set( path, "0", 0 );
                }
            }
        }
        else if ( action == DDNS_ACTION_DISABLE ) {
            cfmanager_log_message( L_WARNING, "Disable ddns %s!\n", section_nm );
            snprintf( path, sizeof( path ), "%s.%s.status",
                CFMANAGER_CONFIG_NAME, section_nm );
            config_uci_set( path, "0", 0 );
        }
        else if ( action == DDNS_ACTION_DELETE ) {
            cfmanager_log_message( L_WARNING, "Delete ddns %s!\n", section_nm );
            config_del_named_section( CFMANAGER_CONFIG_NAME, "ddns", section_nm );
        }
        else if ( action == DDNS_ACTION_ADD ) {
            cfmanager_log_message( L_WARNING, "Update ddns %s!\n", section_nm );
            
            blobmsg_parse( ddns_policy,
                __DDNS_MAX,
                old_tb,
                blob_data( cm_cfg->cf_section.config ),
                blob_len( cm_cfg->cf_section.config  ) );
            
            for ( i = 0; i < __DDNS_MAX; i++ ) {
                // When edit ddns, status won't change.
                if ( !new_tb[i] || i == DDNS_ACTION || i == DDNS_STATUS ) {
                    continue;
                }

                if ( sgreq_compar_attr( new_tb[i], old_tb[i], ddns_policy[i].type ) ) {
                    continue;
                }

                snprintf( path, sizeof( path ), "%s.%s.%s",
                    CFMANAGER_CONFIG_NAME, section_nm, ddns_policy[i].name );
                config_set_by_blob( new_tb[i], path, ddns_policy[i].type );
            }
        }
        else {
            cfmanager_log_message( L_WARNING, "Meaningless action %d for ddns %s !\n",
                action, section_nm );
        }
    }
    else {
        cfmanager_log_message( L_WARNING, "Invalid action, ddns %s, action %d!\n",
            section_nm, action );
        ret = ERRCODE_PARAMETER_ERROR;
    }

    if ( same_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "The ddns %s config is same\n", section_nm );
    }

    return ret;
}


/* EOF */

