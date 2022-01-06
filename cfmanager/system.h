/****************************************************************************
* *
* * FILENAME:        $RCSfile:system.h,v $
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
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

//=================
//  Includes
//=================
#include "ubus.h"

//=================
//  Defines
//=================
#define SYS_CONFIG_NAME "system"

//=================
//  Typedefs
//=================
struct sys_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
};

struct country_list {
    const char *value;
    const char *label;
};

struct zone_list {
    const char *value;
    const char *label;
};

//=================
//  Globals
//=================
extern struct vlist_tree basic_system_vltree;
extern const char* timezone_values[];
extern const char* timezone_names[];
extern const int timezone_num;
extern const struct country_list countries[];
extern struct zone_list timezones_name[];
extern const int countries_num;
extern const int timezones_name_num;

//=================
//  Locals
//=================

//=================
//  Functions
//=================
void
cfparse_sys_deinit(
    void
);

void
cfparse_sys_init(
    void
);

int
cfparse_load_sys(
    void
);


#endif
