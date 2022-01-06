/****************************************************************************
* *
* * FILENAME:        $RCSfile: cfmanager.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/05
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
#include "utils.h"
#include "cfmanager.h"
#include "sgrequest.h"
#include "config.h"
#include "cfparse.h"
#include "wireless.h"
#include "firewall.h"
#include "dhcp.h"
#include "controller.h"
#include "network.h"
#include "grandstream.h"
#include "system.h"
#include "ipc.h"
#include "track.h"
#include "time.h"
#include "apply.h"
#include "bwctrl.h"
#include "smb.h"
#include "tr069.h"
#include "schedule.h"
#include "gsportalcfg.h"
#include "vpn.h"
#include "gs_utils.h"
#include "cm2gs.h"

//=================
//  Defines
//=================
#define LED_PROC_PATH   "proc/gxp/led_patterns"
#define LEASETIME_UNIT_MINUTE "m"

//=================
//  Typedefs
//=================
enum {
    CM_HOOKER_WAN,
    CM_HOOKER_WIRELESS,
    CM_HOOKER_LAN,
    CM_HOOKER_VLAN,
    CM_HOOKER_SWITCH_PORT,
    CM_HOOKER_UPGRADE_AUTO,
    CM_HOOKER_GUEST_SSID,
    CM_HOOKER_ACCESS,
    CM_HOOKER_GLOBAL_ACCESS,
    CM_HOOKER_SCHEDULE_ACCESS,
    CM_HOOKER_BASIC_SYSTEM,
    CM_HOOKER_EXTERN_SYS_LOG,
    CM_HOOKER_EMAIL,
    CM_HOOKER_NOTIFY,
    CM_HOOKER_CONTROLLER,
    CM_HOOKER_AP,
    CM_HOOKER_GENERAL,
    CM_HOOKER_VPN_CLIENT,
    CM_HOOKER_VPN_SPLIT,
    CM_HOOKER_PORT_MAPPING,
    CM_HOOKER_DMZ,
    CM_HOOKER_UPNP,
    CM_HOOKER_DDNS,
    CM_HOOKER_STATIC_ROUTE_IPV4,
    CM_HOOKER_MESH_SSID,
    CM_HOOKER_CLIENT_LIMIT,
    CM_HOOKER_USB_SHARE,
    CM_HOOKER_PORTAL_POLICY,
    CM_HOOKER_WAN_ALIAS,
    CM_HOOKER_STATIC_ROUTE_IPV6,
    CM_HOOKER_TR069,
    CM_HOOKER_HOSTNAME,
    CM_HOOKER_SNMP_CONFIG,
    CM_HOOKER_SNMP_PORTS,
    CM_HOOKER_SNMP_V3_AUTH,
    CM_HOOKER_VPN_SERVER, // Don't change the order of CM_HOOKER_VPN_SERVER, CM_HOOKER_IPSEC_CMN_SETTING and CM_HOOKER_ISPEC_DIAL_IN_USER.
    CM_HOOKER_IPSEC_CMN_SETTING,
    CM_HOOKER_IPSEC_DIAL_IN_USER,
    CM_HOOKER_ADDIT_SSID,
    CM_HOOKER_RADIO,
    CM_HOOKER_FIREWALL_DOS,
    CM_HOOKER_ACCELERATION,
    CM_HOOKER_SCHEDULE,

    __CM_HOOKER_MAX             //If this value is set, there is no hooker function
};

struct cm_config_name {
    const char *name;
};

extern const struct blobmsg_policy mesh_ssid_attrs_policy[__MESH_SSID_ATTR_MAX];

//=================
//  Globals
//=================
struct vlist_tree cm_wan_vltree;
struct vlist_tree cm_wireless_vltree;
struct vlist_tree cm_lan_vltree;
struct vlist_tree cm_vlan_vltree;
struct vlist_tree cm_switch_port_vltree;
struct vlist_tree cm_upgrade_auto_vltree;
struct vlist_tree cm_guest_ssid_vltree;
struct vlist_tree cm_access_vltree;
struct vlist_tree cm_global_access_vltree;
struct vlist_tree cm_schedule_access_vltree;
struct vlist_tree cm_basic_system_vltree;
struct vlist_tree cm_extern_log_vltree;
struct vlist_tree cm_CM_EMAIL_vltree;
struct vlist_tree cm_notification_vltree;
struct vlist_tree cm_controller_vltree;
struct vlist_tree cm_ap_vltree;
struct vlist_tree cm_grandstream_vltree;
struct vlist_tree cm_general_vltree;
struct vlist_tree cm_vpn_client_vltree;
struct vlist_tree cm_vpn_split_vltree;
struct vlist_tree cm_port_mapping_vltree;
struct vlist_tree cm_dmz_vltree;
struct vlist_tree cm_upnp_vltree;
struct vlist_tree cm_ddns_vltree;
struct vlist_tree cm_static_route_ipv4_vltree;
struct vlist_tree cm_mesh_ssid_vltree;
struct vlist_tree cm_client_limit_vltree;
struct vlist_tree cm_portal_policy_vltree;
struct vlist_tree cm_tr069_vltree;
struct vlist_tree cm_usb_share_vltree;
struct vlist_tree cm_firewall_dos_vltree;
struct vlist_tree cm_wan_alias_vltree;
struct vlist_tree cm_static_route_ipv6_vltree;
struct vlist_tree cm_hostname_vltree;
struct vlist_tree cm_snmp_config_vltree;
struct vlist_tree cm_snmp_ports_vltree;
struct vlist_tree cm_snmp_v3_auth_vltree;
struct vlist_tree cm_vpn_server_vltree;
struct vlist_tree cm_ipsec_cmn_setting_vltree;
struct vlist_tree cm_ipsec_dial_in_user_vltree;
struct vlist_tree cm_addit_ssid_vltree;
struct vlist_tree cm_radio_vltree;
struct vlist_tree cm_acceleration_vltree;
struct vlist_tree cm_schedule_vltree;

extern const struct blobmsg_policy wan_policy[__WAN_MAX];
extern const struct blobmsg_policy wireless_policy[__WIRELESS_MAX];
extern const struct blobmsg_policy lan_policy[__LAN_MAX];
extern const struct blobmsg_policy vlan_policy[__VLAN_MAX];
extern const struct blobmsg_policy switch_port_policy[__SWITCH_PORT_MAX];
extern const struct blobmsg_policy upgrade_auto_policy[__UPGRADE_AUTO_MAX];
extern const struct blobmsg_policy guest_ssid_policy[__GUEST_SSID_MAX];
extern const struct blobmsg_policy bind_ip_policy[__BIND_IP_MAX];

extern const struct blobmsg_policy access_policy[__ACCESS_ATTR_MAX];
extern const struct blobmsg_policy cm_access_parse_policy[__CM_PARSE_ACCESS_MAX];

extern const struct blobmsg_policy global_access_attrs_policy[__GLOBAL_ACCESS_ATTR_MAX];

extern const struct blobmsg_policy schedule_access_attrs_policy[__SCHEDULE_ACCESS_ATTR_MAX];

extern const struct blobmsg_policy basic_system_policy[__BASIC_SET_MAX];
extern const struct blobmsg_policy cm_extern_sys_log_policy[_EXTERNAL_LOG_MAX];
extern const struct blobmsg_policy cm_email_policy[_CM_EMAIL_MAX];
extern const struct blobmsg_policy cm_notification_policy[_CM_NOTIFY_MAX];
extern const struct blobmsg_policy controller_policy[__CONTROLLER_MAX];
extern const struct blobmsg_policy cm_ap_policy[__CM_AP_MAX];
extern const struct blobmsg_policy general_policy[__GENERAL_MAX];
extern const struct blobmsg_policy vpn_client_policy[__VPN_CLIENT_MAX];
extern const struct blobmsg_policy vpn_split_policy[__VPN_SPLIT_MAX];
extern const struct blobmsg_policy dhcp_dnsmasq_policy[__DHCP_DNSMASQ_MAX];
extern const struct blobmsg_policy port_mapping_policy[__PORT_MAPPING_MAX];
extern const struct blobmsg_policy dmz_policy[__DMZ_MAX];
extern const struct blobmsg_policy upnp_policy[__UPNP_MAX];
extern const struct blobmsg_policy ddns_policy[__DDNS_MAX];
extern const struct blobmsg_policy static_route_ipv4_policy[__STATIC_ROUTE_IPV4_MAX];
extern const struct blobmsg_policy client_limit_policy[__CLIENT_LIMIT_MAX];
extern const struct blobmsg_policy usb_share_policy[__USB_SHARE_MAX];
extern const struct blobmsg_policy wan_alias_policy[__WAN_ALIAS_ATTR_MAX];
extern const struct blobmsg_policy static_route_ipv6_policy[__STATIC_ROUTE_IPV6_MAX];
extern const struct blobmsg_policy cm_tr069_policy[__CM_TR069_MAX];
extern const struct blobmsg_policy hostname_policy[__HOSTNAME_MAX];
extern const struct blobmsg_policy vpn_server_policy[__VPN_SERVER_MAX];
extern const struct blobmsg_policy sgreq_vpn_server_policy[__VPN_SERVER_MAX];
extern const struct blobmsg_policy ipsec_cmn_setting_policy[__IPSEC_CMN_SETTING_ATTR_MAX];
extern const struct blobmsg_policy ipsec_dial_in_user_policy[__IPSEC_DIAL_IN_USER_ATTR_MAX];
extern const struct blobmsg_policy cm_radio_policy[__CM_RADIO_MAX];
extern const struct blobmsg_policy cm_addit_ssid_policy[__CM_ADDIT_SSID_MAX];

extern const struct blobmsg_policy cm_portal_policy[__PORTAL_POLICY_MAX];
extern const struct blobmsg_policy snmp_policy[__SNMP_MAX];
extern const struct blobmsg_policy snmp_config_policy[__SNMP_CONFIG_MAX];
extern const struct blobmsg_policy firewall_dos_policy[__FIREWALL_DOS_MAX];
extern const struct blobmsg_policy acceleration_policy[__ACCELERATION_MAX];
extern const struct blobmsg_policy cm_schedule_policy[__CM_SCHEDULE_MAX];

extern struct device_info device_info;

static bool cm_force_load_flag = false;             //If this flag is set, no memory matching will be done when loading
//=================
//  Locals
//=================

/*
 * Private Functions
 */
static int
cm_wan_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_wireless_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_lan_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_vlan_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_switch_port_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_upgrade_auto_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_guest_ssid_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_access_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_global_access_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_schedule_access_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_system_basic_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_extern_sys_log_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_email_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_notification_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_controller_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_ap_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_mesh_ssid_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_general_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_vpn_client_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_vpn_split_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_upnp_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_ddns_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_static_route_ipv4_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_client_limit_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int 
cm_tr069_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_usb_share_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_portal_policy_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_wan_alias_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_static_route_ipv6_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_hostname_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_vpn_server_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_ipsec_cmn_setting_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_ipsec_dial_in_user_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);


static int
cm_snmp_config_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_addit_ssid_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_snmp_ports_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_snmp_v3_auth_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_radio_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_firewall_hooker_dos(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_acceleration_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cm_schedule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

/*
 * Private Data
 */

static const struct uci_blob_param_list cm_wan_policy_list = {
    .n_params = __WAN_MAX,
    .params = wan_policy,
};

static const struct uci_blob_param_list cm_wireless_policy_list = {
    .n_params = __WIRELESS_MAX,
    .params = wireless_policy,
};

static const struct uci_blob_param_list cm_lan_policy_list = {
    .n_params = __LAN_MAX,
    .params = lan_policy,
};

static const struct uci_blob_param_list cm_vlan_policy_list = {
    .n_params = __VLAN_MAX,
    .params = vlan_policy,
};

static const struct uci_blob_param_list cm_switch_port_policy_list = {
    .n_params = __SWITCH_PORT_MAX,
    .params = switch_port_policy,
};

static const struct uci_blob_param_list cm_pgrade_auto_policy_list = {
    .n_params = __UPGRADE_AUTO_MAX,
    .params = upgrade_auto_policy,
};

static const struct uci_blob_param_list cm_guest_ssid_policy_list = {
    .n_params = __GUEST_SSID_MAX,
    .params = guest_ssid_policy,
};

static const struct uci_blob_param_list cm_access_policy_list = {
    .n_params = __CM_PARSE_ACCESS_MAX,
    .params = cm_access_parse_policy,
};

static const struct uci_blob_param_list cm_global_access_policy_list = {
    .n_params = __GLOBAL_ACCESS_ATTR_MAX,
    .params = global_access_attrs_policy,
};

static const struct uci_blob_param_list cm_schedule_access_policy_list = {
    .n_params = __SCHEDULE_ACCESS_ATTR_MAX,
    .params = schedule_access_attrs_policy,
};

static const struct uci_blob_param_list cm_basic_set_policy_list = {
    .n_params = __BASIC_SET_MAX,
    .params = basic_system_policy,
};

static const struct uci_blob_param_list cm_extern_log_policy_list = {
    .n_params = _EXTERNAL_LOG_MAX,
    .params = cm_extern_sys_log_policy,
};

static const struct uci_blob_param_list cm_CM_EMAIL_list = {
    .n_params = _CM_EMAIL_MAX,
    .params = cm_email_policy,
};

static const struct uci_blob_param_list cm_notification_list = {
    .n_params = _CM_NOTIFY_MAX,
    .params = cm_notification_policy,
};

static const struct  uci_blob_param_list cm_controller_policy_list = {
    .n_params = __CONTROLLER_MAX,
    .params = controller_policy,
};
static const struct uci_blob_param_list cm_mesh_ssid_policy_list = {
    .n_params = __MESH_SSID_ATTR_MAX,
    .params = mesh_ssid_attrs_policy,
};

static const struct uci_blob_param_list cm_ap_policy_list = {
    .n_params = __CM_AP_MAX,
    .params = cm_ap_policy,
};

static const struct uci_blob_param_list cm_general_policy_list = {
    .n_params = __GENERAL_MAX,
    .params = general_policy,
};

static const struct  uci_blob_param_list cm_vpn_client_policy_list = {
    .n_params = __VPN_CLIENT_MAX,
    .params = vpn_client_policy,
};

static const struct  uci_blob_param_list cm_vpn_split_policy_list = {
    .n_params = __VPN_SPLIT_MAX,
    .params = vpn_split_policy,
};

static const struct  uci_blob_param_list cm_port_mapping_policy_list = {
    .n_params = __PORT_MAPPING_MAX,
    .params = port_mapping_policy,
};

static const struct  uci_blob_param_list cm_dmz_policy_list = {
    .n_params = __DMZ_MAX,
    .params = dmz_policy,
};

static const struct  uci_blob_param_list cm_upnp_policy_list = {
    .n_params = __UPNP_MAX,
    .params = upnp_policy,
};

static const struct  uci_blob_param_list cm_ddns_policy_list = {
    .n_params = __DDNS_MAX,
    .params = ddns_policy,
};

static const struct  uci_blob_param_list cm_static_route_ipv4_policy_list = {
    .n_params = __STATIC_ROUTE_IPV4_MAX,
    .params = static_route_ipv4_policy,
};

static const struct  uci_blob_param_list cm_client_limit_policy_list = {
    .n_params = __CLIENT_LIMIT_MAX,
    .params = client_limit_policy,
};

static const struct uci_blob_param_list cm_tr069_policy_list = {
    .n_params = __CM_TR069_MAX,
    .params = cm_tr069_policy,
};

static const struct uci_blob_param_list cm_usb_share_policy_list = {
    .n_params = __USB_SHARE_MAX,
    .params = usb_share_policy,
};

static const struct  uci_blob_param_list cm_portal_policy_list = {
    .n_params = __PORTAL_POLICY_MAX,
    .params = cm_portal_policy,
};

static const struct  uci_blob_param_list cm_wan_alias_policy_list = {
    .n_params = __WAN_ALIAS_ATTR_MAX,
    .params = wan_alias_policy,
};

static const struct uci_blob_param_list cm_static_route_ipv6_policy_list = {
    .n_params = __STATIC_ROUTE_IPV6_MAX,
    .params = static_route_ipv6_policy,
};

static const struct uci_blob_param_list cm_hostname_policy_list = {
    .n_params = __HOSTNAME_MAX,
    .params = hostname_policy,
};

static const struct uci_blob_param_list cm_snmp_config_policy_list = {
    .n_params = __SNMP_CONFIG_MAX,
    .params = snmp_config_policy,
};

static const struct uci_blob_param_list cm_firewall_dos_policy_list = {
    .n_params = __FIREWALL_DOS_MAX,
    .params = firewall_dos_policy,
};

static const struct uci_blob_param_list cm_snmp_ports_policy_list = {
    .n_params = __SNMP_PORTS_MAX,
    .params = grandstream_snmp_ports_policy,
};

static const struct uci_blob_param_list cm_snmp_v3_auth_policy_list = {
    .n_params = __SNMP_V3_MAX,
    .params = grandstream_snmp_v3_auth_policy,
};

static const struct  uci_blob_param_list cm_vpn_server_policy_list = {
    .n_params = __VPN_SERVER_MAX,
    .params = vpn_server_policy,
};

static const struct uci_blob_param_list cm_ipsec_cmn_setting_policy_list = {
    .n_params = __IPSEC_CMN_SETTING_ATTR_MAX,
    .params = ipsec_cmn_setting_policy,
};

static const struct uci_blob_param_list cm_ipsec_dial_in_user_policy_list = {
    .n_params = __IPSEC_DIAL_IN_USER_ATTR_MAX,
    .params = ipsec_dial_in_user_policy,
};

static const struct uci_blob_param_list cm_radio_policy_list = {
    .n_params = __CM_RADIO_MAX,
    .params = cm_radio_policy,
};

static const struct uci_blob_param_list cm_cm_addit_ssid_policy_list = {
    .n_params = __CM_ADDIT_SSID_MAX,
    .params = cm_addit_ssid_policy,
};

static const struct uci_blob_param_list cm_acceleration_policy_list = {
    .n_params = __ACCELERATION_MAX,
    .params = acceleration_policy,
};

static const struct uci_blob_param_list cm_schedule_policy_list = {
    .n_params = __CM_SCHEDULE_MAX,
    .params = cm_schedule_policy,
};


//Modified config profile
static const struct cm_config_name cm_cfg_name[__CM_CFG_MAX] = {
    [CM_CFG_NETWORK] = { .name = "network" },
    [CM_CFG_DHCP] = { .name = "dhcp" },
    [CM_CFG_FIREWALL] = { .name = "firewall" },
    [CM_CFG_WIRELESS] = { .name = "wireless" },
    [CM_CFG_SCHEDULE] = { .name = "schedule" },
    [CM_CFG_QOS] = { .name = "qos" },
    [CM_CFG_BLACKHOLE] = { .name = "blackhole" },
    [CM_CFG_SYSTEM] = { .name = "system" },
    [CM_CFG_EMAIL] = {.name = "email"},
    [CM_CFG_NOTIFY] = {.name = "notification"},
    [CM_CFG_GRANDSTREAM] = { .name = "grandstream" },
    [CM_CFG_CONTROLLER] = { .name = "controller" },
    [CM_CFG_CFMANAGER] = { .name = CFMANAGER_CONFIG_NAME },
    [CM_CFG_UPNPD] = { .name = "upnpd" },
    [CM_CFG_DDNS] = { .name = "ddns" },
    [CM_CFG_BWCTRL] = { .name = "bwctrl" },
    [CM_CFG_SAMBA] = {.name = "samba"},
    [CM_CFG_GSPORTAL] = { .name = "gsportalcfg" },
    [CM_CFG_TR069] = {.name = "tr069"},
    [CM_CFG_ACCELERATION] = { .name = "ecm" },
};

//After the cfmanager file is modified, set the corresponding callback processing
static const struct cm_hooker_policy cm_hp[__CM_HOOKER_MAX] ={
    [CM_HOOKER_WAN] = { .cb = cm_wan_hooker, NULL },
    [CM_HOOKER_WIRELESS] = { .cb = cm_wireless_hooker, NULL },
    [CM_HOOKER_LAN] = { .cb = cm_lan_hooker, NULL },
    [CM_HOOKER_VLAN] = { .cb = cm_vlan_hooker, NULL },
    [CM_HOOKER_SWITCH_PORT] = { .cb = cm_switch_port_hooker, NULL },
    [CM_HOOKER_UPGRADE_AUTO] = { .cb = cm_upgrade_auto_hooker, NULL },
    [CM_HOOKER_GUEST_SSID] = { .cb = cm_guest_ssid_hooker, NULL },
    [CM_HOOKER_ACCESS] = { .cb = cm_access_hooker, NULL },
    [CM_HOOKER_GLOBAL_ACCESS] = { .cb = cm_global_access_hooker, NULL },
    [CM_HOOKER_SCHEDULE_ACCESS] = { .cb = cm_schedule_access_hooker, NULL },
    [CM_HOOKER_BASIC_SYSTEM] = { .cb = cm_system_basic_hooker, NULL },
    [CM_HOOKER_EXTERN_SYS_LOG] = {.cb = cm_extern_sys_log_hooker, .cm2gs_cb = cm2gs_extern_sys_log_hooker },
    [CM_HOOKER_EMAIL] = {.cb = cm_email_hooker, .cm2gs_cb = cm2gs_email_hooker },
    [CM_HOOKER_NOTIFY] = {.cb = cm_notification_hooker, .cm2gs_cb = cm2gs_notification_hooker },
    [CM_HOOKER_CONTROLLER] = { .cb = cm_controller_hooker, NULL },
    [CM_HOOKER_AP] = { .cb = cm_ap_hooker, cm2gs_ap_hooker },
    [CM_HOOKER_GENERAL] = { .cb = cm_general_hooker, NULL },
    [CM_HOOKER_VPN_CLIENT] = { .cb = cm_vpn_client_hooker, NULL },
    [CM_HOOKER_VPN_SPLIT] = { .cb = cm_vpn_split_hooker, NULL },
    [CM_HOOKER_UPNP] = { .cb = cm_upnp_hooker, NULL },
    [CM_HOOKER_DDNS] = { .cb = cm_ddns_hooker, NULL },
    [CM_HOOKER_STATIC_ROUTE_IPV4] = { .cb = cm_static_route_ipv4_hooker, NULL },
    [CM_HOOKER_MESH_SSID] = { .cb = cm_mesh_ssid_hooker, NULL },
    [CM_HOOKER_CLIENT_LIMIT] = { .cb = cm_client_limit_hooker, NULL },
    [CM_HOOKER_TR069] = {.cb = cm_tr069_hooker, .cm2gs_cb = cm2gs_tr069_hooker },
    [CM_HOOKER_USB_SHARE] = {.cb = cm_usb_share_hooker, NULL },
    [CM_HOOKER_PORTAL_POLICY] = { .cb = cm_portal_policy_hooker, NULL },
    [CM_HOOKER_WAN_ALIAS] = { .cb = cm_wan_alias_hooker, NULL },
    [CM_HOOKER_STATIC_ROUTE_IPV6] = { .cb = cm_static_route_ipv6_hooker, NULL },
    [CM_HOOKER_HOSTNAME] = { .cb = cm_hostname_hooker, NULL },
    [CM_HOOKER_SNMP_CONFIG] = { .cb = cm_snmp_config_hooker, NULL },
    [CM_HOOKER_SNMP_PORTS] = { .cb = cm_snmp_ports_hooker, NULL },
    [CM_HOOKER_SNMP_V3_AUTH] = { .cb = cm_snmp_v3_auth_hooker, NULL },
    [CM_HOOKER_VPN_SERVER] = { .cb = cm_vpn_server_hooker, NULL },
    [CM_HOOKER_IPSEC_CMN_SETTING] = { .cb = cm_ipsec_cmn_setting_hooker, NULL },
    [CM_HOOKER_IPSEC_DIAL_IN_USER] = { .cb = cm_ipsec_dial_in_user_hooker, NULL },
    [CM_HOOKER_ADDIT_SSID] = { .cb = cm_addit_ssid_hooker, .cm2gs_cb = cm2gs_additional_ssid_hooker },
    [CM_HOOKER_RADIO] = { .cb = cm_radio_hooker, .cm2gs_cb = cm2gs_radio_hooker },
    [CM_HOOKER_FIREWALL_DOS] = { .cb = cm_firewall_hooker_dos, NULL },
    [CM_HOOKER_ACCELERATION] = { .cb = cm_acceleration_hooker, NULL },
    [CM_HOOKER_SCHEDULE] = { .cb = cm_schedule_hooker, NULL },
};

//Put the binary tree into this set
const struct cm_vltree_info cm_vltree_info[__CM_VLTREE_MAX] = {
    [CM_VLTREE_WAN] = { 
        .key = "wan",
        .vltree = &cm_wan_vltree,
        .policy_list = &cm_wan_policy_list,
        .hooker = CM_HOOKER_WAN
    },

    [CM_VLTREE_WIRELESS] = {
        .key = "wireless",
        .vltree = &cm_wireless_vltree,
        .policy_list = &cm_wireless_policy_list,
        .hooker = CM_HOOKER_WIRELESS
    },

    [CM_VLTREE_LAN] =   {
        .key = "lan",
        .vltree = &cm_lan_vltree,
        .policy_list = &cm_lan_policy_list,
        .hooker = CM_HOOKER_LAN
    },

    [CM_VLTREE_VLAN] =   {
        .key = "vlan",
        .vltree = &cm_vlan_vltree,
        .policy_list = &cm_vlan_policy_list,
        .hooker = CM_HOOKER_VLAN
    },

    [CM_VLTREE_SWITCH_PORT] =   {
        .key = "switch_port",
        .vltree = &cm_switch_port_vltree,
        .policy_list = &cm_switch_port_policy_list,
        .hooker = CM_HOOKER_SWITCH_PORT
    },

    [CM_VLTREE_UPGRADE_AUTO] = {
        .key = "upgrade_auto",
        .vltree = &cm_upgrade_auto_vltree,
        .policy_list = &cm_pgrade_auto_policy_list,
        .hooker = CM_HOOKER_UPGRADE_AUTO
    },

    [CM_VLTREE_GUEST_SSID] = {
        .key = "guest_ssid",
        .vltree = &cm_guest_ssid_vltree,
        .policy_list = &cm_guest_ssid_policy_list,
        .hooker = CM_HOOKER_GUEST_SSID
    },

    [CM_VLTREE_ACCESS] = {
        .key = "access",
        .vltree = &cm_access_vltree,
        .policy_list = &cm_access_policy_list,
        .hooker = CM_HOOKER_ACCESS
    },

    [CM_VLTREE_GLOBAL_ACCESS] = {
        .key = "global_access",
        .vltree = &cm_global_access_vltree,
        .policy_list = &cm_global_access_policy_list,
        .hooker = CM_HOOKER_GLOBAL_ACCESS
    },

    [CM_VLTREE_SCHEDULE_ACCESS] = {
        .key = "schedule_access",
        .vltree = &cm_schedule_access_vltree,
        .policy_list = &cm_schedule_access_policy_list,
        .hooker = CM_HOOKER_SCHEDULE_ACCESS
    },

    [CM_VLTREE_BASIC] = {
        .key = "basic",
        .vltree = &cm_basic_system_vltree,
        .policy_list = &cm_basic_set_policy_list,
        .hooker = CM_HOOKER_BASIC_SYSTEM
    },

    [CM_VLTREE_SYS_LOG] = {
        .key = "debug",
        .vltree = &cm_extern_log_vltree,
        .policy_list = &cm_extern_log_policy_list,
        .hooker = CM_HOOKER_EXTERN_SYS_LOG
    },

    [CM_VLTREE_EMAIL] = {
        .key = "email",
        .vltree = &cm_CM_EMAIL_vltree,
        .policy_list = &cm_CM_EMAIL_list,
        .hooker = CM_HOOKER_EMAIL
    },

    [CM_VLTREE_NOTIFICATION] = {
        .key = "notification",
        .vltree = &cm_notification_vltree,
        .policy_list = &cm_notification_list,
        .hooker = CM_HOOKER_NOTIFY
    },

    [CM_VLTREE_CONTROLLER] = {
        .key = "controller",
        .vltree = &cm_controller_vltree,
        .policy_list = &cm_controller_policy_list,
        .hooker = CM_HOOKER_CONTROLLER
    },

    [CM_VLTREE_AP] = {
        .key = "ap",
        .vltree = &cm_ap_vltree,
        .policy_list = &cm_ap_policy_list,
        .hooker = CM_HOOKER_AP
    },

    [CM_VLTREE_GENERAL] = {
        .key = "general",
        .vltree = &cm_general_vltree,
        .policy_list = &cm_general_policy_list,
        .hooker = CM_HOOKER_GENERAL
    },

    [CM_VLTREE_VPN_CLIENT] = {
        .key = "vpn_service",
        .vltree = &cm_vpn_client_vltree,
        .policy_list = &cm_vpn_client_policy_list,
        .hooker = CM_HOOKER_VPN_CLIENT
    },

    [CM_VLTREE_VPN_SPLIT] = {
        .key = "vpn_split",
        .vltree = &cm_vpn_split_vltree,
        .policy_list = &cm_vpn_split_policy_list,
        .hooker = CM_HOOKER_VPN_SPLIT
    },

    [CM_VLTREE_PORT_MAPPING] = {
        .key = "port_mapping",
        .vltree = &cm_port_mapping_vltree,
        .policy_list = &cm_port_mapping_policy_list,
        .hooker = CM_HOOKER_PORT_MAPPING
    },

    [CM_VLTREE_DMZ] = {
        .key = "dmz",
        .vltree = &cm_dmz_vltree,
        .policy_list = &cm_dmz_policy_list,
        .hooker = CM_HOOKER_DMZ
    },

    [CM_VLTREE_UPNP] = {
        .key = "upnp",
        .vltree = &cm_upnp_vltree,
        .policy_list = &cm_upnp_policy_list,
        .hooker = CM_HOOKER_UPNP
    },

    [CM_VLTREE_DDNS] = {
        .key = "ddns",
        .vltree = &cm_ddns_vltree,
        .policy_list = &cm_ddns_policy_list,
        .hooker = CM_HOOKER_DDNS
    },

    [CM_VLTREE_STATIC_ROUTE_IPV4] = {
        .key = "ipv4_static_route",
        .vltree = &cm_static_route_ipv4_vltree,
        .policy_list = &cm_static_route_ipv4_policy_list,
        .hooker = CM_HOOKER_STATIC_ROUTE_IPV4
    },

    [CM_VLTREE_MESH_SSID] = {
        .key = "mesh_ssid",
        .vltree = &cm_mesh_ssid_vltree,
        .policy_list = &cm_mesh_ssid_policy_list,
        .hooker = CM_HOOKER_MESH_SSID
    },

    [CM_VLTREE_CLIENT_LIMIT] = {
        .key = "client_limit",
        .vltree = &cm_client_limit_vltree,
        .policy_list = &cm_client_limit_policy_list,
        .hooker = CM_HOOKER_CLIENT_LIMIT
    },

    [CM_VLTREE_TR069] = {
        .key = "tr069",
        .vltree = &cm_tr069_vltree,
        .policy_list = &cm_tr069_policy_list,
        .hooker = CM_HOOKER_TR069
    },

    [CM_VLTREE_USB_SHARE] = {
        .key = "usb_share",
        .vltree = &cm_usb_share_vltree,
        .policy_list = &cm_usb_share_policy_list,
        .hooker = CM_HOOKER_USB_SHARE
    },

    [CM_VLTREE_WAN_ALIAS] = {
        .key = CM_WAN_ALIAS_SECTION_TYPE,
        .vltree = &cm_wan_alias_vltree,
        .policy_list = &cm_wan_alias_policy_list,
        .hooker = CM_HOOKER_WAN_ALIAS
    },

    [CM_VLTREE_POTAL_POLICY] = {
        .key = "portal_policy",
        .vltree = &cm_portal_policy_vltree,
        .policy_list = &cm_portal_policy_list,
        .hooker = CM_HOOKER_PORTAL_POLICY
    },

    [CM_VLTREE_STATIC_ROUTE_IPV6] = {
        .key = CM_STATIC_ROUTEV6_SECTION,
        .vltree = &cm_static_route_ipv6_vltree,
        .policy_list = &cm_static_route_ipv6_policy_list,
        .hooker = CM_HOOKER_STATIC_ROUTE_IPV6
    },

    [CM_VLTREE_HOSTNAME] = {
        .key = "hostname",
        .vltree = &cm_hostname_vltree,
        .policy_list = &cm_hostname_policy_list,
        .hooker = CM_HOOKER_HOSTNAME
    },

    [CM_VLTREE_SNMP_CONFIG] = {
        .key = "snmpd",
        .vltree = &cm_snmp_config_vltree,
        .policy_list = &cm_snmp_config_policy_list,
        .hooker = CM_HOOKER_SNMP_CONFIG
    },
    [CM_VLTREE_SNMP_PORTS] = {
        .key = "snmpd_ports",
        .vltree = &cm_snmp_ports_vltree,
        .policy_list = &cm_snmp_ports_policy_list,
        .hooker = CM_HOOKER_SNMP_PORTS
    },
    [CM_VLTREE_SNMP_V3_AUTH] = {
        .key = "snmpv3_auth",
        .vltree = &cm_snmp_v3_auth_vltree,
        .policy_list = &cm_snmp_v3_auth_policy_list,
        .hooker = CM_HOOKER_SNMP_V3_AUTH
    },

    [CM_VLTREE_VPN_SERVER] = {
        .key = "vpn_server",
        .vltree = &cm_vpn_server_vltree,
        .policy_list = &cm_vpn_server_policy_list,
        .hooker = CM_HOOKER_VPN_SERVER
    },

    [CM_VLTREE_IPSEC_CMN_SETTING] = {
        .key = "ipsec_cmn_setting",
        .vltree = &cm_ipsec_cmn_setting_vltree,
        .policy_list = &cm_ipsec_cmn_setting_policy_list,
        .hooker = CM_HOOKER_IPSEC_CMN_SETTING
    },

    [CM_VLTREE_IPSEC_DIAL_IN_USER] = {
        .key = "ipsec_dial_in_user",
        .vltree = &cm_ipsec_dial_in_user_vltree,
        .policy_list = &cm_ipsec_dial_in_user_policy_list,
        .hooker = CM_HOOKER_IPSEC_DIAL_IN_USER
    },

    [CM_VLTREE_ADDIT_SSID] = {
        .key = "additional_ssid",
        .vltree = &cm_addit_ssid_vltree,
        .policy_list = &cm_cm_addit_ssid_policy_list,
        .hooker = CM_HOOKER_ADDIT_SSID
    },

    [CM_VLTREE_RADIO] = {
        .key = "radio",
        .vltree = &cm_radio_vltree,
        .policy_list = &cm_radio_policy_list,
        .hooker = CM_HOOKER_RADIO
    },

    [CM_VLTREE_FIREWALL_DOS] = {
        .key = "firewall",
        .vltree = &cm_firewall_dos_vltree,
        .policy_list = &cm_firewall_dos_policy_list,
        .hooker = CM_HOOKER_FIREWALL_DOS
    },

    [CM_VLTREE_ACCELERATION] = {
        .key = "acceleration",
        .vltree = &cm_acceleration_vltree,
        .policy_list = &cm_acceleration_policy_list,
        .hooker = CM_HOOKER_ACCELERATION
    },

    [CM_VLTREE_SCHEDULE] =   {
        .key = "schedule",
        .vltree = &cm_schedule_vltree,
        .policy_list = &cm_schedule_policy_list,
        .hooker = CM_HOOKER_SCHEDULE
    },

};

static struct blob_buf b;
static uint32_t cfg_option;         //The bit field of the configuration file to be saved

//=================
//  Functions
//=================

//=============================================================================
static struct vlist_tree*
cm_find_tree(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __CM_VLTREE_MAX; i++ ) {

        if( !cm_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( cm_vltree_info[i].key, section_type ) ) {
            return cm_vltree_info[i].vltree;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n", section_type );
    return NULL;
}

//=============================================================================
static const struct uci_blob_param_list*
cm_find_blob_param_list(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __CM_VLTREE_MAX; i++ ) {

        if( !cm_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( cm_vltree_info[i].key, section_type ) ) {
            return cm_vltree_info[i].policy_list;
        }
    }

    cfmanager_log_message( L_ERR, "No corresponding parsing strategy was found according to %s\n", section_type );
    return NULL;
}

//=============================================================================
static int
cm_find_hooker(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return -1;
    }

    for( i = 1; i < __CM_VLTREE_MAX; i++ ) {

        if( !cm_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( cm_vltree_info[i].key, section_type ) ) {
            if( __CM_HOOKER_MAX == cm_vltree_info[i].hooker ) {
                return -1;
            }

            return cm_vltree_info[i].hooker;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n", section_type );
    return -1;
}

//=============================================================================
static void
cm_node_free(
    struct cm_config *cmcfg
)
//=============================================================================
{
    SAFE_FREE( cmcfg->cf_section.config );
    SAFE_FREE( cmcfg );
}

//=============================================================================
static void
cm_tree_free(
    struct vlist_tree *vltree,
    struct cm_config *cmcfg
)
//=============================================================================
{
    avl_delete( &vltree->avl, &cmcfg->node.avl );

    cm_node_free( cmcfg );
}

//=============================================================================
static void
cm_set_wan_compl_option(
    uint64_t *set_compl_option,
    int wan,
    int wan_type
)
//=============================================================================
{
    if( NULL == set_compl_option ) {
        return;
    }

    switch ( wan_type ) {
        case WANTYPE_DHCP:
            break;
        case WANTYPE_STATIC:
            if( WAN0 == wan ) {
                *set_compl_option |= BIT( WAN_IP4ADDRESS );
                *set_compl_option |= BIT( WAN_NETMASK );
                *set_compl_option |= BIT( WAN_GATEWAY );
            }
#ifdef DOUBLE_WAN
            else {
                *set_compl_option |= BIT( WAN1_IP4ADDRESS );
                *set_compl_option |= BIT( WAN1_NETMASK );
                *set_compl_option |= BIT( WAN1_GATEWAY );
            }
#endif
            break;
        case WANTYPE_PPPOE:
            if( WAN0 == wan ) {
                *set_compl_option |= BIT( WAN_PPPOEUSER );
                *set_compl_option |= BIT( WAN_PPPOEPASSWORD );
            }
#ifdef DOUBLE_WAN
            else {
                *set_compl_option |= BIT( WAN1_PPPOEUSER );
                *set_compl_option |= BIT( WAN1_PPPOEPASSWORD );
            }
#endif
            break;
        case WANTYPE_PPTP:
            if( WAN0 == wan ) {
                *set_compl_option |= BIT( WAN_PPTPUSER );
                *set_compl_option |= BIT( WAN_PPTPPASSWORD );
                *set_compl_option |= BIT( WAN_PPTPSERVER );
                *set_compl_option |= BIT( WAN_PPTPIP4ADDRESS );
                *set_compl_option |= BIT( WAN_PPTPNETMASK );
                *set_compl_option |= BIT( WAN_PPTPGATEWAY );
                *set_compl_option |= BIT( WAN_PPTPIPTYPE );
            }
#ifdef DOUBLE_WAN
            else {
                *set_compl_option |= BIT( WAN1_PPTPUSER );
                *set_compl_option |= BIT( WAN1_PPTPPASSWORD );
                *set_compl_option |= BIT( WAN1_PPTPSERVER );
                *set_compl_option |= BIT( WAN1_PPTPIP4ADDRESS );
                *set_compl_option |= BIT( WAN1_PPTPNETMASK );
                *set_compl_option |= BIT( WAN1_PPTPGATEWAY );
                *set_compl_option |= BIT( WAN1_PPTPIPTYPE );
            }
#endif
            break;
        case WANTYPE_L2TP:
            if( WAN0 == wan ) {
                *set_compl_option |= BIT( WAN_L2TPUSER );
                *set_compl_option |= BIT( WAN_L2TPPASSWORD );
                *set_compl_option |= BIT( WAN_L2TPSERVER );
                *set_compl_option |= BIT( WAN_L2TPIP4ADDRESS );
                *set_compl_option |= BIT( WAN_L2TPNETMASK );
                *set_compl_option |= BIT( WAN_L2TPGATEWAY );
                *set_compl_option |= BIT( WAN_L2TPIPTYPE );
            }
#ifdef DOUBLE_WAN
            else {
                *set_compl_option |= BIT( WAN1_L2TPUSER );
                *set_compl_option |= BIT( WAN1_L2TPPASSWORD );
                *set_compl_option |= BIT( WAN1_L2TPSERVER );
                *set_compl_option |= BIT( WAN1_L2TPIP4ADDRESS );
                *set_compl_option |= BIT( WAN1_L2TPNETMASK );
                *set_compl_option |= BIT( WAN1_L2TPGATEWAY );
                *set_compl_option |= BIT( WAN1_L2TPIPTYPE );
            }
#endif
            break;
        default:
            break;
    }
}

//=============================================================================
static int
cm_set_wan_mac(
    int wan_type,
    struct blob_attr **wan_attr
)
//=============================================================================
{
    char *section_type = NULL;
    char *eth_name = NULL;
    char format_mac[MAC_STR_MAX_LEN+1] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_64] = { 0 };
    struct network_config_parse *nwcfpar = NULL;
    struct blob_attr *mac_attr = NULL;
    struct blob_attr *wan_mac_mode_attr = NULL;
    struct blob_attr *wan_mtu_attr = NULL;
    struct blob_attr *wan_proto_attr = NULL;
    int option = 0;
    int wan_mac_mode = 0;
    int mtu = 0;
    int wan_proto = 0;

    if( WAN0 == wan_type ) {
        section_type = WAN_MAC_CUSTOM_SECTION;
        eth_name = WAN_IFNAME;

        mac_attr = wan_attr[WAN_MAC];
        wan_mac_mode_attr = wan_attr[WAN_MACMODE];
        wan_mtu_attr = wan_attr[WAN_MTU];
        wan_proto_attr = wan_attr[WAN_TYPE];
    }
#ifdef DOUBLE_WAN
    else {
        section_type = WAN1_MAC_CUSTOM_SECTION;
        eth_name = WAN1_IFNAME;

        mac_attr = wan_attr[WAN1_MAC];
        wan_mac_mode_attr = wan_attr[WAN1_MACMODE];
        wan_mtu_attr = wan_attr[WAN1_MTU];
        wan_proto_attr = wan_attr[WAN1_TYPE];
    }
#endif

    wan_mac_mode = util_blobmsg_get_int( wan_mac_mode_attr, 0 );
    wan_proto = util_blobmsg_get_int( wan_proto_attr, 0 );
    mtu = util_blobmsg_get_int( wan_mtu_attr, config_get_mtu_defvalue( wan_proto ) );

    switch ( wan_mac_mode ) {
        case WANMAC_ROUTER:
            if( WANMAC_ROUTER == wan_mac_mode ) {
                nwcfpar = util_get_vltree_node( &network_device_vltree,
                    VLTREE_NETWORK, section_type );

                if( nwcfpar ) {
                    snprintf( path, sizeof( path ),
                        "network.%s", section_type );
                    config_uci_del( path, 0 );
                }
            }

            option |= BIT( CM_CFG_NETWORK );
            break;
        case WANMAC_PC:
            break;
        case WANMAC_CUSTOM:
            if( !mac_attr ) {
                cfmanager_log_message( L_DEBUG, "Missing mac information\n" );
                break;
            }

            nwcfpar = util_get_vltree_node( &network_device_vltree,
                    VLTREE_NETWORK, section_type );
            if( !nwcfpar ) {
                config_add_named_section( "network", "device", section_type );
            }

            snprintf( path, sizeof( path ), "network.%s.name", section_type );
            config_uci_set( path, eth_name, 0 );
            util_formatted_mac_with_colo( util_blobmsg_get_string( mac_attr, "" ), format_mac );

            snprintf( path, sizeof( path ), "network.%s.macaddr", section_type );
            config_uci_set( path, format_mac, 0 );

            /*  bug 193779
             *  After setting the custom Mac,
             *  the original MTU setting will be invalid,
             *  and the MTU field needs to be added to the
             *  current configuration set of the custom Mac
             */
            snprintf( path, sizeof( path ), "network.%s.mtu", section_type );
            snprintf( value, sizeof( value ), "%d", mtu );
            config_uci_set( path, value, 0 );

            option |= BIT( CM_CFG_NETWORK );
            break;
        default:
            cfmanager_log_message( L_DEBUG, "unknown type:%d\n", wan_mac_mode );
            break;
    }

    return option;
}

//=============================================================================
static int
cm_wan_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    struct network_config_parse *nwcfpar = NULL;
    char ipv4[IP4ADDR_MAX_LEN+1] = { 0 };
    char first_dns[IP4ADDR_MAX_LEN+1] = { 0 };
    char second_dns[IP4ADDR_MAX_LEN+1] = { 0 };
    char temp[BUF_LEN_128] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    int option = 0;
    int type = 0;
    int link = 0;
    bool enable;
    int action = extend->action;

    if( MODE_AP == device_info.work_mode ) {
        cfmanager_log_message( L_ERR, "Configuration of Wan is not supported in ap mode\n" );
        return 0;
    }

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch ( index ) {
                case WAN_TYPE:
                    type = util_blobmsg_get_int( cur_attr, 0 );

                    config_edit_wan_type( new_config, type, "wan0", 0 );
                    if( WANTYPE_DHCP == type ) {
                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_STATIC );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_PPPOE );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_PPTP );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_L2TP );
                    }
                    else if( WANTYPE_STATIC == type ) {
                        extend->need_set_option |= BIT( WAN_IP4ADDRESS );
                        extend->need_set_option |= BIT( WAN_NETMASK );
                        extend->need_set_option |= BIT( WAN_GATEWAY );

                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_PPPOE );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_PPTP );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_L2TP );
                    }
                    else if( WANTYPE_PPPOE == type ) {
                        extend->need_set_option |= BIT( WAN_PPPOEUSER );
                        extend->need_set_option |= BIT( WAN_PPPOEPASSWORD );

                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_STATIC );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_PPTP );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_L2TP );
                    }
                    else if( WANTYPE_PPTP == type ) {
                        extend->need_set_option |= BIT( WAN_PPTPUSER );
                        extend->need_set_option |= BIT( WAN_PPTPPASSWORD );
                        extend->need_set_option |= BIT( WAN_PPTPSERVER );
                        extend->need_set_option |= BIT( WAN_PPTPIP4ADDRESS );
                        extend->need_set_option |= BIT( WAN_PPTPNETMASK );
                        extend->need_set_option |= BIT( WAN_PPTPGATEWAY );
                        extend->need_set_option |= BIT( WAN_PPTPIPTYPE );
                        extend->need_set_option |= BIT( WAN_PPTPMPPE );

                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_STATIC );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_PPPOE );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_L2TP );
                    }
                    else if( WANTYPE_L2TP == type ) {
                        extend->need_set_option |= BIT( WAN_L2TPUSER );
                        extend->need_set_option |= BIT( WAN_L2TPPASSWORD );
                        extend->need_set_option |= BIT( WAN_L2TPSERVER );
                        extend->need_set_option |= BIT( WAN_L2TPIP4ADDRESS );
                        extend->need_set_option |= BIT( WAN_L2TPNETMASK );
                        extend->need_set_option |= BIT( WAN_L2TPGATEWAY );
                        extend->need_set_option |= BIT( WAN_L2TPIPTYPE );

                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_STATIC );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_PPPOE );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN0, WANTYPE_PPTP );
                    }
                    extend->set_compl_option |= BIT( WAN_STATIC_DNSENABLE );
                    extend->set_compl_option |= BIT( WAN_FIRSTDNS );
                    extend->set_compl_option |= BIT( WAN_SECONDDNS );
                    // fall through
                case WAN_STATIC_DNSENABLE:
                    //static ip has no dns enable option.
                    if ( WANTYPE_STATIC != type ) {
                        enable = util_blobmsg_get_bool( new_config[WAN_STATIC_DNSENABLE], false );
                        type = util_blobmsg_get_int( new_config[WAN_TYPE], 0 );
                        // All connection types except static ip can obtain dynamic dns servers
                        // and also can use static dns by config.
                        if ( WANTYPE_L2TP == type || WANTYPE_PPTP == type ) {
                            snprintf( path, sizeof( path ), "network.%s.peerdns",
                                WAN0_VPN_SECTION_NAME );
                            snprintf( temp, sizeof( temp ), "%d", !enable );
                            config_uci_set( path, temp, 0 );
                        }
                        else {
                            snprintf( temp, sizeof( temp ), "%d", !enable );
                            config_uci_set( "network.wan0.peerdns", temp, 0 );
                        }

                        if ( !enable ) {
                            if( WANTYPE_L2TP == type || WANTYPE_PPTP == type ) {
                                config_del_wan_dns_info( WAN0_VPN_SECTION_NAME, 0 );
                            }
                            else {
                                config_del_wan_dns_info( "wan0", 0 );
                            }
                            break;
                        }
                    }
                    else {
                        config_uci_set( "network.wan0.peerdns", "0", 0 );
                    }
                case WAN_FIRSTDNS:
                case WAN_SECONDDNS:
                    type = util_blobmsg_get_int( new_config[WAN_TYPE], 0 );

                    if( WAN_FIRSTDNS == index ) {
                        extend->set_compl_option |= BIT( WAN_SECONDDNS );
                    }

                    if( new_config[WAN_FIRSTDNS] ) {
                        strncpy( first_dns, util_blobmsg_get_string( new_config[WAN_FIRSTDNS], "" ),
                            IP4ADDR_MAX_LEN );
                    }

                    if( new_config[WAN_SECONDDNS] ) {
                        strncpy( second_dns, util_blobmsg_get_string( new_config[WAN_SECONDDNS], "" ),
                                IP4ADDR_MAX_LEN );
                    }

                    if( WANTYPE_L2TP == type || WANTYPE_PPTP == type ) {
                        config_del_wan_dns_info( WAN0_VPN_SECTION_NAME, 0 );
                        snprintf( path, sizeof( path ), "network.%s.dns", WAN0_VPN_SECTION_NAME );
                        if( '\0' != first_dns[0] ) {
                            config_uci_add_list( path, first_dns, 0 );
                        }

                        if( '\0' != second_dns[0] ) {
                            config_uci_add_list( path, second_dns, 0 );
                        }
                    }
                    else {
                        //After deleting the data of type list, write it to the configuration file
                        config_del_wan_dns_info( "wan0", 0 );

                        if( '\0' != first_dns[0] ) {
                            config_uci_add_list( "network.wan0.dns", first_dns, 0 );
                        }

                        if( '\0' != second_dns[0] ) {
                            config_uci_add_list( "network.wan0.dns", second_dns, 0 );
                        }
                    }

                    break;
                case WAN_UPRATE:
                case WAN_DOWNRATE:
#ifdef DOUBLE_WAN
                case WAN1_UPRATE:
                case WAN1_DOWNRATE:
#endif
                {
                    int wan0_uprate = 0;
                    int wan0_downrate = 0;
#ifdef DOUBLE_WAN
                    int wan1_uprate = 0;
                    int wan1_downrate = 0;
#endif

                    enable = false;

                    wan0_uprate = util_blobmsg_get_int( new_config[WAN_UPRATE], 0 );
                    wan0_downrate = util_blobmsg_get_int( new_config[WAN_DOWNRATE], 0 );
#ifdef DOUBLE_WAN
                    wan1_uprate = util_blobmsg_get_int( new_config[WAN1_UPRATE], 0 );
                    wan1_downrate = util_blobmsg_get_int( new_config[WAN1_DOWNRATE], 0 );
#endif

                    if ( !wan0_uprate ) {
                        config_uci_set( "qos.qos_instancewan0.upstream_qos_enabled", "0", 0 );
                        config_uci_set( "qos.qos_instancewan0.upstream_bandwidth", "", 0 );
                    }
                    else {
                        config_uci_set( "qos.qos_instancewan0.upstream_qos_enabled", "1", 0 );

                        sprintf( temp, "%dKbit", wan0_uprate );
                        config_uci_set( "qos.qos_instancewan0.upstream_bandwidth", temp, 0 );

                        enable = true;
                    }

                    if ( !wan0_downrate ) {
                        config_uci_set( "qos.qos_instancewan0.downstream_qos_enabled", "0", 0 );
                        config_uci_set( "qos.qos_instancewan0.downstream_bandwidth", "", 0 );
                    }
                    else {
                        config_uci_set( "qos.qos_instancewan0.downstream_qos_enabled", "1", 0 );

                        sprintf( temp, "%dKbit", wan0_downrate );
                        config_uci_set( "qos.qos_instancewan0.downstream_bandwidth", temp, 0 );

                        enable = true;
                    }
#ifdef DOUBLE_WAN
                    if ( !wan1_uprate ) {
                        config_uci_set( "qos.qos_instancewan1.upstream_qos_enabled", "0", 0 );
                        config_uci_set( "qos.qos_instancewan1.upstream_bandwidth", "", 0 );
                    }
                    else {
                        config_uci_set( "qos.qos_instancewan1.upstream_qos_enabled", "1", 0 );

                        sprintf( temp, "%dKbit", wan1_uprate );
                        config_uci_set( "qos.qos_instancewan1.upstream_bandwidth", temp, 0 );

                        enable = true;
                    }

                    if ( !wan1_downrate ) {
                        config_uci_set( "qos.qos_instancewan1.downstream_qos_enabled", "0", 0 );
                        config_uci_set( "qos.qos_instancewan1.downstream_bandwidth", "", 0 );
                    }
                    else {
                        config_uci_set( "qos.qos_instancewan1.downstream_qos_enabled", "1", 0 );

                        sprintf( temp, "%dKbit", wan1_downrate );
                        config_uci_set( "qos.qos_instancewan1.downstream_bandwidth", temp, 0 );

                        enable = true;
                    }
#endif
                    if ( enable )
                        config_uci_set( "qos.qos.enabled", "1", 0 );
                    else
                        config_uci_set( "qos.qos.enabled", "0", 0 );
                    option &= ~BIT( CM_CFG_NETWORK );
                    option |= BIT( CM_CFG_QOS );
                    break;
                }
                case WAN_MTU:
#ifdef DOUBLE_WAN
                case WAN1_MTU:
#endif
                {
                    char *wan = NULL;
                    char *vpn = NULL;

                    if ( WAN_MTU == index ) {
                        type = util_blobmsg_get_int( new_config[WAN_TYPE], 0 );
                        wan = "wan0";
                        vpn = WAN0_VPN_SECTION_NAME;

                        /*  bug 193779
                         *  After setting the custom Mac,
                         *  the original MTU setting will be invalid,
                         *  and the MTU field needs to be added to the
                         *  current configuration set of the custom Mac.
                         *  So here we need to check the MAC mode set under
                         */
                        extend->need_set_option |= BIT( WAN_MACMODE );
                    }
#ifdef DOUBLE_WAN
                    else {
                        type = util_blobmsg_get_int( new_config[WAN1_TYPE], 0 );
                        wan = "wan1";
                        vpn = WAN1_VPN_SECTION_NAME;

                        extend->need_set_option |= BIT( WAN1_MACMODE );
                    }
#endif
                    if( WANTYPE_PPTP == type || WANTYPE_L2TP == type ) {
                        sprintf( path, "network.%s.mtu", vpn );
                    }
                    else {
                        int i;

                        // Adjust vpn service list mtu
                        for ( i = 0; i < WAN_CNT_MAX; i++ ) {
                            char wan_str[BUF_LEN_16] = { 0 };
                            char mtu_str[BUF_LEN_16] = { 0 };
                            char vpn_type[BUF_LEN_16] = { 0 };
                            int vpn_id = i + VPN_CLIENT_ID_OFFSET;

                            snprintf( path, sizeof(path), "network.vpn%d.interface", vpn_id );
                            if ( config_uci_get_option(path, wan_str, sizeof(wan_str)) ) {
                                cfmanager_log_message( L_INFO,
                                    "Non-exist vpn%d, don't need to adjust MTU.\n", vpn_id );
                            }
                            else {
                                if ( !strcmp(wan_str, "wan0") ) {
                                    int mtu;

                                    cfmanager_log_message( L_INFO,
                                        "Active vpn%d refer to wan0 mtu, adjust it.\n", vpn_id );
                                    snprintf( path, sizeof(path), "network.vpn%d.proto", vpn_id );
                                    config_uci_get_option(path, vpn_type, sizeof(vpn_type));
                                    mtu = strtoul( util_blobmsg_get_string( cur_attr, "" ), NULL, 10 );
                                    if ( !strcmp(vpn_type, "l2tp") ) {
                                        mtu = mtu - 40;
                                    }
                                    else {
                                        mtu = mtu - 50;
                                    }
                                    snprintf( path, sizeof(path), "network.vpn%d.mtu", vpn_id );
                                    memset( mtu_str, 0, sizeof(mtu_str) );
                                    snprintf( mtu_str, sizeof(mtu_str), "%d", mtu );
                                    config_uci_set( path, mtu_str, 0 );
                                }
                            }
                        }

                        snprintf( path, sizeof(path), "network.%s.mtu", wan );
                    }

                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                }
                case WAN_LINKSPEED:
                    link = util_blobmsg_get_int( cur_attr, 0 );
                    config_set_wan_link_speed( "wan0", link );
                    break;
                case WAN_MACMODE:
                case WAN_MAC:
                    option = cm_set_wan_mac( WAN0, new_config );
                    break;
                case WAN_IP4ADDRESS:
                    nwcfpar = util_get_vltree_node( &network_interface_vltree,
                        VLTREE_NETWORK, "wan0" );
                    if( !nwcfpar ) {
                        break;
                    }

                    strncpy( ipv4, util_blobmsg_get_string( cur_attr, "" ), IP4ADDR_MAX_LEN );
                    if( '\0' != nwcfpar->ipv4[0] ) {
                        //If it is list type, delete it first and then add it
                        config_uci_del( "network.wan0.ipaddr", 0 );
                    }

                    /* If you restart the process,
                     * the static IP in the network may disappear,
                     * which is due to the cache, and the 'nwcfpar->ipv4' may already be invalid
                     */
                    config_uci_add_list( "network.wan0.ipaddr", ipv4, 0 );
                    break;
                case WAN_NETMASK:
                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ), IP4ADDR_MAX_LEN );
                    config_uci_set( "network.wan0.netmask", temp, 0 );
                    break;
                case WAN_GATEWAY:
                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ), IP4ADDR_MAX_LEN );
                    config_uci_set( "network.wan0.gateway", temp, 0 );
                    break;
                case WAN_PPPOEUSER:
                    // Disable IPv6 negotiation.
                    config_uci_set( "network.wan0.ipv6", "0", 0 );
                    config_uci_set( "network.wan0.username",
                        util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN_PPPOEPASSWORD:
                    config_uci_set( "network.wan0.password",
                        util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN_L2TPUSER:
                case WAN_PPTPUSER:
                    snprintf( path, sizeof( path ), "network.%s.username",
                        WAN0_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN_L2TPPASSWORD:
                case WAN_PPTPPASSWORD:
                    snprintf( path, sizeof( path ), "network.%s.password",
                        WAN0_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN_L2TPSERVER:
                case WAN_PPTPSERVER:
                    snprintf( path, sizeof( path ), "network.%s.server",
                        WAN0_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN_L2TPIPTYPE:
                case WAN_PPTPIPTYPE:
                    snprintf( path, sizeof( path ), "network.%s.gs_ip_type",
                        WAN0_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN_L2TPIP4ADDRESS:
                case WAN_PPTPIP4ADDRESS:
                    snprintf( path, sizeof( path ), "network.%s.gs_local_ip",
                        WAN0_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN_L2TPNETMASK:
                case WAN_PPTPNETMASK:
                    snprintf( path, sizeof( path ), "network.%s.gs_netmask",
                        WAN0_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN_L2TPGATEWAY:
                case WAN_PPTPGATEWAY:
                    snprintf( path, sizeof( path ), "network.%s.gs_remote_ip",
                        WAN0_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN_PPTPMPPE:
                    snprintf( path, sizeof( path ), "network.%s.mppe",
                        WAN0_VPN_SECTION_NAME );
                    if ( util_blobmsg_get_bool( new_config[index], false ) ) {
                        config_uci_set( path, "1", 0 );
                    }
                    else {
                        config_uci_set( path, "0", 0 );
                    }
                    break;
#ifdef DOUBLE_WAN
                case WAN1_ENABLE: {
                    char cmd[BUF_LEN_64] = { 0 };

                    nwcfpar = util_get_vltree_node( &network_interface_vltree,
                        VLTREE_NETWORK, "wan1" );
                    enable = util_blobmsg_get_bool( cur_attr, false );
                    if( enable ) {
                        //Control LED light
                        snprintf( cmd, sizeof( cmd ), "echo start enable_wan2>%s", LED_PROC_PATH );
                        system( cmd );
                        snprintf( cmd, sizeof( cmd ), "echo stop lan4_link>%s", LED_PROC_PATH );
                        system( cmd );

                        extend->need_set_option |= BIT( WAN1_TYPE );
                        //This could be cm restart,don't destroy the original protocol
                        if( nwcfpar ) {
                            break;
                        }

                        config_set_section( "network", "interface", "wan1" );
                        //Open double Wan for the first time,set the default protocol
                        config_uci_set( "network.wan1.proto", "dhcp", 0 );
                        snprintf( temp, sizeof( temp ), "%d", DHCP_MTU_DEVALUE );
                        config_uci_set( "network.wan1.mtu", temp, 0 );
                        config_uci_set( "network.wan1.ifname", "eth3", 0 );
                        config_uci_set( "network.wan1.metric", "41", 0 );
                        config_uci_set( "network.wan1.force_link", "0", 0 );
                        config_uci_set( "network.wan1.peerdns", "1", 0 );
                        config_uci_set( "network.lan0_zone0.ifname", ROUTER_DOUBWAN_LAN_IFNAME, 0 );
                    }
                    else {
                        //Control LED light
                        snprintf( cmd, sizeof( cmd ), "echo disable enable_wan2>%s", LED_PROC_PATH );
                        system( cmd );
                        snprintf( cmd, sizeof( cmd ), "echo stop wan2_link>%s", LED_PROC_PATH );
                        system( cmd );

                        if( nwcfpar ) {
                            config_uci_del( "network.wan1", 0 );
                            config_uci_set( "network.lan0_zone0.ifname",
                                ROUTER_SINGLEWAN_LAN_IFNAME, 0 );
                        }

                        nwcfpar = util_get_vltree_node( &network_interface_vltree,
                            VLTREE_NETWORK, WAN1_VPN_SECTION_NAME );
                        if( nwcfpar ) {
                            snprintf( path, sizeof( path ), "network.%s",
                                WAN1_VPN_SECTION_NAME );
                            config_uci_del( path, 0 );
                        }

                        nwcfpar = util_get_vltree_node( &network_device_vltree,
                            VLTREE_NETWORK, WAN1_MAC_CUSTOM_SECTION );
                        if( nwcfpar ) {
                            snprintf( path, sizeof( path ), "network.%s",
                                WAN1_MAC_CUSTOM_SECTION );
                            config_uci_del( path, 0 );
                        }

                        extend->set_compl_option |= BIT( BIT_MAX );
                    }
                }
                    break;
                case WAN1_TYPE:
                    type = util_blobmsg_get_int( cur_attr, 0 );

                    config_edit_wan_type( new_config, type, "wan1", 0 );
                    if( WANTYPE_DHCP == type ) {
                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_STATIC );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_PPPOE );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_PPTP );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_L2TP );
                    }
                    else if( WANTYPE_STATIC == type ) {
                        extend->need_set_option |= BIT( WAN1_IP4ADDRESS );
                        extend->need_set_option |= BIT( WAN1_NETMASK );
                        extend->need_set_option |= BIT( WAN1_GATEWAY );

                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_PPPOE );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_PPTP );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_L2TP );
                    }
                    else if( WANTYPE_PPPOE == type ) {
                        extend->need_set_option |= BIT( WAN1_PPPOEUSER );
                        extend->need_set_option |= BIT( WAN1_PPPOEPASSWORD );

                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_STATIC );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_PPTP );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_L2TP );
                    }
                    else if( WANTYPE_PPTP == type ) {
                        extend->need_set_option |= BIT( WAN1_PPTPUSER );
                        extend->need_set_option |= BIT( WAN1_PPTPPASSWORD );
                        extend->need_set_option |= BIT( WAN1_PPTPSERVER );
                        extend->need_set_option |= BIT( WAN1_PPTPIP4ADDRESS );
                        extend->need_set_option |= BIT( WAN1_PPTPNETMASK );
                        extend->need_set_option |= BIT( WAN1_PPTPGATEWAY );
                        extend->need_set_option |= BIT( WAN1_PPTPIPTYPE );
                        extend->need_set_option |= BIT( WAN1_PPTPMPPE );

                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_STATIC );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_PPPOE );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_L2TP );
                    }
                    else if( WANTYPE_L2TP == type ) {
                        extend->need_set_option |= BIT( WAN1_L2TPUSER );
                        extend->need_set_option |= BIT( WAN1_L2TPPASSWORD );
                        extend->need_set_option |= BIT( WAN1_L2TPSERVER );
                        extend->need_set_option |= BIT( WAN1_L2TPIP4ADDRESS );
                        extend->need_set_option |= BIT( WAN1_L2TPNETMASK );
                        extend->need_set_option |= BIT( WAN1_L2TPGATEWAY );
                        extend->need_set_option |= BIT( WAN1_L2TPIPTYPE );

                        //Items that are not within a type are not processed
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_STATIC );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_PPPOE );
                        cm_set_wan_compl_option( &extend->set_compl_option, WAN1, WANTYPE_PPTP );
                    }
                    extend->set_compl_option |= BIT( WAN1_STATIC_DNSENABLE );
                    extend->set_compl_option |= BIT( WAN1_FIRSTDNS );
                    extend->set_compl_option |= BIT( WAN1_SECONDDNS );
                    // fall through
                case WAN1_STATIC_DNSENABLE:
                    //static ip has no dns enable option.
                    if ( WANTYPE_STATIC != type ) {
                        enable = util_blobmsg_get_bool( new_config[WAN1_STATIC_DNSENABLE], false );
                        type = util_blobmsg_get_int( new_config[WAN1_TYPE], 0 );
                        if ( WANTYPE_L2TP == type || WANTYPE_PPTP == type ) {
                            snprintf( path, sizeof( path ), "network.%s.peerdns",
                                WAN1_VPN_SECTION_NAME );
                            snprintf( temp, sizeof( temp ), "%d", !enable );
                            config_uci_set( path, temp, 0 );
                        }
                        else {
                            snprintf( temp, sizeof( temp ), "%d", !enable );
                            config_uci_set( "network.wan1.peerdns", temp, 0 );
                        }
                        
                        //static has no dns enable information
                        if ( !enable ) {
                            if ( WANTYPE_L2TP == type || WANTYPE_PPTP == type ) {
                                config_del_wan_dns_info( WAN1_VPN_SECTION_NAME, 0 );
                            }
                            else {
                                config_del_wan_dns_info( "wan1", 0 );
                            }
                            break;
                        }
                    }
                    else {
                        config_uci_set( "network.wan1.peerdns", "0", 0 );
                    }
                case WAN1_FIRSTDNS:
                case WAN1_SECONDDNS:
                    type = util_blobmsg_get_int( new_config[WAN1_TYPE], 0 );

                    if( WAN1_FIRSTDNS == index ) {
                        extend->set_compl_option |= BIT( WAN1_SECONDDNS );
                    }

                    if( new_config[WAN1_FIRSTDNS] ) {
                        strncpy( first_dns, util_blobmsg_get_string( new_config[WAN1_FIRSTDNS], "" ),
                            IP4ADDR_MAX_LEN );
                    }

                    if( new_config[WAN1_SECONDDNS] ) {
                        strncpy( second_dns, util_blobmsg_get_string( new_config[WAN1_SECONDDNS], "" ),
                                IP4ADDR_MAX_LEN );
                    }

                    if( WANTYPE_L2TP == type || WANTYPE_PPTP == type ) {
                        snprintf( path, sizeof( path ), "network.%s.dns",
                            WAN1_VPN_SECTION_NAME );
                        config_del_wan_dns_info( WAN1_VPN_SECTION_NAME, 0 );
                        if( '\0' != first_dns[0] ) {
                            config_uci_add_list( path, first_dns, 0 );
                        }
                        
                        if( '\0' != second_dns[0] ) {
                            config_uci_add_list( path, second_dns, 0 );
                        }
                    }
                    else {
                        //After deleting the data of type list, write it to the configuration file
                        config_del_wan_dns_info( "wan1", 0 );

                        if( '\0' != first_dns[0] ) {
                            config_uci_add_list( "network.wan1.dns", first_dns, 0 );
                        }

                        if( '\0' != second_dns[0] ) {
                            config_uci_add_list( "network.wan1.dns", second_dns, 0 );
                        }
                    }
                    break;
                case WAN1_LINKSPEED:
                    link = util_blobmsg_get_int( cur_attr, 0 );
                    config_set_wan_link_speed( "wan1", link );
                    break;
                case WAN1_IP4ADDRESS:
                    strncpy( ipv4, util_blobmsg_get_string( cur_attr, "" ), IP4ADDR_MAX_LEN );
                    nwcfpar = util_get_vltree_node( &network_interface_vltree,
                        VLTREE_NETWORK, "wan1" );
                    if( !nwcfpar ) {
                        //Maybe open double Wan for the first time,no data in the cache
                        config_uci_add_list( "network.wan1.ipaddr", ipv4, 0 );
                    }
                    else {
                        if( '\0' != nwcfpar->ipv4[0] ) {
                            //If it is list type, delete it first and then add it
                            config_uci_del( "network.wan1.ipaddr", 0 );
                        }

                        /* If you restart the process,
                        * the static IP in the network may disappear,
                        * which is due to the cache, and the 'nwcfpar->ipv4' may already be invalid
                        */
                        config_uci_add_list( "network.wan1.ipaddr", ipv4, 0 );
                    }
                    break;
                case WAN1_NETMASK:
                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ),
                        IP4ADDR_MAX_LEN );
                    config_uci_set( "network.wan1.netmask", temp, 0 );

                    break;
                case WAN1_GATEWAY:
                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ),
                        IP4ADDR_MAX_LEN );
                    config_uci_set( "network.wan1.gateway", temp, 0 );

                    break;
                case WAN1_PPPOEUSER:
                    // Disable IPv6 negotiation.
                    config_uci_set( "network.wan1.ipv6", "0", 0 );
                    config_uci_set( "network.wan1.username",
                        util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN1_PPPOEPASSWORD:
                    config_uci_set( "network.wan1.password",
                        util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN1_L2TPUSER:
                case WAN1_PPTPUSER:
                    snprintf( path, sizeof( path ), "network.%s.username",
                        WAN1_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN1_L2TPPASSWORD:
                case WAN1_PPTPPASSWORD:
                    snprintf( path, sizeof( path ), "network.%s.password",
                        WAN1_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN1_L2TPSERVER:
                case WAN1_PPTPSERVER:
                    snprintf( path, sizeof( path ), "network.%s.server",
                        WAN1_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN1_L2TPIPTYPE:
                case WAN1_PPTPIPTYPE:
                    snprintf( path, sizeof( path ), "network.%s.gs_ip_type",
                        WAN1_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN1_L2TPIP4ADDRESS:
                case WAN1_PPTPIP4ADDRESS:
                    snprintf( path, sizeof( path ), "network.%s.gs_local_ip",
                        WAN1_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN1_L2TPNETMASK:
                case WAN1_PPTPNETMASK:
                    snprintf( path, sizeof( path ), "network.%s.gs_netmask",
                        WAN1_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN1_L2TPGATEWAY:
                case WAN1_PPTPGATEWAY:
                    snprintf( path, sizeof( path ), "network.%s.gs_remote_ip",
                        WAN1_VPN_SECTION_NAME );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    break;
                case WAN1_PPTPMPPE:
                    snprintf( path, sizeof( path ), "network.%s.mppe",
                        WAN1_VPN_SECTION_NAME );
                    if ( util_blobmsg_get_bool( new_config[index], false ) ) {
                        config_uci_set( path, "1", 0 );
                    }
                    else {
                        config_uci_set( path, "0", 0 );
                    }
                    break;
                case WAN1_MACMODE:
                case WAN1_MAC:
                    enable = util_blobmsg_get_bool( new_config[WAN1_ENABLE], false );
                    if( !enable ) {
                        /* If wan1 is not turned on,
                         * the user-defined MAC is not allowed to be configured
                         */
                        break;
                    }
                    option = cm_set_wan_mac( WAN1, new_config );
                    break;
#endif
                case WAN_BALANCE:
                    break;
                default:
                    break;
            }
    }
    option |= BIT( CM_CFG_NETWORK );

    return option;
}

//=============================================================================
static int
cm_wireless_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    struct wireless_crypto_parameter crypto_param;
    bool merge_radio = false;
    bool enable = false;
    bool wifi_enable = false;
    static char iface_2g[BUF_LEN_8] = { 0 };
    static char iface_5g[BUF_LEN_8] = { 0 };
//    char path[LOOKUP_STR_SIZE] = { 0 };
    unsigned char hide = 0;
    char temp[BUF_LEN_64] = { 0 };
    char ssid_name[SSID_NAME_MAX_LEN+1] = { 0 };
    int ret = 0;
    int option = 0;
    int action = extend->action;
    bool enable_2g4 = false;
    bool enable_5g = false;
    char *id = MASTER_SSID_ID;

    //If 'wifiEnable' is not obtained, a value of true is required by default
    wifi_enable = util_blobmsg_get_bool( new_config[WIRELESS_WIFIENABL], true );
    merge_radio = util_blobmsg_get_bool( new_config[WIRELESS_MERGERADIOENABL], false );
    enable_2g4 = util_blobmsg_get_bool( new_config[WIRELESS_2G4ENABLE], false );
    enable_5g = util_blobmsg_get_bool( new_config[WIRELESS_5GENABLE], false );

    if( merge_radio ) {
        //wifiEnable is only valid when merge_radio is open
        if( '\0' == iface_2g[0] && wifi_enable ) {
            config_get_ifname( iface_2g, sizeof( iface_2g ), RADIO_2G, NET_TYPE_MASTER, id );
        }

        if( '\0' == iface_5g[0] && wifi_enable ) {
            config_get_ifname( iface_5g, sizeof( iface_5g ), RADIO_5G, NET_TYPE_MASTER, id );
        }
    }
    else {
        //2.4g and 5g switches are only valid when merge_radio is off
        if( '\0' == iface_2g[0] && enable_2g4 ) {
            config_get_ifname( iface_2g, sizeof( iface_2g ), RADIO_2G, NET_TYPE_MASTER, id );
        }

        if( '\0' == iface_5g[0] && enable_5g ) {
            config_get_ifname( iface_5g, sizeof( iface_5g ), RADIO_5G, NET_TYPE_MASTER, id );
        }
    }

    cfmanager_log_message( L_DEBUG, "iface_2g:%s****iface_5g:%s\n", iface_2g, iface_5g );

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case WIRELESS_WIFIENABL:
                    /* wifiEnable is only effective when the
                     * multi frequency integration is turned on
                     */
                    if( !merge_radio ) {
                        break;
                    }

                    if( !wifi_enable ) {
                        if( '\0' != iface_2g[0] ) {
                            config_del_iface_section( iface_2g );
                        }
                        if( '\0' != iface_5g[0] ) {
                            config_del_iface_section( iface_5g );
                        }

                        memset( iface_2g, 0, sizeof( iface_2g ) );
                        memset( iface_5g, 0, sizeof( iface_5g ) );

                        extend->set_compl_option |= BIT( WIRELESS_MERGERADIOENABL );
                        extend->set_compl_option |= BIT( WIRELESS_2G4ENABLE );
                        extend->set_compl_option |= BIT( WIRELESS_5GENABLE );
                    }
                    else {
                        extend->need_set_option |= BIT( WIRELESS_MERGERADIOENABL );
                        extend->need_set_option |= BIT( WIRELESS_2G4ENABLE );
                        extend->need_set_option |= BIT( WIRELESS_5GENABLE );
                    }
                    break;
                case WIRELESS_MERGERADIOENABL:
                    extend->need_set_option |= BIT( WIRELESS_UPRATE );
                    if( !merge_radio ) {

                        extend->need_set_option |= BIT( WIRELESS_2G4ENABLE );
                        extend->need_set_option |= BIT( WIRELESS_5GENABLE );
                        break;
                    }

                    if( !wifi_enable ) {
                        if( '\0' != iface_2g[0] ) {
                            config_del_iface_section( iface_2g );
                            memset( iface_2g, 0, sizeof( iface_2g ) );
                        }

                        if( '\0' != iface_5g[0] ) {
                            config_del_iface_section( iface_5g );
                            memset( iface_5g, 0, sizeof( iface_5g ) );
                        }

                        extend->set_compl_option |= BIT( WIRELESS_2G4ENABLE );
                        extend->set_compl_option |= BIT( WIRELESS_2G4SSIDNAME );
                        extend->set_compl_option |= BIT( WIRELESS_2G4CRYPTO );
                        extend->set_compl_option |= BIT( WIRELESS_2G4PASSWORD );
                        extend->set_compl_option |= BIT( WIRELESS_2G4SSIDHIDEENABLE );
                        extend->set_compl_option |= BIT( WIRELESS_5GENABLE );
                        extend->set_compl_option |= BIT( WIRELESS_5GSSIDNAME );
                        extend->set_compl_option |= BIT( WIRELESS_5GCRYPTO );
                        extend->set_compl_option |= BIT( WIRELESS_5GPASSWORD );
                        extend->set_compl_option |= BIT( WIRELESS_5GSSIDHIDEENABLE );

                        break;
                    }

                    if( '\0' == iface_2g[0] ) {

                        ret = config_create_ssid( iface_2g, sizeof( iface_2g ),
                                RADIO_2G, NET_TYPE_MASTER, id );
                        if( ERRCODE_SUCCESS != ret ) {
                            cfmanager_log_message( L_DEBUG, "create 2.4g ssid failed\n" );
                            return 0;
                        }
                    }

                    if( '\0' == iface_5g[0] ) {
                        ret = config_create_ssid( iface_5g, sizeof( iface_5g ),
                                RADIO_5G, NET_TYPE_MASTER, id );
                        if( ERRCODE_SUCCESS != ret ) {
                            cfmanager_log_message( L_DEBUG, "create 5g ssid failed\n" );
                            return 0;
                        }
                    }

                    extend->need_set_option |= BIT( WIRELESS_2G4SSIDNAME );
                    extend->need_set_option |= BIT( WIRELESS_2G4CRYPTO );
                    extend->need_set_option |= BIT( WIRELESS_2G4PASSWORD );
                    extend->need_set_option |= BIT( WIRELESS_2G4SSIDHIDEENABLE );

                    extend->set_compl_option |= BIT( WIRELESS_5GSSIDNAME );
                    extend->set_compl_option |= BIT( WIRELESS_5GCRYPTO );
                    extend->set_compl_option |= BIT( WIRELESS_5GPASSWORD );
                    extend->set_compl_option |= BIT( WIRELESS_5GSSIDHIDEENABLE );
                    break;
                case WIRELESS_2G4ENABLE:
                    extend->need_set_option |= BIT( WIRELESS_UPRATE );
                    if( merge_radio ) {
                        break;
                    }

                    enable = util_blobmsg_get_bool( cur_attr, false );
                    if( !enable ) {
                        config_del_iface_section( iface_2g );
                        memset( iface_2g, 0, sizeof( iface_2g ) );

                        extend->set_compl_option |= BIT( WIRELESS_2G4SSIDNAME );
                        extend->set_compl_option |= BIT( WIRELESS_2G4CRYPTO );
                        extend->set_compl_option |= BIT( WIRELESS_2G4PASSWORD );
                        extend->set_compl_option |= BIT( WIRELESS_2G4SSIDHIDEENABLE );
                    }
                    else {
                        if( '\0' == iface_2g[0] ) {
                            ret = config_create_ssid( iface_2g, sizeof( iface_2g ),
                                    RADIO_2G, NET_TYPE_MASTER, id );
                            if( ERRCODE_SUCCESS != ret ) {
                                cfmanager_log_message( L_DEBUG, "create 2.4g ssid failed\n" );
                                return 0;
                            }
                        }

                        extend->need_set_option |= BIT( WIRELESS_2G4SSIDNAME );
                        extend->need_set_option |= BIT( WIRELESS_2G4CRYPTO );
                        extend->need_set_option |= BIT( WIRELESS_2G4PASSWORD );
                        extend->need_set_option |= BIT( WIRELESS_2G4SSIDHIDEENABLE );
                    }
                    break;
                case WIRELESS_2G4SSIDNAME:
                    strncpy( ssid_name, util_blobmsg_get_string( cur_attr, "" ),
                        SSID_NAME_MAX_LEN );

                    config_set_wireless( CM_ADDIT_SSID_NAME, iface_2g, ssid_name, RADIO_2G );
                    if( merge_radio ) {
                        config_set_wireless( CM_ADDIT_SSID_NAME, iface_5g, ssid_name, RADIO_5G );
                    }
                    option |= BIT( CM_CFG_GSPORTAL );

                    break;
                case WIRELESS_2G4CRYPTO:
                case WIRELESS_2G4PASSWORD:
                    memset( &crypto_param, 0, sizeof( crypto_param ) );

                    crypto_param.radio = RADIO_2G;
                    crypto_param.crypto = util_blobmsg_get_int( new_config[WIRELESS_2G4CRYPTO], 0 );
                    strncpy( crypto_param.ifname, iface_2g,
                            sizeof( crypto_param.ifname ) -1 );
                    strncpy( crypto_param.password, util_blobmsg_get_string( new_config[WIRELESS_2G4PASSWORD], "" ),
                            sizeof( crypto_param.password ) -1 );

                    if( action == VLTREE_ACTION_UPDATE ) {
                        crypto_param.crypto_old = util_blobmsg_get_int( extend->old_config[WIRELESS_2G4CRYPTO], 0 );
                    }
                    else {
                        /* No old encryption when the process is restarted or when a new ssid is added
                         * So the current encryption method is considered to be the old encryption method
                         */
                        crypto_param.crypto_old = util_blobmsg_get_int( new_config[WIRELESS_2G4CRYPTO], 0 );
                    }

                    config_set_wireless_crypto( &crypto_param );

                    if( merge_radio ) {
                        crypto_param.radio = RADIO_5G;
                        memset( crypto_param.ifname, 0, sizeof(crypto_param.ifname) );
                        strncpy( crypto_param.ifname, iface_5g,
                            sizeof( crypto_param.ifname ) -1 );

                        config_set_wireless_crypto( &crypto_param );
                    }

                    extend->set_compl_option |= BIT(WIRELESS_2G4PASSWORD);
                    break;
                case WIRELESS_2G4SSIDHIDEENABLE:
                    hide = util_blobmsg_get_bool( cur_attr, false );
                    sprintf( temp, "%d", hide );

                    config_set_wireless( CM_ADDIT_SSID_SSIDHIDEENABLE, iface_2g, temp, RADIO_2G );
                    if( merge_radio ) {
                        config_set_wireless( CM_ADDIT_SSID_SSIDHIDEENABLE, iface_5g, temp, RADIO_5G );
                    }
                    break;
                case WIRELESS_5GENABLE:
                    extend->need_set_option |= BIT( WIRELESS_UPRATE );
                    if( merge_radio ) {
                        break;
                    }

                    enable = util_blobmsg_get_bool( cur_attr, false );
                    if( !enable ) {
                        config_del_iface_section( iface_5g );
                        memset( iface_5g, 0, sizeof( iface_5g ) );

                        extend->set_compl_option |= BIT( WIRELESS_5GSSIDNAME );
                        extend->set_compl_option |= BIT( WIRELESS_5GCRYPTO );
                        extend->set_compl_option |= BIT( WIRELESS_5GPASSWORD );
                        extend->set_compl_option |= BIT( WIRELESS_5GSSIDHIDEENABLE );
                    }
                    else {
                        if( '\0' == iface_5g[0] ) {
                        ret = config_create_ssid( iface_5g, sizeof( iface_5g ),
                                RADIO_5G, NET_TYPE_MASTER, id );
                            if( ERRCODE_SUCCESS != ret ) {
                                cfmanager_log_message( L_DEBUG, "create 5g ssid failed\n" );
                                return 0;
                            }
                        }

                        extend->need_set_option |= BIT( WIRELESS_5GSSIDNAME );
                        extend->need_set_option |= BIT( WIRELESS_5GCRYPTO );
                        extend->need_set_option |= BIT( WIRELESS_5GPASSWORD );
                        extend->need_set_option |= BIT( WIRELESS_5GSSIDHIDEENABLE );
                    }
                    break;
                case WIRELESS_5GSSIDNAME:
                    if( merge_radio ) {
                        break;
                    }
                    strncpy( ssid_name, util_blobmsg_get_string( cur_attr, "" ),
                        SSID_NAME_MAX_LEN );

                    config_set_wireless( CM_ADDIT_SSID_NAME, iface_5g, ssid_name, RADIO_5G );
                    option |= BIT( CM_CFG_GSPORTAL );
                    break;
                case WIRELESS_5GCRYPTO:
                case WIRELESS_5GPASSWORD:
                    if( merge_radio ) {
                        break;
                    }
                    memset( &crypto_param, 0, sizeof( crypto_param ) );

                    crypto_param.radio = RADIO_5G;
                    crypto_param.crypto = util_blobmsg_get_int( new_config[WIRELESS_5GCRYPTO], 0 );
                    strncpy( crypto_param.ifname, iface_5g,
                            sizeof( crypto_param.ifname ) -1 );
                    strncpy( crypto_param.password, util_blobmsg_get_string( new_config[WIRELESS_5GPASSWORD], "" ),
                            sizeof( crypto_param.password ) -1 );

                    if( action == VLTREE_ACTION_UPDATE ) {
                        crypto_param.crypto_old = util_blobmsg_get_int( extend->old_config[WIRELESS_5GCRYPTO], 0 );
                    }
                    else {
                        /* No old encryption when the process is restarted or when a new ssid is added
                         * So the current encryption method is considered to be the old encryption method
                         */
                        crypto_param.crypto_old = util_blobmsg_get_int( new_config[WIRELESS_5GCRYPTO], 0 );
                    }

                    config_set_wireless_crypto( &crypto_param );

                    extend->set_compl_option |= BIT(WIRELESS_5GPASSWORD);
                    break;
                case WIRELESS_5GSSIDHIDEENABLE:
                    if( merge_radio ) {
                        break;
                    }
                    hide = util_blobmsg_get_bool( cur_attr, false );

                    sprintf( temp, "%d", hide );
                    config_set_wireless( CM_ADDIT_SSID_SSIDHIDEENABLE, iface_5g, temp, RADIO_5G );
                    break;
                case WIRELESS_UPRATE:
                case WIRELESS_DOWNRATE: {
                    struct bwctrl_param bwctrl;
                    int up_rate = util_blobmsg_get_int( new_config[WIRELESS_UPRATE], 0 );
                    int down_rate = util_blobmsg_get_int( new_config[WIRELESS_DOWNRATE], 0 );

                    memset( &bwctrl, 0, sizeof( bwctrl ) );
                    bwctrl.up_rate = up_rate;
                    bwctrl.down_rate = down_rate;
                    bwctrl.cm_id =  MASTER_BWCTRL_CM_ID;
                    if( merge_radio ) {
                        strncpy( bwctrl.iface_2g, iface_2g, sizeof( bwctrl.iface_2g ) -1 );
                        strncpy( bwctrl.iface_5g, iface_5g, sizeof( bwctrl.iface_5g ) -1 );
                    }
                    else {
                        enable = blobmsg_get_bool( new_config[WIRELESS_2G4ENABLE] );
                        if( enable ) {
                            strncpy( bwctrl.iface_2g, iface_2g, sizeof( bwctrl.iface_2g ) -1 );
                        }

                        enable = blobmsg_get_bool( new_config[WIRELESS_5GENABLE] );
                        if( enable ) {
                            strncpy( bwctrl.iface_5g, iface_5g, sizeof( bwctrl.iface_5g ) -1 );
                        }
                    }

                    config_set_wireless_limit( &bwctrl );

                    option |= BIT( CM_CFG_BWCTRL );

                    return option;
                }
                case WIRELESS_2G4CHANNEL:
                case WIRELESS_2G4CHANNELWIDTH:
                case WIRELESS_2G4TXPOWER:
                case WIRELESS_5GCHANNEL:
                case WIRELESS_5GCHANNELWIDTH:
                case WIRELESS_5GTXPOWER:
                case WIRELESS_MUMIMOENABLE:
#if defined(GWN7062)
                case WIRELESS_COMPATIBILITYMODE:
#endif
                default:
                    break;
            }

        option |= BIT( CM_CFG_WIRELESS );

        break;
    }

    return option;
}

//=============================================================================
static int
cm_lan_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    char temp[BUF_LEN_64] = { 0 };
    char first_dns[IP4ADDR_MAX_LEN+1] = { 0 };
    char second_dns[IP4ADDR_MAX_LEN+1] = { 0 };
    char format_dns_info[BUF_LEN_64] = { 0 };
    bool enable = 0;
    char *p = NULL;
    char start_str[BUF_LEN_8] = { 0 };
    char end_str[BUF_LEN_8] = { 0 };
    int option = 0;
    int limit = 0;
    int action = extend->action;

    enable = util_blobmsg_get_bool( new_config[LAN_DHCP_ENABLE], false );
    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case LAN_IP4ADDRESS:
                    config_uci_del( "network.lan0_zone0.ipaddr", 0 );

                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ), IP4ADDR_MAX_LEN );
                    config_uci_add_list( "network.lan0_zone0.ipaddr", temp, 0 );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case LAN_NETMASK:
                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ), IP4ADDR_MAX_LEN );
                    config_uci_set( "network.lan0_zone0.netmask", temp, 0 );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case LAN_DHCP_ENABLE:

                    if( !enable ) {
                        //config_del_lan_dhcp( 0 );
                        config_uci_set( "dhcp.lan0_zone0.ignore", "1", 0 );
                        option |= BIT( CM_CFG_DHCP );

                        extend->set_compl_option |= BIT( LAN_DHCP_IPRANGE );
                        extend->set_compl_option |= BIT( LAN_DHCP_LEASETIME );
                        extend->set_compl_option |= BIT( LAN_DHCP_FIRSTDNS );
                        extend->set_compl_option |= BIT( LAN_DHCP_SECONDDNS );
                    }
                    else {
                        config_uci_set( "dhcp.lan0_zone0.ignore", "0", 0 );
                        extend->need_set_option |= BIT( LAN_DHCP_IPRANGE );
                        extend->need_set_option |= BIT( LAN_DHCP_LEASETIME );
                        extend->need_set_option |= BIT( LAN_DHCP_FIRSTDNS );
                        extend->need_set_option |= BIT( LAN_DHCP_SECONDDNS );
                    }
                    break;
                case LAN_DHCP_IPRANGE:
                    if( !enable ) {
                        break;
                    }

                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ), sizeof( temp ) -1 );

                    p = strstr( temp, "-" );
                    if( !p ) {
                        cfmanager_log_message( L_DEBUG, "Format error\n" );
                        return 0;
                    }

                    p[0] = '\0';
                    strcpy( start_str, temp );
                    strcpy( end_str, p+1 );

                    limit = atoi( end_str ) - atoi( start_str ) +1;
                    snprintf( temp, sizeof( temp ), "%d", limit );
                    config_uci_set( "dhcp.lan0_zone0.start", start_str, 0 );
                    config_uci_set( "dhcp.lan0_zone0.limit", temp, 0 );

                    option |= BIT( CM_CFG_DHCP );
                    break;
                case LAN_DHCP_LEASETIME:
                    if( !enable ) {
                        break;
                    }

                    snprintf( temp, sizeof( temp ), "%s%s", util_blobmsg_get_string( cur_attr, "" ),
                                        LEASETIME_UNIT_MINUTE );
                    config_uci_set( "dhcp.lan0_zone0.leasetime", temp, 0 );

                    option |= BIT( CM_CFG_DHCP );
                    break;
                case LAN_DHCP_FIRSTDNS:
                case LAN_DHCP_SECONDDNS:
                    if( !enable ) {
                       break;
                    }

                    if( LAN_DHCP_FIRSTDNS == index ) {
                        extend->set_compl_option |= BIT( LAN_DHCP_SECONDDNS );
                    }

                    config_del_lan_dns( LAN_DEFAULT_INTERFACE, 0 );

                    if( new_config[LAN_DHCP_FIRSTDNS] ) {
                        strncpy( first_dns, util_blobmsg_get_string( new_config[LAN_DHCP_FIRSTDNS], "" ),
                            IP4ADDR_MAX_LEN );
                    }

                    if( new_config[LAN_DHCP_SECONDDNS] ) {
                        strncpy( second_dns, util_blobmsg_get_string( new_config[LAN_DHCP_SECONDDNS], "" ),
                            IP4ADDR_MAX_LEN );
                    }

                    if( '\0' == first_dns[0] && '\0' == second_dns[0] ) {
                        option |= BIT( CM_CFG_DHCP );
                        break;
                    }

                    if( '\0' != first_dns[0] ) {
                        sprintf( temp, "6,%s", first_dns );
                    }
                    else {
                        //In one case, only second_dns is configured
                        sprintf( temp, "6" );
                    }

                    if( '\0' != second_dns[0] ) {
                        sprintf( format_dns_info, "%s,%s", temp, second_dns );
                    }
                    else {
                        sprintf( format_dns_info, "%s", temp );
                    }

                    config_uci_add_list( "dhcp.lan0_zone0.dhcp_option", format_dns_info, 0 );

                    option |= BIT( CM_CFG_DHCP );
                    break;
                case LAN_BINDIP: {
                    struct blob_attr *cur = NULL;
                    struct dhcp_config_parse *dpcfparse = NULL;
                    char value[BUF_LEN_64] = { 0 };
                    char action_str[BUF_LEN_8] = { 0 };
                    char mac[MAC_STR_MAX_LEN+1];
                    char ip[IP4ADDR_MAX_LEN+1];
                    char temp[BUF_LEN_64];
                    char path[LOOKUP_STR_SIZE] = { 0 };
                    int rem = 0;
                    int action = 0;
                    int ret = 0;

                    if( !cur_attr ) {
                        break;
                    }

                    //format action:xxx,mac:00:0b:82:9c:ef:99,mapIp4Address:xxx
                    blobmsg_for_each_attr( cur, cur_attr, rem ) {
                    if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING ) {
                        continue;
                    }
                        memset( value, 0, sizeof( value ) );
                        memset( action_str, 0, sizeof( action_str ) );
                        strncpy( value, util_blobmsg_get_string( cur, "" ), sizeof( value ) -1 );

                        ret = util_parse_string_data( value, ",", "action",
                                7, action_str, sizeof( action_str ) );
                        if( ret < 0 ) {
                            continue;
                        }
                        action = atoi( action_str );

                        memset( mac, 0, sizeof( mac ) );
                        ret = util_parse_string_data( value, ",", "mac",
                                4, mac, sizeof( mac ) );
                        if( ret < 0 ) {
                            continue;
                        }

                        memset( ip, 0, sizeof( ip ) );
                        ret = util_parse_string_data( value, ",", "mapIp4Address",
                                14, ip, sizeof( ip ) );
                        if( ret < 0 ) {
                            continue;
                        }

                        memset( temp, 0, sizeof( temp ) );

                        if( BIND_IP_ACTION_DEL == action ) {
                            util_str_erase_colon( mac, temp );
                            sprintf( path, "dhcp.%s", temp );
                            config_uci_del( path, 0 );

                            option |= BIT( CM_CFG_DHCP );
                        }
                        else if( BIND_IP_ACTION_ADD == action ) {
                            util_str_erase_colon( mac, temp );
                            dpcfparse = util_get_vltree_node( &dhcp_host_vltree, VLTREE_DHCP, temp );
                            if( !dpcfparse ) {
                                config_set_section( "dhcp", "host", temp );
                            }
                            sprintf( path, "dhcp.%s.mac", temp );
                            config_uci_set( path, mac, 0 );

                            sprintf( path, "dhcp.%s.ip", temp );
                            config_uci_set( path, ip, 0 );

                            option |= BIT( CM_CFG_DHCP );
                        }
                    }
                }
                    break;
                default:
                    break;
            }
    }

    return option;
}

//=============================================================================
static int
cm_vlan_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_64] = { 0 };
    char session_name[BUF_LEN_64] = {0};
    char interface_name[BUF_LEN_64] = {0};
    char first_dns[IP4ADDR_MAX_LEN+1] = { 0 };
    char second_dns[IP4ADDR_MAX_LEN+1] = { 0 };
    char format_dns_info[BUF_LEN_64] = { 0 };
    bool dhcp_enable = 0;
    char *p = NULL;
    char start_str[BUF_LEN_8] = { 0 };
    char end_str[BUF_LEN_8] = { 0 };
    int vlan = 0;
    int option = 0;
    int limit = 0;
    int action = extend->action;

    vlan = atoi( util_blobmsg_get_string( new_config[VLAN_ID], "1" ) );
    if ( LAN_DEFAULT_VLAN_ID == vlan ) {
        /* To maintain compatibility with other models */
        snprintf( interface_name, BUF_LEN_64, LAN_DEFAULT_INTERFACE );
    }
    else {
        snprintf( interface_name, BUF_LEN_64, "zone%d", vlan );
    }
    dhcp_enable = util_blobmsg_get_bool( new_config[VLAN_DHCP_ENABLE], false );
    switch ( action ) {
        case VLTREE_ACTION_DEL:
            switch( index ) {
                case VLAN_ID:
                    snprintf( path, sizeof( path ), "network.switch0.enable_vlan" );
                    config_uci_set( path, config_vlan_is_enable() ? "1" : "0", 0 );

                    snprintf( session_name, BUF_LEN_8, "vlan%d", vlan );
                    config_del_named_section( CF_CONFIG_NAME_NETWORK, "switch_vlan", session_name );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case VLAN_NAME:
                    break;
                case VLAN_PRIORITY:
                    break;
                case VLAN_IP4ENABLE:
                case VLAN_IP4ADDRESS:
                case VLAN_NETMASK:
                    config_del_named_section( CF_CONFIG_NAME_NETWORK, "interface", interface_name );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case VLAN_DHCP_ENABLE:
                case VLAN_DHCP_IPRANGE:
                case VLAN_DHCP_LEASETIME:
                case VLAN_DHCP_FIRSTDNS:
                case VLAN_DHCP_SECONDDNS:
                    config_del_named_section( CF_CONFIG_NAME_DHCP, "dhcp", interface_name );

                    option |= BIT( CM_CFG_DHCP );
                    break;
                case VLAN_IGMPSNOOPING_ENABLE:
                    break;
                default:
                    break;
            }
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case VLAN_ID:
                    snprintf( path, sizeof( path ), "network.switch0.enable_vlan" );
                    config_uci_set( path, config_vlan_is_enable() ? "1" : "0", 0 );

                    snprintf( session_name, BUF_LEN_64, "vlan%d", vlan );
                    config_creat_no_exist_section( "network", "switch_vlan", session_name, &network_switch_vlan_vltree, VLTREE_NETWORK );
                    snprintf( path, sizeof( path ), "network.vlan%d.device", vlan );
                    config_uci_set( path, "switch0", 0 );

                    snprintf( path, sizeof( path ), "network.vlan%d.vlan", vlan );
                    strncpy( value, util_blobmsg_get_string( cur_attr, "0" ), BUF_LEN_64 - 1 );
                    config_uci_set( path, value, 0 );

                    snprintf( path, sizeof( path ), "network.vlan%d.vid", vlan );
                    strncpy( value, util_blobmsg_get_string( cur_attr, "0" ), BUF_LEN_64 - 1 );
                    config_uci_set( path, value, 0 );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case VLAN_NAME:
                    break;
                case VLAN_PRIORITY:
                    break;
                case VLAN_IP4ENABLE:
                    snprintf( session_name, BUF_LEN_64, "%s", interface_name );
                    config_creat_no_exist_section( "network", "interface", session_name, &network_interface_vltree, VLTREE_NETWORK );

                    snprintf( path, sizeof( path ), "network.%s.type", interface_name );
                    config_uci_set( path, "bridge" , 0 );

                    snprintf( path, sizeof( path ), "network.%s.proto", interface_name );
                    config_uci_set( path, "static" , 0 );

                    snprintf( path, sizeof( path ), "network.%s.multicast_querier", interface_name );
                    config_uci_set( path, "0" , 0 );

                    snprintf( path, sizeof( path ), "network.%s.igmp_snooping", interface_name );
                    config_uci_set( path, "0" , 0 );

                    snprintf( path, sizeof( path ), "network.%s.ieee1905managed", interface_name );
                    config_uci_set( path, "1" , 0 );

                    snprintf( path, sizeof( path ), "network.%s.ifname", interface_name );
                    if ( LAN_DEFAULT_VLAN_ID == vlan ) {
                        if ( config_wan1_is_enable() ) {
                            snprintf( value, sizeof( value ), ROUTER_DOUBWAN_LAN_IFNAME );
                        }
                        else {
                            snprintf( value, sizeof( value ), ROUTER_SINGLEWAN_LAN_IFNAME );
                        }
                    }
                    else {
                        if ( config_wan1_is_enable() ) {
                            snprintf( value, sizeof( value ), "eth0.%d eth1.%d eth2.%d", vlan, vlan, vlan );
                        }
                        else {
                            snprintf( value, sizeof( value ), "eth0.%d eth1.%d eth2.%d eth3.%d", vlan, vlan, vlan, vlan );
                        }
                    }
                    config_uci_set( path, value , 0 );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case VLAN_IP4ADDRESS:
                    snprintf( path, sizeof( path ), "network.%s.ipaddr", interface_name );
                    config_uci_del( path, 0 );
                    strncpy( value, util_blobmsg_get_string( cur_attr, "" ), IP4ADDR_MAX_LEN );
                    config_uci_add_list( path, value, 0 );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case VLAN_NETMASK:
                    snprintf( path, sizeof( path ), "network.%s.netmask", interface_name );
                    strncpy( value, util_blobmsg_get_string( cur_attr, "" ), IP4ADDR_MAX_LEN );
                    config_uci_set( path, value, 0 );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case VLAN_DHCP_ENABLE:
                    snprintf( session_name, BUF_LEN_64, "%s", interface_name );
                    config_creat_no_exist_section( "dhcp", "dhcp", session_name, &dhcp_vltree, VLTREE_DHCP );

                    snprintf( path, sizeof( path ), "dhcp.%s.interface", interface_name );
                    snprintf( value, sizeof( value ), "%s", interface_name );
                    config_uci_set( path, value, 0 );

                    if( !dhcp_enable ) {
                        snprintf( path, sizeof( path ), "dhcp.%s.ignore", interface_name );
                        config_uci_set( path, "1", 0 );
                        option |= BIT( CM_CFG_DHCP );

                        extend->set_compl_option |= BIT( VLAN_DHCP_IPRANGE );
                        extend->set_compl_option |= BIT( VLAN_DHCP_LEASETIME );
                    }
                    else {
                        snprintf( path, sizeof( path ), "dhcp.%s.ignore", interface_name );
                        config_uci_set( path, "0", 0 );
                        extend->need_set_option |= BIT( VLAN_DHCP_IPRANGE );
                        extend->need_set_option |= BIT( VLAN_DHCP_LEASETIME );
                    }
                    break;
                case VLAN_DHCP_IPRANGE:
                    if( !dhcp_enable ) {
                        break;
                    }

                    strncpy( value, util_blobmsg_get_string( cur_attr, "" ), sizeof( value ) -1 );

                    p = strstr( value, "-" );
                    if( !p ) {
                        cfmanager_log_message( L_DEBUG, "Format error\n" );
                        return 0;
                    }

                    p[0] = '\0';
                    strcpy( start_str, value );
                    strcpy( end_str, p+1 );

                    snprintf( path, sizeof( path ), "dhcp.%s.start", interface_name );
                    config_uci_set( path, start_str, 0 );

                    snprintf( path, sizeof( path ), "dhcp.%s.limit", interface_name );
                    limit = atoi( end_str ) - atoi( start_str ) +1;
                    snprintf( value, sizeof( value ), "%d", limit );
                    config_uci_set( path, value, 0 );

                    option |= BIT( CM_CFG_DHCP );
                    break;
                case VLAN_DHCP_LEASETIME:
                    if( !dhcp_enable ) {
                        break;
                    }

                    snprintf( path, sizeof( path ), "dhcp.%s.leasetime", interface_name );
                    snprintf( value, sizeof( value ), "%s%s", util_blobmsg_get_string( cur_attr, "" ),
                                        LEASETIME_UNIT_MINUTE );
                    config_uci_set( path, value, 0 );

                    option |= BIT( CM_CFG_DHCP );
                    break;
                case VLAN_DHCP_FIRSTDNS:
                case VLAN_DHCP_SECONDDNS:
                    if( !dhcp_enable ) {
                       break;
                    }

                    if( VLAN_DHCP_FIRSTDNS == index ) {
                        extend->set_compl_option |= BIT( VLAN_DHCP_SECONDDNS );
                    }

                    snprintf( session_name, BUF_LEN_64, "%s", interface_name );
                    config_del_lan_dns( session_name, 0 );

                    if( new_config[VLAN_DHCP_FIRSTDNS] ) {
                        strncpy( first_dns, util_blobmsg_get_string( new_config[VLAN_DHCP_FIRSTDNS], "" ),
                            IP4ADDR_MAX_LEN );
                    }

                    if( new_config[VLAN_DHCP_SECONDDNS] ) {
                        strncpy( second_dns, util_blobmsg_get_string( new_config[VLAN_DHCP_SECONDDNS], "" ),
                            IP4ADDR_MAX_LEN );
                    }

                    if( '\0' == first_dns[0] && '\0' == second_dns[0] ) {
                        option |= BIT( CM_CFG_DHCP );
                        break;
                    }

                    if( '\0' != first_dns[0] ) {
                        sprintf( value, "6,%s", first_dns );
                    }
                    else {
                        //In one case, only second_dns is configured
                        sprintf( value, "6" );
                    }

                    if( '\0' != second_dns[0] ) {
                        sprintf( format_dns_info, "%s,%s", value, second_dns );
                    }
                    else {
                        sprintf( format_dns_info, "%s", value );
                    }

                    snprintf( path, sizeof( path ), "dhcp.%s.dhcp_option", interface_name );
                    config_uci_add_list( path, format_dns_info, 0 );

                    option |= BIT( CM_CFG_DHCP );
                    break;
                case VLAN_IGMPSNOOPING_ENABLE:
                    break;
                default:
                    break;
            }
    }

    return option;
}

