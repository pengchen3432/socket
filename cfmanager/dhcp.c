/****************************************************************************
* *
* * FILENAME:        $RCSfile: dhcp.c,v $
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
#include "dhcp.h"
#include "apply.h"
#include "time.h"
#include "cfparse.h"
#include "check.h"

//=================
//  Defines
//=================
#define DHCP_SCRIPT_NAME SPLICE_STR(CM_FOLDER,/dhcp_apply.sh)

//=================
//  Typedefs
//=================
enum {
    CM_HOOKER_DP_ALL,
    CM_HOOKER_DP_DHCP,
    CM_HOOKER_DP_DNSMASQ,
    CM_HOOKER_DP_HOST,

    __CM_HOOKER_DP_MAX
};


enum {
    DP_VLTREE_ALL,
    DP_VLTREE_DHCP,
    DP_VLTREE_DNSMASQ,
    DP_VLTREE_HOST,

    __DP_VLTREE_MAX
};

//=================
//  Globals
//=================
const struct blobmsg_policy dhcp_policy[__DHCP_MAX] = {
    [DHCP_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
    [DHCP_IGNORE] = { .name = "ignore", .type = BLOBMSG_TYPE_STRING },
    [DHCP_START] = { .name = "start", .type = BLOBMSG_TYPE_STRING },
    [DHCP_LIMIT] = { .name = "limit", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DHCP_OPTION] = { .name = "dhcp_option", .type = BLOBMSG_TYPE_ARRAY },
    [DHCP_LEASETIME] = { .name = "leasetime", .type = BLOBMSG_TYPE_STRING },
    [DHCP_NETWORK_ID] = { .name = "network_id", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DHCPV6] = { .name = "dhcpv6", .type = BLOBMSG_TYPE_STRING },
    [DHCP_RA] = { .name = "ra", .type = BLOBMSG_TYPE_STRING },
    [DHCP_NDP] = { .name = "ndp", .type = BLOBMSG_TYPE_STRING },
    [DHCP_MASTER] = { .name = "master", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy dhcp_host_policy[__DHCP_HOST_MAX] = {
    [DHCP_HOST_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
    [DHCP_HOST_IP] = { .name = "ip", .type = BLOBMSG_TYPE_STRING },
};

struct vlist_tree dhcp_vltree;
struct vlist_tree dhcp_host_vltree;
//=================
//  Locals
//=================

/*
 * Private Functions
 */
static int
cfparse_dhcp_host_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_dhcp_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int
cfparse_dhcp_dnsmasq_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);
/*
 * Private Data
 */

static const struct uci_blob_param_list dhcp_policy_list = {
    .n_params = __DHCP_MAX,
    .params = dhcp_policy,
};

static const struct uci_blob_param_list dhcp_host_policy_list = {
    .n_params = __DHCP_HOST_MAX,
    .params = dhcp_host_policy,
};

const struct blobmsg_policy dhcp_dnsmasq_policy[__DHCP_DNSMASQ_MAX] = {
    [DHCP_DNSMASQ_DOMAINNEEDED] = { .name = "domainneeded", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_BOGUSPRIV] = { .name = "boguspriv", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_FILTERWIN2K] = { .name = "filterwin2k", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_LOCALISE_QUERIES] = { .name = "localise_queries", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_HIJACK_PROTECTION] = { .name = "hijack_protection", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_REBIND_LOCALHOST] = { .name = "rebind_localhost", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_LOCAL] = { .name = "local", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_DOMAIN] = { .name = "domain", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_EXPANDHOSTS] = { .name = "expandhosts", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_NONEGCACHE] = { .name = "nonegcache", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_AUTHORITATIVE] = { .name = "authoritative", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_READETHERS] = { .name = "readethers", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_LEASEFILE] = { .name = "leasefile", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_RESOLVFILE] = { .name = "resolvfile", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_LOCALSERVICE] = { .name = "localservice", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_CONNTRACK] = { .name = "conntrack", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_REBIND_PROTECTION] = { .name = "rebind_protection", .type = BLOBMSG_TYPE_STRING },
    [DHCP_DNSMASQ_IPSET] = { .name = "ipset", .type = BLOBMSG_TYPE_ARRAY }
};

