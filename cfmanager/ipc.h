/****************************************************************************
* *
* * FILENAME:        $RCSfile: ipc.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/18
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
#ifndef __IPC_H__
#define __IPC_H__
//=================
//  Includes
//=================
#include <stdbool.h>
//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
enum {
    __IPC_SG_MAX
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
int
cfmanager_ipc_init(
    void
);

int
cfmanager_ipc_tx_msg(
    void *buf,
    int len,
    bool async
);

#endif //__IPC_H__