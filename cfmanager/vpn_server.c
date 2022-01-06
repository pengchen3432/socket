/****************************************************************************
*
* FILENAME:        vpn_server.c
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
extern const struct blobmsg_policy vpn_server_policy[__VPN_SERVER_MAX];
extern const struct blobmsg_policy sgreq_vpn_server_policy[__VPN_SERVER_MAX];
extern const struct blobmsg_policy ipsec_cmn_setting_policy[__IPSEC_CMN_SETTING_ATTR_MAX];
extern const struct blobmsg_policy ipsec_dial_in_user_policy[__IPSEC_DIAL_IN_USER_ATTR_MAX];

// Cached reload action before execute init script.
int32_t ipsec_final_reload_action = IPSEC_RELOAD_ACTION_NONE;
// enabled ipsec vpn count on both WANs.
uint32_t ipsec_vpn_cnt[WAN_CNT_MAX][__VPN_ROLE_MAX] = { {0} };
uchar vpn_user_handle_done[VPN_SERVER_CNT_MAX] = { 0 };

static uint64_t server_type_required_paras[__VPN_SERVER_TYPE_MAX] = {
    [VPN_SERVER_TYPE_IPSEC] = (uint64_t)1 << VPN_SERVER_ID |
                              (uint64_t)1 << VPN_SERVER_ENABLE |
#ifndef GWN7052
                              (uint64_t)1 << VPN_SERVER_NAME |
#endif
                              (uint64_t)1 << VPN_SERVER_TYPE |
                              (uint64_t)1 << VPN_SERVER_WAN_INTF,

};

static uint64_t ipsec_dial_in_user_required_paras[__IPSEC_DIAL_IN_USER_TYPE_MAX] = {
    [IPSEC_DIAL_IN_USER_TYPE_IKEV1] = (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_ID |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_TYPE |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_VLAN_ID |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_IP_RANGE,

    [IPSEC_DIAL_IN_USER_TYPE_IKEV2] = (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_ID |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_TYPE |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_VLAN_ID |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_IP_RANGE, 

    [IPSEC_DIAL_IN_USER_TYPE_XAUTH] = (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_ID |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_TYPE |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_USER |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_PASSWORD |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_VLAN_ID |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_IP_RANGE |
                                      (uint64_t)1 << IPSEC_DIAL_IN_USER_ATTR_ACTION,

};

static uint64_t ipsec_cmn_setting_required_paras = (uint64_t)1 << IPSEC_CMN_SETTING_ATTR_PSK |
                                                IPSEC_CMN_SETTING_ATTR_ENCRYPT_ALG |
                                                IPSEC_CMN_SETTING_ATTR_AUTH_ALG |
                                                IPSEC_CMN_SETTING_ATTR_DH;

char *ipsec_action2str[__IPSEC_RELOAD_ACTION_MAX] = {
    "NONE",
    "START",
    "STOP",
    "RESTART",
    "ADD CONNECTION",
    "DELETE CONNECTION",
    "REPLACE CONNECTION",
    "REPLACE AND UP CONNECTION"
};

static char *ipsec_status2str[__IPSEC_STATUS_MAX] = {
    "NONE",
    "ENABLE",
    "DISABLE",
};

static char *vpn_role2str[__VPN_ROLE_MAX] = {
    "NONE",
    "CLIENT",
    "SERVER"
};

static char *wan2str[WAN_CNT_MAX] = {
    "wan0",
    "wan1"
};

static char *ip_range_list = NULL;
static char *user_list = NULL;

/* Functions */
//==============================================================================
void
vpn_prepare_reload_ipsec(
    void
)
//==============================================================================
{
    ipsec_final_reload_action = IPSEC_RELOAD_ACTION_NONE;
    memset( vpn_user_handle_done, 0, sizeof(vpn_user_handle_done) );
    util_cpy_file( TEMP_IPSEC_RELOAD_HELPER_SCRIPT, IPSEC_RELOAD_HELPER_SCRIPT );
    remove( TEMP_IPSEC_RELOAD_HELPER_SCRIPT );
}

