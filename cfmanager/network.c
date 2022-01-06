/****************************************************************************
* *
* * FILENAME:        $RCSfile: network.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/19
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
#include <libubox/vlist.h>
#include <uci.h>
#include "global.h"
#include "track.h"
#include "initd.h"
#include "utils.h"
#include "network.h"
#include "time.h"
#include "apply.h"
#include "sgrequest.h"
#include "cfmanager.h"
#include "cfparse.h"
#include "check.h"

//=================
//  Defines
//=================
#define NETWORK_SCRIPT_NAME SPLICE_STR(CM_FOLDER,/network_apply.sh)
//=================
//  Typedefs
//=================
enum {
    NETWORK_GLOBAL_ULA_PREFIX,

    __NETWORK_GLOBAL_MAX
};

enum {
    NETWORK_SWITCH_NAME,
    NETWORK_SWITCH_RESET,
    NETWORK_SWITCH_ENABLE_VLAN,

    __NETWORK_SWITCH_MAX
};

enum {
    NETWORK_SWITCH_VLAN_DEVICE,
    NETWORK_SWITCH_VLAN_PORTS,
    NETWORK_SWITCH_VLAN_VLAN,
    NETWORK_SWITCH_VLAN_VID,

    __NETWORK_SWITCH_VLAN_MAX
};

enum {
    NETWORK_GS_SWITCH_PORT_PVID,
    NETWORK_GS_SWITCH_PORT_VLAN,

    __NETWORK_GS_SWITCH_PORT_MAX
};

enum {
    NETWORK_ROUTE_INTERFACE,
    NETWORK_ROUTE_TARGET,
    NETWORK_ROUTE_NETMASK,
    NETWORK_ROUTE_GATEWAY,
    NETWORK_ROUTE_METRIC,

    __NETWORK_ROUTE_MAX
};

enum {
    NETWORK_ROUTE6_INTERFACE,
    NETWORK_ROUTE6_TARGET,
    NETWORK_ROUTE6_GATEWAY,
    NETWORK_ROUTE6_METRIC,

    __NETWORK_ROUTE6_MAX
};

enum {
    NOT_NEED_UPDATE,
    NEED_UPDATE,
    ADD_SECTION,
    DELETE_SECTION,
};

enum {
    __NW_VLTREE_ALL,
    NW_VLTREE_INTERFACE,
    NW_VLTREE_SWITCH,
    NW_VLTREE_SWITCH_VLAN,
    NW_VLTREE_GS_SWITCH_PORT,
    NW_VLTREE_GLOBAL,
    NW_VLTREE_DEVICE,
    NW_VLTREE_ROUTE,
    NW_VLTREE_ALIAS,
    NW_VLTREE_ROUTE6,

    __NW_VLTREE_MAX
};

enum {
    CM_HOOKER_NW_INTERFACE,
    CM_HOOKER_NW_SWITCH,
    CM_HOOKER_NW_SWITCH_VLAN,
    CM_HOOKER_NW_GS_SWITCH_PORT,
    CM_HOOKER_NW_GLOBAL,
    CM_HOOKER_NW_DEVICE,
    CM_HOOKER_NW_ROUTE,
    CM_HOOKER_NW_ALIAS,
    CM_HOOKER_NW_ROUTE6,

    __CM_HOOKER_NW_MAX
};

//=================
//  Globals
//=================
extern const struct blobmsg_policy vlan_policy[__VLAN_MAX];
struct vlist_tree network_interface_vltree;
struct vlist_tree network_switch_vltree;
struct vlist_tree network_switch_vlan_vltree;
struct vlist_tree network_gs_switch_port_vltree;
struct vlist_tree network_global_vltree;
struct vlist_tree network_device_vltree;
struct vlist_tree network_route_vltree;
struct vlist_tree network_alias_vltree;
struct vlist_tree network_route6_vltree;

const struct blobmsg_policy network_interface_policy[__NETWORK_INTERFACE_MAX] = {
    [NETWORK_INTERFACE_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_FORCE_LINK] = { .name = "force_link", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_EMPTY] = { .name = "empty", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_PROTO] = { .name = "proto", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_MTU] = { .name = "mtu", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_ARRAY },
    [NETWORK_INTERFACE_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_REQOPS] = { .name = "reqops", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_DNS] = { .name = "dns", .type = BLOBMSG_TYPE_ARRAY },
    [NETWORK_INTERFACE_GATEWAY] = { .name = "gateway", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_METRIC] = { .name = "metric", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_MULTICAST_QUERIER] = { .name = "multicast_querier", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_IEEE1905MANAGED] = { .name = "ieee1905managed", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_USERNAME] = { .name = "username", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_SERVER] = { .name = "server", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_PEERDNS] = { .name = "peerdns", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_DELEGATE] = { .name = "delegate", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_MPPE] = { .name = "mppe", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_CHECKUP_INTERVAL] = { .name = "checkup_interval", .type = BLOBMSG_TYPE_STRING },
    //[NETWORK_INTERFACE_GS_BLOCK_RESTART] = { .name = "gs_block_restart", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_GS_IP_TYPE] = { .name = "gs_ip_type", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_GS_LOCAL_IP] = { .name = "gs_local_ip", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_GS_REMOTE_IP] = { .name = "gs_remote_ip", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_GS_NETMASK] = { .name = "gs_netmask", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_GS_STATIC_DNS] = { .name = "gs_static_dns", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_GS_DNS1] = { .name = "gs_dns1", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_INTERFACE_GS_DNS2] = { .name = "gs_dns2", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy network_alias_policy[__NETWORK_ALIAS_MAX] = {
    [NETWORK_ALIAS_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ALIAS_PROTO] = { .name = "proto", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ALIAS_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ALIAS_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ALIAS_GATEWAY] = { .name = "gateway", .type = BLOBMSG_TYPE_STRING },
};

//=================
//  Locals
//=================

/*
 * Private Functions
 */
