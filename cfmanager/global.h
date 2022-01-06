/****************************************************************************
* *
* * FILENAME:        $RCSfile: global.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2020/12/22
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
#ifndef __GLOBAL_H__
#define __GLOBAL_H__
//=================
//  Includes
//=================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <errno.h>
#include <net/if_arp.h>
#include <glob.h>
#include <dirent.h>

#include <uci.h>
#include <uci_blob.h>
#include <libubus.h>
#include <libubox/list.h>
#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubox/vlist.h>
#include <libubox/utils.h>

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_util.h>


#include "gs_public.h"
//=================
//  Defines
//=================
//#define VERSIONSTR "1.0.0"    //Home version number
#define VERSIONSTR "1.1.0"      //Enterprise version number

#define VERSIONNUM(a,b,c) (((a) << 16) + ((b) << 8) + (c))

#define GSCFMANAGER_VERSION ( calculate_version( VERSIONSTR ) )

#define SAFE_FREE(buf) \
    if ( buf ) { \
        free ( buf ); \
        buf = NULL; \
    }

#ifndef BIT
#define BIT(x) (1Ull << (x))
#endif

#define CFMANAGER_CONFIG_NAME   "cfmanager"
#define MAC_STR_MAX_LEN         17
#define BUF_LEN_8               8
#define BUF_LEN_16              16
#define BUF_LEN_32              32
#define BUF_LEN_64              64
#define BUF_LEN_128             128
#define BUF_LEN_256             256
#define BUF_LEN_512             512
#define COMMAND_LEN             64
#define BIT_MAX                 63

#define VPN_CLIENT_CNT_MAX      128
#define VPN_SERVER_CNT_MAX      128
#define VPN_TOTAL_CNT           (VPN_CLIENT_CNT_MAX + VPN_SERVER_CNT_MAX)

#define VPN_CLIENT_ID_OFFSET    WAN_CNT_MAX
#define VPN_SERVER_ID_OFFSET    (VPN_CLIENT_CNT_MAX + WAN_CNT_MAX)
#define VPN_CLIENT_METRIC_OFFSET    60
#define VPN_SERVER_METRIC_OFFSET    (VPN_SERVER_ID_OFFSET + VPN_CLIENT_METRIC_OFFSET)

#define OPTION_FLAGS_NEED_RELOAD       BIT( OPTION_FLAG_RELOAD )
#define OPTION_FLAGS_CONFIG_CHANGED    BIT( OPTION_FLAG_CHANGED )

//=================
//  Typedefs
//=================
enum {
    WAN0,
    WAN1,

    WAN_CNT_MAX
};

enum
{
    L_EMERG = 0,
    L_ALERT,
    L_CRIT,
    L_ERR,
    L_WARNING,
    L_NOTICE,
    L_INFO,
    L_DEBUG
};

enum {
    CHECK_STATE_MIN,
    CHECK_STATE_COMPL,
    CHECK_STATE_NEED_SET,

    __CHECK_STATE_MAX
};

enum {
    VLTREE_ACTION_UPDATE,
    VLTREE_ACTION_DEL,
    VLTREE_ACTION_ADD,

    __VLTREE_ACTION_MAX
};

enum {
    VPN_CONN_STATUS_DISCONNECTED,
    VPN_CONN_STATUS_CONNECTING,
    VPN_CONN_STATUS_CONNECTED,

    __VPN_CONN_STATUS_MAX
};

struct cm_vltree_extend{
    uint64_t set_compl_option;          //There is no need to reset the configuration
    uint64_t need_set_option;           //The configuration needs to be set
    int action;
    char *section_name;
    struct blob_attr **old_config;      // Old config, used to update hook.
};

struct cm2gs_extend{
    int action;                         //add delete or update
    char *section_name;
};

struct config_section_content {
    char *name;                     //The name of the section
    char *type;                     //The name of the section type
    struct blob_attr *config;       //Content of section
};

typedef int ( *cm_hooker )( struct blob_attr **, int, struct cm_vltree_extend * );
typedef int ( *cm2gs_hooker )( struct blob_attr **, struct cm2gs_extend * );

struct cm_hooker_policy {
    cm_hooker cb;               //Callbacks generated from cfmanager to other configuration files
    cm2gs_hooker cm2gs_cb;      //Current section conversion in cfmanager to grandstream
};

struct cm_vltree_info {
    const char *key;                                //section type name is recommended
    struct vlist_tree *vltree;
    const struct uci_blob_param_list *policy_list;
    int hooker;
};

struct cm_service_init {
    bool first_load_cfg;        //loading configuration file for the first time
    bool boot_start;            //whether start through boot
};

struct device_info {
    char mac[18];
    char product_model[10];
    char part_number[33];
    char version_boot[10];
    char version_firmware[10];
    unsigned char mac_raw[6];
    unsigned short version_mask;
    char work_mode;                          //ap or router
};

//=================
//  Globals
//=================
extern struct uci_context *uci_ctx;
extern struct cm_service_init cm_sve_init;
extern struct device_info device_info;
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
static inline void
uci_add_option(
    struct uci_ptr *ptr,
    const char *name,
    const char *value
)
//=============================================================================
{
    ptr->o = NULL;
    ptr->option = name;
    ptr->value = value;
    uci_set( uci_ctx, ptr );
}

//=============================================================================
static inline void
uci_clean_package(
    const char *package
)
//=============================================================================
{
    int fd;

    unlink( package );
    fd = open( package,
          O_RDWR | O_CREAT,
          S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP );
    if ( fd > 2 )
        close( fd );
}

//=============================================================================

void
cfmanager_set_log_level(
    int level
);

uint32_t
cfmanager_calculate_version(
    char *version
);

#endif //__GLOBAL_H__
