/****************************************************************************
*
* FILENAME:        vpn.c
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
#include "vpn.h"
#include "track.h"
#include "apply.h"
#include "time.h"

#include "gs_utils.h"
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
extern const struct blobmsg_policy vpn_client_policy[__VPN_CLIENT_MAX];
extern const struct blobmsg_policy vpn_split_policy[__VPN_SPLIT_MAX];
extern const struct blobmsg_policy vpn_split_dev_attrs_policy[__VPN_SPLIT_DEV_ATTR_MAX];

static uint64_t client_required_paras[__VPN_CLIENT_TYPE_MAX] = {
    [VPN_CLIENT_TYPE_L2TP] = (uint64_t)1 << VPN_CLIENT_ID |
                             (uint64_t)1 << VPN_CLIENT_ENABLE |
                             (uint64_t)1 << VPN_CLIENT_NAME |
                             (uint64_t)1 << VPN_CLIENT_TYPE |
                             (uint64_t)1 << VPN_CLIENT_L2TP_SERVER |
                             (uint64_t)1 << VPN_CLIENT_L2TP_USER | 
                             (uint64_t)1 << VPN_CLIENT_L2TP_PASSWORD |
                             (uint64_t)1 << VPN_CLIENT_WAN_INTF,

    [VPN_CLIENT_TYPE_PPTP] = (uint64_t)1 << VPN_CLIENT_ID | 
                             (uint64_t)1 << VPN_CLIENT_ENABLE |
                             (uint64_t)1 << VPN_CLIENT_NAME |
                             (uint64_t)1 << VPN_CLIENT_TYPE |
                             (uint64_t)1 << VPN_CLIENT_PPTP_SERVER |
                             (uint64_t)1 << VPN_CLIENT_PPTP_USER |
                             (uint64_t)1 << VPN_CLIENT_PPTP_PASSWORD |
                             (uint64_t)1 << VPN_CLIENT_WAN_INTF,

    [VPN_CLIENT_TYPE_IPSEC] = (uint64_t)1 << VPN_CLIENT_ID | 
                              (uint64_t)1 << VPN_CLIENT_ENABLE |
                              (uint64_t)1 << VPN_CLIENT_NAME |
                              (uint64_t)1 << VPN_CLIENT_TYPE |
                              (uint64_t)1 << VPN_CLIENT_WAN_INTF |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P1_REMOTE_GW |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P1_IKE_VER |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P1_IKE_LIFETIME |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P1_NEGO_MODE |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P1_AUTH_METHOD |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P1_ENCRYPT |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P1_AUTH |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P1_DH |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P2_LOCAL_SUBNET |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P2_LOCAL_SOURCE_IP |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P2_REMOTE_SUBNET_LIST |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P2_SA_LIFETIME |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P2_PROTO |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P2_ESP_ENCRYPT |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P2_ESP_AUTH |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P2_ENCAP_MODE |
                              (uint64_t)1 << VPN_CLIENT_IPSEC_P2_PFS_GROUP,

};

static uint64_t optional_en_cfg_required_paras[__VPN_CLIENT_MAX] = {
    [VPN_CLIENT_IPSEC_P1_REKEY] = (uint64_t)1 << VPN_CLIENT_IPSEC_P1_REKEY |
                                  (uint64_t)1 << VPN_CLIENT_IPSEC_P1_KEYINGTRIES,

    [VPN_CLIENT_IPSEC_P1_DPD_EN] = (uint64_t)1 << VPN_CLIENT_IPSEC_P1_DPD_EN |
                                   (uint64_t)1 << VPN_CLIENT_IPSEC_P1_DPD_DELAY |
                                   (uint64_t)1 << VPN_CLIENT_IPSEC_P1_DPD_IDLE |
                                   (uint64_t)1 << VPN_CLIENT_IPSEC_P1_DPD_ACTION,
};

char *support_ike_alg[__IKE_ALG_TYPE_MAX][IKE_ALG_CNT_MAX] = {
    [IKE_ALG_TYPE_ENCRYPT] = {
        "3des",
        "aes128",
        "aes192",
        "aes256",
        NULL
    },
    [IKE_ALG_TYPE_AUTH] = {
        "md5",
        "sha1",
        "sha2_256",
        "sha2_384",
        "sha2_512",
        NULL
    },
    [IKE_ALG_TYPE_DH] = {
        "dh2",
        "dh5",
        "dh14",
        "dh19",
        "dh20",
        "dh21",
        NULL
   }
};

char *support_auth_method[__IKE_AUTH_METHOD_MAX] = {
    "psk"
};

/* Functions */

