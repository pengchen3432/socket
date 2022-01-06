/****************************************************************************
* *
* * FILENAME:        $RCSfile: usb.c,v $
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
//=================
//  Includes
//=================
#include "usb.h"

#include "utils.h"

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

const struct blobmsg_policy usb_storage_partition_event_policy[__USB_STORAGE_PARTITION_EVENT_MAX] = {
        [USB_STORAGE_PARTITION_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_BOOL },
        [USB_STORAGE_PARTITION_DEVICE_PATH] = { .name = "device_path", .type = BLOBMSG_TYPE_STRING },
        [USB_STORAGE_PARTITION_MOUNT_PATH] = { .name = "mount_path", .type = BLOBMSG_TYPE_STRING },
        [USB_STORAGE_PARTITION_LABEL] = { .name = "label", .type = BLOBMSG_TYPE_STRING },
        [USB_STORAGE_PARTITION_UUID] = { .name = "uuid", .type = BLOBMSG_TYPE_STRING },
        [USB_STORAGE_PARTITION_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
};
//=================
//  Functions
//=================

int
usb_get_devices_info(
    struct usb_partition_info * usb_partition
)
//=============================================================================
{
    int ret = 0;
    FILE *file;
    char result_line[BUF_LEN_128];

    file=popen("usb.sh show partitions", "r");
    if( NULL != file ) {
        if( NULL != fgets(result_line, sizeof(result_line), file) ) {
            sscanf(result_line,
                   "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s",
                   usb_partition->device_path,
                   usb_partition->mount_path,
                   usb_partition->name,
                   usb_partition->uuid,
                   usb_partition->format,
                   usb_partition->total,
                   usb_partition->usable);

            cfmanager_log_message(L_DEBUG,
                  "found a partition %s, mount path: %s, label: %s, uuid: %s, type: %s, total size: %s, usbable size: %s.",
                  usb_partition->device_path,
                  usb_partition->mount_path,
                  usb_partition->name,
                  usb_partition->uuid,
                  usb_partition->format,
                  usb_partition->total,
                  usb_partition->usable );
            ret = 1;
        }
        pclose(file);
    }
    return ret;
}