static const struct uci_blob_param_list dhcp_dnsmasq_policy_list = {
    .n_params = __DHCP_DNSMASQ_MAX,
    .params = dhcp_dnsmasq_policy,
};

struct vlist_tree dhcp_dnsmasq_vltree;
static struct blob_buf b;

static const struct cm_hooker_policy cm_hp[__CM_HOOKER_DP_MAX] ={
    [CM_HOOKER_DP_DHCP] = { .cb = cfparse_dhcp_hooker },
    [CM_HOOKER_DP_DNSMASQ] = { .cb = cfparse_dhcp_dnsmasq_hooker },
    [CM_HOOKER_DP_HOST] = { .cb = cfparse_dhcp_host_hooker },
};

static const struct cm_vltree_info dp_vltree_info[__DP_VLTREE_MAX] = {
    [DP_VLTREE_DHCP] = {
        .key = "dhcp",
        .vltree = &dhcp_vltree,
        .policy_list = &dhcp_policy_list,
        .hooker = CM_HOOKER_DP_DHCP
    },

    [DP_VLTREE_DNSMASQ] = {
        .key = "dnsmasq",
        .vltree = &dhcp_dnsmasq_vltree,
        .policy_list = &dhcp_dnsmasq_policy_list,
        .hooker = CM_HOOKER_DP_DNSMASQ
    },

    [DP_VLTREE_HOST] = {
        .key = "host",
        .vltree = &dhcp_host_vltree,
        .policy_list = &dhcp_host_policy_list,
        .hooker = CM_HOOKER_DP_HOST
    },
};

//=================
//  Functions
//=================

//=============================================================================
static struct vlist_tree*
cfparse_dhcp_find_tree(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __DP_VLTREE_MAX; i++ ) {

        if( !dp_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( dp_vltree_info[i].key, section_type ) ) {
            return dp_vltree_info[i].vltree;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static const struct uci_blob_param_list*
cfparse_dhcp_find_blob_param_list(
    const char *section_type
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type ) {
        return NULL;
    }

    for( i = 1; i < __DP_VLTREE_MAX; i++ ) {

        if( !dp_vltree_info[i].key ) {
            continue;
        }

        if( 0 == strcmp( dp_vltree_info[i].key, section_type ) ) {
            return dp_vltree_info[i].policy_list;
        }
    }

    cfmanager_log_message( L_ERR, "Binary tree not found according to %s\n",
        section_type );

    return NULL;
}

//=============================================================================
static void
cfparse_dhcp_node_free(
    struct dhcp_config_parse *dpcfpar
)
//=============================================================================
{
    SAFE_FREE( dpcfpar->cf_section.config );
    SAFE_FREE( dpcfpar );
}

//=============================================================================
static void
cfparse_dhcp_tree_free(
    struct vlist_tree *vltree,
    struct dhcp_config_parse *dpcfpar
)
//=============================================================================
{
    avl_delete( &vltree->avl, &dpcfpar->node.avl );

    cfparse_dhcp_node_free( dpcfpar );
}

//=============================================================================
static int
cfparse_dhcp_host_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    switch ( index ) {
        case DHCP_HOST_MAC:
            break;
        case DHCP_HOST_IP:
            break;
        default:
            break;
    }

    apply_set_reload_flag( CONFIG_TRACK_DHCP );

    return 0;
}

//=============================================================================
static int
cfparse_dhcp_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    switch ( index ) {
        case DHCP_INTERFACE:
            break;
        case DHCP_IGNORE:
            apply_set_reload_flag( CONFIG_TRACK_WIRELESS );
            break;
        case DHCP_START:
            apply_set_reload_flag( CONFIG_TRACK_WIRELESS );
            break;
        case DHCP_LIMIT:
            apply_set_reload_flag( CONFIG_TRACK_WIRELESS );
            break;
        case DHCP_DHCP_OPTION:
            break;
        case DHCP_LEASETIME:
            break;
        case DHCP_NETWORK_ID:
            break;
        case DHCP_DHCPV6:
            break;
        case DHCP_RA:
            break;
        case DHCP_NDP:
            break;
        case DHCP_MASTER:
            break;
        default:
            break;
    }

    apply_set_reload_flag( CONFIG_TRACK_DHCP );

    return 0;
}