//==============================================================================
int
vpn_update_ipsec_reload_helper(
    int action,
    char *ipsec_conn
)
//==============================================================================
{
    FILE *fp;
    int need_update = 0;

    if ( action == IPSEC_RELOAD_ACTION_NONE ) {
        cfmanager_log_message( L_WARNING, "Ignore ipsec reload action 'NONE'\n" );
        return 0;
    }

    if ( !ipsec_conn && action != IPSEC_RELOAD_ACTION_START && action != IPSEC_RELOAD_ACTION_STOP &&
        action != IPSEC_RELOAD_ACTION_RESTART ) {
        cfmanager_log_message( L_WARNING, 
            "Ignore 'NULL' ipsec connection when action isn't START/STOP/RESTART!\n" );
        return 0;
    }

    cfmanager_log_message( L_ERR, "IPsec reload action %s, last reload action %s\n",
        ipsec_action2str[action],
        ipsec_action2str[ipsec_final_reload_action] );

    switch (action) {
        case IPSEC_RELOAD_ACTION_START:
            if ( ipsec_final_reload_action == IPSEC_RELOAD_ACTION_STOP ) {
                need_update = 1;
                ipsec_final_reload_action = IPSEC_RELOAD_ACTION_RESTART;
            }
            else if ( ipsec_final_reload_action != IPSEC_RELOAD_ACTION_RESTART ) {
                need_update = 1;
                ipsec_final_reload_action = action; // Can safe override other situations.
            }
            break;

        case IPSEC_RELOAD_ACTION_STOP:
            if ( ipsec_final_reload_action == IPSEC_RELOAD_ACTION_START ) {
                need_update = 1;
                ipsec_final_reload_action = IPSEC_RELOAD_ACTION_RESTART;
            }
            else if ( ipsec_final_reload_action != IPSEC_RELOAD_ACTION_RESTART ) {
                need_update = 1;
                ipsec_final_reload_action = action; // Can safe override other situations.
            }
            break;

        case IPSEC_RELOAD_ACTION_RESTART:
            // always update.
            need_update = 1;
            ipsec_final_reload_action = action;
            break;

        case IPSEC_RELOAD_ACTION_ADD_CONN:
        case IPSEC_RELOAD_ACTION_DEL_CONN:
        case IPSEC_RELOAD_ACTION_REPLACE_CONN:
        case IPSEC_RELOAD_ACTION_REPLACE_AND_UP_CONN:
            switch ( ipsec_final_reload_action ) {
                case IPSEC_RELOAD_ACTION_ADD_CONN:
                case IPSEC_RELOAD_ACTION_DEL_CONN:
                case IPSEC_RELOAD_ACTION_REPLACE_CONN:
                case IPSEC_RELOAD_ACTION_REPLACE_AND_UP_CONN:
                case IPSEC_RELOAD_ACTION_NONE:
                    need_update = 1;
                    ipsec_final_reload_action = action;
                    break;
            }
            break;

        default:
            break;
    }

    cfmanager_log_message( L_ERR, "%s %s update, final ipsec reload action '%s'!\n",
        TEMP_IPSEC_RELOAD_HELPER_SCRIPT,
        need_update ? "needs to" : "doesn't need to",
        ipsec_action2str[ipsec_final_reload_action] );

    if ( need_update ) {
        /* Tell ipsec reload to do what. */
        char *mode = "w+";

        if ( ipsec_final_reload_action == IPSEC_RELOAD_ACTION_ADD_CONN ||
            ipsec_final_reload_action == IPSEC_RELOAD_ACTION_DEL_CONN ||
            ipsec_final_reload_action == IPSEC_RELOAD_ACTION_REPLACE_CONN ||
            ipsec_final_reload_action == IPSEC_RELOAD_ACTION_REPLACE_AND_UP_CONN ) {
            mode = "a+";
        }

        fp = fopen( TEMP_IPSEC_RELOAD_HELPER_SCRIPT, mode );
        if ( !fp ) {
            cfmanager_log_message( L_ERR, "fopen %s failed, %s!\n",
                TEMP_IPSEC_RELOAD_HELPER_SCRIPT, strerror(errno) );
            return -1;
        }

        switch (ipsec_final_reload_action) {
            case IPSEC_RELOAD_ACTION_START:
                fprintf( fp, "start\n" );
                break;

            case IPSEC_RELOAD_ACTION_STOP:
                fprintf( fp, "stop\n" );
                break;

            case IPSEC_RELOAD_ACTION_RESTART:
                fprintf( fp, "stop\n" );
                fprintf( fp, "start\n" );
                break;

            case IPSEC_RELOAD_ACTION_ADD_CONN:
                fprintf( fp, "ipsec auto --add %s\n", ipsec_conn );
                break;

            case IPSEC_RELOAD_ACTION_DEL_CONN:
                fprintf( fp, "ipsec auto --delete %s\n", ipsec_conn );
                break;

            case IPSEC_RELOAD_ACTION_REPLACE_CONN:
                fprintf( fp, "ipsec auto --replace %s\n", ipsec_conn );
                break;

            case IPSEC_RELOAD_ACTION_REPLACE_AND_UP_CONN:
                fprintf( fp, "ipsec auto --replace %s\n", ipsec_conn );
                fprintf( fp, "ipsec auto --up %s\n", ipsec_conn );
                break;

            default:
                break;
        }
        
        fflush( fp );
        fclose( fp );

        // Trigger ipsec reload.
        apply_add( "ipsec" );
        apply_timer_start();
    }

    return 0;
}

//==============================================================================
int
vpn_update_ipsec_cnt_and_reload_helper(
    int wan_index,
    int role,
    int status,
    char *ipsec_conn
)
//==============================================================================
{
    int reload_action = IPSEC_RELOAD_ACTION_NONE;

    cfmanager_log_message( L_INFO, "%s ipsec vpn %s on %s\n",
        ipsec_status2str[status],
        vpn_role2str[role],
        wan2str[wan_index]);

    if ( status == IPSEC_STATUS_ENABLE ) {
        ipsec_vpn_cnt[wan_index][role]++;
    }
    else if ( status == IPSEC_STATUS_DISABLE ) {
        ipsec_vpn_cnt[wan_index][role]--;
    }

    if ( status == IPSEC_STATUS_ENABLE ) {
        if ( ipsec_vpn_cnt[WAN0][VPN_ROLE_CLIENT] + 
            ipsec_vpn_cnt[WAN0][VPN_ROLE_SERVER] + 
            ipsec_vpn_cnt[WAN1][VPN_ROLE_CLIENT] + 
            ipsec_vpn_cnt[WAN1][VPN_ROLE_SERVER] == 1 ) {
            // Only one ipsec vpn enabled on both WANs.
            reload_action = IPSEC_RELOAD_ACTION_START;
        }
        else if ( ipsec_vpn_cnt[wan_index][VPN_ROLE_CLIENT] + 
            ipsec_vpn_cnt[wan_index][VPN_ROLE_SERVER] == 1 ) {
            /*
             * Only one ipsec vpn enabled on this wan,
             * and another wan has additional ipsec vpns enabled.
             */
             reload_action = IPSEC_RELOAD_ACTION_RESTART;
        }
        else {
            // Both WANs have ipsec vpn enabled, just need to add connection.
            reload_action = IPSEC_RELOAD_ACTION_ADD_CONN;
            if ( !ipsec_conn ) {
                cfmanager_log_message( L_ERR, "Cannot add a 'NULL' ipsec connection!\n" );
                return -1;
            }
        }
    }
    else if ( status == IPSEC_STATUS_DISABLE ) {
        if ( ipsec_vpn_cnt[WAN0][VPN_ROLE_CLIENT] + 
            ipsec_vpn_cnt[WAN0][VPN_ROLE_SERVER] + 
            ipsec_vpn_cnt[WAN1][VPN_ROLE_CLIENT] + 
            ipsec_vpn_cnt[WAN1][VPN_ROLE_SERVER] == 0 ) {
            // No ipsec vpn enabled on both WANs.
            reload_action = IPSEC_RELOAD_ACTION_STOP;
        }
        else if ( ipsec_vpn_cnt[wan_index][VPN_ROLE_CLIENT] + 
            ipsec_vpn_cnt[wan_index][VPN_ROLE_SERVER] == 0 ) {
            // No ipsec vpn enabled on this WAN, but another WAN has ipsec vpn enabled.
            reload_action = IPSEC_RELOAD_ACTION_RESTART;
        }
        else {
            // Both WANs have ipsec vpn enabled, just need to update connection.
            reload_action = IPSEC_RELOAD_ACTION_DEL_CONN;
            if ( !ipsec_conn ) {
                cfmanager_log_message( L_ERR, "Cannot delete a 'NULL' ipsec connection!\n" );
                return -1;
            } 
        }
    }
    else  {
        cfmanager_log_message( L_ERR, "Don't support ipsec status %d\n", status );
        return -1;
    }
 
    cfmanager_log_message( L_INFO, "IPsec vpn count, wan0: clients=%d, server=%d, "
        "wan1: clients=%d, server=%d, final ipsec reload action %s\n",
        ipsec_vpn_cnt[WAN0][VPN_ROLE_CLIENT], ipsec_vpn_cnt[WAN0][VPN_ROLE_SERVER],
        ipsec_vpn_cnt[WAN1][VPN_ROLE_CLIENT], ipsec_vpn_cnt[WAN1][VPN_ROLE_SERVER],
        ipsec_action2str[reload_action] );

    vpn_update_ipsec_reload_helper( reload_action, ipsec_conn );

    return 0;
}

