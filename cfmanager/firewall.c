/****************************************************************************
* *
* * FILENAME:        $RCSfile: firewall.c,v $
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
#include "cfparse.h"
#include "config.h"
#include "ubus.h"
#include "firewall.h"
#include "time.h"
#include "track.h"
#include "apply.h"
#include "sgrequest.h"
#include "cfmanager.h"

//=================
//  Defines
//=================
//#define FIREWALL_CONFIG_PATH "/etc/config/firewall"
#define FIREWALL_CONFIG_PATH SPLICE_STR(CM_CONFIG_PATH,/firewall)

//=================
//  Typedefs
//=================
enum {
    FIREWALL_REDIRECT_PROTO_TCP,
    FIREWALL_REDIRECT_PROTO_UDP,
    FIREWALL_REDIRECT_PROTO_TCP_UDP,

    __FIREWALL_REDIRECT_PROTO_MAX
};

//=================
//  Globals
//=================
extern const struct blobmsg_policy port_mapping_policy[__PORT_MAPPING_MAX];
extern struct vlist_tree cm_port_mapping_vltree;
extern const struct blobmsg_policy dmz_policy[__DMZ_MAX];
extern const struct blobmsg_policy firewall_dos_policy[__FIREWALL_DOS_MAX];

//=================
//  Locals
//=================

static struct uci_context *ctx;

// Must set rule in order
static struct firewall_rule wan_default_rule[] = {
    {
        .name = "Allow-DHCP-Renew",
        .family = "ipv4",
        .proto = "udp",
        .dest_port = "68",
        .target = "ACCEPT"
    },

    {
        .name = "Allow-Ping",
        .family = "ipv4",
        .proto = "icmp",
        .icmp_type = { 
            [0] = "echo-request"
        },
        .icmp_type_cnt = 1,
        .target = "ACCEPT"
    },

    {
        .name = "Allow-IGMP",
        .family = "ipv4",
        .proto = "igmp",
        .target = "ACCEPT"
    },

    {
        .name = "Allow-DHCPv6",
        .family = "ipv6",
        .src_ip = "fe80::/10",
        .proto = "udp",
        .dest_ip = "fe80::/10",
        .dest_port = "546",
        .target = "ACCEPT"
    },

    {
        .name = "Allow-MLD",
        .family = "ipv6",
        .src_ip = "fe80::/10",
        .proto = "icmp",
        .icmp_type = { 
            [0] = "130/0",
            [1] = "131/0",
            [2] = "132/0",
            [3] = "134/0",
            [4] = "143/0"
        },
        .icmp_type_cnt = 5,
        .target = "ACCEPT"
    },

    {
        .name = "Allow-ICMPv6-Input",
        .family = "ipv6",
        .proto = "icmp",
        .icmp_type = {
            [0] = "echo-request",
            [1] = "echo-reply",
            [2] = "destination-unreachable",
            [3] = "packet-too-big",
            [4] = "time-exceeded",
            [5] = "bad-header",
            [6] = "unknown-header-type",
            [7] = "router-solicitation",
            [8] = "neighbour-solicitation",
            [9] = "neighbour-advertisement",
        },
        .icmp_type_cnt = 10,
        .target = "ACCEPT"
    },

    {
        .name = "Allow-ICMPv6-Forward",
        .family = "ipv6",
        .proto = "icmp",
        .icmp_type = {
            [0] = "echo-request",
            [1] = "echo-reply",
            [2] = "destination-unreachable",
            [3] = "packet-too-big",
            [4] = "time-exceeded",
            [5] = "bad-header",
        },
        .icmp_type_cnt = 6,
        .dest = DEFAULT_LAN,
        .target = "ACCEPT"
    }
};

static struct firewall_rule controller_default_rule[] = {
    {
        .name = "Allow-Controller-Input",
        .src = DEFAULT_LAN,
        .proto = "tcpudp",
        .dest_port = "14",
        .target = "ACCEPT"
    },

    {
        .name = "Allow-Controller-Output",
        .dest = DEFAULT_LAN,
        .proto = "tcpudp",
        .src_port = "14",
        .target = "ACCEPT"
    }
};

static struct firewall_rule lan_default_dhcp_rule[] = {
    {
        .name = DEFAULT_LAN"-Allow-DHCP-Input",
        .src = DEFAULT_LAN,
        .proto = "udp",
        .src_port = "67 68",
        .dest_port = "67 68",
        .target = "ACCEPT"
    },

    {
        .name = DEFAULT_LAN"-Allow-DHCP-Output",
        .dest = DEFAULT_LAN,
        .proto = "udp",
        .src_port = "67 68",
        .dest_port = "67 68",
        .target = "ACCEPT"
    }
};

static struct firewall_rule auto_forward_rule = {
    .family = "any",
    .proto = "all",
    .target = "ACCEPT"
};

/*
 * Private Functions
 */
static int
firewall_parse_def_DOS(
    struct uci_package *cfmanager_pkg,
    struct uci_package *firewall_pkg
);

/*
 * Private Data
 */