//=============================================================================
static int
cfparse_dhcp_dnsmasq_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    switch ( index ) {
        case DHCP_DNSMASQ_DOMAINNEEDED:
            break;
        case DHCP_DNSMASQ_BOGUSPRIV:
            break;
        case DHCP_DNSMASQ_FILTERWIN2K:
            break;
        case DHCP_DNSMASQ_LOCALISE_QUERIES:
            break;
        case DHCP_DNSMASQ_HIJACK_PROTECTION:
            break;
        case DHCP_DNSMASQ_REBIND_LOCALHOST:
            break;
        case DHCP_DNSMASQ_LOCAL:
            break;
        case DHCP_DNSMASQ_DOMAIN:
            break;
        case DHCP_DNSMASQ_EXPANDHOSTS:
            break;
        case DHCP_DNSMASQ_NONEGCACHE:
            break;
        case DHCP_DNSMASQ_AUTHORITATIVE:
            break;
        case DHCP_DNSMASQ_READETHERS:
            break;
        case DHCP_DNSMASQ_LEASEFILE:
            break;
        case DHCP_DNSMASQ_RESOLVFILE:
            break;
        case DHCP_DNSMASQ_LOCALSERVICE:
            break;
        case DHCP_DNSMASQ_CONNTRACK:
            break;
        case DHCP_DNSMASQ_REBIND_PROTECTION:
            break;
        case DHCP_DNSMASQ_IPSET:
            break;
        default:
            break;
    }

    apply_set_reload_flag( CONFIG_TRACK_DHCP );

    return 0;
}

//=============================================================================
void
cfparse_dhcp_update_cfg(
    const struct blobmsg_policy *policy,
    struct dhcp_config_parse *dpcfpar_old,
    struct dhcp_config_parse *dpcfpar_new,
    int policy_size,
    int hooker
)
//=============================================================================
{
    struct blob_attr *tb_old[policy_size];
    struct blob_attr *tb_new[policy_size];
    struct cm_vltree_extend extend;
    int i = 0;

    struct blob_attr *config_old = dpcfpar_old->cf_section.config;
    struct blob_attr *config_new = dpcfpar_new->cf_section.config;

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

    for( i = 0; i < policy_size; i++ ) {
        if( !blob_attr_equal( tb_new[i], tb_old[i] ) ||
            extend.need_set_option & BIT(i) ) {

            if ( cm_hp[hooker].cb ) {
                cm_hp[hooker].cb( tb_new, i, &extend );
            }
        }
    }
}

//=============================================================================
static void
cfparse_dhcp_call_update_func(
    const char *section_type,
    struct dhcp_config_parse *dpcfpar_old,
    struct dhcp_config_parse *dpcfpar_new
)
//=============================================================================
{
    int i = 0;

    if( NULL == section_type || NULL == dpcfpar_old || NULL == dpcfpar_new ) {
        return;
    }

    for( i = 1; i < __DP_VLTREE_MAX; i++ ) {

        if( 0 == strcmp( dp_vltree_info[i].key, section_type ) ) {

            cfparse_dhcp_update_cfg( dp_vltree_info[i].policy_list->params,
                dpcfpar_old,
                dpcfpar_new,
                dp_vltree_info[i].policy_list->n_params,
                dp_vltree_info[i].hooker );
        }
    }
}