// ip range format - 192.168.1.1-192.168.1.100
//==============================================================================
static int
vpn_ip_range_overlap(
    char *old_range,
    char *new_range
)
//==============================================================================
{
    char *old_start = NULL, *old_end = NULL;
    char *new_start = NULL, *new_end = NULL;
    char *old_bak = NULL, *new_bak = NULL;
    char *p = NULL;
    uint32_t old_start_ip, old_end_ip;
    uint32_t new_start_ip, new_end_ip;
    int ret;
    int overlap = 0;

    old_bak = strdup( old_range );
    new_bak = strdup( new_range );

    p = strchr( old_bak, '-' );
    *p = '\0';
    p++;
    old_start = old_bak;
    old_end = p;

    p = strchr( new_bak, '-' );
    *p = '\0';
    p++;
    new_start = new_bak;
    new_end = p;

    inet_pton( AF_INET, old_start, &old_start_ip );
    inet_pton( AF_INET, old_end, &old_end_ip );
    inet_pton( AF_INET, new_start, &new_start_ip );
    inet_pton( AF_INET, new_end, &new_end_ip );

    ret = memcmp( &new_start_ip, &old_start_ip, sizeof(uint32_t) );
    if ( ret == 0 && !memcmp(&new_end_ip, &old_end_ip, sizeof(uint32_t)) ) {
        /* exact match */
    }
    else if ( ret < 0 ? memcmp(&new_end_ip, &old_start_ip, sizeof(uint32_t)) < 0 :
                memcmp(&new_start_ip, &old_end_ip, sizeof(uint32_t)) > 0 ) {
        /* before or after */
    }
    else {
        /* overlap */
        overlap = 1;
    }

    if ( old_bak ) {
        free( old_bak );
        old_bak = NULL;
    }

    if ( new_bak ) {
        free( new_bak );
        new_bak = NULL;
    }

    return overlap;
}

//==============================================================================
static int
vpn_is_user_or_ip_range_existed(
    char **orig_list,
    char *new_list_mem
)
//==============================================================================
{
    char *p = NULL;
    char *start = NULL, *end = NULL;
    char *list = NULL;
    char *list_bak = NULL;
    char *mem;
    uint32_t found = 0;
    int len, old_len;

    // ip range list format - 192.168.1.1-192.168.1.100;192.168.2.1-192.168.2.100;
    // user list format - user1;user2;
    list = *orig_list;
    if ( !list ) {
        len = strlen(new_list_mem) + 1 + 1;
        *orig_list = calloc( len, 1 );        
        snprintf( *orig_list, len, "%s;", new_list_mem );
    }
    else {
        list_bak = strdup( list );
        start = list_bak;
        end = list_bak + strlen( list_bak );

        for ( mem = start; mem < end; ) {
            p = strchr( list_bak, ';' );
            if ( p ) {
                *p = '\0';
                p++;
            }

            if ( (list == user_list && !strcmp(mem, new_list_mem)) ||
                (list == ip_range_list && vpn_ip_range_overlap(mem, new_list_mem)) ) {
                found = 1;
                break;
            }
            else {
                mem = p;
            }
        }

        if ( !found ) {
            old_len = strlen( list );
            len = old_len + strlen(new_list_mem) + 1 + 1;
            list = realloc( list, len );
            snprintf( &list[old_len], len - old_len, "%s;", new_list_mem );
            *orig_list = list;
        }
    }

    if ( list_bak ) {
        free( list_bak );
        list_bak = NULL;
    }

    return found;
}