//==============================================================================
int
vpn_aync_reload_ipsec(
    void
)
//==============================================================================
{
    vpn_update_ipsec_reload_helper( IPSEC_RELOAD_ACTION_RESTART, NULL );
    return 0;
}

//==============================================================================
int
vpn_ctrl_ipsec_status(
    char *ipsec_conn,
    int action
)
//==============================================================================
{
    char cmd[BUF_LEN_256] = { 0 };
    char *action2str[__VPN_CLIENT_ACTION_MAX] = {
        "connect",
        "concel",
        "disconnect"
    };

    cfmanager_log_message( L_DEBUG, "Control ipsec connection %s to %s!\n",
        ipsec_conn, action2str[action] );
    if ( action == VPN_CLIENT_ACTION_CONNECT ) {
        snprintf( cmd, sizeof(cmd), "ipsec auto --start %s", ipsec_conn );
    }
    else {
        snprintf( cmd, sizeof(cmd), "ipsec auto --delete %s", ipsec_conn );
    }
    system( cmd );

    return 0;
}

//==============================================================================
int
vpn_get_ipsec_status(
    char *remote_gw
)
//==============================================================================
{
    char *ipaddr;
    int connected = 0;
    int trap = 0;
    FILE *fp;
    char eroute_status[BUF_LEN_32] = { 0 };
    char line[BUF_LEN_512] = { 0 };

    fp = fopen( "/proc/net/ipsec_eroute", "r" );
    if ( !fp ) {
        cfmanager_log_message( L_ERR, "fopen /proc/net/ipsec_eroute failed, %s!\n",
            strerror(errno) );
        return VPN_CONN_STATUS_DISCONNECTED;
    }

    while ( fgets( line, sizeof(line), fp ) ) {
        sscanf( line, "%*u %*s %*c %*c %*s %*c %*c %s", eroute_status );

        if ( eroute_status != NULL ) {
            if ( !strcmp( eroute_status, "%trap" ) )
                trap = 1;
        }

        ipaddr = strtok( eroute_status, "@" );
        ipaddr = strtok( NULL, "@" );

        /* compare vpn gateway to the one read from ipsec_eroute
        * if connected the gateway should be present in the eroute */
        if ( !ipaddr ) {
            continue;
        }

        if ( !strcmp(remote_gw, ipaddr) ) {
            connected = 1;
            break;
        }
    }

    fclose(fp);

    if ( connected ) {
        return VPN_CONN_STATUS_CONNECTED;
    }
    else if ( trap ) {
        return VPN_CONN_STATUS_CONNECTING;
    }

    return VPN_CONN_STATUS_DISCONNECTED;    
}

//==============================================================================
int
vpn_ike_alg_match(
    char *alg,
    int type    
)
//==============================================================================
{
    char *p;

    for ( p = support_ike_alg[type][0]; p != NULL; p++ ) {
        if ( !strcmp(alg, p) ) {
            return 1;
        }
    }

    return 0;
}

//==============================================================================
static inline int
vpn_ike_auth_method_match(
    char *method,
    int *method_index
)
//==============================================================================
{
    int i;

    *method_index = -1;
    for ( i = 0; i < __IKE_AUTH_METHOD_MAX; i++ ) {
        if ( !strcmp(method, support_auth_method[i]) ) {
            *method_index = i;
            return 1;
        }
    }

    return 0;
}

