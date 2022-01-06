/****************************************************************************
*
* FILENAME:        route.h
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

#ifndef __ROUTE_H__
#define __ROUTE_H__

//===========================
// Includes
//===========================

//===========================
// Defines
//===========================

//===========================
// Typedefs
//===========================

//===========================
// Globals
//===========================

//===========================
// Functions
//===========================
int
route_delete_ipv4_static_route_by_wan(
    char *wan
);

int
route_parse_ipv4_static_route(
    struct blob_attr *attr
);

int
route_parse_ipv6_static_route(
    struct blob_attr *attr
);

#endif
/* EOF */

