/****************************************************************************
* *
* * FILENAME:        $RCSfile: ubus.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2020/12/23
* *
* * DESCRIPTION:     xxxx feature: 
* *
* *
* * Copyright (c) 2020 by Grandstream Networks, Inc.
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
#ifndef __UBUS_H__
#define __UBUS_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================
enum {
    EXTEND_MIN,
    EXTEND_GET_ALL,

    __EXTEND_CMD_MAX
};

//=================
//  Typedefs
//=================

//=================
//  Globals
//=================
struct sg_params {
    int id;
    int cmd;
    const char *hashID;
    const char *method;
    void *data;
};

struct me_info {
    char mac[MAC_STR_MAX_LEN+1];
};
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
cfubus_init(
    const char *path
);

void
cfubus_done(
    void
);

void
cfubus_wss_error_state_handle(
    int error_state,
    char *erro_buf,
    int error_buf_size
);

int
cfubus_get_vpn_status(
    char *intf,
    int *status
);

int
cfubus_ctrl_vpn_status(
    char *intf,
    int action
);

void
cfubus_event_config_change(
    void
);

void
send_uns_notification(
    struct ubus_context *ctx,
    const char* msg,
    int priority
);

void
cfubus_get_me_info(
    struct me_info *meif
);

void
cfubus_get_wan_mac_info(
    char *mac,
    int wan_type
);

bool
cfubus_judge_need_close_radio(
    const char *country,
    const char *channel_with
);

struct ubus_context *
cfubus_get_ubus_context(
    void
);

unsigned int
cfubus_get_netmask(
    int wan_type
);

void
cfubus_resync_slave_cfg(
    void
);

#endif //__UBUS_H__