static int
cfparse_network_switch_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_network_switch_vlan_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_network_gs_switch_port_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_network_interface_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_network_global_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_network_device_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_network_route_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_network_alias_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_network_route6_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

/*
 * Private Data
 */
static struct blob_buf b;

static const struct uci_blob_param_list network_interface_list = {
    .n_params = __NETWORK_INTERFACE_MAX,
    .params = network_interface_policy,
};

static const struct blobmsg_policy network_global_policy[__NETWORK_GLOBAL_MAX] = {
    [NETWORK_GLOBAL_ULA_PREFIX] = { .name = "ula_prefix", .type = BLOBMSG_TYPE_STRING },
};

static const struct uci_blob_param_list network_global_list = {
    .n_params = __NETWORK_GLOBAL_MAX,
    .params = network_global_policy,
};

static const struct blobmsg_policy network_switch_policy[__NETWORK_SWITCH_MAX] = {
    [NETWORK_SWITCH_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_SWITCH_RESET] = { .name = "reset", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_SWITCH_ENABLE_VLAN] = { .name = "enable_vlan", .type = BLOBMSG_TYPE_STRING },
};

static const struct uci_blob_param_list network_switch_list = {
    .n_params = __NETWORK_SWITCH_MAX,
    .params = network_switch_policy,
};

static const struct blobmsg_policy network_switch_vlan_policy[__NETWORK_SWITCH_VLAN_MAX] = {
    [NETWORK_SWITCH_VLAN_DEVICE] = { .name = "device", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_SWITCH_VLAN_PORTS] = { .name = "ports", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_SWITCH_VLAN_VLAN] = { .name = "vlan", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_SWITCH_VLAN_VID] = { .name = "vid", .type = BLOBMSG_TYPE_STRING },
};

static const struct uci_blob_param_list network_switch_vlan_list = {
    .n_params = __NETWORK_SWITCH_VLAN_MAX,
    .params = network_switch_vlan_policy,
};

static const struct blobmsg_policy network_gs_switch_port_policy[__NETWORK_GS_SWITCH_PORT_MAX] = {
    [NETWORK_GS_SWITCH_PORT_PVID] = { .name = "pvid", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_GS_SWITCH_PORT_VLAN] = { .name = "vlan", .type = BLOBMSG_TYPE_STRING },
};

