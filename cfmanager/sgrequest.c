/****************************************************************************
* *
* * FILENAME:        $RCSfile: config.c,v $
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
//=================
//  Includes
//=================
#define _GNU_SOURCE
#include <sys/sysinfo.h>
#include "global.h"
#include "utils.h"
#include "cfparse.h"
#include "config.h"
#include "wireless.h"
#include "sgrequest.h"
#include "ubus.h"
#include "network.h"
#include "dhcp.h"
#include "schedule.h"
#include "cfmanager.h"
#include "firewall.h"
#include "mwan3.h"
#include "time.h"
#include "grandstream.h"
#include "controller.h"
#include "vpn.h"
#include "upnpd.h"
#include "track.h"
#include "time.h"
#include "apply.h"
#include "route.h"
#include "system.h"
#include "ddns.h"
#include "check.h"
#include "smb.h"
#include "usb.h"
#include "gsportalcfg.h"

#include "gs_utils.h"

//=================
//  Defines
//=================
#define DEVICE_DOMAIN "myrouter.grandstream.com"

//=================
//  Typedefs
//=================
enum {
    RADIO_TYPE_HOME,
    RADIO_TYPE_ENTERPRISE,

    __RADIO_TYPE_MAX
};

enum {
    DEV_AVAILABLE,
    DEV_MEMBER,

    __DEV_MAX
};
//=================
//  Globals
//=================

//=================
//  Locals
//=================
const struct blobmsg_policy wan_policy[__WAN_MAX] = {
    [WAN_TYPE] = { .name = "wanType", .type = BLOBMSG_TYPE_STRING },
    [WAN_STATIC_DNSENABLE] = { .name = "wanStaticDnsEnable", .type = BLOBMSG_TYPE_BOOL },
    [WAN_FIRSTDNS] = { .name = "wanFirstDns", .type = BLOBMSG_TYPE_STRING },
    [WAN_SECONDDNS] = { .name = "wanSecondDns", .type = BLOBMSG_TYPE_STRING },
    [WAN_UPRATE] = { .name = "wanUpRate", .type = BLOBMSG_TYPE_STRING },
    [WAN_DOWNRATE] = { .name = "wanDownRate", .type = BLOBMSG_TYPE_STRING },
    [WAN_MTU] = { .name = "wanMtu", .type = BLOBMSG_TYPE_STRING },
    [WAN_LINKSPEED] = { .name = "wanLinkSpeed", .type = BLOBMSG_TYPE_STRING },
    [WAN_MACMODE] = { .name = "wanMacMode", .type = BLOBMSG_TYPE_STRING },
    [WAN_MAC] = { .name = "wanMac", .type = BLOBMSG_TYPE_STRING },
    [WAN_IP4ADDRESS] = { .name = "wanIp4Address", .type = BLOBMSG_TYPE_STRING },
    [WAN_NETMASK] = { .name = "wanNetmask", .type = BLOBMSG_TYPE_STRING },
    [WAN_GATEWAY] = { .name = "wanGateway", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPPOEUSER] = { .name = "wanPppoeUser", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPPOEPASSWORD] = { .name = "wanPppoePassword", .type = BLOBMSG_TYPE_STRING },
    [WAN_ALIAS_ENABLE] = { .name = "wanAliasEnable", .type = BLOBMSG_TYPE_BOOL },
    [WAN_ALIAS_IP] = { .name = "wanAliasIp4Address", .type = BLOBMSG_TYPE_ARRAY },
    [WAN_L2TPUSER] = { .name = "wanL2tpUser", .type = BLOBMSG_TYPE_STRING },
    [WAN_L2TPPASSWORD] = { .name = "wanL2tpPassword", .type = BLOBMSG_TYPE_STRING },
    [WAN_L2TPSERVER] = { .name = "wanL2tpServer", .type = BLOBMSG_TYPE_STRING },
    [WAN_L2TPIP4ADDRESS] = { .name = "wanL2tpIp4Address", .type = BLOBMSG_TYPE_STRING },
    [WAN_L2TPNETMASK] = { .name = "wanL2tpNetmask", .type = BLOBMSG_TYPE_STRING },
    [WAN_L2TPGATEWAY] = { .name = "wanL2tpGateway", .type = BLOBMSG_TYPE_STRING },
    [WAN_L2TPIPTYPE] = { .name = "wanL2tpIpType", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPTPUSER] = { .name = "wanPptpUser", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPTPPASSWORD] = { .name = "wanPptpPassword", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPTPSERVER] = { .name = "wanPptpServer", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPTPIP4ADDRESS] = { .name = "wanPptpIp4Address", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPTPNETMASK] = { .name = "wanPptpNetmask", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPTPGATEWAY] = { .name = "wanPptpGateway", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPTPIPTYPE] = { .name = "wanPptpIpType", .type = BLOBMSG_TYPE_STRING },
    [WAN_PPTPMPPE] = { .name = "wanPptpMppe", .type = BLOBMSG_TYPE_BOOL },
#ifdef DOUBLE_WAN
    [WAN1_ENABLE] = { .name = "wan1Enable", .type = BLOBMSG_TYPE_BOOL },
    [WAN1_TYPE] = { .name = "wan1Type", .type = BLOBMSG_TYPE_STRING },
    [WAN1_STATIC_DNSENABLE] = { .name = "wan1StaticDnsEnable", .type = BLOBMSG_TYPE_BOOL },
    [WAN1_FIRSTDNS] = { .name = "wan1FirstDns", .type = BLOBMSG_TYPE_STRING },
    [WAN1_SECONDDNS] = { .name = "wan1SecondDns", .type = BLOBMSG_TYPE_STRING },
    [WAN1_UPRATE] = { .name = "wan1UpRate", .type = BLOBMSG_TYPE_STRING },
    [WAN1_DOWNRATE] = { .name = "wan1DownRate", .type = BLOBMSG_TYPE_STRING },
    [WAN1_MTU] = { .name = "wan1Mtu", .type = BLOBMSG_TYPE_STRING },
    [WAN1_LINKSPEED] = { .name = "wan1LinkSpeed", .type = BLOBMSG_TYPE_STRING },
    [WAN1_MACMODE] = { .name = "wan1MacMode", .type = BLOBMSG_TYPE_STRING },
    [WAN1_MAC] = { .name = "wan1Mac", .type = BLOBMSG_TYPE_STRING },
    [WAN1_IP4ADDRESS] = { .name = "wan1Ip4Address", .type = BLOBMSG_TYPE_STRING },
    [WAN1_NETMASK] = { .name = "wan1Netmask", .type = BLOBMSG_TYPE_STRING },
    [WAN1_GATEWAY] = { .name = "wan1Gateway", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPPOEUSER] = { .name = "wan1PppoeUser", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPPOEPASSWORD] = { .name = "wan1PppoePassword", .type = BLOBMSG_TYPE_STRING },
    [WAN1_L2TPUSER] = { .name = "wan1L2tpUser", .type = BLOBMSG_TYPE_STRING },
    [WAN1_L2TPPASSWORD] = { .name = "wan1L2tpPassword", .type = BLOBMSG_TYPE_STRING },
    [WAN1_L2TPSERVER] = { .name = "wan1L2tpServer", .type = BLOBMSG_TYPE_STRING },
    [WAN1_L2TPIP4ADDRESS] = { .name = "wan1L2tpIp4Address", .type = BLOBMSG_TYPE_STRING },
    [WAN1_L2TPNETMASK] = { .name = "wan1L2tpNetmask", .type = BLOBMSG_TYPE_STRING },
    [WAN1_L2TPGATEWAY] = { .name = "wan1L2tpGateway", .type = BLOBMSG_TYPE_STRING },
    [WAN1_L2TPIPTYPE] = { .name = "wan1L2tpIpType", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPTPUSER] = { .name = "wan1PptpUser", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPTPPASSWORD] = { .name = "wan1PptpPassword", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPTPSERVER] = { .name = "wan1PptpServer", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPTPIP4ADDRESS] = { .name = "wan1PptpIp4Address", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPTPNETMASK] = { .name = "wan1PptpNetmask", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPTPGATEWAY] = { .name = "wan1PptpGateway", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPTPIPTYPE] = { .name = "wan1PptpIpType", .type = BLOBMSG_TYPE_STRING },
    [WAN1_PPTPMPPE] = { .name = "wan1PptpMppe", .type = BLOBMSG_TYPE_BOOL },
    [WAN1_ALIAS_ENABLE] = { .name = "wan1AliasEnable", .type = BLOBMSG_TYPE_BOOL },
    [WAN1_ALIAS_IP] = { .name = "wan1AliasIp4Address", .type = BLOBMSG_TYPE_ARRAY },
#endif
    [WAN_BALANCE] = { .name = "wanBalance", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy wireless_policy[__WIRELESS_MAX] = {
    [WIRELESS_WIFIENABL] = { .name = "wifiEnable", .type = BLOBMSG_TYPE_BOOL },
    [WIRELESS_MERGERADIOENABL] = { .name = "mergeRadioEnable", .type = BLOBMSG_TYPE_BOOL },
    [WIRELESS_MUMIMOENABLE] = { .name = "mumimoEnable", .type = BLOBMSG_TYPE_BOOL },
#if defined(GWN7062)
    [WIRELESS_COMPATIBILITYMODE] = { .name = "compatibilityMode", .type = BLOBMSG_TYPE_BOOL },
#endif
    [WIRELESS_2G4ENABLE] = { .name = "2g4Enable", .type = BLOBMSG_TYPE_BOOL },
    [WIRELESS_2G4SSIDNAME] = { .name = "2g4SsidName", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_2G4CRYPTO] = { .name = "2g4Crypto", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_2G4PASSWORD] = { .name = "2g4Password", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_2G4SSIDHIDEENABLE] = { .name = "2g4SsidHideEnable", .type = BLOBMSG_TYPE_BOOL },
    [WIRELESS_5GENABLE] = { .name = "5gEnable", .type = BLOBMSG_TYPE_BOOL },
    [WIRELESS_5GSSIDNAME] = { .name = "5gSsidName", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_5GCRYPTO] = { .name = "5gCrypto", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_5GPASSWORD] = { .name = "5gPassword", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_5GSSIDHIDEENABLE] = { .name = "5gSsidHideEnable", .type = BLOBMSG_TYPE_BOOL },
    [WIRELESS_UPRATE] = { .name = "upRate", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DOWNRATE] = { .name = "downRate", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_2G4CHANNEL] = { .name = "2g4Channel", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_2G4CHANNELWIDTH] = { .name = "2g4ChannelWidth", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_2G4TXPOWER] = { .name = "2g4TxPower", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_5GCHANNEL] = { .name = "5gChannel", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_5GCHANNELWIDTH] = { .name = "5gChannelWidth", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_5GTXPOWER] = { .name = "5gTxPower", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_UPRATE] = { .name = "upRate", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DOWNRATE] = { .name = "downRate", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy cm_radio_policy[__CM_RADIO_MAX] = {
    [CM_RADIO_2G4CHANNEL] = { .name = "2g4Channel", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_2G4CHANNELWIDTH] = { .name = "2g4ChannelWidth", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_2G4CHANNEL_LOCATION] = { .name = "2g4ChannelLocation", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_2G4TXPOWER] = { .name = "2g4TxPower", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_2G4CUSTOM_TXPOWER] = { .name = "2g4CustomTxPower", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_2G4SHORTGI] = { .name = "2g4ShortGuardInterval", .type = BLOBMSG_TYPE_BOOL },
    [CM_RADIO_2G4ALLOW_LEGACY_DEV] = { .name = "2g4AllowLegacyDev", .type = BLOBMSG_TYPE_BOOL },
    [CM_RADIO_2G4RSSI_ENABLE] = { .name = "2g4RssiEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_RADIO_2G4RSSI_THRESHOLD] = { .name = "2g4RssiThreshold", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_2G4RATE_LIMIT_ENABLE] = { .name = "2g4RateLimitEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_RADIO_2G4MINI_RATE] = { .name = "2g4MiniRate", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_5GCHANNEL] = { .name = "5gChannel", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_5GCHANNELWIDTH] = { .name = "5gChannelWidth", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_5GTXPOWER] = { .name = "5gTxPower", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_5GCUSTOM_TXPOWER] = { .name = "5gCustomTxPower", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_5GSHORTGI] = { .name = "5gShortGuardInterval", .type = BLOBMSG_TYPE_BOOL },
    [CM_RADIO_5GRSSI_ENABLE] = { .name = "5gRssiEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_RADIO_5GRSSI_THRESHOLD] = { .name = "5gRssiThreshold", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_5GRATE_LIMIT_ENABLE] = { .name = "5gRateLimitEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_RADIO_5GMINI_RATE] = { .name = "5gMiniRate", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_BEACON_INTERVAL] = { .name = "beaconInterval", .type = BLOBMSG_TYPE_STRING },
    [CM_RADIO_ATF_MODE] = { .name = "airtimeFairness", .type = BLOBMSG_TYPE_BOOL },
    [CM_RADIO_MUMIMOENABLE] = { .name = "mumimoEnable", .type = BLOBMSG_TYPE_BOOL },
#if defined(GWN7062)
    [CM_RADIO_COMPATIBILITYMODE] = { .name = "compatibilityMode", .type = BLOBMSG_TYPE_BOOL },
#endif
};

const struct blobmsg_policy cm_addit_ssid_policy[__CM_ADDIT_SSID_MAX] = {
    [CM_ADDIT_SSID_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [CM_ADDIT_SSID_BAND] = { .name = "ssidBand", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_NAME] = { .name = "ssidName", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_CRYPTO] = { .name = "crypto", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_WPA_KEY_MODE] = { .name = "wpaKeyMode", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_WPA_CRYPTO_TYPE] = { .name = "wpaCryptoType", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_SSIDHIDEENABLE] = { .name = "ssidHideEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_ADDIT_SSID_ISOLATEMODE] = { .name = "isolateMode", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_GATEWAYMAC] = { .name = "gatewayMac", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_DTIM_PERIOD] = { .name = "dtimPeriod", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_CLIENT_LIMIT] = { .name = "clientLimit", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_STA_IDLE_TIMEOUT] = { .name = "staIdleTimeout", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_UAPSD] = { .name = "uapsdEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_ADDIT_SSID_PROXY_ARP] = { .name = "proxyArpEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_ADDIT_SSID_MCAST_TO_UCAST] = { .name = "mcastToUcast", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_BMS] = { .name = "bms", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_BRIDGE_ENABLE] = { .name = "bridgeEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_ADDIT_SSID_80211W] = { .name = "80211w", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_VLAN] = { .name = "vlan", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_PORTAL_ENABLE] = { .name = "portalEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_ADDIT_SSID_PORTAL_POLICY] = { .name = "portalPolicy", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_UPRATE] = { .name = "upRateLimit", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_DOWNRATE] = { .name = "downRateLimit", .type = BLOBMSG_TYPE_STRING },
    [CM_ADDIT_SSID_SCHEDULE_ENABLE] = { .name = "scheduleEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_ADDIT_SSID_SCHEDULE_ID] = { .name = "scheduleId", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy lan_policy[__LAN_MAX] = {
    [LAN_IP4ADDRESS] = { .name = "lanIp4Address", .type = BLOBMSG_TYPE_STRING },
    [LAN_NETMASK] = { .name = "lanNetmask", .type = BLOBMSG_TYPE_STRING },
    [LAN_DHCP_ENABLE] = { .name = "dhcpEnable", .type = BLOBMSG_TYPE_BOOL },
    [LAN_DHCP_IPRANGE] = { .name = "dhcpIpRange", .type = BLOBMSG_TYPE_STRING },
    [LAN_DHCP_LEASETIME] = { .name = "dhcpLeaseTime", .type = BLOBMSG_TYPE_STRING },
    [LAN_DHCP_FIRSTDNS] = { .name = "dhcpFirstDns", .type = BLOBMSG_TYPE_STRING },
    [LAN_DHCP_SECONDDNS] = { .name = "dhcpSecondDns", .type = BLOBMSG_TYPE_STRING },
    [LAN_BINDIP] = { .name = "bindIp", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy vlan_policy[__VLAN_MAX] = {
    [VLAN_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
    [VLAN_ID] = { .name = "vlanId", .type = BLOBMSG_TYPE_STRING },
    [VLAN_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [VLAN_PRIORITY] = { .name = "priority", .type = BLOBMSG_TYPE_STRING },
    [VLAN_IP4ENABLE] = { .name = "ipv4Enable", .type = BLOBMSG_TYPE_BOOL },
    [VLAN_IP4ADDRESS] = { .name = "lanIp4Address", .type = BLOBMSG_TYPE_STRING },
    [VLAN_NETMASK] = { .name = "lanNetmask", .type = BLOBMSG_TYPE_STRING },
    [VLAN_DHCP_ENABLE] = { .name = "dhcpEnable", .type = BLOBMSG_TYPE_BOOL },
    [VLAN_DHCP_IPRANGE] = { .name = "dhcpIpRange", .type = BLOBMSG_TYPE_STRING },
    [VLAN_DHCP_LEASETIME] = { .name = "dhcpLeaseTime", .type = BLOBMSG_TYPE_STRING },
    [VLAN_DHCP_FIRSTDNS] = { .name = "dhcpFirstDns", .type = BLOBMSG_TYPE_STRING },
    [VLAN_DHCP_SECONDDNS] = { .name = "dhcpSecondDns", .type = BLOBMSG_TYPE_STRING },
    [VLAN_IGMPSNOOPING_ENABLE] = { .name = "igmpSnoopingEnable", .type = BLOBMSG_TYPE_BOOL },
};

const struct blobmsg_policy vlan_get_policy[__VLAN_GET_MAX] = {
    [VLAN_GET_TYPE] = { .name = "simple", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy switch_port_attrs_policy[__SWITCH_PORT_ATTR_MAX] = {
    [SWITCH_PORT_ATTR_PORTS] = { .name = "ports", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy switch_port_policy[__SWITCH_PORT_MAX] = {
    [SWITCH_PORT_PORT_ID] = { .name = "portId", .type = BLOBMSG_TYPE_STRING },
    [SWITCH_PORT_PVID] = { .name = "pvid", .type = BLOBMSG_TYPE_STRING },
    [SWITCH_PORT_VLANS] = { .name = "vlans", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy upgrade_auto_policy[__UPGRADE_AUTO_MAX] = {
    [UPGRADE_AUTO_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [UPGRADE_AUTO_SYNC_SLAVE_ENABLE] = { .name = "slaveSyncEnable", .type = BLOBMSG_TYPE_BOOL },
    [UPGRADE_AUTO_STARTTIME] = { .name = "upgradeStartTime", .type = BLOBMSG_TYPE_STRING },
    [UPGRADE_AUTO_ENDTIME] = { .name = "upgradeEndTime", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy guest_ssid_policy[__GUEST_SSID_MAX] = {
    [GUEST_SSID_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [GUEST_SSID_SSIDNAME] = { .name = "ssidName", .type = BLOBMSG_TYPE_STRING },
    [GUEST_SSID_ENCRYPTO] = { .name = "encryptoMode", .type = BLOBMSG_TYPE_STRING },
    [GUEST_SSID_PASSWD] = { .name = "passwordSsid", .type = BLOBMSG_TYPE_STRING },
    [GUEST_SSID_ALLOW_MASTER] = { .name = "allowMaster", .type = BLOBMSG_TYPE_BOOL },
    [GUEST_SSID_UPRATESSID] = { .name = "upRateSsid", .type = BLOBMSG_TYPE_STRING },
    [GUEST_SSID_DOWNRATESSID] = { .name = "downRateSsid", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy bind_ip_policy[__BIND_IP_MAX] = {
    [BIND_IP_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
    [BIND_IP_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
    [BIND_IP_IP] = { .name = "mapIp4Address", .type = BLOBMSG_TYPE_STRING },
    [BIND_IP_VLAN] = { .name = "vlan", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy network_attrs_policy[__NETWORK_ATTR_MAX] = {
    [NETWORK_ATTR_UP] = { .name = "up", .type = BLOBMSG_TYPE_BOOL },
    [NETWORK_ATTR_PROTO] = { .name = "proto", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ATTR_IPV4_ADDRESS] = { .name = "ipv4-address", .type = BLOBMSG_TYPE_ARRAY },
    [NETWORK_ATTR_ROUTE] = { .name = "route", .type = BLOBMSG_TYPE_ARRAY },
    [NETWORK_ATTR_USERNAME] = { .name = "username", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ATTR_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ATTR_DNS_SERVER] = { .name = "dns-server", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy upgrade_devices_policy[__UPGRADE_DEVICES_MAX] = {
    [DEVAP] = { .name = "devAp", .type = BLOBMSG_TYPE_STRING },
    [DEVMESH] = { .name = "devMesh", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy firewall_dos_policy[__FIREWALL_DOS_MAX] = {
    [FIREWALL_DOS_PROTECT] = { .name = "dosProtect", .type = BLOBMSG_TYPE_BOOL },
    [FIREWALL_DOS_SYNFLOOD_PROTECT] = { .name = "synFloodProtect", .type = BLOBMSG_TYPE_BOOL },
    [FIREWALL_DOS_SYNFLOOD_RATE] = { .name = "synFloodRate", .type = BLOBMSG_TYPE_STRING },
    //[FIREWALL_DOS_SYNFLOOD_BURST] = { .name = "synFloodBurst", .type = BLOBMSG_TYPE_STRING },
    [FIREWALL_DOS_UDPFLOOD_PROTECT] = { .name = "udpFloodProtect", .type = BLOBMSG_TYPE_BOOL },
    [FIREWALL_DOS_UDPFLOOD_RATE] = { .name = "udpFloodRate", .type = BLOBMSG_TYPE_STRING },
    //[FIREWALL_DOS_UDPFLOOD_BURST] = { .name = "udpFloodBurst", .type = BLOBMSG_TYPE_STRING },
    [FIREWALL_DOS_ICMPFLOOD_PROTECT] = { .name = "icmpFloodProtect", .type = BLOBMSG_TYPE_BOOL },
    [FIREWALL_DOS_ICMPFLOOD_RATE] = { .name = "icmpFloodRate", .type = BLOBMSG_TYPE_STRING },
    //[FIREWALL_DOS_ICMPFLOOD_BURST] = { .name = "icmpFloodBurst", .type = BLOBMSG_TYPE_STRING },
    [FIREWALL_DOS_PINGOFDEATH_PROTECT] = { .name = "pingOfDeathProtect", .type = BLOBMSG_TYPE_BOOL },
    [FIREWALL_DOS_PINGOFDEATH_LENGTH] = { .name = "pingOfDeathLength", .type = BLOBMSG_TYPE_STRING },
};

/*
 * Used to parse the message sent by sg
 */
const struct blobmsg_policy access_attrs_policy[__ACCESS_ATTR_MAX] = {
    [ACCESS_ATTR_CLIENT] = { .name = "clientAccess", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy snmp_policy[__SNMP_MAX] = {
    [SNMP_CONFIG] = { .name = "snmpConfig", .type = BLOBMSG_TYPE_TABLE },
    [SNMP_PORTS] = { .name = "snmpPorts", .type = BLOBMSG_TYPE_ARRAY },
    [SNMP_V3_AUTH] = { .name = "snmpv3Auth", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy snmp_config_policy[__SNMP_CONFIG_MAX] = {
    [SNMP_CONFIG_ENABLE_V1_V2C] = { .name = "enableV1V2C", .type = BLOBMSG_TYPE_BOOL },
    [SNMP_CONFIG_ENABLE_V3]  = { .name = "enableV3",      .type = BLOBMSG_TYPE_BOOL },
    [SNMP_CONFIG_SYS_LOCATION]  = { .name = "sysLocation",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_SYS_CONTACT]  = { .name = "sysContact",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_SYS_NAME]   = { .name = "sysName",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_RO_COMMUNITYV4]  = { .name = "roCommunityV4",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_RW_COMMUNITYV4]  = { .name = "rwCommunityV4",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_RO_COMMUNITYV6]  = { .name = "roCommunityV6",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_RW_COMMUNITYV6]   = { .name = "rwCommunityV6",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_TRAP_TYPE]  = { .name = "trapType",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_TRAP_HOST]  = { .name = "trapHost",      .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_TRAP_PORT]  = { .name = "trapPort",  .type = BLOBMSG_TYPE_STRING },
    [SNMP_CONFIG_TRAP_COMMUNITY]   = { .name = "trapCommunity",  .type = BLOBMSG_TYPE_STRING }
};

// vpn client
const struct blobmsg_policy vpn_client_attrs_policy[__VPN_CLIENT_ATTR_MAX] = {
    [VPN_CLIENT_ATTR_CLIENT_LIST] = { .name = "vpnServiceList", .type = BLOBMSG_TYPE_ARRAY },
};

// vpn server
const struct blobmsg_policy vpn_server_attrs_policy[__VPN_SERVER_ATTR_MAX] = {
    [VPN_SERVER_ATTR_SERVER_LIST] = { .name = "vpnServerList", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy vpn_split_attrs_policy[__VPN_SPLIT_ATTR_MAX] = {
    [VPN_SPLIT_ATTR_SERVICE_LIST] = { .name = "vpnSplit", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy vpn_split_dev_attrs_policy[__VPN_SPLIT_DEV_ATTR_MAX] = {
    [VPN_SPLIT_DEV_ATTR_NAME] = { .name = "devName", .type = BLOBMSG_TYPE_STRING },
    [VPN_SPLIT_DEV_ATTR_MAC] = { .name = "devMac", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy port_mapping_attrs_policy[__PORT_MAPPING_ATTR_MAX] = {
    [PORT_MAPPING_ATTR_RULE] = { .name = "rule", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy upnp_mapping_attrs_policy[__UPNP_MAPPING_ATTR_MAX] = {
    [UPNP_MAPPING_ATTR_DESCRIPTION] = { .name = "description", .type = BLOBMSG_TYPE_STRING },
    [UPNP_MAPPING_ATTR_IP] = { .name = "ip", .type = BLOBMSG_TYPE_STRING },
    [UPNP_MAPPING_ATTR_EXT_PORT] = { .name = "externalPort", .type = BLOBMSG_TYPE_STRING },
    [UPNP_MAPPING_ATTR_INTER_PORT] = { .name = "internalPort", .type = BLOBMSG_TYPE_STRING },
    [UPNP_MAPPING_ATTR_PROTO] = { .name = "proto", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy ddns_attrs_policy[__DDNS_ATTR_MAX] = {
    [DDNS_ATTR_LIST] = { .name = "list", .type = BLOBMSG_TYPE_ARRAY }
};

const struct blobmsg_policy static_route_ipv4_attrs_policy[__STATIC_ROUTE_IPV4_ATTR_MAX] = {
    [STATIC_ROUTE_IPV4_ATTR_RULE] = { .name = "rule", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy usb_share_policy[__USB_SHARE_MAX] = {
    [USB_SHARE_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [USB_SHARE_ANONYMITY] = { .name = "anonymity", .type = BLOBMSG_TYPE_BOOL },
    [USB_SHARE_USER] = { .name = "smbUser", .type = BLOBMSG_TYPE_STRING },
    [USB_SHARE_PASSWORD] = { .name = "smbPassword", .type = BLOBMSG_TYPE_STRING },
};

/*
 * Used to parse cm configuration files
 */
const struct blobmsg_policy cm_access_parse_policy[__CM_PARSE_ACCESS_MAX] = {
    [CM_PARSE_ACCESS_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
    [CM_PARSE_ACCESS_HOSTNAME]  = { .name = "hostName", .type = BLOBMSG_TYPE_STRING },
    [CM_PARSE_ACCESS_CLIENTOS]  = { .name = "clientOs", .type = BLOBMSG_TYPE_STRING },
    [CM_PARSE_ACCESS_WHITE]  = { .name = "white",  .type = BLOBMSG_TYPE_STRING },
    [CM_PARSE_ACCESS_BLACK]   = { .name = "black",  .type = BLOBMSG_TYPE_STRING },
    [CM_PARSE_ACCESS_BLOCK]   = { .name = "block",  .type = BLOBMSG_TYPE_STRING }
};

const struct blobmsg_policy global_access_attrs_policy[__GLOBAL_ACCESS_ATTR_MAX] = {
    [GLOBAL_ACCESS_ATTR_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [GLOBAL_ACCESS_ATTR_FORBIDURL] = { .name = "forbidUrl", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy schedule_access_attrs_policy[__SCHEDULE_ACCESS_ATTR_MAX] = {
    [SCHEDULE_ACCESS_ATTR_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [SCHEDULE_ACCESS_ATTR_WEEK] = { .name = "week", .type = BLOBMSG_TYPE_STRING },
    [SCHEDULE_ACCESS_ATTR_TIMESTART] = { .name = "timeSeqStart", .type = BLOBMSG_TYPE_STRING },
    [SCHEDULE_ACCESS_ATTR_TIMEEND] = { .name = "timeSeqEnd", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy basic_system_policy[__BASIC_SET_MAX] = {
    [BASIC_COUNTRY_CODE] = {  .name = "country", .type = BLOBMSG_TYPE_STRING },
    [BASIC_TIME_ZONE] = {  .name = "timeZone", .type = BLOBMSG_TYPE_STRING },
    [BASIC_LED_ON] = {  .name = "led", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy cm_extern_sys_log_policy[_EXTERNAL_LOG_MAX] = {
    [CM_ISSUE_AP_LIST] = { .name = "issueApList", .type = BLOBMSG_TYPE_ARRAY },
    [CM_EXTERN_LOG_URI] = { .name = "logUrl", .type = BLOBMSG_TYPE_STRING },
    [CM_EXTERN_LOG_LEVEL] = { .name = "logLevel", .type = BLOBMSG_TYPE_STRING },
    [CM_EXTERN_LOG_PROTOCOL] = { .name = "logProtocol", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy cm_email_policy[_CM_EMAIL_MAX] = {
    [CM_EMAIL_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING},
    [CM_EMAIL_HOST] = { .name = "host", .type = BLOBMSG_TYPE_STRING},
    [CM_EMAIL_USER] = { .name = "user", .type = BLOBMSG_TYPE_STRING},
    [CM_EMAIL_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING},
    [CM_EMAIL_DO_NOT_VALIDATE] = { .name = "doNotValidate", .type = BLOBMSG_TYPE_BOOL},
    [CM_EMAIL_ENABLE_NOTIFICATION] = { .name = "enableNotification", .type = BLOBMSG_TYPE_BOOL},
    [CM_EMAIL_FROM_ADDRESS] = { .name = "fromEmailAddress", .type = BLOBMSG_TYPE_STRING},
    [CM_EMAIL_FROM_NAME] = { .name = "fromName", .type = BLOBMSG_TYPE_STRING},
    [CM_EMAIL_EMAILADDRESS] = { .name = "emailaddress", .type = BLOBMSG_TYPE_ARRAY},
};

const struct blobmsg_policy cm_notification_policy[_CM_NOTIFY_MAX] = {
    [CM_NOTIFY_CM_MEMORY_USAGE] = { .name = "notifyMemoryUsage", .type = BLOBMSG_TYPE_BOOL},
    [CM_MEMORY_USAGE_THRESHOLD] = { .name = "memoryUsageThreshold", .type = BLOBMSG_TYPE_STRING},
    [CM_NOTIFY_AP_THROUGHPUT] = { .name = "notifyApThroughput", .type = BLOBMSG_TYPE_BOOL},
    [CM_AP_THROUGHPUT_THRESHOLD] = { .name = "apThroughputThreshold", .type = BLOBMSG_TYPE_STRING},
    [CM_NOTIFY_SSID_THROUGHPUT] = { .name = "notifySsidThroughput", .type = BLOBMSG_TYPE_BOOL},
    [CM_SSID_THROUGHPUT_THRESHOLD] = { .name = "ssidThroughputThreshold", .type = BLOBMSG_TYPE_STRING},
    [CM_NOTIFY_PASSWORD_CHANGE] = { .name = "notifyPasswordChange", .type = BLOBMSG_TYPE_BOOL},
    [CM_NOTIFY_FIRMWARE_UPGRADE] = { .name = "notifyFirmwareUpgrade", .type = BLOBMSG_TYPE_BOOL},
    [CM_NOTIFY_AP_OFFLINE] = { .name = "notifyApOffline", .type = BLOBMSG_TYPE_BOOL},
    [CM_NOTIFY_FIND_ROGUEAP] = { .name = "notifyFindRogueap", .type = BLOBMSG_TYPE_BOOL},
};

const struct blobmsg_policy mesh_ssid_attrs_policy[__MESH_SSID_ATTR_MAX] = {
    [MESH_SSID_ATTR_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [MESH_SSID_ATTR_MODE] = { .name = "mode", .type = BLOBMSG_TYPE_STRING },
    [MESH_SSID_ATTR_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
    [MESH_SSID_ATTR_BSSID] = { .name = "bssid", .type = BLOBMSG_TYPE_STRING },
    [MESH_SSID_ATTR_ENCRYPTION] = { .name = "encryption", .type = BLOBMSG_TYPE_STRING },
    [MESH_SSID_ATTR_KEY] = { .name = "key", .type = BLOBMSG_TYPE_STRING },
    [MESH_SSID_ATTR_ISADDED] = { .name = "isadded", .type = BLOBMSG_TYPE_BOOL },
    [MESH_SSID_ATTR_MASTER_ID] = { .name = "master_id", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy pair_status_policy[__PAIR_STATUS_MAX] = {
    [STATUS] = { .name = "status", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy cm_ap_policy[__CM_AP_MAX] = {
    [CM_AP_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_MESH] = { .name = "mesh", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_FREQUENCY] = { .name = "frequency", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_BAND_STEERING] = { .name = "bandSteering", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_ZONES] = { .name = "zones", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_SSIDS] = { .name = "ssids", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_TLS1_2] = { .name = "tls12", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_POSITION] = { .name = "position", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_POSITION_CUSTOM] = { .name = "positionCustom", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_SUPERIOR] = { .name = "superior", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_SSIDS] = { .name = "ssids", .type = BLOBMSG_TYPE_STRING  },
    [CM_AP_NETPORT_TYPE] = { .name = "netportType", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV4_ENABLE] = { .name = "ipv4Enable", .type = BLOBMSG_TYPE_BOOL },
    [CM_AP_IPV4_IP] = { .name = "ipv4IpAddress", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV4_NETMASK] = { .name = "ipv4NetMask", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV4_GATEWAY] = { .name = "ipv4Gateway", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV4_FIRST_DNS] = { .name = "ipv4FirstDns", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV4_SECOND_DNS] = { .name = "ipv4SecondDns", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV6_ENABLE] = { .name = "ipv6Enable", .type = BLOBMSG_TYPE_BOOL },
    [CM_AP_IPV6_IP] = { .name = "ipv6IpAddress", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV6_PREFIX_LENGTH] = { .name = "ipv6PrefixLength", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV6_GATEWAY] = { .name = "ipv6Gateway", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV6_FIRST_DNS] = { .name = "ipv6FirstDns", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_IPV6_SECOND_DNS] = { .name = "ipv6SecondDns", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_DISABLE] = { .name = "2g4Disable", .type = BLOBMSG_TYPE_BOOL },
    [CM_AP_2G4_WIDTH] = { .name = "2g4Width", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_CHANNEL_LOCATION] = { .name = "2g4ControlChannel", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_CHANNEL] = { .name = "2g4Channel", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_POWER] = { .name = "2g4TxPower", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_CUSTOM_TXPOWER] = { .name = "2g4CustomTxPower", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_RSSI_TYPE] = { .name = "2g4RssiType", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_RSSI_THRESHOLD] = { .name = "2g4RssiThreshold", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_MODE] = { .name = "2g4Mode", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_RATE_LIMIT_TYPE] = { .name = "2g4RateLimitType", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_MINI_RATE] = { .name = "2g4MiniRate", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_SHORTGI] = { .name = "2g4Shortgi", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_2G4_ALLOW_LEGACY_DEV] = { .name = "2g4AllowLegacyDev", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_DISABLE] = { .name = "5gDisable", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_WIDTH] = { .name = "5gWidth", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_CHANNEL] = { .name = "5gChannel", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_POWER] = { .name = "5gTXPower", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_CUSTOM_TXPOWER] = { .name = "5gCustomTxPower", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_RSSI_TYPE] = { .name = "5gRssiType", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_RSSI_THRESHOLD] = { .name = "5gRssiThreshold", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_RATE_LIMIT_TYPE] = { .name = "5gRateLimitType", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_MINI_RATE] = { .name = "5gMiniRate", .type = BLOBMSG_TYPE_STRING },
    [CM_AP_5G_SHORTGI] = { .name = "5gShortgi", .type = BLOBMSG_TYPE_STRING },
};

// vpn client
const struct blobmsg_policy vpn_client_policy[__VPN_CLIENT_MAX] = {
    [VPN_CLIENT_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [VPN_CLIENT_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_WAN_INTF] = { .name = "wan", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_L2TP_SERVER] = { .name = "l2tpServer", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_L2TP_USER] = { .name = "l2tpUser", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_L2TP_PASSWORD] = { .name = "l2tpPassword", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_PPTP_SERVER] = { .name = "pptpServer", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_PPTP_USER] = { .name = "pptpUser", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_PPTP_PASSWORD] = { .name = "pptpPassword", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_REMOTE_GW] = { .name = "ipsecP1RemoteGateway", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_IKE_VER]  = { .name = "ipsecP1IkeVersion", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_IKE_LIFETIME] = { .name = "ipsecP1IkeLifetime", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_NEGO_MODE] = { .name = "ipsecP1NegotiationMode", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_AUTH_METHOD] = { .name = "ipsecP1AuthMethod", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_PSK] = { .name = "ipsecP1Psk", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_ENCRYPT] = { .name = "ipsecP1Encryption", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_AUTH] = { .name = "ipsecP1Authentication", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_DH] = { .name = "ipsecP1Dh", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_REKEY] = { .name = "ipsecP1Rekey", .type = BLOBMSG_TYPE_BOOL },
    [VPN_CLIENT_IPSEC_P1_KEYINGTRIES] = { .name = "ipsecP1Keyingtries", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_DPD_EN] = { .name = "ipsecP1DpdEnable", .type = BLOBMSG_TYPE_BOOL },
    [VPN_CLIENT_IPSEC_P1_DPD_DELAY] = { .name = "ipsecP1DpdDelay", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_DPD_IDLE] = { .name = "ipsecP1DpdIdle", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P1_DPD_ACTION] = { .name = "ipsecP1DpdAction", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P2_LOCAL_SUBNET] = { .name = "ipsecP2LocalSubnet", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P2_LOCAL_SOURCE_IP] = { .name = "ipsecP2LocalSourceIp", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P2_REMOTE_SUBNET_LIST] = { .name = "ipsecP2RemoteSubnetList", .type = BLOBMSG_TYPE_ARRAY },
    [VPN_CLIENT_IPSEC_P2_SA_LIFETIME] = { .name = "ipsecP2SaLifetime", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P2_PROTO] = { .name = "ipsecP2Proto", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P2_ESP_ENCRYPT] = { .name = "ipsecP2EspEncryption", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P2_ESP_AUTH] = { .name = "ipsecP2EspAuthentication", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P2_ENCAP_MODE] = { .name = "ipsecP2EncapMode", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_IPSEC_P2_PFS_GROUP] = { .name = "ipsecP2PfsGroup", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy vpn_split_policy[__VPN_SPLIT_MAX] = {
    [VPN_SPLIT_SERVICE_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [VPN_SPLIT_MODE] = { .name = "mode", .type = BLOBMSG_TYPE_STRING },
    [VPN_SPLIT_SERVICE_ADDR_LIST] = { .name = "serviceAddrList", .type = BLOBMSG_TYPE_ARRAY },
    [VPN_SPLIT_DEV_LIST] = { .name = "devList", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy port_mapping_policy[__PORT_MAPPING_MAX] = {
    [PORT_MAPPING_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [PORT_MAPPING_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [PORT_MAPPING_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [PORT_MAPPING_EXT_PORT] = { .name = "externalPort", .type = BLOBMSG_TYPE_STRING },
    [PORT_MAPPING_INTER_PORT] = { .name = "internalPort", .type = BLOBMSG_TYPE_STRING },
    [PORT_MAPPING_INTER_IP] = { .name = "internalIP", .type = BLOBMSG_TYPE_STRING },
    [PORT_MAPPING_INTF] = { .name = "wan", .type = BLOBMSG_TYPE_STRING },
    [PORT_MAPPING_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
    [PORT_MAPPING_EXT_IP] = { .name = "externalIP", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy dmz_policy[__DMZ_MAX] = {
    [DMZ_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [DMZ_INTF] = { .name = "wan", .type = BLOBMSG_TYPE_STRING },
    [DMZ_DEV_IP] = { .name = "devIp", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy upnp_policy[__UPNP_MAX] = {
    [UPNP_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [UPNP_INTF] = { .name = "wan", .type = BLOBMSG_TYPE_STRING },
    [UPNP_MAPPING_TABLE] = { .name = "mapping_table", .type = BLOBMSG_TYPE_ARRAY }
};

const struct blobmsg_policy ddns_policy[__DDNS_MAX] = {
    [DDNS_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [DDNS_SERVICE_PROVIDER] = { .name = "serviceProvider", .type = BLOBMSG_TYPE_STRING },
    [DDNS_USERNAME] = { .name = "username", .type = BLOBMSG_TYPE_STRING },
    [DDNS_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING },
    [DDNS_HOSTNAME] = { .name = "hostname", .type = BLOBMSG_TYPE_STRING },
    [DDNS_INTF] = { .name = "wan", .type = BLOBMSG_TYPE_STRING },
    [DDNS_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_BOOL },
    [DDNS_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING }, 
};

const struct blobmsg_policy static_route_ipv4_policy[__STATIC_ROUTE_IPV4_MAX] = {
    [STATIC_ROUTE_IPV4_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV4_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV4_IP] = { .name = "ip", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV4_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV4_OUT_INTF] = { .name = "outInterface", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV4_NEXTHOP] = { .name = "nexthop", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV4_METRIC] = { .name = "metric", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV4_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_INT8 },
    [STATIC_ROUTE_IPV4_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy get_prired_devices_policy[__GS_GET_PAIRED_DEVICES_MAX] = {
    [GS_GET_PAIRED_DEVICES_DEVTYPE] = { .name = "devType", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy get_devices_from_controller_list_policy[__GS_CONTROLLER_DEVICES_ATTR_MAX] = {
    [GS_CONTROLLER_DEVICES_ATTR_LIST] = { .name = "devices", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy get_devices_from_controller_policy[__GS_CONTROLLER_DEVICES_INFO_MAX] = {
    [GS_CONTROLLER_DEVICES_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
    [GS_CONTROLLER_DEVICES_IPV4] = { .name = "ipv4", .type = BLOBMSG_TYPE_ARRAY },
    [GS_CONTROLLER_DEVICES_VERSION] = { .name = "version_firmware", .type = BLOBMSG_TYPE_STRING },
    [GS_CONTROLLER_DEVICES_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_STRING },
    [GS_CONTROLLER_DEVICES_SUPERIOR] = { .name = "superior", .type = BLOBMSG_TYPE_STRING },
    [GS_CONTROLLER_DEVICES_RADIO] = { .name = "radio", .type = BLOBMSG_TYPE_STRING },
    [GS_CONTROLLER_DEVICES_RSSI] = { .name = "rssi", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy cm_tr069_policy[__CM_TR069_MAX] = {
    [CM_TR069_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [CM_TR069_ACS_URL] = { .name = "acsUrl", .type = BLOBMSG_TYPE_STRING },
    [CM_TR069_ACS_NAME] = { .name = "acsName", .type = BLOBMSG_TYPE_STRING },
    [CM_TR069_ACS_PASSWORD] = { .name = "acsPassword", .type = BLOBMSG_TYPE_STRING },
    [CM_TR069_PERIODIC_INFORM_ENABLE] = { .name = "periodicInformEnable", .type = BLOBMSG_TYPE_BOOL },
    [CM_TR069_PERIODIC_INFORM_INTERVAL] = { .name = "periodicInformInterval", .type = BLOBMSG_TYPE_STRING },
    [CM_TR069_CPE_CERT_FILE] = { .name = "certFile", .type = BLOBMSG_TYPE_STRING },
    [CM_TR069_CPE_CERT_KEY] = { .name = "certKey", .type = BLOBMSG_TYPE_STRING },
    [CM_TR069_CONN_REQ_NAME] = { .name = "connReqName", .type = BLOBMSG_TYPE_STRING },
    [CM_TR069_CONN_REQ_PASSWORD] = { .name = "connReqPassword", .type = BLOBMSG_TYPE_STRING },
    [CM_TR069_CONN_REQ_PORT] = { .name = "connReqPort", .type = BLOBMSG_TYPE_STRING },
};

/*
 * Used to parse control command
 */
const struct blobmsg_policy vpn_client_ctrl_policy[__VPN_CLIENT_CTRL_MAX] = {
    [VPN_CLIENT_CTRL_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [VPN_CLIENT_CTRL_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy client_limit_policy[__CLIENT_LIMIT_MAX] = {
    [CLIENT_LIMIT_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [CLIENT_LIMIT_MAC] = { .name = "clientMac", .type = BLOBMSG_TYPE_STRING },
    [CLIENT_LIMIT_UPRAT] = { .name = "upRate", .type = BLOBMSG_TYPE_STRING },
    [CLIENT_LIMIT_DOWNRATE] = { .name = "downRate", .type = BLOBMSG_TYPE_STRING },
    //Speed limit is required for all SSIDs. This parameter 'ssidName' is not available for the time being
    [CLIENT_LIMIT_SSID_NAME] = { .name = "ssidName", .type = BLOBMSG_TYPE_STRING },
};


/*
 * Used to parse CMD_MAIN_EXT command
 */
const struct blobmsg_policy ex_unauthenticated_policy[__EX_UNAUTHENTICATED_MAX] = {
    [EX_UNAUTHENTICATED_FROM] = { .name = "from", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy wan_alias_policy[__WAN_ALIAS_ATTR_MAX] = {
    [WAN_ALIAS_ATTR_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [WAN_ALIAS_ATTR_IP] = { .name = "ip", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy static_route_ipv6_attrs_policy[__STATIC_ROUTE_IPV6_ATTR_MAX] = {
    [STATIC_ROUTE_IPV6_ATTR_RULE] = { .name = "rule", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy static_route_ipv6_policy[__STATIC_ROUTE_IPV6_MAX] = {
    [STATIC_ROUTE_IPV6_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [STATIC_ROUTE_IPV6_OUT_INTF] = { .name = "outInterface", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV6_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV6_PREFIX_LENGTH] = { .name = "prefixLength", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV6_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV6_NEXTHOP] = { .name = "nexthop", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV6_METRIC] = { .name = "metric", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV6_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
    [STATIC_ROUTE_IPV6_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy hostname_policy[__HOSTNAME_MAX] = {
    [HOSTNAME_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [HOSTNAME_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
};

// vpn server
const struct blobmsg_policy sgreq_vpn_server_policy[__VPN_SERVER_MAX] = {
    [VPN_SERVER_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [VPN_SERVER_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [VPN_SERVER_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [VPN_SERVER_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [VPN_SERVER_WAN_INTF] = { .name = "wan", .type = BLOBMSG_TYPE_STRING },
    [VPN_SERVER_IPSEC_CMN_SETTING] = { .name = "ipsecCommonSetting", .type = BLOBMSG_TYPE_TABLE },
    [VPN_SERVER_IPSEC_DIAL_IN_USER] = { .name = "ipsecDialInUser", .type = BLOBMSG_TYPE_ARRAY },
    // put other vpn server configs at here.
    [VPN_SERVER_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING }    
};

const struct blobmsg_policy vpn_server_policy[__VPN_SERVER_MAX] = {
    [VPN_SERVER_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [VPN_SERVER_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [VPN_SERVER_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [VPN_SERVER_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [VPN_SERVER_WAN_INTF] = { .name = "wan", .type = BLOBMSG_TYPE_STRING },
    [VPN_SERVER_IPSEC_CMN_SETTING] = { .name = "ipsecCommonSetting", .type = BLOBMSG_TYPE_STRING },
    [VPN_SERVER_IPSEC_DIAL_IN_USER] = { .name = "ipsecDialInUser", .type = BLOBMSG_TYPE_ARRAY },
    // put other vpn server configs at here.
    [VPN_SERVER_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING }
};

const struct blobmsg_policy ipsec_cmn_setting_policy[__IPSEC_CMN_SETTING_ATTR_MAX] = {
    [IPSEC_CMN_SETTING_ATTR_PSK] = { .name = "psk", .type = BLOBMSG_TYPE_STRING },
    [IPSEC_CMN_SETTING_ATTR_ENCRYPT_ALG] = { .name = "encryptionAlg", .type = BLOBMSG_TYPE_ARRAY },
    [IPSEC_CMN_SETTING_ATTR_AUTH_ALG] = { .name = "authenticationAlg", .type = BLOBMSG_TYPE_ARRAY },
    [IPSEC_CMN_SETTING_ATTR_DH] = { .name = "dh", .type = BLOBMSG_TYPE_ARRAY }
};

const struct blobmsg_policy ipsec_dial_in_user_policy[__IPSEC_DIAL_IN_USER_ATTR_MAX] = {
    [IPSEC_DIAL_IN_USER_ATTR_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [IPSEC_DIAL_IN_USER_ATTR_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [IPSEC_DIAL_IN_USER_ATTR_USER] = { .name = "user", .type = BLOBMSG_TYPE_STRING },
    [IPSEC_DIAL_IN_USER_ATTR_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING },
    [IPSEC_DIAL_IN_USER_ATTR_VLAN_ID] = { .name = "vlanId", .type = BLOBMSG_TYPE_STRING },
    [IPSEC_DIAL_IN_USER_ATTR_IP_RANGE] = { .name = "ipRange", .type = BLOBMSG_TYPE_STRING },
    [IPSEC_DIAL_IN_USER_ATTR_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING }  
};

const struct blobmsg_policy acceleration_policy[__ACCELERATION_MAX] = {
    [ACCELERATION_ENABLE] = { .name = "accelerationEnable", .type = BLOBMSG_TYPE_BOOL },
    [ACCELERATION_ENGINE] = { .name = "accelerationEngine", .type = BLOBMSG_TYPE_STRING },
};

/*
  * For portal set portal policy
*/
const struct blobmsg_policy cm_portal_policy[__PORTAL_POLICY_MAX] = {
    [PORTAL_POLICY_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_AUTH_TYPE] = { .name = "authType", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_EXPIRATION] = { .name = "expiration", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_IDLE_TIMEOUT] = { .name = "idleTimeout", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_PAGE_PATH] = { .name = "portalPagePath", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_LANDING_PAGE] = { .name = "landingPage", .type = BLOBMSG_TYPE_BOOL },
    [PORTAL_POLICY_LANDING_PAGE_URL] = { .name = "landingPageUrl", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_ENABLE_HTTPS] = { .name = "enableHttps", .type = BLOBMSG_TYPE_BOOL },
    [PORTAL_POLICY_ENABLE_HTTPS_REDIRECT] = { .name = "enableHttpsRedirect", .type = BLOBMSG_TYPE_BOOL },
    [PORTAL_POLICY_ENABLE_FAIL_SAFE] = { .name = "enableFailSafe", .type = BLOBMSG_TYPE_BOOL },
    [PORTAL_POLICY_ENABLE_DALY_LIMIT] = { .name = "enableDailyLimit", .type = BLOBMSG_TYPE_BOOL },

    [PORTAL_POLICY_RADIUS_SERVER] = { .name = "radiusServer", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_PORT] = { .name = "radiusPort", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_SECRET] = { .name = "radiusSecret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_METHOD] = { .name = "radiusMethod", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_DYNAMIC_VLAN] = { .name = "radiusDynamicVlan", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_ACCT_SERVER] = { .name = "radiusAcctServer", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_ACCT_PORT] = { .name = "radiusAcctPort", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_ACCT_SECRET] = { .name = "radiusAcctSecret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_ACCT_UPDATE_INTERVAL] = { .name = "radiusAcctUpdateInterval", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_RADIUS_NAS_ID] = { .name = "radiusNasId", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_ENABLE_THIRD] = { .name = "enableThird", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_ENABLE_BYTE_LIMIT] = { .name = "enableByteLimit", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_WECHAT_SHOP_ID] = { .name = "wechatShopId", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_WECHAT_APP_ID] = { .name = "wechatAppId", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_WECHAT_SECRET_KEY] = { .name = "wechatSecretKey", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_FB_APP_ID] = { .name = "fbAppId", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_FB_APP_SECRET] = { .name = "fbAppSecret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_FB_OAUTH_DOMAIN] = { .name = "fbAppOauthDomain", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_FB_OAUTH_PORT] = { .name = "fbAppOauthPort", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_FB_REAUTH] = { .name = "fbReAuth", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_GG_CLIENT_ID] = { .name = "ggClientId", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_GG_CLIENT_SECRET] = { .name = "ggClientSecret", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_TWITER_ID] = { .name = "twitterId", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_TWITER_CONSUMER_KEY] = { .name = "twitterConsumerKey", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_TWITER_CONSUMER_SECRET] = { .name = "twitterConsumerSecret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_TWITER_FORCE_TO_FOLLOW] = { .name = "twitterForceToFollow", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_SIMPLE_PASSWD] = { .name = "simplePasswd", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_OM_SPLASH_PAGE_URL] = { .name = "omSplashPageUrl", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_OM_SPLASH_PAGE_SECRET] = { .name = "omSplashPageSecret", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_OM_SPLASH_PAGE_AUTH_URL] = { .name = "omSplashPageAuthUrl", .type = BLOBMSG_TYPE_STRING },
    [PORTAL_POLICY_OM_SPLASH_PAGE_AUTH_SECRET] = { .name = "omSplashPageAuthSecret", .type = BLOBMSG_TYPE_STRING },

    [PORTAL_POLICY_PRE_AUTH] = { .name = "preAuth", .type = BLOBMSG_TYPE_ARRAY},
    [PORTAL_POLICY_POST_AUTH] = { .name = "postAuth", .type = BLOBMSG_TYPE_ARRAY },
};


const struct blobmsg_policy dev_ssid_policy[__DEV_SSID_MAX] = {
    [DEV_SSID_ID] = { .name = "ssidId", .type = BLOBMSG_TYPE_STRING },
    [DEV_SSID_AVAILABLE_DEV] = { .name = "availableDev", .type = BLOBMSG_TYPE_ARRAY },
    [DEV_SSID_MEMBER_DEV] = { .name = "memberDev", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy cm_schedule_policy[__CM_SCHEDULE_MAX] = {
    [CM_SCHEDULE_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_WEEKLY_TIME1] = { .name = "wtime1", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_WEEKLY_TIME2] = { .name = "wtime2", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_WEEKLY_TIME3] = { .name = "wtime3", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_WEEKLY_TIME4] = { .name = "wtime4", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_WEEKLY_TIME5] = { .name = "wtime5", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_WEEKLY_TIME6] = { .name = "wtime6", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_WEEKLY_TIME7] = { .name = "wtime7", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_ABTIME_LISTS] = { .name = "abtime", .type = BLOBMSG_TYPE_ARRAY },
};

const struct blobmsg_policy cm_schedule_abtime_policy[__CM_SCHEDULE_ABTIME_MAX] = {
    [CM_SCHEDULE_ABTIME_ABDATE] = { .name = "abdate", .type = BLOBMSG_TYPE_STRING },
    [CM_SCHEDULE_ABTIME_ABTIME] = { .name = "abtime", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy cm_schedule_get_policy[__CM_SCHEDULE_ABTIME_MAX] = {
    [CM_SCHEDULE_GET_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
};


static struct blobmsg_policy get_ap_info_policy[__GET_AP_INFO_MAX] = {
    [GET_AP_INFO_MAC]= { .name = "mac", .type = BLOBMSG_TYPE_STRING },
};

/*
 * Private Functions
 */
static int
sgreq_set_radio_by_blob(
    struct blob_attr *data,
    int type
);

/*
 * Private Data
 */
static struct blob_buf reply;
static struct blob_buf send_buf;
static int vpn_client_chg = 0;

//=================
//  Functions
//=================

//=============================================================================
static void
sgreq_return_must_info_to_sg(
    struct blob_buf *reply,
    struct sg_params *sg_params,
    int error_state
)
//=============================================================================
{
    char temp_buf[BUF_LEN_128];
    if( NULL == sg_params ) {
        return;
    }

    //blobmsg_add_string( reply, "hashID", sg_params->hashID );
    blobmsg_add_u32( reply, "id", sg_params->id );
    cfubus_wss_error_state_handle( error_state, temp_buf, sizeof( temp_buf ) );
    blobmsg_add_u32( reply, "retVal", error_state );
    blobmsg_add_string( reply, "retMsg", temp_buf );
}

//=============================================================================
int
sgreq_convert_cm_proto(
    const char *proto_str
)
//=============================================================================
{
    if( NULL == proto_str ) {
        return __WANTYPE_MAX;
    }

    if( 0 == strcmp( "dhcp", proto_str ) ) {
        return WANTYPE_DHCP;
    }
    else if( 0 == strcmp( "static", proto_str ) ) {
        return WANTYPE_STATIC;
    }
    else if( 0 == strcmp( "pppoe", proto_str ) ) {
        return WANTYPE_PPPOE;
    }
    else if( 0 == strcmp( "pptp", proto_str ) ) {
        return WANTYPE_PPTP;
    }
    else if( 0 == strcmp( "l2tp", proto_str ) ) {
        return WANTYPE_L2TP;
    }
    else {
        cfmanager_log_message( L_DEBUG, "Unknown proto:%s\n",
            proto_str );
        return __WANTYPE_MAX;
    }
}

//=============================================================================
void
sgreq_convert_sg_proto(
    int proto,
    char *proto_str,
    int size
)
//=============================================================================
{
    if( NULL == proto_str )
        return;

    switch ( proto ) {
        case WANTYPE_DHCP:
            strncpy( proto_str, "dhcp", size -1 );
            break;
        case WANTYPE_STATIC:
            strncpy( proto_str, "static", size -1 );
            break;
        case WANTYPE_PPPOE:
            strncpy( proto_str, "pppoe", size -1 );
            break;
        case WANTYPE_PPTP:
            strncpy( proto_str, "pptp", size -1 );
            break;
        case WANTYPE_L2TP:
            strncpy( proto_str, "l2tp", size -1 );
            break;
        default:
            cfmanager_log_message( L_DEBUG, "Unknown proto:%d\n",
                proto );
            break;
    };
}

//=============================================================================
int
sgreq_convert_cm_crypto(
    const char *crypto
)
//=============================================================================
{
    if ( 0 == strcmp( crypto, "psk+aes" ) )
        return CRYPTO_WPA_WPA2_PERSONAL;
    else if ( 0 == strcmp( crypto, "sae-psk2+aes" ) )
        return CRYPTO_WPA2_WPA3_PERSONAL;
    else if ( 0 == strcmp( crypto, "sae+aes" ) )
        return CRYPTO_WPA3_PERSONAL;
    else {
        cfmanager_log_message( L_DEBUG, "Unknown encryption method:%s\n",
            crypto );

        return __CRYPTO_MAX;
    }
}

//=============================================================================
int
sgreq_convert_cm_link_speed(
    const char *link_speed
)
//=============================================================================
{
    if ( strcmp( link_speed, "speed:1000baseT" ) == 0 )
        return WANLINKSPEED_1000M;
    else if ( strcmp( link_speed, "speed:100baseT" ) == 0 )
        return WANLINKSPEED_100M;
    else
        return WANLINKSPEED_AUTO;
}

//=============================================================================
void
sgreq_ctrl_vpn_client(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_attr *tb1[__VPN_CLIENT_CTRL_MAX] = { 0 };
    struct blob_attr *tb2[__VPN_CLIENT_MAX] = { 0 };
    char ifname[BUF_LEN_16] = { 0 };
    struct cm_config *cm_cfg = NULL;
    char section_nm[BUF_LEN_64] = { 0 };
    char *client_id;
    int found = 0;
    int action;

    blobmsg_parse( vpn_client_ctrl_policy, 
        __VPN_CLIENT_CTRL_MAX,
        tb1,
        blobmsg_data( data ),
        blobmsg_len( data) );

    if ( !tb1[VPN_CLIENT_CTRL_ID] || !tb1[VPN_CLIENT_CTRL_ACTION] ) {                                
        cfmanager_log_message( L_ERR, "Missing id or action in message!\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto out;
    }

    action = atoi( blobmsg_get_string(tb1[VPN_CLIENT_CTRL_ACTION]) );
    if ( action >= __VPN_CLIENT_ACTION_MAX ) {
        cfmanager_log_message( L_ERR, "Invalid action %d!\n", action );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto out;
    }

    client_id = blobmsg_get_string(tb1[VPN_CLIENT_CTRL_ID]);
    snprintf( section_nm, sizeof(section_nm), "vpn_service%s", client_id );
    vlist_for_each_element( &cm_vpn_client_vltree, cm_cfg, node ) {
        blobmsg_parse( vpn_client_policy,
            __VPN_CLIENT_MAX,
            tb2,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        if ( !strcmp( section_nm, cm_cfg->cf_section.name ) ) {
            found = 1;
            if ( !blobmsg_get_bool(tb2[VPN_CLIENT_ENABLE]) && action != VPN_CLIENT_ACTION_DELETE ) {
                cfmanager_log_message( L_ERR, "Vpn service %s is disabled, cannot execute action %d!\n",
                    blobmsg_get_string(tb2[VPN_CLIENT_ID]), action );
                error_state = ERRCODE_COMMAND_INVALID;
                goto out;
            }
            break;
        }
    }

    if ( !found ) {
        cfmanager_log_message( L_ERR, "Cannot found vpn client id %s\n", client_id );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto out;
    }

    snprintf( ifname, sizeof(ifname), "vpn%d",
        atoi(client_id) + VPN_CLIENT_ID_OFFSET );
    if ( action == VPN_CLIENT_ACTION_DELETE ) {
        vpn_del_client( section_nm );
        vpn_del_vpn_split_by_client( client_id );
        cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
        cfparse_load_file( "mwan3", LOAD_ALL_SECTION, false );
    }
    else {
        if ( util_blobmsg_get_int(tb2[VPN_CLIENT_TYPE], VPN_CLIENT_TYPE_L2TP) == VPN_CLIENT_TYPE_IPSEC ) {
            vpn_ctrl_ipsec_status( section_nm, action );
        }
        else if ( cfubus_ctrl_vpn_status( ifname, action ) < 0 ) {
            error_state = ERRCODE_INTERNAL_ERROR;
        };
    }

out:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
bool
sgreq_compar_attr(
    struct blob_attr *a1,
    struct blob_attr *a2,
    int policy_type
)
//=============================================================================
{
    int size = 0;

    if ( !a1 && !a2 ) {
        return true;
    }

    if ( !a1 || !a2 ) {
        if( BLOBMSG_TYPE_STRING != policy_type ) {
            return false;
        }

        if( !a1 ) {
            size = strlen( blobmsg_get_string( a2 ) );
        }
        else {
            size = strlen( blobmsg_get_string( a1 ) );
        }

        if( 0 == size ) {
            return true;
        }
        else {
            return false;
        }
    }

    if ( blob_pad_len( a1 ) != blob_pad_len( a2 ) ) {
        return false;
    }

    return !memcmp( a1, a2, blob_pad_len( a1 ) );
}

//=============================================================================
void
sgreq_set_upgrade_auto(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__UPGRADE_AUTO_MAX];
    struct blob_attr *cm_tb[__UPGRADE_AUTO_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int error_state = 0;
    int i = 0;
    bool need_update = false;

    blobmsg_parse( upgrade_auto_policy,
        __UPGRADE_AUTO_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    memset( cm_tb, 0, __UPGRADE_AUTO_MAX * sizeof(*cm_tb) );
    cm_cfg = util_get_vltree_node( &cm_upgrade_auto_vltree, VLTREE_CM_TREE, "upgrade_auto" );
    if( !cm_cfg ) {
        error_state = config_add_named_section( CFMANAGER_CONFIG_NAME,
                          "upgrade_auto", "upgrade_auto" );
        if ( error_state != 0 ) {
            cfmanager_log_message( L_ERR,
             "failed to add section upgrade_auto in %s\n", CFMANAGER_CONFIG_NAME );
            error_state = ERRCODE_INTERNAL_ERROR;
            goto return_value;
        }
    }
    else {
        blobmsg_parse( upgrade_auto_policy,
            __UPGRADE_AUTO_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );
    }

    for( i = 0; i < __UPGRADE_AUTO_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], upgrade_auto_policy[i].type ) ) {
            continue;
        }

        sprintf( path, "%s.upgrade_auto.%s", CFMANAGER_CONFIG_NAME, upgrade_auto_policy[i].name );
        config_set_by_blob( tb[i], path, upgrade_auto_policy[i].type );

        need_update = true;
    }

    if( !need_update ) {
        cfmanager_log_message( L_DEBUG, "The upgrade_auto config is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_UPGRADE_AUTO, false );

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
static int
sgreq_parse_wan_alias(
    struct blob_attr *enable_attr,
    struct blob_attr *ip_attr,
    int wan_type
)
//=============================================================================
{
    struct blob_attr *cur = NULL;
    struct cm_config *vltree_node = NULL;
    int i = 0;
    int ret = 0;
    const char *section_type = CM_WAN_ALIAS_SECTION_TYPE;
    const char *section_name = NULL;
    char path[LOOKUP_STR_SIZE];
    char *ip = NULL;

    if( !enable_attr || !ip_attr ) {
        cfmanager_log_message( L_ERR, "Illegal parameter\n" );
        return ERRCODE_INTERNAL_ERROR;
    }

    if( WAN0 == wan_type ) {
        section_name= CM_WAN0_ALIAS_SECTION_NAME;
    }
    else {
        section_name= CM_WAN1_ALIAS_SECTION_NAME;
    }

    vltree_node = util_get_vltree_node( &cm_wan_alias_vltree, VLTREE_CM_TREE, section_name );
    if( NULL == vltree_node ) {
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, section_type, section_name );
        if( ret < 0 ) {
            cfmanager_log_message( L_ERR, "create section %s failed\n", section_name  );
            return ERRCODE_INTERNAL_ERROR;
        }
    }
    else {
        snprintf( path, sizeof( path ), "%s.%s.ip", CFMANAGER_CONFIG_NAME, section_name );
        config_uci_del( path, 0 );
    }

    snprintf( path, sizeof( path ), "%s.%s.enable", CFMANAGER_CONFIG_NAME, section_name );
    config_set_by_blob( enable_attr, path, wan_alias_policy[WAN_ALIAS_ATTR_ENABLE].type );

    snprintf( path, sizeof( path ), "%s.%s.ip", CFMANAGER_CONFIG_NAME, section_name );
    blobmsg_for_each_attr( cur, ip_attr, i ) {
        if( BLOBMSG_TYPE_STRING != blobmsg_type( cur ) ) {
            continue;
        }

        ip = util_blobmsg_get_string( cur, "" );

        config_uci_add_list( path, ip, 0 );
    }

    return ERRCODE_SUCCESS;
}
//=============================================================================
void
sgreq_set_wan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__WAN_MAX];
    struct blob_attr *cm_tb[__WAN_MAX];
    struct cm_config *cm_cfg = NULL;
    struct me_info meif = {{ 0 }};
    char path[LOOKUP_STR_SIZE] = { 0 };
    char ipv4[IP4ADDR_MAX_LEN+1];
    char mtu_str[BUF_LEN_8] = { 0 };
    int wan_type_old = 0;
    int wan_type_new = 0;
    int error_state = 0;
    int i = 0;
    int flag = 0;
    int wan_alias_flag = 0;
    int mtu = 0;
    //need to regenerate the flags of firewall and mwan3
    int firewall_flag = 0;
    int mwan3_flag = 0;
    int wan0_cancel_vpn = 0;
    int wan0_set_vpn = 0;
    char vpn_id_on_wan[BUF_LEN_16] = { 0 };
#ifdef DOUBLE_WAN
    int wan1_cancel_vpn = 0;
    int wan1_set_vpn = 0;
    int wan1_en_chg = 0;
    int wan1_enable;
#endif

    blobmsg_parse( wan_policy,
        __WAN_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    cm_cfg = util_get_vltree_node( &cm_wan_vltree, VLTREE_CM_TREE, "wan" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No wan information found in %s\n", CFMANAGER_CONFIG_NAME );
        goto return_value;
    }

    blobmsg_parse( wan_policy,
        __WAN_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

    for( i = 0; i < __WAN_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], wan_policy[i].type ) ) {
            continue;
        }

        //Judge whether or not need to resolve and generate firewall and mwan3 again
        switch (i) {
            case WAN_STATIC_DNSENABLE:
            case WAN_FIRSTDNS:
            case WAN_SECONDDNS:
#ifdef DOUBLE_WAN
            case WAN1_ENABLE:
            case WAN1_STATIC_DNSENABLE:
            case WAN1_FIRSTDNS:
            case WAN1_SECONDDNS:
#endif
                firewall_flag = 1;
                mwan3_flag = 1;
#ifdef DOUBLE_WAN
                if ( i == WAN1_ENABLE ) {
                    wan1_en_chg = 1;
                }
#endif
                break;
            case WAN_TYPE:
#ifdef DOUBLE_WAN
            case WAN1_TYPE:
#endif
            {
                char *wan_type_new_str;
                char *wan_type_old_str;

                wan_type_new_str = blobmsg_get_string( tb[i] );
                wan_type_old_str = blobmsg_get_string( cm_tb[i] );

                wan_type_new = atoi( wan_type_new_str );
                if( wan_type_old_str ) {
                    wan_type_old = atoi( wan_type_old_str );

                    // Check if wan type change from vpn to others.
                    if ( (wan_type_old == WANTYPE_PPTP || wan_type_old == WANTYPE_L2TP) &&
                            wan_type_new != WANTYPE_PPTP && wan_type_new != WANTYPE_L2TP ) {
                        if ( i == WAN_TYPE ) {
                            wan0_cancel_vpn = 1;
                        }
#ifdef DOUBLE_WAN
                        else {
                            wan1_cancel_vpn = 1;
                        }
#endif
                    }

                    // Check if wan type change from others to vpn.
                    if ( wan_type_old != WANTYPE_PPTP && wan_type_old != WANTYPE_L2TP &&
                            (wan_type_new == WANTYPE_PPTP || wan_type_new == WANTYPE_L2TP) ) {
                        if ( i == WAN_TYPE ) {
                            wan0_set_vpn = 1;
                        }
#ifdef DOUBLE_WAN
                        else {
                            wan1_set_vpn = 1;
                        }
#endif
                    }
                }
                else {
                    if( WANTYPE_PPTP == wan_type_new || WANTYPE_L2TP == wan_type_new ) {
                        if ( i == WAN_TYPE ) {
                            wan0_set_vpn = 1;
                        }
#ifdef DOUBLE_WAN
                        else {
                            wan1_set_vpn = 1;
                        }
#endif
                    }
                }

                firewall_flag = 1;
                mwan3_flag = 1;
                /* This may be the first time to quickly configure.
                 * MTU will not be issued. We need to check whether
                 * the value of MTU is correct
                 */
                if( WANTYPE_PPPOE == wan_type_new && !tb[WAN_MTU] ) {
                    strncpy( mtu_str, util_blobmsg_get_string( cm_tb[WAN_MTU], "" ),
                        sizeof( mtu_str ) -1 );
                    if( atoi( mtu_str ) > PPPOE_MTU_DEVALUE ) {
                        snprintf( path, sizeof( path ), "%s.wan.wanMtu",
                            CFMANAGER_CONFIG_NAME );
                        snprintf( mtu_str, sizeof( mtu_str ), "%d", PPPOE_MTU_DEVALUE );
                        config_uci_set( path, mtu_str, 0 );
                    }
                }
            }
            case WAN_BALANCE:
                mwan3_flag = 1;
                break;
            case WAN_IP4ADDRESS:
            case WAN_L2TPIP4ADDRESS:
            case WAN_PPTPIP4ADDRESS:
#ifdef DOUBLE_WAN
            case WAN1_IP4ADDRESS:
            case WAN1_L2TPIP4ADDRESS:
            case WAN1_PPTPIP4ADDRESS:
#endif
                //IP legitimacy check
                memset( ipv4, 0, sizeof( ipv4 ) );
                strncpy( ipv4, blobmsg_get_string( tb[i] ), IP4ADDR_MAX_LEN );
                if( !util_validate_ipv4( ipv4 ) ) {
                    error_state = ERRCODE_PARAMETER_ERROR;
                    cfmanager_log_message( L_ERR, "The set IP address is illegal\n" );
                    goto return_value;
                }
                break;
            case WAN_MTU:
#ifdef DOUBLE_WAN
            case WAN1_MTU:
#endif
                if( WAN_MTU == i ) {
                    wan_type_new = util_blobmsg_get_int( tb[WAN_TYPE], 0 );
                }
#ifdef DOUBLE_WAN
                else {
                    wan_type_new = util_blobmsg_get_int( tb[WAN1_TYPE], 0 );
                }
#endif
                mtu = util_blobmsg_get_int( tb[i], 0 );
                if( MIN_MTU > mtu || MAX_MTU < mtu ) {
                    error_state = ERRCODE_PARAMETER_ERROR;
                    cfmanager_log_message( L_ERR, "The mtu:%d is illegal, the wan proto:%d\n",
                        mtu, wan_type_new );
                    goto return_value;
                }
                break;
            case WAN_MACMODE:
#ifdef DOUBLE_WAN
            case WAN1_MACMODE:
#endif
                if( WANMAC_PC == util_blobmsg_get_int( tb[i], 0 ) ) {
                    cfubus_get_me_info( &meif );
                    snprintf( path, sizeof( path ),
                        "%s.wan.%s", CFMANAGER_CONFIG_NAME, wan_policy[i].name );
                    config_uci_set( path, "2", 0 );

                    if( WAN_MACMODE == i ) {
                        snprintf( path, sizeof( path ),
                            "%s.wan.wanMac", CFMANAGER_CONFIG_NAME );
                    }
#ifdef DOUBLE_WAN
                    else {
                        snprintf( path, sizeof( path ),
                            "%s.wan.wan1Mac", CFMANAGER_CONFIG_NAME );
                    }
#endif
                    config_uci_set( path, meif.mac, 0 );

                    flag = 1;
                    continue;
                }
                break;
            case WAN_ALIAS_ENABLE:
            case WAN_ALIAS_IP:
                error_state = sgreq_parse_wan_alias( tb[WAN_ALIAS_ENABLE], tb[WAN_ALIAS_IP], WAN0 );
                if( ERRCODE_SUCCESS != error_state ) {
                    cfmanager_log_message( L_ERR, "wan0 alias set failed error:%d", error_state );
                    goto return_value;
                }
                wan_alias_flag = 1;
                break;
#ifdef DOUBLE_WAN
            case WAN1_ALIAS_ENABLE:
            case WAN1_ALIAS_IP:
                error_state = sgreq_parse_wan_alias( tb[WAN1_ALIAS_ENABLE], tb[WAN1_ALIAS_IP], WAN1 );
                if( ERRCODE_SUCCESS != error_state ) {
                    cfmanager_log_message( L_ERR, "wan1 alias set failed error:%d", error_state );
                    goto return_value;
                }
                wan_alias_flag = 1;
                break;
#endif
            default:
                break;
        }

        snprintf( path, sizeof( path ), "%s.wan.%s", CFMANAGER_CONFIG_NAME, wan_policy[i].name );
        config_set_by_blob( tb[i], path, wan_policy[i].type );

        flag = 1;
    }

    if( !flag && !wan_alias_flag ) {
        cfmanager_log_message( L_DEBUG, "The wan config is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );

    if( flag ) {
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_WAN, false );
    }
    if( wan_alias_flag ) {
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_WAN_ALIAS, false );
    }

#ifdef DOUBLE_WAN
    wan1_enable = util_blobmsg_get_bool( tb[WAN1_ENABLE], false );
#endif

    // If wan type change from vpn to others,
    // we should delete such vpn split tunneling refered to this wan.
    if ( wan0_cancel_vpn ) {
        snprintf( vpn_id_on_wan, sizeof(vpn_id_on_wan), "%d", VPN_ID_ON_WAN0 );
        vpn_del_vpn_split_by_client( vpn_id_on_wan );
    }

#ifdef DOUBLE_WAN
    if ( wan1_cancel_vpn && wan1_enable ) {
        snprintf( vpn_id_on_wan, sizeof(vpn_id_on_wan), "%d", VPN_ID_ON_WAN1 );
        vpn_del_vpn_split_by_client( vpn_id_on_wan );
    }
#endif

    // If wan type is vpn, we should delete other vpn client refer to this wan.
    if ( wan0_set_vpn ) {
        vpn_del_client_by_wan( "wan0" );
    }

#ifdef DOUBLE_WAN
    if ( wan1_set_vpn && wan1_enable ) {
        vpn_del_client_by_wan( "wan1" );
    }

    if ( wan1_en_chg ) {
        if ( !wan1_enable ) {
            ddns_delete_by_wan( "wan1" );
            firewall_delete_port_mapping_by_wan( "wan1" );
            upnpd_reset_by_wan( "wan1" );
            firewall_delete_dmz_by_wan( "wan1" );
            route_delete_ipv4_static_route_by_wan( "wan1" );
            vpn_del_client_by_wan( "wan1" );
            vpn_del_server_by_wan( "wan1" );
        }
    }
#endif

    if( firewall_flag ) {
        cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
    }

    if( mwan3_flag ) {
        cfparse_load_file( "mwan3", LOAD_ALL_SECTION, false );
    }

return_value:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
static void
sgreq_flush_bind_ip(
    struct blob_attr *data,
    char *path
 )
//=============================================================================
{
    //Delete all del data
    struct blob_attr *cur = NULL;
    char value[BUF_LEN_128] = { 0 };
    char temp[BUF_LEN_128] = { 0 };
    int rem = 0;
    int action = 0;
    char action_str[BUF_LEN_8] = { 0 };
    char mac[MAC_STR_MAX_LEN+1] = { 0 };
    char ip[IP4ADDR_MAX_LEN+1] = { 0 };
    char vlan[BUF_LEN_8+1] = {0};
    int ret = 0;

    if( NULL == data || NULL == path ) {
        return;
    }

    blobmsg_for_each_attr( cur, data, rem ) {
        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING ) {
            continue;
        }
        memset( value, 0, sizeof( value ) );
        strncpy( value, blobmsg_get_string( cur ), sizeof( value ) -1 );

        ret = util_parse_string_data( value, ",", "action",
                7, action_str, sizeof( action_str ) );
        if( ret < 0 ) {
            continue;
        }
        action = atoi( action_str );
        if( BIND_IP_ACTION_DEL != action ) {
            continue;
        }

        ret = util_parse_string_data( value, ",", "mac",
                4, mac, sizeof( mac ) );
        if( ret < 0 ) {
            continue;
        }

        ret = util_parse_string_data( value, ",", "mapIp4Address",
                14, ip, sizeof( ip ) );
        if( ret < 0 ) {
            continue;
        }

        ret = util_parse_string_data( value, ",", "vlan",
                14, vlan, sizeof( vlan ) );
        if( ret < 0 ) {
            continue;
        }

        sprintf( temp, "action:%d,mac:%s,mapIp4Address:%s,vlan:%s", action, mac, ip, vlan );
        config_uci_del_list( path, temp, 0 );
    }
}

//=============================================================================
static void
sgreq_set_bind_ip(
    struct blob_attr *data_new,
    char *path
)
//=============================================================================
{
    //format action:xxx,mac:00:0b:82:9c:ef:99,mapIp4Address:xxx,vlan:xxxx
    struct blob_attr *cur = NULL;
    struct blob_attr *tb[__BIND_IP_MAX];
    char value[BUF_LEN_128];
    char mac[MAC_STR_MAX_LEN+1];
    char format_mac[MAC_STR_MAX_LEN+1];
    char ip[IP4ADDR_MAX_LEN+1];
    char vlan[BUF_LEN_8] = {0};
    char temp[BUF_LEN_128];
    int action = 0;
    int rem = 0;

    if( NULL == data_new || NULL == path ) {
        return;
    }

    blobmsg_for_each_attr( cur, data_new, rem ) {
        blobmsg_parse( bind_ip_policy,
            __BIND_IP_MAX,
            tb,
            blobmsg_data( cur ),
            blobmsg_len( cur ) );

        if( !tb[BIND_IP_ACTION] || !tb[BIND_IP_MAC] || !tb[BIND_IP_IP] ) {
            continue;
        }

        memset( mac, 0, sizeof( mac ) );
        memset( ip, 0, sizeof( ip ) );
        memset( format_mac, 0, sizeof( format_mac ) );

        action = atoi( blobmsg_get_string( tb[BIND_IP_ACTION] ) );
        strncpy( mac, blobmsg_get_string( tb[BIND_IP_MAC] ), sizeof( mac ) -1 );
        util_formatted_mac_with_colo( mac, format_mac );
        strncpy( ip, blobmsg_get_string( tb[BIND_IP_IP] ), sizeof( ip ) -1 );
        strncpy( vlan, util_blobmsg_get_string( tb[BIND_IP_VLAN], "1" ), sizeof( vlan ) -1 );
        snprintf( value, sizeof( value ), "action:%d,mac:%s,mapIp4Address:%s,vlan:%s", action, format_mac, ip, vlan );

        if( BIND_IP_ACTION_DEL == action ) {
            //Delete add data
            sprintf( temp, "action:%d,mac:%s,mapIp4Address:%s,vlan:%s", BIND_IP_ACTION_ADD, format_mac, ip, vlan );
            config_uci_del_list( path, temp, 0 );
            config_uci_add_list( path, value, 0 );
        }
        else if( BIND_IP_ACTION_ADD == action ) {
            config_uci_add_list( path, value, 0 );
        }
        else {
            cfmanager_log_message( L_DEBUG, "Unknown action:%d\n", action );
            continue;
        }
    }
}

//=============================================================================
static int
sgreq_check_ipaddr(
    const char *address,
    const char *netmask,
    const char *iprange
)
//=============================================================================
{
    uint32_t addr = 0;
    uint32_t mask = 0;
    uint32_t imask = 0;
    uint32_t max = 0;

    //RFC 1918
    uint32_t addr_s1 = utils_addr_to_int( "10.0.0.0" );
    uint32_t addr_e1 = utils_addr_to_int( "10.255.255.255" );

    uint32_t addr_s2 = utils_addr_to_int( "172.16.0.0" );
    uint32_t addr_e2 = utils_addr_to_int( "172.31.255.255" );

    uint32_t addr_s3 = utils_addr_to_int( "192.168.0.0" );
    uint32_t addr_e3 = utils_addr_to_int( "192.168.255.255" );

    char range[16] = { 0 };
    char buf[32] = { 0 };
    char *p = NULL;

    uint32_t dhcp_start = 0;
    uint32_t dhcp_end = 0;

    char dhcp_s[32];
    char dhcp_e[32];

    if ( address ) {
        addr = utils_addr_to_int( address );
        if ( ( ( addr >= addr_s1 ) && ( addr <= addr_e1 ) )
            || ( ( addr >= addr_s2 ) && ( addr <= addr_e2 ) )
            || ( ( addr >= addr_s3 ) && ( addr <= addr_e3 ) ) ) {
            cfmanager_log_message( L_DEBUG, "Address %s RFC1918 compliant\n", address );
        }
        else {
            cfmanager_log_message( L_ERR, "Wrong address %s\n", address );
            return ERRCODE_PARAMETER_ERROR;
        }
    }

    if ( netmask ) {
        mask = utils_addr_to_int( netmask );
        imask = ~mask;

        if ( ( ( imask + 1 ) & imask ) ) { //Determine whether it is 2^n
            cfmanager_log_message( L_ERR, "Wrong mask %s\n", netmask );

            return ERRCODE_PARAMETER_ERROR;
        }

        max = (addr & mask) | imask;
    }


    if ( iprange ) {
        snprintf( range, sizeof( range ), "%s", iprange );

        p = strstr( range, "-" );
        if( !p ) {
            cfmanager_log_message( L_ERR, "Wrong ip range %s\n", range );
            return ERRCODE_PARAMETER_ERROR;
        }

        dhcp_end = atoi( p + 1 );
        *p = '\0';
        dhcp_start = atoi( range );

        cfmanager_log_message( L_DEBUG, "dhcp_start %d, dhcp_end %d\n", dhcp_start, dhcp_end );

        if ( dhcp_start > dhcp_end  ) {
            cfmanager_log_message( L_ERR, "Wrong ip range %d-%d\n", dhcp_start, dhcp_end  );
            return ERRCODE_PARAMETER_ERROR;
        }

        if ( address ) {
            snprintf( buf, sizeof( buf ), "%s", address );
            p = strrchr( buf, '.' );
            if ( p ) {
                *p = '\0';
            }

            snprintf( dhcp_s, sizeof( dhcp_s ), "%s.%d", buf, dhcp_start );
            dhcp_start = utils_addr_to_int( dhcp_s );

            snprintf( dhcp_e, sizeof( dhcp_e ), "%s.%d", buf, dhcp_end );
            dhcp_end = utils_addr_to_int( dhcp_e );

            cfmanager_log_message( L_DEBUG, "dhcp_s %s; dhcp_e %s\n", dhcp_s, dhcp_e );

            /*
             * At present, the broadcast address is not checked
             * ex:
             * ipaddress 192.168.80.1 netmask 255.255.255.248
             * dhcp range 192.168.80.1 ~ 192.168.80.6
             * broadcast 192.168.80.7
             * 192.168.80.7 is still legal
             *
             * This does not affect usage, because no broadcast address is assigned in dnsmasq
             */

            if ( ( addr < dhcp_start ) || ( addr > dhcp_end ) )   {
                if ( max && ( dhcp_end > max || addr > max ) ) {
                    cfmanager_log_message( L_ERR, "dhcp pool end is not less than the max\n");
                    return ERRCODE_PARAMETER_ERROR;
                }
            }
            else {
                cfmanager_log_message( L_ERR, "IPv4 static address is within DHCP range\n");
                return ERRCODE_PARAMETER_ERROR;
            }
        }
    }

    return ERRCODE_SUCCESS;
}

//=============================================================================
void
sgreq_set_lan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__LAN_MAX];
    struct blob_attr *cm_tb[__LAN_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int error_state = 0;
    int i = 0;
    int flag = 0;
    int firewall_flag = 0;
    int mwan3_flag = 0;

    const char *address = NULL;
    const char *netmask = NULL;
    const char *iprange = NULL;

    blobmsg_parse( lan_policy,
        __LAN_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    cm_cfg = util_get_vltree_node( &cm_lan_vltree, VLTREE_CM_TREE, "lan" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No lan information found in %s\n", CFMANAGER_CONFIG_NAME );
        goto return_value;
    }

    blobmsg_parse( lan_policy,
        __LAN_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

    if ( tb[LAN_IP4ADDRESS] )
        address = blobmsg_get_string( tb[LAN_IP4ADDRESS] );

    if ( tb[LAN_NETMASK] )
        netmask = blobmsg_get_string( tb[LAN_NETMASK] );

    if ( tb[LAN_DHCP_IPRANGE] )
        iprange = blobmsg_get_string( tb[LAN_DHCP_IPRANGE] );

    error_state = sgreq_check_ipaddr( address, netmask, iprange );
    if ( error_state != ERRCODE_SUCCESS )
        goto return_value;


    for( i = 0; i < __LAN_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], lan_policy[i].type ) ) {
            continue;
        }

        snprintf( path, sizeof( path ),
            "%s.lan.%s", CFMANAGER_CONFIG_NAME, lan_policy[i].name );

        switch( i ) {
            case LAN_BINDIP:
                //Delete all del data
                sgreq_flush_bind_ip( cm_tb[i], path );
                sgreq_set_bind_ip( tb[i], path );
                break;
            case LAN_DHCP_ENABLE:
                firewall_flag = 1;
            default:
                config_set_by_blob( tb[i], path, lan_policy[i].type );
                break;
        }

        flag = 1;
    }

    if( !flag ) {
        cfmanager_log_message( L_DEBUG, "The lan config is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_LAN, false );

    if( firewall_flag ) {
        cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
    }

    if( mwan3_flag ) {
        cfparse_load_file( "mwan3", LOAD_ALL_SECTION, false );
    }

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void
sgreq_set_vlan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__VLAN_MAX];
    struct blob_attr *cm_tb[__VLAN_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
//    char value[LOOKUP_STR_SIZE] = { 0 };
    char session_name[BUF_LEN_64] = {0};
    int vlanid = 0;
    int error_state = 0;
    int i = 0;
    bool need_update = false;
    bool need_update_wireless = false;
    int firewall_flag = 0;
    int mwan3_flag = 0;
    int action = 0;

    const char *address = NULL;
    const char *netmask = NULL;
    const char *iprange = NULL;

    blobmsg_parse( vlan_policy,
        __VLAN_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[VLAN_ACTION] ) {
        cfmanager_log_message( L_ERR, "No action" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }
    action = atoi( blobmsg_get_string( tb[VLAN_ACTION] ) );

    /* get vlanId as key */
    if ( !tb[VLAN_ID] ) {
        cfmanager_log_message( L_ERR, "No vlanId" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    vlanid = atoi( blobmsg_get_string( tb[VLAN_ID] ) );
    if ( vlanid < 1 || vlanid > 4094 ) {
        cfmanager_log_message( L_ERR, "vlanId is %d", vlanid );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }
    snprintf( session_name, BUF_LEN_64, "vlan%d", vlanid );

    if ( CONFIG_DEL == action ) {
        config_del_named_section( CFMANAGER_CONFIG_NAME, "vlan", session_name );
        need_update = true;
        need_update_wireless = true;
        firewall_flag = 1;
    }
    else if ( CONFIG_SET == action || CONFIG_EDIT == action ) {
        if ( tb[VLAN_IP4ADDRESS] )
            address = blobmsg_get_string( tb[VLAN_IP4ADDRESS] );

        if ( tb[VLAN_NETMASK] )
            netmask = blobmsg_get_string( tb[VLAN_NETMASK] );

        if ( tb[VLAN_DHCP_IPRANGE] )
            iprange = blobmsg_get_string( tb[VLAN_DHCP_IPRANGE] );

        error_state = sgreq_check_ipaddr( address, netmask, iprange );
        if ( error_state != ERRCODE_SUCCESS )
            goto return_value;


        memset( cm_tb, 0, __VLAN_MAX * sizeof(*cm_tb) );
        cm_cfg = util_get_vltree_node( &cm_vlan_vltree, VLTREE_CM_TREE, session_name );
        if( !cm_cfg ) {
            cfmanager_log_message( L_DEBUG, "No %s found in %s, we will add it", session_name, CFMANAGER_CONFIG_NAME );
            error_state = config_add_named_section( CFMANAGER_CONFIG_NAME, "vlan", session_name );
            if ( error_state != 0 ) {
                cfmanager_log_message( L_ERR, "failed to add section vlan.%s in %s\n", session_name, CFMANAGER_CONFIG_NAME );
                error_state = ERRCODE_INTERNAL_ERROR;
                goto return_value;
            }

            need_update = true;
        }
        else {
            blobmsg_parse( vlan_policy,
                __VLAN_MAX,
                cm_tb,
                blob_data( cm_cfg->cf_section.config ),
                blob_len( cm_cfg->cf_section.config  ) );
        }

        for( i = VLAN_ID; i < __VLAN_MAX; i++ ) {
            if( !tb[i] ) {
                continue;
            }

            if( sgreq_compar_attr( tb[i], cm_tb[i], vlan_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ),
                "%s.%s.%s", CFMANAGER_CONFIG_NAME, session_name, vlan_policy[i].name );

            switch( i ) {
                case VLAN_DHCP_ENABLE:
                    firewall_flag = 1;
                default:
                    config_set_by_blob( tb[i], path, vlan_policy[i].type );
                    break;
            }

            need_update = true;
        }
    }


    if( !need_update ) {
        cfmanager_log_message( L_DEBUG, "The vlan config is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_VLAN, false );

    if( firewall_flag ) {
        cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
    }

    if( mwan3_flag ) {
        cfparse_load_file( "mwan3", LOAD_ALL_SECTION, false );
    }

    if( need_update_wireless ) {
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_WIRELESS, false );
    }

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
static void
sgreq_set_switch_port(
    struct ubus_context *ctx,
    struct blob_attr *attr,
    int *update
)
//=============================================================================
{
    struct blob_attr *tb[__SWITCH_PORT_MAX];
    struct blob_attr *cm_tb[__SWITCH_PORT_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    const char *session_type = "switch_port";
    char session_name[BUF_LEN_8] = {0};
    int error_state = 0;
    int i = 0;

    *update = 0;
    blobmsg_parse( switch_port_policy,
        __SWITCH_PORT_MAX,
        tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    /* get portId as key */
    if ( !tb[SWITCH_PORT_PORT_ID] ) {
        cfmanager_log_message( L_ERR, "No portId" );
        return;
    }
    snprintf( session_name, BUF_LEN_8, "port%s", blobmsg_get_string( tb[SWITCH_PORT_PORT_ID] ) );

    memset( cm_tb, 0, __SWITCH_PORT_MAX * sizeof(*cm_tb) );
    cm_cfg = util_get_vltree_node( &cm_switch_port_vltree, VLTREE_CM_TREE, session_name );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No %s found in %s, we will add it", session_name, CFMANAGER_CONFIG_NAME );
        error_state = config_add_named_section( CFMANAGER_CONFIG_NAME, session_type, session_name );
        if ( error_state != 0 ) {
            cfmanager_log_message( L_ERR, "failed to add section %s in %s\n", session_name, CFMANAGER_CONFIG_NAME );
            error_state = ERRCODE_INTERNAL_ERROR;
            return;
        }

        *update = 1;
    }
    else {
        blobmsg_parse( switch_port_policy,
            __SWITCH_PORT_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );
    }

    for( i = 0; i < __SWITCH_PORT_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], switch_port_policy[i].type ) ) {
            continue;
        }

        snprintf( path, sizeof( path ),
            "%s.%s.%s", CFMANAGER_CONFIG_NAME, session_name, switch_port_policy[i].name );
        config_set_by_blob( tb[i], path, switch_port_policy[i].type );

        *update = 1;
    }
}

//=============================================================================
void
sgreq_set_switch_ports(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__SWITCH_PORT_ATTR_MAX];
    int error_state = ERRCODE_SUCCESS;
    struct blob_attr *cur;
    unsigned int rem;
    bool need_update = false;
    int need_network_update = 0;

    blobmsg_parse( switch_port_attrs_policy,
        __SWITCH_PORT_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[SWITCH_PORT_ATTR_PORTS] ||
            blobmsg_type(tb[SWITCH_PORT_ATTR_PORTS]) != BLOBMSG_TYPE_ARRAY ) {
        cfmanager_log_message( L_ERR,
            "Missing ports in json or type is not blob array!\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    blobmsg_for_each_attr( cur, tb[SWITCH_PORT_ATTR_PORTS], rem ) {
        sgreq_set_switch_port( ctx, cur, &need_network_update );

        if ( need_network_update ) {
            need_update = true;
        }
    }

    if ( need_update ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_SWITCH_PORT, false );
    }


return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
static int
sgreq_validate_ssid_name_legal(
    struct blob_attr *attr
)
//=============================================================================
{
    if( !attr ) {
        return ERRCODE_SUCCESS;
    }

    return util_validate_legal( attr, CEK_SSID_NAME, ERRCODE_SSID_NAME_TOO_LONG );
}

//=============================================================================
void
sgreq_set_wireless(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__WIRELESS_MAX];
    struct blob_attr *cm_tb[__WIRELESS_MAX];
    struct cm_config *cm_cfg = NULL;
    static struct blob_buf radio_buf;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char ssid_name2g[SSID_NAME_MAX_LEN+1] = { 0 };
    char ssid_name5g[SSID_NAME_MAX_LEN+1] = { 0 };
    int error_state = 0;
    int i = 0;
    bool wireless_change = false;
    bool radio_change = false;
    bool merge_radio = false;

    blobmsg_parse( wireless_policy,
        __WIRELESS_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    blob_buf_init( &radio_buf, 0 );

    cm_cfg = util_get_vltree_node( &cm_wireless_vltree, VLTREE_CM_TREE, "wireless" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No wireless information found in %s\n", CFMANAGER_CONFIG_NAME );
        goto return_value;
    }

    blobmsg_parse( wireless_policy,
        __WIRELESS_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

    /* Check the validity of SSID name.
     * The length is not allowed to exceed 32 bytes.
     * The page cannot judge whether it contains Chinese
    */
    error_state = sgreq_validate_ssid_name_legal( tb[WIRELESS_2G4SSIDNAME] );
    if( ERRCODE_SUCCESS != error_state ) {
        cfmanager_log_message( L_ERR, "2g ssid name is too long\n" );
        goto return_value;
    }

    error_state = sgreq_validate_ssid_name_legal( tb[WIRELESS_5GSSIDNAME] );
    if( ERRCODE_SUCCESS != error_state ) {
        cfmanager_log_message( L_ERR, "5g ssid name is too long\n" );
        goto return_value;
    }

    for( i = 0; i < __WIRELESS_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        switch( i ) {
            case WIRELESS_5GSSIDNAME:
                merge_radio = util_blobmsg_get_bool( tb[WIRELESS_MERGERADIOENABL],
                    util_blobmsg_get_bool( cm_tb[WIRELESS_MERGERADIOENABL], false ) );

                if( !merge_radio ) {
                    strncpy( ssid_name2g,
                            util_blobmsg_get_string( tb[WIRELESS_2G4SSIDNAME],  "" ),
                                sizeof( ssid_name2g ) -1 );
                    strncpy( ssid_name5g, util_blobmsg_get_string( tb[i],  "" ),
                            sizeof( ssid_name5g ) -1 );
                    if( 0 == strcmp( ssid_name2g, ssid_name5g ) ) {
                        error_state = ERRCODE_MESSAGE_WRONG_FROMAT;
                        cfmanager_log_message( L_ERR, "the ssid name is the same\n" );
                        goto return_value;
                    }
                }
                break;
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
                blobmsg_add_blob( &radio_buf, tb[i] );
                radio_change = true;
                break;
            default:
                break;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], wireless_policy[i].type ) ) {
            continue;
        }

        sprintf( path, "%s.wireless.%s", CFMANAGER_CONFIG_NAME, wireless_policy[i].name );
        config_set_by_blob( tb[i], path, wireless_policy[i].type );

        wireless_change = true;
    }

    if( wireless_change ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_WIRELESS, false );
    }

    if( radio_change ) {
        sgreq_set_radio_by_blob( radio_buf.head, RADIO_TYPE_HOME );
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_RADIO, false );
    }

return_value:

    blob_buf_free( &radio_buf );
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
void
sgreq_set_tr069(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    int i = 0;
    bool config_change = false;
    struct blob_attr *tb[__CM_TR069_MAX];
    struct blob_attr *cm_tb[__CM_TR069_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE];

    blobmsg_parse( cm_tr069_policy,
        __CM_TR069_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    cm_cfg = util_get_vltree_node( &cm_tr069_vltree, VLTREE_CM_TREE, "tr069" );
    if( cm_cfg ) {
        blobmsg_parse( cm_tr069_policy,
            __CM_TR069_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );
    }
    else {
        error_state = config_add_named_section( CFMANAGER_CONFIG_NAME,
            "tr069", "tr069" );
        if( error_state < 0 ) {
            cfmanager_log_message( L_ERR, "add cfmanager section 'tr069' failed\n" );
            error_state = ERRCODE_INTERNAL_ERROR;
            goto return_value;
        }

        memset( cm_tb, 0, __CM_TR069_MAX * sizeof(*cm_tb) );
    }

    for( i = 0; i < __CM_TR069_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], cm_tr069_policy[i].type ) ) {
            continue;
        }

        snprintf( path, sizeof( path ), "%s.tr069.%s",
            CFMANAGER_CONFIG_NAME, cm_tr069_policy[i].name );
        config_set_by_blob( tb[i], path, cm_tr069_policy[i].type );

        config_change = true;
    }

    if( config_change ) {
        config_commit( CFMANAGER_CONFIG_NAME, 0 );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_TR069, false );
    }

return_value:

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
static void
sgreq_del_ap_ssid_id(
    char *ssid_id
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    char ssids[BUF_LEN_128] = { 0 };

    if( !ssid_id ) {
        return;
    }

    vlist_for_each_element( &cm_ap_vltree, cm_cfg, node ) {

        memset( ssids, 0, sizeof( ssids ) );
        config_get_cm_ap_ssids( cm_cfg->cf_section.name, ssids, sizeof( ssids ) );

        if( util_match_ssids( ssids, ssid_id )  ) {
            config_del_dev_ssid_id( cm_cfg->cf_section.name, ssid_id );
        }
    }
}

//=============================================================================
void
sgreq_set_additional_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__WIRELESS_MAX];
    struct blob_attr *cm_tb[__WIRELESS_MAX];
    struct cm_config *cm_cfg = NULL;
    char section_name[BUF_LEN_32] = { 0 };
    char *section_type = "additional_ssid";
    char *action = NULL;
    char path[LOOKUP_STR_SIZE];
    char *value = NULL;
    int error_state = ERRCODE_SUCCESS;
    int i = 0;
    bool config_change = false;
    bool need_reload_ap = false;

    blobmsg_parse( cm_addit_ssid_policy,
        __CM_ADDIT_SSID_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if( !tb[CM_ADDIT_SSID_ID] || !tb[CM_ADDIT_SSID_ACTION] ) {
        cfmanager_log_message( L_ERR, "miss additional ssid id or action\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    strncpy( section_name, blobmsg_get_string( tb[CM_ADDIT_SSID_ID] ), sizeof( section_name ) -1 );
    action = blobmsg_get_string( tb[CM_ADDIT_SSID_ACTION] );
    if( 0 == strcmp( "1", action ) ) {
        error_state = config_del_named_section( CFMANAGER_CONFIG_NAME, section_type, section_name );
        if( 0 > error_state ) {
            error_state = ERRCODE_INTERNAL_ERROR;
            cfmanager_log_message( L_ERR, "delete section '%s' failed\n", section_name );
        }
        else {
            sgreq_del_ap_ssid_id( section_name );
            error_state = ERRCODE_SUCCESS;
            config_change = true;
            need_reload_ap = true;
        }

        goto return_value;
    }

    memset( cm_tb, 0, __WIRELESS_MAX * sizeof( *cm_tb ) );
    cm_cfg = util_get_vltree_node( &cm_addit_ssid_vltree, VLTREE_CM_TREE, section_name );
    if( !cm_cfg ) {
        error_state = config_add_named_section( CFMANAGER_CONFIG_NAME, section_type, section_name );
        if( 0 > error_state ) {
            cfmanager_log_message( L_ERR, "add section '%s' failed\n", section_name );
            error_state = ERRCODE_INTERNAL_ERROR;
            goto return_value;
        }
    }
    else {
        blobmsg_parse( cm_addit_ssid_policy,
            __CM_ADDIT_SSID_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );
    }

    for( ; i < __CM_ADDIT_SSID_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], cm_addit_ssid_policy[i].type ) ) {
            continue;
        }

        switch( i ) {
            case CM_ADDIT_SSID_BMS:
                value = blobmsg_get_string( tb[i] );
                if( 0 == strcmp( value, "2" ) ) {
                    snprintf( path, sizeof( path ), "%s.%s.%s",
                        CFMANAGER_CONFIG_NAME, section_name, cm_addit_ssid_policy[CM_ADDIT_SSID_PROXY_ARP].name );
                    config_uci_set( path, "1", 0 );
                }
                break;
            default:
                break;
        }

        snprintf( path, sizeof( path ),
            "%s.%s.%s", CFMANAGER_CONFIG_NAME, section_name, cm_addit_ssid_policy[i].name );
        config_set_by_blob( tb[i], path, cm_addit_ssid_policy[i].type );

        config_change = true;
    }

return_value:

    if( config_change ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );

        if( need_reload_ap ) {
            config_commit( CFMANAGER_CONFIG_NAME, false );
        }
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_ADDIT_SSID, false );
    }

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
static int
sgreq_set_radio_by_blob(
    struct blob_attr *data,
    int type
)
//=============================================================================
{
    struct blob_attr *tb[__CM_RADIO_MAX];
    struct blob_attr *cm_tb[__CM_RADIO_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE];
    int error_state = ERRCODE_SUCCESS;
    int i = 0;
    const char *section_name = "radio";   //The same applies to section type

    if( RADIO_TYPE_HOME == type ) {
        blobmsg_parse( cm_radio_policy,
            __CM_RADIO_MAX,
            tb,
            blob_data( data ),
            blob_len( data ) );
    }
    else {
        blobmsg_parse( cm_radio_policy,
            __CM_RADIO_MAX,
            tb,
            blobmsg_data( data ),
            blobmsg_len( data ) );
    }

    memset( cm_tb, 0, __CM_RADIO_MAX * sizeof( *cm_tb ) );
    cm_cfg = util_get_vltree_node( &cm_radio_vltree, VLTREE_CM_TREE, section_name );
    if( !cm_cfg ) {
        error_state = config_add_named_section( CFMANAGER_CONFIG_NAME, section_name, section_name );
        if( 0 > error_state ) {
            cfmanager_log_message( L_ERR, "add section '%s' failed\n", section_name );
            error_state = ERRCODE_INTERNAL_ERROR;
        }
    }
    else {
        blobmsg_parse( cm_radio_policy,
            __CM_RADIO_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );
    }

    if( ERRCODE_SUCCESS == error_state ) {
        for( ; i < __CM_RADIO_MAX; i++ ) {
            if( !tb[i] ) {
                continue;
            }

            if( sgreq_compar_attr( tb[i], cm_tb[i], cm_radio_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, section_name, cm_radio_policy[i].name );
            config_set_by_blob( tb[i], path, cm_radio_policy[i].type );
        }
    }

    cfmanager_log_message( L_DEBUG, "error_state:%d\n", error_state );
    return error_state;
}

//=============================================================================
void
sgreq_set_radio(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = ERRCODE_SUCCESS;

    error_state = sgreq_set_radio_by_blob( data, RADIO_TYPE_ENTERPRISE );

    if( ERRCODE_SUCCESS == error_state ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_RADIO, false );
    }

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
void
sgreq_set_firewall_dos(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__FIREWALL_DOS_MAX];
    struct blob_attr *cm_tb[__FIREWALL_DOS_MAX];
    struct cm_config *cm_cfg = NULL;

    char path[LOOKUP_STR_SIZE] = { 0 };
    int error_state = 0;
    int i = 0;
    int flag = 0;

    blobmsg_parse( firewall_dos_policy,
        __FIREWALL_DOS_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    cm_cfg = util_get_vltree_node( &cm_firewall_dos_vltree, VLTREE_CM_TREE, "dos_def" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No dos_def information found in %s\n", CFMANAGER_CONFIG_NAME );
        goto return_value;
    }

    blobmsg_parse( firewall_dos_policy,
        __FIREWALL_DOS_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

    for( i = 0; i < __FIREWALL_DOS_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], firewall_dos_policy[i].type ) ) {
            continue;
        }

        snprintf( path, sizeof(path),"%s.dos_def.%s", CFMANAGER_CONFIG_NAME, firewall_dos_policy[i].name );
        config_set_by_blob( tb[i], path, firewall_dos_policy[i].type );

        flag = 1;
    }

    if( !flag ) {
        cfmanager_log_message( L_DEBUG, "The dos_dof config is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_FIREWALL_DOS, false );
    cfparse_load_file( FIREWALL_CONFIG_NAME, LOAD_ALL_SECTION, false );

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
void
sgreq_set_ipv6_static_route(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_attr *tb[__STATIC_ROUTE_IPV6_ATTR_MAX];
    struct blob_attr *cur;
    int rem;

    blobmsg_parse( static_route_ipv6_attrs_policy,
        __STATIC_ROUTE_IPV6_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if( !tb[STATIC_ROUTE_IPV6_ATTR_RULE] ||
        blobmsg_type(tb[STATIC_ROUTE_IPV6_ATTR_RULE]) != BLOBMSG_TYPE_ARRAY ) {

        cfmanager_log_message( L_ERR, "Wrong message type\n" );
        error_state = ERRCODE_MESSAGE_WRONG_FROMAT;
    }

    if( ERRCODE_SUCCESS == error_state ) {
        blobmsg_for_each_attr( cur, tb[STATIC_ROUTE_IPV6_ATTR_RULE], rem ) {
            error_state = route_parse_ipv6_static_route( cur );
            if ( ERRCODE_SUCCESS != error_state ) {
                cfmanager_log_message( L_ERR, "set static route ipv6 failed error_state:%d\n", error_state );
                break;
            }
        }
    }

    if( ERRCODE_SUCCESS == error_state ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_STATIC_ROUTE_IPV6, false );
    }

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;

}

//=============================================================================
void
sgreq_set_hostname(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    int i = 0;
    char section_name[BUF_LEN_32];
    char *mac = NULL;
    char path[LOOKUP_STR_SIZE];
    bool config_change = false;
    struct blob_attr *tb[__HOSTNAME_MAX];
    struct blob_attr *cm_tb[__HOSTNAME_MAX];
    struct cm_config *cm_cfg = NULL;

    blobmsg_parse( hostname_policy,
        __HOSTNAME_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if( !tb[HOSTNAME_NAME] || !tb[HOSTNAME_MAC] ) {
        error_state = ERRCODE_MESSAGE_WRONG_FROMAT;
        cfmanager_log_message( L_ERR, "lack 'name' or 'mac'\n" );
    }

    mac = blobmsg_get_string( tb[HOSTNAME_MAC] );
    snprintf( section_name, sizeof( section_name ), "hn%s", mac );

    cm_cfg = util_get_vltree_node( &cm_hostname_vltree, VLTREE_CM_TREE, section_name );
    if( !cm_cfg ) {
        error_state = config_add_named_section( CFMANAGER_CONFIG_NAME, "hostname", section_name );
        if( error_state < 0 ) {
            cfmanager_log_message( L_ERR, "add section '%s' failed\n", section_name );
            goto return_value;
        }

        memset( cm_tb, 0, __HOSTNAME_MAX * sizeof(*cm_tb) );
    }
    else {
        blobmsg_parse( hostname_policy,
            __HOSTNAME_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );
    }

    for( i = 0; i < __HOSTNAME_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], hostname_policy[i].type ) ) {
            continue;
        }

        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, section_name, hostname_policy[i].name );
        config_set_by_blob( tb[i], path, hostname_policy[i].type );

        config_change = true;
    }

    if( config_change ) {
        config_commit( CFMANAGER_CONFIG_NAME, 0 );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_HOSTNAME, false );
    }

return_value:

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
static int
parse_client_access(
    struct blob_attr *attr
)
//=============================================================================
{
    enum {
        PARSE_ACCESS_MAC,
        PARSE_ACCESS_HOSTNAME,
        PARSE_ACCESS_CLIENTOS,
        PARSE_ACCESS_WHITEBLACK,
        PARSE_ACCESS_SURFINGINTERNET
    };
    static const struct blobmsg_policy parse_access_policy[] = {
        [PARSE_ACCESS_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
        [PARSE_ACCESS_HOSTNAME]  = { .name = "hostName",      .type = BLOBMSG_TYPE_STRING },
        [PARSE_ACCESS_CLIENTOS]  = { .name = "clientOs",      .type = BLOBMSG_TYPE_STRING },
        [PARSE_ACCESS_WHITEBLACK]  = { .name = "whiteBlack",  .type = BLOBMSG_TYPE_STRING },
        [PARSE_ACCESS_SURFINGINTERNET]   = { .name = "surfingInternet",  .type = BLOBMSG_TYPE_STRING }
    };
    enum { __PARSE_ACCESS_MAX = ( sizeof( parse_access_policy ) / sizeof( parse_access_policy[0] ) ) };

    enum {
        WHITELIST_ADD = 1,
        WHITELIST_DEL,
        BLACKLIST_ADD,
        BLACKLIST_DEL,
    };

    enum {
        SURFING_INTERNET_ALLOW,
        SURFING_INTERNET_DENY
    };

    struct blob_attr *tb[__PARSE_ACCESS_MAX];
    struct blob_attr *cur;

    const char *mac = NULL;
    char format_mac[MAC_STR_MAX_LEN+1] = { 0 };
    const char *hostName = NULL;
    const char *clientOs = NULL;
    int whiteBlack = 0;
    int surfingInternet = 0;

    char path[LOOKUP_STR_SIZE];
    struct cm_config *cm_cfg = NULL;

    int ret = 0;

    if ( blobmsg_type(attr) != BLOBMSG_TYPE_TABLE )
        return ERRCODE_PARAMETER_ERROR;

    blobmsg_parse( parse_access_policy,
                   __PARSE_ACCESS_MAX,
                   tb,
                   blobmsg_data( attr ),
                   blobmsg_data_len( attr ) );

    cur = tb[PARSE_ACCESS_MAC];
    if ( !cur )
        return ERRCODE_PARAMETER_ERROR;

    mac = blobmsg_get_string( cur );

    cur = tb[PARSE_ACCESS_HOSTNAME];
    if ( cur )
        hostName = blobmsg_get_string( cur );

    cur = tb[PARSE_ACCESS_CLIENTOS];
    if ( cur )
        clientOs = blobmsg_get_string( cur );

    cur = tb[PARSE_ACCESS_WHITEBLACK];
    whiteBlack = util_blobmsg_get_int( cur, 0 );

    cur = tb[PARSE_ACCESS_SURFINGINTERNET];
    surfingInternet = util_blobmsg_get_int( cur, -1 );

    cfmanager_log_message( L_DEBUG,
        "mac %s, whiteBlack %d, surfingInternet %d \n", mac, whiteBlack, surfingInternet  );

    cm_cfg = util_get_vltree_node( &cm_access_vltree, VLTREE_CM_TREE, mac );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "No access information found in %s\n", CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "access", mac );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
             "failed to add section %s in %s\n", mac, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

    }
    else {
        cfmanager_log_message( L_DEBUG,
            "Found same access information(%s) in %s\n", mac, CFMANAGER_CONFIG_NAME );
    }

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, mac, cm_access_parse_policy[CM_PARSE_ACCESS_MAC].name );

    util_formatted_mac_with_colo( mac, format_mac );
    config_uci_set( path, format_mac, 0 );

    if ( hostName && hostName[0] != '\0' ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, mac, cm_access_parse_policy[CM_PARSE_ACCESS_HOSTNAME].name );
        config_uci_set( path, (char *)hostName, 0 );
    }

    if ( clientOs && clientOs[0] != '\0' ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, mac, cm_access_parse_policy[CM_PARSE_ACCESS_CLIENTOS].name );
        config_uci_set( path, (char *)clientOs, 0 );
    }

    switch( whiteBlack ) {
        case WHITELIST_ADD:
            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, mac, cm_access_parse_policy[CM_PARSE_ACCESS_WHITE].name );
            config_uci_set( path, "1", 0 );
        case WHITELIST_DEL:
            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, mac, cm_access_parse_policy[CM_PARSE_ACCESS_WHITE].name );
            config_uci_set( path, "0", 0 );
            break;
        case BLACKLIST_ADD:
            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, mac, cm_access_parse_policy[CM_PARSE_ACCESS_BLACK].name );
            config_uci_set( path, "1", 0 );
            break;
        case BLACKLIST_DEL:
            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, mac, cm_access_parse_policy[CM_PARSE_ACCESS_BLACK].name );
            config_uci_set( path, "0", 0 );
            break;
    }

    switch( surfingInternet ) {
        case SURFING_INTERNET_ALLOW:
            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, mac, cm_access_parse_policy[CM_PARSE_ACCESS_BLOCK].name );
            config_uci_set( path, "0", 0 );
            break;
        case SURFING_INTERNET_DENY:
            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, mac, cm_access_parse_policy[CM_PARSE_ACCESS_BLOCK].name );
            config_uci_set( path, "1", 0 );
            break;
    }

    return ERRCODE_SUCCESS;
}

//=============================================================================
void
sgreq_set_access(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__ACCESS_ATTR_MAX];

    unsigned int rem;
    struct blob_attr *cur;
    int ret = ERRCODE_SUCCESS;
    int err = ERRCODE_SUCCESS;

    blobmsg_parse( access_attrs_policy,
        __ACCESS_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[ACCESS_ATTR_CLIENT] ) {
        ret = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    blobmsg_for_each_attr( cur, tb[ACCESS_ATTR_CLIENT], rem ) {
        err = parse_client_access( cur );
        if ( err ) {
            ret = err;
            break;
        }
    }

    if ( ret == ERRCODE_SUCCESS ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_ACCESS, false );
    }

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, ret );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void
sgreq_set_global_access(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__GLOBAL_ACCESS_ATTR_MAX];
    struct blob_attr *cm_tb[__GLOBAL_ACCESS_ATTR_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_128] = { 0 };

    unsigned int rem;
    struct blob_attr *cur;
    int ret = ERRCODE_SUCCESS;

    bool changed = false;
    bool need_update = false;

    blobmsg_parse( global_access_attrs_policy,
        __GLOBAL_ACCESS_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[GLOBAL_ACCESS_ATTR_ENABLE] ) {
        ret = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    if ( !tb[GLOBAL_ACCESS_ATTR_FORBIDURL] ) {
        ret = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    cm_cfg = util_get_vltree_node( &cm_global_access_vltree, VLTREE_CM_TREE, "global_access" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "No global_access information found in %s vltree\n", CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "global_access", "global_access" );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
             "failed to add section global_access in %s\n", CFMANAGER_CONFIG_NAME );
            ret = ERRCODE_INTERNAL_ERROR;
            goto return_value;
        }

        need_update = true;
    }
    else {
        cfmanager_log_message( L_DEBUG,
            "Found same access information(global_access) in %s\n", CFMANAGER_CONFIG_NAME );

        blobmsg_parse( global_access_attrs_policy,
        __GLOBAL_ACCESS_ATTR_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

        if ( !blob_attr_equal( tb[GLOBAL_ACCESS_ATTR_ENABLE],
                              cm_tb[GLOBAL_ACCESS_ATTR_ENABLE] ) ) {
            changed = true;
        }

        if ( !blob_attr_equal( tb[GLOBAL_ACCESS_ATTR_FORBIDURL],
                              cm_tb[GLOBAL_ACCESS_ATTR_FORBIDURL] ) ) {
            changed = true;
        }

        if ( changed ) {
            config_del_named_section( CFMANAGER_CONFIG_NAME, "global_access", "global_access" );
            config_add_named_section( CFMANAGER_CONFIG_NAME, "global_access", "global_access" );

            need_update = true;
        }
    }

    if ( need_update ) {
        snprintf( path, sizeof( path ),
            "%s.global_access.%s", CFMANAGER_CONFIG_NAME,
            global_access_attrs_policy[GLOBAL_ACCESS_ATTR_ENABLE].name );
        snprintf( value, sizeof( value ),
            "%d", blobmsg_get_bool( tb[GLOBAL_ACCESS_ATTR_ENABLE] ) );
        config_uci_set( path, value, 0 );

        snprintf( path, sizeof( path ),
            "%s.global_access.%s", CFMANAGER_CONFIG_NAME,
            global_access_attrs_policy[GLOBAL_ACCESS_ATTR_FORBIDURL].name );

        blobmsg_for_each_attr( cur, tb[GLOBAL_ACCESS_ATTR_FORBIDURL], rem ) {
            const char *url = blobmsg_get_string( cur );
            cfmanager_log_message( L_DEBUG, "forbid url %s \n", url );
            config_uci_add_list( path, blobmsg_get_string( cur ), 0 );
        }

        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_GLOBAL_ACCESS, false );

        cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
    }
    else {
        cfmanager_log_message( L_DEBUG, "The global_access config is same\n" );
    }

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, ret );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void
sgreq_set_schedule_access(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__SCHEDULE_ACCESS_ATTR_MAX];
    struct blob_attr *cm_tb[__SCHEDULE_ACCESS_ATTR_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_32] = { 0 };

    int i = 0;
    int ret = ERRCODE_SUCCESS;

    bool changed = false;
    bool need_update = false;


    blobmsg_parse( schedule_access_attrs_policy,
        __SCHEDULE_ACCESS_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[SCHEDULE_ACCESS_ATTR_ENABLE]
        || !tb[SCHEDULE_ACCESS_ATTR_WEEK]
        || !tb[SCHEDULE_ACCESS_ATTR_TIMESTART]
        || !tb[SCHEDULE_ACCESS_ATTR_TIMEEND] ) {
        ret = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    cm_cfg = util_get_vltree_node( &cm_schedule_access_vltree,
                                   VLTREE_CM_TREE, "schedule_access" );
    if( !cm_cfg ) {
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME,
                                        "schedule_access",
                                        "schedule_access" );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
             "failed to add section schedule_access in %s\n", CFMANAGER_CONFIG_NAME );
            ret = ERRCODE_INTERNAL_ERROR;
            goto return_value;
        }

        need_update = true;
    }
    else {
        cfmanager_log_message( L_DEBUG,
            "Found same information(schedule_access) in %s\n", CFMANAGER_CONFIG_NAME );

        blobmsg_parse( schedule_access_attrs_policy,
        __SCHEDULE_ACCESS_ATTR_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

        for( i = 0; i < __SCHEDULE_ACCESS_ATTR_MAX; i++ ) {
            if( sgreq_compar_attr( tb[i], cm_tb[i], schedule_access_attrs_policy[i].type ) ) {
                continue;
            }

            changed = true;
        }
        if ( changed ) {
            config_del_named_section( CFMANAGER_CONFIG_NAME,
                "schedule_access", "schedule_access" );
            config_add_named_section( CFMANAGER_CONFIG_NAME,
                "schedule_access", "schedule_access" );

            need_update = true;
        }
    }

    if ( need_update ) {
        snprintf( path, sizeof( path ),
            "%s.schedule_access.%s", CFMANAGER_CONFIG_NAME,
            schedule_access_attrs_policy[SCHEDULE_ACCESS_ATTR_ENABLE].name );
        snprintf( value, sizeof( value ),
            "%d", blobmsg_get_bool( tb[SCHEDULE_ACCESS_ATTR_ENABLE] ) );
        config_uci_set( path, value, 0 );

        snprintf( path, sizeof( path ),
            "%s.schedule_access.%s", CFMANAGER_CONFIG_NAME,
            schedule_access_attrs_policy[SCHEDULE_ACCESS_ATTR_WEEK].name );
        config_uci_set( path, blobmsg_get_string( tb[SCHEDULE_ACCESS_ATTR_WEEK] ), 0 );

        snprintf( path, sizeof( path ),
            "%s.schedule_access.%s", CFMANAGER_CONFIG_NAME,
            schedule_access_attrs_policy[SCHEDULE_ACCESS_ATTR_TIMESTART].name );
        config_uci_set( path, blobmsg_get_string( tb[SCHEDULE_ACCESS_ATTR_TIMESTART] ), 0 );

        snprintf( path, sizeof( path ),
            "%s.schedule_access.%s", CFMANAGER_CONFIG_NAME,
            schedule_access_attrs_policy[SCHEDULE_ACCESS_ATTR_TIMEEND].name );
        config_uci_set( path, blobmsg_get_string( tb[SCHEDULE_ACCESS_ATTR_TIMEEND] ), 0 );


        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_SCHEDULE_ACCESS, false );
    }
    else {
        cfmanager_log_message( L_DEBUG, "The schedule_access config is same\n" );
    }

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, ret );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_set_guest_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__GUEST_SSID_MAX];
    struct blob_attr *cm_tb[__GUEST_SSID_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int error_state = 0;
    int i = 0;
    int flag = 0;

    blobmsg_parse( guest_ssid_policy,
        __GUEST_SSID_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    cm_cfg = util_get_vltree_node( &cm_guest_ssid_vltree, VLTREE_CM_TREE, "guest_ssid" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No upgrade_auto information found in %s\n", CFMANAGER_CONFIG_NAME );
        goto return_value;
    }

    blobmsg_parse( guest_ssid_policy,
        __GUEST_SSID_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

    /* Check the validity of SSID name.
     * The length is not allowed to exceed 32 bytes.
     * The page cannot judge whether it contains Chinese
    */
    error_state = sgreq_validate_ssid_name_legal( tb[GUEST_SSID_SSIDNAME] );
    if( ERRCODE_SUCCESS != error_state ) {
        cfmanager_log_message( L_ERR, "ssid name is too long\n" );
        goto return_value;
    }

    for( i = 0; i < __GUEST_SSID_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], guest_ssid_policy[i].type ) ) {
            continue;
        }

        snprintf( path, sizeof( path ),
            "%s.guest_ssid.%s", CFMANAGER_CONFIG_NAME, guest_ssid_policy[i].name );
        config_set_by_blob( tb[i], path, guest_ssid_policy[i].type );

        flag = 1;
    }

    if( !flag ) {
        cfmanager_log_message( L_DEBUG, "The guest_ssid config is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_GUEST_SSID, false );

return_value:

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
void
sgreq_set_mesh_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__MESH_SSID_ATTR_MAX];
    struct blob_attr *cm_tb[__MESH_SSID_ATTR_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int error_state = 0;
    int i = 0;
    int flag = 0;

    blobmsg_parse( mesh_ssid_attrs_policy,
        __MESH_SSID_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    cm_cfg = util_get_vltree_node( &cm_mesh_ssid_vltree, VLTREE_CM_TREE, "mesh_ssid" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No mesh_ssid information found in %s\n", CFMANAGER_CONFIG_NAME );
        goto return_value;
    }

    blobmsg_parse( mesh_ssid_attrs_policy,
        __MESH_SSID_ATTR_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

    for( i = 0; i < __MESH_SSID_ATTR_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( blob_attr_equal( tb[i], cm_tb[i] ) ) {
            continue;
        }

        sprintf( path, "%s.mesh_ssid.%s", CFMANAGER_CONFIG_NAME, mesh_ssid_attrs_policy[i].name );
        config_set_by_blob( tb[i], path, mesh_ssid_attrs_policy[i].type );

        flag = 1;
    }

    if( !flag ) {
        cfmanager_log_message( L_DEBUG, "func:%s The mesh_ssid config is same\n", __func__ );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfmanager_log_message( L_DEBUG, "%s.%d ", __func__, __LINE__ );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_MESH_SSID, false );

return_value:

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
void
sgreq_set_acl(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void
sgreq_set_qos(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
static int
parse_snmp_config(
    struct blob_attr *attr
)
//=============================================================================
{
    struct blob_attr *tb[__SNMP_CONFIG_MAX];
    struct blob_attr *cur;

    const char *snmp_config_name = "snmpd";

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

    struct cm_config *cm_cfg = NULL;

    int ret = ERRCODE_SUCCESS;

    if ( blobmsg_type(attr) != BLOBMSG_TYPE_TABLE )
        return ERRCODE_PARAMETER_ERROR;

    blobmsg_parse( snmp_config_policy,
                   __SNMP_CONFIG_MAX,
                   tb,
                   blobmsg_data( attr ),
                   blobmsg_data_len( attr ) );

    cur = tb[SNMP_CONFIG_ENABLE_V1_V2C];
    if ( cur ) {
        enable_v1_v2c = blobmsg_get_bool( cur );
    }

    cur = tb[SNMP_CONFIG_ENABLE_V3];
    if ( cur ) {
        enable_v3 = blobmsg_get_bool( cur );
    }

    cur = tb[SNMP_CONFIG_SYS_LOCATION];
    if ( cur ) {
        sysLocation = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_SYS_CONTACT];
    if ( cur ) {
        sysContact = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_SYS_NAME];
    if ( cur ) {
        sysName = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_RO_COMMUNITYV4];
    if ( cur ) {
        roCommunityV4 = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_RW_COMMUNITYV4];
    if ( cur ) {
        rwCommunityV4 = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_RO_COMMUNITYV6];
    if ( cur ) {
        roCommunityV6 = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_RW_COMMUNITYV6];
    if ( cur ) {
        rwCommunityV6 = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_TRAP_TYPE];
    if ( cur ) {
        trapType = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_TRAP_HOST];
    if ( cur ) {
        trapHost = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_TRAP_PORT];
    if ( cur ) {
        trapPort = blobmsg_get_string( cur );
    }

    cur = tb[SNMP_CONFIG_TRAP_COMMUNITY];
    if ( cur ) {
        trapCommunity = blobmsg_get_string( cur );
    }

    cm_cfg = util_get_vltree_node( &cm_snmp_config_vltree, VLTREE_CM_TREE, snmp_config_name );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "No %s information found in %s vltree\n",
            snmp_config_name , CFMANAGER_CONFIG_NAME );

        ret = config_add_named_section( CFMANAGER_CONFIG_NAME,
                snmp_config_name , snmp_config_name );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
             "failed to add section %s in %s\n",
             snmp_config_name, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }
    }
    else {
        cfmanager_log_message( L_DEBUG,
            "Found same %s information in %s\n", snmp_config_name, CFMANAGER_CONFIG_NAME );
    }

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, snmp_config_name, snmp_config_policy[SNMP_CONFIG_ENABLE_V1_V2C].name );

    if ( enable_v1_v2c )
        config_uci_set( path, "1", 0 );
    else
        config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, snmp_config_name, snmp_config_policy[SNMP_CONFIG_ENABLE_V3].name );

    if ( enable_v3 )
        config_uci_set( path, "1", 0 );
    else
        config_uci_set( path, "0", 0 );

    if ( sysLocation ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, snmp_config_name,
            snmp_config_policy[SNMP_CONFIG_SYS_LOCATION].name );

        config_uci_set( path, (char *)sysLocation, 0 );
    }

    if ( sysContact ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, snmp_config_name,
            snmp_config_policy[SNMP_CONFIG_SYS_CONTACT].name );

        config_uci_set( path, (char *)sysContact, 0 );
    }

    if ( sysName ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, snmp_config_name,
            snmp_config_policy[SNMP_CONFIG_SYS_NAME].name );

        config_uci_set( path, (char *)sysName, 0 );
    }

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, snmp_config_name,
        snmp_config_policy[SNMP_CONFIG_RO_COMMUNITYV4].name );
 
    if ( roCommunityV4 ) {
        config_uci_set( path, (char *)roCommunityV4, 0 );
    }
    else {
        roCommunityV4 = "public";
        config_uci_set( path, "public", 0 );
    }

    if ( rwCommunityV4 ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, snmp_config_name,
            snmp_config_policy[SNMP_CONFIG_RW_COMMUNITYV4].name );

        config_uci_set( path, (char *)rwCommunityV4, 0 );
    }


    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, snmp_config_name,
        snmp_config_policy[SNMP_CONFIG_RO_COMMUNITYV6].name );
 
    if ( roCommunityV6 ) {
        config_uci_set( path, (char *)roCommunityV6, 0 );
    }
    else {
        roCommunityV6 = rwCommunityV4 ? rwCommunityV4 : "public";
        config_uci_set( path, (char *)roCommunityV6, 0 );
    }

    if ( rwCommunityV6 ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, snmp_config_name,
            snmp_config_policy[SNMP_CONFIG_RW_COMMUNITYV6].name );

        config_uci_set( path, (char *)rwCommunityV6, 0 );
    }


    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, snmp_config_name,
        snmp_config_policy[SNMP_CONFIG_TRAP_TYPE].name );
    if ( trapType ) {
        config_uci_set( path, (char *)trapType, 0 );
    }
    else {
        config_uci_set( path, "NONE", 0 );
    }

    if ( trapHost ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, snmp_config_name,
            snmp_config_policy[SNMP_CONFIG_TRAP_HOST].name );

        config_uci_set( path, (char *)trapHost, 0 );
    }

    if ( trapPort ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, snmp_config_name,
            snmp_config_policy[SNMP_CONFIG_TRAP_PORT].name );

        config_uci_set( path, (char *)trapPort, 0 );
    }


    if ( trapCommunity ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, snmp_config_name,
            snmp_config_policy[SNMP_CONFIG_TRAP_COMMUNITY].name );

        config_uci_set( path, (char *)trapCommunity, 0 );
    }

    return ERRCODE_SUCCESS;
}


//=============================================================================
static int
parse_snmp_ports(
    struct blob_attr *attr
)
//=============================================================================
{
    struct blob_attr *tb[__SNMP_PORTS_MAX];
    struct blob_attr *cur;

    const char *id = NULL;
    const char *port = NULL;
    const char *protocol = NULL;
    const char *Ip4Address = NULL;

    int port_num = 0;

    char path[LOOKUP_STR_SIZE];

    struct cm_config *cm_cfg = NULL;

    int ret = ERRCODE_SUCCESS;

    if ( blobmsg_type(attr) != BLOBMSG_TYPE_TABLE )
        return ERRCODE_PARAMETER_ERROR;

    blobmsg_parse( grandstream_snmp_ports_policy,
                   __SNMP_PORTS_MAX,
                   tb,
                   blobmsg_data( attr ),
                   blobmsg_data_len( attr ) );

    cur = tb[SNMP_PORTS_ID];
    if ( !cur )
        return ERRCODE_PARAMETER_ERROR;

    id = blobmsg_get_string( cur );

    if ( sscanf( id, "port%d", &port_num ) != 1 ) {
        return ERRCODE_PARAMETER_ERROR;
    }

    cur = tb[SNMP_PORTS_PORT];
    if ( cur )
        port = blobmsg_get_string( cur );

    cur = tb[SNMP_PORTS_PROTOCOL];
    if ( cur )
        protocol = blobmsg_get_string( cur );

    cur = tb[SNMP_PORTS_IPV4ADDRESS];
    if ( cur )
        Ip4Address = blobmsg_get_string( cur );

    cm_cfg = util_get_vltree_node( &cm_snmp_ports_vltree, VLTREE_CM_TREE, id );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "No %s information found in %s\n", id, CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "snmpd_ports", id );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
             "failed to add section %s in %s\n", id, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

    }
    else {
        cfmanager_log_message( L_DEBUG,
            "Found same snmpd_ports information(%s) in %s\n", id, CFMANAGER_CONFIG_NAME );
    }

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, id, grandstream_snmp_ports_policy[SNMP_PORTS_ID].name );

    config_uci_set( path, (char *)id, 0 );


    if ( port ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, id, grandstream_snmp_ports_policy[SNMP_PORTS_PORT].name );

        config_uci_set( path, (char *)port, 0 );
    }

    if ( protocol ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, id, grandstream_snmp_ports_policy[SNMP_PORTS_PROTOCOL].name );

        config_uci_set( path, (char *)protocol, 0 );
    }

    if ( Ip4Address ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, id, grandstream_snmp_ports_policy[SNMP_PORTS_IPV4ADDRESS].name );

        config_uci_set( path, (char *)Ip4Address, 0 );
    }

    return ERRCODE_SUCCESS;
}

//=============================================================================
static int
parse_snmp_v3_auth(
    struct blob_attr *attr
)
//=============================================================================
{
    struct blob_attr *tb[__SNMP_V3_MAX];
    struct blob_attr *cur;

    const char *id = NULL;
    const char *userName = NULL;
    const char *authType = NULL;
    const char *authPassPhrase = NULL;
    const char *privProto = NULL;
    const char *privPassPhrase = NULL;
    const char *accessCtrl = NULL;

    int user_num = 0;

    char path[LOOKUP_STR_SIZE];

    struct cm_config *cm_cfg = NULL;

    int ret = ERRCODE_SUCCESS;

    if ( blobmsg_type(attr) != BLOBMSG_TYPE_TABLE )
        return ERRCODE_PARAMETER_ERROR;

    blobmsg_parse( grandstream_snmp_v3_auth_policy,
                   __SNMP_V3_MAX,
                   tb,
                   blobmsg_data( attr ),
                   blobmsg_data_len( attr ) );

    cur = tb[SNMP_V3_ID];
    if ( !cur )
        return ERRCODE_PARAMETER_ERROR;

    id = blobmsg_get_string( cur );

    if ( sscanf( id, "user%d", &user_num ) != 1 ) {
        return ERRCODE_PARAMETER_ERROR;
    }

    cur = tb[SNMP_V3_NAME];
    if ( cur )
        userName = blobmsg_get_string( cur );

    cur = tb[SNMP_V3_AUTH_TYPE];
    if ( cur )
        authType = blobmsg_get_string( cur );

    cur = tb[SNMP_V3_AUTH_PASS_PHRASE];
    if ( cur )
        authPassPhrase = blobmsg_get_string( cur );

    cur = tb[SNMP_V3_PRIV_PROTO];
    if ( cur )
        privProto = blobmsg_get_string( cur );

    cur = tb[SNMP_V3_PRIV_PASS_PHRASE];
    if ( cur )
        privPassPhrase = blobmsg_get_string( cur );

    cur = tb[SNMP_V3_ACCESS_CTRL];
    if ( cur )
        accessCtrl = blobmsg_get_string( cur );

    cm_cfg = util_get_vltree_node( &cm_snmp_v3_auth_vltree, VLTREE_CM_TREE, id );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "No %s information found in %s\n", id, CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "snmpv3_auth", id );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
             "failed to add section %s in %s\n", id, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

    }
    else {
        cfmanager_log_message( L_DEBUG,
            "Found same snmpv3_auth information(%s) in %s\n", id, CFMANAGER_CONFIG_NAME );
    }

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, id, grandstream_snmp_v3_auth_policy[SNMP_V3_ID].name );

    config_uci_set( path, (char *)id, 0 );


    if ( userName ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, id, grandstream_snmp_v3_auth_policy[SNMP_V3_NAME].name );

        config_uci_set( path, (char *)userName, 0 );
    }

    if ( authType ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, id, grandstream_snmp_v3_auth_policy[SNMP_V3_AUTH_TYPE].name );

        config_uci_set( path, (char *)authType, 0 );
    }

    if ( authPassPhrase ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, id, grandstream_snmp_v3_auth_policy[SNMP_V3_AUTH_PASS_PHRASE].name );

        config_uci_set( path, (char *)authPassPhrase, 0 );
    }

    if ( privProto ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, id, grandstream_snmp_v3_auth_policy[SNMP_V3_PRIV_PROTO].name );

        config_uci_set( path, (char *)privProto, 0 );
    }


    if ( privPassPhrase ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, id, grandstream_snmp_v3_auth_policy[SNMP_V3_PRIV_PASS_PHRASE].name );

        config_uci_set( path, (char *)privPassPhrase, 0 );
    }

    if ( accessCtrl ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, id, grandstream_snmp_v3_auth_policy[SNMP_V3_ACCESS_CTRL].name );

        config_uci_set( path, (char *)accessCtrl, 0 );
    }
    return ERRCODE_SUCCESS;
}

//=============================================================================
void
sgreq_set_snmp(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__SNMP_MAX];

    static struct blob_buf snmp_ports;

    unsigned int rem;
    struct blob_attr *cur;
    
    int ret = ERRCODE_SUCCESS;
    int err = ERRCODE_SUCCESS;

    blobmsg_parse( snmp_policy,
        __SNMP_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[SNMP_CONFIG] ) {
        ret = ERRCODE_PARAMETER_ERROR;
        cfmanager_log_message( L_DEBUG, "Miss %s\n", snmp_policy[SNMP_CONFIG].name );
        goto return_value;
    }

    ret = parse_snmp_config( tb[SNMP_CONFIG] );
    if ( ret != ERRCODE_SUCCESS ) {
        ret = ERRCODE_PARAMETER_ERROR;
        cfmanager_log_message( L_DEBUG, "failed to parse %s\n", snmp_policy[SNMP_CONFIG].name );
        goto return_value;
    }
 
    if ( !tb[SNMP_PORTS] ) {
        void *a;
        void *t1;
        void *t2;
        struct blob_attr *tb1[__SNMP_MAX];
        
        blob_buf_init( &snmp_ports, 0 );
        a = blobmsg_open_array( &snmp_ports, "snmpPorts" );

        t1 = blobmsg_open_table( &snmp_ports, NULL );
        blobmsg_add_string( &snmp_ports, "id", "port0" );
        blobmsg_add_string( &snmp_ports, "protocol", "udp" );
        blobmsg_close_table( &snmp_ports, t1 );

        t2 = blobmsg_open_table( &snmp_ports, NULL );
        blobmsg_add_string( &snmp_ports, "id", "port1" );
        blobmsg_add_string( &snmp_ports, "protocol", "udpv6" );
        blobmsg_close_table( &snmp_ports, t2 );

        blobmsg_close_array( &snmp_ports, a );

        blobmsg_parse( snmp_policy,
            __SNMP_MAX,
            tb1,
            blobmsg_data( snmp_ports.head ),
            blobmsg_len( snmp_ports.head ) );

        blobmsg_for_each_attr( cur, tb1[SNMP_PORTS], rem ) {
            err = parse_snmp_ports( cur );
            if ( err ) {
                ret = err;
                break;
            }
        }
        blob_buf_free( &snmp_ports );
    }
    else {
        blobmsg_for_each_attr( cur, tb[SNMP_PORTS], rem ) {
            err = parse_snmp_ports( cur );
            if ( err ) {
                ret = err;
                break;
            }
        }
    }

    if ( ret != ERRCODE_SUCCESS ) {
        cfmanager_log_message( L_DEBUG, "failed to parse %s\n", snmp_policy[SNMP_PORTS].name );
        goto return_value;
    }


    blobmsg_for_each_attr( cur, tb[SNMP_V3_AUTH], rem ) {
        err = parse_snmp_v3_auth( cur );
        if ( err ) {
            cfmanager_log_message( L_DEBUG, "failed to parse %s\n", snmp_policy[SNMP_V3_AUTH].name );
            ret = err;
            break;
        }
    }

    if ( ret == ERRCODE_SUCCESS ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_SNMP_CONFIG, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_SNMP_PORTS, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_SNMP_V3_AUTH, false );
    }

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, ret );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void
sgreq_set_usb_share(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0, i = 0;
    char path[LOOKUP_STR_SIZE] = {0};
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__USB_SHARE_MAX];
    struct blob_attr *cm_tb[__USB_SHARE_MAX];

    blob_buf_init( &reply, 0 );

    blobmsg_parse( usb_share_policy,
                   __USB_SHARE_MAX,
                   tb,
                   blobmsg_data(data),
                   blobmsg_len(data) );

    cm_cfg = util_get_vltree_node( &cm_usb_share_vltree,
                                   VLTREE_CM_TREE,
                                   "usb_share" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG,
                               "No found usb share information in '%s'.\n",
                               CFMANAGER_CONFIG_NAME );
        goto return_value;
    }

    blobmsg_parse( usb_share_policy,
                   __USB_SHARE_MAX,
                   cm_tb,
                   blob_data(cm_cfg->cf_section.config),
                   blob_len(cm_cfg->cf_section.config) );

    for( i = 0; i < __USB_SHARE_MAX; i++ ) {
        if( !tb[i] || blob_attr_equal(tb[i],cm_tb[i]) ) {
            cfmanager_log_message( L_DEBUG,
                                   "'%s' option does not need to be set.\n",
                                   usb_share_policy[i].name );
            continue;
        }

        snprintf( path, sizeof(path),
                  "%s.usb_share.%s",
                  CFMANAGER_CONFIG_NAME,
                  usb_share_policy[i].name );
        config_set_by_blob( tb[i], path, usb_share_policy[i].type );
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_USB_SHARE, false );

return_value:
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
void
sgreq_set_static_router(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
void
sgreq_set_controller(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__CONTROLLER_MAX];
    struct blob_attr *cm_tb[__CONTROLLER_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
//    char value[BUF_LEN_128] = { 0 };
    int i = 0,error_state = 0;
    bool need_update = false;
    bool need_update_general = false;
    char key[BUF_LEN_64] = {0};

    blobmsg_parse( controller_policy,
        __CONTROLLER_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    memset( cm_tb, 0, __CONTROLLER_MAX * sizeof(*cm_tb) );
    cm_cfg = util_get_vltree_node( &cm_controller_vltree, VLTREE_CM_TREE, "main" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No controller found in %s, we will add it", CFMANAGER_CONFIG_NAME );
        error_state = config_add_named_section( CFMANAGER_CONFIG_NAME, "controller", "main" );
        if ( error_state != 0 ) {
            cfmanager_log_message( L_ERR, "failed to add section controller.main in %s\n", CFMANAGER_CONFIG_NAME );
            error_state = ERRCODE_INTERNAL_ERROR;
            goto return_value;
        }

        need_update = true;
    }
    else {
        blobmsg_parse( controller_policy,
            __CONTROLLER_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );
    }

    for( i = 0; i < __CONTROLLER_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        /* parms 'from' do not need save */
        if ( CONTROLLER_SET_FROM == i ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], controller_policy[i].type ) ) {
            continue;
        }

        sprintf( path, "%s.main.%s", CFMANAGER_CONFIG_NAME, controller_policy[i].name );
        config_set_by_blob( tb[i], path, controller_policy[i].type );

        need_update = true;

        /* others cfmanager handle*/
         switch( i ) {
            case CONTROLLER_ROLE:
                if ( !strcmp( blobmsg_get_string( tb[i] ), "master" ) ) {
                    // switch role to master
                    config_creat_no_exist_section( CFMANAGER_CONFIG_NAME, "general", "general", &cm_general_vltree, VLTREE_CM_TREE );
                    config_random_string( key, sizeof( key ) );
                    config_set_option_not_exist( "cfmanager.general.pairing_key", key, 0 );
                    config_random_string( key, sizeof( key ) );
                    config_set_option_not_exist( "cfmanager.general.failover_key", key, 0 );

                    need_update_general = true;
                }
                else if ( !strcmp( blobmsg_get_string( tb[i] ), "slave" ) ) {
                    // do not need handle
                }

                break;
            case CONTROLLER_WORK_MODE:
                if ( !strcmp( blobmsg_get_string( tb[i] ), "1" ) ) {
                    if ( tb[CONTROLLER_SET_FROM] ) {
                        /* called by mesh, can not reboot the device */
                        config_uci_set( "cfmanager.main.role", "slave", 0 );
                        config_uci_del( "cfmanager.general.pairing_key", 0 );
                        config_uci_del( "cfmanager.general.failover_key", 0 );

                        need_update_general = true;
                    }
                    else {
                        /* called by mesh local web or app, we MUST reboot the device in /usr/sbin/mode_change */
                        error_state = cfparse_controller_del_paired_devices();
                        if ( 0 == error_state) {
                            check_set_defvalue( CHECK_CFMANAGER );
                            system( "/usr/sbin/mode_change ap &" );
                            goto return_value;
                        }
                    }
                }
                else if ( !strcmp( blobmsg_get_string( tb[i] ), "0" ) ) {
                    // do not need handle
                }
                break;

            default:
                break;
        }
    }

    if( !need_update ) {
        cfmanager_log_message( L_DEBUG, "func:%s The mode config is same\n", __func__ );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_CONTROLLER, false );
    if ( need_update_general ) {
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_GENERAL, false );
    }

return_value:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );
    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void
sgreq_set_general(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__GENERAL_MAX];
    struct blob_attr *cm_tb[__GENERAL_MAX];
    struct cm_config *cm_cfg = NULL;
    char   path[LOOKUP_STR_SIZE] = { 0 };
    int    ret = ERRCODE_SUCCESS, i = 0;
    bool   need_update = false;

    blobmsg_parse( general_policy,
        __GENERAL_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    memset( cm_tb, 0, __GENERAL_MAX * sizeof(*cm_tb) );
    cm_cfg = util_get_vltree_node( &cm_general_vltree, VLTREE_CM_TREE, "general" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "No general information found in %s vltree\n", CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "general", "general" );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
             "failed to add section global_access in %s\n", CFMANAGER_CONFIG_NAME );
            ret = ERRCODE_INTERNAL_ERROR;
            goto return_value;
        }

        need_update = true;
    }
    else{
        blobmsg_parse( general_policy,
            __GENERAL_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );
    }

    for( i = 0; i < __GENERAL_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], general_policy[i].type ) ) {
            continue;
        }

        snprintf( path, sizeof(path), "%s.general.%s", CFMANAGER_CONFIG_NAME, general_policy[i].name );
        config_set_by_blob( tb[i], path, general_policy[i].type );

        need_update = true;
    }

    if( !need_update ) {
        cfmanager_log_message( L_DEBUG, "The general config is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_GENERAL, false );

return_value:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, ret );
    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void 
sgreq_set_basic(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__BASIC_SET_MAX];
    struct blob_attr *cm_tb[__BASIC_SET_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_128] = { 0 };
    int error_state = 0;
    int i = 0;
    int flag = 0;

    blobmsg_parse( basic_system_policy,
        __BASIC_SET_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    cm_cfg = util_get_vltree_node( &cm_basic_system_vltree, VLTREE_CM_TREE, "basic" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No basic information found in %s\n", CFMANAGER_CONFIG_NAME );

        error_state = config_set_section( CFMANAGER_CONFIG_NAME, "basic", "basic" );
        if ( error_state ) {
            cfmanager_log_message( L_ERR, "Creat basic in %s\n", CFMANAGER_CONFIG_NAME );
            goto return_value;
        }
    }

    if ( cm_cfg ) {
        blobmsg_parse( basic_system_policy,
            __BASIC_SET_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );
    }

    for( i = 0; i < __BASIC_SET_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( cm_tb[i] && sgreq_compar_attr( tb[i], cm_tb[i], basic_system_policy[i].type ) ) {
            continue;
        }

        sprintf( path, "%s.basic.%s", CFMANAGER_CONFIG_NAME, basic_system_policy[i].name );
        if( BLOBMSG_TYPE_BOOL == basic_system_policy[i].type ) {

            sprintf( value, "%d", blobmsg_get_bool( tb[i] ) );
            config_uci_set( path, value, 0 );
        }
        else if( BLOBMSG_TYPE_INT32 == basic_system_policy[i].type ) {

            sprintf( value, "%d", blobmsg_get_u32( tb[i] ) );
            config_uci_set( path, value, 0 );
        }
        else {
            config_uci_set( path, blobmsg_get_string( tb[i] ), 0 );
        }

        flag = 1;
    }

    if( !flag ) {
        cfmanager_log_message( L_DEBUG, "The mode basic is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_BASIC, false );

return_value:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );
    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void 
sgreq_set_extern_sys_log(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[_EXTERNAL_LOG_MAX] = { 0 };
    struct blob_attr *cm_tb[_EXTERNAL_LOG_MAX] = { 0 };
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *cur;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int error_state = 0;
    int i = 0;
    int flag = 0;
    unsigned int rem;

    blobmsg_parse( cm_extern_sys_log_policy,
    _EXTERNAL_LOG_MAX,
    tb,
    blobmsg_data( data ),
    blobmsg_len( data ) );

    cm_cfg = util_get_vltree_node( &cm_extern_log_vltree, VLTREE_CM_TREE, "debug" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No debug information found in %s\n", CFMANAGER_CONFIG_NAME );

        error_state = config_set_section( CFMANAGER_CONFIG_NAME, "debug", "debug" );
        if ( error_state ) {
            cfmanager_log_message( L_ERR, "Creat debug in %s\n", CFMANAGER_CONFIG_NAME );
            goto return_value;
        }
    }

    if ( cm_cfg ) {
        blobmsg_parse( cm_extern_sys_log_policy,
        _EXTERNAL_LOG_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );
    }

    for( i = 0; i < _EXTERNAL_LOG_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }
        if( cm_tb[i] && sgreq_compar_attr( tb[i], cm_tb[i], cm_extern_sys_log_policy[i].type ) ) {
            continue;
        }

        sprintf( path, "%s.debug.%s", CFMANAGER_CONFIG_NAME, cm_extern_sys_log_policy[i].name );

        if( cm_extern_sys_log_policy[i].type == BLOBMSG_TYPE_ARRAY ) {
            config_uci_del( path, 0 );
            blobmsg_for_each_attr(cur, data, rem) {
                config_uci_add_list( path, blobmsg_get_string( cur ), 0 );
            }
        }
        else{
            config_set_by_blob( tb[i], path, cm_extern_sys_log_policy[i].type );
        }
        
        flag = 1;
    }
    
    if( !flag ) {
        cfmanager_log_message( L_DEBUG, "The mode extern sys log is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_SYS_LOG, false );

return_value:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );
    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void
sgreq_set_email(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[_CM_EMAIL_MAX] = { 0 };
    struct blob_attr *cm_tb[_CM_EMAIL_MAX] = { 0 };
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int error_state = 0;
    int i = 0;
    int flag = 0;

    blobmsg_parse( cm_email_policy,
    _CM_EMAIL_MAX,
    tb,
    blobmsg_data(data),
    blobmsg_data_len(data) );

    cm_cfg = util_get_vltree_node(&cm_CM_EMAIL_vltree, VLTREE_CM_TREE, "email");
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No email information found in %s\n", CFMANAGER_CONFIG_NAME );

        error_state = config_set_section( CFMANAGER_CONFIG_NAME, "email", "email" );
        if ( error_state ) {
            cfmanager_log_message( L_ERR, "Creat eamil in %s\n", CFMANAGER_CONFIG_NAME );
            goto return_value;
        }
    }

    if ( cm_cfg ) {
        blobmsg_parse( cm_email_policy,
            _CM_EMAIL_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );
    }

    for (i = 0; i < _CM_EMAIL_MAX; i++) {
        if ( !tb[i] ) {
            continue;
        }
        if( cm_tb[i] && sgreq_compar_attr( tb[i], cm_tb[i], cm_email_policy[i].type ) ) {
            continue;
        }

        sprintf( path, "%s.email.%s", CFMANAGER_CONFIG_NAME, cm_email_policy[i].name );
        config_set_by_blob( tb[i], path, cm_email_policy[i].type );
        
        flag = 1;
    }
    
    if( !flag ) {
        cfmanager_log_message( L_DEBUG, "The mode email is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_EMAIL, false );

return_value:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );
    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_set_notification(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[_CM_NOTIFY_MAX] = { 0 };
    struct blob_attr *cm_tb[_CM_NOTIFY_MAX] = { 0 };
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int error_state = 0;
    int i = 0;
    int flag = 0;

    blobmsg_parse( cm_notification_policy,
    _CM_NOTIFY_MAX,
    tb,
    blobmsg_data(data),
    blobmsg_data_len(data) );

    cm_cfg = util_get_vltree_node(&cm_notification_vltree, VLTREE_CM_TREE, "notification");
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No notification information found in %s\n", CFMANAGER_CONFIG_NAME );

        error_state = config_set_section( CFMANAGER_CONFIG_NAME, "notification", "notification" );
        if ( error_state ) {
            cfmanager_log_message( L_ERR, "Creat notification in %s\n", CFMANAGER_CONFIG_NAME );
            goto return_value;
        }
    }

    if ( cm_cfg ) {
        blobmsg_parse( cm_notification_policy,
            _CM_NOTIFY_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );
    }

    for (i = 0; i < _CM_NOTIFY_MAX; i++) {
        if ( !tb[i] ) {
            continue;
        }
        if( cm_tb[i] && sgreq_compar_attr( tb[i], cm_tb[i], cm_notification_policy[i].type ) ) {
            continue;
        }

        sprintf( path, "%s.notification.%s", CFMANAGER_CONFIG_NAME, cm_notification_policy[i].name );
        config_set_by_blob( tb[i], path, cm_notification_policy[i].type );
        
        flag = 1;
    }

    if( !flag ) {
        cfmanager_log_message( L_DEBUG, "The mode notification is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_NOTIFICATION, false );

return_value:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );
    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;

}

//=============================================================================
void
sgreq_set_manage_ap_mesh(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__MANAGE_APS_ATTR_MAX];
    int error_state = ERRCODE_SUCCESS;
    struct blob_attr *cur;
    unsigned int rem;
    bool need_update = false;
    int need_controller_update = 0;

    blobmsg_parse( manage_aps_attrs_list_policy,
        __MANAGE_APS_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[MANAGE_APS_ATTR_LIST] ||
            blobmsg_type(tb[MANAGE_APS_ATTR_LIST]) != BLOBMSG_TYPE_ARRAY ) {
        cfmanager_log_message( L_ERR,
            "Missing aps in json or type is not blob array!\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    blobmsg_for_each_attr( cur, tb[MANAGE_APS_ATTR_LIST], rem ) {
        cfparse_grandstream_manage_set_ap( ctx, cur, &need_controller_update );

        if ( need_controller_update ) {
            need_update = true;
        }
    }

    if ( need_update ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_AP, false );
    }


return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_set_ap(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__CM_AP_MAX];
    struct blob_attr *cm_tb[__CM_AP_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    bool need_update = false;
    bool need_update_radio = false;
    int ret = ERRCODE_SUCCESS;
    int i = 0;
    char msg[LOOKUP_STR_SIZE] = { 0 };

    blobmsg_parse( cm_ap_policy,
        __CM_AP_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[CM_AP_MAC] ) {
        cfmanager_log_message( L_ERR, "No mac" );
        ret = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    cm_cfg = util_get_vltree_node( &cm_ap_vltree, VLTREE_CM_TREE, blobmsg_get_string( tb[CM_AP_MAC] ) );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No ap session %s found in %s\n",
            blobmsg_get_string( tb[CM_AP_MAC] ), CFMANAGER_CONFIG_NAME );
        ret = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    blobmsg_parse( cm_ap_policy,
        __CM_AP_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

    for( i = CM_AP_TYPE; i < __CM_AP_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], cm_ap_policy[i].type ) ) {
            continue;
        }

        switch( i ) {
            case CM_AP_NAME:
                snprintf( msg, LOOKUP_STR_SIZE, "%s%s%s%s", "AP(", blobmsg_get_string( tb[CM_AP_MAC] )
                    , ") name have been changed to ", blobmsg_get_string( tb[CM_AP_NAME] ) );

                send_uns_notification( ctx, msg, 0 );
                break;
            case CM_AP_2G4_DISABLE:
            case CM_AP_2G4_WIDTH:
            case CM_AP_2G4_CHANNEL_LOCATION:
            case CM_AP_2G4_CHANNEL:
            case CM_AP_2G4_POWER:
            case CM_AP_2G4_CUSTOM_TXPOWER:
            case CM_AP_2G4_RSSI_TYPE:
            case CM_AP_2G4_RSSI_THRESHOLD:
            case CM_AP_2G4_MODE:
            case CM_AP_2G4_RATE_LIMIT_TYPE:
            case CM_AP_2G4_MINI_RATE:
            case CM_AP_2G4_SHORTGI:
            case CM_AP_2G4_ALLOW_LEGACY_DEV:
            case CM_AP_5G_DISABLE:
            case CM_AP_5G_WIDTH:
            case CM_AP_5G_CHANNEL:
            case CM_AP_5G_POWER:
            case CM_AP_5G_CUSTOM_TXPOWER:
            case CM_AP_5G_RSSI_TYPE:
            case CM_AP_5G_RSSI_THRESHOLD:
            case CM_AP_5G_RATE_LIMIT_TYPE:
            case CM_AP_5G_MINI_RATE:
            case CM_AP_5G_SHORTGI:
                need_update_radio = true;
                break;
            default:
                break;
        }

        sprintf( path, "%s.%s.%s", CFMANAGER_CONFIG_NAME, blobmsg_get_string( tb[CM_AP_MAC] ), cm_ap_policy[i].name );
        config_set_by_blob( tb[i], path, cm_ap_policy[i].type );

        need_update = true;
    }

    if( !need_update ) {
        cfmanager_log_message( L_DEBUG, "The ap config session %s is same\n", blobmsg_get_string( tb[CM_AP_MAC] ) );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_AP, false );
    if( need_update_radio ) {
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_RADIO, true );
    }

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, ret );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
void
sgreq_set_vpn_client(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__VPN_CLIENT_ATTR_MAX];
    unsigned int rem;
    struct blob_attr *cur;
    int error_state = ERRCODE_SUCCESS;
    int fw_reload = 0;
    int mwan3_reload = 0;
    int need_commit = 0;

    blobmsg_parse( vpn_client_attrs_policy,
        __VPN_CLIENT_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[VPN_CLIENT_ATTR_CLIENT_LIST] ||
            blobmsg_type(tb[VPN_CLIENT_ATTR_CLIENT_LIST]) != BLOBMSG_TYPE_ARRAY ) {
        cfmanager_log_message( L_ERR,
            "Missing vpn client list in json or type is not blob array!\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto out;
    }

    blobmsg_for_each_attr( cur, tb[VPN_CLIENT_ATTR_CLIENT_LIST], rem ) {
        int  fw_need_load = 0, mwan3_need_load = 0;

        error_state = vpn_parse_client_list( cur, &fw_need_load, &mwan3_need_load );
        if ( error_state ) {
            break;
        }

        if ( fw_need_load ) {
            fw_reload = 1;
        }

        if ( mwan3_need_load ) {
            mwan3_reload = 1;
        }

        need_commit = 1;
    }

    if ( need_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_VPN_CLIENT, false );
        
        if( fw_reload ) {
            cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
        }
        
        if( mwan3_reload ) {
            cfparse_load_file( "mwan3", LOAD_ALL_SECTION, false );
        }
    }

    vpn_client_chg = 1;
out:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_set_vpn_split_tunneling(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = ERRCODE_SUCCESS;
    struct blob_attr *tb[__VPN_SPLIT_ATTR_MAX];
    unsigned int rem;
    struct blob_attr *cur;
    int mwan3_reload = 0;
    int need_commit = 0;

    blobmsg_parse( vpn_split_attrs_policy,
        __VPN_SPLIT_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[VPN_SPLIT_ATTR_SERVICE_LIST] ||
            blobmsg_type(tb[VPN_SPLIT_ATTR_SERVICE_LIST]) != BLOBMSG_TYPE_ARRAY ) {
        cfmanager_log_message( L_ERR,
            "Missing vpn split in json or type is not blob array!\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto out;
    }

    blobmsg_for_each_attr( cur, tb[VPN_SPLIT_ATTR_SERVICE_LIST], rem ) {
        int mwan3_need_load = 0;

        error_state = vpn_parse_split_list( cur, &mwan3_need_load );
        if ( error_state ) {
            break;
        }

        if ( mwan3_need_load ) {
            mwan3_reload = 1;
        }

        need_commit = 1;
    }

    if ( need_commit ) {
        // Delete non-exist section first.
        struct cm_config *cm_cfg = NULL;

        vlist_for_each_element( &cm_vpn_split_vltree, cm_cfg, node ) {
            int found = 0;

            blobmsg_for_each_attr( cur, tb[VPN_SPLIT_ATTR_SERVICE_LIST], rem ) {
                struct blob_attr *new_tb[__VPN_SPLIT_MAX];

                blobmsg_parse( vpn_split_policy,
                    __VPN_SPLIT_MAX,
                    new_tb,
                    blobmsg_data( cur ),
                    blobmsg_len( cur ) );

                if ( new_tb[VPN_SPLIT_SERVICE_ID] ) {
                    char section_nm[BUF_LEN_64] = { 0 };

                    snprintf( section_nm, sizeof(section_nm), "vpn_split%s", blobmsg_get_string(new_tb[VPN_SPLIT_SERVICE_ID]) );
                    if ( !strcmp( section_nm, cm_cfg->cf_section.name ) ) {
                        found = 1;
                        break;
                    }
                }
            }

            if ( !found ) {
                config_del_named_section( CFMANAGER_CONFIG_NAME,
                    "vpn_split", cm_cfg->cf_section.name );
                mwan3_reload = 1;
            }
        }

        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_VPN_SPLIT, false );

        if ( mwan3_reload ) {
            cfparse_load_file( "mwan3", LOAD_ALL_SECTION, false );
        }
    }

out:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_set_port_mapping(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = ERRCODE_SUCCESS;
    struct blob_attr *tb[__PORT_MAPPING_ATTR_MAX];
    unsigned int rem;
    struct blob_attr *cur;
    int fw_reload = 0;
    int need_commit = 0;

    blobmsg_parse( port_mapping_attrs_policy,
        __PORT_MAPPING_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[PORT_MAPPING_ATTR_RULE] ||
            blobmsg_type(tb[PORT_MAPPING_ATTR_RULE]) != BLOBMSG_TYPE_ARRAY ) {
        cfmanager_log_message( L_ERR,
            "Missing rule in json or type is not blob array!\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto out;
    }

    blobmsg_for_each_attr( cur, tb[PORT_MAPPING_ATTR_RULE], rem ) {
        int fw_need_load = 0;

        error_state = firewall_parse_port_mapping( cur, &fw_need_load );
        if ( error_state ) {
            break;
        }

        if ( fw_need_load ) {
            fw_reload = 1;
        }

        need_commit = 1;
    }

    if ( need_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_PORT_MAPPING, false );
        
        if( fw_reload ) {
            cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
        }
    }

out:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_set_dmz(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = ERRCODE_SUCCESS;
    int fw_reload = 0;

    error_state = firewall_parse_dmz( data, &fw_reload );
    if ( error_state == ERRCODE_SUCCESS ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_DMZ, false );
        
        if( fw_reload ) {
            cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
        }
    }

    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_set_upnp(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = ERRCODE_SUCCESS;
    int fw_reload = 0;

    error_state = upnpd_parse_upnp( data, &fw_reload );
    if ( error_state == ERRCODE_SUCCESS ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_UPNP, false );

        if( fw_reload ) {
            cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
        }
    }

    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_set_ddns(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = ERRCODE_SUCCESS;
    struct blob_attr *tb[__DDNS_ATTR_MAX];
    struct blob_attr *cur;
    int rem;
    int need_commit = 0;

    blobmsg_parse( ddns_attrs_policy,
        __DDNS_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[DDNS_ATTR_LIST] || blobmsg_type(tb[DDNS_ATTR_LIST] ) != BLOBMSG_TYPE_ARRAY ) {
        cfmanager_log_message( L_ERR,
            "Missing %s in json or type is not array!\n", ddns_attrs_policy[DDNS_ATTR_LIST].name );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto out;
    }

    blobmsg_for_each_attr( cur, tb[DDNS_ATTR_LIST], rem ) {
        error_state = ddns_parse( cur );
        if ( error_state ) {
            break;
        }

        need_commit = 1;
    }

    if ( need_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_DDNS, false );
    }

out:
    blob_buf_init( &reply, 0 );
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_set_ipv4_static_route(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__STATIC_ROUTE_IPV4_ATTR_MAX];
    unsigned int rem;
    struct blob_attr *cur;
    int error_state = ERRCODE_SUCCESS;
    int need_commit = 0;

    blobmsg_parse( static_route_ipv4_attrs_policy,
        __STATIC_ROUTE_IPV4_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[STATIC_ROUTE_IPV4_ATTR_RULE] ||
            blobmsg_type(tb[STATIC_ROUTE_IPV4_ATTR_RULE]) != BLOBMSG_TYPE_ARRAY ) {
        cfmanager_log_message( L_ERR,
            "Missing rule in json or type is not blob array!\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto out;
    }

    blobmsg_for_each_attr( cur, tb[STATIC_ROUTE_IPV4_ATTR_RULE], rem ) {
        error_state = route_parse_ipv4_static_route( cur );
        if ( error_state ) {
            break;
        }

        need_commit = 1;
    }

    if ( need_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_STATIC_ROUTE_IPV4, false );
    }

out:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}
//=============================================================================
void
sgreq_set_client_limit(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__CLIENT_LIMIT_MAX];
    struct blob_attr *cm_tb[__CLIENT_LIMIT_MAX];
    struct cm_config *cm_cfg = NULL;
    int error_state = ERRCODE_SUCCESS;
    char mac[MAC_STR_MAX_LEN+1] = { 0 };
    char section_name[BUF_LEN_32] = { 0 };
    char path[LOOKUP_STR_SIZE];
    int ret = 0;
    int i = 0;
    bool need_update = false;

    blobmsg_parse( client_limit_policy,
        __CLIENT_LIMIT_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if( !tb[CLIENT_LIMIT_MAC] ) {
        cfmanager_log_message( L_ERR, "Missing client mac information\n" );
        error_state = ERRCODE_MESSAGE_WRONG_FROMAT;
        goto return_value;
    }

    strncpy( mac, blobmsg_get_string( tb[CLIENT_LIMIT_MAC] ), MAC_STR_MAX_LEN );
    snprintf( section_name, sizeof( section_name ), "%s%s", CLIENT_LIMIT_PREFIX, mac );

    cm_cfg = util_get_vltree_node( &cm_client_limit_vltree, VLTREE_CM_TREE, section_name );
    if( !cm_cfg ) {
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "client_limit", section_name );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
             "failed to add section client_limit in %s\n", CFMANAGER_CONFIG_NAME );
            ret = ERRCODE_INTERNAL_ERROR;
            goto return_value;
        }

        for( i = 0; i < __CLIENT_LIMIT_MAX; i++ ) {
            if( !tb[i] ) {
                continue;
            }

            sprintf( path, "%s.%s.%s", CFMANAGER_CONFIG_NAME,
                section_name, client_limit_policy[i].name );
            config_set_by_blob( tb[i], path, client_limit_policy[i].type );

            need_update = true;
        }

        goto update;
    }

    blobmsg_parse( client_limit_policy,
        __CLIENT_LIMIT_MAX,
        cm_tb,
        blobmsg_data( cm_cfg->cf_section.config ),
        blobmsg_len( cm_cfg->cf_section.config ) );

    for( i = 0; i < __CLIENT_LIMIT_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        if( sgreq_compar_attr( tb[i], cm_tb[i], client_limit_policy[i].type ) ) {
            continue;
        }

        sprintf( path, "%s.%s.%s", CFMANAGER_CONFIG_NAME,
            section_name, client_limit_policy[i].name );
        config_set_by_blob( tb[i], path, client_limit_policy[i].type );

        need_update = true;
    }

update:
    if( need_update ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_CLIENT_LIMIT, false );
    }

return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
static void
add_auth_rule(
    struct blob_attr *data,
    char *path
 )
//=============================================================================
{
    enum {
        PARSE_AUTH_DST_TYPE,
        PARSE_AUTH_DST_VAL,
        PARSE_AUTH_SERVICES_TYPE,
        PARSE_AUTH_SERVICES_VAL
    };
    static const struct blobmsg_policy parse_auth_policy[] = {
        [PARSE_AUTH_DST_TYPE] = { .name = "dstType", .type = BLOBMSG_TYPE_STRING },
        [PARSE_AUTH_DST_VAL]  = { .name = "dstValue",      .type = BLOBMSG_TYPE_STRING },
        [PARSE_AUTH_SERVICES_TYPE]  = { .name = "serviceType",      .type = BLOBMSG_TYPE_STRING },
        [PARSE_AUTH_SERVICES_VAL]  = { .name = "serviceValue",  .type = BLOBMSG_TYPE_STRING }
    };
    enum { __PARSE_AUTH_MAX = ( sizeof( parse_auth_policy ) / sizeof( parse_auth_policy[0] ) ) };
    struct blob_attr *tb[__PARSE_AUTH_MAX];
    struct blob_attr *cur = NULL;
    char value[BUF_LEN_128] = { 0 };
    int rem = 0;
    int i = 0, len = 0;

    blobmsg_for_each_attr( cur, data, rem ) {
        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_TABLE )
            continue;

        blobmsg_parse( parse_auth_policy,
            __PARSE_AUTH_MAX,
            tb,
            blobmsg_data( cur ),
            blobmsg_len( cur ) );

        memset( value, 0, sizeof( value ) );
        for ( i = PARSE_AUTH_DST_TYPE; i < __PARSE_AUTH_MAX; i++ ) {
            if ( tb[i] ) {
                snprintf( value + strlen( value ),
                    sizeof( value ) - strlen( value ), "%s ", blobmsg_get_string( tb[i] ) );
            }
        }

        len = strlen( value );
        if ( !len ) {
            continue;
        }

        if ( value[len - 1] == ' ' ) {
            value[len - 1] = '\0';
        }

        config_uci_add_list( path, value, 0 );
    }
}

//=============================================================================
static void
del_auth_rule(
    struct blob_attr *data,
    char *path
 )
//=============================================================================
{
    struct blob_attr *cur = NULL;
    char value[BUF_LEN_128] = { 0 };
    int rem = 0;

    blobmsg_for_each_attr( cur, data, rem ) {
        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING )
            continue;

        snprintf( value, sizeof( value ), "%s", blobmsg_get_string( cur ) );

        config_uci_del_list( path, value, 0 );
    }
}

//=============================================================================
static void
parse_auth_rule(
    struct blob_attr *new_rule,
    struct blob_attr *old_rule,
    char *path
)
//=============================================================================
{
    if ( path == NULL )
        return;

    if ( old_rule ) {
        cfmanager_log_message( L_DEBUG, "del old auth rule" );
        del_auth_rule( old_rule, path );
    }

    if ( new_rule ) {
        cfmanager_log_message( L_DEBUG, "add old auth rule" );
        add_auth_rule( new_rule, path );
    }
}

//=============================================================================
void
sgreq_set_portal_policy(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__PORTAL_POLICY_MAX];
    struct blob_attr *cm_tb[__PORTAL_POLICY_MAX] ;
    struct cm_config *cm_cfg = NULL;
    const char *id = NULL;
    const char *section_name = NULL;
    int idx = 0, tmp_idx = 0;
    int i = 0;
    int ret = 0;
    bool create_new = false;
    bool config_changed = false;
    char name[BUF_LEN_32] = { 0 };
    char path[BUF_LEN_64] = { 0 };

    memset( cm_tb, 0, __PORTAL_POLICY_MAX * sizeof( *cm_tb ) );

    blobmsg_parse( cm_portal_policy,
        __PORTAL_POLICY_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( tb[PORTAL_POLICY_ID] ) {
        id = blobmsg_get_string( tb[PORTAL_POLICY_ID] );
        strncpy( name, id, sizeof( name ) - 1 );

        cm_cfg = util_get_vltree_node( &cm_portal_policy_vltree, VLTREE_CM_TREE, name );
        if ( !cm_cfg ) {
            create_new = true;
        }
        else {
            blobmsg_parse( cm_portal_policy,
                __PORTAL_POLICY_MAX,
                cm_tb,
                blob_data( cm_cfg->cf_section.config ),
                blob_len( cm_cfg->cf_section.config  ) );
        }
    }
    else {
        vlist_for_each_element(
            &cm_portal_policy_vltree, cm_cfg, node ) {

            section_name = cm_cfg->cf_section.name;

            sscanf( section_name, "portal_policy_%d", &tmp_idx );

            idx |= BIT( tmp_idx );
        }

        for ( i = 0; i < MAX_PORTAL_POLICY_NUMBER; i++ ) {
            if ( !( idx & BIT( i ) ) ) {
                snprintf( name, sizeof( name ), "portal_policy_%d", i );
                break;
            }
        }

        create_new = true;
    }

    do {
        if ( create_new ) {
            ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "portal_policy", name );
            if ( ret != 0 ) {
                cfmanager_log_message( L_ERR,
                 "failed to add section %s in %s\n", name, CFMANAGER_CONFIG_NAME );
                ret = ERRCODE_INTERNAL_ERROR;
                break;
            }

            config_changed = true;
        }

        for( i = __PORTAL_POLICY_MIN; i < __PORTAL_POLICY_MAX; i++ ) {
            if( !tb[i] ) {
                continue;
            }

            if( sgreq_compar_attr( tb[i], cm_tb[i], cm_portal_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof(path),
                "%s.%s.%s", CFMANAGER_CONFIG_NAME, name, cm_portal_policy[i].name );

            if ( i == PORTAL_POLICY_PRE_AUTH
                || i == PORTAL_POLICY_POST_AUTH ) {
                parse_auth_rule( tb[i], cm_tb[i], path );
            }
            else
                config_set_by_blob( tb[i], path, cm_portal_policy[i].type );

            config_changed = true;
        }

        if( config_changed ) {
            config_commit( CFMANAGER_CONFIG_NAME, 0 );
            cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_POTAL_POLICY, false );
        }

        ret = ERRCODE_SUCCESS;
    } while( 0 );


    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, ret );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
void
sgreq_set_vpn_server(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__VPN_SERVER_ATTR_MAX];
    unsigned int rem;
    struct blob_attr *cur;
    int error_state = ERRCODE_SUCCESS;
    int fw_reload = 0;
    int mwan3_reload = 0;
    int need_commit = 0;

    blobmsg_parse( vpn_server_attrs_policy,
        __VPN_SERVER_ATTR_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[VPN_SERVER_ATTR_SERVER_LIST] ||
            blobmsg_type(tb[VPN_SERVER_ATTR_SERVER_LIST]) != BLOBMSG_TYPE_ARRAY ) {
        cfmanager_log_message( L_ERR,
            "Missing vpn server list in json or type is not blob array!\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto out;
    }

    blobmsg_for_each_attr( cur, tb[VPN_SERVER_ATTR_SERVER_LIST], rem ) {
        int  fw_need_load = 0, mwan3_need_load = 0;

        error_state = vpn_parse_server_list( cur, &fw_need_load, &mwan3_need_load );
        if ( error_state ) {
            break;
        }

        if ( fw_need_load ) {
            fw_reload = 1;
        }

        if ( mwan3_need_load ) {
            mwan3_reload = 1;
        }

        need_commit = 1;
    }

    if ( need_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_VPN_SERVER, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_IPSEC_CMN_SETTING, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_IPSEC_DIAL_IN_USER, false );

        if( fw_reload ) {
            cfparse_load_file( "firewall", LOAD_ALL_SECTION, false );
        }
        
        if( mwan3_reload ) {
            cfparse_load_file( "mwan3", LOAD_ALL_SECTION, false );
        }
    }

    //vpn_client_chg = 1;
out:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}


//=============================================================================
void
sgreq_set_acceleration(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;

    char path[LOOKUP_STR_SIZE];
    bool config_change = false;
    bool enable = false;
    bool cm_enable = false;
    const char *engine = NULL;
    const char *cm_engine = NULL;
    struct blob_attr *tb[__ACCELERATION_MAX];
    struct blob_attr *cm_tb[__ACCELERATION_MAX];
    struct cm_config *cm_cfg = NULL;

    blobmsg_parse( acceleration_policy,
        __ACCELERATION_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );


    enable = util_blobmsg_get_bool( tb[ACCELERATION_ENABLE], false );
    engine = util_blobmsg_get_string( tb[ACCELERATION_ENGINE], "auto" );

    cm_cfg = util_get_vltree_node( &cm_acceleration_vltree, 
                VLTREE_CM_TREE, "acceleration" );

    if( !cm_cfg ) {
        error_state = config_add_named_section( CFMANAGER_CONFIG_NAME,
                            "acceleration", "acceleration" );
        if( error_state < 0 ) {
            cfmanager_log_message( L_ERR,
                "add section '%s' failed\n", "acceleration" );
            goto return_value;
        }
    }
    else {
        blobmsg_parse( acceleration_policy,
            __ACCELERATION_MAX,
            cm_tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_data_len( cm_cfg->cf_section.config ) );
    }


    cm_enable = util_blobmsg_get_bool( cm_tb[ACCELERATION_ENABLE], false );
    cm_engine = util_blobmsg_get_string( cm_tb[ACCELERATION_ENGINE], "" );


    if ( cm_enable != enable ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, "acceleration",
            acceleration_policy[ACCELERATION_ENABLE].name );
        config_uci_set( path, enable ? "1" : "0", 0 );
        config_change = true;
    }

    if ( strcmp( cm_engine, engine ) != 0 ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CFMANAGER_CONFIG_NAME, "acceleration",
            acceleration_policy[ACCELERATION_ENGINE].name );
        config_uci_set( path, (char *)engine, 0 );
        config_change = true;
    }

    if( config_change ) {
        config_commit( CFMANAGER_CONFIG_NAME, 0 );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_ACCELERATION, false );
    }

return_value:

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
void
sgreq_set_dev_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_attr *tb[__DEV_SSID_MAX];
    struct blob_attr *cur = NULL;
    char mac_str[MAC_STR_MAX_LEN+1];
    char ssids[BUF_LEN_128] = { 0 };
    char *ssid_id = NULL;
    int rem = 0;
    bool config_change = false;

    blobmsg_parse( dev_ssid_policy,
        __DEV_SSID_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if( !tb[DEV_SSID_ID] ) {
        cfmanager_log_message( L_ERR, "miss ssid id\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    ssid_id = blobmsg_get_string( tb[DEV_SSID_ID] );

    if( tb[DEV_SSID_AVAILABLE_DEV] ) {
        blobmsg_for_each_attr( cur, tb[DEV_SSID_AVAILABLE_DEV], rem ) {
            if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING ) {
                continue;
            }

            memset( mac_str, 0, sizeof( mac_str ) );
            memset( ssids, 0, sizeof( ssids ) );
            strncpy( mac_str, util_blobmsg_get_string( cur, ""), sizeof( mac_str ) -1 );

            config_get_cm_ap_ssids( mac_str, ssids, sizeof( ssids ) );

            if( !util_match_ssids( ssids, ssid_id ) ) {
                continue;
            }

            //If the ssid id is matched in an addable device, the ap should remove the ssid id
            config_del_dev_ssid_id( mac_str, ssid_id );

            config_change = true;
        }
    }

    if( !config_change && tb[DEV_SSID_MEMBER_DEV] ) {
        blobmsg_for_each_attr( cur, tb[DEV_SSID_MEMBER_DEV], rem ) {
            if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING ) {
                continue;
            }

            memset( mac_str, 0, sizeof( mac_str ) );
            memset( ssids, 0, sizeof( ssids ) );
            strncpy( mac_str, util_blobmsg_get_string( cur, ""), sizeof( mac_str ) -1 );

            config_get_cm_ap_ssids( mac_str, ssids, sizeof( ssids ) );

            //If the ssid id does not match in the added devices, the device need add this ssid id
            if( !util_match_ssids( ssids, ssid_id ) ) {
                config_add_dev_ssid_id( mac_str, ssid_id );
                config_change = true;
            }
         }
    }

    if( config_change ) {
        config_commit( CFMANAGER_CONFIG_NAME, 0 );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_AP, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_ADDIT_SSID, true );
    }
    else {
        cfmanager_log_message( L_DEBUG, "config unchanged\n" );
    }

return_value:

    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
    return;
}

//=============================================================================
static void
parse_schedule_abtimes(
    struct blob_attr *attr,
    char *path
)
//=============================================================================
{
    struct blob_attr *tb[__CM_SCHEDULE_ABTIME_MAX];
    char value[BUF_LEN_512] = { 0 };

    blobmsg_parse( cm_schedule_abtime_policy,
        __CM_SCHEDULE_ABTIME_MAX,
        tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    if ( !tb[CM_SCHEDULE_ABTIME_ABDATE] || !tb[CM_SCHEDULE_ABTIME_ABTIME] ) {
        cfmanager_log_message( L_ERR, "Missing some params in json!" );
        return;
    }

    snprintf( value, sizeof( value ), "%s:%s",
            util_blobmsg_get_string( tb[CM_SCHEDULE_ABTIME_ABDATE], "00000000" ),
            util_blobmsg_get_string( tb[CM_SCHEDULE_ABTIME_ABTIME], "0000-0000" ) );
    config_uci_add_list( path, value, 0 );
}

//=============================================================================
void
sgreq_set_schedule(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__CM_SCHEDULE_MAX];
    struct blob_attr *cm_tb[__CM_SCHEDULE_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
//    char value[LOOKUP_STR_SIZE] = { 0 };
    char session_name[BUF_LEN_64] = {0};
    int scheduleid = 0;
    int error_state = 0;
    int i = 0;
    bool need_update = false;
    int action = 0;
    struct blob_attr *abtimes_cur;
    unsigned int rem;

    blobmsg_parse( cm_schedule_policy,
        __CM_SCHEDULE_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb[CM_SCHEDULE_ACTION] ) {
        cfmanager_log_message( L_ERR, "No action" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }
    action = atoi( blobmsg_get_string( tb[CM_SCHEDULE_ACTION] ) );

    /* schedule id as key */
    if ( !tb[CM_SCHEDULE_ID] ) {
        cfmanager_log_message( L_ERR, "No sehedule Id" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    scheduleid = atoi( blobmsg_get_string( tb[CM_SCHEDULE_ID] ) );
    snprintf( session_name, BUF_LEN_64, "schedule%d", scheduleid );


    if ( CONFIG_DEL == action ) {
        config_del_named_section( CFMANAGER_CONFIG_NAME, "schedule", session_name );
        need_update = true;
    }
    else if ( CONFIG_SET == action || CONFIG_EDIT == action ) {
        memset( cm_tb, 0, __CM_SCHEDULE_MAX * sizeof(*cm_tb) );
        cm_cfg = util_get_vltree_node( &cm_schedule_vltree, VLTREE_CM_TREE, session_name );
        if( !cm_cfg ) {
            cfmanager_log_message( L_DEBUG, "No %s found in %s, we will add it", session_name, CFMANAGER_CONFIG_NAME );
            error_state = config_add_named_section( CFMANAGER_CONFIG_NAME, "schedule", session_name );
            if ( error_state != 0 ) {
                cfmanager_log_message( L_ERR, "failed to add section %s in %s\n", session_name, CFMANAGER_CONFIG_NAME );
                error_state = ERRCODE_INTERNAL_ERROR;
                goto return_value;
            }

            need_update = true;
        }
        else {
            blobmsg_parse( cm_schedule_policy,
                __CM_SCHEDULE_MAX,
                cm_tb,
                blob_data( cm_cfg->cf_section.config ),
                blob_len( cm_cfg->cf_section.config  ) );
        }

        for( i = CM_SCHEDULE_ID; i < __CM_SCHEDULE_MAX; i++ ) {
            if( !tb[i] ) {
                continue;
            }

            if( sgreq_compar_attr( tb[i], cm_tb[i], cm_schedule_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ),
                "%s.%s.%s", CFMANAGER_CONFIG_NAME, session_name, cm_schedule_policy[i].name );

            switch( i ) {
                case CM_SCHEDULE_ABTIME_LISTS:
                    /* clear abtime list */
                    config_uci_del( path, 0 );

                    /* clear abtime list */
                    blobmsg_for_each_attr( abtimes_cur, tb[i], rem ) {
                        parse_schedule_abtimes( abtimes_cur, path );
                        need_update = true;
                    }
                    break;
                default:
                    config_set_by_blob( tb[i], path, cm_schedule_policy[i].type );
                    break;
            }

            need_update = true;
        }
    }


    if( !need_update ) {
        cfmanager_log_message( L_DEBUG, "The schedule config is same\n" );
        goto return_value;
    }

    config_commit( CFMANAGER_CONFIG_NAME, false );
    cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_SCHEDULE, false );


return_value:
    blob_buf_init( &reply, 0 );

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return;
}

//=============================================================================
static int
sgreq_get_info_by_cmtree(
    struct blob_buf *b,
    struct vlist_tree *vltree,
    const struct blobmsg_policy *policy,
    char *key,
    int policy_size
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[policy_size];
    uint8_t lan_mac[6] = {0};
    char    lan0_zone0_macaddr[18] = {0};
    int i = 0, ret = 0;

    if( NULL == vltree || NULL == policy || NULL == b ) {
        return ERRCODE_INTERNAL_ERROR;
    }

    cm_cfg = util_get_vltree_node( vltree, VLTREE_CM_TREE, key );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No %s information found in %s\n", key, CFMANAGER_CONFIG_NAME );
        return ERRCODE_INTERNAL_ERROR;
    }

    blobmsg_parse( policy,
        policy_size,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    for( i = 0; i < policy_size; i++ ) {
        if( !tb[i] || BLOBMSG_TYPE_ARRAY == policy[i].type ) {
            continue;
        }

        blobmsg_add_blob( b, tb[i] );
    }

    ret = gs_get_mac_byname( "br-lan0_zone0", lan_mac );
    if ( ret == 0 ) {
        snprintf( lan0_zone0_macaddr,
            sizeof( lan0_zone0_macaddr ),
            "%02x:%02x:%02x:%02x:%02x:%02x",
            lan_mac[0], lan_mac[1], lan_mac[2], lan_mac[3], lan_mac[4], lan_mac[5] );

        blobmsg_add_string( b, "lanMac", lan0_zone0_macaddr );
    }
    else {
        cfmanager_log_message( L_ERR, "Get br-lan0_zone0 mac faild\n" );
        blobmsg_add_string( b, "lanMac", "" );
    }

    return ERRCODE_SUCCESS;
}

//=============================================================================
static void
sgreq_add_wan_alia_ip(
    struct blob_buf *b,
    int wan_type
)
//=============================================================================
{
    struct cm_config *cm_cfg;
    struct blob_attr *tb[__WAN_ALIAS_ATTR_MAX];
    struct blob_attr *cur;
    void *a;
    int i;
    const char *key = NULL;
    const char *cm_vltree_key = NULL;

    if( WAN0 == wan_type ) {
        key = wan_policy[WAN_ALIAS_IP].name;
        cm_vltree_key = CM_WAN0_ALIAS_SECTION_NAME;
    }
    else{
        key = wan_policy[WAN1_ALIAS_IP].name;
        cm_vltree_key = CM_WAN1_ALIAS_SECTION_NAME;
    }

    a = blobmsg_open_array( b, key );

    cm_cfg = util_get_vltree_node( &cm_wan_alias_vltree, VLTREE_CM_TREE, cm_vltree_key );
    if( !cm_cfg ) {
        cfmanager_log_message( L_ERR, "No wan alias information, wan type:%d\n", wan_type );
        blobmsg_close_array( b, a );
        return;
    }

    blobmsg_parse( wan_alias_policy,
        __WAN_ALIAS_ATTR_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    if( !tb[WAN_ALIAS_ATTR_IP] ) {
        cfmanager_log_message( L_ERR, "No wan alias ip information, wan type:%d\n", wan_type );
        blobmsg_close_array( b, a );
        return;
    }

    blobmsg_for_each_attr( cur, tb[WAN_ALIAS_ATTR_IP], i ) {
        blobmsg_add_blob( b, cur );
    }

    blobmsg_close_array( b, a );
}

//=============================================================================
void
sgreq_get_wan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    char mac_str[MAC_STR_MAX_LEN+1] = { 0 };

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    error_state = sgreq_get_info_by_cmtree( b, &cm_wan_vltree, wan_policy, "wan", __WAN_MAX );

    sgreq_add_wan_alia_ip( b, WAN0 );
    sgreq_add_wan_alia_ip( b, WAN1 );

    //Wan's MAC address is fixed, adopt the router's MAC offset can be obtained
    snprintf( mac_str, sizeof( mac_str ), COMPACT_MACSTR,
        device_info.mac_raw[0],
        device_info.mac_raw[1],
        device_info.mac_raw[2],
        device_info.mac_raw[3],
        device_info.mac_raw[4],
        device_info.mac_raw[5] +4 );
    blobmsg_add_string( b, "wanRouterMac", mac_str );

    snprintf( mac_str, sizeof( mac_str ), COMPACT_MACSTR,
        device_info.mac_raw[0],
        device_info.mac_raw[1],
        device_info.mac_raw[2],
        device_info.mac_raw[3],
        device_info.mac_raw[4],
        device_info.mac_raw[5] +3 );
    blobmsg_add_string( b, "wan1RouterMac", mac_str );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
static void
sgreq_get_bind_ip(
    struct blob_buf *b
)
//=============================================================================
{
    //action:xxx,mac:xxx,mapIp4Address:xxx,vlan:xxxx
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__LAN_MAX];
    struct blob_attr *cur = NULL;
    char value[BUF_LEN_128];
    char *p = NULL;
    char *tmp = NULL;
    char mac[MAC_STR_MAX_LEN+1];
    char format_mac[MAC_STR_MAX_LEN+1];
    char ip[IP4ADDR_MAX_LEN+1];
    char vlan[BUF_LEN_8] = {0};
    int rem = 0;
    int action = 0;
    void *array;
    void *table;

    if( NULL == b ) {
        return;
    }

    cm_cfg = util_get_vltree_node( &cm_lan_vltree, VLTREE_CM_TREE, "lan" );
    if( !cm_cfg ) {
        return;
    }

    blobmsg_parse( lan_policy,
        __LAN_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    if( !tb[LAN_BINDIP] ) {
        array = blobmsg_open_array( b, "bindIp" );
        blobmsg_close_array( b, array );
        return;
    }

    array = blobmsg_open_array( b, "bindIp" );

    blobmsg_for_each_attr( cur, tb[LAN_BINDIP], rem ) {
        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING ) {
            continue;
        }
        memset( value, 0, sizeof( value ) );
        strncpy( value, blobmsg_get_string( cur ), sizeof( value ) -1 );

        //action
        p = strtok( value, "," );
        if( !p ) {
            continue;
        }
        tmp = strstr( p, ":" );
        if( !tmp ) {
            continue;
        }
        action = atoi( tmp+1 );
        if( BIND_IP_ACTION_DEL == action ) {
            continue;
        }

        //mac:xxx
        p = strtok( NULL, "," );
        if( !p ) {
            continue;
        }

        tmp = strstr( p, "mac" );
        if( !tmp ) {
            continue;
        }
        memset( mac, 0, sizeof( mac ) );
        memset( format_mac, 0, sizeof( format_mac ) );

        strncpy( mac, tmp+4, sizeof( mac ) -1 );
        util_str_erase_colon( mac, format_mac );

        //mapIp4Address:xxx
        p = strtok( NULL, "," );
        if( !p ) {
            continue;
        }

        tmp = strstr( p, ":" );
        if( !tmp ) {
            continue;
        }

        memset( ip, 0, sizeof( ip ) );
        strncpy( ip, tmp+1, sizeof( ip ) -1 );

        //vlan:xxx
        p = strtok( NULL, "," );
        if( !p ) {
            continue;
        }

        tmp = strstr( p, ":" );
        if( !tmp ) {
            continue;
        }

        memset( vlan, 0, sizeof( vlan ) );
        strncpy( vlan, tmp+1, sizeof( vlan ) -1 );


        table = blobmsg_open_table( b, NULL );

        blobmsg_add_string( b, "mac", format_mac );
        blobmsg_add_string( b, "mapIp4Address", ip );
        blobmsg_add_string( b, "vlan", vlan );

        blobmsg_close_table( b, table );
    }

    blobmsg_close_array( b, array );
}

//=============================================================================
void
sgreq_get_lan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    sgreq_get_info_by_cmtree( b, &cm_lan_vltree, lan_policy, "lan", __LAN_MAX );
    sgreq_get_bind_ip( b );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_vlan(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb_input[__VLAN_GET_MAX];
    int error_state = 0;
    struct blob_buf *b;
    void *a;
    void *t;
    int i;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__VLAN_MAX];
    int is_simple = false;
    int total_count = 0;
    char str_total_count[BUF_LEN_8] = {0};

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    /* get input params */
    blobmsg_parse( vlan_get_policy,
        __VLAN_GET_MAX,
        tb_input,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( tb_input[VLAN_GET_TYPE] ) {
        if ( !strcmp( blobmsg_get_string( tb_input[VLAN_GET_TYPE] ), "simple" ) )
            is_simple = true;
    }

    a = blobmsg_open_array( b, "vlans" );
    vlist_for_each_element( &cm_vlan_vltree, cm_cfg, node ) {
        blobmsg_parse( vlan_policy,
            __VLAN_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );

        t = blobmsg_open_table( b, NULL );
        if ( is_simple ) {
            blobmsg_add_blob( b, tb[VLAN_ID] );
            blobmsg_add_blob( b, tb[VLAN_NAME] );
        }
        else {
            for ( i = VLAN_ID; i < __VLAN_MAX; i++ ) {
                if ( !tb[i] ) {
                    continue;
                }
                blobmsg_add_blob( b, tb[i] );
            }
        }
        blobmsg_close_table( b, t );
        total_count++;
    }
    blobmsg_close_array( b, a );

    snprintf( str_total_count, BUF_LEN_8, "%d", total_count );
    blobmsg_add_string( b, "totalCount", str_total_count );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_switch_ports(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    void *a;
    void *t;
    int i;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__SWITCH_PORT_MAX];

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    a = blobmsg_open_array( b, "ports" );
    vlist_for_each_element( &cm_switch_port_vltree, cm_cfg, node ) {
        blobmsg_parse( switch_port_policy,
            __SWITCH_PORT_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );

        t = blobmsg_open_table( b, NULL );
        for ( i = 0; i < __SWITCH_PORT_MAX; i++ ) {
            if ( !tb[i] ) {
                continue;
            }
            blobmsg_add_blob( b, tb[i] );
        }
        blobmsg_close_table( b, t );
    }
    blobmsg_close_array( b, a );


    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_wireless(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    error_state = sgreq_get_info_by_cmtree( b, &cm_wireless_vltree,
        wireless_policy, "wireless", __WIRELESS_MAX );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_firewall_dos(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    error_state = sgreq_get_info_by_cmtree( b, &cm_firewall_dos_vltree,
        firewall_dos_policy, "dos_def", __FIREWALL_DOS_MAX );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_ipv6_static_route(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__STATIC_ROUTE_IPV6_MAX];
    struct cm_config *cm_cfg = NULL;
    struct blob_buf *b;
    void *a;
    void *t;
    int error_state = 0;
    int i = 0;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }
    a = blobmsg_open_array( b, "rule" );

    vlist_for_each_element( &cm_static_route_ipv6_vltree, cm_cfg, node ) {

        t = blobmsg_open_table( b, NULL );

        blobmsg_parse( static_route_ipv6_policy,
            __STATIC_ROUTE_IPV6_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );

        for( i = 0; i < __STATIC_ROUTE_IPV6_MAX; i++ ) {
            if( tb[i] ) {
                blobmsg_add_blob( b, tb[i] );
            }
        }

        blobmsg_close_table( b, t );
    }

    blobmsg_close_array( b, a );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
static void
sgreq_get_wan_ip_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_attr *tb[__NETWORK_ATTR_MAX];
    struct blob_attr *cur_table;
    char address[IP4ADDR_MAX_LEN+1] = { 0 };
    char mask[IP4ADDR_MAX_LEN+1] = { 0 };
    int i = 0;
    struct blob_buf *reply = (struct blob_buf *)req->priv;

    blobmsg_parse( network_attrs_policy,
        __NETWORK_ATTR_MAX,
        tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( tb[NETWORK_ATTR_IPV4_ADDRESS] ) {
        blobmsg_for_each_attr( cur_table, tb[NETWORK_ATTR_IPV4_ADDRESS], i ) {
            if ( blobmsg_type( cur_table ) != BLOBMSG_TYPE_TABLE ) {
                continue;
            }

            memset(address, 0, sizeof(address));
            config_parse_ipv4_addr_attr( cur_table, mask, address );

            blobmsg_add_string( reply, NULL, address );

            break;
        }
    }
}

//=============================================================================
void
sgreq_get_wan_ip(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    int rem;
    uint32 search_id = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__WAN_ALIAS_ATTR_MAX];
    struct blob_attr *cur = NULL;
    void *cookie;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cookie = blobmsg_open_array( b, "wanIp4Address" );

    if ( ctx && !ubus_lookup_id( ctx, "network.interface.wan0", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status", NULL, sgreq_get_wan_ip_cb, b, 1000 );
    }

    cm_cfg = util_get_vltree_node( &cm_wan_alias_vltree, VLTREE_CM_TREE, "wan_alias" );
    if( cm_cfg ) {
        blobmsg_parse( wan_alias_policy,
            __WAN_ALIAS_ATTR_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );

        blobmsg_for_each_attr( cur, tb[WAN_ALIAS_IP], rem ) {
            blobmsg_add_blob( b, cur );
        }
    }

    blobmsg_close_array( b, cookie );

#ifdef DOUBLE_WAN
    cookie = blobmsg_open_array( b, "wan1Ip4Address" );

    if ( ctx && !ubus_lookup_id( ctx, "network.interface.wan1", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status", NULL, sgreq_get_wan_ip_cb, b, 1000 );
    }

    cm_cfg = util_get_vltree_node( &cm_wan_alias_vltree, VLTREE_CM_TREE, "wan1_alias" );
    if( cm_cfg ) {
        blobmsg_parse( wan_alias_policy,
            __WAN_ALIAS_ATTR_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );

        blobmsg_for_each_attr( cur, tb[WAN_ALIAS_IP], rem ) {
            blobmsg_add_blob( b, cur );
        }
    }

    blobmsg_close_array( b, cookie );
#endif

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
static void
sgreq_get_clients_online_count_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_buf *reply = (struct blob_buf *)req->priv;
    char *str = NULL;
    json_object *parsed_object = NULL;
    json_object *client_object = NULL;
    json_object *sta_object = NULL;
    json_object *temp_object = NULL;
    json_bool json_result = TRUE;
    int cnt = 0, i = 0;
    int clients_online_cnt_total = 0;
    int clients_online_cnt_lan = 0;
    int clients_online_cnt_2g4 = 0;
    int clients_online_cnt_5g = 0;
    char count_str_tmp[16] = {0};

    str = blobmsg_format_json( msg, true );
    if ( !str ) {
        cfmanager_log_message( L_ERR, "client info can not format to json" );
        return;
    }

    parsed_object = json_tokener_parse( str );
    free( str );

    if ( !parsed_object ) {
        cfmanager_log_message( L_ERR, "Parse json string to json object failed" );
        return;
    }

    json_result =
        json_object_object_get_ex( parsed_object, "clients_list", &client_object );
    if ( !json_result ) {
        cfmanager_log_message( L_ERR, "Can not get clients_list array" );
        json_object_put( parsed_object );
        return;
    }

    cnt = json_object_array_length( client_object );
    if ( cnt <= 0 ) {
        cfmanager_log_message( L_ERR, "Client count is %d", cnt );
        json_object_put( parsed_object );
        return;
    }

    for ( i = 0; i < cnt; i++ ) {
        int wireless = 0;
        sta_object = json_object_array_get_idx( client_object, i );

        json_result =
            json_object_object_get_ex( sta_object, "mac", &temp_object );
        if ( !json_result ) {
            cfmanager_log_message( L_ERR, "Can not get mac" );
            continue;
        }

        json_result =
            json_object_object_get_ex( sta_object, "online", &temp_object );
        if ( !json_result || json_object_get_int( temp_object ) != 1 ) {
            /* just get online clients */
            continue;
        }

        json_result =
            json_object_object_get_ex( sta_object, "wireless", &temp_object );
        if ( !json_result ) {
            cfmanager_log_message( L_ERR, "Can not get wireless" );
            continue;
        }
        wireless = json_object_get_int( temp_object );

        if ( wireless == 1 ) {
            int band = 0;
            json_result =
                json_object_object_get_ex( sta_object, "connect_mode", &temp_object );
            band = json_object_get_int64( temp_object );
            if ( band == 1 )
                clients_online_cnt_2g4++;
            else if( band == 2 )
                clients_online_cnt_5g++;
            else
                cfmanager_log_message( L_ERR, "band Type(%d) is error", band );
        }
        else {
            clients_online_cnt_lan++;
        }

        clients_online_cnt_total++;
    }

    if ( parsed_object )
        json_object_put( parsed_object );


    /* Filling data */
    snprintf( count_str_tmp, sizeof(count_str_tmp)-1, "%d", clients_online_cnt_total );
    blobmsg_add_string( reply, "totalOnlineClientsNum", count_str_tmp );
    snprintf( count_str_tmp, sizeof(count_str_tmp)-1, "%d", clients_online_cnt_lan );
    blobmsg_add_string( reply, "lanOnlineClientsNum", count_str_tmp );
    snprintf( count_str_tmp, sizeof(count_str_tmp)-1, "%d", clients_online_cnt_2g4 );
    blobmsg_add_string( reply, "2g4OnlineClientsNum", count_str_tmp );
    snprintf( count_str_tmp, sizeof(count_str_tmp)-1, "%d", clients_online_cnt_5g );
    blobmsg_add_string( reply, "5gOnlineClientsNum", count_str_tmp );

    return;
}

//=============================================================================
void
sgreq_get_client(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    void *array;
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    // TODO: clientInfo and  accessSchedule waiting to be supplemented
    //If clientInfo and accessSchedule has no data, it also needs to reply to an empty array
    array = blobmsg_open_array( b, "clientInfo" );
    blobmsg_close_array( b, array );
    array = blobmsg_open_array( b, "accessSchedule" );
    blobmsg_close_array( b, array );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
static void
sgreq_add_dev_wan_info(
    struct blob_buf *reply,
    struct blob_attr *msg,
    int wan
)
//=============================================================================
{
    struct blob_attr *tb[__NETWORK_ATTR_MAX];
    struct blob_attr *cur_table;
    char address[IP4ADDR_MAX_LEN+1] = { 0 };
    char mask[IP4ADDR_MAX_LEN+1] = { 0 };
    int dns_count = 0;
    int i = 0;

    blobmsg_parse( network_attrs_policy,
        __NETWORK_ATTR_MAX,
        tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( tb[NETWORK_ATTR_IPV4_ADDRESS] ) {
        blobmsg_for_each_attr( cur_table, tb[NETWORK_ATTR_IPV4_ADDRESS], i ) {
            if ( blobmsg_type( cur_table ) != BLOBMSG_TYPE_TABLE ) {
                continue;
            }

            memset(address, 0, sizeof(address));
            config_parse_ipv4_addr_attr( cur_table, mask, address );

            if( WAN0 == wan ) {
                    blobmsg_add_string( reply, "wanIp4Address", address );
                    blobmsg_add_string( reply, "wanNetmask", mask );
            }
#ifdef DOUBLE_WAN
            else {
                    blobmsg_add_string( reply, "wan1Ip4Address", address );
                    blobmsg_add_string( reply, "wan1Netmask", mask );
            }
#endif
        }
    }

    if( tb[NETWORK_ATTR_ROUTE] ) {
        blobmsg_for_each_attr( cur_table, tb[NETWORK_ATTR_ROUTE], i ) {
            if ( blobmsg_type( cur_table ) != BLOBMSG_TYPE_TABLE ) {
                continue;
            }

            memset(address, 0, sizeof(address));
            config_parse_route_attr( cur_table, address );

            if( WAN0 == wan )
                blobmsg_add_string( reply, "wanGateway", address );
#ifdef DOUBLE_WAN
            else
                blobmsg_add_string( reply, "wan1Gateway", address );
#endif
        }
    }

    if( tb[NETWORK_ATTR_DNS_SERVER] ) {
        blobmsg_for_each_attr( cur_table, tb[NETWORK_ATTR_DNS_SERVER], i ) {
            if( blobmsg_type( cur_table ) != BLOBMSG_TYPE_STRING ) {
                continue;
            }

            if( 0 == dns_count ) {
                if( WAN0 == wan )
                    blobmsg_add_string( reply, "wanFirstDns", blobmsg_get_string( cur_table ) );
#ifdef DOUBLE_WAN
                else
                    blobmsg_add_string( reply, "wan1FirstDns", blobmsg_get_string( cur_table ) );
#endif
            }
            else if ( 1 == dns_count ) {
                if( WAN0 == wan )
                    blobmsg_add_string( reply, "wanSecondDns", blobmsg_get_string( cur_table ) );
#ifdef DOUBLE_WAN
                else
                    blobmsg_add_string( reply, "wan1SecondDns", blobmsg_get_string( cur_table ) );
#endif
            }
            else {
                continue;
            }

            dns_count++;
        }
    }
}

//=============================================================================
static void
sgreq_get_dev_wan0_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_buf *reply = (struct blob_buf *)req->priv;

    sgreq_add_dev_wan_info( reply, msg, WAN0 );
}

#ifdef DOUBLE_WAN
//=============================================================================
static void
sgreq_get_dev_wan1_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_buf *reply = (struct blob_buf *)req->priv;

    sgreq_add_dev_wan_info( reply, msg, WAN1 );
}
#endif
//=============================================================================
void
sgreq_get_dev_lan0_zone0_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_attr *tb[__NETWORK_ATTR_MAX];
    struct blob_buf *reply = (struct blob_buf *)req->priv;
    struct blob_attr *cur_table;
    char address[IP4ADDR_MAX_LEN+1] = { 0 };
    char mask[IP4ADDR_MAX_LEN+1] = { 0 };
    int dns_count = 0;
    int i = 0;
    struct cm_config *cm_lan_cfg = NULL;
    struct blob_attr *tb_lan[__LAN_MAX];

    blobmsg_parse( network_attrs_policy,
        __NETWORK_ATTR_MAX,
        tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( tb[NETWORK_ATTR_IPV4_ADDRESS] ) {
        blobmsg_for_each_attr( cur_table, tb[NETWORK_ATTR_IPV4_ADDRESS], i ) {
            if ( blobmsg_type( cur_table ) != BLOBMSG_TYPE_TABLE ) {
                continue;
            }

            config_parse_ipv4_addr_attr( cur_table, mask, address );

            blobmsg_add_string( reply, "lanIp4Address", address );
            blobmsg_add_string( reply, "lanNetmask", mask );
        }
    }

    if( tb[NETWORK_ATTR_DNS_SERVER] ) {
        blobmsg_for_each_attr( cur_table, tb[NETWORK_ATTR_DNS_SERVER], i ) {
            if( blobmsg_type( cur_table ) != BLOBMSG_TYPE_STRING ) {
                continue;
            }

            if( 0 == dns_count ) {
                blobmsg_add_string( reply, "dhcpFirstDns", blobmsg_get_string( cur_table ) );
            }
            else if ( 1 == dns_count ) {
                blobmsg_add_string( reply, "dhcpSecondDns", blobmsg_get_string( cur_table ) );
            }
            else {
                continue;
            }

            dns_count++;
        }
    }

    /* If 'dns_count' is 0, it means that the DNS information is not analysis success.
     * So read the DNS information from the configuration file
     */
    if( 0 == dns_count ) {
        cm_lan_cfg = util_get_vltree_node( &cm_lan_vltree, VLTREE_CM_TREE, "lan" );
        if( !cm_lan_cfg ) {
           cfmanager_log_message( L_ERR, "No lan information found in %s\n", CFMANAGER_CONFIG_NAME );
        }
        else {
            blobmsg_parse( lan_policy,
                __LAN_MAX,
                tb_lan,
                blob_data( cm_lan_cfg->cf_section.config ),
                blob_len( cm_lan_cfg->cf_section.config ) );

            if ( !tb_lan[LAN_DHCP_FIRSTDNS] && !tb_lan[LAN_DHCP_SECONDDNS] ) {
                blobmsg_add_string( reply, "dhcpFirstDns", util_blobmsg_get_string( tb_lan[LAN_IP4ADDRESS], "-" ) );
            }
            else {
                blobmsg_add_string( reply, "dhcpFirstDns", util_blobmsg_get_string( tb_lan[LAN_DHCP_FIRSTDNS], "-" ) );
                blobmsg_add_string( reply, "dhcpSecondDns", util_blobmsg_get_string( tb_lan[LAN_DHCP_SECONDDNS], "-" ) );
            }
        }
    }
}

//=============================================================================
static void
sgreq_add_wan_rate(
    struct blob_buf *reply,
    struct blob_attr *data,
    int wan_type
)
//=============================================================================
{
    enum {
        WAN_THROUGHPUT,

        __WAN_MAX
    };

    enum {
        THROUGHPUT_TX_BPS,
        THROUGHPUT_RX_BPS,

        __THROUGHPUT_MAX
    };

    static const struct blobmsg_policy wan_status_policy[__WAN_MAX] = {
        [WAN_THROUGHPUT] = { .name = "throughput", .type = BLOBMSG_TYPE_TABLE }
    };

    static const struct blobmsg_policy wan_throughput_policy[__THROUGHPUT_MAX] = {
        [THROUGHPUT_TX_BPS] = { .name = "tx_bps", .type = BLOBMSG_TYPE_STRING },
        [THROUGHPUT_RX_BPS] = { .name = "rx_bps", .type = BLOBMSG_TYPE_STRING }
    };

    if( !data ) {
        return;
    }

    struct blob_attr *wan_status_tb[__WAN_MAX];
    struct blob_attr *throughput_tb[__THROUGHPUT_MAX];

    blobmsg_parse( wan_status_policy,
        __WAN_MAX,
        wan_status_tb,
        blobmsg_data( data ),
        blobmsg_data_len( data ) );

    if( !wan_status_tb[WAN_THROUGHPUT] ) {
        return;
    }

    blobmsg_parse( wan_throughput_policy,
        __THROUGHPUT_MAX,
        throughput_tb,
        blobmsg_data( wan_status_tb[WAN_THROUGHPUT] ),
        blobmsg_data_len( wan_status_tb[WAN_THROUGHPUT] ) );

    if( WAN0 == wan_type ) {
        blobmsg_add_string( reply, "wanTxRate",
            util_blobmsg_get_string( throughput_tb[THROUGHPUT_TX_BPS], 0 ) );

        blobmsg_add_string( reply, "wanRxRate",
            util_blobmsg_get_string( throughput_tb[THROUGHPUT_RX_BPS], 0 ) );
    }
#ifdef DOUBLE_WAN
    else {
        blobmsg_add_string( reply, "wan1TxRate",
            util_blobmsg_get_string( throughput_tb[THROUGHPUT_TX_BPS], 0 ) );

        blobmsg_add_string( reply, "wan1RxRate",
            util_blobmsg_get_string( throughput_tb[THROUGHPUT_RX_BPS], 0 ) );
    }
#endif
}

//=============================================================================
static void
sgreq_get_wan_rate_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_buf *reply = (struct blob_buf *)req->priv;

    static const struct blobmsg_policy wan_policy[] = {
        [WAN0] = { .name = "wan0", .type = BLOBMSG_TYPE_TABLE },
#ifdef DOUBLE_WAN
        [WAN1] = { .name = "wan1", .type = BLOBMSG_TYPE_TABLE }
#endif
    };
    int wan_policy_size = ( sizeof( wan_policy ) / sizeof( wan_policy[0] ) );

    struct blob_attr *wan_tb[wan_policy_size];
    blobmsg_parse( wan_policy,
        wan_policy_size,
        wan_tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( wan_tb[WAN0] ) {
        sgreq_add_wan_rate( reply, wan_tb[WAN0], WAN0 );
    }
#ifdef DOUBLE_WAN
    if( wan_tb[WAN1] ) {
        sgreq_add_wan_rate( reply, wan_tb[WAN1], WAN1 );
    }
#endif
}

//=============================================================================
static void
sgreq_get_wan_link_rate(
    struct blob_buf *reply,
    int wan_type
)
//=============================================================================
{
#define WAN_LINK_0_RATE    "0"
#define WAN_LINK_100_RATE  "1"
#define WAN_LINK_1000_RATE "2"

    int port = 0;
    char cmd[BUF_LEN_64] = { 0 };
    char value[BUF_LEN_128] = { 0 };
    const char *key = NULL;
    int ret = 0;

    if( WAN0 == wan_type ) {
        port = WAN_LINK_PORT;
        key = "wanLinkRate";
    }
    else {
        port = WAN1_LINK_PORT;
        key = "wan1LinkRate";
    }

    snprintf( cmd, sizeof( cmd ), "swconfig dev switch0 show | grep port:%d", port );
    ret = util_run_shell_command( cmd, value, sizeof( value ) );
    if( ret ) {
        blobmsg_add_string( reply, key, WAN_LINK_0_RATE );
        return;
    }

    if( strstr( value, "100baseT" ) ) {
        blobmsg_add_string( reply, key, WAN_LINK_100_RATE );
    }
    else if( strstr( value, "1000baseT" ) ) {
        blobmsg_add_string( reply, key, WAN_LINK_1000_RATE );
    }
    else {
        blobmsg_add_string( reply, key, WAN_LINK_0_RATE );
    }
}

//=============================================================================
void
sgreq_get_device(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    int ret = 0;
    char value[BUF_LEN_128];
    char format_mac[MAC_STR_MAX_LEN+1];
    char time_str[BUF_LEN_32] = { 0 };
#ifdef DOUBLE_WAN
    char path[LOOKUP_STR_SIZE] = { 0 };
#endif
    struct sysinfo s_info;
    uint32_t search_id;
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    memset(value, 0, sizeof(value));
    ret = util_read_file_content("/proc/gxp/dev_info/dev_id", value, sizeof(value));
    if( !ret )
        blobmsg_add_string( b, "devProductName", value );

    memset(value, 0, sizeof(value));
    ret = util_read_file_content("/proc/gxp/dev_info/dev_mac", value, sizeof(value));
    if( !ret ) {
        memset( format_mac, 0, sizeof( format_mac ) );
        util_str_erase_colon( value, format_mac );
        blobmsg_add_string( b, "devMac", format_mac );
    }

    memset(value, 0, sizeof(value));
    ret = util_read_file_content("/proc/gxp/dev_info/security/sn", value, sizeof(value));
    if( !ret )
        blobmsg_add_string( b, "devSn", value );

    memset(value, 0, sizeof(value));
    ret = util_read_file_content("/proc/gxp/dev_info/dev_rev", value, sizeof(value));
    if( !ret )
        blobmsg_add_string( b, "devHwVersion", value );

    memset(value, 0, sizeof(value));
    ret = util_read_file_content("/tmp/gs_version", value, sizeof(value));
    if( !ret )
        blobmsg_add_string( b, "devSoftVersion", value );

#ifdef DOUBLE_WAN
    memset(value, 0, sizeof(value));
    snprintf( path, sizeof( path ), "%s.wan.wan1Enable", CFMANAGER_CONFIG_NAME );
    config_uci_get_option( path, value, sizeof( value ) );
    if( '1' == value[0] ) {
        blobmsg_add_u8( b, "wan1Enable", true );
    }
    else {
        blobmsg_add_u8( b, "wan1Enable", false );
    }
#endif

    sysinfo( &s_info );
    blobmsg_add_u32( b, "devUptime", s_info.uptime );

    //time
    utils_time_value2time_str( time( NULL ), time_str, BUF_LEN_32 );
    blobmsg_add_string( b, "devSystemTime", time_str );

    //lan data
    if ( ctx && !ubus_lookup_id( ctx, "network.interface.lan0_zone0", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status", NULL, sgreq_get_dev_lan0_zone0_cb, b, 1000 );
    }

    //wan data
    if ( ctx && !ubus_lookup_id( ctx, "network.interface.wan0", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status", NULL, sgreq_get_dev_wan0_cb, b, 1000 );
    }

#ifdef DOUBLE_WAN
    if ( ctx && !ubus_lookup_id( ctx, "network.interface.wan1", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status", NULL, sgreq_get_dev_wan1_cb, b, 1000 );
    }
#endif

    blob_buf_init( &send_buf, 0 );
    blobmsg_add_string( &send_buf, "unit",  "bit" );

    if ( ctx && !ubus_lookup_id( ctx, "controller.core", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status_wan", send_buf.head, sgreq_get_wan_rate_cb, b, 1000 );
    }
    blob_buf_free( &send_buf );

    sgreq_get_wan_link_rate( b, WAN0 );
#ifdef DOUBLE_WAN
    sgreq_get_wan_link_rate( b, WAN1 );
#endif
    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_guest_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__GUEST_SSID_MAX];
    int i = 0;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cm_cfg = util_get_vltree_node( &cm_guest_ssid_vltree, VLTREE_CM_TREE, "guest_ssid" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No guest_ssid information found in %s\n", CFMANAGER_CONFIG_NAME );
        error_state = ERRCODE_INTERNAL_ERROR;
        goto return_value;
    }

    blobmsg_parse( guest_ssid_policy,
        __GUEST_SSID_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    for( i = 0; i < __GUEST_SSID_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        blobmsg_add_blob( b, tb[i] );
    }

return_value:
    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_mesh_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__MESH_SSID_ATTR_MAX];
    int i = 0;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cm_cfg = util_get_vltree_node( &cm_mesh_ssid_vltree, VLTREE_CM_TREE, "mesh_ssid" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No mesh_ssid information found in %s\n", CFMANAGER_CONFIG_NAME );
        error_state = ERRCODE_INTERNAL_ERROR;
        goto return_value;
    }

    blobmsg_parse( mesh_ssid_attrs_policy,
        __MESH_SSID_ATTR_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    for( i = 0; i < __MESH_SSID_ATTR_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        blobmsg_add_blob( b, tb[i] );
    }

return_value:
    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );

    return;
}

//=============================================================================
void
sgreq_get_acl(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    void *array;
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    // TODO: data waiting to be supplemented
    //If portMap has no data, it also needs to reply to an empty array
    array = blobmsg_open_array( b, "portMap" );
    blobmsg_close_array( b, array );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );
    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_get_qos(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    void *array;
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    // TODO: data waiting to be supplemented
    //If clientQos and ssidQos has no data, it also needs to reply to an empty array
    array = blobmsg_open_array( b, "clientQos" );
    blobmsg_close_array( b, array );
    array = blobmsg_open_array( b, "ssidQos" );
    blobmsg_close_array( b, array );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );
    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_get_network_interface(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    void *array;
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    // TODO: data waiting to be supplemented
    //If networkInf has no data, it also needs to reply to an empty array
    array = blobmsg_open_array( b, "networkInf" );
    blobmsg_close_array( b, array );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );
    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_get_controller(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
   int error_state = 0;
   struct blob_buf *b;
   struct cm_config *cm_cfg = NULL;
   struct blob_attr *tb[__CONTROLLER_MAX];
   int i = 0;

   if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
       b = (struct blob_buf *)sg_params->data;
   }
   else {
       b = &reply;
       blob_buf_init( b, 0 );
   }

   cm_cfg = util_get_vltree_node( &cm_controller_vltree, VLTREE_CM_TREE, "main" );
   if( !cm_cfg ) {
       cfmanager_log_message( L_DEBUG, "No basic information found in %s\n", CFMANAGER_CONFIG_NAME );
       error_state = ERRCODE_INTERNAL_ERROR;
       goto return_value;
   }

   blobmsg_parse( controller_policy,
       __CONTROLLER_MAX,
       tb,
       blob_data( cm_cfg->cf_section.config ),
       blob_len( cm_cfg->cf_section.config ) );

   for( i = 0; i < __CONTROLLER_MAX; i++ ) {
       if( !tb[i] ) {
           continue;
       }

       blobmsg_add_blob( b, tb[i] );
   }

return_value:
   //When sg issues "get all", the return information is returned from sgreq_get_all
   if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
       return;
   }

   sgreq_return_must_info_to_sg( b, sg_params, error_state );

   ubus_send_reply( ctx, req, b->head );
   blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_basic(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__BASIC_SET_MAX];
    int i = 0, index = 0;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cm_cfg = util_get_vltree_node( &cm_basic_system_vltree, VLTREE_CM_TREE, "basic" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No basic information found in %s\n", CFMANAGER_CONFIG_NAME );
        error_state = ERRCODE_INTERNAL_ERROR;
        goto return_value;
    }

    blobmsg_parse( basic_system_policy,
        __BASIC_SET_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    for( i = 0; i < __BASIC_SET_MAX; i++ ) {
        if( !tb[i] ) {
            continue;
        }

        blobmsg_add_blob( b, tb[i] );

        if ( i == BASIC_COUNTRY_CODE ) {
            for ( index = 0; index < countries_num; index++ ) {
                if ( 0 == strncmp( countries[index].value, blobmsg_get_string(tb[BASIC_COUNTRY_CODE]),
                    strlen(countries[index].value) ) ) {
                    blobmsg_add_string( b, "countryLable", countries[index].label );
                    break;
                }
            }
        }

        if ( i == BASIC_TIME_ZONE ) {
            for ( index = 0; index < timezones_name_num; index++ ) {
                if ( 0 == strncmp( timezones_name[index].value, blobmsg_get_string(tb[BASIC_TIME_ZONE]), 
                    strlen(timezones_name[index].value) ) ) {
                    blobmsg_add_string( b, "timeZoneLable", timezones_name[index].label );
                    break;
                }
            }
        }

    }

return_value:
    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_extern_sys_log(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[_EXTERNAL_LOG_MAX];
    int i = 0;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cm_cfg = util_get_vltree_node( &cm_extern_log_vltree, VLTREE_CM_TREE, "debug" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No debug information found in %s\n", CFMANAGER_CONFIG_NAME );
        error_state = ERRCODE_INTERNAL_ERROR;
        goto return_value;
    }

    blobmsg_parse( cm_extern_sys_log_policy,
        _EXTERNAL_LOG_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    for (i = 0; i < _EXTERNAL_LOG_MAX; i++) {
        if( !tb[i] ) {
            continue;
        }

        blobmsg_add_blob( b, tb[i] );
    }

return_value:
    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );

}

//=============================================================================
void
sgreq_get_email(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[_CM_EMAIL_MAX];
    int i = 0;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cm_cfg = util_get_vltree_node( &cm_CM_EMAIL_vltree, VLTREE_CM_TREE, "email" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No email information found in %s\n", CFMANAGER_CONFIG_NAME );
        error_state = ERRCODE_INTERNAL_ERROR;
        goto return_value;
    }

    blobmsg_parse( cm_email_policy,
        _CM_EMAIL_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    for (i = 0; i < _CM_EMAIL_MAX; i++) {
        if( !tb[i] ) {
            continue;
        }

        blobmsg_add_blob( b, tb[i] );
    }

return_value:
    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );

}

//=============================================================================
void
sgreq_get_notification(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[_CM_NOTIFY_MAX];
    int i = 0;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cm_cfg = util_get_vltree_node( &cm_notification_vltree, VLTREE_CM_TREE, "notification" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_DEBUG, "No notification information found in %s\n", CFMANAGER_CONFIG_NAME );
        error_state = ERRCODE_INTERNAL_ERROR;
        goto return_value;
    }

    blobmsg_parse( cm_notification_policy,
        _CM_NOTIFY_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    for (i = 0; i < _CM_NOTIFY_MAX; i++) {
        if( !tb[i] ) {
            continue;
        }

        blobmsg_add_blob( b, tb[i] );
    }

return_value:
    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_ap(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct blob_attr *tb[__GET_AP_INFO_MAX];
    char *ap_mac = NULL;

    if( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    blobmsg_parse( get_ap_info_policy,
        __GET_AP_INFO_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if( !tb[GET_AP_INFO_MAC] ) {
        cfmanager_log_message( L_ERR, "miss ap mac\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    ap_mac = blobmsg_get_string( tb[GET_AP_INFO_MAC] );

    error_state = sgreq_get_info_by_cmtree( b, &cm_ap_vltree,
            cm_ap_policy, ap_mac, __CM_AP_MAX );

return_value:

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_access(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    void *a;
    void *t;

    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb1[__CM_PARSE_ACCESS_MAX];
    struct blob_attr *tb2[__GLOBAL_ACCESS_ATTR_MAX];
    struct blob_attr *tb3[__SCHEDULE_ACCESS_ATTR_MAX];

    if( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    a = blobmsg_open_array( b, "blackList" );
    vlist_for_each_element( &cm_access_vltree, cm_cfg, node ) {
        const char *mac = NULL;
        const char *name = NULL;
        const char *os = NULL;
        const char *black = NULL;
        blobmsg_parse( cm_access_parse_policy,
            __CM_PARSE_ACCESS_MAX,
            tb1,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        if ( !tb1[ CM_PARSE_ACCESS_MAC ] )
            continue;

        mac = blobmsg_get_string( tb1[ CM_PARSE_ACCESS_MAC ] );

        if ( !tb1[ CM_PARSE_ACCESS_BLACK ] )
            continue;

        black = blobmsg_get_string( tb1[ CM_PARSE_ACCESS_BLACK ] );

        if ( tb1[ CM_PARSE_ACCESS_HOSTNAME ] )
            name = blobmsg_get_string( tb1[ CM_PARSE_ACCESS_HOSTNAME ] );

        if ( tb1[ CM_PARSE_ACCESS_CLIENTOS] )
            os = blobmsg_get_string( tb1[ CM_PARSE_ACCESS_CLIENTOS ] );

        if ( black && black[0] == '1' ) {
            t = blobmsg_open_table( b, NULL );
            blobmsg_add_string( b, "mac", mac );
            blobmsg_add_string( b, "name", name ? name : "" );
            blobmsg_add_string( b, "clientOs", os ? os : "0" );
            blobmsg_close_table( b, t );
        }
    }
    blobmsg_close_array( b, a );


    t = blobmsg_open_table( b, "accessSiteControl" );
    cm_cfg = util_get_vltree_node( &cm_global_access_vltree, VLTREE_CM_TREE, "global_access" );
    if( cm_cfg ) {
        unsigned int rem;
        struct blob_attr *cur;

        blobmsg_parse( global_access_attrs_policy,
            __GLOBAL_ACCESS_ATTR_MAX,
            tb2,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        blobmsg_add_u8( b, global_access_attrs_policy[GLOBAL_ACCESS_ATTR_ENABLE].name,
            util_blobmsg_get_bool( tb2[ GLOBAL_ACCESS_ATTR_ENABLE ], false ) );


        a = blobmsg_open_array( b, global_access_attrs_policy[GLOBAL_ACCESS_ATTR_FORBIDURL].name );
        if ( tb2[GLOBAL_ACCESS_ATTR_FORBIDURL] ) {
            blobmsg_for_each_attr( cur, tb2[GLOBAL_ACCESS_ATTR_FORBIDURL], rem ) {
                blobmsg_add_string( b, NULL, blobmsg_get_string( cur ) );
            }
        }
        blobmsg_close_array( b, a );
    }
    else {
        cfmanager_log_message( L_DEBUG, "can not find global_access\n" );
        blobmsg_add_u8( b, "enable", 0 );
        a = blobmsg_open_array( b, "forbidUrl" );
        blobmsg_close_array( b, a );
    }

    blobmsg_close_table( b, t );


    t = blobmsg_open_table( b, "accessTimeControl" );

    cm_cfg = util_get_vltree_node( &cm_schedule_access_vltree,
                                   VLTREE_CM_TREE, "schedule_access" );
    if( cm_cfg ) {
        blobmsg_parse( schedule_access_attrs_policy,
        __SCHEDULE_ACCESS_ATTR_MAX,
        tb3,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

        blobmsg_add_u8( b,
            schedule_access_attrs_policy[SCHEDULE_ACCESS_ATTR_ENABLE].name,
            util_blobmsg_get_bool( tb3[SCHEDULE_ACCESS_ATTR_ENABLE], false ) );

        blobmsg_add_string( b,
            schedule_access_attrs_policy[SCHEDULE_ACCESS_ATTR_WEEK].name,
            util_blobmsg_get_string( tb3[SCHEDULE_ACCESS_ATTR_WEEK], "" ) );

        blobmsg_add_string( b,
            schedule_access_attrs_policy[SCHEDULE_ACCESS_ATTR_TIMESTART].name,
            util_blobmsg_get_string( tb3[SCHEDULE_ACCESS_ATTR_TIMESTART], "" ) );

        blobmsg_add_string( b,
            schedule_access_attrs_policy[SCHEDULE_ACCESS_ATTR_TIMEEND].name,
            util_blobmsg_get_string( tb3[SCHEDULE_ACCESS_ATTR_TIMEEND], "" ) );
    }
    else {
        blobmsg_add_u8( b,
            schedule_access_attrs_policy[SCHEDULE_ACCESS_ATTR_ENABLE].name,
            0 );
    }
    blobmsg_close_table( b, t );


    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_upgrade_auto(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    char timer_buf[BUF_LEN_32] = { 0 };

    if( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    sgreq_get_info_by_cmtree( b, &cm_upgrade_auto_vltree,
        upgrade_auto_policy, "upgrade_auto", __UPGRADE_AUTO_MAX );

    utils_time_value2time_str( time( NULL ), timer_buf, BUF_LEN_32 );
    blobmsg_add_string( b, "systemTime", timer_buf );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_vpn_client(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    void *a;
    void *t;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb1[__VPN_CLIENT_MAX];
    struct blob_attr *tb2[__WAN_MAX];
    int i = 0;
    char *conn_status2str[__VPN_CONN_STATUS_MAX] = {
        "0",
        "1",
        "2"
    };

    if( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    a = blobmsg_open_array( b, "vpnServiceList" );
    vlist_for_each_element( &cm_vpn_client_vltree, cm_cfg, node ) {
        int id;
        char ifname[BUF_LEN_16] = { 0 };
        int enable;
        int status = __VPN_CONN_STATUS_MAX;

        blobmsg_parse( vpn_client_policy,
            __VPN_CLIENT_MAX,
            tb1,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        t = blobmsg_open_table( b, NULL );
        for( i = 0; i < __VPN_CLIENT_MAX; i++ ) {
            if( !tb1[i] ) {
                continue;
            }

            blobmsg_add_blob( b, tb1[i] );
        }

        // Get vpn status
        id = atoi( blobmsg_get_string(tb1[VPN_CLIENT_ID]) );
        snprintf( ifname, sizeof(ifname), "vpn%d", id + VPN_CLIENT_ID_OFFSET );
        enable = blobmsg_get_bool( tb1[VPN_CLIENT_ENABLE] );
        if ( !enable ) {
            blobmsg_add_string( b, "status", conn_status2str[VPN_CONN_STATUS_DISCONNECTED] );
        }
        else {
            if ( util_blobmsg_get_int(tb1[VPN_CLIENT_TYPE], VPN_CLIENT_TYPE_L2TP) == VPN_CLIENT_TYPE_IPSEC ) {
                status = vpn_get_ipsec_status( util_blobmsg_get_string(tb1[VPN_CLIENT_IPSEC_P1_REMOTE_GW], "") );
            }
            else {
                cfubus_get_vpn_status( ifname, &status );
            }

            /*
             * Web will get vpn client status immediately once vpn service config changed.
             * If vpn has connected and only vpn type changed, we should change its status to VPN_CONN_STATUS_CONNECTING.
             * Next time, network may have reloaded and web can get newest status.
             */
            if ( vpn_client_chg && status == VPN_CONN_STATUS_CONNECTED ) {
                status = VPN_CONN_STATUS_CONNECTING;
            }

            if ( status == __VPN_CONN_STATUS_MAX ) {
                cfmanager_log_message( L_ERR, "Invalid connection status, set to DISCONNECTED!\n" );
                status = VPN_CONN_STATUS_DISCONNECTED;
            }
            blobmsg_add_string( b, "status", conn_status2str[status] );
        }

        blobmsg_close_table( b, t );
    }

    // Return these vpn configured on wan status.
    cm_cfg = util_get_vltree_node( &cm_wan_vltree, VLTREE_CM_TREE, "wan" );
    if ( cm_cfg ) {
        int type;
        int status;
#ifdef DOUBLE_WAN
        int wan1_enable = 0;
#endif

        blobmsg_parse( wan_policy,
            __WAN_MAX,
            tb2,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        type = atoi( blobmsg_get_string(tb2[WAN_TYPE]) );
        if ( type == WAN_TYPE_L2TP || type == WAN_TYPE_PPTP ) {
            char *ifname = "vpn0";
            char id[BUF_LEN_16]  = { 0 };

            snprintf( id, sizeof(id), "%d", VPN_ID_ON_WAN0 );
            t = blobmsg_open_table( b, NULL );
            blobmsg_add_string( b, "id", id );
            blobmsg_add_string( b, "name", "WAN1_VPN" );
            if ( type == WAN_TYPE_L2TP ) {
                blobmsg_add_string( b, "type", "0" );
                blobmsg_add_string( b, "l2tpServer", blobmsg_get_string(tb2[WAN_L2TPSERVER]));
                blobmsg_add_string( b, "l2tpUser", blobmsg_get_string(tb2[WAN_L2TPUSER]));
            }
            else {
                blobmsg_add_string( b, "type", "1" );
                blobmsg_add_string( b, "pptpServer", blobmsg_get_string(tb2[WAN_PPTPSERVER]));
                blobmsg_add_string( b, "pptpUser", blobmsg_get_string(tb2[WAN_PPTPUSER]));
            }

            cfubus_get_vpn_status( ifname, &status );
            blobmsg_add_string( b, "status", conn_status2str[status] );
            blobmsg_close_table( b, t );
        }

#ifdef DOUBLE_WAN
        wan1_enable = tb2[WAN1_ENABLE] ? blobmsg_get_bool( tb2[WAN1_ENABLE] ) : 0;
        if ( wan1_enable ) {
            type = atoi( blobmsg_get_string(tb2[WAN1_TYPE]) );
            if ( type == WAN_TYPE_L2TP || type == WAN_TYPE_PPTP ) {
                char *ifname = "vpn1";
                char id[BUF_LEN_16]  = { 0 };

                snprintf( id, sizeof(id), "%d", VPN_ID_ON_WAN1 );
                t = blobmsg_open_table( b, NULL );
                blobmsg_add_string( b, "id", id );
                blobmsg_add_string( b, "name", "WAN2_VPN" );
                if ( type == WAN_TYPE_L2TP ) {
                    blobmsg_add_string( b, "type", "0" );
                    blobmsg_add_string( b, "l2tpServer", blobmsg_get_string(tb2[WAN1_L2TPSERVER]));
                    blobmsg_add_string( b, "l2tpUser", blobmsg_get_string(tb2[WAN1_L2TPUSER]));
                }
                else {
                    blobmsg_add_string( b, "type", "1" );
                    blobmsg_add_string( b, "pptpServer", blobmsg_get_string(tb2[WAN1_PPTPSERVER]));
                    blobmsg_add_string( b, "pptpUser", blobmsg_get_string(tb2[WAN1_PPTPUSER]));
                }

                cfubus_get_vpn_status( ifname, &status );
                blobmsg_add_string( b, "status", conn_status2str[status] );
                blobmsg_close_table( b, t );
            }
        }
#endif
    }
    blobmsg_close_array( b, a );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
    vpn_client_chg = 0;
}

//=============================================================================
void
sgreq_get_vpn_split_tunneling(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    void *a1, *a2, *a3;
    void *t1, *t2;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__VPN_SPLIT_MAX];
    struct blob_attr *cur = NULL;
    int rem;

    if( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    a1 = blobmsg_open_array( b, "vpnSplit" );
    vlist_for_each_element( &cm_vpn_split_vltree, cm_cfg, node ) {
        blobmsg_parse( vpn_split_policy,
            __VPN_SPLIT_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        t1 = blobmsg_open_table( b, NULL );
        blobmsg_add_string( b, "id", blobmsg_get_string(tb[VPN_SPLIT_SERVICE_ID]) );
        blobmsg_add_string( b, "mode", blobmsg_get_string(tb[VPN_SPLIT_MODE]) );

        // Add serviceAddrList array
        a2 = blobmsg_open_array( b, "serviceAddrList" );
        blobmsg_for_each_attr( cur, tb[VPN_SPLIT_SERVICE_ADDR_LIST], rem ) {
            blobmsg_add_string( b, NULL, blobmsg_get_string(cur) );
        }
        blobmsg_close_array( b, a2 );

        // Add devList array
        a3 = blobmsg_open_array( b, "devList" );
        blobmsg_for_each_attr( cur, tb[VPN_SPLIT_DEV_LIST], rem ) {
            char dev_name[BUF_LEN_512];
            char *dev_mac;
            char *data, *p;

            data = blobmsg_get_string(cur);
            p = strchr( data, '/' );
            dev_mac = p + 1;
            memset( dev_name, 0, sizeof(dev_name) );
            memcpy( dev_name, data, p - data );
            t2 = blobmsg_open_table( b, NULL );
            if ( strlen(dev_name) ) {
                blobmsg_add_string( b, "devName", dev_name );
            }
            blobmsg_add_string( b, "devMac", dev_mac );
            blobmsg_close_table( b, t2 );
        }
        blobmsg_close_array( b, a3 );

        blobmsg_close_table( b, t1 );
    }
    blobmsg_close_array( b, a1 );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_port_mapping(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    void *a;
    void *t;
    int i;
    struct blob_attr *tb[__PORT_MAPPING_MAX];
    struct uci_element *e = NULL;
    struct uci_package *package = NULL;

    package = cfparse_init_package( CFMANAGER_CONFIG_NAME );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load %s package failed\n", CFMANAGER_CONFIG_NAME );
        return;
    }

    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    a = blobmsg_open_array( b, "rule" );
    uci_foreach_element( &package->sections, e ) {
        struct blob_buf tmp_blob;
        struct uci_section *s = uci_to_section( e );
        const struct uci_blob_param_list* uci_blob_list = NULL;

        if( strcmp(cm_vltree_info[CM_VLTREE_PORT_MAPPING].key, s->type) ) {
            continue;
        }

        uci_blob_list = cm_vltree_info[CM_VLTREE_PORT_MAPPING].policy_list;
        memset( &tmp_blob, 0, sizeof( struct blob_buf ) );
        blob_buf_init( &tmp_blob, 0 );
        uci_to_blob( &tmp_blob, s, uci_blob_list );

        blobmsg_parse( port_mapping_policy,
            __PORT_MAPPING_MAX,
            tb,
            blob_data( tmp_blob.head ),
            blob_len( tmp_blob.head ) );

        t = blobmsg_open_table( b, NULL );
        for ( i = 0; i < __PORT_MAPPING_MAX; i++ ) {
            if ( !tb[i] ) {
                continue;
            }
            blobmsg_add_blob( b, tb[i] );
        }
        blobmsg_close_table( b, t );
        blob_buf_free( &tmp_blob );
    }

    blobmsg_close_array( b, a );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_dmz(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__DMZ_MAX];
    int i;

    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cm_cfg = util_get_vltree_node( &cm_dmz_vltree, VLTREE_CM_TREE, "dmz" );
    if ( cm_cfg ) {
        blobmsg_parse( dmz_policy,
            __DMZ_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __DMZ_MAX; i++ ) {
            if ( !tb[i] ) {
                continue;
            }
            blobmsg_add_blob( b, tb[i] );
        }
    }

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_upnp(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct upnpd_config_parse *upnpd_cfpar = NULL;
    struct blob_attr *tb1[__UPNP_MAX];
    struct blob_attr *tb2[__UPNPD_MAX];
    int i;
    void *a;
    void *t;
    char *lease_file;
    FILE *fp = NULL;
    char *line = NULL;
    size_t len;

    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cm_cfg = util_get_vltree_node( &cm_upnp_vltree, VLTREE_CM_TREE, "upnp" );
    if ( cm_cfg ) {
        blobmsg_parse( upnp_policy,
            __UPNP_MAX,
            tb1,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __UPNP_MAX; i++ ) {
            if ( !tb1[i] || i == UPNP_MAPPING_TABLE ) {
                continue;
            }
            blobmsg_add_blob( b, tb1[i] );
        }

        // Fill mapping table
        if ( blobmsg_get_bool(tb1[UPNP_ENABLE]) ) {
            upnpd_cfpar = util_get_vltree_node( &upnpd_vltree, VLTREE_UPNPD, "config" );
            if ( !upnpd_cfpar ) {
                cfmanager_log_message( L_DEBUG, "Cannot find node upnpd\n" );
                goto out;
            }

            blobmsg_parse( upnpd_policy,
                __UPNPD_MAX,
                tb2,
                blob_data( upnpd_cfpar->cf_section.config ),
                blob_len( upnpd_cfpar->cf_section.config  ) );

            lease_file = blobmsg_get_string( tb2[UPNPD_UPNP_LEASE_FILE] );
            fp = fopen( lease_file, "r" );
            if ( !fp ) {
                cfmanager_log_message( L_DEBUG, "Open %s failed, %s\n", lease_file, strerror(errno) );
                goto out;
            }

            a = blobmsg_open_array( b, "mapping_table" );
            line = NULL;
            while( getline(&line, &len, fp) !=  -1 ) {
                char *proto, *ext_port, *ip, *inter_port, *name;

                if ( line[strlen(line) - 1] == '\n' ) {
                    line[strlen(line) - 1] = '\0';
                }
                proto = strtok( line, ":" ); // proto
                ext_port = strtok( NULL, ":" ); // external port
                ip = strtok( NULL, ":" ); // ip
                inter_port = strtok( NULL, ":" ); // internal port
                strtok( NULL, ":" ); // Don't care.
                name = strtok( NULL, ":" ); // name

                t = blobmsg_open_table( b, NULL );
                blobmsg_add_string( b, "description", name );
                blobmsg_add_string( b, "ip", ip );
                blobmsg_add_string( b, "externalPort", ext_port );
                blobmsg_add_string( b, "internalPort", inter_port );
                blobmsg_add_string( b, "proto", proto );
                blobmsg_close_table( b, t );
            }
            blobmsg_close_array( b, a );

            if ( line ) {
                free( line );
                line = NULL;
            }
            fclose( fp );
        }
    }

out:
    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_ddns(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct blob_attr *tb[__DDNS_MAX];
    int i;
    void *a;
    void *t;
    struct uci_element *e = NULL;
    struct uci_package *package = NULL;

    package = cfparse_init_package( CFMANAGER_CONFIG_NAME );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load %s package failed\n", CFMANAGER_CONFIG_NAME );
        return;
    }

    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    a = blobmsg_open_array( b, "list" );
    uci_foreach_element( &package->sections, e ) {
        struct blob_buf tmp_blob;
        struct uci_section *s = uci_to_section( e );
        const struct uci_blob_param_list* uci_blob_list = NULL;

        if( strcmp(cm_vltree_info[CM_VLTREE_DDNS].key, s->type) ) {
            continue;
        }

        uci_blob_list = cm_vltree_info[CM_VLTREE_DDNS].policy_list;
        //Failure to initialize may cause a crash
        memset( &tmp_blob, 0, sizeof( struct blob_buf ) );
        blob_buf_init( &tmp_blob, 0 );
        uci_to_blob( &tmp_blob, s, uci_blob_list );

        blobmsg_parse( ddns_policy,
            __DDNS_MAX,
            tb,
            blob_data( tmp_blob.head ),
            blob_len( tmp_blob.head ) );

        t = blobmsg_open_table( b, NULL );
        for ( i = 0; i < __DDNS_MAX; i++ ) {
            if ( !tb[i] ) {
                continue;
            }
            blobmsg_add_blob( b, tb[i] );
        }
        blobmsg_close_table( b, t );
        blob_buf_free( &tmp_blob );
    }
    blobmsg_close_array( b, a );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_ipv4_static_route(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    void *a;
    void *t;
    struct blob_attr *tb[__STATIC_ROUTE_IPV4_MAX];
    int i;
    struct uci_element *e = NULL;
    struct uci_package *package = NULL;

    package = cfparse_init_package( CFMANAGER_CONFIG_NAME );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load %s package failed\n", CFMANAGER_CONFIG_NAME );
        return;
    }

    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    a = blobmsg_open_array( b, "rule" );
    uci_foreach_element( &package->sections, e ) {
        struct blob_buf tmp_blob;
        struct uci_section *s = uci_to_section( e );
        const struct uci_blob_param_list* uci_blob_list = NULL;

        if( strcmp(cm_vltree_info[CM_VLTREE_STATIC_ROUTE_IPV4].key, s->type) ) {
            continue;
        }

        uci_blob_list = cm_vltree_info[CM_VLTREE_STATIC_ROUTE_IPV4].policy_list;
        memset( &tmp_blob, 0, sizeof( struct blob_buf ) );
        blob_buf_init( &tmp_blob, 0 );
        uci_to_blob( &tmp_blob, s, uci_blob_list );

        blobmsg_parse( static_route_ipv4_policy,
            __STATIC_ROUTE_IPV4_MAX,
            tb,
            blob_data( tmp_blob.head ),
            blob_len( tmp_blob.head ) );

        t = blobmsg_open_table( b, NULL );
        for ( i = 0; i < __STATIC_ROUTE_IPV4_MAX; i++ ) {
            if ( !tb[i] ) {
                continue;
            }
            blobmsg_add_blob( b, tb[i] );
        }
        blobmsg_close_table( b, t );
        blob_buf_free( &tmp_blob );
    }
    blobmsg_close_array( b, a );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}
//=============================================================================
static void
sgreq_stats_dev_ssid_by_ssid_id(
    struct blob_buf *b,
    char *ssid_id,
    int dev_type
)
//=============================================================================
{
    struct blob_attr *cm_tb[__CM_AP_MAX];
    struct cm_config *cm_cfg = NULL;
    char ssids[BUF_LEN_128] = { 0 };
    const char *dev_field = NULL;
    void *a = NULL;
    void *t = NULL;

    if( !b || !ssid_id ) {
        return;
    }

    if( DEV_AVAILABLE == dev_type ) {
        dev_field = "availableDev";
    }
    else {
        dev_field = "memberDev";
    }

    a = blobmsg_open_array( b, dev_field );

    vlist_for_each_element( &cm_ap_vltree, cm_cfg, node ) {

        memset( ssids, 0, sizeof( ssids ) );
        config_get_cm_ap_ssids( cm_cfg->cf_section.name, ssids, sizeof( ssids ) );

        if( ( DEV_AVAILABLE == dev_type && util_match_ssids( ssids, ssid_id ) ) ||
            ( DEV_MEMBER == dev_type && !util_match_ssids( ssids, ssid_id ) ) ) {

            continue;
        }

        t = blobmsg_open_table( b, NULL );

        blobmsg_parse( cm_ap_policy,
            __CM_AP_MAX,
            cm_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );

        blobmsg_add_string( b, "type", util_blobmsg_get_string( cm_tb[CM_AP_TYPE], "" ) );
        blobmsg_add_string( b, "mac", cm_cfg->cf_section.name );
        // TODO:Statistics on the number of ssids of devices
        blobmsg_add_string( b, "2g4Count", "0" );
        blobmsg_add_string( b, "5gCount", "0" );
        blobmsg_add_string( b, "totalSsidCount", "0" );

        blobmsg_close_table( b, t );
    }

    blobmsg_close_array( b, a );
}

//=============================================================================
void
sgreq_get_dev_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct blob_attr *tb[__DEV_SSID_MAX];
    char *ssid_id = NULL;

    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    blobmsg_parse( dev_ssid_policy,
        __DEV_SSID_MAX,
        tb,
        blobmsg_data( data),
        blobmsg_len(data ) );

    if( !tb[DEV_SSID_ID] ) {
        cfmanager_log_message( L_ERR, "miss ssid id\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    ssid_id = blobmsg_get_string( tb[DEV_SSID_ID] );

    sgreq_stats_dev_ssid_by_ssid_id( b, ssid_id, DEV_AVAILABLE );
    sgreq_stats_dev_ssid_by_ssid_id( b, ssid_id, DEV_MEMBER );

return_value:

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
static void
sgreq_add_ssid_name_to_ap_status(
    struct blob_buf *reply,
    const char *mac
)
//=============================================================================
{
    char ssids[BUF_LEN_256] = { 0 };
    char ssid_name[SSID_NAME_MAX_LEN+1] = { 0 };
    char *p = NULL;

    if( !reply || !mac ) {
        return;
    }

    config_get_cm_ap_ssids( mac, ssids, sizeof( ssids ) );
    if( '\0' == ssids[0] ) {
        cfmanager_log_message( L_ERR, "get %s ssids failed\n", mac );
        return;
    }

    p = strtok( ssids, " " );
    while( p ) {
        if( !config_get_ssid_enable( p ) ) {
            continue;
        }

        config_get_ssid_name( p, ssid_name, sizeof( ssid_name ) );
        if( '\0' == ssid_name[0] ) {
            continue;
        }

        blobmsg_add_string( reply, NULL, ssid_name );

        p = strtok( NULL, " " );
    }
}

//=============================================================================
static void
sgreq_add_link_info(
    const char *lan_name,
    const char *id,
    json_object *parsed_object,
    struct blob_buf *reply
)
//=============================================================================
{
    json_bool json_result = FALSE;
    json_object *ap_status_object = NULL;
    json_object *temp_object = NULL;

    if( !lan_name || !parsed_object || !reply || !id ) {
        return;
    }

    json_result = json_object_object_get_ex( parsed_object, lan_name, &ap_status_object );
    if( json_result ) {
        blobmsg_add_string( reply, "id", id );
        if( json_object_object_get_ex( ap_status_object, "link_status", &temp_object ) ) {
            blobmsg_add_string( reply, "status", json_object_get_string( temp_object ) );
        }
        if( json_object_object_get_ex( ap_status_object, "link_speed", &temp_object ) ) {
            blobmsg_add_string( reply, "speed", json_object_get_string( temp_object ) );
        }
        if( json_object_object_get_ex( ap_status_object, "link_duplex", &temp_object ) ) {
            blobmsg_add_string( reply, "duplex", json_object_get_string( temp_object ) );
        }
    }
}

//=============================================================================
static void
sgreq_get_ap_status_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_buf *reply = (struct blob_buf *)req->priv;
    char *str = NULL;
    char mac[18] = { 0 };
    int current_time_unix = 0;
    char value[128]= { 0 };
    json_object *parsed_object = NULL;
    json_object *ap_status_object = NULL;
    json_bool json_result = FALSE;
    json_object *temp_object = NULL;
    void *a = NULL;
    void *t = NULL;
    bool device_is_me = false;
    uint32_t search_id;
    int cnt = 0;
    int i = 0;
    double cpu = 0;
    double cpu_load_average = 0;

    str = blobmsg_format_json( msg, true );
    if ( !str ) {
        cfmanager_log_message( L_ERR, "get ap status can not format to json\n" );
        return;
    }

    parsed_object = json_tokener_parse( str );
    SAFE_FREE( str );

    if( !parsed_object ) {
        cfmanager_log_message( L_ERR, "Parse json string to json object failed\n" );
        return;
    }

    json_result = json_object_object_get_ex( parsed_object, "mac", &ap_status_object );
    if( json_result ) {
        util_str_erase_colon( json_object_get_string( ap_status_object ), mac );
    }
    else {
        if( parsed_object ) {
            json_object_put( parsed_object );
        }
        cfmanager_log_message( L_ERR, "get mac failed\n" );
        return;
    }

    device_is_me = utils_device_is_me( mac );

    json_result = json_object_object_get_ex( parsed_object, "product_model", &ap_status_object );
    if( json_result ) {
        blobmsg_add_string( reply, "productModel", json_object_get_string( ap_status_object ) );
    }

    json_result = json_object_object_get_ex( parsed_object, "part_number", &ap_status_object );
    if( json_result ) {
        blobmsg_add_string( reply, "partNumber", json_object_get_string( ap_status_object ) );
    }

#if 0
    json_result = json_object_object_get_ex( parsed_object, "clientbridge_isolated", &ap_status_object );
    if( json_result ) {
        blobmsg_add_u8( reply, "clientBridgeIsolated", json_object_get_boolean( ap_status_object ) );
    }
#endif

    json_result = json_object_object_get_ex( parsed_object, "uptime", &ap_status_object );
    if( json_result ) {
        snprintf( value, sizeof( value ), "%d", json_object_get_int( ap_status_object ) );
        blobmsg_add_string( reply, "upTime", value );
    }

    json_result = json_object_object_get_ex( parsed_object, "time", &ap_status_object );
    if( json_result ) {
        if( device_is_me ) {
            /* The format of the data transferred over is 'year-month-day hour-minute-second'.
             * For example '2021-12-13 20:26:26'.
             */
            current_time_unix = ( int )time( NULL );
        }
        else {
            current_time_unix = json_object_get_int( ap_status_object );
        }

        snprintf( value, sizeof( value ), "%d", current_time_unix );
        blobmsg_add_string( reply, "currentTime", value );
    }

    //version info
    json_result = json_object_object_get_ex( parsed_object, "version", &ap_status_object );
    if( json_result ) {
        if( json_object_object_get_ex( ap_status_object, "boot", &temp_object ) ) {
            blobmsg_add_string( reply, "bootVersion", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "firmware", &temp_object ) ) {
            blobmsg_add_string( reply, "firmwareVersion", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "updated", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "updated", value );
        }

        if( json_object_object_get_ex( ap_status_object, "update_required", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "updateRequired", value );
        }
    }

    //cpu info
    json_result = json_object_object_get_ex( parsed_object, "cpu", &ap_status_object );
    if( json_result ) {
        t = blobmsg_open_table( reply, "cpuInfo" );
        if( json_object_object_get_ex( ap_status_object, "cpu1", &temp_object ) ) {
            cpu = json_object_get_int( temp_object );
            cpu_load_average = cpu / 65536;
            snprintf( value, sizeof( value ), "%.2lf", cpu_load_average );
            blobmsg_add_string( reply, "cpu1", value );
        }

        if( json_object_object_get_ex( ap_status_object, "cpu5", &temp_object ) ) {
            cpu = json_object_get_int( temp_object );
            cpu_load_average = cpu / 65536;
            snprintf( value, sizeof( value ), "%.2lf", cpu_load_average );
            blobmsg_add_string( reply, "cpu5", value );
        }

        if( json_object_object_get_ex( ap_status_object, "cpu15", &temp_object ) ) {
            cpu = json_object_get_int( temp_object );
            cpu_load_average = cpu / 65536;
            snprintf( value, sizeof( value ), "%.2lf", cpu_load_average );
            blobmsg_add_string( reply, "cpu15", value );
        }
        blobmsg_close_table( reply, t );
    }

#if 0   //These parameters are not currently required
    //memory info
    json_result = json_object_object_get_ex( parsed_object, "memory", &ap_status_object );
    if( json_result ) {
        if( json_object_object_get_ex( ap_status_object, "total", &temp_object ) ) {
            blobmsg_add_string( reply, "total", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "used", &temp_object ) ) {
            blobmsg_add_string( reply, "used", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "free", &temp_object ) ) {
            blobmsg_add_string( reply, "free", json_object_get_string( temp_object ) );
        }
    }

    //throughput info
    json_result = json_object_object_get_ex( parsed_object, "throughput", &ap_status_object );
    if( json_result ) {
        if( json_object_object_get_ex( ap_status_object, "tx_bps", &temp_object ) ) {
            blobmsg_add_string( reply, "txBps", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "rx_bps", &temp_object ) ) {
            blobmsg_add_string( reply, "rxBps", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "tx_pps", &temp_object ) ) {
            blobmsg_add_string( reply, "txPps", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "rx_pps", &temp_object ) ) {
            blobmsg_add_string( reply, "rxPps", json_object_get_string( temp_object ) );
        }
    }

    //aggregate info
    json_result = json_object_object_get_ex( parsed_object, "aggregate", &ap_status_object );
    if( json_result ) {
        if( json_object_object_get_ex( ap_status_object, "tx_bytes", &temp_object ) ) {
            blobmsg_add_string( reply, "txBytes", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "tx_packets", &temp_object ) ) {
            blobmsg_add_string( reply, "txPackets", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "rx_bytes", &temp_object ) ) {
            blobmsg_add_string( reply, "rxBytes", json_object_get_string( temp_object ) );
        }

        if( json_object_object_get_ex( ap_status_object, "rx_packets", &temp_object ) ) {
            blobmsg_add_string( reply, "rxPackets", json_object_get_string( temp_object ) );
        }
    }
#endif

    //wifi2g4 info
    json_result = json_object_object_get_ex( parsed_object, "wifi2g4", &ap_status_object );
    if( json_result ) {
        if( json_object_object_get_ex( ap_status_object, "channel", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "2g4Channel", value );
        }

        if( json_object_object_get_ex( ap_status_object, "users", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "2g4ClientsCount", value );
        }

#if 0
        if( json_object_object_get_ex( ap_status_object, "guests", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "2g4Guests", value );
        }
#endif

        if( json_object_object_get_ex( ap_status_object, "txpower", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "2g4TxPower", value );
        }
    }

    //wifi5g info
    json_result = json_object_object_get_ex( parsed_object, "wifi5g", &ap_status_object );
    if( json_result ) {
        if( json_object_object_get_ex( ap_status_object, "channel", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "5gChannel", value );
        }

        if( json_object_object_get_ex( ap_status_object, "users", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "5gClientsCount", value );
        }

#if 0
        if( json_object_object_get_ex( ap_status_object, "guests", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "5gGuests", value );
        }
#endif

        if( json_object_object_get_ex( ap_status_object, "txpower", &temp_object ) ) {
            snprintf( value, sizeof( value ), "%d", json_object_get_int( temp_object ) );
            blobmsg_add_string( reply, "5gTxPower", value );
        }
    }

    //link info
    if( device_is_me ) {
        a = blobmsg_open_array( reply, "linkInfo" );

        //wan info
        t = blobmsg_open_table( reply, NULL );
        sgreq_add_link_info( "lan4", "wan0", parsed_object, reply );
        blobmsg_close_table( reply, t );

        if( config_wan1_is_enable() ) {
            t = blobmsg_open_table( reply, NULL );
            sgreq_add_link_info( "lan3", "wan1", parsed_object, reply );
            blobmsg_close_table( reply, t );
        }

        //lan info
        t = blobmsg_open_table( reply, NULL );
        sgreq_add_link_info( "lan0", "lan0", parsed_object, reply );
        blobmsg_close_table( reply, t );

        t = blobmsg_open_table( reply, NULL );
        sgreq_add_link_info( "lan1", "lan1", parsed_object, reply );
        blobmsg_close_table( reply, t );

        t = blobmsg_open_table( reply, NULL );
        sgreq_add_link_info( "lan2", "lan2", parsed_object, reply );
        blobmsg_close_table( reply, t );

        if( !config_wan1_is_enable() ) {
            t = blobmsg_open_table( reply, NULL );
            sgreq_add_link_info( "lan3", "lan3", parsed_object, reply );
            blobmsg_close_table( reply, t );
        }

        blobmsg_close_array( reply, a );
    }
    else {
        //slave link info
        json_result = json_object_object_get_ex( parsed_object, "link", &ap_status_object );
        if( json_result ) {
            blobmsg_add_json_element(
                reply,
                "linkInfo",
                ap_status_object );
        }
    }

    //ssid name info
    a = blobmsg_open_array( reply, "ssidName" );
    sgreq_add_ssid_name_to_ap_status( reply, mac );
    blobmsg_close_array( reply, a );

    //ipv4 info
    a = blobmsg_open_table( reply, "ipv4Info" );
    if( device_is_me ) {
        if ( req->ctx && !ubus_lookup_id( req->ctx, "network.interface.wan0", &search_id ) ) {
            ubus_invoke( req->ctx, search_id, "status", NULL, sgreq_get_dev_wan0_cb, reply, 1000 );
        }

        if( config_wan1_is_enable() ) {
            if ( req->ctx && !ubus_lookup_id( req->ctx, "network.interface.wan1", &search_id ) ) {
                ubus_invoke( req->ctx, search_id, "status", NULL, sgreq_get_dev_wan0_cb, reply, 1000 );
            }
        }
    }
    else {
        json_result = json_object_object_get_ex( parsed_object, "ipv4", &ap_status_object );
        if( json_result ) {
            cnt = json_object_array_length( ap_status_object );

            for( i = 0; i < cnt; i++ ) {
                temp_object = json_object_array_get_idx( ap_status_object, i );
                memset( value, 0, sizeof( value ) );
                strncpy( value, json_object_get_string( temp_object ), sizeof( value )-1 );

                if( 0 == strcmp( "$ipv4_broadcast", value ) ||
                    0 == strcmp( "$ipv4_mask", value ) ) {
                    continue;
                }

                blobmsg_add_string( reply, "ipv4Adress", value );
                break;
            }
        }
    }
    blobmsg_close_array( reply, a );

    if( parsed_object ) {
        json_object_put( parsed_object );
    }
}

//=============================================================================
void
sgreq_get_ap_status(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct blob_attr *tb[__GET_AP_INFO_MAX];
    uint32_t search_id;
    char *ap_mac = NULL;

    if( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    blobmsg_parse( get_ap_info_policy,
        __GET_AP_INFO_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if( !tb[GET_AP_INFO_MAC] ) {
        cfmanager_log_message( L_ERR, "miss ap mac\n" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    ap_mac = blobmsg_get_string( tb[GET_AP_INFO_MAC] );

    blobmsg_add_string( b, "mac", ap_mac );

    if ( ctx && !ubus_lookup_id( ctx, "controller.core", &search_id ) ) {
        if( utils_device_is_me( ap_mac ) ) {
            ubus_invoke( ctx, search_id, "ap_status_master", NULL, sgreq_get_ap_status_cb, b, 2000 );
        }
        else {
            blob_buf_init( &send_buf, 0 );
            blobmsg_add_string( &send_buf, "mac", ap_mac );
            ubus_invoke( ctx, search_id, "ap_status", send_buf.head, sgreq_get_ap_status_cb, b, 2000 );
            blob_buf_free( &send_buf );
        }
    }

return_value:

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_ap_simple_info(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__CM_AP_MAX];
    void *a = NULL;
    void *t = NULL;

    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    a = blobmsg_open_array( b, "aps simple info" );

    vlist_for_each_element( &cm_ap_vltree, cm_cfg, node ) {

        blobmsg_parse( cm_ap_policy,
            __CM_AP_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );

        t = blobmsg_open_table( b, NULL );

        blobmsg_add_string( b, "mac", util_blobmsg_get_string( tb[CM_AP_MAC], "" ) );
        blobmsg_add_string( b, "type", util_blobmsg_get_string( tb[CM_AP_TYPE], "" ) );

        blobmsg_close_table( b, t );
    }

    blobmsg_close_array( b, a );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_schedule(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb_input[__CM_SCHEDULE_GET_MAX];
    int error_state = 0;
    struct blob_buf *b;
    void *a1, *a2;
    void *t1, *t2;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__CM_SCHEDULE_MAX];
    struct blob_attr *cur = NULL;
    char *abtime = NULL;
    char *p = NULL;
    char tmp[BUF_LEN_512] = { 0 };
    int rem;
    int total_count = 0;
    char str_total_count[BUF_LEN_8] = {0};
    int is_usage = false;

    if( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    /* get input params */
    blobmsg_parse( cm_schedule_get_policy,
        __CM_SCHEDULE_GET_MAX,
        tb_input,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( tb_input[CM_SCHEDULE_GET_TYPE] ) {
        if ( !strcmp( blobmsg_get_string( tb_input[CM_SCHEDULE_GET_TYPE] ), "usage" ) )
            is_usage = true;
    }

    a1 = blobmsg_open_array( b, "scheduleLists" );
    vlist_for_each_element( &cm_schedule_vltree, cm_cfg, node ) {
        blobmsg_parse( cm_schedule_policy,
            __CM_SCHEDULE_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        t1 = blobmsg_open_table( b, NULL );
        blobmsg_add_string( b, "id", blobmsg_get_string(tb[CM_SCHEDULE_ID]) );
        blobmsg_add_string( b, "name", blobmsg_get_string(tb[CM_SCHEDULE_NAME]) );

        if ( is_usage ) {
            blobmsg_add_string( b, "usage", "00000" );//TODO
        }
        else {
            blobmsg_add_string( b, "wtime1", blobmsg_get_string(tb[CM_SCHEDULE_WEEKLY_TIME1]) );
            blobmsg_add_string( b, "wtime2", blobmsg_get_string(tb[CM_SCHEDULE_WEEKLY_TIME2]) );
            blobmsg_add_string( b, "wtime3", blobmsg_get_string(tb[CM_SCHEDULE_WEEKLY_TIME3]) );
            blobmsg_add_string( b, "wtime4", blobmsg_get_string(tb[CM_SCHEDULE_WEEKLY_TIME4]) );
            blobmsg_add_string( b, "wtime5", blobmsg_get_string(tb[CM_SCHEDULE_WEEKLY_TIME5]) );
            blobmsg_add_string( b, "wtime6", blobmsg_get_string(tb[CM_SCHEDULE_WEEKLY_TIME6]) );
            blobmsg_add_string( b, "wtime7", blobmsg_get_string(tb[CM_SCHEDULE_WEEKLY_TIME7]) );

            // Add abtimes array
            a2 = blobmsg_open_array( b, "abtimes" );
            blobmsg_for_each_attr( cur, tb[CM_SCHEDULE_ABTIME_LISTS], rem ) {
                memset( tmp, 0x0, BUF_LEN_512 );
                abtime = blobmsg_get_string(cur);
                p = strchr( abtime, ':' );
                if ( p ) {
                    t2 = blobmsg_open_table( b, NULL );
                    memcpy( tmp, abtime, p-abtime );
                    blobmsg_add_string( b, "abdate", tmp );
                    strncpy( tmp, p+1, sizeof(tmp)-1 );
                    blobmsg_add_string( b, "abtime", tmp );
                    blobmsg_close_table( b, t2 );
                }
            }
            blobmsg_close_array( b, a2 );
        }

        blobmsg_close_table( b, t1 );
        total_count++;
    }
    blobmsg_close_array( b, a1 );

    snprintf( str_total_count, BUF_LEN_8, "%d", total_count );
    blobmsg_add_string( b, "totalCount", str_total_count );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_paired_devices_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_attr *tb_list[__GS_CONTROLLER_DEVICES_ATTR_MAX];
//    struct blob_attr *tb[__GS_CONTROLLER_DEVICES_INFO_MAX];
    struct gs_device_info *gs_devinfo = (struct gs_device_info *)req->priv;
    struct blob_attr *cur_devie_table;
//    struct blob_attr *cur_ipv4_table;
    unsigned int rem = 0;
//    char address[IP4ADDR_MAX_LEN+1] = { 0 };
//    char mask[IP4ADDR_MAX_LEN+1] = { 0 };
//    int i = 0;

    blobmsg_parse( get_devices_from_controller_list_policy,
        __GS_CONTROLLER_DEVICES_ATTR_MAX,
        tb_list,
        blobmsg_data( msg ),
        blobmsg_len( msg ) );

    if ( !tb_list[GS_CONTROLLER_DEVICES_ATTR_LIST] ||
            blobmsg_type(tb_list[GS_CONTROLLER_DEVICES_ATTR_LIST]) != BLOBMSG_TYPE_ARRAY ) {
        cfmanager_log_message( L_ERR,
            "Missing devices in json or type is not blob array!\n" );
        return;
    }

    blobmsg_for_each_attr( cur_devie_table, tb_list[GS_CONTROLLER_DEVICES_ATTR_LIST], rem ) {
        char *msg_str = NULL;
        json_object *buffer_obj = NULL;
        json_object *ipv4_list_obj = NULL;
        json_object *ipv4_obj = NULL;
        json_object *tmp_obj = NULL;
        int ipv4_list_num = 0;
        int upgrade = 0;
        msg_str = blobmsg_format_json_indent( cur_devie_table, true, -1 );
        if ( msg_str ) {
            buffer_obj = json_tokener_parse( msg_str );
            if ( buffer_obj ) {
                if ( json_object_object_get_ex( buffer_obj, "mac", &tmp_obj ) ) {
                    if ( strncmp( json_object_get_string( tmp_obj ) , gs_devinfo->mac, MAC_STR_MAX_LEN ) != 0 ) {
                        free( msg_str );
                        json_object_put( buffer_obj );
                        continue;
                    }
                }

                /*
                "ipv4": [
                        "192.168.80.76",
                        "$ipv4_broadcast",
                        "$ipv4_mask"
                ],
                */
                if ( json_object_object_get_ex( buffer_obj, "ipv4", &ipv4_list_obj ) ) {
                    ipv4_list_num = json_object_array_length( ipv4_list_obj );
                    if ( ipv4_list_num >= 1 ) {
                        ipv4_obj = json_object_array_get_idx( ipv4_list_obj, 0 );
                        if ( ipv4_obj ) {
                            strncpy( gs_devinfo->ip4Address, json_object_get_string( ipv4_obj ), IPV4_MAX_LENGTH );
                        }
                    }
                    if ( json_object_object_get_ex( ipv4_obj, "version_firmware", &tmp_obj ) )
                        strncpy( gs_devinfo->systemVersion, json_object_get_string( tmp_obj ), GS_VERSION_NUM_MAXLEN );
                }
                if ( json_object_object_get_ex( buffer_obj, "version_firmware", &tmp_obj ) )
                    strncpy( gs_devinfo->systemVersion, json_object_get_string( tmp_obj ), GS_VERSION_NUM_MAXLEN );
                if ( json_object_object_get_ex( buffer_obj, "status", &tmp_obj ) )
                    strncpy( gs_devinfo->status, json_object_get_string( tmp_obj ), GS_PAIR_STATUS_MAXLEN );
                if ( json_object_object_get_ex( buffer_obj, "superior", &tmp_obj ) )
                    strncpy( gs_devinfo->superior, json_object_get_string( tmp_obj ), MAC_STR_MAX_LEN );
                if ( json_object_object_get_ex( buffer_obj, "radio", &tmp_obj ) )
                    strncpy( gs_devinfo->radio, json_object_get_string( tmp_obj ), GS_RADIO_STRING_MAXLEN );
                if ( json_object_object_get_ex( buffer_obj, "rssi", &tmp_obj ) )
                    strncpy( gs_devinfo->rssi, json_object_get_string( tmp_obj ), GS_RSSI_STRING_MAXLEN );
                if ( json_object_object_get_ex( buffer_obj, "need_upgrade_fw", &tmp_obj ) ) {
                    upgrade = json_object_get_int( tmp_obj );
                    if ( upgrade == 1 ) {               //upgrade and version is not greater than master
                        strncpy( gs_devinfo->upgrade, "1", GS_UPGRADE_MAXLEN );
                    }
                    else {                              //upgrade but version is greater than master
                        strncpy( gs_devinfo->upgrade, "2", GS_UPGRADE_MAXLEN );
                    }
                }
                else {                                  //upgrade == 0 ;not need upgrade
                    strncpy( gs_devinfo->upgrade, "0", GS_UPGRADE_MAXLEN );
                }
                if ( json_object_object_get_ex( buffer_obj, "new_version_firmware", &tmp_obj ) )
                    strncpy( gs_devinfo->newFirmwareVersion, json_object_get_string( tmp_obj ), GS_VERSION_NUM_MAXLEN );

                json_object_put( buffer_obj );
            }

            free( msg_str );
        }

#if 0
        blobmsg_parse( get_devices_from_controller_policy,
            __GS_CONTROLLER_DEVICES_INFO_MAX,
            tb,
            blob_data( cur_devie_table ),
            blob_len( cur_devie_table ) );
        if ( !tb[GS_CONTROLLER_DEVICES_MAC] )
            continue;
        else if ( strncmp( blobmsg_get_string( tb[GS_CONTROLLER_DEVICES_MAC] ) , gs_devinfo->mac, MAC_STR_MAX_LEN ) != 0 )
            continue;

        if( tb[GS_CONTROLLER_DEVICES_IPV4] ) {
            blobmsg_for_each_attr( cur_ipv4_table, tb[GS_CONTROLLER_DEVICES_IPV4], i ) {
                if ( blobmsg_type( cur_ipv4_table ) != BLOBMSG_TYPE_TABLE ) {
                    continue;
                }

                config_parse_ipv4_addr_attr( cur_ipv4_table, mask, address );
                strncpy( gs_devinfo->ip4Address, address, IPV4_MAX_LENGTH );
            }
        }

        if ( tb[GS_CONTROLLER_DEVICES_VERSION] )
            strncpy( gs_devinfo->systemVersion, blobmsg_get_string( tb[GS_CONTROLLER_DEVICES_VERSION] ), GS_VERSION_NUM_MAXLEN );
        if ( tb[GS_CONTROLLER_DEVICES_STATUS] )
            strncpy( gs_devinfo->status, blobmsg_get_string( tb[GS_CONTROLLER_DEVICES_STATUS] ), GS_PAIR_STATUS_MAXLEN );
        if ( tb[GS_CONTROLLER_DEVICES_SUPERIOR] )
            strncpy( gs_devinfo->superior, blobmsg_get_string( tb[GS_CONTROLLER_DEVICES_SUPERIOR] ), MAC_STR_MAX_LEN );
        if ( tb[GS_CONTROLLER_DEVICES_RADIO] )
            strncpy( gs_devinfo->radio, blobmsg_get_string( tb[GS_CONTROLLER_DEVICES_RADIO] ), GS_RADIO_STRING_MAXLEN );
        if ( tb[GS_CONTROLLER_DEVICES_RSSI] )
            strncpy( gs_devinfo->rssi, blobmsg_get_string( tb[GS_CONTROLLER_DEVICES_RSSI] ), GS_RSSI_STRING_MAXLEN );
#endif
    }
}

//=============================================================================
void
sgreq_get_paired_devices(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb_input[__GS_GET_PAIRED_DEVICES_MAX];
    int error_state = 0;
    struct blob_buf *b;
    struct blob_buf c;
    void *a;
    void *t;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__CM_AP_MAX];
    uint32_t search_id;
    int get_dev_type = GS_GET_DEV_TYPE_NONE;
    int total_count = 0;
    char total_count_str[4] = {0};
    struct gs_device_info gs_devinfo = {{ 0 }};
    char value[BUF_LEN_128] = {0};
    char format_mac[MAC_STR_MAX_LEN+1] = {0};
    int ret = 0;
    int is_router = 0;


    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    blobmsg_parse( get_prired_devices_policy,
        __GS_GET_PAIRED_DEVICES_MAX,
        tb_input,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if ( !tb_input[GS_GET_PAIRED_DEVICES_DEVTYPE] ) {
        cfmanager_log_message( L_ERR, "No devType" );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    if ( !strcmp( blobmsg_get_string( tb_input[GS_GET_PAIRED_DEVICES_DEVTYPE] ), "ap" ) ) {
        get_dev_type = GS_GET_DEV_TYPE_AP;
    }
    else if ( !strcmp( blobmsg_get_string( tb_input[GS_GET_PAIRED_DEVICES_DEVTYPE] ), "mesh" ) ) {
        get_dev_type = GS_GET_DEV_TYPE_MESH;
    }
    else {
        cfmanager_log_message( L_ERR, "devType(%d) is error", get_dev_type );
        error_state = ERRCODE_PARAMETER_ERROR;
        goto return_value;
    }

    /* get master mac */
    ret = util_read_file_content("/proc/gxp/dev_info/dev_mac", value, sizeof(value));
    if( !ret ) {
        util_str_erase_colon( value, format_mac );
    }

    a = blobmsg_open_array( b, "devList" );
    vlist_for_each_element( &cm_ap_vltree, cm_cfg, node ) {
        blobmsg_parse( cm_ap_policy,
            __CM_AP_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        if ( !strncmp( blobmsg_get_string( tb[__CM_AP_MAX] ), format_mac, MAC_STR_MAX_LEN ) ) {
            is_router = 1;
        }
        else {
            is_router = 0;
            total_count ++;
        }

        if ( GS_GET_DEV_TYPE_MESH == get_dev_type ) {
            /* ap do not show master router, but mesh need */
            if ( 0 == is_router ) {
                if ( !strcmp( blobmsg_get_string( tb[CM_AP_MESH] ), "0" ) )
                    continue;
            }
        }
        else if ( GS_GET_DEV_TYPE_AP == get_dev_type ) {
            if ( is_router || !strcmp( blobmsg_get_string( tb[CM_AP_MESH] ), "1" ) )
                continue;
        }

        memset( &gs_devinfo, 0x0, sizeof( struct gs_device_info ) );
        memset( &c, 0, sizeof( c ) );
        blob_buf_init( &c, 0 );
        blobmsg_add_blob( &c, tb[CM_AP_MAC] );
        if ( ctx && !ubus_lookup_id( ctx, "controller.discovery", &search_id ) ) {
            strncpy( gs_devinfo.mac, blobmsg_get_string( tb[CM_AP_MAC] ), MAC_STR_MAX_LEN );
            ubus_invoke( ctx, search_id, "get_paired_devices", c.head, sgreq_get_paired_devices_cb, (void *)&gs_devinfo, 1000 );
        }
        blob_buf_free( &c );
        t = blobmsg_open_table( b, NULL );
        blobmsg_add_string( b, "devMac", blobmsg_get_string( tb[CM_AP_MAC] ) );
        blobmsg_add_string( b, "type", blobmsg_get_string( tb[CM_AP_TYPE] ) );
        blobmsg_add_string( b, "ip4Address", gs_devinfo.ip4Address );
        blobmsg_add_string( b, "devName", tb[CM_AP_NAME] ? blobmsg_get_string( tb[CM_AP_NAME] ) : blobmsg_get_string( tb[CM_AP_TYPE] ) );
        blobmsg_add_string( b, "systemVersion", gs_devinfo.systemVersion );
        blobmsg_add_string( b, "status", gs_devinfo.status );
        blobmsg_add_string( b, "upgrade", gs_devinfo.upgrade );
        blobmsg_add_string( b, "newFirmwareVersion", gs_devinfo.newFirmwareVersion );
        if ( GS_GET_DEV_TYPE_MESH == get_dev_type ) {
            blobmsg_add_string( b, "curSuperior", gs_devinfo.superior );
            blobmsg_add_string( b, "radio", gs_devinfo.radio );
            blobmsg_add_string( b, "rssi", gs_devinfo.rssi );
        }
        blobmsg_add_string( b, "devPosition", tb[CM_AP_POSITION] ? blobmsg_get_string( tb[CM_AP_POSITION] ) : "0" );
        blobmsg_add_string( b, "customDevPosition", tb[CM_AP_POSITION_CUSTOM] ? blobmsg_get_string( tb[CM_AP_POSITION_CUSTOM] ) : "" );
        blobmsg_add_string( b, "superior", tb[CM_AP_SUPERIOR] ? blobmsg_get_string( tb[CM_AP_SUPERIOR] ) : "" );
        blobmsg_close_table( b, t );
    }
    blobmsg_close_array( b, a );

    /* add total count */
    snprintf( total_count_str, 3, "%d", total_count );
    blobmsg_add_string( b, "totalCount", total_count_str );


return_value:
  
    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_client_limit(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb[__CLIENT_LIMIT_MAX];
    int error_state = ERRCODE_SUCCESS;
    char mac[MAC_STR_MAX_LEN+1] = { 0 };
    char section_name[BUF_LEN_32] = { 0 };
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    blobmsg_parse( client_limit_policy,
        __CLIENT_LIMIT_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    if( !tb[CLIENT_LIMIT_MAC] ) {
        cfmanager_log_message( L_ERR, "Lack of MAC information\n" );
        error_state = ERRCODE_MESSAGE_WRONG_FROMAT;
        goto return_vlaue;
    }
    strncpy( mac, blobmsg_get_string( tb[CLIENT_LIMIT_MAC] ), MAC_STR_MAX_LEN );
    snprintf( section_name, sizeof( section_name ), "%s%s", CLIENT_LIMIT_PREFIX, mac );

    /* The parameter has not been set when getting it for the first time.
     * No error should be returned here
     */
    sgreq_get_info_by_cmtree( b, &cm_client_limit_vltree,
        client_limit_policy, section_name, __CLIENT_LIMIT_MAX );

return_vlaue:
    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_overview(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_controller_cfg = NULL;
    struct cm_config *cm_wan_cfg = NULL;
    struct cm_config *cm_wireless_cfg = NULL;
    struct cm_config *cm_guest_ssid_cfg = NULL;
    struct cm_config *cm_ap_cfg = NULL;
    struct blob_attr *tb_controller[__CONTROLLER_MAX];
    struct blob_attr *tb_wan[__WAN_MAX];
    struct blob_attr *tb_wireless[__WIRELESS_MAX];
    struct blob_attr *tb_guest_ssid[__GUEST_SSID_MAX];
    struct blob_attr *tb_ap[__CM_AP_MAX];
    char time_str[BUF_LEN_32] = { 0 };
    uint32_t search_id;
    char value[BUF_LEN_128] = {0};
    int ret = 0;
    struct sysinfo s_info;
    int mesh_num = 0;
    char mesh_num_str[4] = {0};

    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    /* role & workMode */
    cm_controller_cfg = util_get_vltree_node( &cm_controller_vltree, VLTREE_CM_TREE, "main" );
    if( !cm_controller_cfg ) {
        cfmanager_log_message( L_ERR, "No controller tree found in %s\n", CFMANAGER_CONFIG_NAME );
    }
    else {
        blobmsg_parse( controller_policy,
            __CONTROLLER_MAX,
            tb_controller,
            blob_data( cm_controller_cfg->cf_section.config ),
            blob_len( cm_controller_cfg->cf_section.config ) );

        blobmsg_add_string( b, controller_policy[CONTROLLER_ROLE].name, util_blobmsg_get_string( tb_controller[CONTROLLER_ROLE], "slave" ) );
        blobmsg_add_string( b, controller_policy[CONTROLLER_WORK_MODE].name, util_blobmsg_get_string( tb_controller[CONTROLLER_WORK_MODE], "0" ) );
    }

    //lan data
    if ( ctx && !ubus_lookup_id( ctx, "network.interface.lan0_zone0", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status", NULL, sgreq_get_dev_lan0_zone0_cb, b, 1000 );
    }

    //wan0 & wan1 data
    cm_wan_cfg = util_get_vltree_node( &cm_wan_vltree, VLTREE_CM_TREE, "wan" );
    if( !cm_wan_cfg ) {
        cfmanager_log_message( L_ERR, "No wan tree found in %s\n", CFMANAGER_CONFIG_NAME );
    }
    else {
        blobmsg_parse( wan_policy,
            __WAN_MAX,
            tb_wan,
            blob_data( cm_wan_cfg->cf_section.config ),
            blob_len( cm_wan_cfg->cf_section.config ) );
#ifdef DOUBLE_WAN
        blobmsg_add_u8( b, wan_policy[WAN1_ENABLE].name, util_blobmsg_get_bool( tb_wan[ WAN1_ENABLE ], false ) );
#endif
    }
    if ( ctx && !ubus_lookup_id( ctx, "network.interface.wan0", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status", NULL, sgreq_get_dev_wan0_cb, b, 1000 );
    }
#ifdef DOUBLE_WAN
    if ( ctx && !ubus_lookup_id( ctx, "network.interface.wan1", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status", NULL, sgreq_get_dev_wan1_cb, b, 1000 );
    }
#endif

    blob_buf_init( &send_buf, 0 );
    blobmsg_add_string( &send_buf, "unit",  "bit" );

    if ( ctx && !ubus_lookup_id( ctx, "controller.core", &search_id ) ) {
        ubus_invoke( ctx, search_id, "status_wan", send_buf.head, sgreq_get_wan_rate_cb, b, 1000 );
    }
    blob_buf_free( &send_buf );

    sgreq_get_wan_link_rate( b, WAN0 );
#ifdef DOUBLE_WAN
    sgreq_get_wan_link_rate( b, WAN1 );
#endif

    /* devProductName & devSoftVersion & devUptime & devSystemTime */
    memset(value, 0, sizeof(value));
    ret = util_read_file_content("/proc/gxp/dev_info/dev_id", value, sizeof(value));
    if( !ret )
        blobmsg_add_string( b, "devProductName", value );

    memset(value, 0, sizeof(value));
    ret = util_read_file_content("/tmp/gs_version", value, sizeof(value));
    if( !ret )
        blobmsg_add_string( b, "devSoftVersion", value );

    sysinfo( &s_info );
    blobmsg_add_u32( b, "devUptime", s_info.uptime );

    utils_time_value2time_str( time( NULL ), time_str, BUF_LEN_32 );
    blobmsg_add_string( b, "devSystemTime", time_str );


    /* wireless */
    cm_wireless_cfg = util_get_vltree_node( &cm_wireless_vltree, VLTREE_CM_TREE, "wireless" );
    if( !cm_wireless_cfg ) {
        cfmanager_log_message( L_ERR, "No wireless tree found in %s\n", CFMANAGER_CONFIG_NAME );
    }
    else {
        blobmsg_parse( wireless_policy,
            __WIRELESS_MAX,
            tb_wireless,
            blob_data( cm_wireless_cfg->cf_section.config ),
            blob_len( cm_wireless_cfg->cf_section.config ) );

        blobmsg_add_u8( b, wireless_policy[WIRELESS_MERGERADIOENABL].name, util_blobmsg_get_bool( tb_wireless[ WIRELESS_MERGERADIOENABL ], false ) );
        blobmsg_add_u8( b, wireless_policy[WIRELESS_WIFIENABL].name, util_blobmsg_get_bool( tb_wireless[ WIRELESS_WIFIENABL ], true ) );
        blobmsg_add_u8( b, wireless_policy[WIRELESS_2G4ENABLE].name, util_blobmsg_get_bool( tb_wireless[ WIRELESS_2G4ENABLE ], false ) );
        blobmsg_add_string( b, wireless_policy[WIRELESS_2G4SSIDNAME].name, util_blobmsg_get_string( tb_wireless[WIRELESS_2G4SSIDNAME], "" ) );
        blobmsg_add_u8( b, wireless_policy[WIRELESS_5GENABLE].name, util_blobmsg_get_bool( tb_wireless[ WIRELESS_5GENABLE ], false ) );
        if ( 0 == util_blobmsg_get_bool( tb_wireless[ WIRELESS_MERGERADIOENABL ], false ) ){
            blobmsg_add_string( b, wireless_policy[WIRELESS_5GSSIDNAME].name, util_blobmsg_get_string( tb_wireless[WIRELESS_5GSSIDNAME], "" ) );
        }
        else{
            blobmsg_add_string( b, wireless_policy[WIRELESS_5GSSIDNAME].name, util_blobmsg_get_string( tb_wireless[WIRELESS_2G4SSIDNAME], "" ) );
        }
    }

    /* guest_ssid */
    cm_guest_ssid_cfg = util_get_vltree_node( &cm_guest_ssid_vltree, VLTREE_CM_TREE, "guest_ssid" );
    if( !cm_guest_ssid_cfg ) {
        cfmanager_log_message( L_ERR, "No guest_ssid tree found in %s\n", CFMANAGER_CONFIG_NAME );
    }
    else {
        blobmsg_parse( guest_ssid_policy,
            __GUEST_SSID_MAX,
            tb_guest_ssid,
            blob_data( cm_guest_ssid_cfg->cf_section.config ),
            blob_len( cm_guest_ssid_cfg->cf_section.config ) );

        blobmsg_add_u8( b, "guestEnable", util_blobmsg_get_bool( tb_guest_ssid[ GUEST_SSID_ENABLE ], false ) );
        blobmsg_add_string( b, "guestSsidName", util_blobmsg_get_string( tb_guest_ssid[GUEST_SSID_SSIDNAME], "" ) );
    }

    /* mesh */
    vlist_for_each_element( &cm_ap_vltree, cm_ap_cfg, node ) {
        blobmsg_parse( cm_ap_policy,
            __CM_AP_MAX,
            tb_ap,
            blob_data( cm_ap_cfg->cf_section.config ),
            blob_len( cm_ap_cfg->cf_section.config  ) );

        if( tb_ap[CM_AP_MESH] && !strcmp( blobmsg_get_string( tb_ap[CM_AP_MESH] ), "1" ) ) {
            mesh_num++;
        }
    }
    snprintf( mesh_num_str, 3, "%d", mesh_num );
    blobmsg_add_string( b, "meshPairedNum", mesh_num_str );

    /* client */
    blob_buf_init( &send_buf, 0 );
    blobmsg_add_u32( &send_buf, "start", 0 );
    blobmsg_add_u32( &send_buf, "amount", 200 );
    if ( ctx && !ubus_lookup_id( ctx, "clients_center", &search_id ) ) {
        ubus_invoke( ctx, search_id, "get_clients_list", send_buf.head, sgreq_get_clients_online_count_cb, b, 1500 );
    }
    blob_buf_free( &send_buf );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    /*  response data */
    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
static int
sgreq_parse_bandwidths(
    struct blob_attr *data
)
//=============================================================================
{
    char buf[BUF_LEN_16];

    memset( buf, 0, sizeof( buf ) );
    strncpy( buf, blobmsg_get_string( data ), sizeof( buf ) -1 );

    if( 0 == strcmp( "VHT20", buf ) ) {
        return CW_5G_HT20;
    }
    else if( 0 == strcmp( "VHT40", buf ) ||
             0 == strcmp ( "VHT40+", buf ) ||
             0 == strcmp ( "VHT40-", buf ) ){
        return CW_5G_HT40;
    }
    else if( 0 == strcmp( "VHT80", buf ) ) {
        return CW_5G_HT80;
    }
    else if( 0 == strcmp( "VHT160", buf ) ) {
    }

    return __CW_5G_MAX;
}

//=============================================================================
static int
sgreq_parse_channel_info(
    struct blob_buf *reply,
    struct blob_attr *data,
    int radio,
    char *channel_str,
    int channel_str_size
)
//=============================================================================
{
    enum {
        CHANNEL_VALUE_CHANNEL,
        CHANNEL_VALUE_BANDWIDTHS,

        CHANNEL_VALUE_MAX
    };

    static const struct blobmsg_policy parse_channel_value_policy[CHANNEL_VALUE_MAX] = {
        [CHANNEL_VALUE_CHANNEL] = { .name = "channel", .type = BLOBMSG_TYPE_INT32 },
        [CHANNEL_VALUE_BANDWIDTHS] = { .name = "bandwidths", .type = BLOBMSG_TYPE_ARRAY }
    };

    struct blob_attr *tb[CHANNEL_VALUE_MAX];
    struct blob_attr *channel_attr = NULL;
    int option = 0;
    int pos = 0;
    char value[BUF_LEN_32];

    blobmsg_parse( parse_channel_value_policy,
        CHANNEL_VALUE_MAX,
        tb,
        blobmsg_data( data ),
        blobmsg_data_len( data ) );

    channel_attr = tb[CHANNEL_VALUE_CHANNEL];
    if( !channel_attr ) {
        cfmanager_log_message( L_ERR, "Missing channel information\n" );
        return 0;
    }

    snprintf( value, sizeof( value ), "%d", blobmsg_get_u32( channel_attr ) );
    switch ( radio ) {
        case RADIO_2G:
            blobmsg_add_string( reply, NULL, value );
            break;
        case RADIO_5G: {
            struct blob_attr *attr = NULL;
            int i = 0;

            strncpy( channel_str, value, channel_str_size - 1 );
            blobmsg_for_each_attr( attr, tb[CHANNEL_VALUE_BANDWIDTHS], i ) {
                if ( blobmsg_type( attr ) != BLOBMSG_TYPE_STRING ) {
                    continue;
                }

                pos = sgreq_parse_bandwidths( attr );
                option |= BIT( pos );
            }
        }
            break;
         default:
            break;
    }

    return option;
}

//=============================================================================
static void
sgreq_add_channel_and_width(
    struct blob_buf *reply,
    const char *key,
    int count,
    char (*channel)[BUF_LEN_8]
)
//=============================================================================
{
    void *array;
    int i;

    array = blobmsg_open_array( reply, key );

    for( i=0; i < count; i++ ) {
        blobmsg_add_string( reply, NULL, channel[i] );
    }

    blobmsg_close_array( reply, array );
}

//=============================================================================
static void
sgreq_get_channel_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
#define HT20_CHANNEL_NAME "width0"
#define HT40_CHANNEL_NAME "width1"
#define HT80_CHANNEL_NAME "width2"

    enum {
        CHANNEL_2G,
        CHANNEL_5G,

        __CHANNEL_MAX
    };

    static const struct blobmsg_policy parse_channel_policy[__CHANNEL_MAX] = {
        [CHANNEL_2G] = { .name = "channels_2g4", .type = BLOBMSG_TYPE_ARRAY },
        [CHANNEL_5G] = { .name = "channels_5g", .type = BLOBMSG_TYPE_ARRAY }
    };

    struct blob_buf *reply = (struct blob_buf *)req->priv;
    struct blob_attr *tb[__CHANNEL_MAX];
    int i = 0;
    struct blob_attr *cur = NULL;
    void *table;
    void *array;
    uint32_t option;
    char channel[BUF_LEN_8];
    char width0[CHANNEL_MAX_5G][BUF_LEN_8] = {{ 0 }};
    char width1[CHANNEL_MAX_5G][BUF_LEN_8] = {{ 0 }};
    char width2[CHANNEL_MAX_5G][BUF_LEN_8] = {{ 0 }};
    int width0_count = 0;
    int width1_count = 0;
    int width2_count = 0;

    blobmsg_parse( parse_channel_policy,
        __CHANNEL_MAX,
        tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( !tb[CHANNEL_2G] ) {
        cfmanager_log_message( L_ERR, "no 2g channel info\n" );
        return;
    }

    //Fill in 2G channel information
    array = blobmsg_open_array( reply, "channels_2g4" );
    blobmsg_for_each_attr( cur, tb[CHANNEL_2G], i ) {
        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_TABLE ) {
            continue;
        }

        sgreq_parse_channel_info( reply, cur, RADIO_2G, channel, sizeof( channel ) );
    }
    blobmsg_close_array( reply, array );

    if( !tb[CHANNEL_5G] ) {
        cfmanager_log_message( L_ERR, "no 5g channel info\n" );
        return;
    }

    //Fill in 5G channel information
    blobmsg_for_each_attr( cur, tb[CHANNEL_5G], i ) {
        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_TABLE ) {
            continue;
        }

        option = 0;
        memset( channel, 0, sizeof( channel ) );
        option = sgreq_parse_channel_info( reply, cur, RADIO_5G, channel, sizeof( channel ) );

        if( option & BIT( CW_5G_HT20 ) && CHANNEL_MAX_5G >= width0_count ) {
            strncpy( width0[width0_count], channel, BUF_LEN_8 -1 );
            width0_count++;
        }

        if( option & BIT( CW_5G_HT40 ) && CHANNEL_MAX_5G >= width1_count ) {
            strncpy( width1[width1_count], channel, BUF_LEN_8 -1 );
            width1_count++;
        }

        if( option & BIT( CW_5G_HT80 ) && CHANNEL_MAX_5G >= width2_count ) {
            strncpy( width2[width2_count], channel, BUF_LEN_8 -1 );
            width2_count++;
        }
    }

    table = blobmsg_open_table( reply, "channels_5g" );
    sgreq_add_channel_and_width( reply, HT20_CHANNEL_NAME, width0_count, width0 );
    sgreq_add_channel_and_width( reply, HT40_CHANNEL_NAME, width1_count, width1 );
    sgreq_add_channel_and_width( reply, HT80_CHANNEL_NAME, width2_count, width2 );
    blobmsg_close_table( reply, table );
}

//=============================================================================
void
sgreq_get_tr069(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    sgreq_get_info_by_cmtree( b, &cm_tr069_vltree, cm_tr069_policy, "tr069", __CM_TR069_MAX );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_additional_ssid(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct uci_package *package = NULL;
    struct uci_element *e = NULL;
    struct blob_attr *tb[__CM_ADDIT_SSID_MAX];
    void *cookie;
    int i = 0;
    void *t;
    bool enable = false;
    uint64_t rx_total = 0;
    uint64_t tx_total = 0;
    char rate_str[BUF_LEN_32] = { 0 };

    package = cfparse_init_package( CFMANAGER_CONFIG_NAME );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load %s package failed\n", CFMANAGER_CONFIG_NAME );
        return;
    }

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cookie = blobmsg_open_array( b, "ssidInfo" );
    uci_foreach_element( &package->sections, e ) {
        struct blob_buf tmp_blob;
        struct uci_section *s = uci_to_section( e );
        const struct uci_blob_param_list* uci_blob_list = NULL;

        if( strcmp(cm_vltree_info[CM_VLTREE_ADDIT_SSID].key, s->type) ) {
            continue;
        }

        uci_blob_list = cm_vltree_info[CM_VLTREE_ADDIT_SSID].policy_list;
        memset( &tmp_blob, 0, sizeof( struct blob_buf ) );
        blob_buf_init( &tmp_blob, 0 );
        uci_to_blob( &tmp_blob, s, uci_blob_list );

        blobmsg_parse( cm_addit_ssid_policy,
            __CM_ADDIT_SSID_MAX,
            tb,
            blob_data( tmp_blob.head ),
            blob_len( tmp_blob.head ) );

        t = blobmsg_open_table( b, NULL );
        for ( i = 0; i < __CM_ADDIT_SSID_MAX; i++ ) {
            if ( !tb[i] ) {
                continue;
            }

            blobmsg_add_blob( b, tb[i] );
        }

        enable = util_blobmsg_get_bool( tb[CM_ADDIT_SSID_ENABLE], false );
        if( !enable ) {
            continue;
        }

        snprintf( rate_str, sizeof( rate_str ), "%" PRIu64, rx_total );
        blobmsg_add_string( b, "currentUpRate", rate_str );
        snprintf( rate_str, sizeof( rate_str ), "%" PRIu64, tx_total );
        blobmsg_add_string( b, "currentDownRate", rate_str );

        blobmsg_close_table( b, t );
        blob_buf_free( &tmp_blob );
    }
    blobmsg_close_array( b, cookie );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_radio(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cfparse_wifi_global *cfwgl = NULL;
    char country_str[BUF_LEN_8] = { 0 };
    char *section_name = "radio";

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    sgreq_get_info_by_cmtree( b, &cm_radio_vltree,
        cm_radio_policy, section_name, __CM_RADIO_MAX );

    cfwgl = util_get_vltree_node( &wireless_global_vltree, VLTREE_DEV_GLOB, "qcawifi" );
    if( !cfwgl ) {
        snprintf( country_str, sizeof( country_str ), "%d", COUNTRY_DEFVALUE );
    }
    else {
        strncpy( country_str, cfwgl->country, sizeof( country_str ) -1 );
    }
    blobmsg_add_string( b, "CountryCode", country_str );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );

}

//=============================================================================
void
sgreq_get_channel(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = ERRCODE_SUCCESS;
    uint32_t search_id;
    static struct blob_buf b;
    struct cfparse_wifi_global *cfwgl = NULL;
    char country_str[BUF_LEN_8] = { 0 };

    blob_buf_init( &reply, 0 );

    if ( ubus_lookup_id( ctx, "iwpriv_dfs_wifi_cap", &search_id ) ) {
        cfmanager_log_message( L_ERR, "not find iwpriv_dfs_wifi_cap\n" );
        error_state = ERRCODE_INTERNAL_ERROR;
        goto return_value;
    }

    blob_buf_init( &b, 0 );

    cfwgl = util_get_vltree_node( &wireless_global_vltree, VLTREE_DEV_GLOB, "qcawifi" );
    if( !cfwgl ) {
        snprintf( country_str, sizeof( country_str ), "%d", COUNTRY_DEFVALUE );
    }
    else {
        snprintf( country_str, sizeof( country_str ), "%s", cfwgl->country );
    }

    blobmsg_add_string( &b, "country", country_str );
    blobmsg_add_string( &b, "model", device_info.product_model );

    cfmanager_log_message( L_DEBUG, "country_str:%s,device_info.product_model:%s",
        country_str,device_info.product_model );
    ubus_invoke( ctx, search_id, "get_channels", b.head, sgreq_get_channel_cb, &reply, 2000 );

    blob_buf_free( &b );

return_value:
    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

//=============================================================================
void
sgreq_get_portal_policy(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *cm_tb[__PORTAL_POLICY_MAX];
    struct cm_config *cm_cfg = NULL;
    struct blob_buf *b;
    void *policy_array = NULL;
    void *policy_table = NULL;
    int error_state = ERRCODE_SUCCESS;
    int i = 0;

    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    policy_array = blobmsg_open_array( b, "policy" );

    vlist_for_each_element(
        &cm_portal_policy_vltree, cm_cfg, node ) {

        blobmsg_parse( cm_portal_policy,
            __PORTAL_POLICY_MAX,
            cm_tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        policy_table = blobmsg_open_table( b, NULL );

        for( i = __PORTAL_POLICY_MIN; i < __PORTAL_POLICY_MAX; i++ ) {
            if( !cm_tb[i] ) {
                continue;
            }

            switch( cm_portal_policy[i].type ) {
                case BLOBMSG_TYPE_BOOL:
                    blobmsg_add_u8( b,
                        cm_portal_policy[i].name,
                        util_blobmsg_get_bool( cm_tb[i], false ) );
                    break;
                case BLOBMSG_TYPE_STRING:
                    blobmsg_add_string( b,
                        cm_portal_policy[i].name,
                        util_blobmsg_get_string( cm_tb[i], "" ) );
                    break;
                case BLOBMSG_TYPE_ARRAY:
                    blobmsg_add_blob( b, cm_tb[i] );
                    break;
                default:
                    break;
            }
        }

        blobmsg_close_table( b, policy_table );

    }

    blobmsg_close_array( b, policy_array );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}


//=============================================================================
void
sgreq_get_usb_share(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int i = 0, ret = 0, error_state = 0;
    void *table;
    struct usb_partition_info usb_partition;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__USB_SHARE_MAX];
    
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        if( NULL != sg_params->data ) {
            b = (struct blob_buf *)sg_params->data;
        }
        else {
            goto return_value;
        }
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    ret = usb_get_devices_info( &usb_partition );
    if( ret <= 0 ) {
        table = blobmsg_open_table( b, "device" );
        blobmsg_add_u8( b, "status", 0 );
        blobmsg_close_table( b, table );

        if( ret < 0 ) {
            cfmanager_log_message( L_ERR, "[%d]get usb info is error.\n", ret );
            error_state = ERRCODE_INTERNAL_ERROR;
        }

        goto return_value;
    }

    cm_cfg = util_get_vltree_node( &cm_usb_share_vltree, VLTREE_CM_TREE, "usb_share" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_ERR, "No usb_share information found in %s\n", CFMANAGER_CONFIG_NAME );
        error_state = ERRCODE_INTERNAL_ERROR;
        goto return_value;
    }

    table = blobmsg_open_table( b, "device" );
    blobmsg_add_u8( b, "status", 1 );
    blobmsg_add_string( b, "name", usb_partition.name );
    blobmsg_add_string( b, "format", usb_partition.format );
    blobmsg_add_string( b, "total", usb_partition.total );
    blobmsg_add_string( b, "usable", usb_partition.usable );
    blobmsg_close_table( b, table );

    blobmsg_parse( usb_share_policy,
                   __USB_SHARE_MAX,
                   tb,
                   blob_data( cm_cfg->cf_section.config ),
                   blob_len( cm_cfg->cf_section.config ) );

    for( i = 0; i < __USB_SHARE_MAX; i++ ) {
        if( !tb[i] ) continue;
        blobmsg_add_blob( b, tb[i] );
    }


return_value:
    if( sg_params && EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );

    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_snmp(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0, i = 0;
    struct blob_buf *b;
    void *a;
    void *t;

    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb1[__SNMP_CONFIG_MAX];
    struct blob_attr *tb2[__SNMP_PORTS_MAX];
    struct blob_attr *tb3[__SNMP_V3_MAX];

    if( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    t = blobmsg_open_table( b, "snmpConfig" );
    cm_cfg = util_get_vltree_node( &cm_snmp_config_vltree, VLTREE_CM_TREE, "snmpd" );
    if( cm_cfg ) {
        blobmsg_parse( snmp_config_policy,
            __SNMP_CONFIG_MAX,
            tb1,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );


        for ( i = SNMP_CONFIG_ENABLE_V1_V2C; i < __SNMP_CONFIG_MAX; i++ ) {
            switch( snmp_config_policy[i].type ) {
                case BLOBMSG_TYPE_BOOL:
                    blobmsg_add_u8( b,
                        snmp_config_policy[i].name,
                        util_blobmsg_get_bool( tb1[i], false ) );
                    break;
                case BLOBMSG_TYPE_STRING:
                    blobmsg_add_string( b,
                        snmp_config_policy[i].name,
                        util_blobmsg_get_string( tb1[ i ], "" ) );
                    break;
                default:
                    break;
            }
        }

    }
    else {
        cfmanager_log_message( L_DEBUG, "can not find snmpd\n" );

        for ( i = SNMP_CONFIG_ENABLE_V1_V2C; i < __SNMP_CONFIG_MAX; i++ ) {
            switch( snmp_config_policy[i].type ) {
                case BLOBMSG_TYPE_BOOL:
                    blobmsg_add_u8( b,
                        snmp_config_policy[i].name, 0 );
                    break;
                case BLOBMSG_TYPE_STRING:
                    blobmsg_add_string( b,
                        snmp_config_policy[i].name, "" );
                    break;
                default:
                    break;
            }
        }
    }

    blobmsg_close_table( b, t );


    a = blobmsg_open_array( b, "snmpPorts" );
    vlist_for_each_element( &cm_snmp_ports_vltree, cm_cfg, node ) {
        const char *id = NULL;

        blobmsg_parse( grandstream_snmp_ports_policy,
            __SNMP_PORTS_MAX,
            tb2,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config  ) );

        if ( !tb2[SNMP_PORTS_ID] )
            continue;

        t = blobmsg_open_table( b, NULL );

        id = blobmsg_get_string( tb2[SNMP_PORTS_ID] );

        blobmsg_add_string( b, grandstream_snmp_ports_policy[SNMP_PORTS_ID].name, id );

        if ( tb2[SNMP_PORTS_PORT] )
            blobmsg_add_string( b, grandstream_snmp_ports_policy[SNMP_PORTS_PORT].name,
                blobmsg_get_string( tb2[SNMP_PORTS_PORT] )  );

        if ( tb2[SNMP_PORTS_PROTOCOL] )
            blobmsg_add_string( b, grandstream_snmp_ports_policy[SNMP_PORTS_PROTOCOL].name,
                blobmsg_get_string( tb2[SNMP_PORTS_PROTOCOL] )  );

        if ( tb2[SNMP_PORTS_IPV4ADDRESS] )
            blobmsg_add_string( b, grandstream_snmp_ports_policy[SNMP_PORTS_IPV4ADDRESS].name,
                blobmsg_get_string( tb2[SNMP_PORTS_IPV4ADDRESS] )  );

         blobmsg_close_table( b, t );

    }
    blobmsg_close_array( b, a );


    a = blobmsg_open_array( b, "snmpv3Auth" );
    vlist_for_each_element( &cm_snmp_v3_auth_vltree, cm_cfg, node ) {
        const char *user_id = NULL;

        blobmsg_parse( grandstream_snmp_v3_auth_policy,
            __SNMP_V3_MAX,
            tb3,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config  ) );

        if ( !tb3[SNMP_V3_ID] )
            continue;

        t = blobmsg_open_table( b, NULL );

        user_id = blobmsg_get_string( tb3[SNMP_V3_ID] );

        blobmsg_add_string( b, grandstream_snmp_v3_auth_policy[SNMP_V3_ID].name, user_id );

        for ( i = SNMP_V3_NAME; i < __SNMP_V3_MAX; i++ ) {
            switch( snmp_config_policy[i].type ) {
                case BLOBMSG_TYPE_BOOL:
                    blobmsg_add_u8( b,
                        grandstream_snmp_v3_auth_policy[i].name,
                        util_blobmsg_get_bool( tb3[i], false ) );
                    break;
                case BLOBMSG_TYPE_STRING:
                    blobmsg_add_string( b,
                        grandstream_snmp_v3_auth_policy[i].name,
                        util_blobmsg_get_string( tb3[i], "" ) );
                    break;
                default:
                    break;
            }
        }

        blobmsg_close_table( b, t );

    }
    blobmsg_close_array( b, a );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}

//=============================================================================
void
sgreq_get_vpn_server(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    void *server_list_array;
    void *server_table;
    struct cm_config *server_cfg = NULL;
    struct cm_config *cmn_setting_cfg = NULL;
    struct cm_config *dial_in_user_cfg = NULL;
    struct blob_attr *server_tb[__VPN_SERVER_MAX];
    struct blob_attr *cmn_setting_tb[__IPSEC_CMN_SETTING_ATTR_MAX];
    struct blob_attr *dial_in_user_tb[__IPSEC_DIAL_IN_USER_ATTR_MAX];
    struct blob_attr *cur = NULL;
    int rem;
    int i = 0, j = 0, k = 0;

    if( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    server_list_array = blobmsg_open_array( b, "vpnServerList" );
    vlist_for_each_element( &cm_vpn_server_vltree, server_cfg, node ) {
        blobmsg_parse( vpn_server_policy,
            __VPN_SERVER_MAX,
            server_tb,
            blob_data( server_cfg->cf_section.config ),
            blob_len( server_cfg->cf_section.config  ) );

        server_table = blobmsg_open_table( b, NULL );
        for( i = 0; i < __VPN_SERVER_MAX; i++ ) {
            if( !server_tb[i] ) {
                continue;
            }

            if ( i == VPN_SERVER_IPSEC_CMN_SETTING ) {
                void *cmn_setting_table;
                char *cmn_setting = blobmsg_get_string(server_tb[i]);

                // Find matched ipsec common setting section.
                vlist_for_each_element( &cm_ipsec_cmn_setting_vltree, cmn_setting_cfg, node ) {
                    if ( strcmp(cmn_setting_cfg->cf_section.name, cmn_setting) ) {
                        continue;
                    }

                    blobmsg_parse( ipsec_cmn_setting_policy,
                        __IPSEC_CMN_SETTING_ATTR_MAX,
                        cmn_setting_tb,
                        blob_data( cmn_setting_cfg->cf_section.config ),
                        blob_len( cmn_setting_cfg->cf_section.config  ) );

                    cmn_setting_table = blobmsg_open_table( b, "ipsecCommonSetting" );
                    for ( j = 0; j < __IPSEC_CMN_SETTING_ATTR_MAX; j++ ) {
                        blobmsg_add_blob( b, cmn_setting_tb[j] );
                    }
                    blobmsg_close_table( b, cmn_setting_table );
                }
            }
            else if ( i == VPN_SERVER_IPSEC_DIAL_IN_USER ) {
                void *dial_in_user_array;
                void *dial_in_user_table;

                dial_in_user_array = blobmsg_open_array( b, "ipsecDialInUser" );
                blobmsg_for_each_attr( cur, server_tb[i], rem ) {
                    char *dial_in_user = blobmsg_get_string( cur );
                    

                    // Find matched ipsec dial-in user section.
                    vlist_for_each_element( &cm_ipsec_dial_in_user_vltree, dial_in_user_cfg, node ) {
                        if ( strcmp( dial_in_user_cfg->cf_section.name, dial_in_user) ) {
                            continue;
                        }

                        blobmsg_parse( ipsec_dial_in_user_policy,
                            __IPSEC_DIAL_IN_USER_ATTR_MAX,
                            dial_in_user_tb,
                            blob_data( dial_in_user_cfg->cf_section.config ),
                            blob_len( dial_in_user_cfg->cf_section.config  ) );

                        dial_in_user_table = blobmsg_open_table( b, NULL );
                        for ( k = 0; k < __IPSEC_DIAL_IN_USER_ATTR_MAX; k++ ) {
                            if ( !dial_in_user_tb[k] ) {
                                continue;
                            }

                            blobmsg_add_blob( b, dial_in_user_tb[k] );
                        }
                        blobmsg_close_table( b, dial_in_user_table );
                    } 
                }
                blobmsg_close_array( b, dial_in_user_array );
            }
            else {
                blobmsg_add_blob( b, server_tb[i] );
            }
        }
        blobmsg_close_table( b, server_table );
    }
    blobmsg_close_array( b, server_list_array );

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );
}


//=============================================================================
void
sgreq_get_acceleration(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct blob_buf *b;
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__ACCELERATION_MAX];
    int i;

    if ( EXTEND_GET_ALL == sg_params->cmd ) {
        b = (struct blob_buf *)sg_params->data;
    }
    else {
        b = &reply;
        blob_buf_init( b, 0 );
    }

    cm_cfg = util_get_vltree_node( &cm_acceleration_vltree, VLTREE_CM_TREE, "acceleration" );
    if ( cm_cfg ) {
        blobmsg_parse( acceleration_policy,
            __ACCELERATION_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __ACCELERATION_MAX; i++ ) {
            if ( !tb[i] ) {
                continue;
            }
            blobmsg_add_blob( b, tb[i] );
        }
    }

    //When sg issues "get all", the return information is returned from sgreq_get_all
    if( EXTEND_GET_ALL == sg_params->cmd ) {
        return;
    }

    sgreq_return_must_info_to_sg( b, sg_params, error_state );
    ubus_send_reply( ctx, req, b->head );
    blob_buf_free( b );

}


//=============================================================================
void
sgreq_get_all(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    int error_state = 0;
    struct sg_params params_data;
    static struct blob_buf b;
    blob_buf_init( &b, 0 );

    memset( &params_data, 0, sizeof( params_data ) );
    params_data.cmd = EXTEND_GET_ALL;
    params_data.data = &b;

    sgreq_get_wan( ctx, data, req, &params_data );
    sgreq_get_lan( ctx, data, req, &params_data );
    sgreq_get_vlan( ctx, data, req, &params_data );
    sgreq_get_switch_ports( ctx, data, req, &params_data );
    sgreq_get_wireless( ctx, data, req, &params_data );
    sgreq_get_client( ctx, data, req, &params_data );
    sgreq_get_device( ctx, data, req, &params_data );
    sgreq_get_guest_ssid( ctx, data, req, &params_data );
    sgreq_get_acl( ctx, data, req, &params_data );
    sgreq_get_qos( ctx, data, req, &params_data );
    sgreq_get_network_interface( ctx, data, req, &params_data );
    sgreq_get_controller( ctx, data, req, &params_data );
    sgreq_get_upgrade_auto( ctx, data, req, &params_data );
    sgreq_get_mesh_ssid( ctx, data, req, &params_data );
    sgreq_get_vpn_client( ctx, data, req, &params_data );
    sgreq_get_vpn_split_tunneling( ctx, data, req, &params_data );
    sgreq_get_port_mapping( ctx, data, req, &params_data );
    sgreq_get_dmz( ctx, data, req, &params_data );
    sgreq_get_upnp( ctx, data, req, &params_data );
    sgreq_get_ddns( ctx, data, req, &params_data );
    sgreq_get_ipv4_static_route( ctx, data, req, &params_data );
    sgreq_get_client_limit( ctx, data, req, &params_data );
    sgreq_get_portal_policy( ctx, data, req, &params_data );
    sgreq_get_usb_share(ctx, data, req, &params_data);
    sgreq_get_firewall_dos( ctx, data, req, &params_data );

    sgreq_return_must_info_to_sg( &b, sg_params, error_state );
    ubus_send_reply( ctx, req, b.head );
    blob_buf_free( &b );
}

//=============================================================================
static void
sgreq_unauthenticated_lan_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_attr *tb[__NETWORK_ATTR_MAX];
    struct blob_buf *reply = (struct blob_buf *)req->priv;
    struct blob_attr *cur_table;
    char address[IP4ADDR_MAX_LEN+1] = { 0 };
    char mask[IP4ADDR_MAX_LEN+1] = { 0 };
    int i = 0;

    blobmsg_parse( network_attrs_policy,
        __NETWORK_ATTR_MAX,
        tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( tb[NETWORK_ATTR_IPV4_ADDRESS] ) {
        blobmsg_for_each_attr( cur_table, tb[NETWORK_ATTR_IPV4_ADDRESS], i ) {
            if ( blobmsg_type( cur_table ) != BLOBMSG_TYPE_TABLE ) {
                continue;
            }

            config_parse_ipv4_addr_attr( cur_table, mask, address );

            blobmsg_add_string( reply, "lanIp4Address", address );
        }
    }
}

//=============================================================================
/*static void
sgreq_unauthenticated_upgrade_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_attr *tb[__UPGRADE_DEVICES_MAX];
    struct blob_buf *reply = (struct blob_buf *)req->priv;

    blobmsg_parse( upgrade_devices_policy,
        __UPGRADE_DEVICES_MAX,
        tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( tb[DEVAP] && tb[DEVMESH]) {

        blobmsg_add_string( reply, "upgradeAp", blobmsg_get_string( tb[DEVAP] ) );
        blobmsg_add_string( reply, "upgradeMesh", blobmsg_get_string( tb[DEVMESH] ) );
    }
}*/
//=============================================================================
void
sgreq_ext_unauthenticated_info(
    struct ubus_context *ctx,
    struct blob_attr *data,
    struct ubus_request_data *req,
    struct sg_params* sg_params
)
//=============================================================================
{
    struct blob_attr *tb_input[__EX_UNAUTHENTICATED_MAX];
    int ret = 0;
    int error_state = 0;
    uint32_t search_id;
    char value[BUF_LEN_128];
    char mac[MAC_STR_MAX_LEN+1] = { 0 };
    char devPassword[BUF_LEN_128] = { 0 };
    struct cm_config *cm_cfg = NULL;
    struct cm_config *cm_general_cfg = NULL;
    struct blob_attr *tb[__CONTROLLER_MAX];
    struct blob_attr *tb_general[__GENERAL_MAX];

    blobmsg_parse( ex_unauthenticated_policy,
        __EX_UNAUTHENTICATED_MAX,
        tb_input,
        blobmsg_data( data ),
        blobmsg_len( data ) );

    blob_buf_init( &reply, 0 );

    memset(value, 0, sizeof(value));
    ret = util_read_file_content("/proc/gxp/dev_info/dev_id", value, sizeof(value));
    if( !ret )
        blobmsg_add_string( &reply, "devProductName", value );

    cm_cfg = util_get_vltree_node( &cm_controller_vltree, VLTREE_CM_TREE, "main" );
    if( !cm_cfg ) {
        //mode default is route
        cfmanager_log_message( L_ERR, "No controller information found in %s\n", CFMANAGER_CONFIG_NAME );
        blobmsg_add_string( &reply, "role", "slave" );
        blobmsg_add_string( &reply, "workMode", "0" );
    }
    else {
        blobmsg_parse( controller_policy,
            __CONTROLLER_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config ) );

        if( tb[CONTROLLER_ROLE] ) {
            blobmsg_add_string( &reply, "role", blobmsg_get_string(tb[CONTROLLER_ROLE]) );
        }
        if( tb[CONTROLLER_WORK_MODE] ) {
            blobmsg_add_string( &reply, "workMode", blobmsg_get_string(tb[CONTROLLER_WORK_MODE]) );
        }
    }

    /* get the device isPaired status */
    cm_general_cfg = util_get_vltree_node( &cm_general_vltree, VLTREE_CM_TREE, "general" );
    if( !cm_general_cfg ) {
        cfmanager_log_message( L_ERR, "No general session found in %s\n", CFMANAGER_CONFIG_NAME );
        blobmsg_add_string( &reply, "isPaired", "0" );
    }
    else {
        blobmsg_parse( general_policy,
            __GENERAL_MAX,
            tb_general,
            blob_data( cm_general_cfg->cf_section.config ),
            blob_len( cm_general_cfg->cf_section.config ) );

        if( tb_general[GENERAL_PAIRING_KEY] ) {
            blobmsg_add_string( &reply, "isPaired", "1" );
        }
        else {
            blobmsg_add_string( &reply, "isPaired", "0" );
        }
    }

    memset(value, 0, sizeof(value));
    ret = util_read_file_content("/proc/gxp/dev_info/dev_mac", value, sizeof(value));
    if( !ret ) {
        util_str_erase_colon( value, mac );
        blobmsg_add_string( &reply, "devMac", mac );
    }

    if ( !ctx || ubus_lookup_id( ctx, "network.interface.lan0_zone0", &search_id ) ) {
        error_state = ERRCODE_INTERNAL_ERROR;
        cfmanager_log_message( L_DEBUG, "Failed to look up network.interface.lan0_zone0 method" );
        goto return_value;
    }
    ubus_invoke( ctx, search_id, "status", NULL, sgreq_unauthenticated_lan_cb, &reply, 1000 );
/*    if ( !ctx || ubus_lookup_id( ctx, "controller.discovery", &search_id ) ) {
        error_state = ERRCODE_INTERNAL_ERROR;
        cfmanager_log_message( L_DEBUG, "Failed to look up controller.discovery method" );
        goto return_value;
    }
    ubus_invoke( ctx, search_id, "get_upgrade_requirements", NULL, sgreq_unauthenticated_upgrade_cb, &reply, 1000 );
*/
    if ( tb_input[EX_UNAUTHENTICATED_FROM]
        && ( !strcmp( blobmsg_get_string( tb_input[EX_UNAUTHENTICATED_FROM] ), "cloud" )
            || !strcmp( blobmsg_get_string( tb_input[EX_UNAUTHENTICATED_FROM] ), "app" ) ) ) {
        memset(value, 0, sizeof(value));
        ret = util_read_file_content(
                "/proc/gxp/dev_info/security/ssid_password",
                value, sizeof( value ) );
        if( !ret ) {
            util_sha256sum( value, devPassword, sizeof( devPassword ) );
            blobmsg_add_string( &reply, "devPassword", devPassword );
        }
    }
    blobmsg_add_string( &reply, "devDomain", DEVICE_DOMAIN );


return_value:

    // TODO: workMode waiting to be supplemented

    sgreq_return_must_info_to_sg( &reply, sg_params, error_state );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );
}