//=============================================================================
static int
cm_switch_port_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_64] = { 0 };
    char session_name[BUF_LEN_64] = {0};
    int option = 0;
    int action = extend->action;

    snprintf( session_name, BUF_LEN_64, "port%s", blobmsg_get_string( new_config[SWITCH_PORT_PORT_ID] ) );
    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case SWITCH_PORT_PORT_ID:
                    config_creat_no_exist_section( "network", "gs_switch_port", session_name, &network_gs_switch_port_vltree, VLTREE_NETWORK );

                    snprintf( path, sizeof( path ), "network.%s.port", session_name );
                    strncpy( value, util_blobmsg_get_string( cur_attr, "1" ), BUF_LEN_64 - 1 );
                    config_uci_set( path, value, 0 );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case SWITCH_PORT_PVID:
                    snprintf( path, sizeof( path ), "network.%s.pvid", session_name );
                    strncpy( value, util_blobmsg_get_string( cur_attr, "1" ), BUF_LEN_64 - 1 );
                    config_uci_set( path, value, 0 );

                    option |= BIT( CM_CFG_NETWORK );
                    break;
                case SWITCH_PORT_VLANS:
                    snprintf( path, sizeof( path ), "network.%s.vlan", session_name );
                    strncpy( value, util_blobmsg_get_string( cur_attr, "1" ), BUF_LEN_64 - 1 );
                    config_uci_set( path, value, 0 );

                    option |= BIT( CM_CFG_NETWORK );
                    break;

                default:
                    break;
            }
    }

    return option;
}

//=============================================================================
static int
cm_upgrade_auto_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    int option = 0;
    unsigned char enable = 0;
    unsigned char sync_slave_enable = 0;
    int sch = 0;
    static char sch_section_name[BUF_LEN_16] = { 0 };
    const char *p = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char start_time[BUF_LEN_8] = { 0 };
    char end_time[BUF_LEN_8] = { 0 };
    char temp[BUF_LEN_64];
    int action = extend->action;

    if( '\0' == sch_section_name[0] ) {
        p = config_get_upgrade_auto_sch();
        if( p ) {
            strncpy( sch_section_name, p, sizeof( sch_section_name ) -1 );
        }
    }

    enable = util_blobmsg_get_bool( cur_attr, false );
    sync_slave_enable = util_blobmsg_get_bool( new_config[UPGRADE_AUTO_SYNC_SLAVE_ENABLE], false );
    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case UPGRADE_AUTO_ENABLE:

                    if( enable ) {
                        if( '\0' == sch_section_name[0] ) {
                            sch = config_set_schedule_section();
                            sprintf( sch_section_name, "schedule%d", sch );
                        }
                    }
                    else {
                        if( '\0' == sch_section_name[0] ) {
                            break;
                        }

                        sprintf( path, "schedule.%s", sch_section_name );
                        config_uci_del( path, 0 );
                        memset( sch_section_name, 0, sizeof( sch_section_name ) );
                    }
                case UPGRADE_AUTO_STARTTIME:
                case UPGRADE_AUTO_ENDTIME:
                    if( !enable ) {
                        extend->set_compl_option |= BIT( BIT_MAX );
                        break;
                    }

                    if( !new_config[UPGRADE_AUTO_STARTTIME] ||
                        !new_config[UPGRADE_AUTO_ENDTIME] ) {

                        cfmanager_log_message( L_DEBUG, "Wrong parameter\n" );
                        return 0;
                    }

                    memset( temp, 0, sizeof( temp ) );
                    strncpy( temp, util_blobmsg_get_string( new_config[UPGRADE_AUTO_STARTTIME], "" ),
                        sizeof( temp ) -1 );
                    util_str_erase_colon( temp, start_time );

                    memset( temp, 0, sizeof( temp ) );
                    strncpy( temp, util_blobmsg_get_string( new_config[UPGRADE_AUTO_ENDTIME], "" ),
                        sizeof( temp ) -1 );
                    util_str_erase_colon( temp, end_time );

                    sprintf( temp, "%s-%s", start_time, end_time );
                    config_set_upgrade_auto_config( temp, sch_section_name, sync_slave_enable );

                    extend->set_compl_option |= BIT( BIT_MAX );
                    break;
                default:
                    break;
            }
    }

    option |= BIT( CM_CFG_SCHEDULE );
    return option;
}

//=============================================================================
static int
cm_guest_ssid_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    int option = 0;
    unsigned char enable = 0;
    unsigned char allow_access_master = 0;
    static char iface_2g[BUF_LEN_8] = { 0 };
    static char iface_5g[BUF_LEN_8] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    char temp[BUF_LEN_64] = { 0 };
    char passwd[ SSID_PASSWD_MAX_LEN+1 ] = { 0 };
    int encrypt = 0;
    int action = extend->action;
    int channel_width = 0;
    char *id = GUEST_SSID_ID;

    enable = util_blobmsg_get_bool( new_config[GUEST_SSID_ENABLE], false );

    if( '\0' == iface_2g[0] && enable ) {
        config_get_ifname( iface_2g, sizeof( iface_2g ), RADIO_2G, NET_TYPE_GUEST, id );
    }
    if( '\0' == iface_5g[0] && enable ) {
       config_get_ifname( iface_5g, sizeof( iface_5g ), RADIO_5G, NET_TYPE_GUEST, id );
    }

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch ( index ) {
                case GUEST_SSID_ENABLE:

                    extend->need_set_option |= BIT( GUEST_SSID_UPRATESSID );
                    if( enable ) {
                        if( '\0' == iface_2g[0] ) {
                            config_create_ssid( iface_2g, sizeof( iface_2g ),
                                RADIO_2G, NET_TYPE_GUEST, id );
                        }
                        if( '\0' == iface_5g[0] ) {
                            config_create_ssid( iface_5g, sizeof( iface_5g ),
                                RADIO_5G, NET_TYPE_GUEST, id );
                        }

                        /* When setting up wireless interface here, need to check channel_width
                         * Because may be need to set the disablecoext parameter
                         */
                        channel_width = config_get_channel_width( RADIO_2G );
                        if( CW_2G_HT20_HT40 == channel_width ) {
                            snprintf( path, sizeof( path ), "wireless.%s.disablecoext", iface_2g );
                            config_uci_set( path, "0", 0 );
                        }
                        else if( CW_2G_HT40 == channel_width ) {
                            snprintf( path, sizeof( path ), "wireless.%s.disablecoext", iface_2g );
                            config_uci_set( path, "1", 0 );
                        }

                        extend->need_set_option |= BIT( GUEST_SSID_SSIDNAME );
                        extend->need_set_option |= BIT( GUEST_SSID_ENCRYPTO );
                        extend->need_set_option |= BIT( GUEST_SSID_PASSWD );
                    }
                    else {

                        if( '\0' != iface_2g[0] ) {
                            config_del_iface_section( iface_2g );
                        }

                        if( '\0' != iface_5g[0] ) {
                            config_del_iface_section( iface_5g );
                        }

                        memset( iface_2g, 0, sizeof( iface_2g ) );
                        memset( iface_5g, 0, sizeof( iface_5g ) );

                        extend->set_compl_option |= BIT( GUEST_SSID_SSIDNAME );
                        extend->set_compl_option |= BIT( GUEST_SSID_ENCRYPTO );
                        extend->set_compl_option |= BIT( GUEST_SSID_PASSWD );
                        extend->set_compl_option |= BIT( GUEST_SSID_ALLOW_MASTER );
                    }
                    break;
                case GUEST_SSID_SSIDNAME:
                    if( !enable ) {
                        break;
                    }

                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ), sizeof( temp ) -1 );

                    config_set_wireless( CM_ADDIT_SSID_NAME, iface_2g, temp, RADIO_2G );
                    config_set_wireless( CM_ADDIT_SSID_NAME, iface_5g, temp, RADIO_5G );
                    option |= BIT( CM_CFG_GSPORTAL );

                    break;
                case GUEST_SSID_ENCRYPTO:
                case GUEST_SSID_PASSWD:
                    if( !enable ) {
                        break;
                    }

                    if( GUEST_SSID_ENCRYPTO == index ) {
                        extend->set_compl_option |= BIT( GUEST_SSID_PASSWD );
                    }

                    encrypt = util_blobmsg_get_int( new_config[GUEST_SSID_ENCRYPTO], 0 );

                    if( GUEST_ENCRYPTO_WAP2_WAP3 == encrypt ) {
                        sprintf( path, "wireless.%s.encryption", iface_2g );
                        config_uci_set( path, GUEST_SSID_DEFAULT_ENCRYP, 0 );

                        sprintf( path, "wireless.%s.encryption", iface_5g );
                        config_uci_set( path, GUEST_SSID_DEFAULT_ENCRYP, 0 );

                        strncpy( passwd, util_blobmsg_get_string( new_config[GUEST_SSID_PASSWD], "" ),
                            SSID_PASSWD_MAX_LEN );

                        snprintf( path, sizeof( path ), "wireless.%s.key", iface_2g );
                        config_uci_set( path, passwd, 0 );
                        snprintf( path, sizeof( path ), "wireless.%s.sae_password", iface_2g );
                        config_uci_set( path, passwd, 0 );

                        snprintf( path, sizeof( path ), "wireless.%s.key", iface_5g );
                        config_uci_set( path, passwd, 0 );
                        snprintf( path, sizeof( path ), "wireless.%s.sae_password", iface_5g );
                        config_uci_set( path, passwd, 0 );

                        snprintf( path, sizeof( path ), "wireless.%s.ieee80211w", iface_2g );
                        config_uci_set( path, "0", 0 );

                        snprintf( path, sizeof( path ), "wireless.%s.ieee80211w", iface_5g );
                        config_uci_set( path, "0", 0 );
                    }
                    else {
                        snprintf( path, sizeof( path ), "wireless.%s.encryption", iface_2g );
                        config_uci_del( path, 0 );

                        snprintf( path, sizeof( path ), "wireless.%s.encryption", iface_5g );
                        config_uci_del( path, 0 );

                        snprintf( path, sizeof( path ), "wireless.%s.key", iface_2g );
                        config_uci_del( path, 0 );
                        snprintf( path, sizeof( path ), "wireless.%s.sae_password", iface_2g );
                        config_uci_del( path, 0 );


                        snprintf( path, sizeof( path ), "wireless.%s.key", iface_5g );
                        config_uci_del( path, 0 );
                        snprintf( path, sizeof( path ), "wireless.%s.sae_password", iface_5g );
                        config_uci_del( path, 0 );

                        snprintf( path, sizeof( path ), "wireless.%s.ieee80211w", iface_2g );
                        config_uci_del( path, 0 );

                        snprintf( path, sizeof( path ), "wireless.%s.ieee80211w", iface_5g );
                        config_uci_del( path, 0 );
                    }
                    break;
                case GUEST_SSID_ALLOW_MASTER:
                    if( !enable ) {
                        break;
                    }

                    allow_access_master = util_blobmsg_get_bool( new_config[GUEST_SSID_ALLOW_MASTER], 0 );
                    sprintf( path, "wireless.%s.isolate", iface_2g );
                    config_uci_set( path, allow_access_master ? "0" : "1", 0 );

                    sprintf( path, "wireless.%s.isolate", iface_5g );
                    config_uci_set( path, allow_access_master ? "0" : "1", 0 );
                    break;
                case GUEST_SSID_UPRATESSID:
                case GUEST_SSID_DOWNRATESSID:{
                    struct bwctrl_param bwctrl;
                    int up_rate = util_blobmsg_get_int( new_config[GUEST_SSID_UPRATESSID], 0 );
                    int down_rate = util_blobmsg_get_int( new_config[GUEST_SSID_DOWNRATESSID], 0 );

                    memset( &bwctrl, 0, sizeof( bwctrl ) );
                    bwctrl.up_rate = up_rate;
                    bwctrl.down_rate = down_rate;
                    bwctrl.cm_id =  GUEST_BWCTRL_CM_ID;

                    if( enable ) {
                        strncpy( bwctrl.iface_2g, iface_2g, sizeof( bwctrl.iface_2g ) -1 );
                        strncpy( bwctrl.iface_5g, iface_5g, sizeof( bwctrl.iface_5g ) -1 );
                    }

                    config_set_wireless_limit( &bwctrl );

                    option |= BIT( CM_CFG_BWCTRL );
                    return option;
                }
                default:
                    break;
            }
    }

    option |= BIT( CM_CFG_WIRELESS );

    return option;
}

//=============================================================================
static int
cm_maclist_add(
    struct cfparse_wifi_interface *vif,
    const char *mac,
    const char *ifname
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };
    char *output = NULL;
    int   size   = 0;

    if ( vif->maclist ) {
        if ( strstr( vif->maclist, mac ) )
            return 0;

        size = strlen( vif->maclist ) + strlen( mac ) + 2;

        output = calloc( 1, size );
        if ( !output )
            return -1;

        snprintf( output, size, "%s %s", vif->maclist, mac );
        free( vif->maclist );
    }
    else {
        size = strlen( mac ) + 1;

        output = calloc( 1, size );
        if ( !output )
            return -1;

        snprintf( output, size, "%s", mac );
    }

    vif->maclist = output;

    snprintf( path, sizeof( path ), "wireless.%s.maclist", ifname );
    config_uci_set( path, output, 0 );

    return 0;
}

//=============================================================================
static int
cm_maclist_del(
    struct cfparse_wifi_interface *vif,
    const char *mac,
    const char *ifname
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };
    char *output = NULL;
    char  substr[32];
    int   size   = 0;
    int   len    = 0;

    if ( !vif->maclist )
        return 0;

    size = strlen( vif->maclist ) + 2;

    output = calloc( 1, size );
    if ( !output )
        return -1;

    snprintf( output, size, "%s ", vif->maclist );
    snprintf( substr, sizeof( substr ), "%s ", mac );

    output = util_rm_substr( output, substr );
    len = strlen( output );

    if ( len && output[len - 1] == ' ' )
        output[len - 1] = '\0';

    snprintf( path, sizeof( path ), "wireless.%s.maclist", ifname );
    config_uci_set( path, output, 0 );

    free( vif->maclist );
    vif->maclist = NULL;

    if ( len )
        vif->maclist = output;
    else
        free( output );

    return 0;
}

//=============================================================================
static int
cm_access_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct cfparse_wifi_device *cfwdev;
    struct cfparse_wifi_interface *vif;

    const char *mac = NULL;
    int block = 0;
    int black = 0;
    int white = 0;

    bool firewall_changed = false;
    bool wireless_changed = false;

    int option = 0;

    int action = extend->action;

    mac = util_blobmsg_get_string( new_config[CM_PARSE_ACCESS_MAC], "" );

    white = util_blobmsg_get_int( new_config[CM_PARSE_ACCESS_WHITE], 0 );
    black = util_blobmsg_get_int( new_config[CM_PARSE_ACCESS_BLACK], 0 );
    block = util_blobmsg_get_int( new_config[CM_PARSE_ACCESS_BLOCK], 0 );

    cfmanager_log_message( L_DEBUG,
        "mac %s, white %d, black %d \n", mac, white, black );

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            if ( block ) {
                //delete rule
                firewall_changed = true;
            }

            if ( white || black ) {
                vlist_for_each_element( &wireless_vltree, cfwdev, node ) {
                    vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
                        cfmanager_log_message( L_DEBUG,
                            "ifname %s, macfilter %s, maclist %s \n",
                            vif->cf_section.name, vif->macfilter, vif->maclist );

                        if ( !vif->cf_section.name )
                            continue;

                        if ( ( white && strcmp( vif->macfilter, "allow" ) == 0 )
                            || ( black && strcmp( vif->macfilter, "deny" )== 0 ) ) {
                            cm_maclist_del( vif, mac, vif->cf_section.name );
                            wireless_changed = true;
                        }
                    }
                }
            }

            extend->set_compl_option |= BIT( BIT_MAX );

            break;
        case VLTREE_ACTION_UPDATE:
            switch( index ) {
                case CM_PARSE_ACCESS_BLOCK:
                    firewall_changed = true;
                    break;
                case CM_PARSE_ACCESS_WHITE:
                    vlist_for_each_element( &wireless_vltree, cfwdev, node ) {
                        vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
                            cfmanager_log_message( L_DEBUG,
                                "ifname %s, macfilter %s, maclist %s \n",
                                vif->cf_section.name, vif->macfilter, vif->maclist );

                            if ( !vif->cf_section.name )
                                continue;

                            if ( strcmp( vif->macfilter, "allow" ) == 0 ) {
                                if ( white ) {
                                    cm_maclist_add( vif, mac, vif->cf_section.name );
                                }
                                else {
                                    cm_maclist_del( vif, mac, vif->cf_section.name );
                                }

                                wireless_changed = true;
                            }
                        }
                    }
                    break;
                case CM_PARSE_ACCESS_BLACK:
                    vlist_for_each_element( &wireless_vltree, cfwdev, node ) {
                        vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
                            cfmanager_log_message( L_DEBUG,
                                "ifname %s, macfilter %s, maclist %s \n",
                                vif->cf_section.name, vif->macfilter, vif->maclist );

                            if ( !vif->cf_section.name )
                                continue;

                            if ( strcmp( vif->macfilter, "deny" ) == 0 ) {
                                if ( black ) {
                                    cm_maclist_add( vif, mac, vif->cf_section.name );
                                }
                                else {
                                    cm_maclist_del( vif, mac, vif->cf_section.name );
                                }

                                wireless_changed = true;
                            }

                        }
                    }
                    break;
                default:
                    break;
            }
            break;
        case VLTREE_ACTION_ADD:
            if ( block ) {
                //add rule
                firewall_changed = true;
            }
            if ( white || black ) {
                vlist_for_each_element( &wireless_vltree, cfwdev, node ) {
                    vlist_for_each_element( &cfwdev->interfaces, vif, node ) {

                        cfmanager_log_message( L_DEBUG,
                            "ifname %s, macfilter %s, maclist %s \n",
                            vif->cf_section.name, vif->macfilter, vif->maclist );

                        if ( !vif->cf_section.name )
                            continue;

                        if ( ( white && strcmp( vif->macfilter, "allow" ) == 0 )
                            || ( black && strcmp( vif->macfilter, "deny" )== 0 ) ) {
                            cm_maclist_add( vif, mac, vif->cf_section.name );

                            wireless_changed = true;
                        }
                    }
                }
            }

            extend->set_compl_option |= BIT( BIT_MAX );
            break;
    }

    if ( firewall_changed ) {
        cfmanager_log_message( L_DEBUG, "Firewall has been changed\n" );
        cfparse_load_firewall();
    }

    if ( wireless_changed )
        option |= BIT( CM_CFG_WIRELESS );

    return option;
}

//=============================================================================
static int
cm_global_access_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    const char *zone_blackhole_section_name = "zone_blackhole0";
    const char *blackhole_policy_section_name = "blackhole_global_access_policy0";
    int action = extend->action;

    char path[LOOKUP_STR_SIZE] = { 0 };
    bool enable = false;
    int  forbid_url_num = 0;
    unsigned int rem;
    struct blob_attr *cur;

    int option = 0;
 
    switch ( action ) {
        case VLTREE_ACTION_DEL:
            uci_clean_package( "/etc/config/blackhole" );
            uci_clean_package( SPLICE_STR(CM_CONFIG_PATH,/blackhole) );
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            uci_clean_package( "/etc/config/blackhole" );
            uci_clean_package( SPLICE_STR(CM_CONFIG_PATH,/blackhole) );

            enable = util_blobmsg_get_bool( new_config[GLOBAL_ACCESS_ATTR_ENABLE], false );
            if ( !enable ) {
                break;
            }

            if( !new_config[GLOBAL_ACCESS_ATTR_FORBIDURL] ) {
                break;
            }

            config_add_named_section( "blackhole", "blackhole_policy", blackhole_policy_section_name );
            snprintf( path, sizeof( path ), "blackhole.%s.bad_domains", blackhole_policy_section_name );

            blobmsg_for_each_attr( cur, new_config[GLOBAL_ACCESS_ATTR_FORBIDURL], rem ) {
                if ( !cur )
                    continue;
                config_uci_add_list( path, blobmsg_get_string( cur ), 0 );
                forbid_url_num++;
            }

            if ( forbid_url_num == 0 ) {
                break;
            }

            config_add_named_section( "blackhole", "zone_blackhole", zone_blackhole_section_name );

            snprintf( path, sizeof( path ), "blackhole.%s.port", zone_blackhole_section_name );
            cfmanager_log_message( L_DEBUG, "path %s \n", path );
            config_uci_set( path, "5353", 0 );


            snprintf( path, sizeof( path ), "blackhole.%s.zones", zone_blackhole_section_name );
            cfmanager_log_message( L_DEBUG, "path %s \n", path );
            config_uci_add_list( path, (char *)"zone0", 0 );

            snprintf( path, sizeof( path ), "blackhole.%s.blackhole_policies", zone_blackhole_section_name );
            cfmanager_log_message( L_DEBUG, "path %s \n", path );
            config_uci_add_list( path, (char *)blackhole_policy_section_name, 0 );

            break;
    }

    extend->set_compl_option |= BIT( BIT_MAX );

    option |= BIT( CM_CFG_BLACKHOLE );

    return option;
}

//=============================================================================
static int
cm_schedule_access_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int action = extend->action;

    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_64] = { 0 };

    bool enable = false;

    char *substr = NULL;
    char *save_ptr = NULL;

    char week[BUF_LEN_16] = { 0 };
    char time_s[BUF_LEN_8] = { 0 };
    char time_e[BUF_LEN_8] = { 0 };
    const char *section_name = "schedule_access0";

    int day = 0;

    int option = 0;

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            config_del_named_section( "schedule", "schedule", section_name );
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            config_del_named_section( "schedule", "schedule", section_name );

            enable = util_blobmsg_get_bool( new_config[SCHEDULE_ACCESS_ATTR_ENABLE], false );
            if ( !enable )
                break;

            config_add_named_section( "schedule", "schedule", section_name );

            strncpy( week,
                util_blobmsg_get_string( new_config[SCHEDULE_ACCESS_ATTR_WEEK], "" ),
                sizeof( week ) - 1 );
            if ( week[0] == '\0' )
                break;

            strncpy( time_s,
                util_blobmsg_get_string( new_config[SCHEDULE_ACCESS_ATTR_TIMESTART], "" ),
                sizeof( time_s ) - 1 );
            if ( time_s[0] == '\0' )
                break;

            util_rm_substr( time_s, ":" );

            strncpy( time_e,
                util_blobmsg_get_string( new_config[SCHEDULE_ACCESS_ATTR_TIMEEND], "" ),
                sizeof( time_e ) - 1 );
            if ( time_e[0] == '\0' )
                break;

            util_rm_substr( time_e, ":" );


            substr = strtok_r( week, ",", &save_ptr );
            while ( substr ) {
                day = atoi( substr );
                if ( day == 0 )
                    day = 7;

                snprintf( path, sizeof( path ), "schedule.%s.wtime%d", section_name, day );
                snprintf( value, sizeof( value ), "%s-%s", time_s, time_e );

                config_uci_set( path, value, 0 );

                substr = strtok_r( NULL, ",", &save_ptr );
            }

            snprintf( path, sizeof( path ), "schedule.%s.firewall", section_name );
            config_uci_set( path, "schedule_access", 0 );


            break;
    }
    extend->set_compl_option |= BIT( BIT_MAX );

    option |= BIT( CM_CFG_SCHEDULE );

    return option;
}

//=============================================================================
static int
cm_controller_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };
//    char value[BUF_LEN_64] = { 0 };
    struct blob_attr *cur_attr = new_config[index];
    int action = extend->action;
    int option = 0;

    cfmanager_log_message( L_DEBUG, "line=%d, action=%d, config[%d]=%s",
        __LINE__, action, index, util_blobmsg_get_string( cur_attr, "" ) );

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case CONTROLLER_ROLE:
                    snprintf( path, sizeof( path ), "%s.main.%s", CF_CONFIG_NAME_CONTROLLER, controller_policy[index].name );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    option |= BIT( CM_CFG_CONTROLLER );
                    break;
                case CONTROLLER_WORK_MODE:
                    snprintf( path, sizeof( path ), "%s.main.%s", CF_CONFIG_NAME_CONTROLLER, controller_policy[index].name );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );
                    option |= BIT( CM_CFG_CONTROLLER );

                    //AP mode we need delete wan and add eth4 interface to bridge mode
                    if ( !strcmp( util_blobmsg_get_string( cur_attr, "" ), "1" ) ) {
                        config_uci_set( "network.lan0_zone0.ifname", "eth0 eth1 eth2 eth3 eth4", 0 );
                        config_uci_set( "network.lan0_zone0.proto", "dhcp", 0 );
                        config_uci_del( "network.lan0_zone0.ipaddr", 0 );
                        config_uci_del( "network.lan0_zone0.netmask", 0 );
                        config_uci_del( "network.wan0", 0 );

                        device_info.work_mode = MODE_AP;
                        option |= BIT( CM_CFG_NETWORK );
                    }
                    else {
                        device_info.work_mode = MODE_ROUTER;
                    }

                    option |= BIT( CM_CFG_CONTROLLER );
                    break;

                default:
                    break;
            }
            break;
        default:
            break;
    }

    return option;
}