static const struct uci_blob_param_list network_gs_switch_port_list = {
    .n_params = __NETWORK_GS_SWITCH_PORT_MAX,
    .params = network_gs_switch_port_policy,
};

static const struct blobmsg_policy network_route_policy[__NETWORK_ROUTE_MAX] = {
    [NETWORK_ROUTE_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ROUTE_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ROUTE_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ROUTE_GATEWAY] = { .name = "gateway", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ROUTE_METRIC] = { .name = "metric", .type = BLOBMSG_TYPE_STRING },
};

static const struct blobmsg_policy network_route6_policy[__NETWORK_ROUTE6_MAX] = {
    [NETWORK_ROUTE6_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ROUTE6_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ROUTE6_GATEWAY] = { .name = "gateway", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_ROUTE6_METRIC] = { .name = "metric", .type = BLOBMSG_TYPE_STRING },
};

static const struct uci_blob_param_list network_route_list = {
    .n_params = __NETWORK_ROUTE_MAX,
    .params = network_route_policy,
};

static const struct uci_blob_param_list network_route6_list = {
    .n_params = __NETWORK_ROUTE6_MAX,
    .params = network_route6_policy,
};

const struct blobmsg_policy network_device_policy[__NETWORK_DEVICE_MAX] = {
    [NETWORK_DEVICE_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [NETWORK_DEVICE_MACADDR] = { .name = "macaddr", .type = BLOBMSG_TYPE_STRING },
};

static const struct uci_blob_param_list network_device_list = {
    .n_params = __NETWORK_DEVICE_MAX,
    .params = network_device_policy,
};

static const struct cm_hooker_policy cm_hp[__CM_HOOKER_NW_MAX] ={
    [CM_HOOKER_NW_SWITCH] = { .cb = cfparse_network_switch_hooker },
    [CM_HOOKER_NW_SWITCH_VLAN] = { .cb = cfparse_network_switch_vlan_hooker },
    [CM_HOOKER_NW_GS_SWITCH_PORT] = { .cb = cfparse_network_gs_switch_port_hooker },
    [CM_HOOKER_NW_INTERFACE] = { .cb = cfparse_network_interface_hooker },
    [CM_HOOKER_NW_GLOBAL] = { .cb = cfparse_network_global_hooker },
    [CM_HOOKER_NW_DEVICE] = { .cb = cfparse_network_device_hooker },
    [CM_HOOKER_NW_ROUTE] = { .cb = cfparse_network_route_hooker },
    [CM_HOOKER_NW_ALIAS] = { .cb = cfparse_network_alias_hooker },
    [CM_HOOKER_NW_ROUTE6] = { .cb = cfparse_network_route6_hooker },
};

static const struct uci_blob_param_list network_alias_list = {
    .n_params = __NETWORK_ALIAS_MAX,
    .params = network_alias_policy,
};


static const struct cm_vltree_info nw_vltree_info[__NW_VLTREE_MAX] = {
    [NW_VLTREE_INTERFACE] = {
        .key = "interface",
        .vltree = &network_interface_vltree,
        .policy_list = &network_interface_list,
        .hooker = CM_HOOKER_NW_INTERFACE
    },

    [NW_VLTREE_SWITCH] = {
        .key = "switch",
        .vltree = &network_switch_vltree,
        .policy_list = &network_switch_list,
        .hooker = CM_HOOKER_NW_SWITCH
    },

    [NW_VLTREE_SWITCH_VLAN] = {
        .key = "switch_vlan",
        .vltree = &network_switch_vlan_vltree,
        .policy_list = &network_switch_vlan_list,
        .hooker = CM_HOOKER_NW_SWITCH_VLAN
    },

    [NW_VLTREE_GS_SWITCH_PORT] = {
        .key = "gs_switch_port",
        .vltree = &network_gs_switch_port_vltree,
        .policy_list = &network_gs_switch_port_list,
        .hooker = CM_HOOKER_NW_GS_SWITCH_PORT
    },

    [NW_VLTREE_GLOBAL] = {
        .key = "globals",
        .vltree = &network_global_vltree,
        .policy_list = &network_global_list,
        .hooker = CM_HOOKER_NW_GLOBAL
    },

    [NW_VLTREE_DEVICE] = {
        .key = "device",
        .vltree = &network_device_vltree,
        .policy_list = &network_device_list,
        .hooker = CM_HOOKER_NW_DEVICE
    },

    [NW_VLTREE_ROUTE] = {
        .key = "route",
        .vltree = &network_route_vltree,
        .policy_list = &network_route_list,
        .hooker = CM_HOOKER_NW_ROUTE
    },

    [NW_VLTREE_ALIAS] = {
        .key = "alias",
        .vltree = &network_alias_vltree,
        .policy_list = &network_alias_list,
        .hooker = CM_HOOKER_NW_ALIAS
    },

    [NW_VLTREE_ROUTE6] = {
        .key = "route6",
        .vltree = &network_route6_vltree,
        .policy_list = &network_route6_list,
        .hooker = CM_HOOKER_NW_ROUTE6
    },
};

//=================
//  Functions
//=================

//=============================================================================
static struct vlist_tree*
cfparse_network_find_tree(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __NW_VLTREE_MAX; i++ ) {

        if( !nw_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( nw_vltree_info[i].key, section_type ) ) {
            return nw_vltree_info[i].vltree;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static const struct uci_blob_param_list*
cfparse_network_find_blob_param_list(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __NW_VLTREE_MAX; i++ ) {

        if( !nw_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( nw_vltree_info[i].key, section_type ) ) {
            return nw_vltree_info[i].policy_list;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static void
cfparse_network_node_free(
    struct network_config_parse *nwcfpar
)
//=============================================================================
{
    SAFE_FREE( nwcfpar->cf_section.config );
    SAFE_FREE( nwcfpar );
}

//=============================================================================
static void
cfparse_network_tree_free(
    struct vlist_tree *vltree,
    struct network_config_parse *nwcfpar
)
//=============================================================================
{
    avl_delete( &vltree->avl, &nwcfpar->node.avl );

    cfparse_network_node_free( nwcfpar );
}

//=============================================================================
static int
cfparse_network_switch_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case NETWORK_SWITCH_NAME:
            break;
        case NETWORK_SWITCH_RESET:
            break;
        case NETWORK_SWITCH_ENABLE_VLAN:
            break;
        default:
            break;
    }

    //rc |= OPTION_FLAGS_CONFIG_CHANGED;
    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
static int
cfparse_network_switch_vlan_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case NETWORK_SWITCH_VLAN_DEVICE:
            break;
        case NETWORK_SWITCH_VLAN_PORTS:
            break;
        case NETWORK_SWITCH_VLAN_VLAN:
            break;
        case NETWORK_SWITCH_VLAN_VID:
            break;
        default:
            break;
    }

    //rc |= OPTION_FLAGS_CONFIG_CHANGED;
    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
static int
cfparse_network_gs_switch_port_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case NETWORK_GS_SWITCH_PORT_PVID:
            break;
        case NETWORK_GS_SWITCH_PORT_VLAN:
            break;
        default:
            break;
    }

    //rc |= OPTION_FLAGS_CONFIG_CHANGED;
    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
static int
cfparse_network_interface_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case NETWORK_INTERFACE_IFNAME:
            //Interface changes need to update the interface state of controller
            if( 0 == strcmp( "lan0_zone0", extend->section_name ) ) {
                apply_set_reload_flag( CONFIG_TRACK_CONTROLLER );
            }
            break;
        case NETWORK_INTERFACE_FORCE_LINK:
            break;
        case NETWORK_INTERFACE_TYPE:
            break;
        case NETWORK_INTERFACE_EMPTY:
            break;
        case NETWORK_INTERFACE_PROTO:
            break;
        case NETWORK_INTERFACE_MTU:
            break;
        case NETWORK_INTERFACE_IPADDR:
            //IP change needs to make the client re associated
            if( 0 == strcmp( "lan0_zone0", extend->section_name ) ) {
                apply_set_reload_flag( CONFIG_TRACK_WIRELESS );
                apply_set_reload_flag( CONFIG_TRACK_DHCP );
                /* If there is an AP taking over under LAN port,
                 * you need to restart the controller to go online again
                 */
                apply_set_reload_flag( CONFIG_TRACK_CONTROLLER );
            }
            break;
        case NETWORK_INTERFACE_NETMASK:
            if( 0 == strcmp( "lan0_zone0", extend->section_name ) ) {
                apply_set_reload_flag( CONFIG_TRACK_WIRELESS );
                apply_set_reload_flag( CONFIG_TRACK_DHCP );
                apply_set_reload_flag( CONFIG_TRACK_CONTROLLER );
            }
            break;
        case NETWORK_INTERFACE_REQOPS:
            break;
        case NETWORK_INTERFACE_DNS:
            break;
        case NETWORK_INTERFACE_GATEWAY:
            break;
        case NETWORK_INTERFACE_METRIC:
            break;
        case NETWORK_INTERFACE_MULTICAST_QUERIER:
            break;
        case NETWORK_INTERFACE_IEEE1905MANAGED:
            break;
        case NETWORK_INTERFACE_USERNAME:
            break;
        case NETWORK_INTERFACE_PASSWORD:
            break;
        case NETWORK_INTERFACE_SERVER:
            break;
        case NETWORK_INTERFACE_INTERFACE:
            break;
        case NETWORK_INTERFACE_CHECKUP_INTERVAL:
            break;
//        case NETWORK_INTERFACE_GS_BLOCK_RESTART:
//            break;
        case NETWORK_INTERFACE_GS_IP_TYPE:
            break;
        case NETWORK_INTERFACE_GS_LOCAL_IP:
            break;
        case NETWORK_INTERFACE_GS_REMOTE_IP:
            break;
        case NETWORK_INTERFACE_GS_NETMASK:
            break;
        case NETWORK_INTERFACE_GS_STATIC_DNS:
            break;
        case NETWORK_INTERFACE_GS_DNS1:
            break;
        case NETWORK_INTERFACE_GS_DNS2:
            break;
        default:
            break;
    }

    //rc |= OPTION_FLAGS_CONFIG_CHANGED;
    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
static int
cfparse_network_global_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case NETWORK_GLOBAL_ULA_PREFIX:
            break;
        default:
            break;
    }

    //rc |= OPTION_FLAGS_CONFIG_CHANGED;
    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