//=============================================================================
static void
cfparse_dhcp_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct dhcp_config_parse *dpcfpar_old = NULL;
    struct dhcp_config_parse *dpcfpar_new = NULL;

    if ( node_old ) {
        dpcfpar_old =
            container_of(node_old, struct dhcp_config_parse, node);
    }

    if ( node_new ) {
        dpcfpar_new =
            container_of(node_new, struct dhcp_config_parse, node);
    }

    if ( dpcfpar_old && dpcfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "update dhcp section '%s'\n", dpcfpar_old->cf_section.name );

        if ( blob_attr_equal( dpcfpar_new->cf_section.config, dpcfpar_old->cf_section.config ) ) {
            cfparse_dhcp_node_free( dpcfpar_new );
            return;
        }

        cfparse_dhcp_call_update_func( dpcfpar_old->cf_section.type, dpcfpar_old, dpcfpar_new );

        SAFE_FREE( dpcfpar_old->cf_section.config );
        dpcfpar_old->cf_section.config = blob_memdup( dpcfpar_new->cf_section.config );
        cfparse_dhcp_node_free( dpcfpar_new );
    }
    else if ( dpcfpar_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete network section '%s'\n", dpcfpar_old->cf_section.name );

        vltree = cfparse_dhcp_find_tree( dpcfpar_old->cf_section.type );
        if( NULL == vltree ) {
            cfmanager_log_message( L_DEBUG, "no tree corresponding to %s was found\n",
                dpcfpar_old->cf_section.type );

            return;
        }
        cfparse_dhcp_tree_free( vltree, dpcfpar_old );

        apply_set_reload_flag( CONFIG_TRACK_DHCP );
    }
    else if ( dpcfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "New dhcp section '%s'\n", dpcfpar_new->cf_section.name );

        apply_set_reload_flag( CONFIG_TRACK_DHCP );
    }
}

//=============================================================================
static void
cfparse_dhcp_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct dhcp_config_parse *dpcfpar = NULL;
    struct vlist_tree *vltree = NULL;
    char *name;
    char *type;

    vltree = cfparse_dhcp_find_tree( section_type );
    if( NULL == vltree ) {
        cfmanager_log_message( L_DEBUG, "No corresponding binary tree was found according to %s\n",
            section_type );

        return;
    }

    dpcfpar = calloc_a( sizeof( *dpcfpar ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type ) +1 );

    if ( !dpcfpar ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc dhcp_config_parse '%s'\n", section_name );

        return;
    }

    dpcfpar->cf_section.name = strcpy( name, section_name );
    dpcfpar->cf_section.type = strcpy( type, section_type );
    dpcfpar->cf_section.config = blob_memdup( data );

    vlist_add( vltree, &dpcfpar->node, dpcfpar->cf_section.name );
}

//=============================================================================
static void
cfparse_dhcp_uci_to_blob(
    struct uci_section *s
)
//=============================================================================
{
    const struct uci_blob_param_list* uci_blob_list = NULL;

    blob_buf_init( &b, 0 );

    uci_blob_list = cfparse_dhcp_find_blob_param_list( s->type );
    if( NULL == uci_blob_list ) {
        cfmanager_log_message( L_DEBUG, "No corresponding uci_blob_param_list was found according to %s\n",
            s->type );

        return;
    }
    uci_to_blob( &b, s, uci_blob_list );

    cfparse_dhcp_add_blob_to_tree( b.head, s->e.name, s->type );

    blob_buf_free( &b );
}

//=============================================================================
int
cfparse_load_dhcp(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct vlist_tree *vltree = NULL;
    struct uci_package *package = NULL;
    int i = 0;

    check_set_defvalue( CHECK_DHCP );

    package = cfparse_init_package( "dhcp" );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load dhcp package failed\n" );
        return -1;
    }

    for( i = 1; i < __DP_VLTREE_MAX; i++ ) {

        vltree = dp_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_update( vltree );
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        cfparse_dhcp_uci_to_blob( s );
    }

    for( i = 1; i < __DP_VLTREE_MAX; i++ ) {

        vltree = dp_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush( vltree );
    }

    if( apply_get_reload_flag( CONFIG_TRACK_DHCP ) ) {
        apply_add( "dhcp" );
        apply_flush_reload_flag( CONFIG_TRACK_DHCP );
        apply_timer_start();
    }

    if( apply_get_reload_flag( CONFIG_TRACK_WIRELESS ) ) {
        apply_add( "wireless" );
        apply_flush_reload_flag( CONFIG_TRACK_WIRELESS );
        apply_timer_start();
    }

    return 0;
}

//=============================================================================
void
cfparse_dhcp_init(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __DP_VLTREE_MAX; i++ ) {

        vltree = dp_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_init( vltree, avl_strcmp, cfparse_dhcp_vltree_update );
        vltree->keep_old = true;
        vltree->no_delete = true;
    }
}

//=============================================================================
void
cfparse_dhcp_deinit(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __DP_VLTREE_MAX; i++ ) {

        vltree = dp_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_flush_all( vltree );
    }
}