//=============================================================================
static int
cm_vpn_client_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    char section_name[BUF_LEN_64] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    char metric[BUF_LEN_16]= { 0 };
    char *proto = NULL;
    char *server = NULL;
    char *username = NULL;
    char *password = NULL;
    int create = 0;
    int option = 0;
    int vpn_id;
    char *new_wan = NULL, *old_wan = NULL;
    char mtu_str[BUF_LEN_16] = { 0 };
    int mtu;
    int new_client_type, old_client_type;
    char *ipsec_conn;
    int new_wan_index, old_wan_index;
    int enable;
    int enable_change = 0, type_change = 0, wan_change = 0;

    vpn_id = util_blobmsg_get_int(config[VPN_CLIENT_ID], 0) + VPN_CLIENT_ID_OFFSET;
    snprintf( section_name, sizeof(section_name), "vpn%d", vpn_id );
    new_client_type = util_blobmsg_get_int( config[VPN_CLIENT_TYPE], VPN_CLIENT_TYPE_L2TP );
    ipsec_conn = (char *)extend->section_name; // Be the same with vpn serice seciton name;
    new_wan = blobmsg_get_string( config[VPN_CLIENT_WAN_INTF] );
    new_wan_index = !strcmp( new_wan, "wan0" ) ? WAN0 : WAN1;
    enable = util_blobmsg_get_bool( config[VPN_CLIENT_ENABLE], false );

    switch ( extend->action ) {
        case VLTREE_ACTION_DEL:
            // Only delete already enabled vpn.
            extend->set_compl_option |= BIT( BIT_MAX );

            if ( enable ) {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    vpn_update_ipsec_cnt_and_reload_helper(new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_DISABLE,
                        ipsec_conn );
                }
                config_del_named_section( "network", "interface", section_name );
            }
            else {
                return option;
            }
            break;

        case VLTREE_ACTION_ADD:
            // Only add enabled vpn, ipsec always enabled for GWN7052.
            extend->set_compl_option |= BIT( BIT_MAX );
            if ( enable ) {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    vpn_update_ipsec_cnt_and_reload_helper(new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_ENABLE,
                        ipsec_conn );
                }

                create = 1;
            }
            else {
                return option;
            }
            break;

        case VLTREE_ACTION_UPDATE:
            switch ( index ) {
                case VPN_CLIENT_NAME:
                    // It doesn't matter if service name changed.
                    return option;

                case VPN_CLIENT_ENABLE:
                    if ( enable ) {
                        create = 1;
                    }
                    else {
                        config_del_named_section( "network", "interface", section_name );
                    }

                    enable_change = 1;
                    extend->set_compl_option |= BIT( BIT_MAX );
                    // go through

                case VPN_CLIENT_TYPE:
                    old_client_type = util_blobmsg_get_int( extend->old_config[VPN_CLIENT_TYPE], 0 );
                    if ( new_client_type != old_client_type ) {
                        if ( !enable_change ) { // enable changed will adjust network config.
                            if ( new_client_type == VPN_CLIENT_TYPE_L2TP ) {
                                if ( old_client_type == VPN_CLIENT_TYPE_PPTP ) {
                                    // delete mppe option
                                    snprintf( path, sizeof(path), "network.%s.mppe", section_name );
                                    config_uci_del( path, 0 );
                                }
                                else { // VPN_CLIENT_TYPE_IPSEC
                                     // delete ifname option
                                    snprintf( path, sizeof(path), "network.%s.ifname", section_name );
                                    config_uci_del( path, 0 );
                                }
                            
                                proto = "l2tp";
                                snprintf( path, sizeof(path), "network.%s.proto", section_name );
                                config_uci_set( path, proto, 0 );
                                snprintf( path, sizeof(path), "network.%s.checkup_interval", section_name );
                                config_uci_set( path, "10", 0 );
                                snprintf( path, sizeof(path), "network.%s.server", section_name );
                                config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_L2TP_SERVER],""), 0 );
                                snprintf( path, sizeof(path), "network.%s.username", section_name );
                                config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_L2TP_USER],""), 0 );
                                snprintf( path, sizeof(path), "network.%s.password", section_name );
                                config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_L2TP_PASSWORD],""), 0 );
                            }
                            else if ( new_client_type == VPN_CLIENT_TYPE_PPTP ) {
                                if ( old_client_type == VPN_CLIENT_TYPE_L2TP ) {
                                    // delete checkup_interval option
                                    snprintf( path, sizeof(path), "network.%s.checkup_interval", section_name );
                                    config_uci_del( path, 0 );
                                }
                                else { // VPN_CLIENT_TYPE_IPSEC
                                    // delete ifname option
                                    snprintf( path, sizeof(path), "network.%s.ifname", section_name );
                                    config_uci_del( path, 0 );
                                }
                            
                                proto = "pptp";
                                snprintf( path, sizeof(path), "network.%s.proto", section_name );
                                config_uci_set( path, proto, 0 );
                                snprintf( path, sizeof(path), "network.%s.mppe", section_name );
                                config_uci_set( path, "0", 0 );
                                snprintf( path, sizeof(path), "network.%s.server", section_name );
                                config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_PPTP_SERVER],""), 0 );
                                snprintf( path, sizeof(path), "network.%s.username", section_name );
                                config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_PPTP_USER],""), 0 );
                                snprintf( path, sizeof(path), "network.%s.password", section_name );
                                config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_PPTP_PASSWORD],""), 0 );
                            }
                            else {
                                if ( old_client_type == VPN_CLIENT_TYPE_L2TP ) {
                                    // delete checkup_interval option
                                    snprintf( path, sizeof(path), "network.%s.checkup_interval", section_name );
                                    config_uci_del( path, 0 );
                                }
                                else {
                                    // delete mppe option
                                    snprintf( path, sizeof(path), "network.%s.mppe", section_name );
                                    config_uci_del( path, 0 );
                                }
                            
                                snprintf( path, sizeof(path), "network.%s.server", section_name );
                                config_uci_del( path, 0 );
                                snprintf( path, sizeof(path), "network.%s.username", section_name );
                                config_uci_del( path, 0 );
                                snprintf( path, sizeof(path), "network.%s.password", section_name );
                                config_uci_del( path, 0 );
                                snprintf( path, sizeof(path), "network.%s.mtu", section_name );
                                config_uci_del( path, 0 );
                            
                                // Add ipsec network config
                                snprintf( path, sizeof(path), "network.%s.proto", section_name );
                                config_uci_set( path, "none", 0 );
                                snprintf( path, sizeof(path), "network.%s.ifname", section_name );
                                if ( new_wan_index == WAN0 ) {
                                    config_uci_set( path, "ipsec0", 0 );
                                }
                                else {
                                    config_uci_set( path, "ipsec1", 0 );
                                }
                            }
                        }

                        type_change = 1;
                        extend->set_compl_option |= BIT( BIT_MAX );
                    }
                    // go through

                case VPN_CLIENT_WAN_INTF:
                    old_wan = util_blobmsg_get_string( extend->old_config[VPN_CLIENT_WAN_INTF], "" );
                    old_wan_index = !strcmp( old_wan, "wan0" ) ? WAN0 : WAN1;

                    if ( new_wan_index != old_wan_index ) {
                        if ( !enable_change ) { // enable changed will adjust network config.
                            sprintf( path, "network.%s.interface", section_name );
                            config_uci_set( path, new_wan, 0 );
                            if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                                snprintf( path, sizeof(path), "network.%s.ifname", section_name );
                                if ( new_wan_index == WAN0 ) {
                                    config_uci_set( path, "ipsec0", 0 );
                                }
                                else {
                                    config_uci_set( path, "ipsec1", 0 );
                                }
                            }
                        }

                        wan_change = 1;
                        extend->set_compl_option |= BIT( BIT_MAX );
                    }

                    // Adjust pptp/l2tp vpn interface mtu
                    if ( new_client_type == VPN_CLIENT_TYPE_L2TP ||
                        new_client_type == VPN_CLIENT_TYPE_PPTP ) {
                        if ( !enable_change && (type_change || wan_change) ) {
                            snprintf( path, sizeof(path), "network.%s.mtu", new_wan );
                            if ( config_uci_get_option(path, mtu_str, sizeof(mtu_str)) ) {
                                cfmanager_log_message( L_ERR, "Missing mtu in interface %s, treat it as 1500!\n", new_wan );
                                mtu = 1500;
                            }
                            else {
                                mtu = strtoul( mtu_str, NULL, 10 );
                            }
                            
                            if ( util_blobmsg_get_int(config[VPN_CLIENT_TYPE],0) == VPN_CLIENT_TYPE_L2TP ) {
                                mtu = mtu - 40;
                            }
                            else {
                                mtu = mtu - 50;
                            }
                            
                            snprintf( path, sizeof(path), "network.%s.mtu", section_name );
                            memset( mtu_str, 0, sizeof(mtu_str) );
                            snprintf( mtu_str, sizeof(mtu_str), "%d", mtu );
                            config_uci_set( path, mtu_str, 0 );
                        }
                    }
                    break;

                    case VPN_CLIENT_L2TP_SERVER:
                    case VPN_CLIENT_L2TP_USER:
                    case VPN_CLIENT_L2TP_PASSWORD:
                        snprintf( path, sizeof(path), "network.%s.server", section_name );
                        config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_L2TP_SERVER],""), 0 );
                        snprintf( path, sizeof(path), "network.%s.username", section_name );
                        config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_L2TP_USER],""), 0 );
                        snprintf( path, sizeof(path), "network.%s.password", section_name );
                        config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_L2TP_PASSWORD],""), 0 );
                    
                        extend->set_compl_option |= BIT(VPN_CLIENT_L2TP_SERVER);
                        extend->set_compl_option |= BIT(VPN_CLIENT_L2TP_USER);
                        extend->set_compl_option |= BIT(VPN_CLIENT_L2TP_PASSWORD);
                        break;
                    
                    case VPN_CLIENT_PPTP_SERVER:
                    case VPN_CLIENT_PPTP_USER:
                    case VPN_CLIENT_PPTP_PASSWORD:
                        snprintf( path, sizeof(path), "network.%s.server", section_name );
                        config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_PPTP_SERVER],""), 0 );
                        snprintf( path, sizeof(path), "network.%s.username", section_name );
                        config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_PPTP_USER],""), 0 );
                        snprintf( path, sizeof(path), "network.%s.password", section_name );
                        config_uci_set( path, util_blobmsg_get_string(config[VPN_CLIENT_PPTP_PASSWORD],""), 0 );
                    
                        extend->set_compl_option |= BIT(VPN_CLIENT_PPTP_SERVER);
                        extend->set_compl_option |= BIT(VPN_CLIENT_PPTP_USER);
                        extend->set_compl_option |= BIT(VPN_CLIENT_PPTP_PASSWORD);
                        break;

                case VPN_CLIENT_IPSEC_P1_REMOTE_GW:
                case VPN_CLIENT_IPSEC_P1_IKE_VER:
                case VPN_CLIENT_IPSEC_P1_IKE_LIFETIME:
                case VPN_CLIENT_IPSEC_P1_NEGO_MODE:
                case VPN_CLIENT_IPSEC_P1_PSK:
                case VPN_CLIENT_IPSEC_P1_ENCRYPT:
                case VPN_CLIENT_IPSEC_P1_AUTH:
                case VPN_CLIENT_IPSEC_P1_DH:
                case VPN_CLIENT_IPSEC_P1_REKEY:
                case VPN_CLIENT_IPSEC_P1_KEYINGTRIES:
                case VPN_CLIENT_IPSEC_P1_DPD_EN:
                case VPN_CLIENT_IPSEC_P1_DPD_DELAY:
                case VPN_CLIENT_IPSEC_P1_DPD_IDLE:
                case VPN_CLIENT_IPSEC_P1_DPD_ACTION:
                case VPN_CLIENT_IPSEC_P2_LOCAL_SUBNET:
                case VPN_CLIENT_IPSEC_P2_LOCAL_SOURCE_IP:
                case VPN_CLIENT_IPSEC_P2_REMOTE_SUBNET_LIST:
                case VPN_CLIENT_IPSEC_P2_SA_LIFETIME:
                case VPN_CLIENT_IPSEC_P2_PROTO:
                case VPN_CLIENT_IPSEC_P2_ESP_ENCRYPT:
                case VPN_CLIENT_IPSEC_P2_ESP_AUTH:
                case VPN_CLIENT_IPSEC_P2_ENCAP_MODE:
                case VPN_CLIENT_IPSEC_P2_PFS_GROUP:
                    // Anything changed for ipsec, we always need to reload ipsec connection.
                    vpn_update_ipsec_reload_helper( IPSEC_RELOAD_ACTION_REPLACE_AND_UP_CONN, 
                        ipsec_conn );

                    extend->set_compl_option |= BIT( BIT_MAX );
                    break;
            }
            break;
    }

    if ( enable_change || type_change || wan_change ) {
        // Do permutation and combination between these changes.
        if ( enable_change && type_change && wan_change ) {
            if ( enable ) {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn client again, change client type from others to ipsec, "
                        "change wan from %s to %s!\n",
                        old_wan, new_wan ); 
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_ENABLE,
                        ipsec_conn );
                }
                // else {}
            }
            else {
                if ( old_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Diabled vpn client again, change client type from ipsec to others, "
                        "change wan from %s to %s!\n",
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_DISABLE,
                        ipsec_conn );
                }
                // else {}
            }
        }
        else if ( enable_change && type_change ) {
            if ( enable ) {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn client again, change client type from others to ipsec!\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_ENABLE,
                        ipsec_conn );
                }
                // else {}
            }
            else {
                if ( old_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Disabled vpn client again, change client type from ipsec to other!\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_DISABLE,
                        ipsec_conn );
                }
                // else {}
            }
        }
        else if ( enable_change && wan_change ) {
            if ( enable ) {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled ipsec client again, change wan from %s to %s!\n", 
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_ENABLE,
                        ipsec_conn );
                }
                // else {}
            }
            else {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Disabled ipsec client again, change wan from %s to %s!\n", 
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_DISABLE,
                        ipsec_conn );
                }
                // else {}
            }
        }
        else if ( enable_change ) {
            if ( enable ) {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled ipsec client again!\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_ENABLE,
                        ipsec_conn );
                }
                // else {}
            }
            else {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Disabled ipsec client again!\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_DISABLE,
                        ipsec_conn );
                }
                // else {}
            }
        }
        else if ( type_change && wan_change ) {
            if ( enable ) {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn client, change client type from other to ipsec, "
                        "change wan from %s to %s!\n", 
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_ENABLE,
                        ipsec_conn );
                }
                else if ( old_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn client, change client type from ipsec to other, "
                        "change wan from %s to %s!\n", 
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_DISABLE,
                        ipsec_conn );
                }
                // else {}
            }
            // Don't care 'Disabled';
        }
        else if ( type_change ) {
            if ( enable ) {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn client, change client type from other to ipsec\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_ENABLE,
                        ipsec_conn );
                }
                else if ( old_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn client, change client type from ipsec to other\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_DISABLE,
                        ipsec_conn );
                }
                // else {}
            }
            // Don't care 'Disabled'
        }
        else if ( wan_change ) {
            if ( enable ) {
                if ( new_client_type == VPN_CLIENT_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled ipsec client, change wan from %s to %s\n",
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_DISABLE,
                        ipsec_conn );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_CLIENT,
                        IPSEC_STATUS_ENABLE,
                        ipsec_conn );
                }
                // else {}
            }
            // Don't care 'Disabled'
        }
    }

    if ( create ) {
        config_add_named_section( "network", "interface", section_name );
        snprintf( metric, sizeof(metric), "%d", vpn_id + VPN_CLIENT_METRIC_OFFSET ); // metric 6x
        snprintf( path, sizeof(path), "network.%s.metric", section_name );
        config_uci_set( path, metric, 0 );
        sprintf( path, "network.%s.interface", section_name );
        config_uci_set( path, new_wan, 0 );

        if ( new_client_type == VPN_CLIENT_TYPE_L2TP || new_client_type == VPN_CLIENT_TYPE_PPTP ) {
            snprintf( path, sizeof(path), "network.%s.mtu", new_wan );
            if ( config_uci_get_option(path, mtu_str, sizeof(mtu_str)) ) {
                cfmanager_log_message( L_ERR, "Missing mtu in interface %s, treat it as 1500!\n", new_wan );
                mtu = 1500;
            }
            else {
                mtu = strtoul( mtu_str, NULL, 10 );
            }
    
            if ( new_client_type == VPN_CLIENT_TYPE_L2TP ) {
                proto = "l2tp";
                server = util_blobmsg_get_string( config[VPN_CLIENT_L2TP_SERVER], "" );
                username = util_blobmsg_get_string( config[VPN_CLIENT_L2TP_USER], "" );
                password = util_blobmsg_get_string( config[VPN_CLIENT_L2TP_PASSWORD], "" );
                snprintf( path, sizeof(path), "network.%s.checkup_interval", section_name );
                config_uci_set( path, "10", 0 );
                mtu = mtu - 40;
            }
            else {
                proto = "pptp";
                server = util_blobmsg_get_string( config[VPN_CLIENT_PPTP_SERVER], "" );
                username = util_blobmsg_get_string( config[VPN_CLIENT_PPTP_USER], "" );
                password = util_blobmsg_get_string( config[VPN_CLIENT_PPTP_PASSWORD], "" );
                snprintf( path, sizeof(path), "network.%s.mppe", section_name );
                config_uci_set( path, "0", 0 );
                mtu = mtu - 50;
            }
    
            memset( mtu_str, 0, sizeof(mtu_str) );
            snprintf( mtu_str, sizeof(mtu_str), "%d", mtu );
            snprintf( path, sizeof(path), "network.%s.mtu", section_name );
            config_uci_set( path, mtu_str, 0 );
            snprintf( path, sizeof(path), "network.%s.proto", section_name );
            config_uci_set( path, proto, 0 );
            snprintf( path, sizeof(path), "network.%s.server", section_name );
            config_uci_set( path, server, 0 );
            snprintf( path, sizeof(path), "network.%s.username", section_name );
            config_uci_set( path, username, 0 );
            snprintf( path, sizeof(path), "network.%s.password", section_name );
            config_uci_set( path, password, 0 );
            snprintf( path, sizeof(path), "network.%s.delegate", section_name );
            config_uci_set( path, "0", 0 );
            snprintf( path, sizeof(path), "network.%s.ipv6", section_name );
            config_uci_set( path, "0", 0 );
            //snprintf( path, sizeof(path), "network.%s.gs_block_restart", section_name );
            //config_uci_set( path, "1", 0 );
        }
        else {
            snprintf( path, sizeof(path), "network.%s.proto", section_name );
            config_uci_set( path, "none", 0 );
            snprintf( path, sizeof(path), "network.%s.ifname", section_name );
            if ( new_wan_index == WAN0 ) {
                config_uci_set( path, "ipsec0", 0 );
            }
            else {
                config_uci_set( path, "ipsec1", 0 );
            }
        }
    }

    option |= BIT( CM_CFG_NETWORK );

    return option;
}

//=============================================================================
static int
cm_vpn_split_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    char path[BUF_LEN_64] = { 0 };
    char cmd[BUF_LEN_128] = { 0 };
    char ipset_nm[BUF_LEN_512] = { 0 };
    int option = 0;
    char *mode = NULL;
    struct blob_attr *cur;
    int rem;
    int add_all_ipset = 0, del_all_ipset = 0;

    switch ( extend->action ) {
        case VLTREE_ACTION_DEL: {
            mode = util_blobmsg_get_string( config[VPN_SPLIT_MODE], "" );
            if ( mode[0] == '1' ) {
                // Split tunneling by dev.
                extend->set_compl_option |= BIT( BIT_MAX );
                return option;
            }

            del_all_ipset = 1;
            extend->set_compl_option |= BIT( BIT_MAX );
            break;
        }

        case VLTREE_ACTION_ADD: {
            mode = util_blobmsg_get_string( config[VPN_SPLIT_MODE], "" );
            if ( mode[0] == '1' ) {
                // Split tunneling by dev.
                extend->set_compl_option |= BIT( BIT_MAX );
                return option;
            }

            add_all_ipset = 1;
            extend->set_compl_option |= BIT( BIT_MAX );
            break;
        }

        case VLTREE_ACTION_UPDATE: 
            switch ( index ) {
                case VPN_SPLIT_MODE: {
                    mode = util_blobmsg_get_string( config[index], "" );
                    if ( mode[0] == '0' ) {
                        // Change mode from dev to service addr,
                        // should add ipset configure in dnsmasq.
                        add_all_ipset = 1;
                    }
                    else {
                        // Change mode from serivce addr to dev,
                        // should delete ipset configure in dnsmasq.
                        del_all_ipset = 1;
                    }

                    extend->set_compl_option |= BIT( BIT_MAX );
                    break;
                }

                case VPN_SPLIT_SERVICE_ADDR_LIST: {
                    struct blob_attr *new_attr, *old_attr;
                    int new_rem, old_rem;
                    struct dhcp_config_parse *dnsmasq_cfg = NULL;
                    struct blob_attr *tb[__DHCP_DNSMASQ_MAX] = { 0 };
                    uint32_t ip;
                    int found = 0;

                    mode = util_blobmsg_get_string( config[VPN_SPLIT_MODE], "" );
                    if ( mode[0] == '1' ) {
                        return option;
                    }

                    dnsmasq_cfg = util_get_vltree_node( &dhcp_dnsmasq_vltree, VLTREE_DHCP, "dnsmasq" );
                    if ( !dnsmasq_cfg ) {
                        cfmanager_log_message( L_DEBUG, "Missing dnsmasq information in dhcp configure\n" );
                        extend->set_compl_option |= BIT( BIT_MAX );
                        return option;
                    }

                    blobmsg_parse( dhcp_dnsmasq_policy,
                        __DHCP_DNSMASQ_MAX,
                        tb,
                        blob_data( dnsmasq_cfg->cf_section.config ),
                        blob_len( dnsmasq_cfg->cf_section.config ) );

                    snprintf( path, sizeof(path), "dhcp.dnsmasq.ipset" );
                    // Check if need to add new ipset in dnsmasq.
                    blobmsg_for_each_attr( new_attr, config[VPN_SPLIT_SERVICE_ADDR_LIST], new_rem ) {
                        if ( !strchr(util_blobmsg_get_string(new_attr, ""), '/') && 
                                inet_pton(AF_INET, util_blobmsg_get_string(new_attr, ""), &ip) != 1 ) {
                            found = 0;

                            if ( tb[DHCP_DNSMASQ_IPSET] ) {
                                blobmsg_for_each_attr( old_attr, tb[DHCP_DNSMASQ_IPSET], old_rem ) {
                                    if ( strstr(util_blobmsg_get_string(old_attr,""),
                                            util_blobmsg_get_string(new_attr,"")) ) {
                                        found = 1;
                                        break;
                                    }
                                }
                            }

                            if ( !found ) {
                                 // Add ipset in dnsmasq
                                 snprintf( ipset_nm, sizeof(ipset_nm), "/%s/vpn_ipset_%s",
                                    util_blobmsg_get_string(new_attr, ""),
                                    util_blobmsg_get_string(config[VPN_SPLIT_SERVICE_ID], "") );
                                 cfmanager_log_message( L_DEBUG, "Add path=%s, value=%s\n", path, ipset_nm );
                                 config_uci_add_list( path, ipset_nm, 0 );
                                 option |= BIT(CM_CFG_DHCP);
                             }
                        }
                    }

                    // Check if need to delete old ipset in dnsmasq. 
                    snprintf( ipset_nm, sizeof(ipset_nm), "vpn_ipset_%s",
                        util_blobmsg_get_string(config[VPN_SPLIT_SERVICE_ID], "") );
                    if ( tb[DHCP_DNSMASQ_IPSET] ) {
                        blobmsg_for_each_attr( old_attr, tb[DHCP_DNSMASQ_IPSET], old_rem ) {
                            if ( !strstr(util_blobmsg_get_string(old_attr, ""), ipset_nm) ) {
                                continue;
                            }

                            found = 0;
                            blobmsg_for_each_attr( new_attr, config[VPN_SPLIT_SERVICE_ADDR_LIST], new_rem ) {
                                if ( !strchr(util_blobmsg_get_string(new_attr, ""), '/') &&
                                        inet_pton(AF_INET, util_blobmsg_get_string(new_attr, ""), &ip) != 1 ) {
                                    if ( strstr(util_blobmsg_get_string(old_attr, ""),
                                            util_blobmsg_get_string(new_attr, "")) ) {
                                        found = 1;
                                        break;
                                    }
                                }
                            } 
    
                            if ( !found ) {
                                // Delete ipset in dnsmasq
                                snprintf( path, sizeof(path), "dhcp.dnsmasq.ipset" );
                                cfmanager_log_message( L_DEBUG, "Delete path=%s, value=%s\n",
                                    path, util_blobmsg_get_string(old_attr, "") );
                                config_uci_del_list( path, util_blobmsg_get_string(old_attr, ""), 0 );
                                option |= BIT(CM_CFG_DHCP);
                            }
                        }
                    }
                    break;
                }
            }
            break;
    }

    if ( del_all_ipset ) {
        struct dhcp_config_parse *dnsmasq_cfg = NULL;

        // Destroy vpn_ipset
        snprintf( ipset_nm, sizeof(ipset_nm), "vpn_ipset_%s",
            util_blobmsg_get_string(config[VPN_SPLIT_SERVICE_ID], "") );
        snprintf( cmd, sizeof(cmd), "ipset -q destroy %s", ipset_nm );
        system( cmd );

        dnsmasq_cfg = util_get_vltree_node( &dhcp_dnsmasq_vltree, VLTREE_DHCP, "dnsmasq" );
        if ( dnsmasq_cfg ) {
            struct blob_attr *tb[__DHCP_DNSMASQ_MAX] = { 0 };

            blobmsg_parse( dhcp_dnsmasq_policy,
                __DHCP_DNSMASQ_MAX,
                tb,
                blob_data( dnsmasq_cfg->cf_section.config ),
                blob_len( dnsmasq_cfg->cf_section.config ) );

            snprintf( path, sizeof(path), "dhcp.dnsmasq.ipset" );
            if ( tb[DHCP_DNSMASQ_IPSET] ) {
                blobmsg_for_each_attr( cur, tb[DHCP_DNSMASQ_IPSET], rem ) {
                    if ( strstr(util_blobmsg_get_string(cur, ""), ipset_nm) ) {
                        cfmanager_log_message( L_DEBUG, "Delete path=%s, value=%s\n",
                            path, util_blobmsg_get_string(cur,"") );
                        config_uci_del_list( path, util_blobmsg_get_string(cur, ""), 0 );
                        option |= BIT(CM_CFG_DHCP);
                    }
                }
            }
        }
        else {
            cfmanager_log_message( L_DEBUG, "Missing dnsmasq information in dhcp configure\n" );
            extend->set_compl_option |= BIT( BIT_MAX );
            return option;
        }
    }

    if ( add_all_ipset ) {
        struct blob_attr *tb[__DHCP_DNSMASQ_MAX] = { 0 };
        struct dhcp_config_parse *dnsmasq_cfg = NULL;

        // Pre-create vpn_ipset_x no matter if the hostname in vpn split refer to it.
        snprintf( ipset_nm, sizeof(ipset_nm), "vpn_ipset_%s",
            util_blobmsg_get_string(config[VPN_SPLIT_SERVICE_ID], "") );
        snprintf( cmd, sizeof(cmd), "ipset -q create %s hash:ip", ipset_nm );
        system( cmd );

        dnsmasq_cfg = util_get_vltree_node( &dhcp_dnsmasq_vltree, VLTREE_DHCP, "dnsmasq" );
        if ( dnsmasq_cfg ) {
            blobmsg_parse( dhcp_dnsmasq_policy,
                __DHCP_DNSMASQ_MAX,
                tb,
                blob_data( dnsmasq_cfg->cf_section.config ),
                blob_len( dnsmasq_cfg->cf_section.config ) );
        }

        blobmsg_for_each_attr( cur, config[VPN_SPLIT_SERVICE_ADDR_LIST], rem ) {
            uint32_t ip;

            // Service addr is a hostname, should create corresponding ipset;
            if ( !strchr(util_blobmsg_get_string(cur,""), '/') &&
                    inet_pton(AF_INET, util_blobmsg_get_string(cur,""), &ip) != 1 ) {
                snprintf( ipset_nm, sizeof(ipset_nm), "/%s/vpn_ipset_%s",
                    util_blobmsg_get_string(cur,""), util_blobmsg_get_string(config[VPN_SPLIT_SERVICE_ID], "") );
                snprintf( path, sizeof(path), "dhcp.dnsmasq.ipset" );
                cfmanager_log_message( L_DEBUG, "Add path=%s, value=%s\n", path, ipset_nm );

                if ( tb[DHCP_DNSMASQ_IPSET] ) {
                    int found = 0;

                    blobmsg_for_each_attr( cur, tb[DHCP_DNSMASQ_IPSET], rem ) {
                        if ( !strcmp(util_blobmsg_get_string(cur, ""), ipset_nm) ) {
                            cfmanager_log_message( L_WARNING, "path=%s, value=%s has existed!!!\n", path, ipset_nm );
                            found = 1;
                            break;
                        }
                    }

                    if ( !found ) {
                        config_uci_add_list( path, ipset_nm, 0 );
                    }
                }
                else {
                    config_uci_add_list( path, ipset_nm, 0 );
                }

                option |= BIT(CM_CFG_DHCP);
            }
        }
    }
    return option;
}

//=============================================================================
static int
cm_mesh_ssid_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    static char iface_5g[BUF_LEN_8] = {0};
    int enable = 0;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int option = 0;
    char *id = MESH_SSID_ID;

    if ( !cur_attr ) {
        cfmanager_log_message( L_DEBUG, "cur_attr is null index=%d\n", index );
        return 0;
    }

    if ( iface_5g[0] == '\0' ) {
        config_get_ifname( iface_5g, sizeof(iface_5g), RADIO_5G, NET_TYPE_MESH, id );
    }
    cfmanager_log_message( L_DEBUG, "index=%d\n", index );

    enable = util_blobmsg_get_bool( new_config[MESH_SSID_ATTR_ENABLE], false );

    switch(index) {
    case MESH_SSID_ATTR_ENABLE:
        if ( enable == 1 && iface_5g[0] == '\0' ) {
            config_create_ssid( iface_5g, sizeof(iface_5g), RADIO_5G, NET_TYPE_MESH, id );

            extend->need_set_option |= BIT(MESH_SSID_ATTR_MODE);
            extend->need_set_option |= BIT(MESH_SSID_ATTR_SSID);
            extend->need_set_option |= BIT(MESH_SSID_ATTR_BSSID);
            extend->need_set_option |= BIT(MESH_SSID_ATTR_ENCRYPTION);
            extend->need_set_option |= BIT(MESH_SSID_ATTR_KEY);
            extend->need_set_option |= BIT(MESH_SSID_ATTR_ISADDED);
            extend->need_set_option |= BIT(MESH_SSID_ATTR_MASTER_ID);
        } else if ( enable == 0 && iface_5g[0] != '\0' ) {
            sprintf( path, "wireless.%s", iface_5g );
            config_uci_del( path, 0 );
            memset( iface_5g, 0, sizeof( iface_5g ) );

            extend->set_compl_option |= BIT(MESH_SSID_ATTR_MODE);
            extend->set_compl_option |= BIT(MESH_SSID_ATTR_SSID);
            extend->set_compl_option |= BIT(MESH_SSID_ATTR_BSSID);
            extend->set_compl_option |= BIT(MESH_SSID_ATTR_ENCRYPTION);
            extend->set_compl_option |= BIT(MESH_SSID_ATTR_KEY);
            extend->set_compl_option |= BIT(MESH_SSID_ATTR_ISADDED);
            extend->set_compl_option |= BIT(MESH_SSID_ATTR_MASTER_ID);
        }
        break;
    case MESH_SSID_ATTR_MODE:
    {
        char *mode = util_blobmsg_get_string( cur_attr, "" );
        snprintf( path, sizeof(path), "wireless.%s.mode", iface_5g );
        config_uci_set( path, mode, 0 );
        break;
    }
    case MESH_SSID_ATTR_SSID:
    {
        char *ssid = util_blobmsg_get_string( cur_attr, "" );
        snprintf( path, sizeof(path), "wireless.%s.ssid", iface_5g );
        config_uci_set( path, ssid, 0 );
        break;
    }
    case MESH_SSID_ATTR_BSSID:
    {
        /*
        char *mac = util_blobmsg_get_string( new_config[index], "" );
        snprintf( path, sizeof(path), "wireless.%s.bssid", iface_5g );
        config_uci_set( path, mac, 0 );
        */
        break;
    }
    case MESH_SSID_ATTR_ENCRYPTION:
    {
        char *encryption = util_blobmsg_get_string( cur_attr, "" );
        char *value = NULL;
        switch( atoi(encryption) ) {
            case CRYPTO_WPA_WPA2_PERSONAL:
                value = "psk+aes";
                break;
            case CRYPTO_WPA2_WPA3_PERSONAL:
                value = "sae-psk2+aes";
                break;
            case CRYPTO_WPA3_PERSONAL:
                value = "sae+aes";
                break;
            default:
                cfmanager_log_message( L_DEBUG, "Unknown encryption method:%d\n", atoi(encryption) );
                break;
        }
        snprintf( path, sizeof(path), "wireless.%s.encryption", iface_5g );
        config_uci_set( path, value, 0 );
        break;
    }
    case MESH_SSID_ATTR_KEY:
    {
        char *key = util_blobmsg_get_string( cur_attr, "" );
        snprintf( path, sizeof(path), "wireless.%s.key", iface_5g );
        config_uci_set( path, key, 0 );
        break;
    }
    case MESH_SSID_ATTR_ISADDED:
    case MESH_SSID_ATTR_MASTER_ID:
    {
        break;
    }

    default: break;
    }

    option |= BIT(CM_CFG_WIRELESS);
    return option;
}

//=============================================================================
static int
cm_upnp_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    char path[LOOKUP_STR_SIZE] = { 0 };

    switch ( extend->action ) {
        case VLTREE_ACTION_DEL:
            // We have already load upnpd when cfmanager (re)start.
            // This shouldn't happen when configure changed.
            cfmanager_log_message( L_WARNING, "Warning: We don't need to delete section in upnpd!\n");
            return option;
 
        case VLTREE_ACTION_ADD:
            snprintf( path, sizeof(path), "upnpd.config.enabled" );
            config_set_by_blob( config[UPNP_ENABLE], path, blobmsg_type(config[UPNP_ENABLE]) );
            snprintf( path, sizeof(path), "upnpd.config.EXTERNAL_iface" );
            config_uci_set( path, util_blobmsg_get_string(config[UPNP_INTF],""), 0 );
            extend->set_compl_option |= BIT( BIT_MAX );
            break;

        case VLTREE_ACTION_UPDATE:
            switch ( index ) {
                case UPNP_ENABLE:
                    snprintf( path, sizeof(path), "upnpd.config.enabled" );
                    config_set_by_blob( config[index], path, blobmsg_type(config[index]) );
                    break;

                case UPNP_INTF:
                    snprintf( path, sizeof(path), "upnpd.config.EXTERNAL_iface" );
                    config_uci_set( path, util_blobmsg_get_string(config[index],""), 0 );
                    break;

        }
    }

    option |= BIT( CM_CFG_UPNPD );

    return option;
}

//=============================================================================
static int
cm_ddns_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char *service_id = NULL;
    char section_nm[BUF_LEN_64] = { 0 };
    char *sp_str[__DDNS_SERVICE_PROVIDER_MAX] = {
        "oray.com",
        "dyndns.org",
        "no-ip.com"
    };

    service_id = util_blobmsg_get_string( config[DDNS_ID], "" );
    if ( !service_id ) {
        cfmanager_log_message( L_ERR, "Missing dns id...!" );
        return option;
    }

    snprintf( section_nm, sizeof(section_nm), "ddns%s", service_id );
    switch ( extend->action ) {
        case VLTREE_ACTION_DEL:
            config_del_named_section( "ddns", "service", section_nm );
            extend->set_compl_option |= BIT( BIT_MAX );
            break;
 
        case VLTREE_ACTION_ADD:
            config_add_named_section( "ddns", "service", section_nm );
            snprintf( path, sizeof(path), "ddns.%s.enabled", section_nm );
            config_set_by_blob( config[DDNS_STATUS], path, blobmsg_type(config[DDNS_STATUS]) );
            snprintf( path, sizeof(path), "ddns.%s.service_name", section_nm );
            config_uci_set( path, sp_str[util_blobmsg_get_int(config[DDNS_SERVICE_PROVIDER],0)], 0 );
            snprintf( path, sizeof(path), "ddns.%s.username", section_nm );
            config_set_by_blob( config[DDNS_USERNAME], path, blobmsg_type(config[DDNS_USERNAME]) );
            snprintf( path, sizeof(path), "ddns.%s.password", section_nm );
            config_set_by_blob( config[DDNS_PASSWORD], path, blobmsg_type(config[DDNS_PASSWORD]) );
            snprintf( path, sizeof(path), "ddns.%s.domain", section_nm );
            config_set_by_blob( config[DDNS_HOSTNAME], path, blobmsg_type(config[DDNS_HOSTNAME]) );
            snprintf( path, sizeof(path), "ddns.%s.interface", section_nm );
            config_set_by_blob( config[DDNS_INTF], path, blobmsg_type(config[DDNS_INTF]) );
            snprintf( path, sizeof(path), "ddns.%s.ip_source", section_nm );
            config_uci_set( path, "network", 0 );
            snprintf( path, sizeof(path), "ddns.%s.ip_network", section_nm );
            config_set_by_blob( config[DDNS_INTF], path, blobmsg_type(config[DDNS_INTF]) );
            snprintf( path, sizeof(path), "ddns.%s.use_syslog", section_nm );
            config_uci_set( path, "1", 0 );
            snprintf( path, sizeof(path), "ddns.%s.force_interval", section_nm );
            config_uci_set( path, "72", 0 );
            snprintf( path, sizeof(path), "ddns.%s.force_unit", section_nm );
            config_uci_set( path, "hours", 0 );
            snprintf( path, sizeof(path), "ddns.%s.retry_interval", section_nm );
            config_uci_set( path, "60", 0 );
            snprintf( path, sizeof(path), "ddns.%s.retry_unit", section_nm );
            config_uci_set( path, "seconds", 0 );
            snprintf( path, sizeof(path), "ddns.%s.check_interval", section_nm );
            config_uci_set( path, "10", 0 );
            snprintf( path, sizeof(path), "ddns.%s.check_unit", section_nm );
            config_uci_set( path, "minutes", 0 );

            extend->set_compl_option |= BIT( BIT_MAX );
            break;

        case VLTREE_ACTION_UPDATE:
            cfmanager_log_message(L_ERR, "update %d\n", index);
            switch ( index ) {
                case DDNS_SERVICE_PROVIDER:
                    snprintf( path, sizeof(path), "ddns.%s.service_name", section_nm );
                    config_uci_set( path, sp_str[util_blobmsg_get_int(config[index],0)], 0 );
                    break;

                case DDNS_USERNAME:
                    snprintf( path, sizeof(path), "ddns.%s.username", section_nm );
                    config_set_by_blob( config[index], path, blobmsg_type(config[index]) );
                    break;

                case DDNS_PASSWORD:
                    snprintf( path, sizeof(path), "ddns.%s.password", section_nm );
                    config_set_by_blob( config[index], path, blobmsg_type(config[index]) );
                    break;

                case DDNS_HOSTNAME:
                    snprintf( path, sizeof(path), "ddns.%s.domain", section_nm );
                    config_set_by_blob( config[index], path, blobmsg_type(config[index]) );
                    break;

                case DDNS_INTF:
                    snprintf( path, sizeof(path), "ddns.%s.interface", section_nm );
                    config_set_by_blob( config[index], path, blobmsg_type(config[index]) );
                    snprintf( path, sizeof(path), "ddns.%s.ip_network", section_nm );
                    config_set_by_blob( config[index], path, blobmsg_type(config[index]) );
                    break;
                case DDNS_STATUS:
                    snprintf( path, sizeof(path), "ddns.%s.enabled", section_nm );
                    config_set_by_blob( config[index], path, blobmsg_type(config[index]) );
                    break;
        }
    }

    option |= BIT( CM_CFG_DDNS );

    /*
     * For now, we support add/del/update operations, and only support to update 'enabled' option.
     * We always need to reload ddns if any change happend. So we add 'ddns' into apply list directly at this point.
     * Beside, the function 'apply_add' is safe to add repeatly.
     */
    apply_add( "ddns" );
    apply_timer_start();

    return option;
}

//=============================================================================
static int
cm_static_route_ipv4_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    char section_name[BUF_LEN_64] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    int create = 0;
    int option = 0;

    snprintf( section_name, sizeof(section_name), "ipv4_static_route%s",
        util_blobmsg_get_string(config[STATIC_ROUTE_IPV4_ID], "") );
    switch ( extend->action ) {
        case VLTREE_ACTION_DEL:
            // Only delete already enabled ipv4 static route.
            extend->set_compl_option |= BIT( BIT_MAX );
            if ( util_blobmsg_get_bool(config[STATIC_ROUTE_IPV4_ENABLE],false) == true ) {
                config_del_named_section( "network", "route", section_name );
            }
            else {
                return option;
            }
            break;

        case VLTREE_ACTION_ADD:
            // Only add enabled ipv4 static route.
            extend->set_compl_option |= BIT( BIT_MAX );
            if ( util_blobmsg_get_bool(config[STATIC_ROUTE_IPV4_ENABLE],false) == true ) {
                create = 1;
            }
            else {
                return option;
            }
            break;

        case VLTREE_ACTION_UPDATE:
            switch ( index ) {
                case STATIC_ROUTE_IPV4_NAME:
                    // It doesn't matter if rule name changed.
                    return option;

                case STATIC_ROUTE_IPV4_ENABLE:
                    if ( util_blobmsg_get_bool(config[index],false) == true ) {
                        create = 1;
                    }
                    else {
                        config_del_named_section( "network", "route", section_name );
                    }
                    extend->set_compl_option |= BIT( BIT_MAX );
                    break;

                case STATIC_ROUTE_IPV4_IP:
                    snprintf( path, sizeof(path), "network.%s.target", section_name );
                    config_uci_set( path, util_blobmsg_get_string(config[index],""), 0 );
                    break;

                case STATIC_ROUTE_IPV4_NETMASK:
                    snprintf( path, sizeof(path), "network.%s.netmask", section_name );
                    config_uci_set( path, util_blobmsg_get_string(config[index],""), 0 );
                    break;

                case STATIC_ROUTE_IPV4_OUT_INTF:
                    snprintf( path, sizeof(path), "network.%s.interface", section_name );
                    config_uci_set( path, util_blobmsg_get_string(config[index],""), 0 );
                    break;

                case STATIC_ROUTE_IPV4_NEXTHOP:
                    snprintf( path, sizeof(path), "network.%s.gateway", section_name );
                    config_uci_set( path, util_blobmsg_get_string(config[index],""), 0 );
                    break;

                case STATIC_ROUTE_IPV4_METRIC:
                    snprintf( path, sizeof(path), "network.%s.metric", section_name );
                    config_uci_set( path, util_blobmsg_get_string(config[index],""), 0 );
                    break;
            }
    }

    if ( create ) {
        config_add_named_section( "network", "route", section_name );
        snprintf( path, sizeof(path), "network.%s.target", section_name );
        config_uci_set( path, util_blobmsg_get_string(config[STATIC_ROUTE_IPV4_IP],""), 0 );
        snprintf( path, sizeof(path), "network.%s.netmask", section_name );
        config_uci_set( path, util_blobmsg_get_string(config[STATIC_ROUTE_IPV4_NETMASK],""), 0 );
        snprintf( path, sizeof(path), "network.%s.interface", section_name );
        config_uci_set( path, util_blobmsg_get_string(config[STATIC_ROUTE_IPV4_OUT_INTF],""), 0 );
        snprintf( path, sizeof(path), "network.%s.gateway", section_name );
        config_uci_set( path, util_blobmsg_get_string(config[STATIC_ROUTE_IPV4_NEXTHOP],""), 0 );
        snprintf( path, sizeof(path), "network.%s.metric", section_name );
        config_uci_set( path, util_blobmsg_get_string(config[STATIC_ROUTE_IPV4_METRIC],""), 0 );
    }

    option |= BIT( CM_CFG_NETWORK );

    return option;
}

