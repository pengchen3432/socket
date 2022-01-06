/****************************************************************************
* *
* * FILENAME:        $RCSfile: schedule.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/04
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
#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================
#define CF_CONFIG_NAME_SCHEDULE      "schedule"


enum {
    SCH_SSID,
    SCH_CLIENT_ACCESS,
    SCH_BWCTRL,
    SCH_FIREWALL,
    SCH_REBOOT,
    SCH_UPGRADE,
    SCH_WTIME1,
    SCH_WTIME2,
    SCH_WTIME3,
    SCH_WTIME4,
    SCH_WTIME5,
    SCH_WTIME6,
    SCH_WTIME7,
    SCH_ABTIME_LISTS,

    __SCH_MAX
};

//=================
//  Typedefs
//=================
struct schedule_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
    int sch_option;         //What are the items int the schedule
};
//=================
//  Globals
//=================
extern const struct blobmsg_policy schedule_policy[__SCH_MAX];
extern struct vlist_tree sch_vltree;
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
cfparse_load_schedule(
    void
);

void
cfparse_schedule_init(
    void
);

void
cfparse_schedule_deinit(
    void
);

#endif //__SCHEDULE_H__
