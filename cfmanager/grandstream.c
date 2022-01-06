/****************************************************************************
* *
* * FILENAME:        $RCSfile: grandstream.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/15
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
#include "global.h"
#include "track.h"
#include "apply.h"
#include "time.h"
#include "initd.h"
#include "utils.h"
#include "cfparse.h"
#include "config.h"
#include "cfmanager.h"
#include "grandstream.h"

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
enum {
    GS_LED = __OPTION_FLAG_MAX,    //skip OPTION_FLAGS_NEED_RELOAD and OPTION_FLAGS_CONFIG_CHANGED
    GS_ADMIN_PASSWD,
    GS_SNMP,

    __GS_MAX
};

//=================
//  Globals
//=================
const struct blobmsg_policy grandstream_led_policy[__LED_MAX] = {
    [LED_ON] = {  .name = "alwaysOff", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy manage_aps_attrs_list_policy[__MANAGE_APS_ATTR_MAX] = {
    [MANAGE_APS_ATTR_LIST] = { .name = "aps", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy manage_ap_policy[__MANAGE_AP_MAX] = {
    [MANAGE_AP_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
    [MANAGE_AP_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
    [MANAGE_AP_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy gs_ap_policy[__GS_AP_MAX] = {
    [GS_AP_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_FREQUENCY] = { .name = "frequency", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_BAND_STEERING] = { .name = "band_steering", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_ZONES] = { .name = "zones", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_SSIDS] = { .name = "ssids", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_TLS1_2] = { .name = "tls12", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_NETPORT_TYPE] = { .name = "netport_type", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_ATF_MODE] = { .name = "atf_mode", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV4_ENABLE] = { .name = "static", .type = BLOBMSG_TYPE_BOOL },
    [GS_AP_IPV4_IP] = { .name = "ipv4_static", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV4_NETMASK] = { .name = "ipv4_static_mask", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV4_GATEWAY] = { .name = "ipv4_route", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV4_FIRST_DNS] = { .name = "preferred_dns", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV4_SECOND_DNS] = { .name = "alternate_dns", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV6_ENABLE] = { .name = "ipv6_static", .type = BLOBMSG_TYPE_BOOL },
    [GS_AP_IPV6_IP] = { .name = "ipv6_addr", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV6_PREFIX_LENGTH] = { .name = "ipv6_prefix_length", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV6_GATEWAY] = { .name = "ipv6_route", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV6_FIRST_DNS] = { .name = "ipv6_preferred_dns", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_IPV6_SECOND_DNS] = { .name = "ipv6_alternate_dns", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_DISABLE] = { .name = "ap_disable_2g4", .type = BLOBMSG_TYPE_BOOL },
    [GS_AP_2G4_WIDTH] = { .name = "2g4_width", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_CHANNEL_LOCATION] = { .name = "2g4_control_channel", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_CHANNEL] = { .name = "2g4_channel", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_POWER] = { .name = "2g4_power", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_CUSTOM_TXPOWER] = { .name = "custom_2g4_power", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_RSSI_TYPE] = { .name = "2g4_rssi_enable", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_RSSI_THRESHOLD] = { .name = "2g4_rssi", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_MODE] = { .name = "2g4_mode", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_RATE_LIMIT_TYPE] = { .name = "2g4_ratelimit_enable", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_MINI_RATE] = { .name = "2g4_minirate", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_SHORTGI] = { .name = "2g4_shortgi", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_2G4_ALLOW_LEGACY_DEV] = { .name = "allow_legacy", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_DISABLE] = { .name = "ap_disable_5g", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_WIDTH] = { .name = "5g_width", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_CHANNEL] = { .name = "5g_channel", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_POWER] = { .name = "5g_power", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_CUSTOM_TXPOWER] = { .name = "custom_5g_power", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_RSSI_TYPE] = { .name = "custom_5g_power", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_RSSI_THRESHOLD] = { .name = "5g_rssi", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_MODE] = { .name = "5g_mode", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_RATE_LIMIT_TYPE] = { .name = "5g_ratelimit_enable", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_MINI_RATE] = { .name = "5g_minirate", .type = BLOBMSG_TYPE_STRING },
    [GS_AP_5G_SHORTGI] = { .name = "5g_shortgi", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy general_policy[__GENERAL_MAX] = {
    [GENERAL_PAIRING_KEY] = { .name = "pairing_key", .type = BLOBMSG_TYPE_STRING },
    [GENERAL_FAILOVER_KEY] = { .name = "failover_key", .type = BLOBMSG_TYPE_STRING },
    [GENERAL_ADMIN_PASSWORD] = { .name = "admin_password", .type = BLOBMSG_TYPE_STRING },
    [GENERAL_WEB_WAN_ACCESS] = { .name = "web_wan_access", .type = BLOBMSG_TYPE_BOOL },
};

const struct blobmsg_policy grandstream_snmp_config_policy[__SNMP_CONFIG_MAX] = {
    [SNMP_CONFIG_ENABLE_V1_V2C] = { .name = "enable_v1_v2c", .type = BLOBMSG_TYPE_BOOL },
    [SNMP_CONFIG_ENABLE_V3]  = { .name = "enable_v3",      .type = BLOBMSG_TYPE_BOOL },
    [SNMP_CONFIG_SYS_LOCATION]  = { .name = "sysLocation",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_SYS_CONTACT]  = { .name = "sysContact",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_SYS_NAME]   = { .name = "sysName",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_RO_COMMUNITYV4]  = { .name = "roCommunityv4",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_RW_COMMUNITYV4]  = { .name = "rwCommunityv4",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_RO_COMMUNITYV6]  = { .name = "roCommunityv6",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_RW_COMMUNITYV6]   = { .name = "rwCommunityv6",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_TRAP_TYPE]  = { .name = "trapType",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_TRAP_HOST]  = { .name = "trapHost",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_TRAP_PORT]  = { .name = "trapPort",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_TRAP_COMMUNITY]   = { .name = "trapCommunity",  .type = BLOBMSG_TYPE_STRING }
};

const struct blobmsg_policy grandstream_snmp_ports_policy[__SNMP_PORTS_MAX] = {
    [SNMP_PORTS_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [SNMP_PORTS_PORT]  = { .name = "port", .type = BLOBMSG_TYPE_STRING },
    [SNMP_PORTS_PROTOCOL]  = { .name = "protocol", .type = BLOBMSG_TYPE_STRING },
    [SNMP_PORTS_IPV4ADDRESS]  = { .name = "Ip4Address", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy grandstream_snmp_v3_auth_policy[__SNMP_V3_MAX] = {
    [SNMP_V3_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [SNMP_V3_NAME]  = { .name = "userName", .type = BLOBMSG_TYPE_STRING },
    [SNMP_V3_AUTH_TYPE]  = { .name = "authType", .type = BLOBMSG_TYPE_STRING },
    [SNMP_V3_AUTH_PASS_PHRASE]  = { .name = "authPassPhrase", .type = BLOBMSG_TYPE_STRING },
    [SNMP_V3_PRIV_PROTO]  = { .name = "privProto", .type = BLOBMSG_TYPE_STRING },
    [SNMP_V3_PRIV_PASS_PHRASE]  = { .name = "privPassPhrase", .type = BLOBMSG_TYPE_STRING },
    [SNMP_V3_ACCESS_CTRL]  = { .name = "accessCtrl", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy gs_addit_ssid_policy[__GS_ADDIT_SSID_MAX] = {
    [GS_ADDIT_SSID_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_BAND] = { .name = "ssid_band", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_NAME] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_CRYPTO] = { .name = "encryption", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_WPA_KEY_MODE] = { .name = "wpa_key_mode", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_WPA_CRYPTO_TYPE] = { .name = "wpa_encryption", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_PASSWORD] = { .name = "wpa_key", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_SSIDHIDEENABLE] = { .name = "ssid_hidden", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_ISOLATEMODE] = { .name = "isolation", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_GATEWAYMAC] = { .name = "gateway_mac", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_DTIM_PERIOD] = { .name = "dtim_period", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_CLIENT_LIMIT] = { .name = "wifi_client_limit", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_STA_IDLE_TIMEOUT] = { .name = "sta_idle_timeout", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_UAPSD] = { .name = "uapsd", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_PROXY_ARP] = { .name = "proxyarp", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_MCAST_TO_UCAST] = { .name = "mcast_to_ucast", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_BMS] = { .name = "bms", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_BRIDGE_ENABLE] = { .name = "bridge_enable", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_80211W] = { .name = "11W", .type = BLOBMSG_TYPE_STRING },
    [GS_ADDIT_SSID_VLAN] = { .name = "vlan", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy gs_extern_sys_log_policy[_GS_EXTERNAL_LOG_MAX] = {
    [GS_ISSUE_AP_LIST] = { .name = "issue_ap_list", .type = BLOBMSG_TYPE_ARRAY },
    [GS_EXTERN_LOG_URI] = { .name = "syslog_uri", .type = BLOBMSG_TYPE_STRING },
    [GS_EXTERN_LOG_LEVEL] = { .name = "log_level", .type = BLOBMSG_TYPE_STRING },
    [GS_EXTERN_LOG_PROTOCOL] = { .name = "log_protocol", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy gs_email_policy[_GS_EMAIL_MAX] = {
    [GS_EMAIL_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING},
    [GS_EMAIL_HOST] = { .name = "host", .type = BLOBMSG_TYPE_STRING},
    [GS_EMAIL_USER] = { .name = "user", .type = BLOBMSG_TYPE_STRING},
    [GS_EMAIL_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING},
    [GS_EMAIL_DO_NOT_VALIDATE] = { .name = "do_not_validate", .type = BLOBMSG_TYPE_BOOL},
    [GS_EMAIL_ENABLE_NOTIFICATION] = { .name = "enable_notification", .type = BLOBMSG_TYPE_BOOL},
    [GS_EMAIL_FROM_CM_EMAIL_ADDRESS] = { .name = "from_email_address", .type = BLOBMSG_TYPE_STRING},
    [GS_EMAIL_FROM_NAME] = { .name = "from_name", .type = BLOBMSG_TYPE_STRING},
    [GS_EMAIL_EMAILADDRESS] = { .name = "emailaddress", .type = BLOBMSG_TYPE_ARRAY},
};

const struct blobmsg_policy gs_notification_policy[_GS_NOTIFY_MAX] = {
    [GS_NOTIFY_CM_MEMORY_USAGE] = { .name = "notify_memory_usage", .type = BLOBMSG_TYPE_BOOL},
    [GS_MEMORY_USAGE_THRESHOLD] = { .name = "memory_usage_threshold", .type = BLOBMSG_TYPE_STRING},
    [GS_NOTIFY_AP_THROUGHPUT] = { .name = "notify_ap_throughput", .type = BLOBMSG_TYPE_BOOL},
    [GS_AP_THROUGHPUT_THRESHOLD] = { .name = "ap_throughput_threshold", .type = BLOBMSG_TYPE_STRING},
    [GS_NOTIFY_SSID_THROUGHPUT] = { .name = "notify_ssid_throughput", .type = BLOBMSG_TYPE_BOOL},
    [GS_SSID_THROUGHPUT_THRESHOLD] = { .name = "ssid_throughput_threshold", .type = BLOBMSG_TYPE_STRING},
    [GS_NOTIFY_PASSWORD_CHANGE] = { .name = "notify_password_change", .type = BLOBMSG_TYPE_BOOL},
    [GS_NOTIFY_FIRMWARE_UPGRADE] = { .name = "notify_firmware_upgrade", .type = BLOBMSG_TYPE_BOOL},
    [GS_NOTIFY_AP_OFFLINE] = { .name = "notify_ap_offline", .type = BLOBMSG_TYPE_BOOL},
    [GS_NOTIFY_FIND_ROGUEAP] = { .name = "notify_find_rogueap", .type = BLOBMSG_TYPE_BOOL},
};

const struct blobmsg_policy gs_radio_policy[__GS_RADIO_MAX] = {
    [GS_RADIO_2G4CHANNEL] = { .name = "2g4_channel", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4CHANNELWIDTH] = { .name = "2g4_width", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4CHANNEL_LOCATION] = { .name = "2g4_control_channel", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4TXPOWER] = { .name = "2g4_power", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4CUSTOM_TXPOWER] = { .name = "custom_2g4_power", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4SHORTGI] =  { .name = "2g4_shortgi", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4ALLOW_LEGACY_DEV] = { .name = "allow_legacy", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4RSSI_ENABLE] = { .name = "2g4_rssi_enable", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4RSSI_THRESHOLD] = { .name = "2g4_rssi", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4RATE_LIMIT_ENABLE] = { .name = "2g4_ratelimit_enable", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_2G4MINI_RATE] = { .name = "2g4_minirate", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_5GCHANNEL] = { .name = "5g_channel", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_5GCHANNELWIDTH] = { .name = "5g_width", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_5GTXPOWER] = { .name = "5g_power", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_5GCUSTOM_TXPOWER] = { .name = "custom_5g_power", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_5GSHORTGI] = { .name = "5g_shortgi", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_5GRSSI_ENABLE] = { .name = "5g_rssi_enable", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_5GRSSI_THRESHOLD] = { .name = "5g_rssi", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_5GRATE_LIMIT_ENABLE] = { .name = "5g_ratelimit_enable", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_5GMINI_RATE] = { .name = "5g_minirate", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_MUMIMOENABLE] = { .name = "MumimoEnable", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_BEACON_INTERVAL] = { .name = "bintval", .type = BLOBMSG_TYPE_STRING },
    [GS_RADIO_ATF_MODE] = { .name = "atf_mode", .type = BLOBMSG_TYPE_STRING },
#if defined(GWN7062)
    [GS_RADIO_COMPATIBILITYMODE] = { .name = "disable_11ax", .type = BLOBMSG_TYPE_STRING },
#endif
};

struct vlist_tree grandstream_led_vltree;
struct vlist_tree grandstream_ap_vltree;
struct vlist_tree grandstream_general_vltree;
struct vlist_tree grandstream_snmpd_vltree;
struct vlist_tree grandstream_snmpd_ports_vltree;
struct vlist_tree grandstream_snmpd_v3_auth_vltree;
struct vlist_tree grandstream_ssid_vltree;
struct vlist_tree grandstream_email_vltree;
struct vlist_tree grandstream_extern_sys_log_vltree;
struct vlist_tree grandstream_notification_vltree;
struct vlist_tree grandstream_radio_vltree;
//=================
//  Locals
//=================
/*
 * Private Date
 */