// Use for add/update ipsec server
//==============================================================================
static int
vpn_verify_ipsec_server_config(
    struct blob_attr **attr
)
//==============================================================================
{
    char server_section[BUF_LEN_32] = { 0 };
    struct cm_config *cm_cfg = NULL;
    unsigned int rem;
    struct blob_attr *cur;
    int i;

    for ( i = 0; i < __VPN_SERVER_MAX; i++ ) {
        if ( (((uint64_t)1 << i) & server_type_required_paras[VPN_SERVER_TYPE_IPSEC]) && 
            (!attr[i] || blobmsg_type(attr[i]) != sgreq_vpn_server_policy[i].type) ) {
            cfmanager_log_message( L_ERR,
                "Missing parameters '%s' or data type error!\n", sgreq_vpn_server_policy[i].name );
            return ERRCODE_PARAMETER_ERROR;
        }
    }

    snprintf( server_section, sizeof(server_section), "vpn_server%s", 
        blobmsg_get_string(attr[VPN_SERVER_ID]) );
    cm_cfg = util_get_vltree_node( &cm_vpn_server_vltree, VLTREE_CM_TREE, server_section );
    if ( !cm_cfg && !attr[VPN_SERVER_IPSEC_CMN_SETTING] ) {
        cfmanager_log_message( L_ERR,
            "Missing parameters '%s' for add a new ipsec server!\n", 
            sgreq_vpn_server_policy[VPN_SERVER_IPSEC_CMN_SETTING].name );
        return ERRCODE_PARAMETER_ERROR;
    }

    if ( attr[VPN_SERVER_IPSEC_DIAL_IN_USER] ) {
        struct blob_attr *user_tb[__IPSEC_DIAL_IN_USER_ATTR_MAX];
        int user_action;
        int dial_in_type;

        if ( blobmsg_type(attr[VPN_SERVER_IPSEC_DIAL_IN_USER]) != BLOBMSG_TYPE_ARRAY ) {
            cfmanager_log_message( L_ERR,
                "parameters '%s' data type error!\n",
                ipsec_dial_in_user_policy[VPN_SERVER_IPSEC_DIAL_IN_USER].name );
            return ERRCODE_PARAMETER_ERROR;
        }

        blobmsg_for_each_attr( cur, attr[VPN_SERVER_IPSEC_DIAL_IN_USER], rem ) {
            blobmsg_parse( ipsec_dial_in_user_policy,
                __IPSEC_DIAL_IN_USER_ATTR_MAX,
                user_tb,
                blobmsg_data(cur),
                blobmsg_len(cur) );

            if ( !user_tb[IPSEC_DIAL_IN_USER_ATTR_ACTION] ) {
                cfmanager_log_message( L_ERR,
                    "Missing parameters '%s' for ipsec dial in user!\n", 
                    ipsec_dial_in_user_policy[IPSEC_DIAL_IN_USER_ATTR_TYPE].name );
                return ERRCODE_PARAMETER_ERROR;
            }

            user_action = util_blobmsg_get_int( user_tb[IPSEC_DIAL_IN_USER_ATTR_ACTION],
                IPSEC_DIAL_IN_USER_ACTION_ADD );
            if ( user_action >= __IPSEC_DIAL_IN_USER_ACTION_MAX ) {
                cfmanager_log_message( L_ERR,
                    "Don't support ipsec dial in user action %d!\n", user_action );
                return ERRCODE_PARAMETER_ERROR;
            }

            if ( user_action == IPSEC_DIAL_IN_USER_ACTION_ADD ) {
                char *user = NULL;
                char *ip_range = NULL;
#if 0
                char *vlan_id;
                char vlan_section[BUF_LEN_32] = { 0 };
                struct cm_config *vlan_cfg = NULL;
#endif
                dial_in_type = util_blobmsg_get_int( user_tb[IPSEC_DIAL_IN_USER_ATTR_TYPE],
                    IPSEC_DIAL_IN_USER_TYPE_IKEV1 );
                if ( dial_in_type >= __IPSEC_DIAL_IN_USER_TYPE_MAX ) {
                    cfmanager_log_message( L_ERR,
                        "Don't support ipsec dial in user type %d!\n", dial_in_type );
                    return ERRCODE_PARAMETER_ERROR;
                }

                for ( i = 0; i < __IPSEC_DIAL_IN_USER_ATTR_MAX; i++ ) {
                    if ( (((uint64_t)1 << i) & ipsec_dial_in_user_required_paras[dial_in_type]) && 
                        (!user_tb[i] || blobmsg_type(user_tb[i]) != ipsec_dial_in_user_policy[i].type) ) {
                        cfmanager_log_message( L_ERR,
                            "Missing parameters '%s' or data type error!\n", ipsec_dial_in_user_policy[i].name );
                        return ERRCODE_PARAMETER_ERROR;
                    }
                }

                // Check if username conflict.
                user = util_blobmsg_get_string( user_tb[IPSEC_DIAL_IN_USER_TYPE_XAUTH], "" );
                if ( dial_in_type == IPSEC_DIAL_IN_USER_TYPE_XAUTH &&
                    vpn_is_user_or_ip_range_existed(&user_list, user) ) {
                    cfmanager_log_message( L_ERR, "The user '%s' has existed!\n", user );
                    return ERRCODE_PARAMETER_ERROR;
                }

                // Check if ip range conflict with other user ip range.
                ip_range = util_blobmsg_get_string( user_tb[IPSEC_DIAL_IN_USER_ATTR_IP_RANGE], "" );
                if ( vpn_is_user_or_ip_range_existed(&ip_range_list, ip_range) ) {
                    cfmanager_log_message( L_ERR, "The ip range '%s' was overlaped!\n", ip_range );
                    return ERRCODE_PARAMETER_ERROR;
                }

                // Check if ip range conflict with DHCP ip range in vlan.
#if 0
                vlan_id = util_blobmsg_get_string( user_tb[IPSEC_DIAL_IN_USER_ATTR_VLAN_ID], "" );
                snprintf( vlan_section, sizeof(vlan_section), "vlan%s", vlan_id );
                vlan_cfg = util_get_vltree_node( &cm_vlan_vltree, VLTREE_CM_TREE, vlan_section );
                if ( !vlan_cfg ) {
                    cfmanager_log_message( L_ERR,
                        "Can not find vlan section '%s'!\n", vlan_section );
                    return ERRCODE_PARAMETER_ERROR;
                }
                else {
                    struct blob_attr *vlan_tb[__VLAN_MAX];
                    int dhcp_enable = 0;

                    blobmsg_parse( vlan_policy,
                        __VLAN_MAX,
                        vlan_tb,
                        blobmsg_data(vlan_cfg->cf_section.config),
                        blobmsg_len(vlan_cfg->cf_section.config) );

                    if ( !vlan_tb[VLAN_ENABLE] || !vlan_tb[VLAN_DHCP_ENABLE] || !vlan_tb[VLAN_DHCP_IPRANGE] ) {
                        cfmanager_log_message( L_ERR,
                            "The vlan '%s' lacks parameters '%s' or '%s' or '%s'!\n", 
                            vlan_section,
                            vlan_policy[VLAN_ENABLE].name,
                            vlan_policy[VLAN_DHCP_ENABLE].name,
                            vlan_policy[VLAN_DHCP_IPRANGE].name);
                        return ERRCODE_PARAMETER_ERROR;
                    };
                    
                    if ( !blobmsg_get_bool(vlan_tb[VLAN_ENABLE]) ) {
                        cfmanager_log_message( L_ERR,
                            "The vlan '%s' didn't enable!\n", vlan_section );
                        return ERRCODE_PARAMETER_ERROR;
                    }

                    if ( !blobmsg_get_bool(vlan_tb[VLAN_DHCP_ENABLE]) ) {
                        cfmanager_log_message( L_ERR,
                            "The vlan '%s' didn't enable DHCP!\n", vlan_section );
                        return ERRCODE_PARAMETER_ERROR;
                    }

                    ip_range = util_blobmsg_get_string( vlan_tb[VLAN_DHCP_IPRANGE], "" );
                    if ( vpn_is_user_or_ip_range_existed(&ip_range_list, ip_range) ) {
                        cfmanager_log_message( L_ERR, "The ip range '%s' was overlaped!\n", ip_range );
                        return ERRCODE_PARAMETER_ERROR;
                    }
                }
#endif                
            }
            else { // IPSEC_DIAL_IN_USER_ACTION_DELETE
                if ( !user_tb[IPSEC_DIAL_IN_USER_ATTR_ID] ) {
                    cfmanager_log_message( L_ERR,
                        "Missing parameters '%s' !\n", 
                        ipsec_dial_in_user_policy[IPSEC_DIAL_IN_USER_ATTR_ID].name );
                    return ERRCODE_PARAMETER_ERROR;
                }
            }
        }
    }
    
    if ( attr[VPN_SERVER_IPSEC_CMN_SETTING] ) {
        struct blob_attr *setting_tb[__IPSEC_CMN_SETTING_ATTR_MAX];

        blobmsg_parse( ipsec_cmn_setting_policy,
            __IPSEC_CMN_SETTING_ATTR_MAX,
            setting_tb,
            blobmsg_data(attr[VPN_SERVER_IPSEC_CMN_SETTING]),
            blobmsg_len(attr[VPN_SERVER_IPSEC_CMN_SETTING]) );

        for ( i = 0; i < __IPSEC_CMN_SETTING_ATTR_MAX; i++ ) {
            if ( (((uint64_t)1 << i) & ipsec_cmn_setting_required_paras) && 
                (!setting_tb[i] || blobmsg_type(setting_tb[i]) != ipsec_cmn_setting_policy[i].type) ) {
                cfmanager_log_message( L_ERR,
                    "Missing parameters '%s' or data type error!\n", ipsec_cmn_setting_policy[i].name );
                return ERRCODE_PARAMETER_ERROR;
            }
        }

        blobmsg_for_each_attr( cur, setting_tb[IPSEC_CMN_SETTING_ATTR_ENCRYPT_ALG], rem ) {
            char *encrypt_alg = blobmsg_get_string( cur );

            if ( !vpn_ike_alg_match( encrypt_alg, IKE_ALG_TYPE_ENCRYPT ) ) {
                cfmanager_log_message( L_ERR,
                    "Don't support encrypt algorithm '%s'!\n", encrypt_alg );
                return ERRCODE_PARAMETER_ERROR;
            }
        }

        blobmsg_for_each_attr( cur, setting_tb[IPSEC_CMN_SETTING_ATTR_AUTH_ALG], rem ) {
            char *auth_alg = blobmsg_get_string( cur );

            if ( !vpn_ike_alg_match( auth_alg, IKE_ALG_TYPE_AUTH ) ) {
                cfmanager_log_message( L_ERR,
                    "Don't support encrypt algorithm '%s'!\n", auth_alg );
                return ERRCODE_PARAMETER_ERROR;
            }
        }

        blobmsg_for_each_attr( cur, setting_tb[IPSEC_CMN_SETTING_ATTR_DH], rem ) {
            char *dh = blobmsg_get_string( cur );

            if ( !vpn_ike_alg_match( dh, IKE_ALG_TYPE_DH ) ) {
                cfmanager_log_message( L_ERR,
                    "Don't support encrypt algorithm '%s'!\n", dh );
                return ERRCODE_PARAMETER_ERROR;
            }
        }
    }

    return ERRCODE_SUCCESS;
}

