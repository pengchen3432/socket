/****************************************************************************
* *
* * FILENAME:        $RCSfile: time.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/15
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
#ifndef __TIME_H__
#define __TIME_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================

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
time_init(
    void
);

void
firewall_restart_timer_start(
    void
);

void
mwan3_restart_timer_start(
    void
);

void
apply_timer_start(
    void
);

void
cfmanager_resync_timer_start(
    void
);

void
apply_execl_timer_start(
    void
);

void
apply_execl_timer_stop(
    void
);

#endif //__TIME_H__