//=============================================================================
static int
cm_static_route_ipv6_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int action = extend->action;
    const char *section_name = extend->section_name;    //cfmanager and network the section name is same
    int option = 0;
    char path[LOOKUP_STR_SIZE];
    char value[BUF_LEN_256];
    char *prefix_length = NULL;
    struct blob_attr *attr = config[index];
    bool enable = false;
    struct network_config_parse *nw_cfg = NULL;

    memset( value, 0, sizeof( value ) );
    switch( action ) {
        case VLTREE_ACTION_DEL:
            config_del_named_section( "network", "route6", section_name );
            break;
        case VLTREE_ACTION_ADD:
        case VLTREE_ACTION_UPDATE: {
            switch( index ) {
                case STATIC_ROUTE_IPV6_ENABLE:
                    enable = util_blobmsg_get_bool( attr, false );

                    nw_cfg = util_get_vltree_node( &network_route6_vltree, VLTREE_NETWORK, section_name );
                    if( enable ) {
                        if( !nw_cfg ) {
                            config_add_named_section( "network", "route6", section_name );
                        }
                    }
                    else {
                        if( nw_cfg ) {
                            config_del_named_section( "network", "route6", section_name );
                        }

                        extend->set_compl_option |= BIT( BIT_MAX );
                    }
                    break;
                case STATIC_ROUTE_IPV6_OUT_INTF:
                    strncpy( value, util_blobmsg_get_string( attr,  "" ), sizeof( value ) -1 );
                    snprintf( path, sizeof( path ), "network.%s.interface", section_name );
                    config_uci_set( path, value, 0 );
                    break;
                case STATIC_ROUTE_IPV6_NAME:
                    break;
                case STATIC_ROUTE_IPV6_PREFIX_LENGTH:
                case STATIC_ROUTE_IPV6_TARGET:
                    prefix_length = util_blobmsg_get_string( config[STATIC_ROUTE_IPV6_PREFIX_LENGTH],
                                        "64" );
                    snprintf( value, sizeof( value ), "%s/%s",
                        util_blobmsg_get_string( config[STATIC_ROUTE_IPV6_TARGET], "" ),
                        prefix_length );
                    snprintf( path, sizeof( path ), "network.%s.target", section_name );
                    config_uci_set( path, value, 0 );

                    extend->set_compl_option |= BIT( STATIC_ROUTE_IPV6_TARGET );
                    break;
                case STATIC_ROUTE_IPV6_NEXTHOP:
                    snprintf( path, sizeof( path ), "network.%s.gateway", section_name );
                    config_uci_set( path, util_blobmsg_get_string( attr, "" ), 0 );
                    break;
                case STATIC_ROUTE_IPV6_METRIC:
                    snprintf( path, sizeof( path ), "network.%s.metric", section_name );
                    config_uci_set( path, util_blobmsg_get_string( attr, "" ), 0 );
                    break;
                case STATIC_ROUTE_IPV6_ACTION:
                    break;
                case STATIC_ROUTE_IPV6_ID:
                    break;
                default:
                    break;
            }
        }
            break;
        default:
            break;
    }

    option |= BIT( CM_CFG_NETWORK );

    return option;
}

//=============================================================================
static int
cm_firewall_hooker_dos(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    int action = extend->action;

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                default:
                    break;
            }
        default:
            break;
    }

    return option;
}

//=============================================================================
static int
cm_acceleration_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    char path[LOOKUP_STR_SIZE] = { 0 };
    bool enable = false;

    switch ( extend->action ) {
        case VLTREE_ACTION_DEL:
            cfmanager_log_message( L_DEBUG, "cm_acceleration_hooker: del \n" );
            snprintf( path, sizeof(path), "ecm.global.acceleration_enable" );
            config_uci_set( path, "0", 0 );
            return option;
 
        case VLTREE_ACTION_ADD:
            cfmanager_log_message( L_DEBUG, "cm_acceleration_hooker: add \n" );
            snprintf( path, sizeof(path), "ecm.global.acceleration_enable" );
            enable =  util_blobmsg_get_bool( config[ACCELERATION_ENABLE], false );
            config_uci_set( path, enable ? "1": "0", 0 );

            snprintf( path, sizeof(path), "ecm.global.acceleration_engine" );
            config_uci_set( path, util_blobmsg_get_string( config[ACCELERATION_ENGINE], "auto" ), 0 );
            extend->set_compl_option |= BIT( BIT_MAX );
            break;

        case VLTREE_ACTION_UPDATE:
            cfmanager_log_message( L_DEBUG, "cm_acceleration_hooker: update %d \n", index );
            switch ( index ) {
                case ACCELERATION_ENABLE:
                    snprintf( path, sizeof(path), "ecm.global.acceleration_enable" );
                    enable =  util_blobmsg_get_bool( config[ACCELERATION_ENABLE], false );
                    config_uci_set( path, enable ? "1": "0", 0 );
                    break;

                case ACCELERATION_ENGINE:
                    snprintf( path, sizeof(path), "ecm.global.acceleration_engine" );
                    config_uci_set( path, util_blobmsg_get_string( config[ACCELERATION_ENGINE], "auto" ), 0 );
                    break;

        }
    }

    option |= BIT( CM_CFG_ACCELERATION );


    return option;
}

//=============================================================================
static int
cm_schedule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    struct blob_attr *abtimes_cur = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char session_name[BUF_LEN_64] = {0};
    int rem = 0;
    int id = 0;
    int option = 0;
    int action = extend->action;

    id = atoi( util_blobmsg_get_string( new_config[CM_SCHEDULE_ID], "0" ) );
    snprintf( session_name, BUF_LEN_64, "schedule%d", id );
    switch ( action ) {
        case VLTREE_ACTION_DEL:
            switch( index ) {
                case CM_SCHEDULE_ID:
                    config_del_named_section( CF_CONFIG_NAME_SCHEDULE, "schedule", session_name );

                    option |= BIT( CM_CFG_SCHEDULE );
                    break;
                default:
                    break;
            }
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case CM_SCHEDULE_ID:
                    config_creat_no_exist_section( CF_CONFIG_NAME_SCHEDULE, "schedule", session_name, &sch_vltree, VLTREE_SCHEDULE );

                    snprintf( path, sizeof( path ), "schedule.%s.id", session_name );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "0" ), 0 );

                    option |= BIT( CM_CFG_SCHEDULE );
                    break;
                case CM_SCHEDULE_NAME:
                case CM_SCHEDULE_WEEKLY_TIME1:
                case CM_SCHEDULE_WEEKLY_TIME2:
                case CM_SCHEDULE_WEEKLY_TIME3:
                case CM_SCHEDULE_WEEKLY_TIME4:
                case CM_SCHEDULE_WEEKLY_TIME5:
                case CM_SCHEDULE_WEEKLY_TIME6:
                case CM_SCHEDULE_WEEKLY_TIME7:
                    snprintf( path, sizeof( path ), "%s.%s.%s",
                            CF_CONFIG_NAME_SCHEDULE, session_name, cm_schedule_policy[index].name );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );

                    option |= BIT( CM_CFG_SCHEDULE );
                    break;
                case CM_SCHEDULE_ABTIME_LISTS:
                    snprintf( path, sizeof( path ), "%s.%s.%s",
                            CF_CONFIG_NAME_SCHEDULE, session_name, cm_schedule_policy[index].name );

                    /* clear abtime list */
                    config_uci_del( path, 0 );

                    /* clear abtime list */
                    blobmsg_for_each_attr( abtimes_cur, cur_attr, rem ) {
                        if ( !abtimes_cur || blobmsg_type( abtimes_cur ) != BLOBMSG_TYPE_STRING ) {
                            continue;
                        }
                        config_uci_add_list( path, blobmsg_get_string( abtimes_cur ), 0 );
                    }

                    option |= BIT( CM_CFG_SCHEDULE );
                    break;
                default:
                    break;
            }
    }

    return option;
}

//=============================================================================
static int
cm_hostname_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    int action = extend->action;

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case HOSTNAME_NAME:
                    break;
                case HOSTNAME_MAC:
                    break;
                default:
                    break;
            }
        default:
            break;
    }

    return option;
}

//=============================================================================
static int
cm_snmp_config_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    bool enable_v1_v2c = false;
    bool enable_v3 = false;
    const char *sysLocation = NULL;
    const char *sysContact = NULL;
    const char *sysName = NULL;

    const char *roCommunityV4 = NULL;
    const char *rwCommunityV4 = NULL;
    const char *roCommunityV6 = NULL;
    const char *rwCommunityV6 = NULL;

    const char *trapType = NULL;
    const char *trapHost = NULL;
    const char *trapPort = NULL;
    const char *trapCommunity = NULL;

    char path[LOOKUP_STR_SIZE];
    int option = 0;

    int action = extend->action;

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            config_del_named_section( "grandstream", "snmpd", "snmpd" );
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            config_del_named_section( "grandstream", "snmpd", "snmpd" );
            config_add_named_section( "grandstream", "snmpd", "snmpd" );
            enable_v1_v2c = util_blobmsg_get_bool( new_config[SNMP_CONFIG_ENABLE_V1_V2C], false );

            snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_ENABLE_V1_V2C].name );
            if ( enable_v1_v2c )
                config_uci_set( path, "1", 0 );
            else
                config_uci_set( path, "0", 0 );

            enable_v3 = util_blobmsg_get_bool( new_config[SNMP_CONFIG_ENABLE_V3], false );
            snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_ENABLE_V3].name );
            if ( enable_v3 )
                config_uci_set( path, "1", 0 );
            else
                config_uci_set( path, "0", 0 );   

            sysLocation = blobmsg_get_string( new_config[SNMP_CONFIG_SYS_LOCATION] );
            if ( sysLocation ) {
                snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_SYS_LOCATION].name );
                config_uci_set( path, (char *)sysLocation, 0 );
            }

            sysContact = blobmsg_get_string( new_config[SNMP_CONFIG_SYS_CONTACT] );
            if ( sysContact ) {
                snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_SYS_CONTACT].name );
                config_uci_set( path, (char *)sysContact, 0 );
            }

            sysName = blobmsg_get_string( new_config[SNMP_CONFIG_SYS_NAME] );
            if ( sysName ) {
                snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_SYS_NAME].name );
                config_uci_set( path, (char *)sysName, 0 );
            }

            snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_RO_COMMUNITYV4].name );
            cfmanager_log_message( L_DEBUG, "#######################line=%d, path=%s",__LINE__, path );
            roCommunityV4 = blobmsg_get_string( new_config[SNMP_CONFIG_RO_COMMUNITYV4] );
            if ( roCommunityV4 ) {
                config_uci_set( path, (char *)roCommunityV4, 0 );
            }
            else {
                roCommunityV4 = "public";
                config_uci_set( path, "public", 0 );
            }

            rwCommunityV4 = blobmsg_get_string( new_config[SNMP_CONFIG_RW_COMMUNITYV4] );
            if ( rwCommunityV4 ) {
                snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_RW_COMMUNITYV4].name );
                config_uci_set( path, (char *)rwCommunityV4, 0 );
            }

            snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_RO_COMMUNITYV6].name );
            cfmanager_log_message( L_DEBUG, "#######################line=%d, path=%s",__LINE__, path );
            roCommunityV6 = blobmsg_get_string( new_config[SNMP_CONFIG_RO_COMMUNITYV6] );

            roCommunityV6 = roCommunityV6 ? roCommunityV4 : "public";
            config_uci_set( path, (char *)roCommunityV6, 0 );

            rwCommunityV6 = blobmsg_get_string( new_config[SNMP_CONFIG_RW_COMMUNITYV6] );
            if ( rwCommunityV6 ) {
                snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_RW_COMMUNITYV6].name );
                config_uci_set( path, (char *)rwCommunityV6, 0 );
            }

            trapType = blobmsg_get_string( new_config[SNMP_CONFIG_TRAP_TYPE] );
            if ( trapType ) {
                snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_TRAP_TYPE].name );
                config_uci_set( path, (char *)trapType, 0 );
            }

            trapHost = blobmsg_get_string( new_config[SNMP_CONFIG_TRAP_HOST] );
            if ( trapHost ) {
                snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_TRAP_HOST].name );
                config_uci_set( path, (char *)trapHost, 0 );
            }

            trapPort = blobmsg_get_string( new_config[SNMP_CONFIG_TRAP_PORT] );
            if ( trapPort ) {
                snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_TRAP_PORT].name );
                config_uci_set( path, (char *)trapPort, 0 );
            }

            trapCommunity = blobmsg_get_string( new_config[SNMP_CONFIG_TRAP_COMMUNITY] );
            if ( trapCommunity ) {
                snprintf( path, sizeof( path ), "grandstream.snmpd.%s", grandstream_snmp_config_policy[SNMP_CONFIG_TRAP_COMMUNITY].name );
                config_uci_set( path, (char *)trapCommunity, 0 );
            }

            break;
    }

    extend->set_compl_option |= BIT( BIT_MAX );

    option |= BIT( CM_CFG_GRANDSTREAM );

    return option;
}


//=============================================================================
static int
cm_snmp_ports_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    const char *id = NULL;
    const char *protocol = NULL;
    const char *ipAddress= NULL;
    const char *port = NULL;

    char path[LOOKUP_STR_SIZE];
    int option = 0;

    int action = extend->action;

    id = blobmsg_get_string( new_config[SNMP_PORTS_ID] );
    switch ( action ) {
        case VLTREE_ACTION_DEL:
            if ( id )
                config_del_named_section( "grandstream", "snmpd_ports", id );
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            if ( !id )
                break;

            config_del_named_section( "grandstream", "snmpd_ports", id );
            config_add_named_section( "grandstream", "snmpd_ports", id );

            port = blobmsg_get_string( new_config[SNMP_PORTS_PORT] );
            protocol = blobmsg_get_string( new_config[SNMP_PORTS_PROTOCOL] );
            ipAddress = blobmsg_get_string( new_config[SNMP_PORTS_IPV4ADDRESS] );

            snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_ports_policy[SNMP_PORTS_ID].name );
            config_uci_set( path, (char *)id, 0 );

            if ( port ) {
                snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_ports_policy[SNMP_PORTS_PORT].name );
                config_uci_set( path, (char *)port, 0 );
            }

            if ( protocol ) {
                snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_ports_policy[SNMP_PORTS_PROTOCOL].name );
                config_uci_set( path, (char *)protocol, 0 );
            }

            if ( ipAddress ) {
                snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_ports_policy[SNMP_PORTS_IPV4ADDRESS].name );
                config_uci_set( path, (char *)ipAddress, 0 );
            }

            break;
    }

    extend->set_compl_option |= BIT( BIT_MAX );

    option |= BIT( CM_CFG_GRANDSTREAM );

    return option;
}


//=============================================================================
static int
cm_snmp_v3_auth_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    const char *id = NULL;
    const char *userName = NULL;
    const char *authType = NULL;
    const char *authPassPhrase = NULL;
    const char *privProto = NULL;
    const char *privPassPhrase = NULL;
    const char *accessCtrl = NULL;

    char path[LOOKUP_STR_SIZE];
    int option = 0;

    int action = extend->action;

    id = blobmsg_get_string( new_config[SNMP_V3_ID] );
    switch ( action ) {
        case VLTREE_ACTION_DEL:
            if ( id )
                config_del_named_section( "grandstream", "snmpv3_auth", id );
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            if ( !id )
                break;

            config_del_named_section( "grandstream", "snmpv3_auth", id );
            config_add_named_section( "grandstream", "snmpv3_auth", id );

            snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_v3_auth_policy[SNMP_V3_ID].name );
            config_uci_set( path, (char *)id, 0 );

            userName = blobmsg_get_string( new_config[SNMP_V3_NAME] );
            authType = blobmsg_get_string( new_config[SNMP_V3_AUTH_TYPE] );
            authPassPhrase = blobmsg_get_string( new_config[SNMP_V3_AUTH_PASS_PHRASE] );
            privProto = blobmsg_get_string( new_config[SNMP_V3_PRIV_PROTO] );
            privPassPhrase = blobmsg_get_string( new_config[SNMP_V3_PRIV_PASS_PHRASE] );
            accessCtrl = blobmsg_get_string( new_config[SNMP_V3_ACCESS_CTRL] );

            if ( userName ) {
                snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_v3_auth_policy[SNMP_V3_NAME].name );
                config_uci_set( path, (char *)userName, 0 );
            }

            if ( authType ) {
                snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_v3_auth_policy[SNMP_V3_AUTH_TYPE].name );
                config_uci_set( path, (char *)authType, 0 );
            }

            if ( authPassPhrase ) {
                snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_v3_auth_policy[SNMP_V3_AUTH_PASS_PHRASE].name );
                config_uci_set( path, (char *)authPassPhrase, 0 );
            }

            if ( privProto ) {
                snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_v3_auth_policy[SNMP_V3_PRIV_PROTO].name );
                config_uci_set( path, (char *)privProto, 0 );
            }

            if ( privPassPhrase ) {
                snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_v3_auth_policy[SNMP_V3_PRIV_PASS_PHRASE].name );
                config_uci_set( path, (char *)privPassPhrase, 0 );
            }

            if ( accessCtrl ) {
                snprintf( path, sizeof( path ), "grandstream.%s.%s", id, grandstream_snmp_v3_auth_policy[SNMP_V3_ACCESS_CTRL].name );
                config_uci_set( path, (char *)accessCtrl, 0 );
            }

            break;
    }

    extend->set_compl_option |= BIT( BIT_MAX );

    option |= BIT( CM_CFG_GRANDSTREAM );

    return option;
}

#if 0
//=============================================================================
static void
cm_set_radio_control_option(
    uint64_t *option,
    int radio
)
//=============================================================================
{
    if( !option ) {
        return;
    }

    if( RADIO_2G == radio ) {
        *option |= BIT( RADIO_2G4CHANNEL );
        *option |= BIT( RADIO_2G4CHANNELWIDTH );
        *option |= BIT( RADIO_2G4CHANNEL_LOCATION );
        *option |= BIT( RADIO_2G4TXPOWER );
        *option |= BIT( RADIO_2G4CUSTOM_TXPOWER );
        *option |= BIT( RADIO_2G4SHORTGI );
        *option |= BIT( RADIO_2G4ALLOW_LEGACY_DEV );
        *option |= BIT( RADIO_2G4RSSI_ENABLE );
        *option |= BIT( RADIO_2G4RSSI_THRESHOLD );
        *option |= BIT( RADIO_2G4RATE_LIMIT_ENABLE );
        *option |= BIT( RADIO_2G4MINI_RATE );
    }
    else {
        *option |= BIT( RADIO_5GCHANNEL );
        *option |= BIT( RADIO_5GCHANNELWIDTH );
        *option |= BIT( RADIO_5GTXPOWER );
        *option |= BIT( RADIO_5GCUSTOM_TXPOWER );
        *option |= BIT( RADIO_5GSHORTGI );
        *option |= BIT( RADIO_5GRSSI_ENABLE );
        *option |= BIT( RADIO_5GRSSI_THRESHOLD );
        *option |= BIT( RADIO_5GRATE_LIMIT_ENABLE );
        *option |= BIT( RADIO_5GMINI_RATE );
    }
}
#endif

//=============================================================================
static int
cm_radio_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = config[index];
    struct blob_attr *ap_tb[__CM_AP_MAX];
    struct cm_config *ap_node = NULL;
    int option = 0;
    int action = extend->action;
    int txpower = 0;
    char value[BUF_LEN_64] = { 0 };
    char mac_str[MAC_STR_MAX_LEN+1] = { 0 };
    char ap_value[BUF_LEN_128] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    bool enable = false;
    bool radio_disable = false;

    memset( ap_tb, 0, __CM_AP_MAX * sizeof(*ap_tb) );
    snprintf( mac_str, sizeof( mac_str ), COMPACT_MACSTR, MAC2STR( device_info.mac_raw ) );
    ap_node = util_get_vltree_node( &cm_ap_vltree, VLTREE_CM_TREE, mac_str );
    if( !ap_node ) {
        cfmanager_log_message( L_ERR, "miss ap '%s' section\n", mac_str );
    }
    else {
        blobmsg_parse( cm_ap_policy,
            __CM_AP_MAX,
            ap_tb,
            blob_data( ap_node->cf_section.config ),
            blob_len( ap_node->cf_section.config ) );
    }

    radio_disable = util_blobmsg_get_bool( ap_tb[CM_AP_2G4_DISABLE], false );
    if( radio_disable ) {
        config_uci_set( "wireless.wifi0.disabled", "1", 0 );
    }
    else {
        config_uci_set( "wireless.wifi0.disabled", "0", 0 );
    }

    radio_disable = util_blobmsg_get_bool( ap_tb[CM_AP_5G_DISABLE], false );
    if( radio_disable ) {
        config_uci_set( "wireless.wifi1.disabled", "1", 0 );
    }
    else {
        config_uci_set( "wireless.wifi1.disabled", "0", 0 );
    }

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case CM_RADIO_2G4CHANNEL:
                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_2G4_CHANNEL], "" ),
                        sizeof(ap_value)-1 );

                    if( '0' == ap_value[0] || '\0' == ap_value[0] ) {
                        config_uci_set( "wireless.wifi0.channel", "auto", 0 );
                    }
                    else {
                        config_uci_set( "wireless.wifi0.channel", ap_value, 0 );
                    }
                    break;
                case CM_RADIO_2G4CHANNELWIDTH:
                case CM_RADIO_2G4CHANNEL_LOCATION:
                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_2G4_WIDTH], "" ),
                        sizeof(ap_value)-1 );

                    if( AP_USE_RF_2G4CHANNELWIDTH == ap_value[0] || '\0' == ap_value[0] ) {
                        config_set_channel_width( util_blobmsg_get_int( config[CM_RADIO_2G4CHANNELWIDTH], 0 ),
                            util_blobmsg_get_int( config[CM_RADIO_2G4CHANNEL_LOCATION], 0 ), RADIO_2G );
                    }
                    else {
                        config_set_channel_width( atoi( ap_value ), CL_AUTO, RADIO_2G );
                    }

                    strncpy( value, util_blobmsg_get_string( config[CM_RADIO_2G4CHANNEL_LOCATION], "0" ),
                        sizeof( value ) -1 );
                    vlist_for_each_element( &cm_ap_vltree, ap_node, node ) {
                        snprintf( path, sizeof( path ), "%s.%s.%s",
                            CF_CONFIG_NAME_GRANDSTREAM, ap_node->cf_section.name,
                            gs_ap_policy[GS_AP_2G4_CHANNEL_LOCATION].name );

                        config_uci_set( path, value, 0 );
                    }

                    option |= BIT( CM_CFG_GRANDSTREAM );
                    extend->set_compl_option |= BIT( CM_RADIO_2G4CHANNEL_LOCATION );
                    break;
                case CM_RADIO_2G4TXPOWER:
                case CM_RADIO_2G4CUSTOM_TXPOWER:
                    txpower = util_blobmsg_get_int( cur_attr, 0 );
                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_2G4_POWER], "" ),
                        sizeof(ap_value)-1 );

                    if( AP_USE_RF_2G4TXPOWER == ap_value[0] || '\0' == ap_value[0] ) {
                        config_set_txpower( txpower, RADIO_2G,
                            util_blobmsg_get_string( config[CM_RADIO_2G4CUSTOM_TXPOWER], POWER_HIGH_STR) );
                    }
                    else {
                        config_set_txpower( atoi(ap_value), RADIO_2G,
                            util_blobmsg_get_string( ap_tb[CM_AP_2G4_CUSTOM_TXPOWER], POWER_HIGH_STR) );
                    }

                    extend->set_compl_option |= BIT( CM_RADIO_2G4CUSTOM_TXPOWER );
                    break;
                case CM_RADIO_2G4SHORTGI:
                    enable = util_blobmsg_get_bool( cur_attr, false );
                    snprintf( value, sizeof( value ), "%d", enable );
                    config_uci_set( "wireless.wifi0.shortgi", value, 0 );

                    vlist_for_each_element( &cm_ap_vltree, ap_node, node ) {
                        snprintf( path, sizeof( path ), "%s.%s.%s",
                            CF_CONFIG_NAME_GRANDSTREAM, ap_node->cf_section.name,
                            gs_ap_policy[GS_AP_2G4_SHORTGI].name );

                        config_uci_set( path, value, 0 );
                    }

                    option |= BIT( CM_CFG_GRANDSTREAM );
                    break;
                case CM_RADIO_2G4ALLOW_LEGACY_DEV:
                    enable = util_blobmsg_get_bool( cur_attr, false );
                    if( enable ) {
                        config_uci_set( "wireless.wifi0.pureg", "0", 0 );
                    }
                    else {
                        config_uci_set( "wireless.wifi0.pureg", "1", 0 );
                    }

                    snprintf( value, sizeof( value ), "%d", enable );
                    vlist_for_each_element( &cm_ap_vltree, ap_node, node ) {
                        snprintf( path, sizeof( path ), "%s.%s.%s",
                            CF_CONFIG_NAME_GRANDSTREAM, ap_node->cf_section.name,
                            gs_ap_policy[GS_AP_2G4_ALLOW_LEGACY_DEV].name );

                        config_uci_set( path, value, 0 );
                    }

                    option |= BIT( CM_CFG_GRANDSTREAM );
                    break;
                case CM_RADIO_2G4RSSI_ENABLE:
                case CM_RADIO_2G4RSSI_THRESHOLD:
                    if( CM_RADIO_2G4RSSI_ENABLE == index ) {
                        extend->set_compl_option |= BIT( CM_RADIO_2G4RSSI_THRESHOLD );
                    }

                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_2G4_RSSI_TYPE], "" ),
                        sizeof(ap_value)-1 );

                    if( AP_USE_RF_2G4RSSI == ap_value[0] || '\0' == ap_value[0] ) {
                        enable = util_blobmsg_get_bool( config[CM_RADIO_2G4RSSI_ENABLE], false );
                        strncpy( value,
                            util_blobmsg_get_string( config[CM_RADIO_2G4RSSI_THRESHOLD], "" ),
                            sizeof( value ) -1 );
                    }
                    else {
                        enable = util_convert_string_to_bool( ap_value );

                        strncpy( value,
                            util_blobmsg_get_string( ap_tb[CM_AP_2G4_RSSI_THRESHOLD], "" ),
                            sizeof( value ) -1 );
                    }

                    config_set_rssi( enable, value, RADIO_2G );
                    break;
                case CM_RADIO_2G4RATE_LIMIT_ENABLE:
                case CM_RADIO_2G4MINI_RATE:
                    if( CM_RADIO_2G4RATE_LIMIT_ENABLE == index ) {
                        extend->set_compl_option |= BIT( CM_RADIO_2G4MINI_RATE );
                    }

                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_2G4_RATE_LIMIT_TYPE], "" ),
                        sizeof(ap_value)-1 );

                    if( AP_USE_RF_2G4RATE == ap_value[0] || '\0' == ap_value[0] ) {
                        enable = util_blobmsg_get_bool( config[CM_RADIO_2G4RATE_LIMIT_ENABLE], false );
                        strncpy( value,
                            util_blobmsg_get_string( config[CM_RADIO_2G4MINI_RATE], "" ),
                            sizeof( value ) -1 );

                    }
                    else {
                        enable = util_convert_string_to_bool( ap_value );

                        strncpy( value,
                            util_blobmsg_get_string( ap_tb[CM_AP_2G4_MINI_RATE], "" ),
                            sizeof( value ) -1 );
                    }

                    config_set_radio_access_rate( enable, value, RADIO_2G );
                    break;
                case CM_RADIO_5GCHANNEL:
                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_5G_CHANNEL], "" ),
                        sizeof(ap_value)-1 );

                    if( '0' == ap_value[0] || '\0' == ap_value[0] ) {
                        config_uci_set( "wireless.wifi1.channel", "auto", 0 );
                    }
                    else {
                        config_uci_set( "wireless.wifi1.channel", ap_value, 0 );
                    }
                    break;
                case CM_RADIO_5GCHANNELWIDTH:
                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_5G_WIDTH], "" ),
                        sizeof(ap_value)-1 );

                    if( AP_USE_RF_5GCHANNELWIDTH == ap_value[0] || '\0' == ap_value[0] ) {
                        config_set_channel_width( util_blobmsg_get_int( cur_attr, 0 ), 0, RADIO_5G );
                    }
                    else {
                        config_set_channel_width( atoi(ap_value), 0, RADIO_5G );
                    }
                    break;
                case CM_RADIO_5GTXPOWER:
                case CM_RADIO_5GCUSTOM_TXPOWER:
                    txpower = util_blobmsg_get_int( cur_attr, 0 );
                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_5G_POWER], "" ),
                        sizeof(ap_value)-1 );

                    if( AP_USE_RF_5GTXPOWER == ap_value[0] || '\0' == ap_value[0] ) {
                        config_set_txpower( txpower, RADIO_5G,
                            util_blobmsg_get_string( config[CM_RADIO_5GCUSTOM_TXPOWER], POWER_HIGH_STR) );
                    }
                    else {
                        config_set_txpower( atoi(ap_value), RADIO_5G,
                            util_blobmsg_get_string( ap_tb[CM_AP_5G_CUSTOM_TXPOWER], POWER_HIGH_STR) );
                    }

                    extend->set_compl_option |= BIT( CM_RADIO_5GCUSTOM_TXPOWER );
                    break;
                case CM_RADIO_5GSHORTGI:
                    enable = util_blobmsg_get_bool( cur_attr, false );
                    snprintf( value, sizeof( value ), "%d", enable );
                    config_uci_set( "wireless.wifi1.shortgi", value, 0 );

                    vlist_for_each_element( &cm_ap_vltree, ap_node, node ) {
                        snprintf( path, sizeof( path ), "%s.%s.%s",
                            CF_CONFIG_NAME_GRANDSTREAM, ap_node->cf_section.name,
                            gs_ap_policy[GS_AP_5G_SHORTGI].name );

                        config_uci_set( path, value, 0 );
                    }

                    option |= BIT( CM_CFG_GRANDSTREAM );
                    break;
                case CM_RADIO_5GRSSI_ENABLE:
                case CM_RADIO_5GRSSI_THRESHOLD:
                    if( CM_RADIO_5GRSSI_ENABLE == index ) {
                        extend->set_compl_option |= BIT( CM_RADIO_5GRSSI_THRESHOLD );
                    }

                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_5G_RSSI_TYPE], "" ),
                        sizeof(ap_value)-1 );

                    if( AP_USE_RF_5GRSSI == ap_value[0] || '\0' == ap_value[0] ) {
                        enable = util_blobmsg_get_bool( config[CM_RADIO_5GRSSI_ENABLE], false );
                        strncpy( value,
                            util_blobmsg_get_string( config[CM_RADIO_5GRSSI_THRESHOLD], "" ),
                            sizeof( value ) -1 );
                    }
                    else {
                        enable = util_convert_string_to_bool( ap_value );

                        strncpy( value,
                            util_blobmsg_get_string( ap_tb[CM_AP_5G_RSSI_THRESHOLD], "" ),
                            sizeof( value ) -1 );
                    }

                    config_set_rssi( enable, value, RADIO_5G );
                    break;
                case CM_RADIO_5GRATE_LIMIT_ENABLE:
                case CM_RADIO_5GMINI_RATE:
                    if( CM_RADIO_5GRATE_LIMIT_ENABLE == index ) {
                        extend->set_compl_option |= BIT( CM_RADIO_5GMINI_RATE );
                    }

                    strncpy( ap_value, util_blobmsg_get_string( ap_tb[CM_AP_5G_RATE_LIMIT_TYPE], "" ),
                        sizeof(ap_value)-1 );

                    if( AP_USE_RF_5GRATE == ap_value[0] || '\0' == ap_value[0] ) {
                        enable = util_blobmsg_get_bool( config[CM_RADIO_5GRATE_LIMIT_ENABLE], false );
                        strncpy( value,
                            util_blobmsg_get_string( config[CM_RADIO_5GMINI_RATE], "" ),
                            sizeof( value ) -1 );
                    }
                    else {
                        enable = util_convert_string_to_bool( ap_value );

                        strncpy( value,
                            util_blobmsg_get_string( ap_tb[CM_AP_5G_MINI_RATE], "" ),
                            sizeof( value ) -1 );
                    }

                    config_set_radio_access_rate( enable, value, RADIO_5G );
                    break;
                case CM_RADIO_BEACON_INTERVAL:
                    strncpy( value, util_blobmsg_get_string( cur_attr, "" ), sizeof( value ) -1 );
                    config_set_beacon_interval( value, RADIO_2G );
                    config_set_beacon_interval( value, RADIO_5G );
                    break;
                case CM_RADIO_ATF_MODE:
                    enable = util_blobmsg_get_bool( cur_attr, false );

                    snprintf( value, sizeof( value ), "%d", enable );
                    config_uci_set( "wireless.qcawifi.atf_mode", value, 0 );
                    config_uci_set( "wireless.wifi0.atf_mode", value, 0 );
                    config_uci_set( "wireless.wifi1.atf_mode", value, 0 );

                    vlist_for_each_element( &cm_ap_vltree, ap_node, node ) {
                        snprintf( path, sizeof( path ), "%s.%s.%s",
                            CF_CONFIG_NAME_GRANDSTREAM, ap_node->cf_section.name,
                            gs_ap_policy[GS_AP_ATF_MODE].name );

                        config_uci_set( path, value, 0 );
                    }

                    option |= BIT( CM_CFG_GRANDSTREAM );
                    break;
                case CM_RADIO_MUMIMOENABLE:
                    enable = util_blobmsg_get_bool( cur_attr, false );

                    snprintf( value, sizeof( value ), "%d", enable );
                    config_uci_set( "wireless.wifi0.mumimo", value, 0 );
                    config_uci_set( "wireless.wifi1.mumimo", value, 0 );
                    break;
