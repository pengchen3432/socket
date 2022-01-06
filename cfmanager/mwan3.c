/****************************************************************************
* *
* * FILENAME:        $RCSfile: mwan3.c,v $
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
#include <uci.h>
#include "global.h"
#include "utils.h"
#include "firewall.h"
#include "cfparse.h"
#include "config.h"
#include "ubus.h"
#include "mwan3.h"
#include "time.h"
#include "track.h"
#include "apply.h"
#include "sgrequest.h"

//=================
//  Defines
//=================
#define MWAN3_CONFIG_NAME "mwan3"
//#define MWAN3_CONFIG_PATH "/etc/config/mwan3"
#define MWAN3_CONFIG_PATH SPLICE_STR(CM_CONFIG_PATH,/mwan3)

#define INTF_WAN0_EN 0x1
#define INTF_WAN1_EN 0X2
#define INTF_VPN0_EN 0x4    // Means config vpn from wan0 setting.
#define INTF_VPN1_EN 0x8    // Means config vpn from wan1 setting.
#define INTF_VPNX0_EN 0x10  // Means config vpn on wan0 from vpn client setting.
#define INTF_VPNX1_EN 0x20  // Means config vpn on wan1 from vpn client setting.

// Single WAN
#define INTF_WAN0_VPN0_EN (INTF_WAN0_EN | INTF_VPN0_EN)
#define INTF_WAN0_VPNX0_EN (INTF_WAN0_EN | INTF_VPNX0_EN)

// Dual WANs
#define INTF_DUAL_WAN_EN (INTF_WAN0_EN | INTF_WAN1_EN)

#define INTF_DUAL_WAN_VPN0_EN (INTF_DUAL_WAN_EN | INTF_VPN0_EN)
#define INTF_DUAL_WAN_VPN1_EN (INTF_DUAL_WAN_EN | INTF_VPN1_EN)

#define INTF_DUAL_WAN_VPN0_VPNX0_EN (INTF_DUAL_WAN_VPN0_EN | INTF_VPNX0_EN)
#define INTF_DUAL_WAN_VPN1_VPNX0_EN (INTF_DUAL_WAN_VPN1_EN | INTF_VPNX0_EN)
#define INTF_DUAL_WAN_VPN0_VPNX1_EN (INTF_DUAL_WAN_VPN0_EN | INTF_VPNX1_EN)
#define INTF_DUAL_WAN_VPN1_VPNX1_EN (INTF_DUAL_WAN_VPN1_EN | INTF_VPNX1_EN)

#define INTF_DUAL_WAN_VPN0_VPN1_EN (INTF_DUAL_WAN_VPN0_EN | INTF_VPN1_EN)

#define INTF_DUAL_WAN_VPNX0_EN (INTF_DUAL_WAN_EN | INTF_VPNX0_EN)
#define INTF_DUAL_WAN_VPNX1_EN (INTF_DUAL_WAN_EN | INTF_VPNX1_EN)
#define INTF_DUAL_WAN_VPNX0_VPNX1_EN (INTF_DUAL_WAN_VPNX0_EN | INTF_VPNX1_EN)

#define WAN0_DEFAULT_IFNAME "wan0"
#define WAN1_DEFAULT_IFNAME "wan1"
#define VPN0_DEFAULT_IFNAME "vpn0"
#define VPN1_DEFAULT_IFNAME "vpn1"

#define WAN0_DEFAULT_MEMBER "m_wan0_auto"
#define WAN1_DEFAULT_MEMBER "m_wan1_auto"
#define VPN0_DEFAULT_MEMBER "m_vpn0_auto"
#define VPN1_DEFAULT_MEMBER "m_vpn1_auto"


#define WAN0_DEFAULT_POLICY "wan0_auto"
#define WAN1_DEFAULT_POLICY "wan1_auto"
#define VPN0_DEFAULT_POLICY "vpn0_auto"
#define VPN1_DEFAULT_POLICY "vpn1_auto"

enum VPN_SENCE {
    VPN_SENCE_INVALID,
    VPN_SENCE_SINGLE_VPN_ON_SINGLE_WAN,
    VPN_SENCE_SINGLE_VPN_ON_DUAL_WAN,
    VPN_SENCE_DUAL_VPN_ON_DUAL_WAN,
    VPN_SENCE_OTHERS,

    __VPN_SENCE_MAX
};

//=================
//  Typedefs
//=================
struct mwan3_cfg {
    int id; // WAN or VPN zone id
    short wan_type;
    short wan_index;
    struct static_dns dns;
    char *weight;
    char *tunnel_default;
};

struct vpn_split_cfg {
    int vpn_id; // VPN zone id
    char vpn_iface[BUF_LEN_32];
    int vpn_sence;
    const char *mode;
    struct uci_option *service_addr;
    struct uci_option *dev;
};

struct balance_rule_cfg {
    char *policy_nm;
    char **member_list;
    int member_cnts;
    char *src;
    char *src_ip;
    int is_default;
    int vpn_sence;
    const char *vpn_iface;
};

//=================
//  Globals
//=================

//=================
//  Locals
//=================
static struct uci_context *ctx;

/*
 * Private Functions
 */

/*
 * Private Data
 */

//=============================================================================
static inline void
mwan3_add_list(
    struct uci_ptr *ptr,
    struct uci_option *o,
    const char *option,
    const char *value
)
//=============================================================================
{
    ptr->o = o;
    ptr->option = option;
    ptr->value = value;
    uci_add_list( ctx, ptr );
}

