/****************************************************************************
* *
* * FILENAME:        $RCSfile: config.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/26
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
#ifndef __SGREQUEST_H__
#define __SGREQUEST_H__
//=================
//  Includes
//=================
#include <libubox/vlist.h>
#include <uci.h>
#include "ubus.h"

//=================
//  Defines
//=================
#define LELEM(opt) ((uint64_t)1 << (opt))
#define LRANGES(first, last) (last - first + last)
#define LRANGE(lwb, upb) LRANGES(LELEM(lwb), LELEM(upb))

//=================
//  Typedefs
//=================
enum {
    WAN_TYPE,
    WAN_STATIC_DNSENABLE,
    WAN_FIRSTDNS,
    WAN_SECONDDNS,
    WAN_UPRATE,
    WAN_DOWNRATE,
    WAN_MTU,
    WAN_LINKSPEED,
    WAN_MACMODE,
    WAN_MAC,
    WAN_IP4ADDRESS,
    WAN_NETMASK,
    WAN_GATEWAY,
    WAN_PPPOEUSER,
    WAN_PPPOEPASSWORD,
    WAN_L2TPUSER,
    WAN_L2TPPASSWORD,
    WAN_L2TPSERVER,
    WAN_L2TPIP4ADDRESS,
    WAN_L2TPNETMASK,
    WAN_L2TPGATEWAY,
    WAN_L2TPIPTYPE,
    WAN_PPTPUSER,
    WAN_PPTPPASSWORD,
    WAN_PPTPSERVER,
    WAN_PPTPIP4ADDRESS,
    WAN_PPTPNETMASK,
    WAN_PPTPGATEWAY,
    WAN_PPTPIPTYPE,
    WAN_PPTPMPPE,
    WAN_ALIAS_ENABLE,
    WAN_ALIAS_IP,
#ifdef DOUBLE_WAN
    WAN1_ENABLE,
    WAN1_TYPE,
    WAN1_STATIC_DNSENABLE,
    WAN1_FIRSTDNS,
    WAN1_SECONDDNS,
    WAN1_UPRATE,
    WAN1_DOWNRATE,
    WAN1_MTU,
    WAN1_LINKSPEED,
    WAN1_MACMODE,
    WAN1_MAC,
    WAN1_IP4ADDRESS,
    WAN1_NETMASK,
    WAN1_GATEWAY,
    WAN1_PPPOEUSER,
    WAN1_PPPOEPASSWORD,
    WAN1_L2TPUSER,
    WAN1_L2TPPASSWORD,
    WAN1_L2TPSERVER,
    WAN1_L2TPIP4ADDRESS,
    WAN1_L2TPNETMASK,
    WAN1_L2TPGATEWAY,
    WAN1_L2TPIPTYPE,
    WAN1_PPTPUSER,
    WAN1_PPTPPASSWORD,
    WAN1_PPTPSERVER,
    WAN1_PPTPIP4ADDRESS,
    WAN1_PPTPNETMASK,
    WAN1_PPTPGATEWAY,
    WAN1_PPTPIPTYPE,
    WAN1_PPTPMPPE,
    WAN1_ALIAS_ENABLE,
    WAN1_ALIAS_IP,
#endif
    WAN_BALANCE,

    __WAN_MAX
};

enum {
    WANTYPE_DHCP,
    WANTYPE_STATIC,
    WANTYPE_PPPOE,
    WANTYPE_PPTP,
    WANTYPE_L2TP,

    __WANTYPE_MAX
};

enum {
    WANLINKSPEED_AUTO,
    WANLINKSPEED_100M,
    WANLINKSPEED_1000M,

    __WANLINKSPEED_MAX
};

enum {
    WANMAC_ROUTER,
    WANMAC_PC,
    WANMAC_CUSTOM,

    __WANMAC_MAX
};

enum
{
    WIRELESS_WIFIENABL,
    WIRELESS_MERGERADIOENABL,
    WIRELESS_2G4ENABLE,
    WIRELESS_2G4SSIDNAME,
    WIRELESS_2G4CRYPTO,
    WIRELESS_2G4PASSWORD,
    WIRELESS_2G4SSIDHIDEENABLE,
    WIRELESS_5GENABLE,
    WIRELESS_5GSSIDNAME,
    WIRELESS_5GCRYPTO,
    WIRELESS_5GPASSWORD,
    WIRELESS_5GSSIDHIDEENABLE,
    WIRELESS_UPRATE,
    WIRELESS_DOWNRATE,
    //Put the radio settings last
    WIRELESS_2G4CHANNEL,
    WIRELESS_2G4CHANNELWIDTH,
    WIRELESS_2G4TXPOWER,
    WIRELESS_5GCHANNEL,
    WIRELESS_5GCHANNELWIDTH,
    WIRELESS_5GTXPOWER,
    WIRELESS_MUMIMOENABLE,
#if defined(GWN7062)
    WIRELESS_COMPATIBILITYMODE,
#endif

    __WIRELESS_MAX
};

enum {
    CW_2G_HT20,
    CW_2G_HT20_HT40,
    CW_2G_HT40,

    __CW_2G_MAX
};

enum {
    CL_AUTO,
    CL_SECONDARY_BELOW_PRIMARY,
    CL_PRIMARY_BELOW_SECONDARY,

    __CL_MAX
};

enum {
    CW_5G_HT20,
    CW_5G_HT40,
    CW_5G_HT80,
    CW_5G_HT160,

    __CW_5G_MAX
};

enum {
    TXPOWER_LOW,
    TXPOWER_MEDIUM,
    TXPOWER_HIGH,
    TXPOWER_CUSTOM,
    TXPOWER_RRM,
    TXPOWER_AUTO,

    __TXPOWER_MAX
};

enum {
    CRYPTO_WPA_WPA2_PERSONAL = 2,   //Keep in line with ap
    CRYPTO_WPA2,
    CRYPTO_OPEN,
    CRYPTO_OSEN,
    CRYPTO_WPA2_WPA3_PERSONAL,
    CRYPTO_WPA3_PERSONAL,
    CRYPTO_WPA3_192,

    __CRYPTO_MAX
};

enum {
    WPA_KEY_MODE_PSK,
    WPA_KEY_MODE_802_1X,

    __WPA_KEY_MODE_MAX
};

enum {
    LAN_IP4ADDRESS,
    LAN_NETMASK,
    LAN_DHCP_ENABLE,
    LAN_DHCP_IPRANGE,
    LAN_DHCP_LEASETIME,
    LAN_DHCP_FIRSTDNS,
    LAN_DHCP_SECONDDNS,
    LAN_BINDIP,

    __LAN_MAX
};

enum {
    VLAN_ACTION,
    VLAN_ID,
    VLAN_NAME,
    VLAN_PRIORITY,
    VLAN_IP4ENABLE,
    VLAN_IP4ADDRESS,
    VLAN_NETMASK,
    VLAN_DHCP_ENABLE,
    VLAN_DHCP_IPRANGE,
    VLAN_DHCP_LEASETIME,
    VLAN_DHCP_FIRSTDNS,
    VLAN_DHCP_SECONDDNS,
    VLAN_IGMPSNOOPING_ENABLE,

    __VLAN_MAX
};

enum {
    VLAN_GET_TYPE,

    __VLAN_GET_MAX
};

enum {
    SWITCH_PORT_ATTR_PORTS,

    __SWITCH_PORT_ATTR_MAX
};

enum {
    SWITCH_PORT_PORT_ID,
    SWITCH_PORT_PVID,
    SWITCH_PORT_VLANS,

    __SWITCH_PORT_MAX
};

enum {
    UPGRADE_AUTO_ENABLE,
    UPGRADE_AUTO_SYNC_SLAVE_ENABLE,
    UPGRADE_AUTO_STARTTIME,
    UPGRADE_AUTO_ENDTIME,

    __UPGRADE_AUTO_MAX
};

enum {
    ROUTE_ATTR_NEXTHOP,

    __ROUTE_ATTR_MAX
};

enum {
    CONFIG_DEL,
    CONFIG_SET,
    CONFIG_EDIT,

    __CONFIG_ACTION_MAX
};

enum {
    BIND_IP_ACTION,
    BIND_IP_MAC,
    BIND_IP_IP,
    BIND_IP_VLAN,

    __BIND_IP_MAX
};

enum {
    BIND_IP_ACTION_DEL,
    BIND_IP_ACTION_ADD,

    __BIND_IP_ACTION_MAX
};

enum {
    GUEST_SSID_ENABLE,
    GUEST_SSID_SSIDNAME,
    GUEST_SSID_ENCRYPTO,
    GUEST_SSID_PASSWD,
    GUEST_SSID_ALLOW_MASTER,
    GUEST_SSID_UPRATESSID,
    GUEST_SSID_DOWNRATESSID,

    __GUEST_SSID_MAX
};

enum {
    GUEST_ENCRYPTO_OPEN,
    GUEST_ENCRYPTO_WAP2_WAP3,

    __GUEST_ENCRYPTO_MAX
};

enum {
    NETWORK_ATTR_UP,
    NETWORK_ATTR_PROTO,
    NETWORK_ATTR_IPV4_ADDRESS,
    NETWORK_ATTR_ROUTE,
    NETWORK_ATTR_USERNAME,
    NETWORK_ATTR_PASSWORD,
    NETWORK_ATTR_DNS_SERVER,

    __NETWORK_ATTR_MAX
};

enum {
    FIREWALL_DOS_PROTECT,
    FIREWALL_DOS_SYNFLOOD_PROTECT,
    FIREWALL_DOS_SYNFLOOD_RATE,
    //FIREWALL_DOS_SYNFLOOD_BURST,
    FIREWALL_DOS_UDPFLOOD_PROTECT,
    FIREWALL_DOS_UDPFLOOD_RATE,
    //FIREWALL_DOS_UDPFLOOD_BURST,
    FIREWALL_DOS_ICMPFLOOD_PROTECT,
    FIREWALL_DOS_ICMPFLOOD_RATE,
    //FIREWALL_DOS_ICMPFLOOD_BURST,
    FIREWALL_DOS_PINGOFDEATH_PROTECT,
    FIREWALL_DOS_PINGOFDEATH_LENGTH,

    __FIREWALL_DOS_MAX
};

enum {
    DEVAP,
    DEVMESH,

    __UPGRADE_DEVICES_MAX
};

enum {
    ACCESS_ATTR_CLIENT, 
    __ACCESS_ATTR_MAX
};

enum { 
    CM_PARSE_ACCESS_MAC,
    CM_PARSE_ACCESS_HOSTNAME,
    CM_PARSE_ACCESS_CLIENTOS,
    CM_PARSE_ACCESS_WHITE,
    CM_PARSE_ACCESS_BLACK,
    CM_PARSE_ACCESS_BLOCK,

    __CM_PARSE_ACCESS_MAX
};

enum {
    GLOBAL_ACCESS_ATTR_ENABLE,
    GLOBAL_ACCESS_ATTR_FORBIDURL,

    __GLOBAL_ACCESS_ATTR_MAX
};

enum {
    SCHEDULE_ACCESS_ATTR_ENABLE,
    SCHEDULE_ACCESS_ATTR_WEEK,
    SCHEDULE_ACCESS_ATTR_TIMESTART,
    SCHEDULE_ACCESS_ATTR_TIMEEND,

    __SCHEDULE_ACCESS_ATTR_MAX
};

enum {
    BASIC_COUNTRY_CODE,
    BASIC_TIME_ZONE,
    BASIC_LED_ON,

    __BASIC_SET_MAX
};

enum {
    CM_ISSUE_AP_LIST,
    CM_EXTERN_LOG_URI,
    CM_EXTERN_LOG_LEVEL,
    CM_EXTERN_LOG_PROTOCOL,

    _EXTERNAL_LOG_MAX
};

enum {
    CM_EMAIL_PORT,
    CM_EMAIL_HOST,
    CM_EMAIL_USER,
    CM_EMAIL_PASSWORD,
    CM_EMAIL_DO_NOT_VALIDATE,
    CM_EMAIL_ENABLE_NOTIFICATION,
    CM_EMAIL_FROM_ADDRESS,
    CM_EMAIL_FROM_NAME,
    CM_EMAIL_EMAILADDRESS,

    _CM_EMAIL_MAX
};

enum {
    CM_NOTIFY_MEMORY_USAGE,
    CM_MEMORY_USAGE_THRESHOLD,
    CM_NOTIFY_AP_THROUGHPUT,
    CM_AP_THROUGHPUT_THRESHOLD,
    CM_NOTIFY_SSID_THROUGHPUT,
    CM_SSID_THROUGHPUT_THRESHOLD,
    CM_NOTIFY_PASSWORD_CHANGE,
    CM_NOTIFY_FIRMWARE_UPGRADE,
    CM_NOTIFY_AP_OFFLINE,
    CM_NOTIFY_FIND_ROGUEAP,

    _CM_NOTIFY_MAX
};

enum {

    MESH_SSID_ATTR_ENABLE,
    MESH_SSID_ATTR_MODE,
    MESH_SSID_ATTR_SSID,
    MESH_SSID_ATTR_BSSID,
    MESH_SSID_ATTR_ENCRYPTION,
    MESH_SSID_ATTR_KEY,
    MESH_SSID_ATTR_ISADDED,
    MESH_SSID_ATTR_MASTER_ID,

    __MESH_SSID_ATTR_MAX
};

enum {
    STATUS,

    __PAIR_STATUS_MAX
};

// vpn server 
enum {
    VPN_SERVER_ATTR_SERVER_LIST, 

    __VPN_SERVER_ATTR_MAX
};

enum {
    VPN_SERVER_TYPE_IPSEC,
    VPN_SERVER_TYPE_OPENVPN,
    // Put other vpn server tyep at here.

    __VPN_SERVER_TYPE_MAX
};

enum {
    VPN_SERVER_ID,
    VPN_SERVER_ENABLE,
    VPN_SERVER_NAME,
    VPN_SERVER_TYPE,
    VPN_SERVER_WAN_INTF,
    VPN_SERVER_IPSEC_CMN_SETTING,
    VPN_SERVER_IPSEC_DIAL_IN_USER,
    // put other vpn server configs at here.
    VPN_SERVER_ACTION,

    // Note: If __VPN_SERVER_MAX >= 64, should adjust parameter vertify logic code.
    __VPN_SERVER_MAX,
};

enum {
    IPSEC_CMN_SETTING_ATTR_PSK,
    IPSEC_CMN_SETTING_ATTR_ENCRYPT_ALG,
    IPSEC_CMN_SETTING_ATTR_AUTH_ALG,
    IPSEC_CMN_SETTING_ATTR_DH,

    __IPSEC_CMN_SETTING_ATTR_MAX
};

enum {
    IPSEC_DIAL_IN_USER_ATTR_ID,
    IPSEC_DIAL_IN_USER_ATTR_TYPE,
    IPSEC_DIAL_IN_USER_ATTR_USER,
    IPSEC_DIAL_IN_USER_ATTR_PASSWORD,
    IPSEC_DIAL_IN_USER_ATTR_VLAN_ID,
    IPSEC_DIAL_IN_USER_ATTR_IP_RANGE,
    IPSEC_DIAL_IN_USER_ATTR_ACTION,

    __IPSEC_DIAL_IN_USER_ATTR_MAX
};

enum {
    IPSEC_DIAL_IN_USER_ACTION_ADD,
    IPSEC_DIAL_IN_USER_ACTION_DELETE,

    __IPSEC_DIAL_IN_USER_ACTION_MAX
};

enum {
    IPSEC_DIAL_IN_USER_TYPE_IKEV1,
    IPSEC_DIAL_IN_USER_TYPE_IKEV2,
    IPSEC_DIAL_IN_USER_TYPE_XAUTH,

    __IPSEC_DIAL_IN_USER_TYPE_MAX
};

enum {
    VPN_SERVER_ACTION_ADD,
    VPN_SERVER_ACTION_DELETE,

    __VPN_SERVER_ACTION_MAX
};

// vpn client
enum {
    VPN_CLIENT_ATTR_CLIENT_LIST, 

    __VPN_CLIENT_ATTR_MAX
};

enum {
    VPN_CLIENT_TYPE_L2TP,
    VPN_CLIENT_TYPE_PPTP,
    VPN_CLIENT_TYPE_IPSEC,

    __VPN_CLIENT_TYPE_MAX
};

enum {
    VPN_CLIENT_ID,
    VPN_CLIENT_ENABLE,
    VPN_CLIENT_NAME,
    VPN_CLIENT_TYPE,
    VPN_CLIENT_WAN_INTF,
    VPN_CLIENT_L2TP_SERVER,
    VPN_CLIENT_L2TP_USER,
    VPN_CLIENT_L2TP_PASSWORD,
    VPN_CLIENT_PPTP_SERVER,
    VPN_CLIENT_PPTP_USER,
    VPN_CLIENT_PPTP_PASSWORD,
    VPN_CLIENT_IPSEC_P1_REMOTE_GW,
    VPN_CLIENT_IPSEC_P1_IKE_VER,
    VPN_CLIENT_IPSEC_P1_IKE_LIFETIME,
    VPN_CLIENT_IPSEC_P1_NEGO_MODE,
    VPN_CLIENT_IPSEC_P1_AUTH_METHOD,
    VPN_CLIENT_IPSEC_P1_PSK,
    VPN_CLIENT_IPSEC_P1_ENCRYPT,
    VPN_CLIENT_IPSEC_P1_AUTH,
    VPN_CLIENT_IPSEC_P1_DH,
    VPN_CLIENT_IPSEC_P1_REKEY,
    VPN_CLIENT_IPSEC_P1_KEYINGTRIES,
    VPN_CLIENT_IPSEC_P1_DPD_EN,
    VPN_CLIENT_IPSEC_P1_DPD_DELAY,
    VPN_CLIENT_IPSEC_P1_DPD_IDLE,
    VPN_CLIENT_IPSEC_P1_DPD_ACTION,
    VPN_CLIENT_IPSEC_P2_LOCAL_SUBNET,
    VPN_CLIENT_IPSEC_P2_LOCAL_SOURCE_IP,
    VPN_CLIENT_IPSEC_P2_REMOTE_SUBNET_LIST,
    VPN_CLIENT_IPSEC_P2_SA_LIFETIME,
    VPN_CLIENT_IPSEC_P2_PROTO,
    VPN_CLIENT_IPSEC_P2_ESP_ENCRYPT,
    VPN_CLIENT_IPSEC_P2_ESP_AUTH,
    VPN_CLIENT_IPSEC_P2_ENCAP_MODE,
    VPN_CLIENT_IPSEC_P2_PFS_GROUP,
#define VPN_CLIENT_IPSEC_CFG_MASK LRANGE(VPN_CLIENT_IPSEC_P1_REMOTE_GW, VPN_CLIENT_IPSEC_P2_PFS_GROUP)

    // Note: If __VPN_CLIENT_MAX >= 64, should adjust parameter vertify logic code.
    __VPN_CLIENT_MAX
};

enum {
    VPN_CLIENT_ACTION_CONNECT,
    VPN_CLIENT_ACTION_CONCEL,
    VPN_CLIENT_ACTION_DISCONNECT,
    VPN_CLIENT_ACTION_DELETE,

    __VPN_CLIENT_ACTION_MAX
};

enum {
    VPN_CLIENT_CTRL_ID,
    VPN_CLIENT_CTRL_ACTION,

    __VPN_CLIENT_CTRL_MAX
};

enum {
    VPN_SPLIT_ATTR_SERVICE_LIST, 

    __VPN_SPLIT_ATTR_MAX
};

enum {
    VPN_ID_ON_WAN0 = WAN_CNT_MAX,
    VPN_ID_ON_WAN1,

    __VPN_ID_ON_WAN_MAX
};

enum {
    VPN_SPLIT_SERVICE_ID,
    VPN_SPLIT_MODE,
    VPN_SPLIT_SERVICE_ADDR_LIST,
    VPN_SPLIT_DEV_LIST,

    __VPN_SPLIT_MAX
};

enum {
    VPN_SPLIT_DEV_ATTR_NAME,
    VPN_SPLIT_DEV_ATTR_MAC,

    __VPN_SPLIT_DEV_ATTR_MAX
};

enum {
    PORT_MAPPING_ID,
    PORT_MAPPING_NAME,
    PORT_MAPPING_TYPE,
    PORT_MAPPING_EXT_PORT,
    PORT_MAPPING_INTER_PORT,
    PORT_MAPPING_INTER_IP,
    PORT_MAPPING_INTF,
    PORT_MAPPING_ACTION,
    PORT_MAPPING_EXT_IP,

    __PORT_MAPPING_MAX
};

enum {
    PORT_MAPPING_ATTR_RULE,

    __PORT_MAPPING_ATTR_MAX
};

enum {
    PORT_MAPPING_ACTION_ADD,
    PORT_MAPPING_ACTION_DELETE,

    __PORT_MAPPING_ACTION_MAX
};

enum {
    DMZ_ENABLE,
    DMZ_INTF,
    DMZ_DEV_IP,

    __DMZ_MAX
};

enum {
    UPNP_ENABLE,
    UPNP_INTF,
    UPNP_MAPPING_TABLE,

    __UPNP_MAX
};

enum {
    UPNP_MAPPING_ATTR_DESCRIPTION,
    UPNP_MAPPING_ATTR_IP,
    UPNP_MAPPING_ATTR_EXT_PORT,
    UPNP_MAPPING_ATTR_INTER_PORT,
    UPNP_MAPPING_ATTR_PROTO,

    __UPNP_MAPPING_ATTR_MAX
};

enum {
    DDNS_ATTR_LIST,

    __DDNS_ATTR_MAX
};

enum {
    DDNS_ID,
    DDNS_SERVICE_PROVIDER,
    DDNS_USERNAME,
    DDNS_PASSWORD,
    DDNS_HOSTNAME,
    DDNS_INTF,
    DDNS_STATUS,
    DDNS_ACTION,

    __DDNS_MAX
};

enum {
    DDNS_SERVICE_PROVIDER_ORAY,
    DDNS_SERVICE_PROVIDER_DYNDNS,
    DDNS_SERVICE_PROVIDER_NO_IP,

    __DDNS_SERVICE_PROVIDER_MAX
};

enum {
    DDNS_ACTION_ADD,
    DDNS_ACTION_ENABLE,
    DDNS_ACTION_DISABLE,
    DDNS_ACTION_DELETE,

    __DDNS_ACTION_MAX
};

enum {
    STATIC_ROUTE_IPV4_ATTR_RULE, 

    __STATIC_ROUTE_IPV4_ATTR_MAX
};

enum {
    STATIC_ROUTE_IPV4_ID,
    STATIC_ROUTE_IPV4_NAME,
    STATIC_ROUTE_IPV4_IP,
    STATIC_ROUTE_IPV4_NETMASK,
    STATIC_ROUTE_IPV4_OUT_INTF,
    STATIC_ROUTE_IPV4_NEXTHOP,
    STATIC_ROUTE_IPV4_METRIC,
    STATIC_ROUTE_IPV4_ENABLE,
    STATIC_ROUTE_IPV4_ACTION,

    __STATIC_ROUTE_IPV4_MAX
};

enum {
    STATIC_ROUTE_IPV4_ACTION_ADD,
    STATIC_ROUTE_IPV4_ACTION_DELETE,

    __STATIC_ROUTE_IPV4_ACTION_MAX
};

enum {
    CLIENT_LIMIT_ENABLE,
    CLIENT_LIMIT_MAC,
    CLIENT_LIMIT_UPRAT,
    CLIENT_LIMIT_DOWNRATE,
    CLIENT_LIMIT_SSID_NAME,

    __CLIENT_LIMIT_MAX
};

enum {
    AP_NAME_NO_CHANGE,
    AP_NAME_CAHNGED
};

enum {
    EX_UNAUTHENTICATED_FROM,

    __EX_UNAUTHENTICATED_MAX,
};

enum {
    CM_TR069_ENABLE,
    CM_TR069_ACS_URL,
    CM_TR069_ACS_NAME,
    CM_TR069_ACS_PASSWORD,
    CM_TR069_PERIODIC_INFORM_ENABLE,
    CM_TR069_PERIODIC_INFORM_INTERVAL,
    CM_TR069_CPE_CERT_FILE,
    CM_TR069_CPE_CERT_KEY,
    CM_TR069_CONN_REQ_NAME,
    CM_TR069_CONN_REQ_PASSWORD,
    CM_TR069_CONN_REQ_PORT,

    __CM_TR069_MAX
};

enum {
    USB_SHARE_ENABLE,
    USB_SHARE_ANONYMITY,
    USB_SHARE_USER,
    USB_SHARE_PASSWORD,

    __USB_SHARE_MAX
};

enum {
    WAN_ALIAS_ATTR_ENABLE,
    WAN_ALIAS_ATTR_IP,

    __WAN_ALIAS_ATTR_MAX
};

enum {
    STATIC_ROUTE_IPV6_ATTR_RULE, 

    __STATIC_ROUTE_IPV6_ATTR_MAX
};

enum {
    STATIC_ROUTE_IPV6_ENABLE,
    STATIC_ROUTE_IPV6_OUT_INTF,
    STATIC_ROUTE_IPV6_NAME,
    STATIC_ROUTE_IPV6_PREFIX_LENGTH,
    STATIC_ROUTE_IPV6_TARGET,
    STATIC_ROUTE_IPV6_NEXTHOP,
    STATIC_ROUTE_IPV6_METRIC,
    STATIC_ROUTE_IPV6_ACTION,
    STATIC_ROUTE_IPV6_ID,

    __STATIC_ROUTE_IPV6_MAX
};

enum {
    HOSTNAME_NAME,
    HOSTNAME_MAC,

    __HOSTNAME_MAX
};

enum {
    SNMP_CONFIG,
    SNMP_PORTS,
    SNMP_V3_AUTH,
    __SNMP_MAX
};

enum {
    ACCELERATION_ENABLE,
    ACCELERATION_ENGINE,

    __ACCELERATION_MAX
};

enum {
    CM_ADDIT_SSID_ID,
    CM_ADDIT_SSID_ACTION,
    CM_ADDIT_SSID_ENABLE,
    CM_ADDIT_SSID_BAND,
    CM_ADDIT_SSID_NAME,
    CM_ADDIT_SSID_CRYPTO,
    CM_ADDIT_SSID_WPA_KEY_MODE,
    CM_ADDIT_SSID_WPA_CRYPTO_TYPE,
    CM_ADDIT_SSID_PASSWORD,
    CM_ADDIT_SSID_SSIDHIDEENABLE,
    CM_ADDIT_SSID_ISOLATEMODE,
    CM_ADDIT_SSID_GATEWAYMAC,
    CM_ADDIT_SSID_DTIM_PERIOD,
    CM_ADDIT_SSID_CLIENT_LIMIT,
    CM_ADDIT_SSID_STA_IDLE_TIMEOUT,
    CM_ADDIT_SSID_UAPSD,
    CM_ADDIT_SSID_PROXY_ARP,
    CM_ADDIT_SSID_MCAST_TO_UCAST,
    CM_ADDIT_SSID_BMS,
    CM_ADDIT_SSID_BRIDGE_ENABLE,
    CM_ADDIT_SSID_80211W,
    CM_ADDIT_SSID_PORTAL_ENABLE,
    CM_ADDIT_SSID_PORTAL_POLICY,
    CM_ADDIT_SSID_UPRATE,
    CM_ADDIT_SSID_DOWNRATE,
    CM_ADDIT_SSID_SCHEDULE_ENABLE,
    CM_ADDIT_SSID_SCHEDULE_ID,
    CM_ADDIT_SSID_VLAN_ENABLE,
    CM_ADDIT_SSID_VLAN_ID,
    CM_ADDIT_SSID_MEMBER_DEV_MAC,
    CM_ADDIT_SSID_AVAILABLE_DEV_MAC,

    __CM_ADDIT_SSID_MAX
};

enum {
    CM_RADIO_2G4CHANNEL,
    CM_RADIO_2G4CHANNELWIDTH,
    CM_RADIO_2G4CHANNEL_LOCATION,
    CM_RADIO_2G4TXPOWER,
    CM_RADIO_2G4CUSTOM_TXPOWER,
    CM_RADIO_2G4SHORTGI,
    CM_RADIO_2G4ALLOW_LEGACY_DEV,
    CM_RADIO_2G4RSSI_ENABLE,
    CM_RADIO_2G4RSSI_THRESHOLD,
    CM_RADIO_2G4RATE_LIMIT_ENABLE,
    CM_RADIO_2G4MINI_RATE,
    CM_RADIO_5GCHANNEL,
    CM_RADIO_5GCHANNELWIDTH,
    CM_RADIO_5GTXPOWER,
    CM_RADIO_5GCUSTOM_TXPOWER,
    CM_RADIO_5GSHORTGI,
    CM_RADIO_5GRSSI_ENABLE,
    CM_RADIO_5GRSSI_THRESHOLD,
    CM_RADIO_5GRATE_LIMIT_ENABLE,
    CM_RADIO_5GMINI_RATE,
    CM_RADIO_BEACON_INTERVAL,
    CM_RADIO_ATF_MODE,
    CM_RADIO_MUMIMOENABLE,
#if defined(GWN7062)
    CM_RADIO_COMPATIBILITYMODE,
#endif
    __CM_RADIO_MAX
};

enum {
    ISOLATE_MODE_CLOSE,
    ISOLATE_MODE_RADIO,
    ISOLATE_MODE_INTERNET,
    ISOLATE_MODE_GATEWAY_MAC,

    __ISOLATE_MODE_MAX
};

enum {
    DEV_SSID_ID,
    DEV_SSID_AVAILABLE_DEV,
    DEV_SSID_MEMBER_DEV,

    __DEV_SSID_MAX
};

enum {
    CM_SCHEDULE_ACTION,
    CM_SCHEDULE_ID,
    CM_SCHEDULE_NAME,
    CM_SCHEDULE_WEEKLY_TIME1,
    CM_SCHEDULE_WEEKLY_TIME2,
    CM_SCHEDULE_WEEKLY_TIME3,
    CM_SCHEDULE_WEEKLY_TIME4,
    CM_SCHEDULE_WEEKLY_TIME5,
    CM_SCHEDULE_WEEKLY_TIME6,
    CM_SCHEDULE_WEEKLY_TIME7,
    CM_SCHEDULE_ABTIME_LISTS,

    __CM_SCHEDULE_MAX
};

enum {
    CM_SCHEDULE_ABTIME_ABDATE,
    CM_SCHEDULE_ABTIME_ABTIME,

    __CM_SCHEDULE_ABTIME_MAX
};

enum {
    CM_SCHEDULE_GET_TYPE,

    __CM_SCHEDULE_GET_MAX
};

enum {
    GET_AP_INFO_MAC,

    __GET_AP_INFO_MAX
};

enum {
    CM_DEV_ADD_SSID_MAC,
    CM_DEV_ADD_SSID_SSIDS,

    __CM_DEV_ADD_SSID_MAX
};

enum {
    CM_AP_MAC,
    CM_AP_TYPE,
    CM_AP_MESH,
    CM_AP_FREQUENCY,
    CM_AP_BAND_STEERING,
    CM_AP_ZONES,
    CM_AP_SSIDS,
    CM_AP_TLS1_2,
    CM_AP_NAME,
    CM_AP_POSITION,
    CM_AP_POSITION_CUSTOM,
    CM_AP_SUPERIOR,
    CM_AP_NETPORT_TYPE,
    CM_AP_IPV4_ENABLE,
    CM_AP_IPV4_IP,
    CM_AP_IPV4_NETMASK,
    CM_AP_IPV4_GATEWAY,
    CM_AP_IPV4_FIRST_DNS,
    CM_AP_IPV4_SECOND_DNS,
    CM_AP_IPV6_ENABLE,
    CM_AP_IPV6_IP,
    CM_AP_IPV6_PREFIX_LENGTH,
    CM_AP_IPV6_GATEWAY,
    CM_AP_IPV6_FIRST_DNS,
    CM_AP_IPV6_SECOND_DNS,
    CM_AP_2G4_DISABLE,
    CM_AP_2G4_WIDTH,
    CM_AP_2G4_CHANNEL_LOCATION,
    CM_AP_2G4_CHANNEL,
    CM_AP_2G4_POWER,
    CM_AP_2G4_CUSTOM_TXPOWER,
    CM_AP_2G4_RSSI_TYPE,
    CM_AP_2G4_RSSI_THRESHOLD,
    CM_AP_2G4_MODE,
    CM_AP_2G4_RATE_LIMIT_TYPE,
    CM_AP_2G4_MINI_RATE,
    CM_AP_2G4_SHORTGI,
    CM_AP_2G4_ALLOW_LEGACY_DEV,
    CM_AP_5G_DISABLE,
    CM_AP_5G_WIDTH,
    CM_AP_5G_CHANNEL,
    CM_AP_5G_POWER,
    CM_AP_5G_CUSTOM_TXPOWER,
    CM_AP_5G_RSSI_TYPE,
    CM_AP_5G_RSSI_THRESHOLD,
    CM_AP_5G_RATE_LIMIT_TYPE,
    CM_AP_5G_MINI_RATE,
    CM_AP_5G_SHORTGI,
    CM_AP_APSMAC,

    __CM_AP_MAX
};

enum {
    CM_GET_PAIRED_DEVICES_DEVTYPE,
    CM_GET_PAIRED_DEVICES_START,
    CM_GET_PAIRED_DEVICES_AMOUNT,
    CM_GET_PAIRED_DEVICES_FILTER,
    CM_GET_PAIRED_DEVICES_DEVTYPE_FILTER,

    __CM_GET_PAIRED_DEVICES_MAX
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

/*****************ctrl cmd***************************/
void
sgreq_ctrl_vpn_client(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

/*****************set cmd***************************/
void
sgreq_set_upgrade_auto(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_wan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_lan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_vlan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_switch_ports(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_wireless(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_tr069(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_additional_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_radio(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_firewall_dos(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_access(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_global_access(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_schedule_access(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_guest_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_mesh_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_acl(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_qos(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_usb_share(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_static_router(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_controller(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_general(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_extern_sys_log(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_email(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_notification(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void 
sgreq_set_basic(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void 
sgreq_set_vpn_client(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_vpn_split_tunneling(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_port_mapping(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_dmz(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_upnp(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_ddns(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_ipv4_static_route(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_manage_ap_mesh(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_ap(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_client_limit(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_ipv6_static_route(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_hostname(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_portal_policy(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_snmp(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);


void
sgreq_set_vpn_server(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_acceleration(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_schedule(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_ssid_wizard(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_dev_add_ssid(
struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_set_ap_batch(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

/*****************get cmd***************************/
void
sgreq_get_all(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_wan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_lan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_vlan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_switch_ports(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_wireless(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_firewall_dos(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_wan_ip(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_client(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_device(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_guest_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_mesh_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_acl(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_qos(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);


void
sgreq_get_network_interface(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_controller(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_access(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_upgrade_auto(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_basic(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_extern_sys_log(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_email(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_notification(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_ap(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_client_limit(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_overview(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_vpn_client(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_aps_simple_info(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_ssid_wizard(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_vpn_split_tunneling(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_port_mapping(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_dmz(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_upnp(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_ddns(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_ipv4_static_route(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_tr069(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_additional_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_radio(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_channel(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_portal_policy(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_snmp(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_acceleration(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_schedule(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);



/*****************ext cmd***************************/
void
sgreq_get_paired_devices(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_ext_unauthenticated_info(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_usb_share(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_vpn_server(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_ipv6_static_route(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

void
sgreq_get_ap_status(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
);

/*****************func***************************/
int
sgreq_convert_cm_proto(
    const char *proto_str
);

void
sgreq_convert_sg_proto(
    int proto,
    char *proto_str,
    int size
);

void
sgreq_convert_sg_txpower(
    int sg_txpower,
    char *txpower,
    int size
);

int
sgreq_convert_cm_link_speed(
    const char *link_speed
);

bool
sgreq_compar_attr(
    struct blob_attr *a1,
    struct blob_attr *a2,
    int policy_type
);

#endif //__SGREQUEST_H__
