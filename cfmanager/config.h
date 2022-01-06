/****************************************************************************
* *
* * FILENAME:        $RCSfile: config.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/27
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
#ifndef __CONFIG_H__
#define __CONFIG_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================
#define WAN0_VPN_SECTION_NAME       "vpn0"
#define WAN1_VPN_SECTION_NAME       "vpn1"
#define MASTER_BWCTRL_CM_ID         "master"
#define GUEST_BWCTRL_CM_ID          "guest"

#ifdef CONFIG_MTK
#define AP_LAN_IFNAME               "eth0"
#else
#define AP_LAN_IFNAME               "eth0 eth1 eth2 eth3 eth4"
#endif

#ifdef SINGLE_WAN
#define ROUTER_SINGLEWAN_LAN_IFNAME "eth0"
#define WAN_IFNAME                  "eth1"
#else //DOUBLE_WAN
#define ROUTER_SINGLEWAN_LAN_IFNAME "eth0 eth1 eth2 eth3"
#define ROUTER_DOUBWAN_LAN_IFNAME   "eth0 eth1 eth2"
#define WAN_IFNAME                  "eth4"
#define WAN1_IFNAME                 "eth3"
#endif

#define LAN_DEFAULT_VLAN_ID         1
#define LAN_DEFAULT_INTERFACE       "lan0_zone0"
#define WAN_MAC_CUSTOM_SECTION      "base_wan0"
#define WAN1_MAC_CUSTOM_SECTION     "base_wan1"
#define CLIENT_LIMIT_PREFIX         "client_limit"
#define PORTAL_POLICY_PREFIX        "portal_policy"
#define CM_FOLDER                   "/tmp/.cfmanager"
#define CM_ABSOLUTE_PATH            "/etc/config/cfmanager"
#define CM_CONFIG_PATH              SPLICE_STR(CM_FOLDER,/config)
#define UCI_DEFAULT_PATH            "/etc/config"
#define CM_WAN_ALIAS_SECTION_TYPE   "wan_alias"
#define CM_WAN0_ALIAS_SECTION_NAME  "wan0_alias"
#define CM_WAN1_ALIAS_SECTION_NAME  "wan1_alias"
#define WAN0_ALIAS_SECTION_PREFIX   "wan0_alias_"
#define WAN1_ALIAS_SECTION_PREFIX   "wan1_alias_"
#define SSID_COUNT_MAX              16
#define MASTER_SSID_ID              "ssid0"
#define GUEST_SSID_ID               "ssid1"
#define MESH_SSID_ID                "mesh"
#define WAN_ALIAS_COUNT_MAX         8
#define LOOKUP_STR_SIZE             65
#define HASH_SIZE                   65
#define SSID_PASSWD_MAX_LEN         32
#define SSID_NAME_MAX_LEN           32
#define MACFILTER_MAX_LEN           8
#define IP_RANGE_STR_MAX_LEN        7
#define LAN_DEFAULT_LEASE_MIN       12*60
#define UPGRADE_TIME_STR_MAX_LEN    5
#define MIN_MTU                     576
#define MAX_MTU                     1500
#define DHCP_MTU_DEVALUE            1500
#define STATIC_MTU_DEVALUE          1500
#define PPPOE_MTU_DEVALUE           1492
#define PPTP_MTU_DEVALUE            1420
#define L2TP_MTU_DEVALUE            1460
#define VPN_COUNT_MAX               2   //At present, only two configurations are supported
#define GUEST_SSID_DEFAULT_ENCRYP   "psk+aes"
#define WAN_LINK_PORT               5
#define WAN1_LINK_PORT              4
#define POWER_LOW                   20
#define POWER_MEDIUM                24
#define POWER_HIGH                  30
#define POWER_LOW_STR               "20"
#define POWER_MEDIUM_STR            "24"
#define POWER_HIGH_STR              "30"
#define CM_STATIC_ROUTEV6_SECTION   "ipv6_static_route"
#define STATIC_ROUTEV6_NAME_PREFIX  "routev6_"
#define RADIO_NAME_PREFIX           "radio_"
#define AP_USE_RF_2G4CHANNELWIDTH   '3'
#define AP_USE_RF_2G4TXPOWER        '4'
#define AP_USE_RF_2G4RSSI           '2'
#define AP_USE_RF_2G4RATE           '2'
#define AP_USE_RF_5GCHANNELWIDTH    '4'
#define AP_USE_RF_5GTXPOWER         '4'
#define AP_USE_RF_5GRSSI            '2'
#define AP_USE_RF_5GRATE            '2'

//=================
//  Typedefs
//=================
enum {
    NET_MODE_MIN,
    NET_MODE_MASTER,
    NET_MODE_GUEST,

    __NET_MODE_MAX
};

enum {
    CFG_FIND_MIN,
    CFG_FIND_EXIST,
    CFG_FIND_NOT_EXIST,

    __CFG_FIND_MAX
};

enum {
    MODE_ROUTER,
    MODE_AP,

    __MODE_MAX
};

enum {
    SSID_BAND_DUAL_FREQ,
    SSID_BAND_2G4,
    SSID_BAND_5G,

    __SSID_BAND_MAX
};

typedef struct {
    struct vlist_node node;
    struct config_section_content cf_section;
} vtree_config_parse;

struct bwctrl_param {
    char *cm_id;
    char iface_2g[BUF_LEN_8];
    char iface_5g[BUF_LEN_8];
    int up_rate;
    int down_rate;
};

struct wireless_crypto_parameter {
    int radio;
    int crypto;
    int crypto_old;
    char ifname[BUF_LEN_8];
    char wpa_key_mode[BUF_LEN_8];
    char wpa_crypto_type[BUF_LEN_8];
    char password[SSID_PASSWD_MAX_LEN+1];
};

//=================
//  Globals
//=================

//=================
//  Locals
//=================

/*
 * Private Functions
 */