//=============================================================================
static inline int
mwan3_add_rule(
    struct uci_package *p,
    struct mwan3_rule *rule
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    int ret = 0;

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section( ctx, p, "rule", &ptr.s );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section '%s' into '%s' failed!\n", "policy", 
            MWAN3_CONFIG_NAME );
        return -1;
    }
    
    uci_add_option( &ptr, "use_policy", rule->use_policy );

    if ( rule->ipset ) {
        uci_add_option( &ptr, "ipset", rule->ipset );
    }

    if ( rule->src ) {
        uci_add_option( &ptr, "src", rule->src );
    }

    if ( rule->proto ) {
        uci_add_option( &ptr, "proto", rule->proto );
        if ( !strcmp(rule->proto, "icmp") ) {
            int i;

            for ( i = 0; i < rule->icmp_type_cnt; i++ ) {
                mwan3_add_list( &ptr, NULL, "icmp_type", rule->icmp_type[i] );
            }
        }
    }

    if ( rule->sticky ) {
        uci_add_option( &ptr, "sticky", rule->sticky );
    }

    if ( rule->timeout ) {
        uci_add_option( &ptr, "timeout", rule->timeout );
    }

    if ( rule->src_mac ) {
        uci_add_option( &ptr, "src_mac", rule->src_mac );
    }

    if ( rule->src_ip ) {
        uci_add_option( &ptr, "src_ip", rule->src_ip );
    }

    if ( rule->src_port ) {
        uci_add_option( &ptr, "src_port", rule->src_port );
    }

    if ( rule->dest_ip ) {
        uci_add_option( &ptr, "dest_ip", rule->dest_ip );
    }

    if ( rule->dest_port ) {
        uci_add_option( &ptr, "dest_port", rule->dest_port );
    }

    if ( rule->family ) {
        uci_add_option( &ptr, "family", rule->family );
    }

    if ( rule->is_default ) {
        char val[BUF_LEN_16] = { 0 };

        snprintf( val, sizeof(val), "%d", rule->is_default );
        uci_add_option( &ptr, "is_default", val );
        snprintf( val, sizeof(val), "%d", rule->vpn_sence );
        uci_add_option( &ptr, "vpn_sence", val );
        if ( rule->vpn_iface ) {
            uci_add_option( &ptr, "vpn_iface", rule->vpn_iface );
        }
    }

    if ( rule->is_split ) {
        char val[BUF_LEN_16] = { 0 };

        snprintf( val, sizeof(val), "%d", rule->is_split );
        uci_add_option( &ptr, "is_split", val );
        if ( rule->vpn_iface ) {
            uci_add_option( &ptr, "vpn_iface", rule->vpn_iface );
        }
    }

    return 0;
}

//=============================================================================
static inline int
mwan3_add_policy(
    struct uci_package *p,
    char *policy_name,
    char **member_list,
    int member_cnts
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    int i;
    int ret = 0;

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section_named( ctx, p, "policy", &ptr.s, policy_name );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section '%s' into '%s' failed, name %s!\n", "policy", 
            MWAN3_CONFIG_NAME, policy_name );
        return -1;
    }

    for ( i = 0; i < member_cnts; i++ ) {
        mwan3_add_list( &ptr, NULL, "use_member", member_list[i] );
    }

    return 0;
}

//=============================================================================
static inline int
mwan3_add_member(
    struct uci_package *p,
    char *memb_name,
    char *ifname,
    char *weight
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    int ret = 0;

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section_named( ctx, p, "member", &ptr.s, memb_name );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section '%s' into '%s' failed, name %s!\n", "member", 
            MWAN3_CONFIG_NAME, memb_name );
        return -1;
    }
    uci_add_option( &ptr, "interface", ifname );
    if ( weight ) {
        uci_add_option( &ptr, "weight", weight );
    }

    return 0;
}

//=============================================================================
static inline int
mwan3_add_interface(
    struct uci_package *p,
    char *ifname,
    struct static_dns *dns,
    char *tunnel_default
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    int ret = 0;

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section_named( ctx, p, "interface", &ptr.s, ifname );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section '%s' into '%s' failed, name %s!\n", "interface", 
            MWAN3_CONFIG_NAME, ifname );
        return -1;
    }

    uci_add_option( &ptr, "enabled", "1" );
    if ( tunnel_default ) {
        uci_add_option( &ptr, "tunnel_default", tunnel_default );
    }
#ifndef GWN7052 // GWN7052 only has a wan interface, doesn't need to track.
    if ( strstr(ifname, "wan") ) {
        uci_add_option( &ptr, "flush_conntrack", "ifdown" );
    }

    if ( dns && dns->valid ) {
        if ( dns->first ) {
            mwan3_add_list( &ptr, NULL, "track_ip", dns->first );
        }

        if ( dns->second ) {
            mwan3_add_list( &ptr, NULL, "track_ip", dns->second );
        }
    }
#endif

    return 0;
}

