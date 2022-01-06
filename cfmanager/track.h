/****************************************************************************
* *
* * FILENAME:        $RCSfile: track.h,v $
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
#ifndef __TRACK_H__
#define __TRACK_H__
//=================
//  Includes
//=================
#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <uci.h>
#include <stdint.h>

//=================
//  Defines
//=================
#ifndef BIT
#define BIT(x) ( 1U << (x) )
#endif

//=================
//  Typedefs
//=================
typedef enum {
    CONFIG_TRACK_WIRELESS,
    CONFIG_TRACK_NETWORK,
    CONFIG_TRACK_DHCP,
    CONFIG_TRACK_FIREWALL,
    CONFIG_TRACK_SCHEDULE,
    CONFIG_TRACK_QOS,
    CONFIG_TRACK_BLACKHOLE,
    CONFIG_TRACK_SYSTEM,
    CONFIG_TRACK_EMAIL,
    CONFIG_TRACK_NOTIFY,
    CONFIG_TRACK_CONTROLLER,
    CONFIG_TRACK_UPNPD,
    CONFIG_TRACK_DDNS,
    CONFIG_TRACK_GRANDSTREAM,
    CONFIG_TRACK_MWAN3,
    CONFIG_TRACK_BWCTRL,
    CONFIG_TRACK_WIFIISOLATE,
    CONFIG_TRACK_SAMBA,
    CONFIG_TRACK_GSPORTAL,
    CONFIG_TRACK_PORTALCGI,
    CONFIG_TRACK_TR069,
    CONFIG_TRACK_ECM,

    __CONFIG_TRACK_MAX
} cfg_track;

enum {
    APPLY_CMPL,
    APPLY_IN_PROCESS
};

enum {
    OPTION_FLAG_RELOAD,
    OPTION_FLAG_CHANGED,

    __OPTION_FLAG_MAX
};

struct track
{
    struct avl_node avl;
    char *name;
    char *init;
    char *exec;
    uint32_t    affects;
    uint32_t    flags;
};

struct track_exec_param {
    const char *name;
};

//=================
//  Globals
//=================
extern int apply_status;
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
track_init(
    void
);

void
track_deinit(
    void
);

int
tracks_load(
    void
);

struct track *
track_get(
    const char *name
);

void
track_apply(
    void
);

#endif //__TRACK_H__