static int
cfparse_network_device_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case NETWORK_DEVICE_NAME:
            break;
        case NETWORK_DEVICE_MACADDR:
            break;
        default:
            break;
    }

    //rc |= OPTION_FLAGS_CONFIG_CHANGED;
    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
static int
cfparse_network_route_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case NETWORK_ROUTE_INTERFACE:
            break;
        case NETWORK_ROUTE_TARGET:
            break;
        case NETWORK_ROUTE_NETMASK:
            break;
        case NETWORK_ROUTE_GATEWAY:
            break;
        case NETWORK_ROUTE_METRIC:
            break;

        default:
            break;
    }

    //rc |= OPTION_FLAGS_CONFIG_CHANGED;
    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
static int
cfparse_network_alias_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case NETWORK_ALIAS_INTERFACE:
            break;
        case NETWORK_ALIAS_PROTO:
            break;
        case NETWORK_ALIAS_IPADDR:
            break;
        case NETWORK_ALIAS_NETMASK:
            break;
        case NETWORK_ALIAS_GATEWAY:
            break;
        default:
            break;
    }

    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}

//=============================================================================
static int
cfparse_network_route6_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case NETWORK_ROUTE6_INTERFACE:
            break;
        case NETWORK_ROUTE6_TARGET:
            break;
        case NETWORK_ROUTE6_GATEWAY:
            break;
        case NETWORK_ROUTE6_METRIC:
            break;
        default:
            break;
    }

    rc |= OPTION_FLAGS_NEED_RELOAD;

    return rc;
}


