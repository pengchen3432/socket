/****************************************************************************
*
* FILENAME:        ddns.h
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

#ifndef __DDNS_H__
#define __DDNS_H__

//===========================
// Includes
//===========================
#include "global.h"

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
ddns_delete_by_wan(
    char *wan
);

int
ddns_parse(
    struct blob_attr *attr
);

#endif
/* EOF */

