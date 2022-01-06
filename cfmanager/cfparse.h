/****************************************************************************
* *
* * FILENAME:        $RCSfile: cfparse.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2020/12/22
* *
* * DESCRIPTION:     xxxx feature:
* *
* *
* * Copyright (c) 2020 by Grandstream Networks, Inc.
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
#ifndef __CFPARSE_H__
#define __CFPARSE_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
enum {
    CONFIG_MIN,
    CONFIG_WIRELESS,
    CONFIG_NETWORK,
    CONFIG_DHCP,
    CONFIG_FIREWALL,

    __CONFIG_MAX
}CONFIG_NAME;

enum {
    CONFIG_LOAD_STATUS_MIN,
    CONFIG_FIRST_LOAD,
    CONFIG_ALREADY_LOADED,

    __CONFIG_LOAD_STATUS_MAX
}CONFIG_LOAD_STATUS;

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
void
cfparse_init(
    void
);

void
cfparse_deinit(
    void
);

struct uci_package *
cfparse_init_package(
    const char *config
);

int
cfparse_load_file(
    const char *config,
    int section,
    bool force_load
);

#endif //__CFPARSE_H__