//=============================================================================
void
cfparse_network_update_cfg(
    const struct blobmsg_policy *policy,
    struct network_config_parse *nwcfpar_old,
    struct network_config_parse *nwcfpar_new,
    int policy_size,
    int hooker
)
//=============================================================================
{
    struct blob_attr *tb_old[policy_size];
    struct blob_attr *tb_new[policy_size];
    struct cm_vltree_extend extend;
    int i = 0;
    int option = 0;

    struct blob_attr *config_old = nwcfpar_old->cf_section.config;
    struct blob_attr *config_new = nwcfpar_new->cf_section.config;

    blobmsg_parse( policy,
            policy_size,
            tb_old,
            blob_data( config_old ),
            blob_len( config_old ) );

    blobmsg_parse( policy,
            policy_size,
            tb_new,
            blob_data( config_new ),
            blob_len( config_new ) );

    memset( &extend, 0, sizeof( extend ) );

    extend.section_name = nwcfpar_old->cf_section.name;

    for( i = 0; i < policy_size; i++ ) {
        if( !blob_attr_equal( tb_new[i], tb_old[i] ) ) {

            if ( cm_hp[hooker].cb ) {
                option |= cm_hp[hooker].cb( tb_new, i, &extend );
            }
        }
    }

    if ( option & OPTION_FLAGS_NEED_RELOAD ) {
        apply_set_reload_flag( CONFIG_TRACK_NETWORK );
    }
}