/*
 * Private Data
 */

//=================
//  Functions
//=================

int config_init(
    void
);

int
config_uci_set(
    char* path,
    char* value,
    int commit
);

int
config_set_option_not_exist(
    char* path,
    char* value,
    int commit
);

int
config_uci_del(
    char* path,
    int commit
);

int
config_uci_get_option(
    char* path,
    char* value,
    int size
);

int
config_get_uci_list(
    const char *package_name,
    const char *section_name,
    const char *option_name,
    struct blob_buf *b,
    const char *key,
    void (*cb)(struct blob_buf *b, const char *key, const char *value,const char *section_name)
);

int
config_set_section(
   const char *config,
   char *section,
   char *section_name
);

int
config_set_ssid_section(
    int network_type,
    int radio
);

int
config_uci_add_list(
    char *path,
    char *value,
    int commit
);

int
config_uci_del_list(
    char *path,
    char *value,
    int commit
);

int
config_commit(
    const char *package_name,
    int overwrite
);

int
config_add_named_section(
    const char *package_name,
    const char *section_type,
    const char *section_name
);

int
config_del_named_section(
    const char *package_name,
    const char *section_type,
    const char *section_name
);

int
config_set_schedule_section(
    void
);

int
config_create_ssid(
    char *ifname,
    int ifname_size,
    int radio,
    int network_type,
    char *ssid_id
);

void
config_edit_wan_type(
    struct blob_attr **new_config,
    int new_type,
    char *wan_name,
    int commit
);

void
config_del_wan_dns_info(
    const char *wan_name,
    int commit
);

void
config_set_wan_link_speed(
    const char *wan_name,
    int link
);

void
config_get_ifname(
    char *iface,
    int iface_size,
    int radio,
    int network_type,
    const char *ssid_id
);

void
config_set_channel_width(
    int channel_width,
    int channel_location,
    int radio
);

void
config_del_lan_dhcp(
    int commit
);

void
config_del_lan_dns(
    char *section_name,
    int commit
);

void
config_set_upgrade_auto_config(
    char *time,
    const char *sch_name,
    unsigned char sync_slave_enable
);

void
config_parse_ipv4_addr_attr(
    struct blob_attr *msg,
    char *mask_str,
    char *ip
);

void
config_parse_route_attr(
    struct blob_attr *msg,
    char *nexthop
);

int
config_set_by_blob(
    struct blob_attr *data,
    char *path,
    int blob_type
);

int
config_lookup_list(
    char *package_name,
    char *list_name
);

const char*
config_get_upgrade_auto_sch(
    void
);

int
config_creat_no_exist_section(
    const char *package_name,
    char *section_type,
    char *section_name,
    struct vlist_tree *vtree,
    int vtree_type
);

int
config_add_ap_default(
    char *package_name,
    char *mac,
    char *type,
    char *mesh
);

void
config_random_string(
    char* target,
    int size
);

int
config_set_wireless_limit(
    struct bwctrl_param *bwctrl
);

char*
config_get_iface_by_ssid_name(
    const char *ssid_name,
    int radio
);

int
config_set_client_limit_iface(
    char *path,
    int radio
);

int
config_get_mtu_defvalue(
    int proto
);

int
config_get_channel_width(
    int radio
);

void
config_del_all_iface(
    int radio
);

void
config_set_radio_enable(
    const char *country,
    const char *channel_with
);

void
config_sync(
    void
);

void
config_set_wan_alias_default_cfg(
    const char *section_name,
    int wan_type
);

int
config_vlan_is_enable(
    void
);

bool
config_wan1_is_enable(
    void
);

void
config_set_wireless(
    int option,
    char *iface,
    char *value,
    int radio
);

void
config_set_rssi(
    bool enable,
    char *rssi_value,
    int radio
);

void
config_set_radio_access_rate(
    bool enable,
    char *value,
    int radio
);

void
config_set_beacon_interval(
    char *value,
    int radio
);

int
config_del_iface_section(
    char *ifname
);

void
config_set_wireless_crypto(
    struct wireless_crypto_parameter *crypto_param
);

int
config_del_bwctrl_section(
    const char *cm_id
);

void
config_get_cm_ap_ssids(
    const char *ap_mac,
    char *buf,
    int buf_size
);

void
config_add_dev_ssid_id(
    char *mac,
    char *ssid_id
);

void
config_del_dev_ssid_id(
    char *mac,
    char *ssid_id
);

int
config_get_schedule_ssids(
    int schedule_id,
    char *schedule_ssids
);

void
config_set_txpower(
    int txpower,
    int radio,
    char *custom_txpower
);

bool
config_get_ssid_enable(
    const char *ssid_id
);

void
config_get_ssid_name(
    const char *ssid_id,
    char *buf,
    int buf_size
);


#endif //__CONFIG_H__
