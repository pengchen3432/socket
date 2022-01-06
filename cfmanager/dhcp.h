/****************************************************************************
* *
* * FILENAME:        $RCSfile: dhcp.h,v $
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
#ifndef _DHCP_H
#define _DHCP_H
//=================
//  Includes
//=================

//=================
//  Defines
//=================
#define CF_CONFIG_NAME_DHCP      "dhcp"

struct dhcp_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
};
//=================
//  Typedefs
//=================
enum {
    DHCP_DNSMASQ_DOMAINNEEDED,
    DHCP_DNSMASQ_BOGUSPRIV,
    DHCP_DNSMASQ_FILTERWIN2K,
    DHCP_DNSMASQ_LOCALISE_QUERIES,
    DHCP_DNSMASQ_HIJACK_PROTECTION,
    DHCP_DNSMASQ_REBIND_LOCALHOST,
    DHCP_DNSMASQ_LOCAL,
    DHCP_DNSMASQ_DOMAIN,
    DHCP_DNSMASQ_EXPANDHOSTS,
    DHCP_DNSMASQ_NONEGCACHE,
    DHCP_DNSMASQ_AUTHORITATIVE,
    DHCP_DNSMASQ_READETHERS,
    DHCP_DNSMASQ_LEASEFILE,
    DHCP_DNSMASQ_RESOLVFILE,
    DHCP_DNSMASQ_LOCALSERVICE,
    DHCP_DNSMASQ_CONNTRACK,
    DHCP_DNSMASQ_REBIND_PROTECTION,
    DHCP_DNSMASQ_IPSET,

    __DHCP_DNSMASQ_MAX
};

enum {
    DHCP_INTERFACE,
    DHCP_IGNORE,
    DHCP_START,
    DHCP_LIMIT,
    DHCP_DHCP_OPTION,    //list
    DHCP_LEASETIME,
    DHCP_NETWORK_ID,
    DHCP_DHCPV6,
    DHCP_RA,
    DHCP_NDP,
    DHCP_MASTER,

    __DHCP_MAX
};

enum {
    DHCP_HOST_MAC,
    DHCP_HOST_IP,

    __DHCP_HOST_MAX
};
//=================
//  Globals
//=================
extern const struct blobmsg_policy dhcp_policy[__DHCP_MAX];
extern const struct blobmsg_policy dhcp_host_policy[__DHCP_HOST_MAX];
extern struct vlist_tree dhcp_vltree;
extern struct vlist_tree dhcp_host_vltree;
extern struct vlist_tree dhcp_dnsmasq_vltree;

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
cfparse_dhcp_init(
    void
);

void
cfparse_dhcp_deinit(
    void
);

int
cfparse_load_dhcp(
    void
);

#endif //DHCP_H
