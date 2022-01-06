/****************************************************************************
* *
* * FILENAME:        $RCSfile: cfmanager.h,v $
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
#ifndef __CFMANAGER_H__
#define __CFMANAGER_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================
#define LOAD_ALL_SECTION        0

//=================
//  Typedefs
//=================
enum {
    CM_VLTREE_ALL,          //Update all binary trees
    CM_VLTREE_FIRST,
    CM_VLTREE_WAN = CM_VLTREE_FIRST,
    CM_VLTREE_LAN,
    CM_VLTREE_VLAN,
    CM_VLTREE_SWITCH_PORT,
    CM_VLTREE_UPGRADE_AUTO,
    CM_VLTREE_GUEST_SSID,
    CM_VLTREE_ACCESS,
    CM_VLTREE_GLOBAL_ACCESS,
    CM_VLTREE_SCHEDULE_ACCESS,
    CM_VLTREE_BASIC,
    CM_VLTREE_SYS_LOG,
    CM_VLTREE_EMAIL,
    CM_VLTREE_NOTIFICATION,
    CM_VLTREE_CONTROLLER,
    CM_VLTREE_AP,
    CM_VLTREE_WIRELESS,
    CM_VLTREE_GENERAL,
    CM_VLTREE_VPN_CLIENT,
    CM_VLTREE_VPN_SPLIT,
    CM_VLTREE_PORT_MAPPING,
    CM_VLTREE_DMZ,
    CM_VLTREE_UPNP,
    CM_VLTREE_DDNS,
    CM_VLTREE_STATIC_ROUTE_IPV4, 
    CM_VLTREE_MESH_SSID, 
    CM_VLTREE_CLIENT_LIMIT,
    CM_VLTREE_TR069,
    CM_VLTREE_USB_SHARE,
    CM_VLTREE_WAN_ALIAS,
    CM_VLTREE_STATIC_ROUTE_IPV6,
    CM_VLTREE_HOSTNAME,
    CM_VLTREE_POTAL_POLICY,
    CM_VLTREE_SNMP_CONFIG,
    CM_VLTREE_SNMP_PORTS,
    CM_VLTREE_SNMP_V3_AUTH,
    CM_VLTREE_VPN_SERVER, // Don't change the order of CM_VLTREE_VPN_SERVER, CM_VLTREE_IPSEC_CMN_SETTING and CM_VLTREE_IPSEC_DIAL_IN_USER.
    CM_VLTREE_IPSEC_CMN_SETTING,
    CM_VLTREE_IPSEC_DIAL_IN_USER,
    CM_VLTREE_ADDIT_SSID,
    CM_VLTREE_RADIO,
    CM_VLTREE_FIREWALL_DOS,
    CM_VLTREE_ACCELERATION,
    CM_VLTREE_SCHEDULE,

    __CM_VLTREE_MAX
};

/* If there is a dependency between profiles, keep their order here.
 * For instance:The configuration file of grandstream needs to be saved first.
 * If the country code is set, some of the places in effect use the
 * country code in grandstream, instead of using the country code in wireless,
 * but the effective execution is completed in wireless,so the modification of
 * grandstream needs to be saved first
 */
enum {
    CM_CFG_MIN,
    CM_CFG_FIRST,
    CM_CFG_GRANDSTREAM = CM_CFG_FIRST,
    CM_CFG_NETWORK,
    CM_CFG_DHCP,
    CM_CFG_FIREWALL,
    CM_CFG_WIRELESS,
    CM_CFG_SCHEDULE,
    CM_CFG_QOS,
    CM_CFG_BLACKHOLE,
    CM_CFG_SYSTEM,
    CM_CFG_EMAIL,
    CM_CFG_NOTIFY,
    CM_CFG_CONTROLLER,
    CM_CFG_CFMANAGER,
    CM_CFG_UPNPD,
    CM_CFG_DDNS,
    CM_CFG_BWCTRL,
    CM_CFG_SAMBA,
    CM_CFG_GSPORTAL,
    CM_CFG_TR069,
    CM_CFG_SNMP,
    CM_CFG_ACCELERATION,

    __CM_CFG_MAX
};

struct cm_config {
    struct vlist_node node;
    struct config_section_content cf_section;
};

//=================
//  Globals
//=================
extern const struct cm_vltree_info cm_vltree_info[__CM_VLTREE_MAX];
extern struct vlist_tree cm_wan_vltree;
extern struct vlist_tree cm_wan_advance_vltree;
extern struct vlist_tree cm_wireless_vltree;
extern struct vlist_tree cm_lan_vltree;
extern struct vlist_tree cm_vlan_vltree;
extern struct vlist_tree cm_switch_port_vltree;
extern struct vlist_tree cm_upgrade_auto_vltree;
extern struct vlist_tree cm_guest_ssid_vltree;
extern struct vlist_tree cm_access_vltree;
extern struct vlist_tree cm_global_access_vltree;
extern struct vlist_tree cm_schedule_access_vltree;
extern struct vlist_tree cm_basic_system_vltree;
extern struct vlist_tree cm_extern_log_vltree;
extern struct vlist_tree cm_CM_EMAIL_vltree;
extern struct vlist_tree cm_notification_vltree;
extern struct vlist_tree cm_controller_vltree;
extern struct vlist_tree cm_ap_vltree;
extern struct vlist_tree cm_general_vltree;
extern struct vlist_tree cm_vpn_client_vltree;
extern struct vlist_tree cm_vpn_split_vltree;
extern struct vlist_tree cm_port_mapping_vltree;
extern struct vlist_tree cm_dmz_vltree;
extern struct vlist_tree cm_upnp_vltree;
extern struct vlist_tree cm_ddns_vltree;
extern struct vlist_tree cm_static_route_ipv4_vltree; 
extern struct vlist_tree cm_mesh_ssid_vltree; 
extern struct vlist_tree cm_client_limit_vltree; 
extern struct vlist_tree cm_usb_share_vltree;
extern struct vlist_tree cm_firewall_dos_vltree;
extern struct vlist_tree cm_wan_alias_vltree;
extern struct vlist_tree cm_static_route_ipv6_vltree;
extern struct vlist_tree cm_portal_policy_vltree;
extern struct vlist_tree cm_tr069_vltree;
extern struct vlist_tree cm_hostname_vltree;
extern struct vlist_tree cm_snmp_config_vltree;
extern struct vlist_tree cm_snmp_ports_vltree;
extern struct vlist_tree cm_snmp_v3_auth_vltree;
extern struct vlist_tree cm_vpn_server_vltree;
extern struct vlist_tree cm_ipsec_cmn_setting_vltree;
extern struct vlist_tree cm_ipsec_dial_in_user_vltree;
extern struct vlist_tree cm_addit_ssid_vltree;
extern struct vlist_tree cm_radio_vltree;
extern struct vlist_tree cm_acceleration_vltree;
extern struct vlist_tree cm_schedule_vltree;

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
void
cm_init(
    void
);

void
cm_deinit(
    void
);

int
cm_load_cfmanager(
    int section,
    bool force_load
);


#endif //__CFMANAGER_H__