//=============================================================================
static void
cfparse_network_call_update_func(
    const char *section_type,
    struct network_config_parse *nwcfpar_old,
    struct network_config_parse *nwcfpar_new
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type || NULL == nwcfpar_old || NULL == nwcfpar_new ) {
        return;
    }

    for( i = 1; i < __NW_VLTREE_MAX; i++ ) {

        if( 0 == strcmp( nw_vltree_info[i].key, section_type ) ) {

            cfparse_network_update_cfg( nw_vltree_info[i].policy_list->params,
                nwcfpar_old,
                nwcfpar_new,
                nw_vltree_info[i].policy_list->n_params,
                nw_vltree_info[i].hooker );
        }
    }
}

//=============================================================================
static void
cfparse_network_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct network_config_parse *nwcfpar_old = NULL;
    struct network_config_parse *nwcfpar_new = NULL;

    if ( node_old ) {
        nwcfpar_old =
            container_of(node_old, struct network_config_parse, node);
    }

    if ( node_new ) {
        nwcfpar_new =
            container_of(node_new, struct network_config_parse, node);
    }

    if ( nwcfpar_old && nwcfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "update network section '%s'\n", nwcfpar_old->cf_section.name );

        if ( blob_attr_equal( nwcfpar_new->cf_section.config, nwcfpar_old->cf_section.config ) ) {
            cfparse_network_node_free( nwcfpar_new );
            return;
        }

        cfparse_network_call_update_func( nwcfpar_old->cf_section.type, nwcfpar_old, nwcfpar_new );

        SAFE_FREE( nwcfpar_old->cf_section.config );
        nwcfpar_old->cf_section.config = blob_memdup( nwcfpar_new->cf_section.config );
        if( 0 == strcmp( nwcfpar_old->cf_section.type, "interface" ) ) {
            nwcfpar_old->proto = nwcfpar_new->proto;
            strncpy( nwcfpar_old->ipv4, nwcfpar_new->ipv4, IP4ADDR_MAX_LEN );
        }

        cfparse_network_node_free( nwcfpar_new );
    }
    else if ( nwcfpar_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete network section '%s'\n", nwcfpar_old->cf_section.name );

        vltree = cfparse_network_find_tree( nwcfpar_old->cf_section.type );
        if( NULL == vltree ) {
            cfmanager_log_message( L_DEBUG, "no tree corresponding to %s was found\n",
                nwcfpar_old->cf_section.type );

            return;
        }
        cfparse_network_tree_free( vltree, nwcfpar_old );

        apply_set_reload_flag( CONFIG_TRACK_NETWORK );
    }
    else if ( nwcfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "New network section '%s'\n", nwcfpar_new->cf_section.name );

        apply_set_reload_flag( CONFIG_TRACK_NETWORK );
    }
}