static int
cfparse_grandstream_led_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_grandstream_ap_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_grandstream_general_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_grandstream_snmpd_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_grandstream_ssid_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_grandstream_extern_sys_log_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_grandstream_email_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_grandstream_notification_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_grandstream_radio_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

const struct uci_blob_param_list grandstream_led_policy_list = {
    .n_params = __LED_MAX,
    .params = grandstream_led_policy,
};

static const struct uci_blob_param_list gs_ap_policy_list = {
    .n_params = __GS_AP_MAX,
    .params = gs_ap_policy,
};

static const struct uci_blob_param_list grandstream_general_policy_list = {
    .n_params = __GENERAL_MAX,
    .params = general_policy,
};

static const struct uci_blob_param_list grandstream_snmpd_policy_list = {
    .n_params = __SNMP_CONFIG_MAX,
    .params = grandstream_snmp_config_policy,
};

static const struct uci_blob_param_list grandstream_snmp_ports_policy_list = {
    .n_params = __SNMP_PORTS_MAX,
    .params = grandstream_snmp_ports_policy,
};

static const struct uci_blob_param_list grandstream_snmp_v3_auth_policy_list = {
    .n_params = __SNMP_V3_MAX,
    .params = grandstream_snmp_v3_auth_policy,
};

static const struct uci_blob_param_list gs_addit_ssid_policy_list = {
    .n_params = __GS_ADDIT_SSID_MAX,
    .params = gs_addit_ssid_policy,
};

