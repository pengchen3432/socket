/****************************************************************************
* *
* * FILENAME:        $RCSfile: wireless.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/12
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
#ifndef __WIRELESS_H__
#define __WIRELESS_H__
//=================
//  Includes
//=================
#include <libubox/vlist.h>
#include <uci.h>
#include <libubox/blobmsg.h>
#include "global.h"
#include "config.h"

//=================
//  Defines
//=================
#define NET_MESH                    "mesh"
#define NET_MASTER                  "master"
#define NET_GUEST                   "guest"
#define NET_ADDIT                   "additional"
#define NET_TYPE_OPTION_NAME        "network_type"
#define CHANNEL_MAX_2G              15
#define CHANNEL_MAX_5G              24
#define COUNTRY_DEFVALUE            840
//=================
//  Typedefs
//=================

enum
{
    WIRELESS_IFACE_DISABLED,
    WIRELESS_IFACE_IFNAME,
    WIRELESS_IFACE_NETWORK,
    WIRELESS_IFACE_MODE,
    WIRELESS_IFACE_MCAST_TO_UCAST,
    WIRELESS_IFACE_BINTVAL,
    WIRELESS_IFACE_DTIM_PERIOD,
    WIRELESS_IFACE_PROXYARP,
    WIRELESS_IFACE_MCASTENHANCE,
    WIRELESS_IFACE_WDS,
    WIRELESS_IFACE_DISABLECOEXT,
    WIRELESS_IFACE_HIDDEN,
    WIRELESS_IFACE_MAXSTA,
    WIRELESS_IFACE_MACLIST,
    WIRELESS_IFACE_MACFILTER,
    WIRELESS_IFACE_WMM,
    WIRELESS_IFACE_VHTSUBFER,
    WIRELESS_IFACE_VHTMUBFER,
    WIRELESS_IFACE_RSSI_ENABLE,
    WIRELESS_IFACE_RSSI_THRESHOLD,
    WIRELESS_IFACE_VOICE_ENTERPRISE,
    WIRELESS_IFACE_11R,
    WIRELESS_IFACE_11K,
    WIRELESS_IFACE_WNM,
    WIRELESS_IFACE_ISOLATE,
    WIRELESS_IFACE_ISOLATION_MODE,
    WIRELESS_IFACE_GATEWAY_MAC,
    WIRELESS_IFACE_PORTAL_ENABLE,
    WIRELESS_IFACE_PORTAL_POLICY,
    WIRELESS_IFACE_VLAN_TRUNK,
    WIRELESS_IFACE_MINIRATE_ENABLE,
    WIRELESS_IFACE_MINIRATE_THRESHOLD,
    WIRELESS_IFACE_VLAN,
    WIRELESS_IFACE_INACT,
    WIRELESS_IFACE_11W,
    WIRELESS_IFACE_SSID,
    WIRELESS_IFACE_ENCRYPTION,
    WIRELESS_IFACE_KEY,
    WIRELESS_IFACE_AUTH_SERVER,
    WIRELESS_IFACE_DYNAMIC_VLAN,
    WIRELESS_IFACE_AUTH_PORT,
    WIRELESS_IFACE_AUTH_SECRET,
    WIRELESS_IFACE_ACCT_SERVER,
    WIRELESS_IFACE_ACCT_PORT,
    WIRELESS_IFACE_ACCT_SECRET,
    WIRELESS_IFACE_NASID,
    WIRELESS_IFACE_NETWORK_TYPE,
    WIRELESS_IFACE_BSSID,
    WIRELESS_IFACE_SAE,
    WIRELESS_IFACE_SUITE_B,
    WIRELESS_IFACE_UAPSD,
    WIRELESS_IFACE_ID,

    __WIRELESS_IFACE_MAX
};

enum
{
    WIRELESS_DEVICE_DISABLED,
    WIRELESS_DEVICE_PHY,
    WIRELESS_DEVICE_OUTDOOR,
    WIRELESS_DEVICE_PUREG,
    WIRELESS_DEVICE_HWMODE,
    WIRELESS_DEVICE_HTMODE,
    WIRELESS_DEVICE_SHORTGI,
    WIRELESS_DEVICE_TXCHAINMASK,
    WIRELESS_DEVICE_RXCHAINMASK,
    WIRELESS_DEVICE_DBDC_ENABLE,
    WIRELESS_DEVICE_CHANNEL,
    WIRELESS_DEVICE_TXPOWER,
    WIRELESS_DEVICE_MUMIMO,
    WIRELESS_DEVICE_GEOANALYTIC,

    __WIRELESS_DEVICE_MAX
};

enum
{
    WIRELESS_GLOBAL_ATF_MODE,
    WIRELESS_GLOBAL_DFS,
    WIRELESS_GLOBAL_COUNTRY,

    __WIRELESS_GLOBAL_MAX
};

enum {
    RADIO_MIN,
    RADIO_2G,
    RADIO_5G,
    RADIO_DUAL,
    RADIO_MAX,
};

enum {
    NET_TYPE_MIN,
    NET_TYPE_MASTER,    //master ssid, ssid_id=MASTER_SSID_ID
    NET_TYPE_GUEST,     //guest ssid, ssid_id=GUEST_SSID_ID
    NET_TYPE_MESH,      //mesh ssid, ssid_id=MESH_SSID_ID
    NET_TYPE_ADDIT,     //SSID added in professional mode, ssid_id=cfmanager section name

    __NET_TYPE_MAX
};

enum {
    MAC_FILTER_DENY,
    MAC_FILTER_ALLOW
};

struct cfparse_wifi_device{
    struct vlist_node node;
    struct vlist_tree interfaces;
    struct config_section_content cf_section;
    char option_flags[__WIRELESS_DEVICE_MAX];
    bool disabled;
    int vif_idx;
    bool need_set_iface;        //The parameters set need to be determined by ath
    char htmode[BUF_LEN_8];
    int flags;
};

struct cfparse_wifi_interface {
    struct vlist_node node;
    struct config_section_content cf_section;
    struct  cfparse_wifi_device *cfwdev;
    char option_flags[__WIRELESS_IFACE_MAX];
    bool disabled;
    int network_type;
    char ssid_name[SSID_NAME_MAX_LEN+1];
    char macfilter[MACFILTER_MAX_LEN+1];
    char *maclist;
    bool sae;                   //Judge whether sae configuration item exists
    bool disablecoext;          //Judge whether disablecoext configuration item exists
    int flags;
    char ssid_id[BUF_LEN_16];   //Corresponds to section name in cfmanager, for example 'ssid0'
    bool rssi_enable;
    bool minirate_enable;
};

struct cfparse_wifi_global {
    struct vlist_node node;
    struct config_section_content cf_section;
    char option_flags[__WIRELESS_GLOBAL_MAX];
    char country[BUF_LEN_8];
};

//=================
//  Globals
//=================
extern struct vlist_tree wireless_vltree;
extern struct vlist_tree wireless_global_vltree;
extern const struct blobmsg_policy wireless_iface_policy[__WIRELESS_IFACE_MAX];
extern const struct blobmsg_policy wireless_device_policy[__WIRELESS_DEVICE_MAX];
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
cfparse_load_wireless(
    void
);

void
cfparse_wireless_init(
    void
);

void
cfparse_wireless_deinit(
    void
);

#endif //__WIRELESS_H__
