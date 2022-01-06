/****************************************************************************
* *
* * FILENAME:        $RCSfile: utils.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/13
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
#ifndef __UTILS_H__
#define __UTILS_H__
//=================
//  Includes
//=================
#include <libubox/vlist.h>
#include <uci.h>

//=================
//  Defines
//=================
#define cfmanager_log_message(LOG_LEVEL, fmt, ...) do { cfmanager_log( LOG_LEVEL, "%s %d: " fmt, __func__, __LINE__, ##__VA_ARGS__ ); } while ( 0 );
#define DEFAULT_LOG_LEVEL   L_WARNING
#define STR_WORLD_MODLE     "0x006a"
#define WORLD_MODLE_PATH    "/proc/gxp/dev_info/dev_region"
#define SPLICE_STR(a,b)     a#b

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#endif

#ifndef __check_format_string
#define __check_format_string(pos_str, pos_args) \
     __attribute__ ((format (printf, (pos_str), (pos_args))))
#endif
//=================
//  Typedefs
//=================
enum {
    VLTREE_MIN,
    VLTREE_CM_TREE,
    VLTREE_DEV,
    VLTREE_IFACE,
    VLTREE_DEV_GLOB,
    VLTREE_DHCP,
    VLTREE_NETWORK,
    VLTREE_SYS,
    VLTREE_GRANDSTREAM,
    VLTREE_CONTR,
    VLTREE_UPNPD,
    VLTREE_BWCTRL,
    VLTREE_TR069,
    VLTREE_PORTAL,
    VLTREE_WIFI_PORTAL,
    VLTREE_SCHEDULE,

    __VLTREE_MAX
};

typedef enum {
    CEK_SSID_NAME,

    __CEK_MAX
} util_check_param;

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
cfmanager_log(
    int priority,
    const char *format,
    ...
) __check_format_string( 2, 3 );

void
cfmanager_set_log_level(
    int level
);

int
util_read_file_content(
    const char *path,
    char *value,
    int value_size
);

void
util_convert_mask_to_str(
    int mask,
    char *mask_str,
    int size
);

int
util_str_erase_colon(
    const char *in_str,
    char *out_str
);

void
util_formatted_mac_with_colo(
    const char *mac,
    char *output
);

int
util_str_to_low_case(
    char *str
);

int
util_format_macaddr(
    char *in,
    char *out
);

int
util_device_is_world_model(
    void
);

void
util_escape_single_quote(
    const char *src,
    char *dest,
    int len
);

const char*
uci_lookup_option_string_from_package_and_section (
    struct uci_context *uci_ctx,
    struct uci_package *p,
    const char *section_str,
    const char *option_name
);

int
interface_is_up(
    const char *devname
);

void*
util_get_vltree_node(
    struct vlist_tree *vltree,
    int vltree_type,
    const char *key
);

int
util_blobmsg_get_int(
    struct blob_attr *attr,
    int defalut_value
);

char*
util_blobmsg_get_string(
    struct blob_attr *attr,
    char *defalut_value
);

bool
util_blobmsg_get_bool(
    struct blob_attr *attr,
    bool defalut_value
);

int
util_split_string_data(
    json_object *data_obj,
    const char *in,
    char seperator,
    char assign
);

char *
util_rm_substr(
    char *str,
    const char *sub
);

int
util_parse_string_data(
    char *data,
    char *divider,
    const char *match,
    int pos,
    char *out,
    int out_size
);

void
util_sha256sum(
    const char *str,
    char *sum,
    int   size
);

int
util_validate_ipv4(
    char *value
);

int
util_validate_ipv6(
char *value
);

int
util_run_shell_command(
    const char * command,
    char * output,
    size_t len
);


uint32_t
utils_addr_to_int(
    const char *addr_string
);


void
utils_time_value2time_str(
    time_t time_val,
    char *time_str,
    int size
);

void
util_del_dup_cmd(
    char *cmd,
    int cmd_size
);

void
util_send_uns_notification(
    struct ubus_context *ctx,
    const char* msg,
    int priority
);

int
util_validate_legal(
    void *param,
    util_check_param type,
    int err_code
);

int
util_cpy_file (
    const char *src,
    const char *dest
);

int
util_convert_specific_char(
    const char *src,
    char *dest,
    unsigned size,
    char old_char,
    char new_char
);

bool
util_match_ssids(
    const char *ssids,
    const char *ssid
);

bool
util_convert_string_to_bool(
    char *value
);

bool
utils_device_is_me(
    const char *mac
);

#endif //__UTILS_H__