//=============================================================================
static int
mwan3_add_vpn_split_tunneling(
    struct uci_package *p,
    struct vpn_split_cfg *split_cfg
)
//=============================================================================
{
    struct uci_element *e;
    struct mwan3_rule rule = { 0 };
    uint32_t ip;
    int match_ipset = 0;
    char ipset_nm[BUF_LEN_64] = { 0 };
    char policy_nm[BUF_LEN_32] = { 0 };

    snprintf( policy_nm, sizeof(policy_nm), "vpn%d_auto", split_cfg->vpn_id );
    if ( split_cfg->mode[0] == '0' ) { // Using service addr
        if ( split_cfg->service_addr ) {
            uci_foreach_element( &split_cfg->service_addr->v.list, e ) {
                memset( &rule, 0, sizeof(rule) );
                if ( strchr(e->name, '/') || inet_pton(AF_INET, e->name, &ip ) == 1 ) {
                    rule.use_policy = policy_nm;
                    rule.is_split = 1;
                    rule.vpn_iface = split_cfg->vpn_iface;
                    rule.src = DEFAULT_LAN;
                    rule.dest_ip = e->name;
                    rule.proto = "all";
                    rule.family = "ipv4";
                    mwan3_add_rule( p, &rule );
                }
                else {
                    match_ipset = 1;
                }
            }

            if ( match_ipset ) {
                memset( &rule, 0, sizeof(rule) );
                snprintf( ipset_nm, sizeof(ipset_nm), "vpn_ipset_%d", split_cfg->vpn_id );
                rule.use_policy = policy_nm;
                rule.is_split = 1;
                rule.vpn_iface = split_cfg->vpn_iface;
                rule.src = DEFAULT_LAN;
                rule.ipset = ipset_nm;
                mwan3_add_rule( p, &rule );
            }
        }
    }
    else {  // Using dev mac
        if ( split_cfg->dev ) {
            uci_foreach_element( &split_cfg->dev->v.list, e ) {
                char mac_str[BUF_LEN_32] = { 0 };
                char *mac = strchr( e->name, '/' ) + 1;
            
                snprintf( mac_str, sizeof(mac_str), "%.2s:%.2s:%.2s:%.2s:%.2s:%.2s",
                    mac + 0, mac + 2, mac + 4,
                    mac + 6, mac + 8, mac + 10 );
                memset( &rule, 0, sizeof(rule) );
                rule.use_policy = policy_nm;
                rule.is_split = 1;
                rule.vpn_iface = split_cfg->vpn_iface;
                rule.src = DEFAULT_LAN;
                rule.src_mac = mac_str;
                rule.proto = "all";
                rule.family = "ipv4";
                mwan3_add_rule( p, &rule );
            }
        }
    }

    return 0;
}

//=============================================================================
static int
mwan3_add_balance(
    struct uci_package *p,
    struct balance_rule_cfg *cfg
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    struct mwan3_rule rule = { 0 };

    if ( mwan3_add_policy(p, cfg->policy_nm, cfg->member_list, cfg->member_cnts) < 0 ) {
        return -1;
    }

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ptr.section = cfg->policy_nm;

    rule.use_policy = cfg->policy_nm;
    rule.src = cfg->src;
    rule.src_ip = cfg->src_ip;
    rule.proto = "all";
    rule.family = "ipv4";
    rule.is_default = cfg->is_default;
    rule.vpn_sence = cfg->vpn_sence;
    rule.vpn_iface = cfg->vpn_iface;
    if ( mwan3_add_rule(p, &rule) < 0 ) {
        return -1;
    }

    return 0;
}

//=============================================================================
static int
mwan3_wan_en(
    struct uci_package *p,
    struct mwan3_cfg *cfg
)
//=============================================================================
{
    struct mwan3_rule rule = { 0 };
    char ipset_name[BUF_LEN_64] = { 0 };
    char ifname[BUF_LEN_32] = { 0 };
    char member_name[BUF_LEN_32] = { 0 };
    char policy_name[BUF_LEN_32] = { 0 };
    char *member_list[WAN_CNT_MAX] = { 0 };

    snprintf( ifname, sizeof(ifname), "wan%d", cfg->id );
    snprintf( member_name, sizeof(member_name), "m_wan%d_auto", cfg->id );
    snprintf( policy_name, sizeof(policy_name), "wan%d_auto", cfg->id );

    if ( mwan3_add_interface(p, ifname, &cfg->dns, NULL) < 0 ) {
        return -1;
    }

    if ( mwan3_add_member(p, member_name, ifname, cfg->weight) < 0 ) {
        return -1;
    }

    member_list[0] = member_name;
    if ( mwan3_add_policy(p, policy_name, member_list, 1) < 0 ) {
        return -1;
    }

    snprintf( ipset_name, sizeof(ipset_name), "%s_dns_ipset1", ifname );
    rule.use_policy = policy_name;
    rule.ipset = ipset_name;
    if ( mwan3_add_rule(p, &rule) < 0 ) {
        return -1;
    }

    if ( cfg->wan_type != WAN_TYPE_STATIC ) {
        snprintf( ipset_name, sizeof(ipset_name), "%s_dns_ipset2", ifname );
        rule.ipset = ipset_name;
        if ( mwan3_add_rule(p, &rule) < 0 ) {
            return -1;
        }
    }

    return 0;
}

//=============================================================================
static int
mwan3_global_en(
    struct uci_package *p
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    int ret = 0;

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section_named( ctx, p, "globals", &ptr.s, "globals" );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section '%s' into '%s' failed!\n", "globals", 
            MWAN3_CONFIG_NAME );
        return -1;
    }
    uci_add_option( &ptr, "enabled", "1" );
#ifndef GWN7052
    uci_add_option( &ptr, "local_source", "owntraffic" );
#endif

    return 0;
} 