//==============================================================================
static int
vpn_del_ipsec_cmn_setting(
    char *section_nm
)
//==============================================================================
{
    struct cm_config *cm_cfg = NULL;

    cm_cfg = util_get_vltree_node( &cm_ipsec_cmn_setting_vltree, VLTREE_CM_TREE, section_nm );
    if ( cm_cfg ) {
        cfmanager_log_message( L_WARNING, "Delete ipsec common setting '%s'!\n", section_nm );
        config_del_named_section( CFMANAGER_CONFIG_NAME, "ipsec_cmn_setting", section_nm );
    }
    else {
        cfmanager_log_message( L_ERR, "Cannot find ipsec common setting '%s'!\n", section_nm );
    }

    return 0;
}

//==============================================================================
static int
vpn_add_or_update_ipsec_cmn_setting(
    struct blob_attr *attr,
    char *section_nm,
    int *changed
)
//==============================================================================
{
    struct cm_config *cm_cfg = NULL; 
    char path[LOOKUP_STR_SIZE] = { 0 };
    struct blob_attr *new_cmn_set_tb[__IPSEC_CMN_SETTING_ATTR_MAX];
    struct blob_attr *old_cmn_set_tb[__IPSEC_CMN_SETTING_ATTR_MAX];
    int i;
    int ret;

    blobmsg_parse( ipsec_cmn_setting_policy,
        __IPSEC_CMN_SETTING_ATTR_MAX,
        new_cmn_set_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    *changed = 0;
    cm_cfg = util_get_vltree_node( &cm_ipsec_cmn_setting_vltree, VLTREE_CM_TREE, section_nm ); 
    if ( !cm_cfg ) {
        cfmanager_log_message( L_WARNING, "NOT found ipsec common setting '%s' information in %s, create it...\n", 
            section_nm, CFMANAGER_CONFIG_NAME );

        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "ipsec_cmn_setting", section_nm );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section '%s' in %s\n", section_nm, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __IPSEC_CMN_SETTING_ATTR_MAX; i++ ) {
            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, section_nm, ipsec_cmn_setting_policy[i].name );

            if ( ipsec_cmn_setting_policy[i].type == BLOBMSG_TYPE_ARRAY ) {
                struct blob_attr *cur;
                int rem;

                blobmsg_for_each_attr( cur, new_cmn_set_tb[i], rem ) {
                    config_uci_add_list( path, blobmsg_get_string(cur), 0 );
                }
            }
            else {
                config_set_by_blob( new_cmn_set_tb[i], path, ipsec_cmn_setting_policy[i].type );
            }
        }

        *changed = 1;
    }
    else {
        cfmanager_log_message( L_WARNING, "Update ipsec common setting section '%s' in %s\n", 
            section_nm, CFMANAGER_CONFIG_NAME );

        blobmsg_parse( ipsec_cmn_setting_policy,
            __IPSEC_CMN_SETTING_ATTR_MAX,
            old_cmn_set_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __IPSEC_CMN_SETTING_ATTR_MAX; i++ ) {
            if ( !new_cmn_set_tb[i] ) {
                continue;
            }

            if ( sgreq_compar_attr( new_cmn_set_tb[i], old_cmn_set_tb[i], ipsec_cmn_setting_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, section_nm, ipsec_cmn_setting_policy[i].name );
            if ( ipsec_cmn_setting_policy[i].type == BLOBMSG_TYPE_ARRAY ) {
                struct blob_attr *cur;
                int rem;

                config_uci_del( path, 0 );
                blobmsg_for_each_attr( cur, new_cmn_set_tb[i], rem ) {
                    config_uci_add_list( path, blobmsg_get_string(cur), 0 );
                }
            }
            else {
                config_set_by_blob( new_cmn_set_tb[i], path, ipsec_cmn_setting_policy[i].type );
            }

            *changed = 1;
        }
    }

    return ERRCODE_SUCCESS;
}

