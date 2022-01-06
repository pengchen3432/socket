/****************************************************************************
* *
* * FILENAME:        $RCSfile: usb.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/14
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
#ifndef __USB_H__
#define __USB_H__

//=================
//  Includes
//=================
#include "global.h"
//=================
//  Defines
//=================
#define USB_DEVICE_INFO_SIZE_MAX 64
//=================
//  Typedefs
//=================
enum {
    USB_STORAGE_PARTITION_ACTION,
    USB_STORAGE_PARTITION_DEVICE_PATH,
    USB_STORAGE_PARTITION_MOUNT_PATH,
    USB_STORAGE_PARTITION_LABEL,
    USB_STORAGE_PARTITION_UUID,
    USB_STORAGE_PARTITION_TYPE,

    __USB_STORAGE_PARTITION_EVENT_MAX
};

enum {
    USB_ACTION_DETACHED,
    USB_ACTION_ATTACHED,
    USB_ACTION_NOACTION,
};

struct usb_partition_info {
    char name[USB_DEVICE_INFO_SIZE_MAX];
    char uuid[USB_DEVICE_INFO_SIZE_MAX];
    char device_path[USB_DEVICE_INFO_SIZE_MAX];
    char mount_path[USB_DEVICE_INFO_SIZE_MAX];
    char format[USB_DEVICE_INFO_SIZE_MAX];
    char total[USB_DEVICE_INFO_SIZE_MAX];
    char usable[USB_DEVICE_INFO_SIZE_MAX];
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
usb_get_devices_info(
    struct usb_partition_info * usb_partition
);

#endif //__USB_H__