//=============================================================================
static void
cfparse_network_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct blob_attr *tb[__NETWORK_INTERFACE_MAX];
    struct blob_attr *alias_tb[__NETWORK_ALIAS_MAX];
    struct blob_attr *cur = NULL;
    struct network_config_parse *nwcfpar = NULL;
    struct vlist_tree *vltree = NULL;
    char *name;
    char *type;
    int i = 0;

    vltree = cfparse_network_find_tree( section_type );
    if( NULL == vltree ) {
        cfmanager_log_message( L_DEBUG, "No corresponding binary tree was found according to %s\n",
            section_type );

        return;
    }

    nwcfpar = calloc_a( sizeof( *nwcfpar ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type ) +1 );

    if ( !nwcfpar ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc network_config_parse '%s'\n", section_name );

        return;
    }

    if( 0 == strcmp( section_type, "interface" ) ) {
        blobmsg_parse( network_interface_policy,
            __NETWORK_INTERFACE_MAX,
            tb,
            blob_data( data ),
            blob_len( data ) );

        if( tb[NETWORK_INTERFACE_PROTO] ) {
            nwcfpar->proto = sgreq_convert_cm_proto(
                blobmsg_get_string( tb[NETWORK_INTERFACE_PROTO] ) );
        }

        if( tb[NETWORK_INTERFACE_IPADDR] ) {
            blobmsg_for_each_attr( cur, tb[NETWORK_INTERFACE_IPADDR], i ) {
                if ( BLOBMSG_TYPE_STRING != blobmsg_type( cur ) ) {
                    continue;
                }

                strncpy( nwcfpar->ipv4, blobmsg_get_string( cur ), IP4ADDR_MAX_LEN );
                //WAN port only supports one IP at present
                break;
            }
        }
    }
    else if ( 0 == strcmp( "alias", section_type ) ) {
        blobmsg_parse( network_alias_policy,
            __NETWORK_ALIAS_MAX,
            alias_tb,
            blob_data( data ),
            blob_len( data ) );

        if( alias_tb[NETWORK_ALIAS_IPADDR] ) {
            strncpy( nwcfpar->ipv4, blobmsg_get_string( alias_tb[NETWORK_ALIAS_IPADDR] ),
                IP4ADDR_MAX_LEN );
        }
    }

    nwcfpar->cf_section.name = strcpy( name, section_name );
    nwcfpar->cf_section.type = strcpy( type, section_type );
    nwcfpar->cf_section.config = blob_memdup( data );

    vlist_add( vltree, &nwcfpar->node, nwcfpar->cf_section.name );
}

//=============================================================================
static void
cfparse_network_uci_to_blob(
    struct uci_section *s
)
//=============================================================================
{
    const struct uci_blob_param_list* uci_blob_list = NULL;

    blob_buf_init( &b, 0 );

    uci_blob_list = cfparse_network_find_blob_param_list( s->type );
    if( NULL == uci_blob_list ) {
        cfmanager_log_message( L_DEBUG, "No corresponding uci_blob_param_list was found according to %s\n",
            s->type );

        return;
    }
    uci_to_blob( &b, s, uci_blob_list );

    cfparse_network_add_blob_to_tree( b.head, s->e.name, s->type );

    blob_buf_free( &b );
}

//=============================================================================
int
cfparse_load_network(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct vlist_tree *vltree = NULL;
    struct uci_package *package = NULL;
    int i = 0;

    check_set_defvalue( CHECK_NETWORK );

    package = cfparse_init_package( "network" );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load network package failed\n" );
        return -1;
    }

    for( i = 1; i < __NW_VLTREE_MAX; i++ ) {

        vltree = nw_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_update( vltree );
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        cfparse_network_uci_to_blob( s );
    }

    for( i = 1; i < __NW_VLTREE_MAX; i++ ) {

        vltree = nw_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush( vltree );
    }

    apply_handle_all_reload_flag();

    return 0;
}

//=============================================================================
void
cfparse_network_init(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __NW_VLTREE_MAX; i++ ) {

        vltree = nw_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_init( vltree, avl_strcmp, cfparse_network_vltree_update );
        vltree->keep_old = true;
        vltree->no_delete = true;
    }
}

//=============================================================================
void
cfparse_network_deinit(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __NW_VLTREE_MAX; i++ ) {

        vltree = nw_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush_all( vltree );
    }
}

