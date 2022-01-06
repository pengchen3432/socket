/****************************************************************************
* *
* * FILENAME:        $RCSfile: notification.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/17
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
//=================
//  Includes
//=================
#include <stdlib.h>
#include "notification.h"
#include "initd.h"
#include "time.h"
#include "track.h"
#include "apply.h"

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
//=============================================================================
int
cfparse_load_notification(
    void
)
//=============================================================================
{
    /*
     * notification does not need to be compared, it needs to be reset every time
     */
    apply_add( "notification" );
    apply_timer_start();
    return 0;
}