static const struct uci_blob_param_list gs_extern_sys_log_policy_list = {
    .n_params = _GS_EXTERNAL_LOG_MAX,
    .params = gs_extern_sys_log_policy,
};

static const struct uci_blob_param_list gs_email_policy_list = {
    .n_params = _GS_EMAIL_MAX,
    .params = gs_email_policy,
};

static const struct uci_blob_param_list gs_notification_list = {
    .n_params = _GS_NOTIFY_MAX,
    .params = gs_notification_policy,
};

static const struct uci_blob_param_list gs_radio_policy_list = {
    .n_params = __GS_RADIO_MAX,
    .params = gs_radio_policy,
};

static const struct cm_hooker_policy grandstream_hp[__CM_GRANDSTREAM_HOOKER_MAX] = {
    [GRANDSTREAM_HOOKER_LED] = { .cb = cfparse_grandstream_led_hooker },
    [GRANDSTREAM_HOOKER_AP] = { .cb = cfparse_grandstream_ap_hooker },
    [GRANDSTREAM_HOOKER_GENERAL] = { .cb = cfparse_grandstream_general_hooker },
    [GRANDSTREAM_HOOKER_SNMPD_CONFIG] = { .cb = cfparse_grandstream_snmpd_hooker },
    [GRANDSTREAM_HOOKER_SNMPD_PORTS] = { .cb = cfparse_grandstream_snmpd_hooker },
    [GRANDSTREAM_HOOKER_SNMPD_V3_AUTH] = { .cb = cfparse_grandstream_snmpd_hooker },
    [GRANDSTREAM_HOOKER_SSID] = { .cb = cfparse_grandstream_ssid_hooker },
    [GRANDSTREAM_HOOKER_EXTERN_SYS_LOG] = { .cb = cfparse_grandstream_extern_sys_log_hooker },
    [GRANDSTREAM_HOOKER_EMAIL] = { .cb = cfparse_grandstream_email_hooker },
    [GRANDSTREAM_HOOKER_NOTIFY] = { .cb = cfparse_grandstream_notification_hooker },
    [GRANDSTREAM_HOOKER_RADIO] = { .cb = cfparse_grandstream_radio_hooker },
};

