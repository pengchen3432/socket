/****************************************************************************
* *
* * FILENAME:        $RCSfile: apply.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/26
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
#ifndef __APPLY_H__
#define __APPLY_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
struct apply_list {
    struct list_head list;
    char *name;
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
void
apply_add(
    const char *name
);

void
apply_flush(
    void
);

void
apply_exec(
    void
);

void
apply_deinit(
    void
);

void
apply_set_reload_flag(
    cfg_track config
);

int
apply_get_reload_flag(
    cfg_track config
);

void
apply_flush_reload_flag(
    cfg_track config
);

void
apply_handle_all_reload_flag(
    void
);

#endif //__APPLY_H__