#if defined(GWN7062)
                case CM_RADIO_COMPATIBILITYMODE:
                    enable = util_blobmsg_get_bool( cur_attr, false );
                    if( enable ) {
                        config_uci_set( "wireless.wifi0.hwmode", "11ng", 0 );
                        config_uci_set( "wireless.wifi1.hwmode", "11ac", 0 );
                    }
                    else {
                        config_uci_set( "wireless.wifi0.hwmode", "11axg", 0 );
                        config_uci_set( "wireless.wifi1.hwmode", "11axa", 0 );
                    }
                    break;
#endif
                default:
                    break;
            }
        default:
            break;
    }

    option |= BIT( CM_CFG_WIRELESS );
    return option;
}

//=============================================================================
static void
cm_set_addit_ssid_control_option(
    uint64_t *option
)
//=============================================================================
{
    if( !option ) {
        return;
    }

    *option |= BIT( CM_ADDIT_SSID_NAME );
    *option |= BIT( CM_ADDIT_SSID_CRYPTO );
    *option |= BIT( CM_ADDIT_SSID_SSIDHIDEENABLE );
    *option |= BIT( CM_ADDIT_SSID_ISOLATEMODE );
    *option |= BIT( CM_ADDIT_SSID_DTIM_PERIOD );
    *option |= BIT( CM_ADDIT_SSID_CLIENT_LIMIT );
    *option |= BIT( CM_ADDIT_SSID_STA_IDLE_TIMEOUT );
    *option |= BIT( CM_ADDIT_SSID_UAPSD );
    *option |= BIT( CM_ADDIT_SSID_PROXY_ARP );
    *option |= BIT( CM_ADDIT_SSID_MCAST_TO_UCAST );
    *option |= BIT( CM_ADDIT_SSID_BMS );
    *option |= BIT( CM_ADDIT_SSID_BRIDGE_ENABLE );
    *option |= BIT( CM_ADDIT_SSID_80211W );
}

//=============================================================================
static int
cm_addit_ssid_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = config[index];
    struct wireless_crypto_parameter crypto_param;
    int option = 0;
    int action = extend->action;
    int id = 0;
    int ret = 0;
    bool enable = false;
    bool find = false;
    int ssid_band = 0;
    static char iface_2g[SSID_COUNT_MAX][BUF_LEN_8] = {{ 0 }};
    static char iface_5g[SSID_COUNT_MAX][BUF_LEN_8] = {{ 0 }};
    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_64] = { 0 };
    char *cm_section_name = extend->section_name;
    char mac_str[MAC_STR_MAX_LEN+1] = { 0 };
    char ssids[BUF_LEN_128] = { 0 };
    int schedule_id = 0;
    int vlan_id = 0;

    sscanf( util_blobmsg_get_string( config[CM_ADDIT_SSID_ID], "ssid0" ), "ssid%d",  &id );

    enable = util_blobmsg_get_bool( config[CM_ADDIT_SSID_ENABLE], false );
    ssid_band = util_blobmsg_get_int( config[CM_ADDIT_SSID_BAND], 0 );

    if( enable ) {
        if( '\0' == iface_2g[id][0]
            && ( SSID_BAND_DUAL_FREQ == ssid_band || SSID_BAND_2G4 == ssid_band ) ) {

            config_get_ifname( iface_2g[id], sizeof( iface_2g[id] ), RADIO_2G, NET_TYPE_ADDIT, cm_section_name );
        }

        if( '\0' == iface_5g[id][0]
            && ( SSID_BAND_DUAL_FREQ == ssid_band || SSID_BAND_5G == ssid_band ) ) {

            config_get_ifname( iface_5g[id], sizeof( iface_5g[id] ), RADIO_5G, NET_TYPE_ADDIT, cm_section_name );
        }
    }

    //Check if the device currently added by ssid has this device
    snprintf( mac_str, sizeof( mac_str ), COMPACT_MACSTR, MAC2STR( device_info.mac_raw ) );

    config_get_cm_ap_ssids( mac_str, ssids, sizeof( ssids ) );
    find = util_match_ssids( ssids, cm_section_name );

    if( !find ) {
        if( '\0' != iface_2g[id][0] ) {
            config_del_iface_section( iface_2g[id] );
            memset( iface_2g[id], 0, sizeof( iface_2g[id] ) );
            option |= BIT( CM_CFG_WIRELESS );
        }

        if( '\0' != iface_5g[id][0] ) {
            config_del_iface_section( iface_5g[id] );
            memset( iface_5g[id], 0, sizeof( iface_5g[id] ) );
            option |= BIT( CM_CFG_WIRELESS );
        }

        ret = config_del_bwctrl_section( cm_section_name );
        if( 0 == ret ) {
            option |= BIT( CM_CFG_BWCTRL );
        }

        extend->set_compl_option |= BIT( BIT_MAX );

        cfmanager_log_message( L_DEBUG, "ssid id:%s not added by the current device\n", cm_section_name );
        return option;
    }
    else {
        extend->need_set_option |= BIT( CM_ADDIT_SSID_ENABLE );
        extend->need_set_option |= BIT( CM_ADDIT_SSID_BAND );
    }

    if( cur_attr && BLOBMSG_TYPE_STRING == blobmsg_type(cur_attr) ) {
        strncpy( value, blobmsg_get_string( cur_attr ), sizeof( value ) -1 );
    }
    else if( cur_attr && BLOBMSG_TYPE_BOOL == blobmsg_type(cur_attr) ) {
        snprintf( value, sizeof( value ), "%d", util_blobmsg_get_bool( cur_attr, false ) );
    }

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            if( '\0' != iface_2g[id][0] ) {
                config_del_iface_section( iface_2g[id] );
                memset( iface_2g[id], 0, sizeof( iface_2g[id] ) );
            }
            if( '\0' != iface_5g[id][0] ) {
                config_del_iface_section( iface_5g[id] );
                memset( iface_5g[id], 0, sizeof( iface_5g[id] ) );
            }

            ret = config_del_bwctrl_section( cm_section_name );
            if( 0 == ret ) {
                option |= BIT( CM_CFG_BWCTRL );
            }
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case CM_ADDIT_SSID_ID:
                    break;
                case CM_ADDIT_SSID_ACTION:
                    break;
                case CM_ADDIT_SSID_ENABLE:
                    if( enable ) {
                        if( '\0' == iface_2g[id][0] &&
                            ( SSID_BAND_DUAL_FREQ == ssid_band || SSID_BAND_2G4 == ssid_band ) ) {

                            ret = config_create_ssid( iface_2g[id], sizeof( iface_2g[id] ),
                                RADIO_2G, NET_TYPE_ADDIT, cm_section_name );
                            if( 0 > ret ) {
                                cfmanager_log_message( L_ERR, "create 2.4g ssid failed id:%d\n", id );
                                return 0;
                            }
                        }

                        if( '\0' == iface_5g[id][0] &&
                            ( SSID_BAND_DUAL_FREQ == ssid_band || SSID_BAND_5G == ssid_band ) ) {

                            ret = config_create_ssid( iface_5g[id], sizeof( iface_5g[id] ),
                                RADIO_5G, NET_TYPE_ADDIT, cm_section_name );
                            if( 0 > ret ) {
                                cfmanager_log_message( L_ERR, "create 5g ssid failed id:%d\n", id );
                                return 0;
                            }
                        }

                        extend->need_set_option |= BIT( CM_ADDIT_SSID_UPRATE );
                        cm_set_addit_ssid_control_option( &extend->need_set_option );
                    }
                    else {
                        if( '\0' != iface_2g[id][0] ) {
                            config_del_iface_section( iface_2g[id] );
                            memset( iface_2g[id], 0, sizeof( iface_2g[id] ) );
                        }
                        if( '\0' != iface_5g[id][0] ) {
                            config_del_iface_section( iface_5g[id] );
                            memset( iface_5g[id], 0, sizeof( iface_5g[id] ) );
                        }

                        extend->need_set_option |= BIT( CM_ADDIT_SSID_UPRATE );
                        cm_set_addit_ssid_control_option( &extend->set_compl_option );
                    }
                    break;
                case CM_ADDIT_SSID_BAND:
                    if( SSID_BAND_DUAL_FREQ == ssid_band ) {
                        if( '\0' == iface_2g[id][0] ) {
                            ret = config_create_ssid( iface_2g[id], sizeof( iface_2g[id] ),
                                RADIO_2G, NET_TYPE_ADDIT, cm_section_name );
                            if( 0 > ret ) {
                                cfmanager_log_message( L_ERR, "create 2.4g ssid failed id:%d\n", id );
                                return 0;
                            }
                        }

                        if( '\0' == iface_5g[id][0] ) {
                            ret = config_create_ssid( iface_5g[id], sizeof( iface_5g[id] ),
                                RADIO_5G, NET_TYPE_ADDIT, cm_section_name );
                            if( 0 > ret ) {
                                cfmanager_log_message( L_ERR, "create 5g ssid failed id:%d\n", id );
                                return 0;
                            }
                        }
                    }
                    else if ( SSID_BAND_2G4 == ssid_band ) {
                        if( '\0' == iface_2g[id][0] ) {
                            ret = config_create_ssid( iface_2g[id], sizeof( iface_2g[id] ),
                                RADIO_2G, NET_TYPE_ADDIT, cm_section_name );
                            if( 0 > ret ) {
                                cfmanager_log_message( L_ERR, "create 2.4g ssid failed id:%d\n", id );
                                return 0;
                            }
                        }

                        if( '\0' != iface_5g[id][0] ) {
                            ret = config_del_iface_section( iface_5g[id] );
                            if( 0 > ret ) {
                                cfmanager_log_message( L_ERR, "delete 5g ssid failed id:%d\n", id );
                                return 0;
                            }

                            memset( iface_5g[id], 0, sizeof( iface_5g[id] ) );
                        }
                    }
                    else if( SSID_BAND_5G == ssid_band ) {
                        if( '\0' != iface_2g[id][0] ) {
                            ret = config_del_iface_section( iface_2g[id] );
                            if( 0 > ret ) {
                                cfmanager_log_message( L_ERR, "delete 2.4g ssid failed id:%d\n", id );
                                return 0;
                            }

                            memset( iface_2g[id], 0, sizeof( iface_2g[id] ) );
                        }

                        if( '\0' == iface_5g[id][0] ) {
                            ret = config_create_ssid( iface_5g[id], sizeof( iface_5g[id] ),
                                RADIO_5G, NET_TYPE_ADDIT, cm_section_name );
                            if( 0 > ret ) {
                                cfmanager_log_message( L_ERR, "create 5g ssid failed id:%d\n", id );
                                return 0;
                            }
                        }
                    }
                    else {
                        cfmanager_log_message( L_ERR, "error ssid band:%d\n", ssid_band );
                        return 0;
                    }

                    extend->need_set_option |= BIT( CM_ADDIT_SSID_UPRATE );
                    cm_set_addit_ssid_control_option( &extend->need_set_option );
                    break;
                case CM_ADDIT_SSID_NAME:
                    option |= BIT( CM_CFG_GSPORTAL );
                case CM_ADDIT_SSID_SSIDHIDEENABLE:
                case CM_ADDIT_SSID_ISOLATEMODE:
                case CM_ADDIT_SSID_GATEWAYMAC:
                case CM_ADDIT_SSID_DTIM_PERIOD:
                case CM_ADDIT_SSID_CLIENT_LIMIT:
                case CM_ADDIT_SSID_STA_IDLE_TIMEOUT:
                case CM_ADDIT_SSID_UAPSD:
                case CM_ADDIT_SSID_PROXY_ARP:
                case CM_ADDIT_SSID_MCAST_TO_UCAST:
                case CM_ADDIT_SSID_BMS:
                case CM_ADDIT_SSID_80211W:
                case CM_ADDIT_SSID_PORTAL_ENABLE:
                case CM_ADDIT_SSID_PORTAL_POLICY:
                    config_set_wireless( index, iface_2g[id], value, RADIO_2G );
                    config_set_wireless( index, iface_5g[id], value, RADIO_5G );
                    break;
                //ssid encryption settings
                case CM_ADDIT_SSID_CRYPTO:
                case CM_ADDIT_SSID_WPA_KEY_MODE:
                case CM_ADDIT_SSID_WPA_CRYPTO_TYPE:
                case CM_ADDIT_SSID_PASSWORD:
                    memset( &crypto_param, 0, sizeof( crypto_param ) );

                    crypto_param.crypto = util_blobmsg_get_int( config[CM_ADDIT_SSID_CRYPTO], 0 );
                    strncpy( crypto_param.wpa_key_mode, util_blobmsg_get_string( config[CM_ADDIT_SSID_WPA_KEY_MODE], "" ),
                            sizeof( crypto_param.wpa_key_mode ) -1 );
                    strncpy( crypto_param.wpa_crypto_type, util_blobmsg_get_string( config[CM_ADDIT_SSID_WPA_CRYPTO_TYPE], "" ),
                            sizeof( crypto_param.wpa_crypto_type ) -1 );
                    strncpy( crypto_param.password, util_blobmsg_get_string( config[CM_ADDIT_SSID_PASSWORD], "password" ),
                            sizeof( crypto_param.password ) -1 );
                    if( action == VLTREE_ACTION_UPDATE ) {
                        crypto_param.crypto_old = util_blobmsg_get_int( extend->old_config[CM_ADDIT_SSID_CRYPTO], 0 );
                    }
                    else {
                        /* No old encryption when the process is restarted or when a new ssid is added
                         * So the current encryption method is considered to be the old encryption method
                         */
                        crypto_param.crypto_old = util_blobmsg_get_int( config[CM_ADDIT_SSID_CRYPTO], 0 );
                    }

                    crypto_param.radio = RADIO_2G;
                    strncpy( crypto_param.ifname, iface_2g[id], sizeof( crypto_param.ifname ) -1 );
                    config_set_wireless_crypto( &crypto_param );

                    crypto_param.radio = RADIO_5G;
                    strncpy( crypto_param.ifname, iface_5g[id], sizeof( crypto_param.ifname ) -1 );
                    config_set_wireless_crypto( &crypto_param );

                    extend->set_compl_option |= BIT(CM_ADDIT_SSID_WPA_KEY_MODE);
                    extend->set_compl_option |= BIT(CM_ADDIT_SSID_WPA_CRYPTO_TYPE);
                    extend->set_compl_option |= BIT(CM_ADDIT_SSID_PASSWORD);
                    break;
                case CM_ADDIT_SSID_BRIDGE_ENABLE:
                    enable = util_blobmsg_get_bool( cur_attr, false );
                    snprintf( value, sizeof( value ), "%d", enable );

                    /*
                     * Equipment can't support dual band client-bridge,
                     * so 2.4G-bridge only enabled in 2.4G single band ssid.
                     */
                    if( SSID_BAND_2G4 == ssid_band ) {
                        config_set_wireless( CM_ADDIT_SSID_BRIDGE_ENABLE, iface_2g[id], value, RADIO_2G );
                    }
                    else {
                        config_set_wireless( CM_ADDIT_SSID_BRIDGE_ENABLE, iface_5g[id], value, RADIO_5G );
                    }
                    break;
                case CM_ADDIT_SSID_UPRATE:
                case CM_ADDIT_SSID_DOWNRATE: {
                    struct bwctrl_param bwctrl;
                    int up_rate = util_blobmsg_get_int( config[CM_ADDIT_SSID_UPRATE], 0 );
                    int down_rate = util_blobmsg_get_int( config[CM_ADDIT_SSID_DOWNRATE], 0 );

                    memset( &bwctrl, 0, sizeof( bwctrl ) );
                    bwctrl.up_rate = up_rate;
                    bwctrl.down_rate = down_rate;
                    bwctrl.cm_id = cm_section_name;
                    if( enable ) {
                        if( SSID_BAND_DUAL_FREQ == ssid_band ) {
                            strncpy( bwctrl.iface_2g, iface_2g[id], sizeof( bwctrl.iface_2g ) -1 );
                            strncpy( bwctrl.iface_5g, iface_5g[id], sizeof( bwctrl.iface_5g ) -1 );
                        }
                        else if( SSID_BAND_2G4 == ssid_band ) {
                            strncpy( bwctrl.iface_2g, iface_2g[id], sizeof( bwctrl.iface_2g ) -1 );
                        }
                        else {
                            strncpy( bwctrl.iface_5g, iface_5g[id], sizeof( bwctrl.iface_5g ) -1 );
                        }
                    }

                    config_set_wireless_limit( &bwctrl );
                    option |= BIT( CM_CFG_BWCTRL );
                    return option;
                }
                    break;
                case CM_ADDIT_SSID_SCHEDULE_ENABLE:
                case CM_ADDIT_SSID_SCHEDULE_ID:
                    schedule_id = atoi( util_blobmsg_get_string( config[CM_ADDIT_SSID_SCHEDULE_ID], "-1" ) );
                    snprintf( path, sizeof( path ), "%s.schedule%d.ssid", CF_CONFIG_NAME_SCHEDULE, schedule_id );

                    if ( 0 == config_get_schedule_ssids( schedule_id, value ) ) {
                        config_uci_del( path, 0 );
                    }
                    else {
                        config_uci_set( path, value, 0 );
                    }
                    option |= BIT( CM_CFG_SCHEDULE );
                    break;
                case CM_ADDIT_SSID_VLAN_ENABLE:
                case CM_ADDIT_SSID_VLAN_ID:
                    if ( util_blobmsg_get_bool( config[CM_ADDIT_SSID_VLAN_ENABLE], false ) ) {
                        vlan_id = atoi( util_blobmsg_get_string( config[CM_ADDIT_SSID_VLAN_ID], "0" ) );
                        if ( LAN_DEFAULT_VLAN_ID == vlan_id || 0 == vlan_id ) {
                            /* To maintain compatibility with other models */
                            snprintf( value, BUF_LEN_64, LAN_DEFAULT_INTERFACE );
                        }
                        else {
                            snprintf( value, BUF_LEN_64, "zone%d", vlan_id );
                        }
                    }
                    else {
                        snprintf( value, BUF_LEN_64, LAN_DEFAULT_INTERFACE );
                    }

                    config_set_wireless( CM_ADDIT_SSID_VLAN_ID, iface_2g[id], value, RADIO_2G );
                    config_set_wireless( CM_ADDIT_SSID_VLAN_ID, iface_5g[id], value, RADIO_5G );
                    break;
                default:
                    break;
            }
        default:
            break;
    }

    option |= BIT( CM_CFG_WIRELESS );
    return option;
}

//=============================================================================
static int
cm_system_basic_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    char temp[BUF_LEN_32] = { 0 };
    char data[BUF_LEN_64] = { 0 };
    struct blob_attr *cur_attr = new_config[index];
    int option = 0;
    int loop = 0;
    int action = extend->action;

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case BASIC_TIME_ZONE:
                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ), sizeof( temp ) -1 );

                    for ( loop = 0; loop < timezone_num; loop++ ) {
                        if ( strncmp( temp, timezone_names[loop], strlen(timezone_names[loop]) -1 ) == 0 ) {
                            memcpy( data, timezone_values[loop], strlen(timezone_values[loop]) );
                            config_uci_set( "system.system.timezone", data, 0 );
                            break;
                        }
                    }
                    option |= BIT( CM_CFG_SYSTEM );
                    break;

                case BASIC_COUNTRY_CODE: {
                    struct cfparse_wifi_device *cfwdev;

                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ), sizeof( temp ) -1 );
                    config_uci_set( "grandstream.general.country", temp, 0 );
                    config_uci_set( "wireless.qcawifi.country", temp, 0 );

                    //Maybe some countries don't support 5g channels
                    cfwdev = util_get_vltree_node( &wireless_vltree, VLTREE_DEV, "wifi1" );
                    if( cfwdev ) {
                        config_set_radio_enable( temp, cfwdev->htmode );
                    }

                    option |= BIT( CM_CFG_GRANDSTREAM );
                    option |= BIT( CM_CFG_WIRELESS );
                }
                    break;

                case BASIC_LED_ON:
                    config_creat_no_exist_section( CF_CONFIG_NAME_GRANDSTREAM, "lights", "lights", &grandstream_led_vltree, VLTREE_GRANDSTREAM );
                    strncpy( temp, util_blobmsg_get_string( cur_attr, "" ), sizeof( temp ) -1 );
                    //led on
                    if ( temp[0] == '1' ) {
                        config_uci_set( "grandstream.lights.alwaysOff", "0", 0 );
                    }
                    //led off
                    if ( temp[0] == '0' ) {
                        config_uci_set( "grandstream.lights.alwaysOff", "1", 0 );
                    }
                    option |= BIT( CM_CFG_GRANDSTREAM );
                    break;

                default:
                        break;
            }
    }
    return option;
}

//=============================================================================
static int
cm_extern_sys_log_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend 
)
//=============================================================================
{
    cfmanager_log_message(LOG_ERR, "cm cb\n");
    int option = 0;
    int action = extend->action;
    char temp[BUF_LEN_64] = { 0 };
    struct blob_attr *cur_attr = new_config[index];
    switch( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case CM_EXTERN_LOG_URI:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    if ( temp[0] == '\0' ) {
                        config_uci_del("system.system.log_ip", 0);
                        break;
                    }
                    config_uci_set("system.system.log_ip", temp, 0);
                    break;
                case CM_EXTERN_LOG_LEVEL:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("system.system.log_level", temp, 0);
                    break;
                case CM_EXTERN_LOG_PROTOCOL:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("system.system.log_proto", temp, 0);
                    break;
            }
            option |= BIT(CM_CFG_SYSTEM);
    }

    return option;
}

//=============================================================================
static int
cm_email_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend 
)
//=============================================================================
{
    int option = 0;
    int action = extend->action;
    char temp[BUF_LEN_64] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    unsigned int rem;
    struct blob_attr *cur_attr = new_config[index];
    struct blob_attr *cur;
    switch( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case CM_EMAIL_PORT:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("email.email.port", temp, 0);
                    break;
                case CM_EMAIL_HOST:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("email.email.host", temp, 0);
                    break;
                case CM_EMAIL_USER:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("email.email.user", temp, 0);
                    break;
                case CM_EMAIL_PASSWORD:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("email.email.password", temp, 0);
                    break;
                case CM_EMAIL_DO_NOT_VALIDATE:
                    strncpy( temp, util_blobmsg_get_bool(cur_attr, false) ? "1" : "0", sizeof(temp)-1 );
                    config_uci_set("email.email.do_not_validate", temp, 0);
                    break;
                case CM_EMAIL_ENABLE_NOTIFICATION:
                    strncpy( temp, util_blobmsg_get_bool(cur_attr, false) ? "1" : "0", sizeof(temp)-1 );
                    config_uci_set("email.email.enable_notification", temp, 0);
                    break;
                case CM_EMAIL_FROM_CM_EMAIL_ADDRESS:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("email.email.from_CM_EMAIL_address", temp, 0);
                    break;
                case CM_EMAIL_FROM_NAME:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("email.email.from_name", temp, 0);
                case CM_EMAIL_EMAILADDRESS:
                    sprintf( path, "%s.email.%s", "email", cm_email_policy[CM_EMAIL_EMAILADDRESS].name );
                    config_uci_del( path, 0 );
                    blobmsg_for_each_attr(cur, cur_attr, rem) {
                        config_uci_add_list( path, blobmsg_get_string( cur ), 0 );
                    }
                    break;
            }
            option |= BIT(CM_CFG_EMAIL);
    }

    return option;
}

//=============================================================================
static int
cm_notification_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend 
)
//=============================================================================
{
    int option = 0;
    int action = extend->action;
    char temp[BUF_LEN_64] = { 0 };

    struct blob_attr *cur_attr = new_config[index];
    switch( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case CM_NOTIFY_CM_MEMORY_USAGE:
                    strncpy( temp, util_blobmsg_get_bool(cur_attr, false) ? "1" : "0", sizeof(temp)-1 );
                    config_uci_set("notification.notification.notify_memory_usage", temp, 0);
                    break;
                case CM_MEMORY_USAGE_THRESHOLD:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("notification.notification.memory_usage_threshold", temp, 0);
                    break;
                case CM_NOTIFY_AP_THROUGHPUT:
                    strncpy( temp, util_blobmsg_get_bool(cur_attr, false) ? "1" : "0", sizeof(temp)-1 );
                    config_uci_set("notification.notification.notify_ap_throughput", temp, 0);
                    break;
                case CM_AP_THROUGHPUT_THRESHOLD:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("notification.notification.ap_throughput_threshold", temp, 0);
                    break;
                case CM_NOTIFY_SSID_THROUGHPUT:
                    strncpy( temp, util_blobmsg_get_bool(cur_attr, false) ? "1" : "0", sizeof(temp)-1 );
                    config_uci_set("notification.notification.notify_ssid_throughput", temp, 0);
                    break;
                case CM_SSID_THROUGHPUT_THRESHOLD:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("notification.notification.ssid_throughput_threshold", temp, 0);
                    break;
                case CM_NOTIFY_PASSWORD_CHANGE:
                    strncpy( temp, util_blobmsg_get_bool(cur_attr, false) ? "1" : "0", sizeof(temp)-1 );
                    config_uci_set("notification.notification.notify_password_change", temp, 0);
                    break;
                case CM_NOTIFY_FIRMWARE_UPGRADE:
                    strncpy( temp, util_blobmsg_get_bool(cur_attr, false) ? "1" : "0", sizeof(temp)-1 );
                    config_uci_set("notification.notification.notify_firmware_upgrade", temp, 0);
                    break;
                case CM_NOTIFY_AP_OFFLINE:
                    strncpy( temp, util_blobmsg_get_bool(cur_attr, false) ? "1" : "0", sizeof(temp)-1 );
                    config_uci_set("notification.notification.notify_ap_offline", temp, 0);
                    break;
                case CM_NOTIFY_FIND_ROGUEAP:
                    strncpy( temp, util_blobmsg_get_bool(cur_attr, false) ? "1" : "0", sizeof(temp)-1 );
                    config_uci_set("notification.notification.notify_find_rogueap", temp, 0);
                    break;
            }
            option |= BIT(CM_CFG_NOTIFY);
    }
    
    return option;
}

//=============================================================================
static int
cm_ap_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int action = extend->action;
    int option = 0;

    switch ( action ) {
        case VLTREE_ACTION_DEL:
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
        default:
            extend->set_compl_option |= BIT( BIT_MAX );
            break;
    }

    return option;
}

//=============================================================================
static int
cm_general_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };
//    char value[BUF_LEN_64] = { 0 };
    struct blob_attr *cur_attr = new_config[index];
    int action = extend->action;
    int option = 0;

    cfmanager_log_message( L_DEBUG, "line=%d, action=%d, config[%d]=%s",
        __LINE__, action, index, util_blobmsg_get_string( cur_attr, "" ) );

    cfmanager_log_message( L_DEBUG, "line=%d, action=%d, config[%d]=%s",
        __LINE__, action, index, util_blobmsg_get_string( cur_attr, "" ) );
    switch ( action ) {
        case VLTREE_ACTION_DEL:
            switch( index ) {
                case GENERAL_PAIRING_KEY:
                case GENERAL_FAILOVER_KEY:
                    snprintf( path, sizeof( path ),
                        "%s.general.%s", CF_CONFIG_NAME_GRANDSTREAM, general_policy[index].name );
                    config_uci_del( path, 0 );

                    option |= BIT( CM_CFG_GRANDSTREAM );
                    break;
                case GENERAL_ADMIN_PASSWORD: {
                    /*
                     * After removing ADMIN_PASSWORD from cfmanager,
                     * restore the ADMIN_PASSWORD in Grandstream to its original value
                     */
                    char password[BUF_LEN_16] = { 0 };
                    char desc_pwd[BUF_LEN_128] = { 0 };
                    util_read_file_content( "/proc/gxp/dev_info/security/ssid_password",
                            password, sizeof( password ) );

                    if ( strlen( password ) ) {
                        util_sha256sum( password, desc_pwd, sizeof( desc_pwd ) );
                        snprintf( path, sizeof( path ),
                            "%s.general.%s", CF_CONFIG_NAME_GRANDSTREAM, general_policy[index].name );
                        config_uci_set( path, desc_pwd, 0 );

                        option |= BIT( CM_CFG_GRANDSTREAM );
                    }
                        
                    break;
                }
                default:
                    break;
            }
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case GENERAL_PAIRING_KEY:
                case GENERAL_FAILOVER_KEY:
                    snprintf( path, sizeof( path ), "%s.general.%s", CF_CONFIG_NAME_GRANDSTREAM, general_policy[index].name );
                    config_uci_set( path, util_blobmsg_get_string( cur_attr, "" ), 0 );

                    option |= BIT( CM_CFG_GRANDSTREAM );
                    break;
                case GENERAL_ADMIN_PASSWORD: {
                    const char *password;
                    password = blobmsg_get_string( cur_attr );
                    if ( password ) {
                        snprintf( path, sizeof( path ),
                            "%s.general.%s", CF_CONFIG_NAME_GRANDSTREAM, general_policy[index].name );
                        config_uci_set( path, (char *)password, 0 );

                        option |= BIT( CM_CFG_GRANDSTREAM );
                    }
                    break;
                }
                case GENERAL_WEB_WAN_ACCESS: {
                    bool web_wan_access;
                    snprintf( path, sizeof( path ), "%s.general.%s", CF_CONFIG_NAME_GRANDSTREAM, general_policy[index].name );
                    web_wan_access = util_blobmsg_get_bool( cur_attr, false );
                    if ( !web_wan_access ){
                        config_uci_set( path, "0", 0 );
                    }
                    else{
                        config_uci_set( path, "1", 0 );
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return option;
}

//=============================================================================
static int
cm_tr069_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    int action = extend->action;
    int ret = 0;
    bool config_change = true;
    bool enable = false;
    struct blob_attr *attr = config[index];
    struct tr069_config_parse *tr069_cfg = NULL;
    char *value = NULL;
    char temp[BUF_LEN_32] = { 0 };

    switch( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case CM_TR069_ENABLE:
                    enable = util_blobmsg_get_bool( attr, false );
                    tr069_cfg = util_get_vltree_node( &tr069_vltree, VLTREE_TR069, "tr069" );

                    if( enable ) {
                        if( !tr069_cfg ) {
                            ret = config_add_named_section( "tr069", "tr069", "tr069" );
                            if( ret < 0 ) {
                                cfmanager_log_message( L_ERR, "add tr069 section failed\n" );
                                option |= BIT( BIT_MAX );
                                config_change = false;
                                break;
                            }
                        }
                    }

                    snprintf( temp, sizeof( temp ), "%d", enable );
                    config_uci_set( "tr069.tr069.enable", temp, 0 );
                    break;
                case CM_TR069_ACS_URL:
                    value = util_blobmsg_get_string( attr, "" );
                    config_uci_set( "tr069.tr069.acs_url", value, 0 );
                    break;
                case CM_TR069_ACS_NAME:
                    value = util_blobmsg_get_string( attr, "" );
                    config_uci_set( "tr069.tr069.acs_name", value, 0 );
                    break;
                case CM_TR069_ACS_PASSWORD:
                    value = util_blobmsg_get_string( attr, "" );
                    config_uci_set( "tr069.tr069.acs_password", value, 0 );
                    break;
                case CM_TR069_PERIODIC_INFORM_ENABLE:
                    enable = util_blobmsg_get_bool( attr, false );
                    snprintf( temp, sizeof( temp ), "%d", enable );
                    config_uci_set( "tr069.tr069.periodic_inform_enable", temp, 0 );
                    break;
                case CM_TR069_PERIODIC_INFORM_INTERVAL:
                    value = util_blobmsg_get_string( attr, "" );
                    config_uci_set( "tr069.tr069.periodic_inform_interval", value, 0 );
                    break;
                case CM_TR069_CPE_CERT_FILE:
                    value = util_blobmsg_get_string( attr, "" );
                    config_uci_set( "tr069.tr069.cpe_cert_file", value, 0 );
                    break;
                case CM_TR069_CPE_CERT_KEY:
                    value = util_blobmsg_get_string( attr, "" );
                    config_uci_set( "tr069.tr069.cpe_cert_key", value, 0 );
                    break;
                case CM_TR069_CONN_REQ_NAME:
                    value = util_blobmsg_get_string( attr, "" );
                    config_uci_set( "tr069.tr069.conn_req_name", value, 0 );
                    break;
                case CM_TR069_CONN_REQ_PASSWORD:
                    value = util_blobmsg_get_string( attr, "" );
                    config_uci_set( "tr069.tr069.conn_req_password", value, 0 );
                    break;
                case CM_TR069_CONN_REQ_PORT:
                    value = util_blobmsg_get_string( attr, "" );
                    config_uci_set( "tr069.tr069.conn_req_port", value, 0 );
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    if( config_change ) {
        option |= BIT( CM_CFG_TR069 );
    }

    return option;
}

//=============================================================================
static int
cm_client_limit_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *cur_attr = new_config[index];
    struct bwctrl_config_parse *bwctrl_cfpar = NULL;
    char mac[MAC_STR_MAX_LEN+1] = { 0 };
    char format_mac[MAC_STR_MAX_LEN+1] = { 0 };
    char path[LOOKUP_STR_SIZE];
    char rate_str[BUF_LEN_32] = { 0 };
    //char ssid_name[SSID_NAME_MAX_LEN+1];
    //char *iface = NULL;
    int option = 0;
    int action = extend->action;
    bool enable = false;

    if( !new_config[CLIENT_LIMIT_MAC] ) {
        cfmanager_log_message( L_ERR, "Lack of MAC information\n" );
        return 0;
    }

    strncpy( mac, util_blobmsg_get_string( new_config[CLIENT_LIMIT_MAC], "" ), MAC_STR_MAX_LEN );
    bwctrl_cfpar = util_get_vltree_node( &bwctrl_rule_vltree, VLTREE_BWCTRL, mac );

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case CLIENT_LIMIT_ENABLE:
                    enable = util_blobmsg_get_bool( cur_attr, false );

                    if( enable ) {
                        if( !bwctrl_cfpar ) {
                            config_add_named_section( "bwctrl", "bwctrl-rule", mac );
                        }

                        snprintf( path, sizeof( path ),
                            "bwctrl.%s.enabled", mac );
                        config_uci_set( path, "1", 0 );

                        extend->need_set_option |= BIT( CLIENT_LIMIT_MAC );
                        extend->need_set_option |= BIT( CLIENT_LIMIT_UPRAT );
                        extend->need_set_option |= BIT( CLIENT_LIMIT_DOWNRATE );
                        extend->need_set_option |= BIT( CLIENT_LIMIT_SSID_NAME );
                    }
                    else {
                        if( bwctrl_cfpar ) {
                            snprintf( path, sizeof( path ),
                                "bwctrl.%s", mac );
                            config_uci_del( path, 0 );
                        }

                        extend->set_compl_option |= BIT( BIT_MAX );
                    }
                    break;
                case CLIENT_LIMIT_MAC:
                    snprintf( path, sizeof( path ), "bwctrl.%s.id", mac );
                    util_formatted_mac_with_colo( mac, format_mac );
                    config_uci_set( path, format_mac, 0 );

                    snprintf( path, sizeof( path ), "bwctrl.%s.type", mac );
                    config_uci_set( path, "mac", 0 );
                    break;
                case CLIENT_LIMIT_UPRAT:
                case CLIENT_LIMIT_DOWNRATE:
                    if( CLIENT_LIMIT_UPRAT == index ) {
                        extend->set_compl_option |= BIT( CLIENT_LIMIT_DOWNRATE );
                    }

                    snprintf( path, sizeof( path ), "bwctrl.%s.urate", mac );
                    snprintf( rate_str, sizeof( rate_str ),
                        "%sKbps", util_blobmsg_get_string( new_config[CLIENT_LIMIT_UPRAT], "" ) );
                    config_uci_set( path, rate_str, 0 );

                    snprintf( path, sizeof( path ), "bwctrl.%s.drate", mac );
                    snprintf( rate_str, sizeof( rate_str ),
                        "%sKbps", util_blobmsg_get_string( new_config[CLIENT_LIMIT_DOWNRATE], "" ) );
                    config_uci_set( path, rate_str, 0 );

                    snprintf( path, sizeof( path ), "bwctrl.%s.dev", mac );
                    if( bwctrl_cfpar ) {
                        config_uci_del( path, 0 );
                    }

                    /* Maybe the wireless interface has changed.
                     * We need to reset the wireless interface every time
                     */
                    config_set_client_limit_iface( path, RADIO_2G );
                    config_set_client_limit_iface( path, RADIO_5G );
                    break;
                case CLIENT_LIMIT_SSID_NAME:
#if 0
                    snprintf( path, sizeof( path ), "bwctrl.%s.dev", mac );
                    if( bwctrl_cfpar ) {
                        config_uci_del( path, 0 );
                    }

                    memset( ssid_name, 0, sizeof( ssid_name ) );
                    strncpy( ssid_name, util_blobmsg_get_string( cur_attr, "" ), SSID_NAME_MAX_LEN );
                    iface = config_get_iface_by_ssid_name( ssid_name, RADIO_2G );
                    if( iface ) {
                        config_uci_add_list( path, iface, 0 );
                    }

                    iface = config_get_iface_by_ssid_name( ssid_name, RADIO_5G );
                    if( iface ) {
                        config_uci_add_list( path, iface, 0 );
                    }
#endif
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    option |= BIT( CM_CFG_BWCTRL );

    return option;
}

//=============================================================================
static int
cm_usb_share_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend 
)
//=============================================================================
{
    int option = 0;
    int action = extend->action;
    unsigned char enable = 0;
    char temp[BUF_LEN_64] = { 0 };
    struct blob_attr *cur_attr = new_config[index];
    struct blob_attr *temp_attr = NULL;
    switch( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case USB_SHARE_ENABLE:
                    enable = util_blobmsg_get_bool(cur_attr, false);
                    snprintf(temp, sizeof(temp), "%d", enable);
                    config_uci_set("samba.global.enable", temp, 0);
                    break;
                case USB_SHARE_USER:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("samba.user.username", temp, 0);
                case USB_SHARE_ANONYMITY:
                    temp_attr = new_config[USB_SHARE_ANONYMITY];
                    enable = util_blobmsg_get_bool(temp_attr, false);
                    if( !enable ) {
                        temp_attr = new_config[USB_SHARE_USER];
                        strncpy( temp, util_blobmsg_get_string(temp_attr, ""), sizeof(temp)-1 );
                        config_uci_set("samba.global.valid_users", temp, 0);
                    } else {
                        config_uci_del("samba.global.valid_users", 0);
                    }
                    break;
                case USB_SHARE_PASSWORD:
                    strncpy( temp, util_blobmsg_get_string(cur_attr, ""), sizeof(temp)-1 );
                    config_uci_set("samba.user.password", temp, 0);
                    break;
            }
            option |= BIT(CM_CFG_SAMBA);
    }

    return option;
}