//==============================================================================
int
vpn_del_client(
    char *section_nm
)
//==============================================================================
{
    struct cm_config *cm_cfg = NULL;

    cm_cfg = util_get_vltree_node( &cm_vpn_client_vltree, VLTREE_CM_TREE, section_nm );
    if ( cm_cfg ) {
        cfmanager_log_message( L_WARNING, "Delete vpn client %s!\n", section_nm );
        config_del_named_section( CFMANAGER_CONFIG_NAME, "vpn_service", section_nm );
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_VPN_CLIENT, false );
    }

    return 0;
}

//==============================================================================
int
vpn_del_vpn_split_by_client(
    char *id
)
//==============================================================================
{
    char section_nm[BUF_LEN_64] = { 0 };
    struct cm_config *cm_cfg = NULL;

    snprintf( section_nm, sizeof(section_nm), "vpn_split%s", id );
    cm_cfg = util_get_vltree_node( &cm_vpn_split_vltree, VLTREE_CM_TREE, section_nm );
    if ( cm_cfg ) {
        cfmanager_log_message( L_WARNING, "Delete vpn split %s!\n", section_nm );
        config_del_named_section( CFMANAGER_CONFIG_NAME, "vpn_split", section_nm );
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_VPN_SPLIT, false );
    }

    return 0;
}