//==============================================================================
static int
vpn_del_ipsec_dial_in_user(
    char *section_nm
)
//==============================================================================
{
    struct cm_config *cm_cfg = NULL;

    cm_cfg = util_get_vltree_node( &cm_ipsec_dial_in_user_vltree, VLTREE_CM_TREE, section_nm );
    if ( cm_cfg ) {
        cfmanager_log_message( L_WARNING, "Delete ipsec dial in user '%s'!\n", section_nm );
        config_del_named_section( CFMANAGER_CONFIG_NAME, "ipsec_dial_in_user", section_nm );
    }
    else {
        cfmanager_log_message( L_ERR, "Cannot find ipsec dial in user '%s'!\n", section_nm );
    }

    return 0;
}

//==============================================================================
static int
vpn_add_or_update_ipsec_dial_in_user(
    struct blob_attr *attr,
    char *section_nm,
    int *changed
)
//==============================================================================
{
    struct cm_config *cm_cfg = NULL; 
    char path[LOOKUP_STR_SIZE] = { 0 };
    struct blob_attr *new_user_tb[__IPSEC_DIAL_IN_USER_ATTR_MAX];
    struct blob_attr *old_user_tb[__IPSEC_DIAL_IN_USER_ATTR_MAX];
    int i;
    int ret;
    int user_action;

    blobmsg_parse( ipsec_dial_in_user_policy,
        __IPSEC_DIAL_IN_USER_ATTR_MAX,
        new_user_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    user_action = util_blobmsg_get_int( new_user_tb[IPSEC_DIAL_IN_USER_ATTR_ACTION], 
        IPSEC_DIAL_IN_USER_ACTION_ADD );
    if ( user_action != IPSEC_DIAL_IN_USER_ACTION_ADD ) {
        cfmanager_log_message( L_ERR,
            "We want to add/update ipsec dial in user section, but action wasn't 'add', ignore it...\n" );
        return ERRCODE_SUCCESS;
    }

    *changed = 0;
    cm_cfg = util_get_vltree_node( &cm_ipsec_dial_in_user_vltree, VLTREE_CM_TREE, section_nm ); 
    if ( !cm_cfg ) {
        cfmanager_log_message( L_WARNING, "NOT found ipsec dial in user section '%s' information in %s, create it...\n", 
            section_nm, CFMANAGER_CONFIG_NAME );

        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "ipsec_dial_in_user", section_nm );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section '%s' in %s\n", section_nm, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __IPSEC_DIAL_IN_USER_ATTR_MAX; i++ ) {
            if ( i != IPSEC_DIAL_IN_USER_ATTR_ACTION && new_user_tb[i] ) {
                snprintf( path, sizeof( path ), "%s.%s.%s",
                    CFMANAGER_CONFIG_NAME, section_nm, ipsec_dial_in_user_policy[i].name );
                config_set_by_blob( new_user_tb[i], path, ipsec_dial_in_user_policy[i].type );
            }
        }

        *changed = 1;
    }
    else {
        cfmanager_log_message( L_WARNING, "Update ipsec dial in user section '%s' in %s\n", 
            section_nm, CFMANAGER_CONFIG_NAME );

        blobmsg_parse( ipsec_dial_in_user_policy,
            __IPSEC_DIAL_IN_USER_ATTR_MAX,
            old_user_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __IPSEC_DIAL_IN_USER_ATTR_MAX; i++ ) {
            if ( !new_user_tb[i] || i == IPSEC_DIAL_IN_USER_ATTR_ACTION ) {
                continue;
            }

            if ( sgreq_compar_attr( new_user_tb[i], old_user_tb[i], ipsec_dial_in_user_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, section_nm, ipsec_dial_in_user_policy[i].name );
            config_set_by_blob( new_user_tb[i], path, ipsec_dial_in_user_policy[i].type );
            *changed = 1;
        }
    }

    return ERRCODE_SUCCESS;
}

