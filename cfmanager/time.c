/****************************************************************************
* *
* * FILENAME:        $RCSfile: time.c,v $
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
//=================
//  Includes
//=================
#include "global.h"
#include "time.h"
#include "track.h"
#include "apply.h"
#include "utils.h"
#include "wireless.h"
#include "ubus.h"

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
#define APPLY_TIME                  ( 2 * 1000 )
#define CFMANAGER_RESYNC_TIME       ( 3 * 1000 )
#define APPLY_EXECL_TIME            ( 30 * 1000 )
#define IFSTAT_RATE_TIME_S          3
#define IFSTAT_RATE_TIME            ( IFSTAT_RATE_TIME_S * 1000 )
//=================
//  Globals
//=================

//=================
//  Locals
//=================

/*
 * Private Functions
 */
static void
apply_timer_cb(
    struct uloop_timeout *timeout
);

static void
apply_execl_timer_cb(
    struct uloop_timeout *timeout
);

static void
cfmanager_resync_timer_cb(
    struct uloop_timeout *timeout
);

/*
 * Private Data
 */
static struct uloop_timeout apply_timer;
static struct uloop_timeout apply_execl_timer;
static struct uloop_timeout cfmanager_resync_timer;
//=================
//  Functions
//=================

//=============================================================================
void
time_init(
    void
)
//=============================================================================
{
    memset( &apply_timer, 0, sizeof( apply_timer ) );
    apply_timer.cb = apply_timer_cb;

    memset( &cfmanager_resync_timer, 0, sizeof( cfmanager_resync_timer ) );
    cfmanager_resync_timer.cb = cfmanager_resync_timer_cb;

    memset( &apply_execl_timer, 0, sizeof( apply_execl_timer ) );
    apply_execl_timer.cb = apply_execl_timer_cb;
}

//=============================================================================
static void
apply_timer_cb(
    struct uloop_timeout *timeout
)
//=============================================================================
{
    apply_exec();
}

//=============================================================================
static void
apply_execl_timer_cb(
    struct uloop_timeout *timeout
)
//=============================================================================
{
    apply_status = APPLY_CMPL;

    cfmanager_log_message( L_DEBUG, "Reset the status of apply_status\n" );
}

//=============================================================================
void
apply_timer_start(
    void
)
//=============================================================================
{
    if( cm_sve_init.boot_start || cm_sve_init.first_load_cfg ) {
        cfmanager_log_message( L_ERR, "Wait for configuration loading to complete\n" );
        return;
    }

    if ( uloop_timeout_remaining( &apply_timer ) > 0 ) {
        return;
    }

    uloop_timeout_set( &apply_timer, APPLY_TIME );
}

//=============================================================================
void
apply_execl_timer_start(
    void
)
//=============================================================================
{
    uloop_timeout_set( &apply_execl_timer, APPLY_EXECL_TIME );
}

//=============================================================================
void
apply_execl_timer_stop(
    void
)
//=============================================================================
{
    uloop_timeout_cancel( &apply_execl_timer );
}

//=============================================================================
static void
cfmanager_resync_timer_cb(
    struct uloop_timeout *timeout
)
//=============================================================================
{
    //system( "ubus call controller.icc notify_slave_check_cm &" );
    cfmanager_log_message( L_DEBUG, "resync config\n" );
    cfubus_resync_slave_cfg();
}

//=============================================================================
void
cfmanager_resync_timer_start(
    void
)
//=============================================================================
{
    if ( uloop_timeout_remaining( &cfmanager_resync_timer ) > 0 ) {
        return;
    }

    uloop_timeout_set( &cfmanager_resync_timer, CFMANAGER_RESYNC_TIME );
}