//==============================================================================
int
vpn_del_client_by_wan(
    char *wan
)
//==============================================================================
{
    struct cm_config *cm_cfg = NULL;

    vlist_for_each_element( &cm_vpn_client_vltree, cm_cfg, node ) {
        struct blob_attr *tb[__VPN_CLIENT_MAX] = { 0 };

        blobmsg_parse( vpn_client_policy,
            __VPN_CLIENT_MAX,
            tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        if ( !strcmp( wan, blobmsg_get_string(tb[VPN_CLIENT_WAN_INTF]) ) ) {
            vpn_del_vpn_split_by_client( blobmsg_get_string(tb[VPN_CLIENT_ID]) );
            vpn_del_client( cm_cfg->cf_section.name );
            break;
        }
    }

    return 0;
}

//==============================================================================
int
vpn_parse_client_list(
    struct blob_attr *attr,
    int *fw_need_load,
    int *mwan3_need_load
)
//==============================================================================
{
    struct blob_attr *new_tb[__VPN_CLIENT_MAX];
    struct blob_attr *old_tb[__VPN_CLIENT_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int i = 0, j = 0;
    int same_cfg = 1;
    int ret = ERRCODE_SUCCESS;
    uint64_t change_option = 0;
    char section_nm[BUF_LEN_64] = { 0 };
    int client_type;
    char *encrypt_alg;
    char *auth_alg;
    char *dh_alg;
    int auth_method;

    blobmsg_parse( vpn_client_policy,
        __VPN_CLIENT_MAX,
        new_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    if ( !new_tb[VPN_CLIENT_TYPE] ) {
        cfmanager_log_message( L_ERR,
            "Missing parameters '%s' for vpn client!\n", vpn_client_policy[VPN_CLIENT_TYPE].name );
        return ERRCODE_PARAMETER_ERROR;
    }

    // Check vpn client type required parameters.
    client_type = util_blobmsg_get_int( new_tb[VPN_CLIENT_TYPE], VPN_CLIENT_TYPE_L2TP );
    if ( client_type >= __VPN_CLIENT_TYPE_MAX ) {
        cfmanager_log_message( L_ERR,
            "Don't support vpn client type %d!\n", client_type );
        return ERRCODE_PARAMETER_ERROR;
    }

    for ( i = 0; i < __VPN_CLIENT_MAX; i++ ) {
        if ( (((uint64_t)1 << i) & client_required_paras[client_type]) && 
            (!new_tb[i] || blobmsg_type(new_tb[i]) != vpn_client_policy[i].type) ) {
            cfmanager_log_message( L_ERR,
                "Missing parameters '%s' or data type error!\n", vpn_client_policy[i].name );
            return ERRCODE_PARAMETER_ERROR;
        }
    }

    // Check optional enable config required parameters.
    for ( i = 0; i < __VPN_CLIENT_MAX; i++ ) {
        if ( optional_en_cfg_required_paras[i] && new_tb[i] &&
            blobmsg_type(new_tb[i]) == BLOBMSG_TYPE_BOOL && blobmsg_get_bool(new_tb[i]) ) {
            for ( j = 0; j < __VPN_CLIENT_MAX; j++ ) {
                if ( (((uint64_t)1 << j) & optional_en_cfg_required_paras[i]) &&
                    (!new_tb[j] || blobmsg_type(new_tb[i]) != vpn_client_policy[i].type) ) {
                    cfmanager_log_message( L_ERR,
                        "Missing parameters '%s' or data type error!\n", vpn_client_policy[j].name );
                    return ERRCODE_PARAMETER_ERROR;
                }
            }
        }
    }

    if ( new_tb[VPN_CLIENT_NAME] ) {
        char *client_nm = blobmsg_get_string( new_tb[VPN_CLIENT_NAME] );

        if ( !strcmp(client_nm, "WAN1_VPN") || !strcmp(client_nm, "WAN2_VPN") ) {
            cfmanager_log_message( L_ERR,
                "Cannot use client name %s!\n", client_nm );
            return ERRCODE_PARAMETER_ERROR;
        }
    }

    // Check auth method
    if ( !vpn_ike_auth_method_match(blobmsg_get_string(new_tb[VPN_CLIENT_IPSEC_P1_AUTH_METHOD]),
        &auth_method) ) {
        cfmanager_log_message( L_ERR,
            "Don't support ike auth method '%s'!\n", 
            blobmsg_get_string(new_tb[VPN_CLIENT_IPSEC_P1_AUTH_METHOD]) );
        return ERRCODE_PARAMETER_ERROR;
    }

    if ( auth_method == IKE_AUTH_METHOD_PSK && !new_tb[VPN_CLIENT_IPSEC_P1_PSK] ) {
        cfmanager_log_message( L_ERR,
            "Missing parameters '%s' for psk auth!\n",
            vpn_client_policy[VPN_CLIENT_IPSEC_P1_PSK].name );
        return ERRCODE_PARAMETER_ERROR; 
    }

    // Check Phase 1 algorithm.
    if ( client_type == VPN_CLIENT_TYPE_IPSEC ) {
        encrypt_alg = blobmsg_get_string( new_tb[VPN_CLIENT_IPSEC_P1_ENCRYPT] );
        if ( !vpn_ike_alg_match(encrypt_alg, IKE_ALG_TYPE_ENCRYPT) ) {
            cfmanager_log_message( L_ERR,
                "Don't support phase1 encryption alg %s!\n", encrypt_alg );
            return ERRCODE_PARAMETER_ERROR;
        }
        
        auth_alg = blobmsg_get_string( new_tb[VPN_CLIENT_IPSEC_P1_AUTH] );
        if ( !vpn_ike_alg_match(auth_alg, IKE_ALG_TYPE_AUTH) ) {
            cfmanager_log_message( L_ERR,
                "Don't support phase1 authentication alg %s!\n", auth_alg );
            return ERRCODE_PARAMETER_ERROR;
        }
        
        dh_alg = blobmsg_get_string( new_tb[VPN_CLIENT_IPSEC_P1_DH] );
        if ( !vpn_ike_alg_match(dh_alg, IKE_ALG_TYPE_DH) ) {
            cfmanager_log_message( L_ERR,
                "Don't support phase1 dh group %s!\n", dh_alg );
            return ERRCODE_PARAMETER_ERROR;
        }
        
        // Check Phase 2 algorithm.
        encrypt_alg = blobmsg_get_string( new_tb[VPN_CLIENT_IPSEC_P2_ESP_ENCRYPT] );
        if ( !vpn_ike_alg_match(encrypt_alg, IKE_ALG_TYPE_ENCRYPT) ) {
            cfmanager_log_message( L_ERR,
                "Don't support phase2 encryption alg %s!\n", encrypt_alg );
            return ERRCODE_PARAMETER_ERROR;
        }
        
        auth_alg = blobmsg_get_string( new_tb[VPN_CLIENT_IPSEC_P2_ESP_AUTH] );
        if ( !vpn_ike_alg_match(auth_alg, IKE_ALG_TYPE_AUTH) ) {
            cfmanager_log_message( L_ERR,
                "Don't support phase2 authentication alg %s!\n", auth_alg );
            return ERRCODE_PARAMETER_ERROR;
        }
        
        dh_alg = blobmsg_get_string( new_tb[VPN_CLIENT_IPSEC_P2_PFS_GROUP] );
        if ( strcmp(dh_alg, "disabled") && !vpn_ike_alg_match( dh_alg,IKE_ALG_TYPE_DH ) ) {
            cfmanager_log_message( L_ERR,
                "Don't support phase2 dh group %s!\n", dh_alg );
            return ERRCODE_PARAMETER_ERROR;
        }
    }

    *fw_need_load = 0;
    *mwan3_need_load = 0;
    snprintf( section_nm, sizeof(section_nm), "vpn_service%s", blobmsg_get_string( new_tb[VPN_CLIENT_ID] ) );
    cm_cfg = util_get_vltree_node( &cm_vpn_client_vltree, VLTREE_CM_TREE, section_nm );
    if ( !cm_cfg ) {
        cfmanager_log_message( L_WARNING, "NOT found vpn client %s information in %s, create it...\n", 
            section_nm, CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "vpn_service", section_nm );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section %s in %s\n", section_nm, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __VPN_CLIENT_MAX; i++ ) {
            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, section_nm, vpn_client_policy[i].name );

            if ( i == VPN_CLIENT_IPSEC_P2_REMOTE_SUBNET_LIST ) {
                struct blob_attr *cur = NULL;
                int rem;

                blobmsg_for_each_attr( cur, new_tb[i], rem ) {
                    cfmanager_log_message( L_DEBUG, "Add path=%s, value=%s\n", path, blobmsg_get_string(cur) );
                    config_uci_add_list( path, blobmsg_get_string(cur), 0 );
                }
            }
            else {
                config_set_by_blob( new_tb[i], path, vpn_client_policy[i].type );
            }
        }

        same_cfg = 0;
        *fw_need_load = 1;
        *mwan3_need_load = 1;
    }
    else {
        cfmanager_log_message( L_WARNING, "Update vpn client %s in %s\n", 
            section_nm, CFMANAGER_CONFIG_NAME );

        blobmsg_parse( vpn_client_policy,
            __VPN_CLIENT_MAX,
            old_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __VPN_CLIENT_MAX; i++ ) {
            if ( !new_tb[i] ) {
                continue;
            }

            if ( sgreq_compar_attr( new_tb[i], old_tb[i], vpn_client_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, section_nm, vpn_client_policy[i].name );
            if ( i == VPN_CLIENT_IPSEC_P2_REMOTE_SUBNET_LIST ) {
                struct blob_attr *cur = NULL;
                int rem;

                config_uci_del( path, 0 );
                blobmsg_for_each_attr( cur, new_tb[i], rem ) {
                    cfmanager_log_message( L_DEBUG, "Add path=%s, value=%s\n", path, blobmsg_get_string(cur) );
                    config_uci_add_list( path, blobmsg_get_string(cur), 0 );
                }
            }
            else {
                config_set_by_blob( new_tb[i], path, vpn_client_policy[i].type );
            }

            same_cfg = 0;
            change_option |= BIT(i);
        }

        // If only name or ipsec config changed, we don't need to reload firewall
        if ( (change_option != BIT(VPN_CLIENT_NAME))
            && (change_option & (~VPN_CLIENT_IPSEC_CFG_MASK))
            ) {
            *fw_need_load = 1;
            *mwan3_need_load = 1;
        }
    }

    if ( same_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "The vpn client %s config is same\n", section_nm );
    }

    return ret;
}

//==============================================================================
int
vpn_parse_split_list(
    struct blob_attr *attr,
    int *mwan3_need_load
)
//==============================================================================
{
    struct blob_attr *new_tb[__VPN_SPLIT_MAX] = { 0 };
    struct blob_attr *old_tb[__VPN_SPLIT_MAX] = { 0 };
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int i = 0;
    int same_cfg = 1;
    int ret = ERRCODE_SUCCESS;
    char service_nm[BUF_LEN_64] = { 0 };

    blobmsg_parse( vpn_split_policy,
        __VPN_SPLIT_MAX,
        new_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    if ( !new_tb[VPN_SPLIT_SERVICE_ID] || !new_tb[VPN_SPLIT_MODE] ) {
        cfmanager_log_message( L_ERR,
            "Missing configure in json!\n" );
        return ERRCODE_PARAMETER_ERROR;
    }

    snprintf( service_nm, sizeof(service_nm), "vpn_split%s", blobmsg_get_string(new_tb[VPN_SPLIT_SERVICE_ID]) );
    cm_cfg = util_get_vltree_node( &cm_vpn_split_vltree, VLTREE_CM_TREE, service_nm );
    if ( !cm_cfg ) {
        cfmanager_log_message( L_WARNING, "NOT found vpn split service %s information in %s, create it...\n", 
            service_nm, CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "vpn_split", service_nm );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section %s in %s\n", service_nm, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __VPN_SPLIT_MAX; i++ ) {
            struct blob_attr *cur;
            int rem;

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, service_nm, vpn_split_policy[i].name );
            if ( i == VPN_SPLIT_SERVICE_ADDR_LIST ) {
                blobmsg_for_each_attr( cur, new_tb[i], rem ) {
                    cfmanager_log_message( L_DEBUG, "Add path=%s, value=%s\n", path, blobmsg_get_string(cur) );
                    config_uci_add_list( path, blobmsg_get_string(cur), 0 );
                }
            }
            else if ( i == VPN_SPLIT_DEV_LIST ) {
                blobmsg_for_each_attr( cur, new_tb[i], rem ) {
                    char val[BUF_LEN_512] = { 0 };
                    char dev_name[BUF_LEN_256] = { 0 };
                    struct blob_attr *tb[__VPN_SPLIT_DEV_ATTR_MAX];

                    blobmsg_parse( vpn_split_dev_attrs_policy,
                        __VPN_SPLIT_DEV_ATTR_MAX,
                        tb,
                        blobmsg_data( cur ),
                        blobmsg_len( cur ) );

                    if ( tb[VPN_SPLIT_DEV_ATTR_NAME] ) {
                        snprintf( dev_name, sizeof(dev_name), "%s",  blobmsg_get_string(tb[VPN_SPLIT_DEV_ATTR_NAME]) );
                    }

                    snprintf( val, sizeof(val), "%s/%s",
                        dev_name,
                        blobmsg_get_string(tb[VPN_SPLIT_DEV_ATTR_MAC]));
                    cfmanager_log_message( L_DEBUG, "Add path=%s, value=%s\n", path, val);
                    config_uci_add_list( path, val, 0 );
                }
            }
            else {
                config_set_by_blob( new_tb[i], path, vpn_split_policy[i].type );
            }
        }

        same_cfg = 0;
    }
    else {
        cfmanager_log_message( L_WARNING, "Update vpn split service %s in %s\n",
            service_nm, CFMANAGER_CONFIG_NAME );

        blobmsg_parse( vpn_split_policy,
            __VPN_SPLIT_MAX,
            old_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __VPN_SPLIT_MAX; i++ ) {
            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, service_nm, vpn_split_policy[i].name );
            if ( i != VPN_SPLIT_SERVICE_ADDR_LIST && i != VPN_SPLIT_DEV_LIST ) {
                if ( !new_tb[i] ) {
                    continue;
                }

                if ( sgreq_compar_attr( new_tb[i], old_tb[i], vpn_split_policy[i].type ) ) {
                    continue;
                }

                same_cfg = 0;
                config_set_by_blob( new_tb[i], path, vpn_split_policy[i].type );
            }
            else {
                int new_rem, old_rem;
                struct blob_attr *new_attr, *old_attr;
                int found = 0;
                char tmp_val[BUF_LEN_512] = { 0 };
                char *attr_val = NULL;

                // Add new data
                blobmsg_for_each_attr( new_attr, new_tb[i], new_rem ) {
                    if ( i == VPN_SPLIT_DEV_LIST ) {
                        struct blob_attr *tb[__VPN_SPLIT_DEV_ATTR_MAX];
                        char dev_name[BUF_LEN_256] = { 0 };
                    
                        blobmsg_parse( vpn_split_dev_attrs_policy,
                            __VPN_SPLIT_DEV_ATTR_MAX,
                            tb,
                            blobmsg_data( new_attr ),
                            blobmsg_len( new_attr ) );

                        if ( tb[VPN_SPLIT_DEV_ATTR_NAME] ) {
                            snprintf( dev_name, sizeof(dev_name), "%s", blobmsg_get_string(tb[VPN_SPLIT_DEV_ATTR_NAME]) );
                        }

                        snprintf( tmp_val, sizeof(tmp_val), "%s/%s",
                            dev_name,
                            blobmsg_get_string(tb[VPN_SPLIT_DEV_ATTR_MAC]));
                        attr_val = tmp_val;
                    }
                    else {
                        attr_val = blobmsg_get_string(new_attr);
                    }

                    blobmsg_for_each_attr( old_attr, old_tb[i], old_rem ) {
                        if ( !strcmp(attr_val, blobmsg_get_string(old_attr)) ) {
                            found = 1;
                            break;
                        }
                    }

                    if ( !found ) {
                        same_cfg = 0;
                        config_uci_add_list( path, attr_val, 0 );
                    }

                    found = 0;
                }       

                // Delete old data
                found = 0;
                blobmsg_for_each_attr( old_attr, old_tb[i], old_rem ) {
                    blobmsg_for_each_attr( new_attr, new_tb[i], new_rem ) {
                        if ( i == VPN_SPLIT_DEV_LIST ) {
                            struct blob_attr *tb[__VPN_SPLIT_DEV_ATTR_MAX];
                            char dev_name[BUF_LEN_256] = { 0 };
                            
                            blobmsg_parse( vpn_split_dev_attrs_policy,
                                __VPN_SPLIT_DEV_ATTR_MAX,
                                tb,
                                blobmsg_data( new_attr ),
                                blobmsg_len( new_attr ) );

                            if ( tb[VPN_SPLIT_DEV_ATTR_NAME] ) {
                                snprintf( dev_name, sizeof(dev_name), "%s", blobmsg_get_string(tb[VPN_SPLIT_DEV_ATTR_NAME]) );
                            }

                            snprintf( tmp_val, sizeof(tmp_val), "%s/%s",
                                dev_name,
                                blobmsg_get_string(tb[VPN_SPLIT_DEV_ATTR_MAC]));
                            attr_val = tmp_val;
                        }
                        else {
                            attr_val = blobmsg_get_string(new_attr);
                        }

                        if ( !strcmp(attr_val, blobmsg_get_string(old_attr)) ) {
                            found = 1;
                            break;
                        }
                    }

                    if ( !found ) {
                        same_cfg = 0;
                        config_uci_del_list( path, blobmsg_get_string(old_attr), 0 );
                    }

                    found = 0;
                }
            }
        }
    }

    if ( same_cfg ) {
        cfmanager_log_message( L_DEBUG, 
            "The vpn split service %s config is same\n", service_nm );
        *mwan3_need_load = 0;
    }
    else {
        *mwan3_need_load = 1;
    }

    return ret;
}

/* EOF */