//==============================================================================
static int
vpn_add_or_update_ipsec_server(
    struct blob_attr **new_server_tb,
    int *fw_need_load,
    int *mwan3_need_load
)
//==============================================================================
{
    char server_section[BUF_LEN_32] = { 0 };
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *old_server_tb[__VPN_SERVER_MAX];
    char path[LOOKUP_STR_SIZE] = { 0 };
    uint64_t change_option = 0;
    int server_id;
    int ret;
    int same_cfg = 1;
    int i;

    server_id = util_blobmsg_get_int(new_server_tb[VPN_SERVER_ID], 0);
    if ( server_id >= VPN_SERVER_CNT_MAX ) {
        cfmanager_log_message( L_ERR, "server id %d is too large!\n", server_id );
        return ERRCODE_INTERNAL_ERROR;
    };
    
    snprintf( server_section, sizeof(server_section), "vpn_server%d", server_id );
    cm_cfg = util_get_vltree_node( &cm_vpn_server_vltree, VLTREE_CM_TREE, server_section );
    if ( !cm_cfg ) {
        int changed = 0;

        cfmanager_log_message( L_WARNING, "NOT found vpn server %s information in %s, create it...\n", 
            server_section, CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "vpn_server", server_section );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section %s in %s\n", server_section, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __VPN_SERVER_MAX; i++ ) {
            // First, write required paras.
            if ( ((uint64_t)1 << i) & server_type_required_paras[VPN_SERVER_TYPE_IPSEC] ) {
                snprintf( path, sizeof( path ), "%s.%s.%s",
                    CFMANAGER_CONFIG_NAME, server_section, sgreq_vpn_server_policy[i].name );
                config_set_by_blob( new_server_tb[i], path, sgreq_vpn_server_policy[i].type );
            }
        }

        if ( new_server_tb[VPN_SERVER_IPSEC_CMN_SETTING] ) {
            char section_nm[BUF_LEN_64] = { 0 };

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, server_section, 
                sgreq_vpn_server_policy[VPN_SERVER_IPSEC_CMN_SETTING].name );
            snprintf( section_nm, sizeof(section_nm), "ipsec_cmn_setting%d", server_id );
            config_uci_set( path, section_nm, 0 );
            vpn_add_or_update_ipsec_cmn_setting( new_server_tb[VPN_SERVER_IPSEC_CMN_SETTING],
                section_nm, &changed );
        }

        if ( new_server_tb[VPN_SERVER_IPSEC_DIAL_IN_USER] ) {
            struct blob_attr *cur;
            int rem;
            char section_nm[BUF_LEN_64] = { 0 };

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, server_section, 
                sgreq_vpn_server_policy[VPN_SERVER_IPSEC_DIAL_IN_USER].name );

            blobmsg_for_each_attr( cur, new_server_tb[VPN_SERVER_IPSEC_DIAL_IN_USER], rem ) {
                struct blob_attr *user_tb[__IPSEC_DIAL_IN_USER_ATTR_MAX];

                blobmsg_parse( ipsec_dial_in_user_policy,
                    __IPSEC_DIAL_IN_USER_ATTR_MAX,
                    user_tb,
                    blobmsg_data( cur ),
                    blobmsg_len( cur ) );

                snprintf( section_nm, sizeof(section_nm), "vpn_server%d_dial_in_user%s", 
                    server_id,
                    blobmsg_get_string(user_tb[IPSEC_DIAL_IN_USER_ATTR_ID]) );
                config_uci_add_list( path, section_nm, 0 );
                vpn_add_or_update_ipsec_dial_in_user( cur, section_nm, &changed );
            }
        }

        same_cfg = 0;
        *fw_need_load = 1;
        *mwan3_need_load = 1;
    }
    else {
        cfmanager_log_message( L_WARNING, "Update vpn server %s in %s\n", 
            server_section, CFMANAGER_CONFIG_NAME );

        blobmsg_parse( vpn_server_policy,
            __VPN_SERVER_MAX,
            old_server_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __VPN_SERVER_MAX; i++ ) {
            if ( !new_server_tb[i] ) {
                continue;
            }

            if ( ((uint64_t)1 << i) & server_type_required_paras[VPN_SERVER_TYPE_IPSEC] ) {
                if ( sgreq_compar_attr( new_server_tb[i], old_server_tb[i], vpn_server_policy[i].type ) ) {
                    continue;
                }

                snprintf( path, sizeof( path ), "%s.%s.%s",
                    CFMANAGER_CONFIG_NAME, server_section, vpn_server_policy[i].name );
                config_set_by_blob( new_server_tb[i], path, vpn_server_policy[i].type );
                same_cfg = 0;
                change_option |= BIT(i);
            }
        }

        // If ipsec common setting changed, handler it.
        if ( new_server_tb[VPN_SERVER_IPSEC_CMN_SETTING] ) {
            char *section_nm = blobmsg_get_string( old_server_tb[VPN_SERVER_IPSEC_CMN_SETTING] );
            int changed = 0;

            cfmanager_log_message( L_WARNING, "Update '%s' in %s\n", 
                section_nm, CFMANAGER_CONFIG_NAME );
            vpn_add_or_update_ipsec_cmn_setting( new_server_tb[VPN_SERVER_IPSEC_CMN_SETTING],
                section_nm, &changed );
            if ( changed ) {
                same_cfg = 0;
                change_option |= BIT(VPN_SERVER_IPSEC_CMN_SETTING);
            }
        }

        // If ipsec dial-in user list changed, handler it.
        if ( new_server_tb[VPN_SERVER_IPSEC_DIAL_IN_USER] ) {
            struct blob_attr *new_cur, *old_cur;
            int new_rem, old_rem;
            char section_nm[BUF_LEN_64] = { 0 };
            int changed = 0;

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, server_section, 
                sgreq_vpn_server_policy[VPN_SERVER_IPSEC_DIAL_IN_USER].name );

            // Find old 'ipsecDialInUser' config in section 'vpn_serverx' according to new config.
            // And according to new 'action' to handler it further.
            blobmsg_for_each_attr( new_cur, new_server_tb[VPN_SERVER_IPSEC_DIAL_IN_USER], new_rem ) {
                struct blob_attr *new_user_tb[__IPSEC_DIAL_IN_USER_ATTR_MAX];
                int found = 0;
                int user_action;
                
                blobmsg_parse( ipsec_dial_in_user_policy,
                    __IPSEC_DIAL_IN_USER_ATTR_MAX,
                    new_user_tb,
                    blobmsg_data( new_cur ),
                    blobmsg_len( new_cur ) );

                changed = 0;
                snprintf( section_nm, sizeof(section_nm), "vpn_server%d_dial_in_user%s", 
                    server_id,
                    blobmsg_get_string(new_user_tb[IPSEC_DIAL_IN_USER_ATTR_ID]) );
                user_action = util_blobmsg_get_int( new_user_tb[IPSEC_DIAL_IN_USER_ATTR_ACTION],
                    IPSEC_DIAL_IN_USER_ACTION_ADD);

                blobmsg_for_each_attr( old_cur, old_server_tb[VPN_SERVER_IPSEC_DIAL_IN_USER], old_rem ) {
                    if ( !strcmp(section_nm, blobmsg_get_string(old_cur)) ) {
                        found = 1;
                        break;
                    }
                }

                if ( found && user_action == IPSEC_DIAL_IN_USER_ACTION_ADD ) {
                    cfmanager_log_message( L_WARNING, "Update '%s' in %s\n", 
                        section_nm, CFMANAGER_CONFIG_NAME );
                    vpn_add_or_update_ipsec_dial_in_user( new_cur, section_nm, &changed );
                }
                else if ( found ) { // user_action == IPSEC_DIAL_IN_USER_ACTION_DELETE
                    cfmanager_log_message( L_WARNING, "Delete '%s' in %s\n", 
                        section_nm, CFMANAGER_CONFIG_NAME );
                    vpn_del_ipsec_dial_in_user( section_nm );
                    config_uci_del_list( path, section_nm, 0 );
                    changed = 1;
                }
                else if ( user_action == IPSEC_DIAL_IN_USER_ACTION_ADD ) { // Not found
                    cfmanager_log_message( L_WARNING, "Add '%s' in %s\n", 
                        section_nm, CFMANAGER_CONFIG_NAME );
                    vpn_add_or_update_ipsec_dial_in_user( new_cur, section_nm, &changed );
                    config_uci_add_list( path, section_nm, 0 );
                }
                else {
                    cfmanager_log_message( L_ERR, "Delete a non-exist section '%s' in %s\n", 
                        section_nm, CFMANAGER_CONFIG_NAME );
                    continue;
                }

                if ( changed ) {
                    same_cfg = 0;
                    change_option |= BIT(VPN_SERVER_IPSEC_DIAL_IN_USER);
                }
            }
        }

        if ( change_option && (change_option & ~(BIT(VPN_SERVER_NAME) | BIT(VPN_SERVER_IPSEC_DIAL_IN_USER))) ) {
            // If only name or user list changed, we don't need to reload firewall
            *fw_need_load = 1;
            *mwan3_need_load = 1;
        }
    }

    if ( same_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "The vpn server '%s' config is same\n", server_section );
    }

    return ret;
}