//=============================================================================
static int
mwan3_vpn_en(
    struct uci_package *p,
    struct mwan3_cfg *cfg
)
//=============================================================================
{
    struct mwan3_rule rule = { 0 };
    char ipset_name[BUF_LEN_64] = { 0 }; 
    char ifname[BUF_LEN_32] = { 0 };
    char member_name[BUF_LEN_32] = { 0 };
    char policy_name[BUF_LEN_32] = { 0 };
    char *member_list[WAN_CNT_MAX] = { 0 };

    snprintf( ifname, sizeof(ifname), "vpn%d", cfg->id );
    snprintf( member_name, sizeof(member_name), "m_vpn%d_auto", cfg->id );
    snprintf( policy_name, sizeof(policy_name), "vpn%d_auto", cfg->id );

    if ( mwan3_add_interface(p, ifname, NULL, cfg->tunnel_default) < 0 ) {
        return -1;
    }

    if ( mwan3_add_member(p, member_name, ifname, cfg->weight) < 0 ) {
        return -1;
    }

    member_list[0] = member_name;
    if ( mwan3_add_policy(p, policy_name, member_list, 1) < 0 ) {
        return -1;
    }

    snprintf( ipset_name, sizeof(ipset_name), "%s_dns_ipset1", ifname );
    rule.use_policy = policy_name;
    rule.ipset = ipset_name;
    if ( mwan3_add_rule(p, &rule) < 0 ) {
        return -1;
    }

    snprintf( ipset_name, sizeof(ipset_name), "%s_dns_ipset2", ifname );
    rule.ipset = ipset_name;
    if ( mwan3_add_rule(p, &rule) < 0 ) {
        return -1;
    }

    return 0; 
}