const struct cm_vltree_info grandstream_vltree_info[__GRANDSTREAM_VLTREE_MAX] = {
    [GRANDSTREAM_VLTREE_LED] = {
        .key = "lights",
        .vltree = &grandstream_led_vltree,
        .policy_list = &grandstream_led_policy_list,
        .hooker = GRANDSTREAM_HOOKER_LED
    },
    [GRANDSTREAM_VLTREE_AP] = {
        .key = "ap",
        .vltree = &grandstream_ap_vltree,
        .policy_list = &gs_ap_policy_list,
        .hooker = GRANDSTREAM_HOOKER_AP
    },
    [GRANDSTREAM_VLTREE_GENERAL] = {
        .key = "general",
        .vltree = &grandstream_general_vltree,
        .policy_list = &grandstream_general_policy_list,
        .hooker = GRANDSTREAM_HOOKER_GENERAL
    },
    [GRANDSTREAM_VLTREE_SNMPD_CONFIG] = {
        .key = "snmpd",
        .vltree = &grandstream_snmpd_vltree,
        .policy_list = &grandstream_snmpd_policy_list,
        .hooker = GRANDSTREAM_HOOKER_SNMPD_CONFIG
    },
    [GRANDSTREAM_VLTREE_SNMPD_PORTS] = {
        .key = "snmpd_ports",
        .vltree = &grandstream_snmpd_ports_vltree,
        .policy_list = &grandstream_snmp_ports_policy_list,
        .hooker = GRANDSTREAM_HOOKER_SNMPD_PORTS
    },
    [GRANDSTREAM_VLTREE_SNMPD_V3_AUTH] = {
        .key = "snmpv3_auth",
        .vltree = &grandstream_snmpd_v3_auth_vltree,
        .policy_list = &grandstream_snmp_v3_auth_policy_list,
        .hooker = GRANDSTREAM_HOOKER_SNMPD_V3_AUTH
    },
    [GRANDSTREAM_VLTREE_SSID] = {
        .key = "additional_ssid",
        .vltree = &grandstream_ssid_vltree,
        .policy_list = &gs_addit_ssid_policy_list,
        .hooker = GRANDSTREAM_HOOKER_SSID
    },
    [GRANDSTREAM_VLTREE_EXTERN_SYS_LOG] = {
        .key = "debug",
        .vltree = &grandstream_extern_sys_log_vltree,
        .policy_list = &gs_extern_sys_log_policy_list,
        .hooker = GRANDSTREAM_HOOKER_EXTERN_SYS_LOG
    },
    [GRANDSTREAM_VLTREE_EMAIL] = {
        .key = "email",
        .vltree = &grandstream_email_vltree,
        .policy_list = &gs_email_policy_list,
        .hooker = GRANDSTREAM_HOOKER_EMAIL
    },
    [GRANDSTREAM_VLTREE_NOTIFY] = {
        .key = "notification",
        .vltree = &grandstream_notification_vltree,
        .policy_list = &gs_notification_list,
        .hooker = GRANDSTREAM_HOOKER_NOTIFY
    },
    [GRANDSTREAM_VLTREE_RADIO] = {
        .key = "radio",
        .vltree = &grandstream_radio_vltree,
        .policy_list = &gs_radio_policy_list,
        .hooker = GRANDSTREAM_HOOKER_RADIO
    },
};

