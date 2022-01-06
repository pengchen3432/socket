/****************************************************************************
* *
* * FILENAME:        $RCSfile: gsportalcfg.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/12/06
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
#ifndef _GSPORTALCFG_H
#define _GSPORTALCFG_H
//=================
//  Includes
//=================

//=================
//  Defines
//=================
#define MAX_PORTAL_POLICY_NUMBER 32

//=================
//  Typedefs
//=================
enum {
    GSPORTALCFG_VLTREE_ALL,
    GSPORTALCFG_VLTREE_WIFI_PORTAL,
    GSPORTALCFG_VLTREE_PORTAL_POLICY,

    __GSPORTALCFG_VLTREE_MAX
};

enum {
    GSPORTALCFG_HOOKER_NONE,
    GSPORTALCFG_HOOKER_WIFI_PORTAL,
    GSPORTALCFG_HOOKER_PORTAL_POLICY,


    __GSPORTALCFG_HOOKER_MAX,
};


struct gsportalcfg_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
};


enum {
    __WIFI_PORTAL_MIN = 0,
    WIFI_PORTAL_ENABLE = __WIFI_PORTAL_MIN,
    WIFI_PORTAL_POLICY,
    WIFI_PORTAL_SSID,
    WIFI_PORTAL_LAN_INTERFACE,

    __WIFI_PORTAL_MAX
};

enum {
    __PORTAL_POLICY_MIN = 0,
    PORTAL_POLICY_ID = __PORTAL_POLICY_MIN,
    PORTAL_POLICY_NAME,
    PORTAL_POLICY_AUTH_TYPE,
    PORTAL_POLICY_EXPIRATION,
    PORTAL_POLICY_IDLE_TIMEOUT,
    PORTAL_POLICY_PAGE_PATH,
    PORTAL_POLICY_LANDING_PAGE,
    PORTAL_POLICY_LANDING_PAGE_URL,
    PORTAL_POLICY_ENABLE_HTTPS,
    PORTAL_POLICY_ENABLE_HTTPS_REDIRECT,
    PORTAL_POLICY_ENABLE_FAIL_SAFE,
    PORTAL_POLICY_ENABLE_DALY_LIMIT,
    PORTAL_POLICY_RADIUS_SERVER,
    PORTAL_POLICY_RADIUS_PORT,
    PORTAL_POLICY_RADIUS_SECRET,
    PORTAL_POLICY_RADIUS_METHOD,
    PORTAL_POLICY_RADIUS_DYNAMIC_VLAN,
    PORTAL_POLICY_RADIUS_ACCT_SERVER,
    PORTAL_POLICY_RADIUS_ACCT_PORT,
    PORTAL_POLICY_RADIUS_ACCT_SECRET,
    PORTAL_POLICY_RADIUS_ACCT_UPDATE_INTERVAL,
    PORTAL_POLICY_RADIUS_NAS_ID,
    PORTAL_POLICY_ENABLE_THIRD,
    PORTAL_POLICY_ENABLE_BYTE_LIMIT,
    PORTAL_POLICY_WECHAT_SHOP_ID,
    PORTAL_POLICY_WECHAT_APP_ID,
    PORTAL_POLICY_WECHAT_SECRET_KEY,
    PORTAL_POLICY_FB_APP_ID,
    PORTAL_POLICY_FB_APP_SECRET,
    PORTAL_POLICY_FB_OAUTH_DOMAIN,
    PORTAL_POLICY_FB_OAUTH_PORT,
    PORTAL_POLICY_FB_REAUTH,
    PORTAL_POLICY_GG_CLIENT_ID,
    PORTAL_POLICY_GG_CLIENT_SECRET,
    PORTAL_POLICY_TWITER_ID,
    PORTAL_POLICY_TWITER_CONSUMER_KEY,
    PORTAL_POLICY_TWITER_CONSUMER_SECRET,
    PORTAL_POLICY_TWITER_FORCE_TO_FOLLOW,
    PORTAL_POLICY_SIMPLE_PASSWD,
    PORTAL_POLICY_OM_SPLASH_PAGE_URL,
    PORTAL_POLICY_OM_SPLASH_PAGE_SECRET,
    PORTAL_POLICY_OM_SPLASH_PAGE_AUTH_URL,
    PORTAL_POLICY_OM_SPLASH_PAGE_AUTH_SECRET,
    PORTAL_POLICY_PRE_AUTH,
    PORTAL_POLICY_POST_AUTH,


    __PORTAL_POLICY_MAX,
};


//=================
//  Globals
//=================
extern struct vlist_tree gsportalcfg_wifi_portal_vltree;
extern struct vlist_tree gsportalcfg_portal_policy_vltree;
extern const struct blobmsg_policy gsportalcfg_wifi_portal_policy[__WIFI_PORTAL_MAX];
extern const struct blobmsg_policy gsportalcfg_portal_policy[__PORTAL_POLICY_MAX];

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
cfparse_load_gsportalcfg(
    void
);

void
cfparse_gsportalcfg_init(
    void
);

void
cfparse_gsportalcfg_deinit(
    void
);
#endif //GSPORTALCFG_H