//=============================================================================
static int
cm_vpn_server_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    char section_name[BUF_LEN_64] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    char metric[BUF_LEN_16]= { 0 };
    int create = 0;
    int option = 0;
    int vpn_id;
    char *new_wan = NULL, *old_wan = NULL;
    int new_wan_index, old_wan_index;
    int new_server_type, old_server_type;
    int enable_change = 0, type_change = 0, wan_change = 0;
    int enable = 0;

    vpn_id = util_blobmsg_get_int( config[VPN_SERVER_ID], VPN_SERVER_ID_OFFSET ) +
            VPN_SERVER_ID_OFFSET;
    snprintf( section_name, sizeof(section_name), "vpn%d", vpn_id );
    enable = util_blobmsg_get_bool( config[VPN_SERVER_ENABLE], false );
    new_server_type = util_blobmsg_get_int( config[VPN_SERVER_TYPE], VPN_SERVER_TYPE_IPSEC );
    new_wan = blobmsg_get_string( config[VPN_SERVER_WAN_INTF] );
    new_wan_index = !strcmp( new_wan, "wan0" ) ? WAN0 : WAN1;

    switch ( extend->action ) {
        case VLTREE_ACTION_DEL:
            // Only delete already enabled vpn server.
            extend->set_compl_option |= BIT( BIT_MAX );
            if ( enable ) {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_DISABLE,
                        NULL );
                }
                config_del_named_section( "network", "interface", section_name );
            }
            else {
                return option;
            }
            break;

        case VLTREE_ACTION_ADD:
            // Only add enabled vpn server, ipsec server always enabled for GWN7052.
            extend->set_compl_option |= BIT( BIT_MAX );
            if ( enable ) {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_ENABLE,
                        NULL );
                }
                create = 1;
            }
            else {
                return option;
            }
            break;

        case VLTREE_ACTION_UPDATE:
            switch ( index ) {
                case VPN_SERVER_NAME:
                    // It doesn't matter if server name changed.
                    return option;

                case VPN_SERVER_ENABLE:
                    if ( enable ) {
                        create = 1;
                    }
                    else {
                        config_del_named_section( "network", "interface", section_name );
                    }
                    enable_change = 1;
                    extend->set_compl_option |= BIT( BIT_MAX );
                    // go through

                case VPN_SERVER_TYPE:
                    old_server_type = util_blobmsg_get_int( extend->old_config[VPN_SERVER_TYPE], 0 );
                    if ( new_server_type != old_server_type ) {
                        if ( !enable_change ) { // Enable changed will adjust network config.
                            if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                                if ( old_server_type == VPN_SERVER_TYPE_OPENVPN ) {
                                    // TODO: del  network setting for openvpn
                                }
                            
                                snprintf( path, sizeof(path), "network.%s.proto", section_name );
                                config_uci_set( path, "none", 0 );
                                snprintf( path, sizeof(path), "network.%s.ifname", section_name );
                                if ( new_wan_index == WAN0 ) {
                                    config_uci_set( path, "ipsec0", 0 );
                                }
                                else {
                                    config_uci_set( path, "ipsec1", 0 );
                                }
                            }
                            else if ( new_server_type == VPN_SERVER_TYPE_OPENVPN ) {
                                if ( old_server_type == VPN_SERVER_TYPE_IPSEC ) {
                                    // delete ifname option
                                    snprintf( path, sizeof(path), "network.%s.ifname", section_name );
                                    config_uci_del( path, 0 );
                                }
                            }
                        }

                        type_change = 1;
                        extend->set_compl_option |= BIT( BIT_MAX );
                    }
                    // go through

                case VPN_SERVER_WAN_INTF:
                    old_wan = blobmsg_get_string( extend->old_config[VPN_SERVER_WAN_INTF] );
                    old_wan_index = !strcmp( old_wan, "wan0" ) ? WAN0 : WAN1;
                    snprintf( metric, sizeof(metric), "%d", vpn_id - VPN_SERVER_ID_OFFSET + VPN_SERVER_METRIC_OFFSET ); 
                    snprintf( path, sizeof(path), "network.%s.metric", section_name );
                    if ( new_wan_index != old_wan_index ) {
                        if ( !enable_change ) { // Enable changed will adjust network config.
                            sprintf( path, "network.%s.interface", section_name );
                            config_uci_set( path, new_wan, 0 );
                            if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                                snprintf( path, sizeof(path), "network.%s.ifname", section_name );
                                if ( new_wan_index == WAN0 ) {
                                    config_uci_set( path, "ipsec0", 0 );
                                }
                                else {
                                    config_uci_set( path, "ipsec1", 0 );
                                }
                            }
                        }

                        wan_change = 1;
                        extend->set_compl_option |= BIT( BIT_MAX );
                    }
                    break;
            }
            break;
    }

    if ( enable_change || type_change || wan_change ) {
        // Do permutation and combination between these changes.
        if ( enable_change && type_change && wan_change ) {
            if ( enable ) {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn server again, change server type from others to ipsec, "
                        "change wan from %s to %s!\n",
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_ENABLE,
                        NULL );
                }
                // else {}
            }
            else {
                if ( old_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Diabled vpn server again, change server type from ipsec to others, "
                        "change wan from %s to %s!\n",
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index, 
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_DISABLE,
                        NULL );
                }
                // else {}
            }
        }
        else if ( enable_change && type_change ) {
            if ( enable ) {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn server again, change server type from others to ipsec!\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_ENABLE,
                        NULL );
                }
                // else {}
            }
            else {
                if ( old_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Disabled vpn server again, change server type from ipsec to other!\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_DISABLE,
                        NULL );
                }
                // else {}
            }
        }
        else if ( enable_change && wan_change ) {
            if ( enable ) {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled ipsec server again, change wan from %s to %s!\n", 
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_ENABLE,
                        NULL );
                }
                // else {}
            }
            else {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Disabled ipsec server again, change wan from %s to %s!\n", 
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_DISABLE,
                        NULL );
                }
                // else {}
            }
        }
        else if ( enable_change ) {
            if ( enable ) {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled ipsec server again!\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_ENABLE,
                        NULL );
                }
                // else {}
            }
            else {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Disabled ipsec server again!\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_DISABLE,
                        NULL );
                }
                // else {}
            }
        }
        else if ( type_change && wan_change ) {
            if ( enable ) {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn server, change server type from other to ipsec, "
                        "change wan from %s to %s!\n", 
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_ENABLE,
                        NULL );
                }
                else if ( old_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn server, change server type from ipsec to other, "
                        "change wan from %s to %s!\n", 
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_DISABLE,
                        NULL );
                }
                // else {}
            }
            // Don't care 'Disabled';
        }
        else if ( type_change ) {
            if ( enable ) {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn server, change server type from other to ipsec\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_ENABLE,
                        NULL );
                }
                else if ( old_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled vpn server, change server type from ipsec to other\n" );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_DISABLE,
                        NULL );
                }
                // else {}
            }
            // Don't care 'Disabled'
        }
        else if ( wan_change ) {
            if ( enable ) {
                if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
                    cfmanager_log_message( L_INFO,
                        "Enabled ipsec server, change wan from %s to %s\n",
                        old_wan, new_wan );
                    vpn_update_ipsec_cnt_and_reload_helper( old_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_DISABLE,
                        NULL );
                    vpn_update_ipsec_cnt_and_reload_helper( new_wan_index,
                        VPN_ROLE_SERVER,
                        IPSEC_STATUS_ENABLE,
                        NULL );
                }
                // else {}
            }
            // Don't care 'Disabled'
        }
    }

    if ( create ) {
        config_add_named_section( "network", "interface", section_name );
        snprintf( metric, sizeof(metric), "%d", vpn_id - VPN_SERVER_ID_OFFSET + VPN_SERVER_METRIC_OFFSET ); 
        snprintf( path, sizeof(path), "network.%s.metric", section_name );
        config_uci_set( path, metric, 0 );
        sprintf( path, "network.%s.interface", section_name );
        config_uci_set( path, new_wan, 0 );

        snprintf( path, sizeof(path), "network.%s.proto", section_name );
        config_uci_set( path, "none", 0 );
        snprintf( path, sizeof(path), "network.%s.ifname", section_name );
        if ( new_server_type == VPN_SERVER_TYPE_IPSEC ) {
            if ( !strcmp(new_wan, "wan0") ) {
                config_uci_set( path, "ipsec0", 0 );
            }
            else {
                config_uci_set( path, "ipsec1", 0 );
            }
        }
    }

    option |= BIT( CM_CFG_NETWORK );

    return option;
}

//=============================================================================
static int
cm_ipsec_cmn_setting_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    struct blob_attr *server_tb[__VPN_SERVER_MAX];
    struct blob_attr *cur = NULL;
    int rem;
    struct cm_config *cm_cfg = NULL;
    char *section_prefix = "ipsec_cmn_setting";
    char server_section[BUF_LEN_32] = { 0 };
    int server_id;
    int option = 0;

    switch ( extend->action ) {
        case VLTREE_ACTION_DEL:
        case VLTREE_ACTION_ADD:
            // If ipsec server hook has done something required.
            // We don't need to handler these situations at here.
            extend->set_compl_option |= BIT( BIT_MAX );
            break;

        case VLTREE_ACTION_UPDATE:
            switch ( index ) {
                case IPSEC_CMN_SETTING_ATTR_PSK:
                case IPSEC_CMN_SETTING_ATTR_ENCRYPT_ALG:
                case IPSEC_CMN_SETTING_ATTR_AUTH_ALG:
                case IPSEC_CMN_SETTING_ATTR_DH:
                    if ( ipsec_final_reload_action == IPSEC_RELOAD_ACTION_START || 
                        ipsec_final_reload_action == IPSEC_RELOAD_ACTION_STOP ||
                        ipsec_final_reload_action == IPSEC_RELOAD_ACTION_RESTART ) {
                        cfmanager_log_message( L_INFO,
                            "ipsec has final reload action '%s', "
                            "so we don't have to handler ipsec common setting changed !\n",
                             ipsec_action2str[ipsec_final_reload_action] );

                        extend->set_compl_option |= BIT( BIT_MAX );
                        return option;
                    }

                    server_id = atoi( &extend->section_name[strlen(section_prefix)] );
                    snprintf( server_section, sizeof(server_section), "vpn_server%d", server_id );
                    cm_cfg = util_get_vltree_node( &cm_vpn_server_vltree, VLTREE_CM_TREE, server_section );
                    if ( !cm_cfg ) {
                        cfmanager_log( L_ERR, "Can not find ipsec server '%s' by ipsec comon setting '%s'\n",
                            server_section, extend->section_name );
                    }
                    else {
                        blobmsg_parse( vpn_server_policy,
                            __VPN_SERVER_MAX,
                            server_tb,
                            blobmsg_data( cm_cfg->cf_section.config ),
                            blobmsg_len( cm_cfg->cf_section.config ) );

                        if ( blobmsg_get_bool(server_tb[VPN_SERVER_ENABLE]) ) {
                            /*
                             * We should replace all dial-in user of this ipsec server 
                             * if anything changed for ipsec common setting when ipsec server enabled.
                             */
                            blobmsg_for_each_attr( cur, server_tb[VPN_SERVER_IPSEC_DIAL_IN_USER], rem ) {
                                vpn_update_ipsec_reload_helper( IPSEC_RELOAD_ACTION_REPLACE_CONN, 
                                    blobmsg_get_string(cur) );
                            }
                            vpn_user_handle_done[server_id] = 1;
                        }
                    }

                    // Trigger ipsec reload.
                    apply_add( "ipsec" );
                    apply_timer_start();

                    extend->set_compl_option |= BIT( BIT_MAX );
                    break;
            }
            break;
    }

    return option;
}

//=============================================================================
static int
cm_ipsec_dial_in_user_hooker(
    struct blob_attr **config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int server_id = 0;   
    int option = 0;

    if ( ipsec_final_reload_action == IPSEC_RELOAD_ACTION_START || 
        ipsec_final_reload_action == IPSEC_RELOAD_ACTION_STOP ||
        ipsec_final_reload_action == IPSEC_RELOAD_ACTION_RESTART ) {
        cfmanager_log_message( L_INFO,
            "ipsec has final reload action '%s', "
            "so we don't have to handler ipsec common setting changed !\n",
             ipsec_action2str[ipsec_final_reload_action] );

        extend->set_compl_option |= BIT( BIT_MAX );
        return option;
    }

    sscanf( extend->section_name, "vpn_server%d_dial_in_user%*d", &server_id );
    if ( vpn_user_handle_done[server_id] ) {
        cfmanager_log_message( L_ERR, "Don't need to handle with %s, "
            "because ipsec commnon setting has handled it!\n",
            extend->section_name );

        extend->set_compl_option |= BIT( BIT_MAX );
        return option;
    }

    switch ( extend->action ) {
        case VLTREE_ACTION_DEL:
            vpn_update_ipsec_reload_helper( IPSEC_RELOAD_ACTION_DEL_CONN,
                (char *)extend->section_name );
            break;
        case VLTREE_ACTION_ADD:
            vpn_update_ipsec_reload_helper( IPSEC_RELOAD_ACTION_ADD_CONN,
                (char *)extend->section_name );      
            break;

        case VLTREE_ACTION_UPDATE:
            switch ( index ) {
                case IPSEC_DIAL_IN_USER_ATTR_ID:
                case IPSEC_DIAL_IN_USER_ATTR_TYPE:
                case IPSEC_DIAL_IN_USER_ATTR_USER:
                case IPSEC_DIAL_IN_USER_ATTR_PASSWORD:
                case IPSEC_DIAL_IN_USER_ATTR_VLAN_ID:
                case IPSEC_DIAL_IN_USER_ATTR_IP_RANGE:
                    vpn_update_ipsec_reload_helper( IPSEC_RELOAD_ACTION_REPLACE_CONN,
                        (char *)extend->section_name ); 
                    break;
            }
            break;
    }

    extend->set_compl_option |= BIT( BIT_MAX );
    return option;
}

//=============================================================================
static int
cm_portal_policy_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    int action = extend->action;
    int i = 0, j = 0;
    int idx = 0, tmp_idx = 0;
    struct gsportalcfg_config_parse *gsportalcfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char name[BUF_LEN_32] = { 0 };
    const char *section_name = NULL;
    const char *value = NULL;
    const char *id = NULL;

    struct blob_attr *cur = NULL;
    int rem = 0;

    switch ( action ) {
        case VLTREE_ACTION_DEL:
        case VLTREE_ACTION_UPDATE:
            if ( new_config[PORTAL_POLICY_ID] )
                id = blobmsg_get_string( new_config[PORTAL_POLICY_ID] );

            if ( !id ) {
                break;
            }

            config_del_named_section( "gsportalcfg", "portal_policy", id );

            if ( action == VLTREE_ACTION_DEL ) {
                break;
            }
        case VLTREE_ACTION_ADD:
            if ( new_config[PORTAL_POLICY_ID] ) {
                strncpy( name,
                    blobmsg_get_string( new_config[PORTAL_POLICY_ID] ),
                    sizeof( name ) - 1 );
            }
            else {
                vlist_for_each_element(
                    &gsportalcfg_portal_policy_vltree,
                    gsportalcfg, node ) {

                    section_name = gsportalcfg->cf_section.name;

                    sscanf( section_name, "portal_policy_%d", &tmp_idx );

                    idx |= BIT( tmp_idx );
                }

                for ( i = 0; i < MAX_PORTAL_POLICY_NUMBER; i++ ) {
                    if ( !( idx & BIT( i ) ) ) {
                        snprintf( name, sizeof( name ), "portal_policy_%d", i );
                        break;
                    }
                }
            }

            if ( strlen( name ) == 0 )
                break;

            config_add_named_section( "gsportalcfg", "portal_policy", name );

            for ( j = __PORTAL_POLICY_MIN;
                    j < __PORTAL_POLICY_MAX; j++ ) {
                if ( !new_config[j] ) {
                    continue;
                }

                snprintf( path, sizeof( path ),
                    "gsportalcfg.%s.%s", name,
                    gsportalcfg_portal_policy[j].name );


                switch( j ) {
                case PORTAL_POLICY_ID:
                    config_uci_set( path, name, 0 );
                    break;
                case PORTAL_POLICY_PRE_AUTH:
                case PORTAL_POLICY_POST_AUTH:
                    if ( !new_config[j] ) {
                        break;
                    }
                    blobmsg_for_each_attr( cur, new_config[j], rem ) {
                        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING )
                            return ERRCODE_PARAMETER_ERROR;

                        value = blobmsg_get_string( cur );
                        config_uci_add_list( path, (char *)value, 0 );
                    };
                    break;
                default:
                    config_set_by_blob( new_config[j],
                        path, gsportalcfg_portal_policy[j].type );

                    break;
                }
            }
            break;
        default:
            break;
    }


    extend->set_compl_option |= BIT( BIT_MAX );

    option |= BIT( CM_CFG_GSPORTAL );

    return option;
}

//=============================================================================
static int
cm_wan_alias_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    int action = extend->action;
    int rem = 0;
    int i = 0;
    int j = 0;
    struct blob_attr *cur_ip = NULL;
    struct network_config_parse *nw_node = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_64];
    char section_name[BUF_LEN_32];
    bool enable = false;
    char *cm_section_name = extend->section_name;
    char *nw_section_prefix = NULL;
    int wan_type = 0;
    int ret = 0;

    if( 0 == strcmp( CM_WAN0_ALIAS_SECTION_NAME, cm_section_name ) ) {
        nw_section_prefix = WAN0_ALIAS_SECTION_PREFIX;
        wan_type = WAN0;
    }
    else {
        nw_section_prefix = WAN1_ALIAS_SECTION_PREFIX;
        wan_type = WAN1;
    }

    switch ( action ) {
        case VLTREE_ACTION_DEL:
            break;
        case VLTREE_ACTION_UPDATE:
        case VLTREE_ACTION_ADD:
            switch( index ) {
                case WAN_ALIAS_ATTR_ENABLE:
                case WAN_ALIAS_ATTR_IP:
                    enable = util_blobmsg_get_bool( new_config[WAN_ALIAS_ATTR_ENABLE], false );

                    if( enable ) {
                        blobmsg_for_each_attr( cur_ip, new_config[WAN_ALIAS_ATTR_IP], rem ) {
                            if( BLOBMSG_TYPE_STRING != blobmsg_type( cur_ip ) ) {
                                continue;
                            }

                            snprintf( section_name, sizeof( section_name ),
                                "%s%d", nw_section_prefix, i );
                            nw_node = util_get_vltree_node( &network_alias_vltree, VLTREE_NETWORK, section_name );
                            if( !nw_node ) {
                                ret = config_add_named_section( "network", "alias", section_name );
                                if( ret < 0 ) {
                                    cfmanager_log_message( L_ERR, "create section '%s' failed\n", section_name );
                                    return 0;
                                }
                            }

                            config_set_wan_alias_default_cfg( section_name, wan_type );

                            memset( value, 0, sizeof( value ) );
                            strncpy( value, util_blobmsg_get_string( cur_ip, "" ), sizeof( value )-1 );
                            snprintf( path, sizeof( path ), "network.%s.ipaddr", section_name );
                            config_uci_set( path, value, 0 );

                            i++;
                        }
                    }

                    /*
                     * After setting the IP address distributed on the page,
                     * delete the previously set IP address
                     */
                    for( j = WAN_ALIAS_COUNT_MAX; j >= i; j-- ) {
                        snprintf( section_name, sizeof( section_name ),
                            "%s%d", nw_section_prefix, j );
                        nw_node = util_get_vltree_node( &network_alias_vltree, VLTREE_NETWORK, section_name );
                        if( !nw_node ) {
                            continue;
                        }

                        config_del_named_section( "network", "alias", section_name );
                        cfmanager_log_message( L_DEBUG, "del '%s'", section_name );
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    option |= BIT( CM_CFG_NETWORK );

    return option;
}

//=============================================================================
void
cm_update_cfg(
    const struct blobmsg_policy *policy,
    struct cm_config *cmcfg_old,
    struct cm_config *cmcfg_new,
    int policy_size,
    int hooker,
    int action
)
//=============================================================================
{
    struct blob_attr *tb_old[policy_size];
    struct blob_attr *tb_new[policy_size];
    struct cm_vltree_extend extend;
    struct cm2gs_extend cm2gs;
    int i = 0;

    struct blob_attr *config_old = cmcfg_old->cf_section.config;
    struct blob_attr *config_new = cmcfg_new->cf_section.config;

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
    extend.action = action;
    extend.section_name = cmcfg_old->cf_section.name;
    extend.old_config = tb_old;

    memset( &cm2gs, 0, sizeof( cm2gs ) );
    cm2gs.action = action;
    cm2gs.section_name = cmcfg_old->cf_section.name;

    for( i = 0; i < policy_size; i++ ) {
        if( cm_force_load_flag ||
            !blob_attr_equal( tb_new[i], tb_old[i] ) ||
            extend.need_set_option & BIT( i )) {

            //If the highest position is 1, all items will be skipped after that
            if( extend.set_compl_option & BIT( BIT_MAX ) ) {
                break;
            }
            else if( extend.set_compl_option & BIT( i ) ) {
                continue;
            }

            if ( cm_hp[hooker].cb ) {
                cfg_option |= cm_hp[hooker].cb( tb_new, i, &extend );
            }
        }
    }

    //Update the configuration in the corresponding grandstream
    if( cm_hp[hooker].cm2gs_cb ) {
        cfmanager_log_message( L_DEBUG, "cfmanager section '%s' convert to grandstream\n", cm2gs.section_name );
        cfg_option |= cm_hp[hooker].cm2gs_cb( tb_new, &cm2gs );
    }
}

//=============================================================================
static void
cm_call_update_func(
    const char *section_type,
    struct cm_config *cmcfg_old,
    struct cm_config *cmcfg_new,
    int action
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type || NULL == cmcfg_old || NULL == cmcfg_new ) {
        return;
    }

    for( i = 1; i < __CM_VLTREE_MAX; i++ ) {

        if( 0 == strcmp( cm_vltree_info[i].key, section_type ) ) {

            cm_update_cfg( cm_vltree_info[i].policy_list->params,
                cmcfg_old,
                cmcfg_new,
                cm_vltree_info[i].policy_list->n_params,
                cm_vltree_info[i].hooker,
                action );
        }
    }
}

//=============================================================================
static void
cm_vltree_handle_add_or_del(
    struct cm_config *cmcfg,
    int action
)
//=============================================================================
{
    int policy_size = 0;
    int hooker = 0;
    int i = 0;
    const struct blobmsg_policy *policy = NULL;
    const struct uci_blob_param_list *blob_policy = NULL;
    struct cm_vltree_extend extend;
    struct cm2gs_extend cm2gs;

    if( !cmcfg ) {
        return;
    }

    hooker = cm_find_hooker( cmcfg->cf_section.type );
    if( 0 > hooker ) {
        cfmanager_log_message( L_WARNING,
            "There is no corresponding callback processing for %s\n",
            cmcfg->cf_section.type );

        return;
    }

    blob_policy = cm_find_blob_param_list( cmcfg->cf_section.type );
    if( NULL == blob_policy ) {
        cfmanager_log_message( L_WARNING,
            "There is no corresponding blob param list for %s\n",
            cmcfg->cf_section.type );

        return;
    }

    policy = blob_policy->params;
    policy_size = blob_policy->n_params;

    struct blob_attr *tb[policy_size];

    blobmsg_parse( policy,
        policy_size,
        tb,
        blob_data( cmcfg->cf_section.config ),
        blob_len( cmcfg->cf_section.config ) );

    memset( &extend, 0, sizeof( extend ) );
    extend.action = action;
    extend.section_name = cmcfg->cf_section.name;

    memset( &cm2gs, 0, sizeof( cm2gs ) );
    cm2gs.action = action;
    cm2gs.section_name = cmcfg->cf_section.name;

    for( i = 0; i < policy_size; i++ ) {

        //If the highest position is 1, all items will be skipped after that
        if( extend.set_compl_option & BIT( BIT_MAX ) ) {
            break;
        }
        else if( extend.set_compl_option & BIT( i ) ) {
            continue;
        }

        if ( cm_hp[hooker].cb ) {
            cfg_option |= cm_hp[hooker].cb( tb, i, &extend );
        }
    }

    //Update the configuration in the corresponding grandstream
    if( cm_hp[hooker].cm2gs_cb ) {
        cfg_option |= cm_hp[hooker].cm2gs_cb( tb, &cm2gs );
    }
}

//=============================================================================
static void
cm_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct cm_config *cmcfg_old = NULL;
    struct cm_config *cmcfg_new = NULL;

    if ( node_old )
        cmcfg_old =
            container_of(node_old, struct cm_config, node);

    if ( node_new )
        cmcfg_new =
            container_of(node_new, struct cm_config, node);

    if ( cmcfg_old && cmcfg_new ) {
        cfmanager_log_message( L_WARNING,
            "update cfmanager section  type '%s', name '%s'\n", 
            cmcfg_old->cf_section.type, cmcfg_old->cf_section.name );

        if ( !cm_force_load_flag &&
                blob_attr_equal( cmcfg_old->cf_section.config, cmcfg_new->cf_section.config ) ) {
            cm_node_free( cmcfg_new );
            return;
        }

        cm_call_update_func( cmcfg_old->cf_section.type, cmcfg_old, cmcfg_new, VLTREE_ACTION_UPDATE );

        SAFE_FREE( cmcfg_old->cf_section.config );
        cmcfg_old->cf_section.config = blob_memdup( cmcfg_new->cf_section.config );
        cm_node_free( cmcfg_new );
    }
    else if ( cmcfg_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete cfmanager section type '%s', name '%s'\n",
            cmcfg_old->cf_section.type, cmcfg_old->cf_section.name );

        vltree = cm_find_tree( cmcfg_old->cf_section.type );
        if( NULL == vltree ) {
            cfmanager_log_message( L_DEBUG, "No tree corresponding to %s was found\n",
                cmcfg_old->cf_section.type );

            return;
        }

        cm_vltree_handle_add_or_del( cmcfg_old, VLTREE_ACTION_DEL );
        cm_tree_free( vltree, cmcfg_old );
    }
    else if ( cmcfg_new ) {
        cfmanager_log_message( L_WARNING,
            "New cfmanager section type '%s', name '%s'\n", 
            cmcfg_new->cf_section.type, cmcfg_new->cf_section.name );

        cm_vltree_handle_add_or_del( cmcfg_new, VLTREE_ACTION_ADD );
    }
}

//=============================================================================
static void
cm_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct cm_config *cmcfg = NULL;
    struct vlist_tree *vltree = NULL;
    char *name;
    char *type;

    vltree = cm_find_tree( section_type );
    if( NULL == vltree ) {
        cfmanager_log_message( L_DEBUG, "No corresponding binary tree was found according to %s\n",
            section_type );

        return;
    }

    cmcfg = calloc_a( sizeof( *cmcfg ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type) +1 );

    if ( !cmcfg ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc cm_config '%s'\n", section_name );

        return;
    }

    cmcfg->cf_section.name = strcpy( name, section_name );
    cmcfg->cf_section.type = strcpy( type, section_type );
    cmcfg->cf_section.config = blob_memdup( data );

    vlist_add( vltree, &cmcfg->node, cmcfg->cf_section.name );
}

//=============================================================================
static void
cm_uci_to_blob(
    struct uci_section *s,
    int section
)
//=============================================================================
{
    const struct uci_blob_param_list* uci_blob_list = NULL;

    blob_buf_init( &b, 0 );

    if( CM_VLTREE_ALL == section ) {
        uci_blob_list = cm_find_blob_param_list( s->type );
    }
    else {
        uci_blob_list = cm_vltree_info[section].policy_list;
    }

    if( NULL == uci_blob_list ) {
        cfmanager_log_message( L_DEBUG, "No corresponding uci_blob_param_list was found according to %s\n",
            s->type );

        return;
    }

    uci_to_blob( &b, s, uci_blob_list );
    cm_add_blob_to_tree( b.head, s->e.name, s->type );

    blob_buf_free( &b );
}

//=============================================================================
static void
cm_load_section(
    struct uci_package *package,
    const char *section_type,
    int section
)
//=============================================================================
{
    struct uci_element *e = NULL;

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        if( 0 != strcmp( section_type, s->type ) ) {
            continue;
        }

        cm_uci_to_blob( s, section );
    }
}

//=============================================================================
int
cm_load_cfmanager(
    int section,
    bool force_load
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct uci_package *package = NULL;
    int i = 0;
    int j = 0;
    int k = 0;

    package = cfparse_init_package( CFMANAGER_CONFIG_NAME );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load %s package failed\n", CFMANAGER_CONFIG_NAME );
        return -1;
    }

    if( CM_VLTREE_ALL == section ) {
        for( i = CM_VLTREE_FIRST; i < __CM_VLTREE_MAX; i++ ) {

            vltree = cm_vltree_info[i].vltree;
            if( !vltree ) {
                continue;
            }

            vlist_update( vltree );
        }
    }
    else {
        vltree = cm_vltree_info[section].vltree;
        if( !vltree ) {
            cfmanager_log_message( L_DEBUG, "No corresponding binary tree was found\n" );

            return -1;
        }

        vlist_update( vltree );
    }

    cm_force_load_flag = force_load;

    if( CM_VLTREE_ALL == section ) {
        /* Load into the tree in the order of enumeration.
         * Ensure that the tree has been loaded while handling certain dependencies.
         */
        for( k = CM_VLTREE_FIRST; k < __CM_VLTREE_MAX; k++ ) {

            if( !cm_vltree_info[k].key ) {
                continue;
            }

            cm_load_section( package, cm_vltree_info[k].key, k );
        }
    }
    else {
        cm_load_section( package, cm_vltree_info[section].key, section );
    }

    if( CM_VLTREE_ALL == section ) {
        for( i = CM_VLTREE_FIRST; i < __CM_VLTREE_MAX; i++ ) {

            vltree = cm_vltree_info[i].vltree;
            if( !vltree ) {
                continue;
            }

            vlist_flush( vltree );
        }
    }
    else {
        vltree = cm_vltree_info[section].vltree;

        vlist_flush( vltree );
    }

    for( j = CM_CFG_FIRST; j < __CM_CFG_MAX; j++ ) {
        if( cfg_option & BIT(j) ) {
            cfg_option &= ~BIT(j);
            cfmanager_log_message( L_DEBUG, "Commit package %s\n", cm_cfg_name[j].name );
            config_commit( cm_cfg_name[j].name, false );
            cfparse_load_file( cm_cfg_name[j].name, LOAD_ALL_SECTION, false );
        }
    }

    cm_force_load_flag = false;
    return 0;
}

//=============================================================================
void
cm_init(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __CM_VLTREE_MAX; i++ ) {

        vltree = cm_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_init( vltree, avl_strcmp, cm_vltree_update );
        vltree->keep_old = true;
        vltree->no_delete = true;
    }
}

//=============================================================================
void
cm_deinit(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __CM_VLTREE_MAX; i++ ) {

        vltree = cm_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush_all( vltree );
    }
}

