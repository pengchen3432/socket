/****************************************************************************
* *
* * FILENAME:        $RCSfile: firewall.h,v $
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
#ifndef __FIREWALL_H__
#define __FIREWALL_H__
//=================
//  Includes
//=================
#include <libubox/vlist.h>
#include <uci.h>

//=================
//  Defines
//=================
#define DEFAULT_LAN "lan0_zone0"

#define ICMP_TYPE_MAX 256

#define FIREWALL_CONFIG_NAME "firewall"

//=================
//  Typedefs
//=================

enum {
    WAN_TYPE_DHCP,
    WAN_TYPE_STATIC,
    WAN_TYPE_PPPOE,
    WAN_TYPE_PPTP,
    WAN_TYPE_L2TP,
    WAN_TYPE_INVALID
};

struct static_dns {
    int valid;
    const char *first;
    const char *second;
};

struct firewall_rule {
   const char *name;
   const char *family;
   const char *src;
   const char *src_ip;
   const char *src_mac;
   const char *src_port;
   const char *proto;
   const char *icmp_type[ICMP_TYPE_MAX];
   int icmp_type_cnt;
   const char *dest;
   const char *dest_ip;
   const char *dest_port;
   const char *start_date;
   const char *stop_date;
   const char *start_time;
   const char *stop_time;
   const char *weekdays;
   const char *monthdays;
   const char *utc_time;
   const char *target;
   const char *mark;            // Match a mark.
   const char *set_mark;
   const char *set_xmark;
   const char *extra;
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
int
firewall_delete_port_mapping_by_wan(
    char *wan
);

int
firewall_parse_port_mapping(
    struct blob_attr *attr,
    int *fw_need_load
);

int
firewall_delete_dmz_by_wan(
    char *wan
);

int
firewall_parse_dmz(
    struct blob_attr *attr,
    int *fw_need_load
);

int
firewall_reparse(
    void
);

int
cfparse_load_firewall(
    void
);

#endif //__FIREWALL_H__
