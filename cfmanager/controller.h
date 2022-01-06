/****************************************************************************
* *
* * FILENAME:        $RCSfile:contrller.h,v $
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
#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

//=================
//  Includes
//=================
#include "ubus.h"

//=================
//  Defines
//=================
#define CF_CONFIG_NAME_CONTROLLER      "controller"
#define STR_64 64

//=================
//  Typedefs
//=================
struct controller_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
};

enum {
    CONTROLLER_ROLE,
    CONTROLLER_WORK_MODE,
    CONTROLLER_SET_FROM,

    __CONTROLLER_MAX,
};

//=================
//  Globals
//=================
extern struct vlist_tree gs_controller_vltree;
extern const struct blobmsg_policy controller_policy[__CONTROLLER_MAX];

//=================
//  Locals
//=================

//=================
//  Functions
//=================
void
cfparse_controller_deinit(
    void
);

void
cfparse_controller_init(
    void
);

int
cfparse_load_controller(
    void
);

int
cfparse_controller_del_paired_devices(
    void
);

#endif
