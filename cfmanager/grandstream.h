/****************************************************************************
* *
* * FILENAME:        $RCSfile: grandstream.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/17
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
#ifndef __GRANDSTREAM_H__
#define __GRANDSTREAM_H__
//=================
//  Includes
//=================
#include "sgrequest.h"

//=================
//  Defines
//=================
#define CF_CONFIG_NAME_GRANDSTREAM      "grandstream"
#define IPV4_MAX_LENGTH                 16
#define GS_VERSION_NUM_MAXLEN           12
#define GS_PAIR_STATUS_MAXLEN           16
#define GS_RADIO_STRING_MAXLEN          4
#define GS_RSSI_STRING_MAXLEN           4
#define GS_UPGRADE_MAXLEN               2

//=================
//  Typedefs
//=================
struct grandstream_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
};

struct gs_device_info {
    char mac[MAC_STR_MAX_LEN];
    char ip4Address[IPV4_MAX_LENGTH];
    char systemVersion[GS_VERSION_NUM_MAXLEN];
    char status[GS_PAIR_STATUS_MAXLEN];
    char superior[MAC_STR_MAX_LEN];
    char radio[GS_RADIO_STRING_MAXLEN];
    char rssi[GS_RSSI_STRING_MAXLEN];
    char upgrade[GS_UPGRADE_MAXLEN];
    char newFirmwareVersion[GS_VERSION_NUM_MAXLEN];
};

enum {
    GS_ISSUE_AP_LIST,
    GS_EXTERN_LOG_URI,
    GS_EXTERN_LOG_LEVEL,
    GS_EXTERN_LOG_PROTOCOL,

    _GS_EXTERNAL_LOG_MAX
};

enum {
    GS_EMAIL_PORT,
    GS_EMAIL_HOST,
    GS_EMAIL_USER,
    GS_EMAIL_PASSWORD,
    GS_EMAIL_DO_NOT_VALIDATE,
    GS_EMAIL_ENABLE_NOTIFICATION,
    GS_EMAIL_FROM_ADDRESS,
    GS_EMAIL_FROM_NAME,
    GS_EMAIL_EMAILADDRESS,

    _GS_EMAIL_MAX
};

enum {
    GS_NOTIFY_MEMORY_USAGE,
    GS_MEMORY_USAGE_THRESHOLD,
    GS_NOTIFY_AP_THROUGHPUT,
    GS_AP_THROUGHPUT_THRESHOLD,
    GS_NOTIFY_SSID_THROUGHPUT,
    GS_SSID_THROUGHPUT_THRESHOLD,
    GS_NOTIFY_PASSWORD_CHANGE,
    GS_NOTIFY_FIRMWARE_UPGRADE,
    GS_NOTIFY_AP_OFFLINE,
    GS_NOTIFY_FIND_ROGUEAP,

    _GS_NOTIFY_MAX
};

enum {
    GS_GET_PAIRED_DEVICES_DEVTYPE,

    __GS_GET_PAIRED_DEVICES_MAX,
};

enum {
    GS_GET_DEV_TYPE_NONE,
    GS_GET_DEV_TYPE_AP,
    GS_GET_DEV_TYPE_MESH,

    __GS_GET_DEV_TYPE_MAX,
};

enum {
    GS_CONTROLLER_DEVICES_ATTR_LIST,

    __GS_CONTROLLER_DEVICES_ATTR_MAX
};

enum {
    GS_CONTROLLER_DEVICES_MAC,
    GS_CONTROLLER_DEVICES_IPV4,
    GS_CONTROLLER_DEVICES_VERSION,
    GS_CONTROLLER_DEVICES_STATUS,
    GS_CONTROLLER_DEVICES_SUPERIOR,
    GS_CONTROLLER_DEVICES_RADIO,
    GS_CONTROLLER_DEVICES_RSSI,

    __GS_CONTROLLER_DEVICES_INFO_MAX,
};


enum {
    GRANDSTREAM_HOOKER_NONE,
    GRANDSTREAM_HOOKER_LED,
    GRANDSTREAM_HOOKER_AP,
    GRANDSTREAM_HOOKER_GENERAL,
    GRANDSTREAM_HOOKER_SNMPD_CONFIG,
    GRANDSTREAM_HOOKER_SNMPD_PORTS,
    GRANDSTREAM_HOOKER_SNMPD_V3_AUTH,
    GRANDSTREAM_HOOKER_SSID,
    GRANDSTREAM_HOOKER_EXTERN_SYS_LOG,
    GRANDSTREAM_HOOKER_EMAIL,
    GRANDSTREAM_HOOKER_NOTIFY,
    GRANDSTREAM_HOOKER_RADIO,

    __CM_GRANDSTREAM_HOOKER_MAX,
};

enum {
    GRANDSTREAM_VLTREE_ALL,
    GRANDSTREAM_VLTREE_LED,
    GRANDSTREAM_VLTREE_AP,
    GRANDSTREAM_VLTREE_GENERAL,
    GRANDSTREAM_VLTREE_SNMPD_CONFIG,
    GRANDSTREAM_VLTREE_SNMPD_PORTS,
    GRANDSTREAM_VLTREE_SNMPD_V3_AUTH,
    GRANDSTREAM_VLTREE_SSID,
    GRANDSTREAM_VLTREE_EXTERN_SYS_LOG,
    GRANDSTREAM_VLTREE_EMAIL,
    GRANDSTREAM_VLTREE_NOTIFY,
    GRANDSTREAM_VLTREE_RADIO,

    __GRANDSTREAM_VLTREE_MAX
};

enum {
    LED_ON,

    __LED_MAX
};

enum {
    MANAGE_APS_ATTR_LIST,

    __MANAGE_APS_ATTR_MAX
};

enum {
    MANAGE_AP_ACTION,
    MANAGE_AP_MAC,
    MANAGE_AP_TYPE,
    MANAGE_AP_MESH,

    __MANAGE_AP_MAX
};

enum {
    GS_AP_MAC,
    GS_AP_TYPE,
    GS_AP_FREQUENCY,
    GS_AP_BAND_STEERING,
    GS_AP_ZONES,
    GS_AP_SSIDS,
    GS_AP_TLS1_2,
    GS_AP_NAME,
    GS_AP_NETPORT_TYPE,
    GS_AP_ATF_MODE,
    GS_AP_IPV4_ENABLE,
    GS_AP_IPV4_IP,
    GS_AP_IPV4_NETMASK,
    GS_AP_IPV4_GATEWAY,
    GS_AP_IPV4_FIRST_DNS,
    GS_AP_IPV4_SECOND_DNS,
    GS_AP_IPV6_ENABLE,
    GS_AP_IPV6_IP,
    GS_AP_IPV6_PREFIX_LENGTH,
    GS_AP_IPV6_GATEWAY,
    GS_AP_IPV6_FIRST_DNS,
    GS_AP_IPV6_SECOND_DNS,
    GS_AP_2G4_DISABLE,
    GS_AP_2G4_WIDTH,
    GS_AP_2G4_CHANNEL_LOCATION,
    GS_AP_2G4_CHANNEL,
    GS_AP_2G4_POWER,
    GS_AP_2G4_CUSTOM_TXPOWER,
    GS_AP_2G4_RSSI_TYPE,
    GS_AP_2G4_RSSI_THRESHOLD,
    GS_AP_2G4_MODE,
    GS_AP_2G4_RATE_LIMIT_TYPE,
    GS_AP_2G4_MINI_RATE,
    GS_AP_2G4_SHORTGI,
    GS_AP_2G4_ALLOW_LEGACY_DEV,
    GS_AP_5G_DISABLE,
    GS_AP_5G_WIDTH,
    GS_AP_5G_CHANNEL,
    GS_AP_5G_POWER,
    GS_AP_5G_CUSTOM_TXPOWER,
    GS_AP_5G_RSSI_TYPE,
    GS_AP_5G_RSSI_THRESHOLD,
    GS_AP_5G_MODE,
    GS_AP_5G_RATE_LIMIT_TYPE,
    GS_AP_5G_MINI_RATE,
    GS_AP_5G_SHORTGI,

    __GS_AP_MAX
};

enum {
    GENERAL_PAIRING_KEY,
    GENERAL_FAILOVER_KEY,
    GENERAL_ADMIN_PASSWORD,
    GENERAL_WEB_WAN_ACCESS,

    __GENERAL_MAX
};


enum {
    SNMP_CONFIG_ENABLE_V1_V2C,
    SNMP_CONFIG_ENABLE_V3,
    SNMP_CONFIG_SYS_LOCATION,
    SNMP_CONFIG_SYS_CONTACT,
    SNMP_CONFIG_SYS_NAME,
    SNMP_CONFIG_RO_COMMUNITYV4,
    SNMP_CONFIG_RW_COMMUNITYV4,
    SNMP_CONFIG_RO_COMMUNITYV6,
    SNMP_CONFIG_RW_COMMUNITYV6,
    SNMP_CONFIG_TRAP_TYPE,
    SNMP_CONFIG_TRAP_HOST,
    SNMP_CONFIG_TRAP_PORT,
    SNMP_CONFIG_TRAP_COMMUNITY,

    __SNMP_CONFIG_MAX
};

enum {
    SNMP_PORTS_ID,
    SNMP_PORTS_PORT,
    SNMP_PORTS_PROTOCOL,
    SNMP_PORTS_IPV4ADDRESS,

    __SNMP_PORTS_MAX
};

enum {
    SNMP_V3_ID,
    SNMP_V3_NAME,
    SNMP_V3_AUTH_TYPE,
    SNMP_V3_AUTH_PASS_PHRASE,
    SNMP_V3_PRIV_PROTO,
    SNMP_V3_PRIV_PASS_PHRASE,
    SNMP_V3_ACCESS_CTRL,

    __SNMP_V3_MAX
};

enum {
    GS_ADDIT_SSID_ID,
    GS_ADDIT_SSID_ENABLE,
    GS_ADDIT_SSID_BAND,
    GS_ADDIT_SSID_NAME,
    GS_ADDIT_SSID_CRYPTO,
    GS_ADDIT_SSID_WPA_KEY_MODE,
    GS_ADDIT_SSID_WPA_CRYPTO_TYPE,
    GS_ADDIT_SSID_PASSWORD,
    GS_ADDIT_SSID_SSIDHIDEENABLE,
    GS_ADDIT_SSID_ISOLATEMODE,
    GS_ADDIT_SSID_GATEWAYMAC,
    GS_ADDIT_SSID_DTIM_PERIOD,
    GS_ADDIT_SSID_CLIENT_LIMIT,
    GS_ADDIT_SSID_STA_IDLE_TIMEOUT,
    GS_ADDIT_SSID_UAPSD,
    GS_ADDIT_SSID_PROXY_ARP,
    GS_ADDIT_SSID_MCAST_TO_UCAST,
    GS_ADDIT_SSID_BMS,
    GS_ADDIT_SSID_BRIDGE_ENABLE,
    GS_ADDIT_SSID_80211W,
    GS_ADDIT_SSID_VLAN,

    __GS_ADDIT_SSID_MAX
};

enum {
    GS_RADIO_2G4CHANNEL,
    GS_RADIO_2G4CHANNELWIDTH,
    GS_RADIO_2G4CHANNEL_LOCATION,
    GS_RADIO_2G4TXPOWER,
    GS_RADIO_2G4CUSTOM_TXPOWER,
    GS_RADIO_2G4SHORTGI,
    GS_RADIO_2G4ALLOW_LEGACY_DEV,
    GS_RADIO_2G4RSSI_ENABLE,
    GS_RADIO_2G4RSSI_THRESHOLD,
    GS_RADIO_2G4RATE_LIMIT_ENABLE,
    GS_RADIO_2G4MINI_RATE,
    GS_RADIO_5GCHANNEL,
    GS_RADIO_5GCHANNELWIDTH,
    GS_RADIO_5GTXPOWER,
    GS_RADIO_5GCUSTOM_TXPOWER,
    GS_RADIO_5GSHORTGI,
    GS_RADIO_5GRSSI_ENABLE,
    GS_RADIO_5GRSSI_THRESHOLD,
    GS_RADIO_5GRATE_LIMIT_ENABLE,
    GS_RADIO_5GMINI_RATE,
    GS_RADIO_BEACON_INTERVAL,
    GS_RADIO_ATF_MODE,
    GS_RADIO_MUMIMOENABLE,
#if defined(GWN7062)
    GS_RADIO_COMPATIBILITYMODE,
#endif
    __GS_RADIO_MAX
};

//=================
//  Globals
//=================
extern struct vlist_tree grandstream_email_vltree;
extern struct vlist_tree grandstream_extern_sys_log_vltree;
extern struct vlist_tree grandstream_notification_vltree;
extern struct vlist_tree grandstream_led_vltree;
extern struct vlist_tree grandstream_ap_vltree;
extern struct vlist_tree grandstream_ssid_vltree;
extern struct vlist_tree grandstream_radio_vltree;
extern const struct blobmsg_policy gs_extern_sys_log_policy[_GS_EXTERNAL_LOG_MAX];
extern const struct blobmsg_policy gs_email_policy[_GS_EMAIL_MAX];
extern const struct blobmsg_policy gs_notification_policy[_GS_NOTIFY_MAX];
extern const struct blobmsg_policy grandstream_led_policy[__LED_MAX];
extern const struct blobmsg_policy gs_ap_policy[__GS_AP_MAX];
extern const struct blobmsg_policy manage_aps_attrs_list_policy[__MANAGE_APS_ATTR_MAX];
extern const struct blobmsg_policy general_policy[__GENERAL_MAX];
extern const struct blobmsg_policy grandstream_snmp_config_policy[__SNMP_CONFIG_MAX];
extern const struct blobmsg_policy grandstream_snmp_ports_policy[__SNMP_PORTS_MAX];
extern const struct blobmsg_policy grandstream_snmp_v3_auth_policy[__SNMP_V3_MAX];
extern const struct blobmsg_policy gs_addit_ssid_policy[__GS_ADDIT_SSID_MAX];
extern const struct blobmsg_policy gs_radio_policy[__GS_RADIO_MAX];
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
int
cfparse_load_grandstream(
    void
);

void
cfparse_grandstream_init(
     void
);

void
cfparse_grandstream_deinit(
     void
);

void
cfparse_grandstream_manage_set_ap(
    struct ubus_context *ctx,
    struct blob_attr *attr,
    int *update
);

#endif
