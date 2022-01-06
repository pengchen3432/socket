/****************************************************************************
* *
* * FILENAME:        $RCSfile: cm2gs.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/11/21
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
#ifndef __CM2GS_H__
#define __CM2GS_H__
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

int
cm2gs_extern_sys_log_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
);

int
cm2gs_email_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
);

int
cm2gs_notification_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
);

int
cm2gs_tr069_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
);

int
cm2gs_ap_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
);

int
cm2gs_additional_ssid_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
);

int
cm2gs_radio_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
);

#endif //__CM2GS_H__