//==============================================================================
static int
vpn_do_del_ipsec_server(
    char *section_nm
)
//==============================================================================
{
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *server_tb[__VPN_SERVER_MAX];

    cm_cfg = util_get_vltree_node( &cm_vpn_server_vltree, VLTREE_CM_TREE, section_nm );
    if ( !cm_cfg ) {
        cfmanager_log_message( L_ERR,
            "Can't find ipsec server section '%s' in %s\n", section_nm, CFMANAGER_CONFIG_NAME );
        return -1;
    }
    else {
        struct blob_attr *cur;
        int rem;

        blobmsg_parse( vpn_server_policy,
            __VPN_SERVER_MAX,
            server_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        vpn_del_ipsec_cmn_setting( blobmsg_get_string(server_tb[VPN_SERVER_IPSEC_CMN_SETTING]) );

        blobmsg_for_each_attr( cur, server_tb[VPN_SERVER_IPSEC_DIAL_IN_USER], rem ) {
            vpn_del_ipsec_dial_in_user( blobmsg_get_string(cur) );
        }
    }

    cfmanager_log_message( L_WARNING, "Delete ipsec vpn server '%s'!\n", section_nm );
    config_del_named_section( CFMANAGER_CONFIG_NAME, "vpn_server", section_nm );

    return 0;
}

//==============================================================================
static int
vpn_del_ipsec_server(
    struct blob_attr **new_server_tb,
    int *fw_need_load,
    int *mwan3_need_load
)
//==============================================================================
{
    char server_section[BUF_LEN_32] = { 0 };
    char *server_id;

    server_id = blobmsg_get_string( new_server_tb[VPN_SERVER_ID] );
    snprintf( server_section, sizeof(server_section), "vpn_server%s", server_id );
    vpn_do_del_ipsec_server( server_section );

    *fw_need_load= 1;
    *mwan3_need_load = 1;

    return 0;
}

//==============================================================================
int
vpn_del_server_by_wan(
    char *wan
)
//==============================================================================
{
    struct cm_config *cm_cfg = NULL;
    int type;

    vlist_for_each_element( &cm_vpn_server_vltree, cm_cfg, node ) {
        struct blob_attr *tb[__VPN_SERVER_MAX] = { 0 };

        blobmsg_parse( vpn_server_policy,
            __VPN_SERVER_MAX,
            tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        if ( !strcmp( wan, blobmsg_get_string(tb[VPN_SERVER_WAN_INTF]) ) ) {
            type = util_blobmsg_get_int( tb[VPN_SERVER_TYPE], VPN_SERVER_TYPE_IPSEC );
            if ( type == VPN_SERVER_TYPE_IPSEC ) {
                vpn_do_del_ipsec_server( cm_cfg->cf_section.name );
            }
            break;
        }
    }

    return 0;
}

//==============================================================================
int
vpn_parse_server_list(
    struct blob_attr *attr,
    int *fw_need_load,
    int *mwan3_need_load
)
//==============================================================================
{
    struct blob_attr *new_tb[__VPN_SERVER_MAX];
    int server_type;
    int server_action;
    int ret;

    blobmsg_parse( sgreq_vpn_server_policy,
        __VPN_SERVER_MAX,
        new_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    if ( !new_tb[VPN_SERVER_ACTION] ) {
        cfmanager_log_message( L_ERR,
            "Missing parameters '%s' for vpn server!\n", sgreq_vpn_server_policy[VPN_SERVER_ACTION].name );
        return ERRCODE_PARAMETER_ERROR;
    };

    server_action = util_blobmsg_get_int( new_tb[VPN_SERVER_ACTION], VPN_SERVER_ACTION_ADD );
    if ( server_action >= __VPN_SERVER_ACTION_MAX ) {
        cfmanager_log_message( L_ERR,
            "Don't support vpn server action %d!\n", server_action );
        return ERRCODE_PARAMETER_ERROR;
    }

    if ( !new_tb[VPN_SERVER_TYPE] ) {
        cfmanager_log_message( L_ERR,
            "Missing parameters '%s' for vpn server!\n", sgreq_vpn_server_policy[VPN_SERVER_TYPE].name );
        return ERRCODE_PARAMETER_ERROR;
    }

    server_type = util_blobmsg_get_int( new_tb[VPN_SERVER_TYPE], VPN_SERVER_TYPE_IPSEC );
    if ( server_type >= __VPN_SERVER_TYPE_MAX ) {
        cfmanager_log_message( L_ERR,
            "Don't support vpn server type %d!\n", server_type );
        return ERRCODE_PARAMETER_ERROR;
    }

    if ( server_action == VPN_SERVER_ACTION_ADD ) {
        if ( server_type == VPN_SERVER_TYPE_IPSEC ) {
            ret = vpn_verify_ipsec_server_config( new_tb );
            if ( ret != ERRCODE_SUCCESS ) {
                return ret;
            }
            else {
                vpn_add_or_update_ipsec_server( new_tb, fw_need_load, mwan3_need_load );
            }
        }
        //else {}
    }
    else { // VPN_SERVER_ACTION_DELETE
        if ( !new_tb[VPN_SERVER_ID] ) {
            cfmanager_log_message( L_ERR,
                "Missing parameters '%s' for vpn server!\n", sgreq_vpn_server_policy[VPN_SERVER_ID].name );
            return ERRCODE_PARAMETER_ERROR;
        }

        if ( server_type == VPN_SERVER_TYPE_IPSEC ) {
            vpn_del_ipsec_server( new_tb, fw_need_load, mwan3_need_load );
        }
        //else {}
    }

    if ( user_list ) {
        free( user_list );
        user_list = NULL;
    }

    if ( ip_range_list ) {
        free( ip_range_list );
        ip_range_list = NULL;
    }

    return ERRCODE_SUCCESS;
}

/* EOF */

