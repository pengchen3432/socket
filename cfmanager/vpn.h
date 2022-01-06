/****************************************************************************
*
* FILENAME:        vpn.h
*
* DESCRIPTION:     Description of this header file's contents
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

#ifndef __VPN_H__
#define __VPN_H__

//===========================
// Includes
//===========================
#include "global.h"

//===========================
// Defines
//===========================
#define IPSEC_RELOAD_HELPER_SCRIPT "/usr/libexec/ipsec/ipsec_reload_helper.sh"
#define TEMP_IPSEC_RELOAD_HELPER_SCRIPT "/tmp/ipsec_reload_helper.sh"

//===========================
// Typedefs
//===========================
enum {
    IKE_VERSION_INVALID,
    IKE_VERSION_1,
    IKE_VERSION_2,

    __IKE_VERSION_MAX,
};

enum {
    IKE1_MODE_MAIN,
    IKE1_MODE_AGGRESSIVE,

    __IKE1_MODE_MAX,
};

enum {
    IKE_ALG_TYPE_ENCRYPT,
    IKE_ALG_TYPE_AUTH,
    IKE_ALG_TYPE_DH,

    __IKE_ALG_TYPE_MAX,
};

enum {
    IKE_AUTH_METHOD_PSK,

    __IKE_AUTH_METHOD_MAX,
};

enum {
    IPSEC_RELOAD_ACTION_NONE,
    IPSEC_RELOAD_ACTION_START,
    IPSEC_RELOAD_ACTION_STOP,
    IPSEC_RELOAD_ACTION_RESTART,
    IPSEC_RELOAD_ACTION_ADD_CONN,
    IPSEC_RELOAD_ACTION_DEL_CONN,
    IPSEC_RELOAD_ACTION_REPLACE_CONN,
    IPSEC_RELOAD_ACTION_REPLACE_AND_UP_CONN,

    __IPSEC_RELOAD_ACTION_MAX
};

enum {
    IPSEC_STATUS_NONE,
    IPSEC_STATUS_ENABLE,
    IPSEC_STATUS_DISABLE,

    __IPSEC_STATUS_MAX
};

enum {
    VPN_ROLE_NONE,
    VPN_ROLE_CLIENT,
    VPN_ROLE_SERVER,

    __VPN_ROLE_MAX
};

#define IKE_ALG_CNT_MAX 32

//===========================
// Globals
//===========================
extern char* support_ike_alg[__IKE_ALG_TYPE_MAX][IKE_ALG_CNT_MAX];
extern int32_t ipsec_reload_action[WAN_CNT_MAX];
extern int32_t ipsec_status[WAN_CNT_MAX];
extern char *ipsec_action2str[__IPSEC_RELOAD_ACTION_MAX];
extern uint32_t ipsec_vpn_cnt[WAN_CNT_MAX][__VPN_ROLE_MAX];
extern int32_t ipsec_final_reload_action;
extern uchar vpn_user_handle_done[VPN_SERVER_CNT_MAX];

//===========================
// Functions
//===========================
int
vpn_aync_reload_ipsec(
    void
);

int
vpn_ctrl_ipsec_status(
    char *ipsec_conn,
    int action
);

int
vpn_get_ipsec_status(
    char *remote_gw
);

int
vpn_ike_alg_match(
    char *alg,
    int type    
);

int
vpn_del_client(
    char *section_nm
);

int
vpn_del_client_by_wan(
    char *wan
);

int
vpn_del_server_by_wan(
    char *wan
);

int
vpn_del_vpn_split_by_client(
    char *id
);

int
vpn_parse_client_list(
    struct blob_attr *attr,
    int *fw_need_load,
    int *mwan3_need_load
);

int
vpn_parse_split_list(
    struct blob_attr *attr,
    int *mwan3_need_load
);

int
vpn_parse_server_list(
    struct blob_attr *attr,
    int *fw_need_load,
    int *mwan3_need_load
);

void
vpn_prepare_reload_ipsec(
    void
);

int
vpn_update_ipsec_reload_helper(
    int action,
    char *ipsec_conn
);

int
vpn_update_ipsec_cnt_and_reload_helper(
    int wan_index,
    int role,
    int status,
    char *ipsec_conn
);


#endif
/* EOF */