//=============================================================================
static inline void
firewall_add_list(
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
static int 
firewall_rule_writer(
    struct uci_package *fw,
    struct firewall_rule *rule
)
//=============================================================================
{
    int ret;
    struct uci_ptr ptr = { 0 };

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = fw;
    ret = uci_add_section( ctx, fw, "rule", &ptr.s );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section 'rule' into '%s' failed, name %s!\n", 
            FIREWALL_CONFIG_NAME, rule->name );
        return -1;
    }

    if ( rule->name ) {
        uci_add_option( &ptr, "name", rule->name );
    }

    if ( rule->family ) {
        uci_add_option( &ptr, "family", rule->family );
    }

    if ( rule->src ) {
        uci_add_option( &ptr, "src", rule->src );
    }

    if ( rule->src_ip ) {
        uci_add_option( &ptr, "src_ip", rule->src_ip );
    }

    if ( rule->src_port ) {
        uci_add_option( &ptr, "src_port", rule->src_port );
    }

    if ( rule->src_mac ) {
        uci_add_option( &ptr, "src_mac", rule->src_mac );
    }

    if ( rule->proto ) {
        uci_add_option( &ptr, "proto", rule->proto );
    }

    /* Handle icmp_type list */
    if ( rule->icmp_type_cnt ) {
        int i;

        for ( i = 0; i < rule->icmp_type_cnt; i++ ) {
            firewall_add_list( &ptr, NULL, "icmp_type", rule->icmp_type[i] );
        }
    }

    if ( rule->dest ) {
        uci_add_option( &ptr, "dest", rule->dest );
    }

    if ( rule->start_date ) {
        uci_add_option( &ptr, "start_date", rule->start_date );
    }

    if ( rule->stop_date ) {
        uci_add_option( &ptr, "stop_date", rule->stop_date );
    }

    if ( rule->start_time ) {
        uci_add_option( &ptr, "start_time", rule->start_time );
    }

    if ( rule->stop_time ) {
        uci_add_option( &ptr, "stop_time", rule->stop_time );
    }

    if ( rule->weekdays ) {
        uci_add_option( &ptr, "weekdays", rule->weekdays );
    }

    if ( rule->monthdays ) {
        uci_add_option( &ptr, "monthdays", rule->monthdays );
    }

    if ( rule->utc_time ) {
        uci_add_option( &ptr, "utc_time", rule->utc_time );
    }

    if ( rule->mark ) {
        uci_add_option( &ptr, "mark", rule->mark );
    }

    if ( rule->set_mark ) {
        uci_add_option( &ptr, "set_mark", rule->set_mark );
    }

    if ( rule->set_xmark ) {
        uci_add_option( &ptr, "set_xmark", rule->set_xmark );
    }

    if ( rule->extra ) {
        uci_add_option( &ptr, "extra", rule->extra );
    }

    if ( rule->dest_ip ) {
        uci_add_option( &ptr, "dest_ip", rule->dest_ip );
    }

    if ( rule->dest_port ) {
        uci_add_option( &ptr, "dest_port", rule->dest_port );
    }

    if ( rule->target ) {
        uci_add_option( &ptr, "target", rule->target );
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_dhcp_rule(
    struct uci_package *p
)
//=============================================================================
{
    int i;

    for ( i = 0; i < ARRAY_SIZE(lan_default_dhcp_rule); i++ ) {
        if ( firewall_rule_writer( p, &lan_default_dhcp_rule[i] ) < 0 ) {
            return -1;
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_blackhole_rule(
    struct uci_package *p,
    struct uci_package *fw
)
//=============================================================================
{
    struct uci_package *cfmanager_pkg = p;
    struct uci_package *blackhole_pkg;
    struct uci_section *section, *zone_section;
    struct uci_option  *option;
    struct uci_element *list_element;
    struct uci_element *zone_list_element;

    char buffer[64];

    struct uci_ptr ptr = { 0 };
    int ret;


    blackhole_pkg = uci_lookup_package( ctx, "blackhole" );
    if ( blackhole_pkg ) {
        uci_unload( ctx, blackhole_pkg );
    }

    if ( uci_load( ctx, "blackhole", &blackhole_pkg ) ) {
        cfmanager_log_message( L_DEBUG,
            "Error loading %s\n", "blackhole" );
        return -1;
    }

    // Create zone blackhole rules last
    uci_foreach_element( &blackhole_pkg->sections, list_element ) {
        const char *port = NULL;
        const char *zone = NULL;
        const char *lanIp4Address = NULL;

        section = uci_to_section( list_element );
        if( strcmp( section->type, "zone_blackhole" ) == 0 ) {
            port = uci_lookup_option_string( ctx, section, "port" );
            if ( !port || port[0] == '\0' ) {
                cfmanager_log_message( L_DEBUG,
                    "blackhole %s miss port\n", section->e.name );
                continue;
            }

            option = uci_lookup_option( ctx, section, "zones" );
            if ( option && option->type == UCI_TYPE_LIST ) {
                uci_foreach_element( &option->v.list, zone_list_element ) {
                    if ( strcmp( zone_list_element->name, "zone0" ) == 0 ) {
                        zone = "lan";
                    }
                    else {
                        cfmanager_log_message( L_DEBUG,
                            "zone %s does not support blackhole\n",
                            zone_list_element->name );
                        continue;
                    }

                    zone_section = uci_lookup_section( ctx,
                                            cfmanager_pkg,
                                            zone );
                    if ( !zone_section ) {
                        continue;
                    }

                    lanIp4Address = uci_lookup_option_string( ctx, zone_section, "lanIp4Address" );
                    if ( !lanIp4Address )
                        continue;

                    memset( &ptr, 0, sizeof ( ptr ) );
                    ptr.p = fw;
                    ret = uci_add_section( ctx, fw, "redirect", &ptr.s );
                    if ( ret != UCI_OK ) {
                        cfmanager_log_message( L_ERR,
                            "Add section '%s' into '%s' failed!\n", "redirect", FIREWALL_CONFIG_NAME );
                        continue;
                    }

                    snprintf( buffer, sizeof( buffer ), "%s to %s", zone_list_element->name, section->e.name );
                    uci_add_option( &ptr, "name", buffer );

                    snprintf( buffer, sizeof( buffer ), "lan0_%s", zone_list_element->name );
                    uci_add_option( &ptr, "src", buffer );

                    uci_add_option( &ptr, "dest_ip", lanIp4Address );
                    uci_add_option( &ptr, "dest_port", port );
                    uci_add_option( &ptr, "src_dport", "53" );
                    uci_add_option( &ptr, "target", "DNAT" );
                    uci_add_option( &ptr, "proto", "tcpudp" );
                    uci_add_option( &ptr, "dest", "*" );
                }
            }
        }
    }

    return 0;    
}

//=============================================================================
static int
firewall_parse_web_wan_access(
    struct uci_package *p
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    struct uci_package *grandstream_pkg = NULL;
    struct uci_element *list_element = NULL;
    struct uci_section *section = NULL;
    const char *web_wan_access = NULL;
    char web_port[BUF_LEN_64] = { 0 };
    int ret = 0;

    memset( &ptr, 0, sizeof( ptr ) );
    ptr.p = p;

    grandstream_pkg = uci_lookup_package( ctx, "grandstream" );
    if ( grandstream_pkg )
    {
        uci_unload( ctx, grandstream_pkg );
    }

    if ( uci_load( ctx, "grandstream", &grandstream_pkg ) )
    {
        cfmanager_log_message( L_DEBUG, "Error loading %s\n", "grandstream" );
        return -1;
    }

    uci_foreach_element( &grandstream_pkg->sections, list_element )
    {
        section = uci_to_section( list_element );
        if( strcmp( section->type, "general" ) == 0 )
        {
            //read configuration option
            web_wan_access = uci_lookup_option_string( ctx, section, "web_wan_access" );
            if( web_wan_access && !strcmp( web_wan_access, "1" ) )
            {
                ret = uci_add_section_named( ctx, p, "rule", &ptr.s, "web_wan_access" );
                if ( ret != UCI_OK )
                {
                    cfmanager_log_message( L_ERR,
                        "Add user-rule section into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                    return -1;
                }
                uci_add_option( &ptr, "name", "Web UI WAN Access" );
                uci_add_option( &ptr, "src", "wan0" );
                uci_add_option( &ptr, "proto", "tcp" );
                snprintf( web_port, sizeof(web_port), "443" );
                uci_add_option( &ptr, "dest_port", web_port );
                uci_add_option( &ptr, "target", "ACCEPT" );
            }
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_upnp_rule(
    struct uci_package *p,
    struct uci_package *fw
)
//=============================================================================
{
    struct uci_package *upnpd_pkg;
    struct uci_element *e1, *e2;
    struct uci_ptr ptr = { 0 };
    int ret;

    upnpd_pkg = uci_lookup_package( ctx, "upnpd" );
    if ( upnpd_pkg ) {
        uci_unload( ctx, upnpd_pkg );
    }

    if ( uci_load( ctx, "upnpd", &upnpd_pkg ) ) {
        cfmanager_log_message( L_DEBUG,
            "Error loading %s\n", "upnpd" );
        return -1;
    }

    uci_foreach_element( &upnpd_pkg->sections, e1 ) {
        struct uci_section *s = uci_to_section(e1);

        if ( !strcmp(s->type, "upnpd") ) {
            const char *enable = uci_lookup_option_string( ctx, s, "enabled" );
            const char *enable_natpmp = uci_lookup_option_string( ctx, s, "enable_natpmp" );
            const char *enable_upnp = uci_lookup_option_string( ctx, s, "enable_upnp" );
            const char *port = uci_lookup_option_string( ctx, s, "port" );
            struct uci_option *internal_iface = uci_lookup_option( ctx, s, "internal_iface" );

            if ( enable[0] != '1' ) {
                continue;
            }

            memset( &ptr, 0, sizeof(ptr) );
            ptr.p = fw;
            ret = uci_add_section_named( ctx, fw, "include", &ptr.s, "miniupnpd" );
            if ( ret != UCI_OK ) {
                cfmanager_log_message( L_ERR, 
                    "Add section type '%s', name %s into '%s' failed!\n",
                    "include", "miniupnpd", FIREWALL_CONFIG_NAME );
                goto out;
            }
            uci_add_option( &ptr, "type", "script" );
            uci_add_option( &ptr, "path", "/usr/share/miniupnpd/firewall.include" );
            uci_add_option( &ptr, "family", "any" );
            uci_add_option( &ptr, "reload", "1" );

            if ( internal_iface ) {
                uci_foreach_element( &internal_iface->v.list, e2 ) {
                    /*create rule to allow input to upnp service for zone */
                    if ( enable_upnp[0] == '1' ) {
                        memset( &ptr, 0, sizeof(ptr) );
                        ptr.p = fw;
                        ret = uci_add_section( ctx, fw, "rule", &ptr.s );
                        if ( ret != UCI_OK ) {
                            cfmanager_log_message( L_ERR, 
                                "Add section 'rule' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                            goto out;
                        }
                        
                        uci_add_option( &ptr, "src", e2->name );
                        uci_add_option( &ptr, "proto", "tcpudp" );
                        uci_add_option( &ptr, "dest_port", port );
                        uci_add_option( &ptr, "target", "ACCEPT" );
                        
                        /* Create rule to allow output from upnp service to zone */
                        memset( &ptr, 0, sizeof(ptr) );
                        ptr.p = fw;
                        ret = uci_add_section( ctx, fw, "rule", &ptr.s );
                        if ( ret != UCI_OK ) {
                            cfmanager_log_message( L_ERR, 
                                "Add section 'rule' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                            goto out;
                        }
                        
                        uci_add_option( &ptr, "dest", e2->name );
                        uci_add_option( &ptr, "proto", "tcpudp" );
                        uci_add_option( &ptr, "src_port", port );
                        uci_add_option( &ptr, "target", "ACCEPT" );
                    }

                    if ( enable_natpmp[0] == '1' ) {
                        /* Create rule to allow input to nat-pmp service for zone */
                        memset( &ptr, 0, sizeof(ptr) );
                        ptr.p = fw;
                        ret = uci_add_section( ctx, fw, "rule", &ptr.s );
                        if ( ret != UCI_OK ) {
                            cfmanager_log_message( L_ERR, 
                                "Add section 'rule' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                            goto out;
                        }
                        
                        uci_add_option( &ptr, "src", e2->name );
                        uci_add_option( &ptr, "proto", "tcpudp" );
                        uci_add_option( &ptr, "dest_port", "5351" );
                        uci_add_option( &ptr, "target", "ACCEPT" );
                        
                        /* Create rule to allow output from  nat-pmp service to zone */
                        memset( &ptr, 0, sizeof(ptr) );
                        ptr.p = fw;
                        ret = uci_add_section( ctx, fw, "rule", &ptr.s );
                        if ( ret != UCI_OK ) {
                            cfmanager_log_message( L_ERR, 
                                "Add section 'rule' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                            goto out;
                        }
                        
                        uci_add_option( &ptr, "dest", e2->name );
                        uci_add_option( &ptr, "proto", "tcpudp" );
                        uci_add_option( &ptr, "src_port", "5351" );
                        uci_add_option( &ptr, "target", "ACCEPT" );
                    }
                }
            }
            break;
        }
    }

out:
    uci_unload( ctx, upnpd_pkg );

    return 0;
}

//=============================================================================
static int
firewall_parse_controller_rule(
    struct uci_package *p
)
//=============================================================================
{
    int i;

    for ( i = 0; i < ARRAY_SIZE(controller_default_rule); i++ ) {
        if ( firewall_rule_writer( p, &controller_default_rule[i] ) < 0 ) {
            return -1;
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_dmz_rule(
    struct uci_package *p,
    struct uci_package *fw
)
//=============================================================================
{
    struct uci_element *e;
    struct uci_ptr ptr = { 0 };
    int ret;

    uci_foreach_element( &p->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( !strcmp(s->type, "dmz") ) {
            const char *enable = uci_lookup_option_string( ctx, s, "enable" );
            const char *wan = uci_lookup_option_string( ctx, s, "wan" );
            const char *devIp = uci_lookup_option_string( ctx, s, "devIp" );

            if ( !enable || !devIp || !wan) {
                continue;
            }

            if ( enable[0] != '1' ) {
                continue;
            }

            memset( &ptr, 0, sizeof(ptr) );
            ptr.p = fw;
            ret = uci_add_section( ctx, fw, "redirect", &ptr.s );
            if ( ret != UCI_OK ) {
                cfmanager_log_message( L_ERR, 
                    "Add section 'redirect' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                return -1;
            }
            
            uci_add_option( &ptr, "target", "DNAT" );
            uci_add_option( &ptr, "src", wan );
            uci_add_option( &ptr, "dest", "lan0_zone0" );
            uci_add_option( &ptr, "proto", "all" );
            uci_add_option( &ptr, "dest_ip", devIp );
            break;
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_port_mapping_rule(
    struct uci_package *p,
    struct uci_package *fw
)
//=============================================================================
{
    struct uci_element *e;
    struct uci_ptr ptr = { 0 };
    int proto;
    int ret;
    char *proto_map[__FIREWALL_REDIRECT_PROTO_MAX] = {
        "tcp",
        "udp",
        "tcpudp"
    };

    uci_foreach_element( &p->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( !strcmp(s->type, "port_mapping") ) {
            const char *name = uci_lookup_option_string( ctx, s, "name" );
            const char *type = uci_lookup_option_string( ctx, s, "type" );
            const char *ext_port = uci_lookup_option_string( ctx, s, "externalPort" );
            const char *inter_port = uci_lookup_option_string( ctx, s, "internalPort" );
            const char *inter_ip = uci_lookup_option_string( ctx, s, "internalIP" );
            const char *wan = uci_lookup_option_string( ctx, s, "wan" );
            const char *ext_ip = uci_lookup_option_string( ctx, s, "externalIP" );

            if ( !type || !inter_port || !inter_ip || !wan) {
                continue;
            }

            memset( &ptr, 0, sizeof(ptr) );
            ptr.p = fw;
            ret = uci_add_section( ctx, fw, "redirect", &ptr.s );
            if ( ret != UCI_OK ) {
                cfmanager_log_message( L_ERR, 
                    "Add section 'redirect' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                return -1;
            }

            uci_add_option( &ptr, "target", "DNAT" );
            uci_add_option( &ptr, "src", wan );
            uci_add_option( &ptr, "dest", "lan0_zone0" );
            proto = atoi( type );
            uci_add_option( &ptr, "proto", proto_map[proto] );
            uci_add_option( &ptr, "dest_ip", inter_ip );
            uci_add_option( &ptr, "dest_port", inter_port );
            if( ext_ip ) {
                uci_add_option( &ptr, "src_dip", ext_ip );
            }

            if ( ext_port ) {
                uci_add_option( &ptr, "src_dport", ext_port );
            }
            else {
                // user didn't specify, so src_port = internal port'
                uci_add_option( &ptr, "src_dport", inter_port );
            }

            if ( name ) {
                uci_add_option( &ptr, "name", name );
            }
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_ipsec_rule(
    struct uci_package *p,
    struct uci_package *fw
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    int ret;
    struct uci_element *e = NULL;

    uci_foreach_element( &p->sections, e ) {
        struct uci_section *s = uci_to_section(e);
        
        if ( !strcmp(s->type, "vpn_service") || !strcmp(s->type, "vpn_server") ) {
            const char *enable = uci_lookup_option_string( ctx, s, "enable" );
            int type = atoi( uci_lookup_option_string(ctx, s, "type") );
            const char *wan = uci_lookup_option_string( ctx, s, "wan" );

            if ( enable[0] == '1' && 
                    ( (!strcmp(s->type, "vpn_service") && type == VPN_CLIENT_TYPE_IPSEC) ||
                    (!strcmp(s->type, "vpn_server") && type == VPN_SERVER_TYPE_IPSEC) ) )  {
                memset( &ptr, 0 , sizeof( ptr ) );
                ptr.p = fw;
                ret = uci_add_section( ctx, fw, "rule", &ptr.s );
                if ( ret != UCI_OK ) {
                    cfmanager_log_message( L_ERR, 
                        "Add section 'rule' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                    return -1;
                }
                uci_add_option( &ptr, "name", "Allow-IPsec-ESP" );
                uci_add_option( &ptr, "src", wan );
                uci_add_option( &ptr, "proto", "50" );
                uci_add_option( &ptr, "target", "ACCEPT" );

                memset( &ptr, 0 , sizeof( ptr ) );
                ptr.p = fw;
                ret = uci_add_section( ctx, fw, "rule", &ptr.s );
                if ( ret != UCI_OK ) {
                    cfmanager_log_message( L_ERR, 
                        "Add section 'rule into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                    return -1;
                }
                uci_add_option( &ptr, "name", "Allow-IPsec-ESP" );
                uci_add_option( &ptr, "dest", wan );
                uci_add_option( &ptr, "proto", "50" );
                uci_add_option( &ptr, "target", "ACCEPT" );
                
                memset( &ptr, 0 , sizeof( ptr ) );
                ptr.p = fw;
                ret = uci_add_section( ctx, fw, "rule", &ptr.s );
                if ( ret != UCI_OK ) {
                    cfmanager_log_message( L_ERR, 
                        "Add section 'rule' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                    return -1;
                }
                uci_add_option( &ptr, "name", "Allow-ISAKMP" );
                uci_add_option( &ptr, "src", wan );
                uci_add_option( &ptr, "proto", "udp" );
                uci_add_option( &ptr, "dest_port", "500" );
                uci_add_option( &ptr, "target", "ACCEPT" );

                memset( &ptr, 0, sizeof( ptr ) );
                ptr.p = fw;
                ret = uci_add_section( ctx, fw, "rule", &ptr.s );
                if ( ret != UCI_OK ) {
                    cfmanager_log_message( L_ERR, 
                        "Add section 'rule' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
                    return -1;
                }
                uci_add_option( &ptr, "name", "Allow-NAT-T" );
                uci_add_option( &ptr, "src", wan );
                uci_add_option( &ptr, "proto", "udp" );
                uci_add_option( &ptr, "dest_port", "4500" );
                uci_add_option( &ptr, "target", "ACCEPT" );
            }
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_nat_rule(
    struct uci_package *p,
    struct uci_package *fw
)
//=============================================================================
{
    return 0;
}

//=============================================================================
static int
firewall_parse_zone_default_wan_rule(
    struct uci_package *p,
    int wan_cnts,
    char *interface_name
)
//=============================================================================
{
    int i, j;
    char src[BUF_LEN_16] = { 0 };
    char dest[BUF_LEN_16] = { 0 };

    // Set WAN default rule
    for ( i = 0; i < wan_cnts; i++ ) {
        snprintf( src, sizeof(src), "wan%u", i );

        for ( j = 0; j < ARRAY_SIZE(wan_default_rule); j++ ) {
            wan_default_rule[j].src = src;
            if ( firewall_rule_writer( p, &wan_default_rule[j] ) < 0 ) {
                return -1;
            }
        }

        for ( j = 0; j < wan_cnts; j++ ) {
            if ( i != j ) {
                struct firewall_rule rule = {
                    .name = "Allow-ICMPv6-Forward",
                    .family = "ipv6",
                    .proto = "icmp",
                    .icmp_type = {
                        [0] = "echo-request",
                        [1] = "echo-reply",
                        [2] = "destination-unreachable",
                        [3] = "packet-too-big",
                        [4] = "time-exceeded",
                        [5] = "bad-header",
                    },
                    .icmp_type_cnt = 6,
                    .target = "ACCEPT"
                };
                
                snprintf( src, sizeof(src), "wan%u", i );
                snprintf( dest, sizeof(dest), "wan%u", j );
                rule.src = src;
                rule.dest = dest;
                if ( firewall_rule_writer( p, &rule ) < 0 ) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_default_lan_rule(
    struct uci_package *p,
    int wan_cnts,
    char *interface_name
)
//=============================================================================
{
    char name[BUF_LEN_64] = { 0 };
    struct firewall_rule rule = { 0 };

    // Set LAN default rule
    snprintf( name, sizeof(name), "Anti-lockout-Rule-%s", interface_name );
    rule.name = name;
    rule.family = "any";
    rule.src = interface_name;
    rule.proto = "tcp";
    rule.dest_port = "22 80 443";
    rule.target = "ACCEPT";
    if ( firewall_rule_writer( p, &rule ) < 0 ) {
        return -1;
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_default_rule(
    struct uci_package *p,
    int wan_cnts,
    char *interface_name
)
//=============================================================================
{
    firewall_parse_zone_default_wan_rule( p, wan_cnts, interface_name );
    firewall_parse_zone_default_lan_rule( p, wan_cnts, interface_name );

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_auto_forward_vpn2wan_rule(
    struct uci_package *p,
    int wan_cnts,
    int *vpn_ids,
    int vpn_cnts,
    char *interface_name
)
//=============================================================================
{
    int i, j;
    char src[BUF_LEN_16] = { 0 };
    char dest[BUF_LEN_16] = { 0 };
    char name[BUF_LEN_64] = { 0 };

    // Set zone forward rule
    for ( i = 0; i < wan_cnts; i++ ) {
        for ( j = 0; j < vpn_cnts; j++ ) {
            // forward VPN to WAN
            snprintf( src, sizeof(src), "vpn%u", vpn_ids[j] );
            snprintf( dest, sizeof(dest), "wan%u", i );
            snprintf( name, sizeof(name), "Vpn%u-Forward-Auto-%s", vpn_ids[j], interface_name );
            auto_forward_rule.name = name,
            auto_forward_rule.src = src;
            auto_forward_rule.dest = dest;
            if ( firewall_rule_writer( p, &auto_forward_rule ) < 0 ) {
                return -1;
            }
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_auto_forward_lan2wan_rule(
    struct uci_package *p,
    int wan_cnts,
    int *vpn_ids,
    int vpn_cnts,
    char *interface_name
)
//=============================================================================
{
    int i;
    char src[BUF_LEN_16] = { 0 };
    char dest[BUF_LEN_16] = { 0 };
    char name[BUF_LEN_64] = { 0 };

    // Set zone forward rule
    for ( i = 0; i < wan_cnts; i++ ) {
        // forward LAN to WAN
        snprintf( src, sizeof(src), interface_name );
        snprintf( dest, sizeof(dest), "wan%u", i );
        snprintf( name, sizeof(name), "Default-Forward-Auto-%s", interface_name );
        auto_forward_rule.name = name,
        auto_forward_rule.src = src;
        auto_forward_rule.dest = dest;
        if ( firewall_rule_writer( p, &auto_forward_rule ) < 0 ) {
            return -1;
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_auto_forward_lan2vpn_rule(
    struct uci_package *p,
    int wan_cnts,
    int *vpn_ids,
    int vpn_cnts,
    char *interface_name
)
//=============================================================================
{
    int i;
    char src[BUF_LEN_16] = { 0 };
    char dest[BUF_LEN_16] = { 0 };
    char name[BUF_LEN_64] = { 0 };

    // forward LAN to VPN
    for ( i = 0; i < vpn_cnts; i++ ) {
        snprintf( src, sizeof(src), interface_name );
        snprintf( dest, sizeof(dest), "vpn%u", vpn_ids[i] );
        snprintf( name, sizeof(name), "Default-Forward-Auto-%s", interface_name );
        auto_forward_rule.name = name,
        auto_forward_rule.src = src;
        auto_forward_rule.dest = dest;
        if ( firewall_rule_writer( p, &auto_forward_rule ) < 0 ) {
            return -1;
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_auto_forward_rule(
    struct uci_package *p,
    int wan_cnts,
    int *vpn_ids,
    int vpn_cnts,
    char *interface_name
)
//=============================================================================
{
    firewall_parse_zone_auto_forward_vpn2wan_rule( p, wan_cnts, vpn_ids, vpn_cnts, DEFAULT_LAN );
    firewall_parse_zone_auto_forward_lan2wan_rule( p, wan_cnts, vpn_ids, vpn_cnts, DEFAULT_LAN );
    firewall_parse_zone_auto_forward_lan2vpn_rule( p, wan_cnts, vpn_ids, vpn_cnts, DEFAULT_LAN );

    return 0;
}

//=============================================================================
static int
firewall_parse_dynamic_ipset(
    struct uci_package *p,
    char *name
)
//=============================================================================
{
    int ret;
    struct uci_ptr ptr = { 0 };

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section( ctx, p, "ipset", &ptr.s );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section 'ipset' into '%s' failed, name %s!\n", FIREWALL_CONFIG_NAME, name );
        return -1;
    }

    uci_add_option( &ptr, "name", name );
    uci_add_option( &ptr, "storage", "hash" );
    uci_add_option( &ptr, "noreload", "1" );
    firewall_add_list( &ptr, NULL, "match", "ip" );
    firewall_add_list( &ptr, NULL, "match", "port" );

    return 0;
}

//=============================================================================
static int
firewall_parse_static_ipset(
    struct uci_package *p,
    char *name,
    struct static_dns *dns
)
//=============================================================================
{
    int ret;
    struct uci_ptr ptr = { 0 };
    char dns_entry[BUF_LEN_64] = { 0 };

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section( ctx, p, "ipset", &ptr.s );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section 'ipset' into '%s' failed, name %s!\n", FIREWALL_CONFIG_NAME, name );
        return -1;
    }

    uci_add_option( &ptr, "name", name );
    uci_add_option( &ptr, "storage", "hash" );

    firewall_add_list( &ptr, NULL, "match", "ip" );
    firewall_add_list( &ptr, NULL, "match", "port" );

    if ( dns && dns->valid ) {
        if ( dns->first ) {
            snprintf( dns_entry, sizeof( dns_entry ), "%s,udp:53", dns->first );
            firewall_add_list( &ptr, NULL, "entry", dns_entry );
        
            snprintf( dns_entry, sizeof( dns_entry ), "%s,tcp:53", dns->first );
            firewall_add_list( &ptr, NULL, "entry", dns_entry );
        }
        
        if ( dns->second ) {
            snprintf( dns_entry, sizeof( dns_entry ), "%s,udp:53", dns->second );
            firewall_add_list( &ptr, NULL, "entry", dns_entry );
        
            snprintf( dns_entry, sizeof( dns_entry ), "%s,tcp:53", dns->second );
            firewall_add_list( &ptr, NULL, "entry", dns_entry );
        }
    } 

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_lan(
    struct uci_package *p,
    char *interface_name
)
//=============================================================================
{
    int ret;
    struct uci_ptr ptr = { 0 };

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section( ctx, p, "zone", &ptr.s );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section '%s' into '%s' failed, name %s!\n", "zone",
            FIREWALL_CONFIG_NAME, interface_name );
        return -1;
    }
    uci_add_option( &ptr, "name", interface_name );
    firewall_add_list( &ptr, NULL, "network", interface_name );
    uci_add_option( &ptr, "input", "ACCEPT" );
    uci_add_option( &ptr, "output", "ACCEPT" );
    uci_add_option( &ptr, "forward", "ACCEPT" );
    uci_add_option( &ptr, "log_limit", "10/second" );

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_vlan(
    struct uci_package *p,
    struct uci_package *fw,
    int wan_cnts,
    int *vpn_ids,
    int vpn_cnts
)
//=============================================================================
{
    struct uci_element *e;
    char interface_name[BUF_LEN_64] = {0};

    uci_foreach_element( &p->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( !strcmp(s->type, "vlan") ) {
            const char *vlanid = uci_lookup_option_string( ctx, s, "vlanId" );

            if ( !vlanid || LAN_DEFAULT_VLAN_ID == atoi(vlanid) ) {
                continue;
            }

            snprintf( interface_name, BUF_LEN_64, "zone%s", vlanid );
            firewall_parse_zone_lan( fw, interface_name );
            firewall_parse_zone_auto_forward_lan2wan_rule( fw, wan_cnts, vpn_ids, vpn_cnts, interface_name );
            firewall_parse_zone_auto_forward_lan2vpn_rule( fw, wan_cnts, vpn_ids, vpn_cnts, interface_name );
            firewall_parse_zone_default_lan_rule( fw, wan_cnts, interface_name );
        }
    }

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_vpn(
    struct uci_package *p,
    char *name,
    int need_masq
)
//=============================================================================
{
    int ret;
    struct uci_ptr ptr = { 0 };

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section( ctx, p, "zone", &ptr.s );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section 'zone' into '%s' failed, name %s!\n", FIREWALL_CONFIG_NAME, name );
        return -1;
    }
    uci_add_option( &ptr, "name", name );
    firewall_add_list( &ptr, NULL, "network", name );
    uci_add_option( &ptr, "input", "ACCEPT" );
    uci_add_option( &ptr, "output", "ACCEPT" );
    uci_add_option( &ptr, "forward", "ACCEPT" );
    if ( need_masq ) {
        uci_add_option( &ptr, "masq", "1" );
    }
    uci_add_option( &ptr, "log_limit", "10/second" );

    return 0;
}

//=============================================================================
static int
firewall_parse_zone_wan(
    struct uci_package *p,
    char *name
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    int ret = 0;

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section( ctx, p, "zone", &ptr.s );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section 'zone' into '%s' failed, name %s!\n", FIREWALL_CONFIG_NAME, name );
        return -1;
    }

    uci_add_option( &ptr, "name", name );
    firewall_add_list( &ptr, NULL, "network", name );
    uci_add_option( &ptr, "input", "REJECT" );
    uci_add_option( &ptr, "output", "ACCEPT" );
    uci_add_option( &ptr, "forward", "REJECT" );
    uci_add_option( &ptr, "masq", "1" );
    uci_add_option( &ptr, "mtu_fix", "1" );
    uci_add_option( &ptr, "log_limit", "10/second" );

    return 0;
}

//=============================================================================
static int
firewall_parse_def_DOS(
    struct uci_package *cfgmanager_pkg,
    struct uci_package *firewall_pkg
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    int ret;

    memset( &ptr, 0, sizeof( ptr ) );
    ptr.p = firewall_pkg;
    ret = uci_add_section_named( ctx, firewall_pkg, "include", &ptr.s, "def_DOS" );
    if ( ret != UCI_OK )
    {
        cfmanager_log_message( L_ERR,
            "Add include section 'include' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
        return -1;
    }
    uci_add_option( &ptr, "enabled", "1" );
    uci_add_option( &ptr, "type", "script" );
    uci_add_option( &ptr, "path", "/usr/share/firewall/gs_def_DOS.sh" );
    uci_add_option( &ptr, "family", "any" );
    uci_add_option( &ptr, "reload", "1" );

    return 0;
}

//=============================================================================
static int
firewall_parse_global_ban(
    struct uci_package *p,
    char *name
)
//=============================================================================
{

    struct uci_ptr ptr = { 0 };
    int ret;

    memset( &ptr, 0, sizeof( ptr ) );
    ptr.p = p;
    ret = uci_add_section_named( ctx, p, "include", &ptr.s, name );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR,
            "Add include section into '%s' failed!\n", FIREWALL_CONFIG_NAME );
        return -1;
    }
    uci_add_option( &ptr, "path", "/usr/share/firewall/gs_global_ban.sh" );
    uci_add_option( &ptr, "reload", "1" );

    return 0;
}

//=============================================================================
static int
firewall_parse_default(
    struct uci_package *p
)
//=============================================================================
{
    struct uci_ptr ptr = { 0 };
    int ret;

    memset( &ptr, 0, sizeof ( ptr ) );
    ptr.p = p;
    ret = uci_add_section( ctx, p, "defaults", &ptr.s );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, 
            "Add section 'defaults' into '%s' failed!\n", FIREWALL_CONFIG_NAME );
        return -1;
    }
    uci_add_option( &ptr, "synflood_protect", "0" );
    uci_add_option( &ptr, "synflood_rate", "50/s" );
    uci_add_option( &ptr, "synflood_burst", "100" );
    uci_add_option( &ptr, "drop_invalid", "0" );
    uci_add_option( &ptr, "force_ct_flush", "0" );
    uci_add_option( &ptr, "input", "REJECT" );
    uci_add_option( &ptr, "output", "REJECT" );
    uci_add_option( &ptr, "forward", "REJECT" );

    return 0;
}

//=============================================================================
int
firewall_reparse(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct uci_package *cfgmanager = NULL, *fw = NULL, *network = NULL;
    struct static_dns wan_dns[WAN_CNT_MAX];
    struct static_dns vpn_dns[WAN_CNT_MAX];
    int vpn_ids[VPN_TOTAL_CNT];
    int vpn_need_masq[VPN_TOTAL_CNT];
    int wan_cnts = 1,  vpn_cnts = 0;
    int lan_enable_dhcp = 0;
    int wan_type[WAN_CNT_MAX] = { WAN_TYPE_INVALID };
    int i, ret;

    cfmanager_log_message( L_INFO, "Parse firewall...\n" );
    ctx = uci_alloc_context();
    if ( NULL == ctx ) {
        cfmanager_log_message( L_ERR, "Failed to connect to config\n" );
        return -1;
    }
    uci_set_confdir( ctx, CM_CONFIG_PATH );

    //Reload package, the config might have changed in the background
    cfgmanager = uci_lookup_package( ctx, CFMANAGER_CONFIG_NAME );
    if ( cfgmanager )
        uci_unload( ctx, cfgmanager );

    if ( uci_load( ctx, CFMANAGER_CONFIG_NAME, &cfgmanager ) ) {
        cfmanager_log_message( L_DEBUG,
            "Error loading %s\n", CFMANAGER_CONFIG_NAME );
        uci_free_context( ctx );
        return -1;
    }

    uci_clean_package( FIREWALL_CONFIG_PATH );
    fw = uci_lookup_package( ctx, FIREWALL_CONFIG_NAME );
    if ( fw ) {
        uci_unload( ctx, fw );
    }

    if ( uci_load( ctx, FIREWALL_CONFIG_NAME, &fw ) ) {
        cfmanager_log_message( L_ERR,
            "Init package '%s' failed\n", FIREWALL_CONFIG_NAME );
        uci_unload( ctx, cfgmanager );
        uci_free_context( ctx );
        return -1;
    }

    // Get wan static dns by network
    memset( &wan_dns, 0, sizeof(wan_dns) );
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
                        dns = &wan_dns[0];
                    }
                    else if ( !strcmp(s->e.name, "wan1") ) {
                        dns = &wan_dns[1];
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

    firewall_parse_default(fw);
    /* for defensing DOS attack*/
    firewall_parse_def_DOS( cfgmanager, fw );
    /* Global bans */
    firewall_parse_global_ban( fw, "global_bans" );

    /* WAN and VPN Zones */
    memset( &vpn_dns, 0, sizeof(vpn_dns) );
    memset( vpn_need_masq, 0x1, sizeof(vpn_need_masq) ); // All vpns need to masquerade as default.
    uci_foreach_element( &cfgmanager->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( !strcmp(s->type, "wan") && !strcmp(s->e.name, "wan") ) {
            const char *wan1_enable = NULL;
            const char *wan0_static_dns_enable = NULL;
            const char *wan1_static_dns_enable = NULL;

            wan_type[0] = atoi( uci_lookup_option_string(ctx, s, "wanType") );
            if ( wan_type[0] == WAN_TYPE_PPTP || wan_type[0] == WAN_TYPE_L2TP ) {
                if ( vpn_cnts >= VPN_TOTAL_CNT ) {
                    cfmanager_log_message( L_ERR, 
                        "Active vpn counts exceed limit!\n" );
                    
                }
                else {
                    vpn_ids[vpn_cnts++] = 0;
                }
            }

            wan1_enable = uci_lookup_option_string( ctx, s, "wan1Enable" );
            if ( wan1_enable && wan1_enable[0] == '1' ) {
                wan_cnts++;
                wan_type[1] = atoi( uci_lookup_option_string(ctx, s, "wan1Type") );
                if ( wan_type[1] == WAN_TYPE_PPTP || wan_type[1] == WAN_TYPE_L2TP ) {
                    if ( vpn_cnts >= VPN_TOTAL_CNT ) {
                        cfmanager_log_message( L_ERR, 
                            "Active vpn counts exceed limit!\n" );
                        
                    }
                    else {
                        vpn_ids[vpn_cnts++] = 1;
                    }
                }
            }

            wan0_static_dns_enable = uci_lookup_option_string( ctx, s, "wanStaticDnsEnable" );
            if ( wan0_static_dns_enable && wan0_static_dns_enable[0] == '1' ) {
                const char *wan0_first_dns = uci_lookup_option_string( ctx, s, "wanFirstDns" );
                const char *wan0_second_dns = uci_lookup_option_string( ctx, s, "wanSecondDns" );

                if ( wan_type[0] == WAN_TYPE_PPTP || wan_type[0] == WAN_TYPE_L2TP ) {
                    vpn_dns[0].valid = 1;
                    vpn_dns[0].first = wan0_first_dns;
                    vpn_dns[0].second = wan0_second_dns;
                }
                else {
                    wan_dns[0].valid = 1;
                    wan_dns[0].first = wan0_first_dns;
                    wan_dns[0].second = wan0_second_dns;
                }
            }

            wan1_static_dns_enable = uci_lookup_option_string( ctx, s, "wan1StaticDnsEnable");
            if ( wan1_static_dns_enable && wan1_static_dns_enable[0] == '1' ) {
                const char *wan1_first_dns = uci_lookup_option_string( ctx, s, "wan1FirstDns" );
                const char *wan1_second_dns = uci_lookup_option_string( ctx, s, "wan1SecondDns" );

                if ( wan_type[1] == WAN_TYPE_PPTP || wan_type[1] == WAN_TYPE_L2TP ) {
                    vpn_dns[1].valid = 1;
                    vpn_dns[1].first = wan1_first_dns;
                    vpn_dns[1].second = wan1_second_dns;
                }
                else {
                    wan_dns[1].valid = 1;
                    wan_dns[1].first = wan1_first_dns;
                    wan_dns[1].second = wan1_second_dns;
                }
            }
        }

        // vpn client
        if ( !strcmp(s->type, "vpn_service") ) {
            const char *enable = uci_lookup_option_string( ctx, s, "enable" );
            const char *type = uci_lookup_option_string( ctx, s, "type" );

            if ( enable[0] == '1' ) {
                if ( vpn_cnts >= VPN_TOTAL_CNT ) {
                    cfmanager_log_message( L_ERR, 
                        "Active vpn counts exceed limit!\n" );
                }
                else {
                    const char *id = uci_lookup_option_string( ctx, s, "id" );

                    if ( atoi(type) == VPN_CLIENT_TYPE_IPSEC ) {
                        // IPsec doesn't need to MASQUERADE.
                        vpn_need_masq[vpn_cnts] = 0;
                    }
                    vpn_ids[vpn_cnts++] = atoi(id) + VPN_CLIENT_ID_OFFSET;
                }
            }
        }

        // vpn server
        if ( !strcmp(s->type, "vpn_server") ) {
            const char *enable = uci_lookup_option_string( ctx, s, "enable" );
            const char *type = uci_lookup_option_string( ctx, s, "type" );

            if ( enable[0] == '1' ) {
                if ( vpn_cnts >= VPN_TOTAL_CNT ) {
                    cfmanager_log_message( L_ERR, 
                        "Active vpn counts exceed limit!\n" );
                }
                else {
                    const char *id = uci_lookup_option_string( ctx, s, "id" );

                    if ( atoi(type) == VPN_SERVER_TYPE_IPSEC ) {
                        // IPsec doesn't need to MASQUERADE.
                        vpn_need_masq[vpn_cnts] = 0;
                    }
                    vpn_ids[vpn_cnts++] = atoi(id) + VPN_SERVER_ID_OFFSET;
                }
            }
        }

        if ( !strcmp(s->type, "lan") && !strcmp(s->e.name, "lan") ) {
            const char *lan_enable_dhcp_str = uci_lookup_option_string( ctx, s, "dhcpEnable" );
             if ( lan_enable_dhcp_str && lan_enable_dhcp_str[0] == '1' ) {
                lan_enable_dhcp = 1;
            }
        }
    }

    // parse zone for wanx
    for ( i = 0; i < wan_cnts; i++ ) {
        char name[BUF_LEN_16] = { 0 };
        char ipset_name[BUF_LEN_64] = { 0 };

        snprintf( name, sizeof(name), "wan%u", i );
        firewall_parse_zone_wan( fw, name );

        snprintf( ipset_name, sizeof( ipset_name ), "wan%u_dns_ipset1", i );
        firewall_parse_static_ipset( fw, ipset_name, &wan_dns[i] );

        if ( wan_type[i] != WAN_TYPE_STATIC ) {
            snprintf( ipset_name, sizeof( ipset_name ), "wan%u_dns_ipset2", i );
            firewall_parse_dynamic_ipset( fw, ipset_name );
        }
    }

    // parse zone for vpnx
    for ( i = 0; i < vpn_cnts; i++ ) {
        char name[BUF_LEN_16] = { 0 };
        char ipset_name[BUF_LEN_64] = { 0 };

        snprintf( name, sizeof(name), "vpn%u", vpn_ids[i] );
        firewall_parse_zone_vpn( fw, name, vpn_need_masq[i] );

        snprintf( ipset_name, sizeof( ipset_name ), "vpn%u_dns_ipset1", vpn_ids[i] );
        // Only these vpns locate in config section 'wan' can have static dns server.
        if ( vpn_ids[i] == 0 ) {
            firewall_parse_static_ipset( fw, ipset_name, &vpn_dns[0] );
        }
        else if ( vpn_ids[i] == 1 ) {
            firewall_parse_static_ipset( fw, ipset_name, &vpn_dns[1] );
        }
        else {
            firewall_parse_static_ipset( fw, ipset_name, NULL );
        }
        snprintf( ipset_name, sizeof( ipset_name ), "vpn%u_dns_ipset2", vpn_ids[i] );
        firewall_parse_dynamic_ipset( fw, ipset_name );
    }

    firewall_parse_zone_lan( fw, DEFAULT_LAN );
    firewall_parse_zone_auto_forward_rule( fw, wan_cnts, vpn_ids, vpn_cnts, DEFAULT_LAN );
    firewall_parse_zone_default_rule( fw, wan_cnts, DEFAULT_LAN );
    firewall_parse_ipsec_rule( cfgmanager, fw );
    firewall_parse_zone_vlan( cfgmanager, fw, wan_cnts, vpn_ids, vpn_cnts );

    firewall_parse_nat_rule( cfgmanager, fw );
    firewall_parse_port_mapping_rule( cfgmanager, fw );
    firewall_parse_dmz_rule( cfgmanager, fw );
    firewall_parse_controller_rule( fw ); 
    firewall_parse_web_wan_access( fw );
    firewall_parse_upnp_rule( cfgmanager, fw ); 
    if ( lan_enable_dhcp ) {
        firewall_parse_dhcp_rule( fw );
    }
    firewall_parse_blackhole_rule( cfgmanager, fw );

    ret = uci_save( ctx, fw );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_DEBUG, "save %s failed\n", FIREWALL_CONFIG_NAME );
        goto err;
    }

    ret = uci_commit( ctx, &fw, false );
    if ( ret != UCI_OK  ) {
        cfmanager_log_message( L_DEBUG, "commit %s failed\n", FIREWALL_CONFIG_NAME );
        goto err;
    }

    uci_unload( ctx, cfgmanager );
    uci_unload( ctx, fw );
    if ( network ) {
        uci_unload( ctx, network );
    }

    uci_free_context( ctx );

    return 0;

err:

    uci_unload( ctx, cfgmanager );
    uci_unload( ctx, fw );
    if ( network ) {
        uci_unload( ctx, network );
    }

    uci_free_context( ctx );

    return -1;
}

//=============================================================================
int
firewall_delete_port_mapping_by_wan(
    char *wan
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    int should_commit = 0;

    vlist_for_each_element( &cm_port_mapping_vltree, cm_cfg, node ) {
        struct blob_attr *tb[__PORT_MAPPING_MAX];
        char *id;
        char *wan_intf;
        char section_nm[BUF_LEN_64] = { 0 };

        blobmsg_parse( port_mapping_policy,
            __PORT_MAPPING_MAX,
            tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        id = blobmsg_get_string( tb[PORT_MAPPING_ID] );
        snprintf( section_nm, sizeof(section_nm), "port_mapping%s", id );
        wan_intf = blobmsg_get_string( tb[PORT_MAPPING_INTF] );
        if ( !strcmp(wan_intf, wan) ) {
            cfmanager_log_message( L_WARNING, "Delete port mapping rule %s, because %s disabled!\n",
                section_nm, wan );
            config_del_named_section( CFMANAGER_CONFIG_NAME, "port_mapping", section_nm );
            should_commit = 1;
        }
    }

    if ( should_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_PORT_MAPPING, false );
    }

    return 0;
}

//=============================================================================
int
firewall_parse_port_mapping(
    struct blob_attr *attr,
    int *fw_need_load
)
//=============================================================================
{    
    struct blob_attr *new_tb[__PORT_MAPPING_MAX];
    struct blob_attr *old_tb[__PORT_MAPPING_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int i = 0;
    int same_cfg = 1;
    int ret = ERRCODE_SUCCESS;
    int change_option = 0;
    char section_nm[BUF_LEN_64] = { 0 };
    int action;

    blobmsg_parse( port_mapping_policy,
        __PORT_MAPPING_MAX,
        new_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    if ( !new_tb[PORT_MAPPING_ACTION] ) {
        cfmanager_log_message( L_ERR,
            "Missing %s in json!\n", port_mapping_policy[PORT_MAPPING_ACTION].name );
        return ERRCODE_PARAMETER_ERROR;
    }

    action = atoi( blobmsg_get_string(new_tb[PORT_MAPPING_ACTION]) );
    if ( action >= __PORT_MAPPING_ACTION_MAX ) {
        cfmanager_log_message( L_ERR,
            "Invalid action %d\n", action);
        return ERRCODE_PARAMETER_ERROR;
    }

    if ( action == PORT_MAPPING_ACTION_DELETE && !new_tb[PORT_MAPPING_ID] ) {
        cfmanager_log_message( L_ERR,
            "Missing %s in json!\n", port_mapping_policy[PORT_MAPPING_ID].name );
        return ERRCODE_PARAMETER_ERROR;
    }
    else if ( action == PORT_MAPPING_ACTION_ADD ) {
        for ( i = 0; i < __PORT_MAPPING_MAX; i++ ) {
            if ( i != PORT_MAPPING_EXT_PORT && i != PORT_MAPPING_EXT_IP && !new_tb[i] ) {
                cfmanager_log_message( L_ERR,
                    "Missing %s in json!\n", port_mapping_policy[i].name );
                return ERRCODE_PARAMETER_ERROR;
            }
        }
    }

    *fw_need_load = 0;
    snprintf( section_nm, sizeof(section_nm), "port_mapping%s",
        blobmsg_get_string( new_tb[PORT_MAPPING_ID] ) );
    cm_cfg = util_get_vltree_node( &cm_port_mapping_vltree, VLTREE_CM_TREE, section_nm );
    if ( !cm_cfg && action == PORT_MAPPING_ACTION_ADD ) {
        cfmanager_log_message( L_WARNING, "NOT found port mapping rule %s in %s, create it...\n", 
            section_nm, CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "port_mapping", section_nm );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section %s in %s\n", section_nm, CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __PORT_MAPPING_MAX; i++ ) {
            if ( i != PORT_MAPPING_ACTION ) {
                snprintf( path, sizeof( path ), "%s.%s.%s",
                    CFMANAGER_CONFIG_NAME, section_nm, port_mapping_policy[i].name );
                config_set_by_blob( new_tb[i], path, port_mapping_policy[i].type );
            }
        }

        same_cfg = 0;
        *fw_need_load = 1;
    }
    else if ( action == PORT_MAPPING_ACTION_ADD ) {
        cfmanager_log_message( L_WARNING, "Update port mapping rule %s in %s\n", 
            section_nm, CFMANAGER_CONFIG_NAME );

        blobmsg_parse( port_mapping_policy,
            __PORT_MAPPING_MAX,
            old_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __PORT_MAPPING_MAX; i++ ) {
            if ( !new_tb[i] || i == PORT_MAPPING_ACTION ) {
                continue;
            }

            if ( sgreq_compar_attr( new_tb[i], old_tb[i], port_mapping_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ), "%s.%s.%s",
                CFMANAGER_CONFIG_NAME, section_nm, port_mapping_policy[i].name );
            config_set_by_blob( new_tb[i], path, port_mapping_policy[i].type );

            same_cfg = 0;
            change_option |= BIT(i);
        }

        if ( change_option != BIT(PORT_MAPPING_NAME) ) {
            // If only name changed, we don't need to reload firewall
            *fw_need_load = 1;
        }
    }
    else if ( cm_cfg ) {
        cfmanager_log_message( L_WARNING, "Delete port mapping rule %s!\n", section_nm );
        config_del_named_section( CFMANAGER_CONFIG_NAME, "port_mapping", section_nm );
        same_cfg = 0;
        *fw_need_load = 1;
    }
    else {
        cfmanager_log_message( L_ERR, "Try to delete port mapping rule %s, but not found!\n", section_nm );
        ret = ERRCODE_PARAMETER_ERROR;
    }

    if ( same_cfg ) {
        cfmanager_log_message( L_DEBUG,
            "The port mapping rule %s config is same\n", section_nm );
    }

    return ret;
}

//=============================================================================
int
firewall_delete_dmz_by_wan(
    char *wan
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    int should_commit = 0;

    vlist_for_each_element( &cm_dmz_vltree, cm_cfg, node ) {
        struct blob_attr *tb[__DMZ_MAX];
        char *wan_intf;

        blobmsg_parse( dmz_policy,
            __DMZ_MAX,
            tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        wan_intf = blobmsg_get_string( tb[DMZ_INTF] );
        if ( !strcmp(wan_intf, wan) ) {
            cfmanager_log_message( L_WARNING, "Delete dmz, because %s disabled!\n",
                wan );
            config_del_named_section( CFMANAGER_CONFIG_NAME, "dmz", "dmz" );
            should_commit = 1;
        }
    }

    if ( should_commit ) {
        config_commit( CFMANAGER_CONFIG_NAME, false );
        cfparse_load_file( CFMANAGER_CONFIG_NAME, CM_VLTREE_DMZ, false );
    }

    return 0;
}

//=============================================================================
int
firewall_parse_dmz(
    struct blob_attr *attr,
    int *fw_need_load
)
//=============================================================================
{
    struct blob_attr *new_tb[__DMZ_MAX];
    struct blob_attr *old_tb[__DMZ_MAX];
    struct cm_config *cm_cfg = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    int i = 0;
    int same_cfg = 1;
    int ret = ERRCODE_SUCCESS;

    blobmsg_parse( dmz_policy,
        __DMZ_MAX,
        new_tb,
        blobmsg_data( attr ),
        blobmsg_len( attr ) );

    for ( i = 0; i < __DMZ_MAX; i++ ) {
        if ( !new_tb[i] ) {
            cfmanager_log_message( L_ERR,
                "Missing %s in json!\n", dmz_policy[i].name );
            return ERRCODE_PARAMETER_ERROR;
        }
    }

    *fw_need_load = 0;
    cm_cfg = util_get_vltree_node( &cm_dmz_vltree, VLTREE_CM_TREE, "dmz" );
    if ( !cm_cfg ) {
        cfmanager_log_message( L_WARNING, "NOT found dmz in %s, create it...\n", 
            CFMANAGER_CONFIG_NAME );
        ret = config_add_named_section( CFMANAGER_CONFIG_NAME, "dmz", "dmz" );
        if ( ret != 0 ) {
            cfmanager_log_message( L_ERR,
                "failed to add section dmz in %s\n", CFMANAGER_CONFIG_NAME );
            return ERRCODE_INTERNAL_ERROR;
        }

        for ( i = 0; i < __DMZ_MAX; i++ ) {
            snprintf( path, sizeof( path ), "%s.dmz.%s",
                CFMANAGER_CONFIG_NAME, dmz_policy[i].name );
            config_set_by_blob( new_tb[i], path, dmz_policy[i].type );
        }

        same_cfg = 0;
        *fw_need_load = 1;
    }
    else {
        cfmanager_log_message( L_WARNING, "Update dmz in %s\n", CFMANAGER_CONFIG_NAME );

        blobmsg_parse( dmz_policy,
            __DMZ_MAX,
            old_tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        for ( i = 0; i < __DMZ_MAX; i++ ) {
            if ( !new_tb[i] ) {
                continue;
            }

            if ( sgreq_compar_attr( new_tb[i], old_tb[i], dmz_policy[i].type ) ) {
                continue;
            }

            snprintf( path, sizeof( path ), "%s.dmz.%s",
                CFMANAGER_CONFIG_NAME, dmz_policy[i].name );
            config_set_by_blob( new_tb[i], path, dmz_policy[i].type );

            *fw_need_load = 1;
            same_cfg = 0;
        }
    }

    if ( same_cfg ) {
        cfmanager_log_message( L_DEBUG, "The dmz config is same\n" );
    }

    return ret;
}

//=============================================================================
int
cfparse_load_firewall(
    void
)
//=============================================================================
{
    firewall_reparse();

    /*
     * firewall does not need to be compared, it needs to be reset every time
     */
    apply_add( "firewall" );
    apply_timer_start();

    return 0;
}
