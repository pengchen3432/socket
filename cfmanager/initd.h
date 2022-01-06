/****************************************************************************
* *
* * FILENAME:        $RCSfile: initd.h,v $
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
#ifndef __INITD_H__
#define __INITD_H__
//=================
//  Includes
//=================
#include <libubox/list.h>

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
struct initd {
    struct list_head list;
    char *name;
    int   reload;
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
initd_load(
    void
);

void
initd_unload(
    void
);

void
initd_apply(
    char *command,
    int cmd_size
);

struct initd *
initd_get(
    const char *name
);
#endif //__INITD_H__