static struct blob_buf b;
//=================
//  Functions
//=================
static int
cfparse_grandstream_led_hooker(
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
                case LED_ON:
                    rc |= BIT( GS_LED );
                    break;

                default:
                    break;
            }
        default:
            break;
    }

    return rc;
}

//=============================================================================
static int
cfparse_grandstream_ap_hooker(
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
            rc |= OPTION_FLAGS_NEED_RELOAD;
            extend->set_compl_option |= BIT( BIT_MAX );
            break;
        default:
            break;
    }

    return rc;
}

//=============================================================================
static int
cfparse_grandstream_general_hooker(
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
                case GENERAL_PAIRING_KEY:
                case GENERAL_FAILOVER_KEY:
                    rc |= OPTION_FLAGS_NEED_RELOAD;
                    break;
                case GENERAL_ADMIN_PASSWORD:
                    rc |= BIT( GS_ADMIN_PASSWD );
                    break;

                default:
                    break;
            }
        default:
            break;
    }

    return rc;
}

//=============================================================================
static int
cfparse_grandstream_snmpd_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    rc |= BIT( GS_SNMP );

    return rc;
}

//=============================================================================
static int
cfparse_grandstream_ssid_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    return rc;
}

//=============================================================================
static int
cfparse_grandstream_extern_sys_log_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    return rc;
}

