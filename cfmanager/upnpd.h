/****************************************************************************
*
* FILENAME:        upnp.h
*
* DESCRIPTION:     Description of this header file's contents
*
* Copyright (c) 2017 by Grandstream Networks, Inc.
* All rights reserved.
*
* This material is proprietary to Grandstream Networks, Inc. and,
* in addition to the above mentioned Copyright, may be
* subject to protection under other intellectual property
* regimes, including patents, trade secrets, designs and/or
* trademarks.
*
* Any use of this material for any purpose, except with an
* express license from Grandstream Networks, Inc. is strictly
* prohibited.
*
***************************************************************************/

#ifndef __UPNPD_H__
#define __UPNPD_H__

//===========================
// Includes
//===========================

//===========================
// Defines
//===========================

//===========================
// Typedefs
//===========================
struct upnpd_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
};

enum {
    UPNPD_ENABLE,
    UPNPD_ENABLE_NATPMP,
    UPNPD_ENABLE_UPNP,
    UPNPD_SECURE_MODE,
    UPNPD_LOG_OUTPUT,
    UPNPD_PORT,
    UPNPD_UPNP_LEASE_FILE,
    UPNPD_EXTERNAL_IFACE,
    UPNPD_INTERNAL_IFACE,

    __UPNPD_MAX
};

//===========================
// Globals
//===========================
extern struct vlist_tree upnpd_vltree;
extern const struct blobmsg_policy upnpd_policy[__UPNPD_MAX];

//===========================
// Functions
//===========================
void
cfparse_upnpd_init(
    void
);

void
cfparse_upnpd_deinit(
    void
);

int
cfparse_load_upnpd(
    void
);

int
upnpd_parse_upnp(
    struct blob_attr *attr,
    int *fw_need_load
);

int
upnpd_reset_by_wan(
    char *wan
);

#endif
/* EOF */

