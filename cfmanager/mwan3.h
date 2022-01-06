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
#ifndef __MWAN3_H__
#define __MWAN_H__
//=================
//  Includes
//=================
#include "global.h"
#include "firewall.h"

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
struct mwan3_rule {
   const char *use_policy;
   const char *ipset;
   const char *src;
   const char *proto;
   const char *sticky;
   const char *timeout;
   const char *src_ip;
   const char *src_mac;
   const char *src_port;
   const char *dest_ip;
   const char *dest_port;
   int is_default;
   int is_split;
   int vpn_sence;
   const char *vpn_iface;
   const char *icmp_type[ICMP_TYPE_MAX];
   int icmp_type_cnt;
   const char *family;
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
cfparse_load_mwan3(
    void
);

#endif