//=============================================================================
static int
cfparse_grandstream_email_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    return rc;
}

//=============================================================================
static int
cfparse_grandstream_notification_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    return rc;
}

//=============================================================================
static int
cfparse_grandstream_radio_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    return rc;
}

//=============================================================================
void
cfparse_grandstream_manage_set_ap(
    struct ubus_context *ctx,
    struct blob_attr *attr,
    int *update
)
//=============================================================================
{
    struct blob_attr *tb[__MANAGE_AP_MAX];
    struct cm_config *cm_cfg = NULL;
    uint32_t search_id;
    static struct blob_buf b;

    blobmsg_parse( manage_ap_policy,
        __MANAGE_AP_MAX,
        tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    *update = 0;
    if ( !tb[MANAGE_AP_ACTION] || !tb[MANAGE_AP_MAC] || !tb[MANAGE_AP_TYPE] ) {
        cfmanager_log_message( L_ERR, "Missing some params in json!" );
        return;
    }

    // pair or unpair
    cm_cfg = util_get_vltree_node( &cm_ap_vltree, VLTREE_CM_TREE, blobmsg_get_string( tb[MANAGE_AP_MAC] ) );
    if ( !cm_cfg && !strcmp( blobmsg_get_string( tb[MANAGE_AP_ACTION] ), "1" ) ) {
        cfmanager_log_message( L_DEBUG, "begin add ap %s", blobmsg_get_string( tb[MANAGE_AP_MAC] ) );
        config_add_ap_default( CFMANAGER_CONFIG_NAME,
                    blobmsg_get_string( tb[MANAGE_AP_MAC] ),
                    blobmsg_get_string( tb[MANAGE_AP_TYPE] ) );
        *update = 1;
    }
    else if ( cm_cfg && !strcmp( blobmsg_get_string( tb[MANAGE_AP_ACTION] ), "2" ) ) {
        cfmanager_log_message( L_DEBUG, "begin delete ap %s", blobmsg_get_string( tb[MANAGE_AP_MAC] ) );

        if ( ctx && !ubus_lookup_id( ctx, "controller.discovery", &search_id ) ) {
            blob_buf_init( &b,0 );
            blobmsg_add_string( &b, "mac", blobmsg_get_string( tb[MANAGE_AP_MAC] ) );
            ubus_invoke( ctx, search_id, "delete_device", b.head, NULL, NULL, 1000 );
            blob_buf_free( &b );
            config_del_named_section( CFMANAGER_CONFIG_NAME, "ap", blobmsg_get_string( tb[MANAGE_AP_MAC] ) );
            *update = 1;
        }
        else {
            cfmanager_log_message( L_ERR, "delete ap %s failed!", blobmsg_get_string( tb[MANAGE_AP_MAC] ) );
            return;
        }

    }
    else {
        cfmanager_log_message( L_ERR, "some param error: action:%s, mac:%s, cm_cfg:%p",
                        blobmsg_get_string( tb[MANAGE_AP_ACTION] ),
                        blobmsg_get_string( tb[MANAGE_AP_MAC] ),
                        cm_cfg );
    }
}

//=============================================================================
static struct vlist_tree*
cfparse_grandstream_find_tree(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __GRANDSTREAM_VLTREE_MAX; i++ ) {

        if( !grandstream_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( grandstream_vltree_info[i].key, section_type ) ) {
            return grandstream_vltree_info[i].vltree;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static const struct uci_blob_param_list*
cfparse_grandstream_find_blob_param_list(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __GRANDSTREAM_VLTREE_MAX; i++ ) {

        if( !grandstream_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( grandstream_vltree_info[i].key, section_type ) ) {
            return grandstream_vltree_info[i].policy_list;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static int
cfparse_grandstream_find_hooker(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return -1;
    }

    for( i = 1; i < __GRANDSTREAM_VLTREE_MAX; i++ ) {

        if( !grandstream_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( grandstream_vltree_info[i].key, section_type ) ) {

            return grandstream_vltree_info[i].hooker;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n", section_type );
    return -1;
}

//=============================================================================
static void
cfparse_grandstream_node_free(
    struct grandstream_config_parse *gmcfpar
)
//=============================================================================
{
    SAFE_FREE( gmcfpar->cf_section.config );
    SAFE_FREE( gmcfpar );
}

//=============================================================================
static void
cfparse_grandstream_tree_free(
    struct vlist_tree *vltree,
    struct grandstream_config_parse *gmcfpar
)
//=============================================================================
{
    avl_delete( &vltree->avl, &gmcfpar->node.avl );

    cfparse_grandstream_node_free( gmcfpar );
}

#if 0
//=============================================================================
static void
cfparse_sync_grandstream(
    void
)
//=============================================================================
{
    char path_src[LOOKUP_STR_SIZE] = { 0 };
    char path_dest[LOOKUP_STR_SIZE] = { 0 };

    snprintf( path_src, sizeof( path_src ), "%s/%s", CM_CONFIG_PATH,
        CF_CONFIG_NAME_GRANDSTREAM );
    snprintf( path_dest, sizeof( path_dest ), "%s/%s", UCI_DEFAULT_PATH,
        CF_CONFIG_NAME_GRANDSTREAM );

    util_cpy_file( path_src, path_dest );
}
#endif

//=============================================================================
static void
cfparse_grandstream_config_change_effective(
    int option
)
//=============================================================================
{
    if ( option & BIT( GS_LED ) ) {
        system( "/usr/sbin/sch_led reset&" );
    }

    if( option & BIT( GS_ADMIN_PASSWD ) ) {
    }

    if ( option & BIT( GS_SNMP ) ) {
        system( "/usr/sbin/generate_snmp_config.sh &" );
    }

    if ( option & OPTION_FLAGS_NEED_RELOAD ) {
        apply_add( "grandstream" );
        apply_timer_start();
    }
}

//=============================================================================
void
cfparse_grandstream_update_cfg(
    const struct blobmsg_policy *policy,
    struct grandstream_config_parse *gmcfpar_old,
    struct grandstream_config_parse *gmcfpar_new,
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

    struct blob_attr *config_old = gmcfpar_old->cf_section.config;
    struct blob_attr *config_new = gmcfpar_new->cf_section.config;

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

            if ( grandstream_hp[hooker].cb ) {
                option |= grandstream_hp[hooker].cb( tb_new, i, &extend );
            }
        }
    }

    cfparse_grandstream_config_change_effective( option );
}

//=============================================================================
static void
cfparse_grandstream_call_update_func(
    const char *section_type,
    struct grandstream_config_parse *gmcfpar_old,
    struct grandstream_config_parse *gmcfpar_new
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type || NULL == gmcfpar_old || NULL == gmcfpar_new ) {
        return;
    }

    for( i = 1; i < __GRANDSTREAM_VLTREE_MAX; i++ ) {

        if( 0 == strcmp( grandstream_vltree_info[i].key, section_type ) ) {

            cfparse_grandstream_update_cfg( grandstream_vltree_info[i].policy_list->params,
                gmcfpar_old,
                gmcfpar_new,
                grandstream_vltree_info[i].policy_list->n_params,
                grandstream_vltree_info[i].hooker );
        }
    }
}

//=============================================================================
static int
cfparse_grandstream_vltree_handle_add_or_del(
    struct grandstream_config_parse *gmcfpar,
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

    if( !gmcfpar ) {
        return -1;
    }

    hooker = cfparse_grandstream_find_hooker( gmcfpar->cf_section.type );
    if( 0 > hooker ) {
        cfmanager_log_message( L_WARNING,
            "There is no corresponding callback processing for %s\n",
            gmcfpar->cf_section.type );

        return -1;
    }

    blob_policy = cfparse_grandstream_find_blob_param_list( gmcfpar->cf_section.type );
    if( NULL == blob_policy ) {
        cfmanager_log_message( L_WARNING,
            "There is no corresponding blob param list for %s\n",
            gmcfpar->cf_section.type );

        return -1;
    }

    policy = blob_policy->params;
    policy_size = blob_policy->n_params;

    struct blob_attr *tb[policy_size];

    blobmsg_parse( policy,
        policy_size,
        tb,
        blob_data( gmcfpar->cf_section.config ),
        blob_len( gmcfpar->cf_section.config ) );

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

        if ( grandstream_hp[hooker].cb ) {
            option |= grandstream_hp[hooker].cb( tb, i, &extend );
        }
    }

    cfparse_grandstream_config_change_effective( option );

    return option;
}

//=============================================================================
static void
cfparse_grandstream_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct grandstream_config_parse *gmcfpar_old = NULL;
    struct grandstream_config_parse *gmcfpar_new = NULL;
    int ret = 0;

    if ( node_old ) {
        gmcfpar_old =
            container_of(node_old, struct grandstream_config_parse, node);
    }

    if ( node_new ) {
        gmcfpar_new =
            container_of(node_new, struct grandstream_config_parse, node);
    }

    if ( gmcfpar_old && gmcfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "update grandstream section '%s'\n", gmcfpar_old->cf_section.name );

        if ( blob_attr_equal( gmcfpar_new->cf_section.config,
                gmcfpar_old->cf_section.config ) ) {
            cfparse_grandstream_node_free( gmcfpar_new );
            return;
        }

        cfparse_grandstream_call_update_func(
            gmcfpar_old->cf_section.type, gmcfpar_old, gmcfpar_new );

        SAFE_FREE( gmcfpar_old->cf_section.config );
        gmcfpar_old->cf_section.config = blob_memdup( gmcfpar_new->cf_section.config );
        cfparse_grandstream_node_free( gmcfpar_new );
    }
    else if ( gmcfpar_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete grandstream section '%s'\n", gmcfpar_old->cf_section.name );

        vltree = cfparse_grandstream_find_tree( gmcfpar_old->cf_section.type );
        if( NULL == vltree ) {
            cfmanager_log_message( L_DEBUG,
                "no tree corresponding to %s was found\n",
                    gmcfpar_old->cf_section.type );

            return;
        }

        ret = cfparse_grandstream_vltree_handle_add_or_del( gmcfpar_old, VLTREE_ACTION_DEL );
        if( ret < 0 ) {
            apply_set_reload_flag( CONFIG_TRACK_GRANDSTREAM );
        }

        cfparse_grandstream_tree_free( vltree, gmcfpar_old );
    }
    else if ( gmcfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "New grandstream section '%s'\n", gmcfpar_new->cf_section.name );

        ret = cfparse_grandstream_vltree_handle_add_or_del( gmcfpar_new, VLTREE_ACTION_ADD );
        if( ret < 0 ) {
            apply_set_reload_flag( CONFIG_TRACK_GRANDSTREAM );
        }
    }
}