//=============================================================================
static int
mwan3_reparse(
    void
)
//=============================================================================
{
    struct uci_package *cfgmanager = NULL, *mw3 = NULL, *network = NULL;
    struct uci_element *e = NULL;
    struct vpn_split_cfg split_cfg[VPN_CLIENT_CNT_MAX];
    struct balance_rule_cfg balance_cfg;
    struct mwan3_cfg wan_cfg[WAN_CNT_MAX];
    struct mwan3_cfg wan_vpn_cfg[WAN_CNT_MAX];
    struct mwan3_cfg vpn_client_cfg[VPN_CLIENT_CNT_MAX];
    struct mwan3_cfg vpn_server_cfg[VPN_CLIENT_CNT_MAX];
    const char *balance_mode = NULL;
    struct mwan3_rule rule = { 0 };
    char *member_list[WAN_CNT_MAX] = { 0 };
    uint32_t wan_vpn_sence = 0;
    int wan_cnt = 0, wan_vpn_cnt = 0;
    int vpn_client_cnt = 0, vpn_server_cnt = 0;
    int vpn_split_cnt = 0;
    int i = 0;
    int ret = 0;

    cfmanager_log_message( L_INFO, "Parse mwan3...\n" );
    ctx = uci_alloc_context();
    if ( NULL == ctx ) {
        cfmanager_log_message( L_ERR, "Failed to connect to config\n" );
        return -1;
    }
    uci_set_confdir( ctx, CM_CONFIG_PATH );

    //Reload package, the config might have changed in the background
    cfgmanager = uci_lookup_package( ctx, CFMANAGER_CONFIG_NAME );
    if ( cfgmanager ) {
        uci_unload( ctx, cfgmanager );
    }

    if ( uci_load( ctx, CFMANAGER_CONFIG_NAME, &cfgmanager ) ) {
        cfmanager_log_message( L_DEBUG,
            "Error loading %s\n", CFMANAGER_CONFIG_NAME );
        uci_free_context( ctx );
        return -1;
    }

    uci_clean_package( MWAN3_CONFIG_PATH );
    mw3 = uci_lookup_package( ctx, MWAN3_CONFIG_NAME );
    if ( mw3 ) {
        uci_unload( ctx, mw3 );
    }

    if ( uci_load( ctx, MWAN3_CONFIG_NAME, &mw3 ) ) {
        cfmanager_log_message( L_ERR,
            "Init package '%s' failed\n", MWAN3_CONFIG_NAME );
        uci_unload( ctx, cfgmanager );
        uci_free_context( ctx );
        return -1;
    }

    // Get wan static dns by network
    memset( &wan_cfg, 0, sizeof(wan_cfg) ); 
    network = uci_lookup_package( ctx, "network" );
    if ( network ) {
        uci_unload( ctx, network );
    }

    if ( uci_load( ctx, "network", &network ) ) {
        cfmanager_log_message( L_ERR, "Error loading network\n" );
    }
    else {
        uci_foreach_element( &network->sections, e ) {
            struct uci_section *s = uci_to_section(e);

            if ( !strcmp(s->type, "interface") && strstr(s->e.name, "wan") ) {
                struct uci_option *dns_option = uci_lookup_option( ctx, s, "dns" );
                struct uci_element *e1 = NULL;
                struct static_dns *dns = NULL;
                int i = 0;

                if ( dns_option ) {
                    if ( !strcmp(s->e.name, "wan0") ) {
                        dns = &wan_cfg[0].dns;
                    }
                    else if ( !strcmp(s->e.name, "wan1") ) {
                        dns = &wan_cfg[1].dns;
                    }

                    uci_foreach_element( &dns_option->v.list, e1 ) {
                        if ( i >= WAN_CNT_MAX ) {
                            break;
                        }

                        dns->valid = 1;
                        if ( i == 0 ) {
                            dns->first = e1->name;
                        }
                        else if ( i == 1 ) {
                            dns->second = e1->name;
                        }
                    
                        i++;
                    } 
                }
            }
        }
    }

    /* WAN and VPN Zones */
    memset( &wan_vpn_cfg, 0, sizeof(wan_vpn_cfg) );
    memset( &vpn_client_cfg, 0, sizeof(vpn_client_cfg) );
    memset( &vpn_server_cfg, 0, sizeof(vpn_server_cfg) );
    uci_foreach_element( &cfgmanager->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( !strcmp(s->type, "wan") && !strcmp(s->e.name, "wan") ) {
            const char *wan1_enable = NULL;
            const char *wan0_static_dns_enable = NULL;
            const char *wan1_static_dns_enable = NULL;

            wan_vpn_sence |= INTF_WAN0_EN;
            wan_cfg[0].id = 0;
            wan_cfg[0].wan_index = WAN0;
            wan_cfg[0].wan_type = atoi( uci_lookup_option_string(ctx, s, "wanType") );
            if ( wan_cfg[0].wan_type == WAN_TYPE_PPTP || wan_cfg[0].wan_type == WAN_TYPE_L2TP ) {
                wan_vpn_sence |= INTF_VPN0_EN;
                wan_vpn_cfg[0].id = 0;
                wan_vpn_cfg[0].wan_index = WAN0;
                wan_vpn_cnt++;
            }

            wan0_static_dns_enable = uci_lookup_option_string( ctx, s, "wanStaticDnsEnable" );
            if ( (wan0_static_dns_enable && wan0_static_dns_enable[0] == '1') || 
                wan_cfg[0].wan_type == WAN_TYPE_STATIC ) {
                const char *wan0_first_dns = uci_lookup_option_string( ctx, s, "wanFirstDns" );
                const char *wan0_second_dns = uci_lookup_option_string( ctx, s, "wanSecondDns" );

                if ( wan_cfg[0].wan_type != WAN_TYPE_PPTP && wan_cfg[0].wan_type != WAN_TYPE_L2TP ) {
                    wan_cfg[0].dns.valid = 1;
                    wan_cfg[0].dns.first = wan0_first_dns;
                    wan_cfg[0].dns.second = wan0_second_dns;
                }
            }
            wan_cnt++;

            wan1_enable = uci_lookup_option_string( ctx, s, "wan1Enable" );
            if ( wan1_enable && wan1_enable[0] == '1' ) {
                wan_cnt++;
                wan_vpn_sence |= INTF_WAN1_EN;
                wan_cfg[1].id = 1;
                wan_cfg[1].wan_index = WAN1;
                wan_cfg[1].wan_type = atoi( uci_lookup_option_string(ctx, s, "wan1Type") );
                if ( wan_cfg[1].wan_type == WAN_TYPE_PPTP || wan_cfg[1].wan_type == WAN_TYPE_L2TP ) {
                    wan_vpn_sence |= INTF_VPN1_EN;
                    wan_vpn_cfg[1].id = 1;
                    wan_vpn_cfg[1].wan_index = WAN1;
                    wan_vpn_cnt++;
                }

                wan1_static_dns_enable = uci_lookup_option_string( ctx, s, "wan1StaticDnsEnable");
                if ( (wan1_static_dns_enable && wan1_static_dns_enable[0] == '1') ||
                    wan_cfg[1].wan_type == WAN_TYPE_STATIC ) {
                    const char *wan1_first_dns = uci_lookup_option_string( ctx, s, "wan1FirstDns" );
                    const char *wan1_second_dns = uci_lookup_option_string( ctx, s, "wan1SecondDns" );

                    if ( wan_cfg[1].wan_type != WAN_TYPE_PPTP && wan_cfg[1].wan_type != WAN_TYPE_L2TP ) {
                        wan_cfg[1].dns.valid = 1;
                        wan_cfg[1].dns.first = wan1_first_dns;
                        wan_cfg[1].dns.second = wan1_second_dns;
                    }
                }

                balance_mode = uci_lookup_option_string( ctx, s, "wanBalance" );
                if ( !balance_mode ) {
                    wan_cfg[0].weight = "1";
                    wan_cfg[1].weight = "1";
                }
                else {
                    char *p = strchr( balance_mode, ':' );
                    *p = '\0';
                    wan_cfg[0].weight = (char *)balance_mode;
                    wan_cfg[1].weight = p + 1;
                }

                // If both wan set vpn from wan setting, we can balance lan traffict between them.
                if ( wan_vpn_cnt == WAN_CNT_MAX ) {
                    wan_vpn_cfg[0].weight = wan_cfg[0].weight;
                    wan_vpn_cfg[1].weight = wan_cfg[1].weight;
                }

                cfmanager_log_message( L_DEBUG,
                    "wan0_weight %s, wan1_weight %s\n", wan_cfg[0].weight, wan_cfg[1].weight );
            }
        }

        if ( !strcmp(s->type, "vpn_service") ) {
            const char *enable = uci_lookup_option_string( ctx, s, "enable" );
            int vpn_type = atoi( uci_lookup_option_string( ctx, s, "type" ) );
            const char *wan_intf = uci_lookup_option_string( ctx, s, "wan" );
            const char *id = uci_lookup_option_string( ctx, s, "id" );

            if ( enable[0] == '1' ) {
                vpn_client_cfg[vpn_client_cnt].id = atoi(id) + VPN_CLIENT_ID_OFFSET;
                if ( !strcmp(wan_intf, "wan0") ) {
                    wan_vpn_sence |= INTF_VPNX0_EN;
                    vpn_client_cfg[vpn_client_cnt].wan_index = WAN0;
                }
                else {
                    wan_vpn_sence |= INTF_VPNX1_EN;
                    vpn_client_cfg[vpn_client_cnt].wan_index = WAN1;
                }

                if ( vpn_type == VPN_CLIENT_TYPE_IPSEC ) {
                    vpn_client_cfg[vpn_client_cnt].tunnel_default = (char *)wan_intf;
                }
                vpn_client_cnt++;
            }
        }

        // Don't consider vpn server for split tunneling.
        if ( !strcmp(s->type, "vpn_server") ) {
            const char *enable = uci_lookup_option_string( ctx, s, "enable" );
            const char *wan_intf = uci_lookup_option_string( ctx, s, "wan" );
            const char *id = uci_lookup_option_string( ctx, s, "id" );

            if ( enable[0] == '1' ) {
                vpn_server_cfg[vpn_server_cnt].id = atoi(id) + VPN_SERVER_ID_OFFSET;
                if ( !strcmp(wan_intf, "wan0") ) {
                    vpn_server_cfg[vpn_server_cnt].wan_index = WAN0;
                }
                else {
                    vpn_server_cfg[vpn_server_cnt].wan_index = WAN1;
                }

                vpn_server_cnt++;
            }
        }
    }

    // For now, only vpn client support split tunneling
    memset( &split_cfg, 0, sizeof(split_cfg) );
    uci_foreach_element( &cfgmanager->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( !strcmp(s->type, "vpn_split") ) {
            int vpn_id = -1;

            if ( vpn_split_cnt >= ARRAY_SIZE(split_cfg) ) {
                cfmanager_log_message( L_ERR,
                    "vpn split tunneling configure count %d exceed limit, only handle %d split config\n",
                    vpn_split_cnt, VPN_CLIENT_CNT_MAX );
                break;
            }

            vpn_id = atoi( uci_lookup_option_string( ctx, s, "id" ) );
            if ( vpn_id >= VPN_CLIENT_CNT_MAX && vpn_id < VPN_SERVER_ID_OFFSET ) {
                // Refer to WAN_VPNx
                vpn_id = vpn_id - VPN_CLIENT_CNT_MAX;
                for ( i = 0; i < wan_vpn_cnt; i++ ) {
                    if ( vpn_id != wan_vpn_cfg[i].id ) {
                        continue;
                    }

                    split_cfg[vpn_split_cnt].vpn_id = vpn_id;
                    snprintf( split_cfg[vpn_split_cnt].vpn_iface, sizeof(split_cfg[vpn_split_cnt].vpn_iface),
                        "vpn%d", vpn_id );
                    if ( vpn_id == 0 ) {
                        split_cfg[vpn_split_cnt].vpn_sence = INTF_VPN0_EN;
                    }
                    else {
                        split_cfg[vpn_split_cnt].vpn_sence = INTF_VPN1_EN;
                    }
                    split_cfg[vpn_split_cnt].mode = uci_lookup_option_string( ctx, s, "mode" );
                    split_cfg[vpn_split_cnt].service_addr = uci_lookup_option( ctx, s, "serviceAddrList" );
                    split_cfg[vpn_split_cnt].dev = uci_lookup_option( ctx, s, "devList" );
                    vpn_split_cnt++;
                    break;
                }
            }
            else if ( vpn_id < VPN_CLIENT_CNT_MAX ) {
                // Refer to VPN client
                vpn_id += VPN_CLIENT_ID_OFFSET;
                for ( i = 0; i < vpn_client_cnt; i++ ) {
                    // Check if referenced vpn enabled
                    if ( vpn_id != vpn_client_cfg[i].id  ) {
                        continue;
                    }

                    split_cfg[vpn_split_cnt].vpn_id = vpn_id;
                    snprintf( split_cfg[vpn_split_cnt].vpn_iface, sizeof(split_cfg[vpn_split_cnt].vpn_iface),
                        "vpn%d", vpn_id );
                    if ( vpn_client_cfg[i].wan_index == WAN0 ) {
                        split_cfg[vpn_split_cnt].vpn_sence = INTF_VPNX0_EN;
                    }
                    else {
                        split_cfg[vpn_split_cnt].vpn_sence = INTF_VPNX1_EN;
                    }
                    split_cfg[vpn_split_cnt].mode = uci_lookup_option_string( ctx, s, "mode" );
                    split_cfg[vpn_split_cnt].service_addr = uci_lookup_option( ctx, s, "serviceAddrList" );
                    split_cfg[vpn_split_cnt].dev = uci_lookup_option( ctx, s, "devList" );
                    vpn_split_cnt++;
                    break;
                }
            }
            else {
                cfmanager_log_message( L_ERR,
                    "Reference vpn id %d is invalid!\n", vpn_id );
            }
        }
    }

    mwan3_global_en( mw3 );
    for ( i = 0; i < wan_cnt; i++ ) {
        mwan3_wan_en( mw3, &wan_cfg[i] );
    }

    for ( i = 0; i < wan_vpn_cnt; i++ ) {
        mwan3_vpn_en( mw3, &wan_vpn_cfg[i] );
    }

    for ( i = 0; i < vpn_client_cnt; i++ ) {
        mwan3_vpn_en( mw3, &vpn_client_cfg[i] );
    }

    for ( i = 0; i < vpn_server_cnt; i++ ) {
        mwan3_vpn_en( mw3, &vpn_server_cfg[i] );
    }

    cfmanager_log_message( L_DEBUG, "wan_vpn_sence 0x%0x\n", wan_vpn_sence );
    // TODO: Should forward all vlan to wanx or vpnx
    switch ( wan_vpn_sence ) {
        // Single WAN
        case INTF_WAN0_EN:
            // Add owntraffic rule
            memset( &rule, 0, sizeof(rule) );
            rule.src_ip = "192.0.2.0";
            rule.use_policy = WAN0_DEFAULT_POLICY;
            mwan3_add_rule(mw3, &rule);

            // forward lan0_zone0 to wan0
            memset( &rule, 0, sizeof(rule) );
            rule.use_policy = WAN0_DEFAULT_POLICY;
            rule.src = DEFAULT_LAN;
            rule.proto = "all";
            rule.family = "ipv4";
            rule.is_default = 1;
            rule.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_rule( mw3, &rule );
            break;

        case INTF_WAN0_VPN0_EN:
            // Add owntraffic rule
            memset( &rule, 0, sizeof(rule) );
            rule.src_ip = "192.0.2.0";
            rule.use_policy = WAN0_DEFAULT_POLICY;
            mwan3_add_rule(mw3, &rule);

            // forward lan0_zone0 to vpn0
            memset( &rule, 0, sizeof(rule) );
            rule.use_policy = VPN0_DEFAULT_POLICY;
            rule.src = DEFAULT_LAN;
            rule.proto = "all";
            rule.family = "ipv4";
            rule.is_default = 1;
            rule.vpn_sence = VPN_SENCE_SINGLE_VPN_ON_SINGLE_WAN;
            rule.vpn_iface = VPN0_DEFAULT_IFNAME;
            mwan3_add_rule( mw3, &rule );

            // Backup rule, forward lan0_zone0 to wan0
            memset( &rule, 0, sizeof(rule) );
            rule.use_policy = WAN0_DEFAULT_POLICY;
            rule.src = DEFAULT_LAN;
            rule.proto = "all";
            rule.family = "ipv4";
            rule.is_default = 1;
            rule.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_rule( mw3, &rule );
            break;

        case INTF_WAN0_VPNX0_EN:
            // Add owntraffic rule
            memset( &rule, 0, sizeof(rule) );
            rule.src_ip = "192.0.2.0";
            rule.use_policy = WAN0_DEFAULT_POLICY;
            mwan3_add_rule(mw3, &rule);

            // Add split tunneling rule
            for ( i = 0; i < vpn_split_cnt; i++ ) {
                if ( split_cfg[i].vpn_sence & INTF_VPNX0_EN ) {
                    mwan3_add_vpn_split_tunneling( mw3, &split_cfg[i] );
                }
                else {
                    cfmanager_log_message( L_ERR,
                        "Vpn split refer to error client id %d!\n", split_cfg[i].vpn_id );
                }
            }

            // forward lan0_zone0 to wan0
            memset( &rule, 0, sizeof(rule) );
            rule.use_policy = WAN0_DEFAULT_POLICY;
            rule.src = DEFAULT_LAN;
            rule.proto = "all";
            rule.family = "ipv4";
            rule.is_default = 1;
            rule.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_rule( mw3, &rule );
            break;

        // Dual WANs
        case INTF_DUAL_WAN_EN:
            member_list[0] = WAN0_DEFAULT_MEMBER;
            member_list[1] = WAN1_DEFAULT_MEMBER;
            // Add own traffic balance rule
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance_own";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src_ip = "192.0.2.0";
            balance_cfg.is_default = 0;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_balance( mw3, &balance_cfg );

            // Backup balance rule
            member_list[0] = WAN0_DEFAULT_MEMBER;
            member_list[1] = WAN1_DEFAULT_MEMBER;
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance_backup";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src = DEFAULT_LAN;
            balance_cfg.is_default = 1;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_balance( mw3, &balance_cfg );
            break;

        case INTF_DUAL_WAN_VPN0_EN:
        case INTF_DUAL_WAN_VPN1_EN:
            // Add own traffic balance rule
            member_list[0] = WAN0_DEFAULT_MEMBER;
            member_list[1] = WAN1_DEFAULT_MEMBER;
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance_own";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src_ip = "192.0.2.0";
            balance_cfg.is_default = 0;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_balance( mw3, &balance_cfg );

            // forward lan0_zone0 to vpnx
            memset( &rule, 0, sizeof(rule) );
            rule.src = DEFAULT_LAN;
            rule.proto = "all";
            rule.family = "ipv4";
            rule.is_default = 1;
            rule.vpn_sence = VPN_SENCE_SINGLE_VPN_ON_DUAL_WAN;
            if ( wan_vpn_sence & INTF_VPN0_EN ) {
                rule.use_policy = VPN0_DEFAULT_POLICY;
                rule.vpn_iface = VPN0_DEFAULT_IFNAME;
            }
            else {
                rule.use_policy = VPN1_DEFAULT_POLICY;
                rule.vpn_iface = VPN1_DEFAULT_IFNAME;
            }
            mwan3_add_rule( mw3, &rule );

            // Backup balance rule
            member_list[0] = WAN0_DEFAULT_MEMBER;
            member_list[1] = WAN1_DEFAULT_MEMBER;
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance_backup";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src = DEFAULT_LAN;
            balance_cfg.is_default = 1;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_balance( mw3, &balance_cfg );
            break;

        case INTF_DUAL_WAN_VPN0_VPNX0_EN:
        case INTF_DUAL_WAN_VPN0_VPNX1_EN:
        case INTF_DUAL_WAN_VPN1_VPNX0_EN:
        case INTF_DUAL_WAN_VPN1_VPNX1_EN:
            // Add own traffic balance rule
            member_list[0] = WAN0_DEFAULT_MEMBER;
            member_list[1] = WAN1_DEFAULT_MEMBER;
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance_own";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src_ip = "192.0.2.0";
            balance_cfg.is_default = 0;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            ret = mwan3_add_balance( mw3, &balance_cfg );

            // Add split tunneling rule
            for ( i = 0; i < vpn_split_cnt; i++ ) {
                if ( split_cfg[i].vpn_sence & (INTF_VPNX0_EN | INTF_VPNX1_EN) ) {
                    mwan3_add_vpn_split_tunneling( mw3, &split_cfg[i] );
                }
                else {
                    cfmanager_log_message( L_ERR,
                        "Vpn split refer to error client id %d!\n", split_cfg[i].vpn_id);
                } 
            }

            // forward lan0_zone0 to vpnx
            memset( &rule, 0, sizeof(rule) );
            rule.src = DEFAULT_LAN;
            rule.proto = "all";
            rule.family = "ipv4";
            rule.is_default = 1;
            rule.vpn_sence = VPN_SENCE_SINGLE_VPN_ON_DUAL_WAN;
            if ( wan_vpn_sence & INTF_VPN0_EN ) {
                rule.use_policy = VPN0_DEFAULT_POLICY;
                rule.vpn_iface = VPN0_DEFAULT_IFNAME;
            }
            else {
                rule.use_policy = VPN1_DEFAULT_POLICY;
                rule.vpn_iface = VPN1_DEFAULT_IFNAME;
            }
            mwan3_add_rule( mw3, &rule );

            // Backup balance rule
            member_list[0] = WAN0_DEFAULT_MEMBER;
            member_list[1] = WAN1_DEFAULT_MEMBER;
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance_backup";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src = DEFAULT_LAN;
            balance_cfg.is_default = 1;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_balance( mw3, &balance_cfg );
            break;

        case INTF_DUAL_WAN_VPN0_VPN1_EN:
            // Add split tunneling rule
            for ( i = 0; i < vpn_split_cnt; i++ ) {
                if ( split_cfg[i].vpn_sence & (INTF_VPN0_EN | INTF_VPN1_EN) ) {
                    mwan3_add_vpn_split_tunneling( mw3, &split_cfg[i] );
                }
                else {
                    cfmanager_log_message( L_ERR,
                        "Vpn split refer to error client id %d!\n", split_cfg[i].vpn_id );
                }
            }

            // Add own traffic balance rule
            member_list[0] = WAN0_DEFAULT_MEMBER;
            member_list[1] = WAN1_DEFAULT_MEMBER;
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance_own";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src_ip = "192.0.2.0";
            balance_cfg.is_default = 0;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_balance( mw3, &balance_cfg );

            // Add vpn balance rule
            member_list[0] = VPN0_DEFAULT_MEMBER;
            member_list[1] = VPN1_DEFAULT_MEMBER;
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src = DEFAULT_LAN;
            balance_cfg.is_default = 1;
            balance_cfg.vpn_sence = VPN_SENCE_DUAL_VPN_ON_DUAL_WAN;
            balance_cfg.vpn_iface = "vpn0 vpn1";
            mwan3_add_balance( mw3, &balance_cfg );

            // Backup balance rule.
            member_list[0] = WAN0_DEFAULT_MEMBER;
            member_list[1] = WAN1_DEFAULT_MEMBER;
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance_backup";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src = DEFAULT_LAN;
            balance_cfg.is_default = 1;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_balance( mw3, &balance_cfg );
            break;

        case INTF_DUAL_WAN_VPNX0_EN:
        case INTF_DUAL_WAN_VPNX1_EN:
        case INTF_DUAL_WAN_VPNX0_VPNX1_EN:
            // Add split tunneling rule
            for ( i = 0; i < vpn_split_cnt; i++ ) {
                if ( split_cfg[i].vpn_sence & (INTF_VPNX0_EN | INTF_VPNX1_EN) ) {
                    mwan3_add_vpn_split_tunneling( mw3, &split_cfg[i] );
                }
                else {
                    cfmanager_log_message( L_ERR,
                        "Vpn split refer to error client id %d!\n", split_cfg[i].vpn_id );
                }
            }

            member_list[0] = WAN0_DEFAULT_MEMBER;
            member_list[1] = WAN1_DEFAULT_MEMBER; 
            // Add own traffic balance rule
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance_own";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src_ip = "192.0.2.0";
            balance_cfg.is_default = 0;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_balance( mw3, &balance_cfg );

            // Add default balance
            memset( &balance_cfg, 0, sizeof(balance_cfg) );
            balance_cfg.policy_nm = "balance";
            balance_cfg.member_list = member_list;
            balance_cfg.member_cnts = ARRAY_SIZE(member_list);
            balance_cfg.src = DEFAULT_LAN;
            balance_cfg.is_default = 1;
            balance_cfg.vpn_sence = VPN_SENCE_OTHERS;
            mwan3_add_balance( mw3, &balance_cfg );
            break;

        default:
            break;
    }

    ret = uci_save( ctx, mw3 );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_DEBUG, "save %s failed\n", MWAN3_CONFIG_NAME );
        goto err;
    }

    ret = uci_commit( ctx, &mw3, false );
    if ( ret != UCI_OK  ) {
        cfmanager_log_message( L_DEBUG, "commit %s failed\n", MWAN3_CONFIG_NAME );
        goto err;
    }

    uci_unload( ctx, cfgmanager );
    uci_unload( ctx, mw3 );
    uci_free_context( ctx );

    return ret;
err:
    uci_unload( ctx, cfgmanager );
    uci_unload( ctx, mw3 );
    uci_free_context( ctx );

    return -1;
}

//=============================================================================
int
cfparse_load_mwan3(
    void
)
//=============================================================================
{
    mwan3_reparse();
    /*
     * mwan3 does not need to be compared, it needs to be reset every time
     */
    apply_add( "mwan3" );
    apply_timer_start();

    return 0;
}

