/****************************************************************************
* *
* * FILENAME:        $RCSfile: network.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/19
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
#ifndef __NETWORK_H__
#define __NETWORK_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================
#define CF_CONFIG_NAME_NETWORK      "network"
#define IP4ADDR_MAX_LEN             15
#define ADDR_MAX_LEN                65
#define IP6ADDR_MAX_LEN             47
//=================
//  Typedefs
//=================
enum {
    NETWORK_INTERFACE_IFNAME,
    NETWORK_INTERFACE_FORCE_LINK,
    NETWORK_INTERFACE_TYPE,
    NETWORK_INTERFACE_EMPTY,
    NETWORK_INTERFACE_PROTO,
    NETWORK_INTERFACE_MTU,
    NETWORK_INTERFACE_IPADDR,
    NETWORK_INTERFACE_NETMASK,
    NETWORK_INTERFACE_REQOPS,
    NETWORK_INTERFACE_DNS,
    NETWORK_INTERFACE_GATEWAY,
    NETWORK_INTERFACE_METRIC,
    NETWORK_INTERFACE_MULTICAST_QUERIER,
    NETWORK_INTERFACE_IEEE1905MANAGED,
    NETWORK_INTERFACE_USERNAME,
    NETWORK_INTERFACE_PASSWORD,
    NETWORK_INTERFACE_SERVER,
    NETWORK_INTERFACE_INTERFACE,
    NETWORK_INTERFACE_PEERDNS,
    NETWORK_INTERFACE_DELEGATE,
    NETWORK_INTERFACE_MPPE,
    NETWORK_INTERFACE_CHECKUP_INTERVAL,
    //NETWORK_INTERFACE_GS_BLOCK_RESTART,
    NETWORK_INTERFACE_GS_IP_TYPE,
    NETWORK_INTERFACE_GS_LOCAL_IP,
    NETWORK_INTERFACE_GS_REMOTE_IP,
    NETWORK_INTERFACE_GS_NETMASK,
    NETWORK_INTERFACE_GS_STATIC_DNS,
    NETWORK_INTERFACE_GS_DNS1,
    NETWORK_INTERFACE_GS_DNS2,

    __NETWORK_INTERFACE_MAX
};

enum {
    NETWORK_ALIAS_INTERFACE,
    NETWORK_ALIAS_PROTO,
    NETWORK_ALIAS_IPADDR,
    NETWORK_ALIAS_NETMASK,
    NETWORK_ALIAS_GATEWAY,

    __NETWORK_ALIAS_MAX
};

enum {
    NETWORK_DEVICE_NAME,
    NETWORK_DEVICE_MACADDR,

    __NETWORK_DEVICE_MAX
};

enum {
    NETWORK_PORT_TYPE_CLOSE,
    NETWORK_PORT_TYPE_UNTAG,
    NETWORK_PORT_TYPE_TAG,

    __NETWORK_PORT_TYPE_MAX
};

struct network_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
    char ipv4[IP4ADDR_MAX_LEN+1];
    int proto;
};
//=================
//  Globals
//=================
extern struct vlist_tree network_interface_vltree;
extern struct vlist_tree network_switch_vltree;
extern struct vlist_tree network_switch_vlan_vltree;
extern struct vlist_tree network_gs_switch_port_vltree;
extern struct vlist_tree network_global_vltree;
extern struct vlist_tree network_device_vltree;
extern struct vlist_tree network_alias_vltree;
extern struct vlist_tree network_route6_vltree;

extern const struct blobmsg_policy network_interface_policy[__NETWORK_INTERFACE_MAX];
extern const struct blobmsg_policy network_device_policy[__NETWORK_DEVICE_MAX];
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
cfparse_network_init(
    void
);

int
cfparse_load_network(
    void
);

void
cfparse_network_deinit(
    void
);

#endif //__NETWORK_H__