//=============================================================================
static void
cfparse_grandstream_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct grandstream_config_parse *gmcfpar = NULL;
    struct vlist_tree *vltree = NULL;
    char *name;
    char *type;

    vltree = cfparse_grandstream_find_tree( section_type );
    if( NULL == vltree ) {
        cfmanager_log_message( L_DEBUG,
            "No corresponding binary tree was found according to %s\n",
            section_type );

        return;
    }

    gmcfpar = calloc_a( sizeof( *gmcfpar ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type ) +1 );

    if ( !gmcfpar ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc grandstream_config_parse '%s'\n", section_name );

        return;
    }

    gmcfpar->cf_section.name = strcpy( name, section_name );
    gmcfpar->cf_section.type = strcpy( type, section_type );
    gmcfpar->cf_section.config = blob_memdup( data );

    vlist_add( vltree, &gmcfpar->node, gmcfpar->cf_section.name );
}

//=============================================================================
static void
cfparse_grandstream_uci_to_blob(
    struct uci_section *s
)
//=============================================================================
{
    const struct uci_blob_param_list* uci_blob_list = NULL;

    blob_buf_init( &b, 0 );

    uci_blob_list = cfparse_grandstream_find_blob_param_list( s->type );
    if( NULL == uci_blob_list ) {
        cfmanager_log_message( L_DEBUG,
            "No corresponding uci_blob_param_list was found according to %s\n",
            s->type );

        return;
    }

    uci_to_blob( &b, s, uci_blob_list );

    cfparse_grandstream_add_blob_to_tree( b.head, s->e.name, s->type );

    blob_buf_free( &b );
}
//=============================================================================
int
cfparse_load_grandstream(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct uci_package *package = NULL;
    struct vlist_tree *vltree = NULL;
    int i = 0;

    package = cfparse_init_package( CF_CONFIG_NAME_GRANDSTREAM );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load grandstream package failed\n" );
        return -1;
    }

    for( i = 1; i < __GRANDSTREAM_VLTREE_MAX; i++ ) {
        vltree = grandstream_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_update( vltree );
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        cfparse_grandstream_uci_to_blob( s );
    }

    for( i = 1; i < __GRANDSTREAM_VLTREE_MAX; i++ ) {

        vltree = grandstream_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush( vltree );
    }

    if( apply_get_reload_flag( CONFIG_TRACK_GRANDSTREAM ) ) {
        apply_add( "grandstream" );
        apply_flush_reload_flag( CONFIG_TRACK_GRANDSTREAM );
        apply_timer_start();
    }

    return 0;
}

//=============================================================================
void
cfparse_grandstream_init(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __GRANDSTREAM_VLTREE_MAX; i++ ) {

        vltree = grandstream_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_init( vltree, avl_strcmp, cfparse_grandstream_vltree_update );
        vltree->keep_old = true;
        vltree->no_delete = true;
    }
}

//=============================================================================
void
cfparse_grandstream_deinit(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __GRANDSTREAM_VLTREE_MAX; i++ ) {

        vltree = grandstream_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush_all( vltree );
    }
}

