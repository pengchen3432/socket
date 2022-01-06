/****************************************************************************
* *
* * FILENAME:        $RCSfile: check.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/04/08
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
#include "global.h"
#include "utils.h"
#include "config.h"
#include "check.h"
#include "grandstream.h"
#include "controller.h"
#include "wireless.h"
#include "gs_utils.h"

//=================
//  Defines
//=================
enum {
    NOT_REBUILD,
    NEED_REBUILD,

    __REBUILD_MAX
};

//=================
//  Typedefs
//=================
struct check_cfg {
    const char *name;
    char rebuild_option;          //When cm restarts or restart the device, the configuration file needs to be regenerated every time
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
#if 0
static const struct check_cfg ck_cfg[__CHECK_MAX] = {
    /*  Bug 194271
     *  If the wireless is cleared,
     *  if other configurations that rely on the wireless configuration
     *  are generated before the wireless configuration by cfmanager, there will be problems,
     *  so the temporary avoidance scheme does not clear the wireless
    */
    [CHECK_WIRELESS]        = { .name = SPLICE_STR(CM_CONFIG_PATH,/wireless), .rebuild_option = NOT_REBUILD },
    [CHECK_DHCP]            = { .name = "/etc/config/dhcp", .rebuild_option = NEED_REBUILD },
    [CHECK_NETWORK]         = { .name = "/etc/config/network", .rebuild_option = NOT_REBUILD },
    [CHECK_QOS]             = { .name = "/etc/config/qos", .rebuild_option = NEED_REBUILD },
    [CHECK_SYSTEM]          = { .name = "/etc/config/system", .rebuild_option = NEED_REBUILD },
    [CHECK_FIREWALL]        = { .name = "/etc/config/firewall", .rebuild_option = NEED_REBUILD },
    [CHECK_SCHEDULE]        = { .name = "/etc/config/schedule", .rebuild_option = NEED_REBUILD },
    [CHECK_MWAN3]           = { .name = "/etc/config/mwan3", .rebuild_option = NEED_REBUILD },
    [CHECK_ECM]             = { .name = "/etc/config/ecm", .rebuild_option = NEED_REBUILD },
    [CHECK_DDNS]            = { .name = "/etc/config/ddns", .rebuild_option = NEED_REBUILD },
    [CHECK_QCACFG80211]     = { .name = "/etc/config/qcacfg80211", .rebuild_option = NEED_REBUILD },
    [CHECK_TRACKS]          = { .name = "/etc/config/tracks", .rebuild_option = NEED_REBUILD },
    [CHECK_FSTAB]           = { .name = "/etc/config/fstab", .rebuild_option = NEED_REBUILD },
    [CHECK_LBD]             = { .name = "/etc/config/lbd", .rebuild_option = NEED_REBUILD },
    [CHECK_IPSEC]           = { .name = "/etc/config/ipsec", .rebuild_option = NEED_REBUILD },
    [CHECK_UPNPD]           = { .name = "/etc/config/upnpd", .rebuild_option = NEED_REBUILD },
    [CHECK_SSID_STEERING]   = { .name = "/etc/config/ssid-steering", .rebuild_option = NEED_REBUILD },
    [CHECK_DROPBEAR]        = { .name = "/etc/config/dropbear", .rebuild_option = NEED_REBUILD },
    [CHECK_HYD]             = { .name = "/etc/config/hyd", .rebuild_option = NEED_REBUILD },
    [CHECK_NSS]             = { .name = "/etc/config/nss", .rebuild_option = NEED_REBUILD },
    [CHECK_RPCD]            = { .name = "/etc/config/rpcd", .rebuild_option = NEED_REBUILD },
    [CHECK_GRANDSTREAM]     = { .name = "/etc/config/grandstream", .rebuild_option = NOT_REBUILD },
    [CHECK_CONTROLLER]      = { .name = "/etc/config/controller", .rebuild_option = NOT_REBUILD },
    [CHECK_BWCTRL]          = { .name = "/etc/config/bwctrl", .rebuild_option = NEED_REBUILD },
    [CHECK_CFMANAGER]       = { .name = "/etc/config/cfmanager", .rebuild_option = NEED_REBUILD },
};
#endif

static const struct check_cfg ck_cfg[__CHECK_MAX] = {
    /*  Bug 194271
     *  If the wireless is cleared,
     *  if other configurations that rely on the wireless configuration
     *  are generated before the wireless configuration by cfmanager, there will be problems,
     *  so the temporary avoidance scheme does not clear the wireless
    */
    [CHECK_WIRELESS]        = { .name = SPLICE_STR(CM_CONFIG_PATH,/wireless), .rebuild_option = NOT_REBUILD },
    [CHECK_DHCP]            = { .name = SPLICE_STR(CM_CONFIG_PATH,/dhcp), .rebuild_option = NEED_REBUILD },
    [CHECK_NETWORK]         = { .name = SPLICE_STR(CM_CONFIG_PATH,/network), .rebuild_option = NOT_REBUILD },
    [CHECK_QOS]             = { .name = SPLICE_STR(CM_CONFIG_PATH,/qos), .rebuild_option = NEED_REBUILD },
    [CHECK_SYSTEM]          = { .name = SPLICE_STR(CM_CONFIG_PATH,/system), .rebuild_option = NEED_REBUILD },
    [CHECK_EMAIL]           = { .name = SPLICE_STR(CM_CONFIG_PATH,/email), .rebuild_option = NEED_REBUILD },
    [CHECK_NOTIFICATION]    = { .name = SPLICE_STR(CM_CONFIG_PATH,/notification), .rebuild_option = NEED_REBUILD },
    [CHECK_FIREWALL]        = { .name = SPLICE_STR(CM_CONFIG_PATH,/firewall), .rebuild_option = NEED_REBUILD },
    [CHECK_SCHEDULE]        = { .name = SPLICE_STR(CM_CONFIG_PATH,/schedule), .rebuild_option = NEED_REBUILD },
    [CHECK_MWAN3]           = { .name = SPLICE_STR(CM_CONFIG_PATH,/mwan3), .rebuild_option = NEED_REBUILD },
    [CHECK_ECM]             = { .name = SPLICE_STR(CM_CONFIG_PATH,/ecm), .rebuild_option = NEED_REBUILD },
    [CHECK_DDNS]            = { .name = SPLICE_STR(CM_CONFIG_PATH,/ddns), .rebuild_option = NEED_REBUILD },
    [CHECK_QCACFG80211]     = { .name = SPLICE_STR(CM_CONFIG_PATH,/qcacfg80211), .rebuild_option = NEED_REBUILD },
    [CHECK_TRACKS]          = { .name = SPLICE_STR(CM_CONFIG_PATH,/tracks), .rebuild_option = NEED_REBUILD },
    [CHECK_FSTAB]           = { .name = SPLICE_STR(CM_CONFIG_PATH,/fstab), .rebuild_option = NEED_REBUILD },
    [CHECK_LBD]             = { .name = SPLICE_STR(CM_CONFIG_PATH,/lbd), .rebuild_option = NEED_REBUILD },
    [CHECK_IPSEC]           = { .name = SPLICE_STR(CM_CONFIG_PATH,/ipsec), .rebuild_option = NEED_REBUILD },
    [CHECK_UPNPD]           = { .name = SPLICE_STR(CM_CONFIG_PATH,/upnpd), .rebuild_option = NEED_REBUILD },
    [CHECK_SSID_STEERING]   = { .name = SPLICE_STR(CM_CONFIG_PATH,/ssid-steering), .rebuild_option = NEED_REBUILD },
    [CHECK_DROPBEAR]        = { .name = SPLICE_STR(CM_CONFIG_PATH,/dropbear), .rebuild_option = NEED_REBUILD },
    [CHECK_HYD]             = { .name = SPLICE_STR(CM_CONFIG_PATH,/hyd), .rebuild_option = NEED_REBUILD },
    [CHECK_NSS]             = { .name = SPLICE_STR(CM_CONFIG_PATH,/nss), .rebuild_option = NEED_REBUILD },
    [CHECK_RPCD]            = { .name = SPLICE_STR(CM_CONFIG_PATH,/rpcd), .rebuild_option = NEED_REBUILD },
    [CHECK_GRANDSTREAM]     = { .name = SPLICE_STR(CM_CONFIG_PATH,/grandstream), .rebuild_option = NOT_REBUILD },
    [CHECK_CONTROLLER]      = { .name = SPLICE_STR(CM_CONFIG_PATH,/controller), .rebuild_option = NOT_REBUILD },
    [CHECK_BWCTRL]          = { .name = SPLICE_STR(CM_CONFIG_PATH,/bwctrl), .rebuild_option = NEED_REBUILD },
    [CHECK_CFMANAGER]       = { .name = SPLICE_STR(CM_CONFIG_PATH,/cfmanager), .rebuild_option = NEED_REBUILD },
    [CHECK_SAMBA]           = { .name = SPLICE_STR(CM_CONFIG_PATH,/samba), .rebuild_option = NEED_REBUILD },
    [CHECK_PORTALCFG]       = { .name = SPLICE_STR(CM_CONFIG_PATH,/gsportalcfg), .rebuild_option = NEED_REBUILD },
    [CHECK_TR069]           = { .name = SPLICE_STR(CM_CONFIG_PATH,/tr069), .rebuild_option = NEED_REBUILD },
    [CHECK_CLIENTBRIDGE]    = { .name = SPLICE_STR(CM_CONFIG_PATH,/clientbridge), .rebuild_option = NEED_REBUILD },
};

static int check_flag = 0;
//=================
//  Functions
//=================

//==========================================================================
static int
check_create_cfg(
    const char *name,
    int pos
)
//==========================================================================
{
    int fd;

    if( NULL == name ) {
        return -1;
    }

    if( check_flag & BIT( pos ) ) {
        return 0;
    }

    fd = open( name,
          O_RDWR | O_CREAT,
          S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP );

    if( 0 > fd ) {
        cfmanager_log_message( L_ERR, "create %s failed\n", name );
        return -1;
    }

    cfmanager_log_message( L_DEBUG, "create %s\n", name );

    if( fd ) {
        close( fd );
    }

    return 0;
}

//==========================================================================
static void
check_set_wireless_defvalue(
    char *section
)
//==========================================================================
{
    char path[LOOKUP_STR_SIZE];

    if( NULL == section ) {
        return;
    }

    config_add_named_section( "wireless", "wifi-device", section );
    snprintf( path, sizeof( path ), "wireless.%s.phy", section );
    config_set_option_not_exist( path, section, 0 );
    snprintf( path, sizeof( path ), "wireless.%s.type", section );
#ifdef CONFIG_MTK
    if( 0 == strcmp( "wifi0", section ) )
        config_set_option_not_exist( path, "mt7615e2", 0 );
    else
        config_set_option_not_exist( path, "mt7615e5", 0 );
#else
    config_set_option_not_exist( path, "qcawificfg80211", 0 );
#endif
    snprintf( path, sizeof( path ), "wireless.%s.pureg", section );
    config_set_option_not_exist( path, "1", 0 );
    snprintf( path, sizeof( path ), "wireless.%s.shortgi", section );
    config_set_option_not_exist( path, "1", 0 );
    snprintf( path, sizeof( path ), "wireless.%s.txchainmask", section );
    config_set_option_not_exist( path, "3", 0 );
    snprintf( path, sizeof( path ), "wireless.%s.rxchainmask", section );
    config_set_option_not_exist( path, "3", 0 );
    snprintf( path, sizeof( path ), "wireless.%s.dbdc_enable", section );
    config_set_option_not_exist( path, "0", 0 );
    // TODO:If support closing WiFi , please delete "disabled"
    snprintf( path, sizeof( path ), "wireless.%s.disabled", section );
    config_set_option_not_exist( path, "0", 0 );
}

//==========================================================================
static void
check_set_wireless_glob_defvalue(
    void
)
//==========================================================================
{
    char temp[BUF_LEN_8] = { 0 };

    config_uci_get_option( "wireless.qcawifi.country", temp, sizeof( temp ) );
    if( '\0' != temp[0] ) {
        return;
    }

    config_add_named_section( "wireless", "qcawifi", "qcawifi" );

    snprintf( temp, sizeof( temp ), "%d", COUNTRY_DEFVALUE );
    config_set_option_not_exist( "wireless.qcawifi.country", temp, 0 );
}

//==========================================================================
static void
check_set_system_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "system", "system", "system" );
    config_set_option_not_exist( "system.system.hostname", "Grandstream", 0 );
    config_set_option_not_exist( "system.system.log_level", "7", 0 );
    config_set_option_not_exist( "system.system.log_file", "/var/log/syslog.log", 0 );
    config_set_option_not_exist( "system.system.log_size", "128", 0 );

    config_add_named_section( "system", "timeserver", "ntp" );
    config_set_option_not_exist( "system.ntp.enabled", "1", 0 );
    config_set_option_not_exist( "system.ntp.enable_server", "0", 0 );
    //The list type is deleted first and then added
    config_uci_del( "system.ntp.server", 0 );
    config_uci_add_list( "system.ntp.server", "0.pool.ntp.org", 0 );
    config_uci_add_list( "system.ntp.server", "1.pool.ntp.org", 0 );
    config_uci_add_list( "system.ntp.server", "2.pool.ntp.org", 0 );
    config_uci_add_list( "system.ntp.server", "3.pool.ntp.org", 0 );
}

//==========================================================================
static void
check_set_email_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "email", "email", "email" );
}

//==========================================================================
static void
check_set_notification_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "notification", "notification", "notification" );

    config_set_option_not_exist( "notification.notification.notify_memory_usage", "0" , 0 );
    config_set_option_not_exist( "notification.notification.notify_ap_throughput", "0" , 0 );
    config_set_option_not_exist( "notification.notification.notify_ssid_throughput", "0" , 0 );
    config_set_option_not_exist( "notification.notification.notify_password_change", "0" , 0 );
    config_set_option_not_exist( "notification.notification.notify_firmware_upgrade", "0" , 0 );
    config_set_option_not_exist( "notification.notification.notify_ap_offline", "0" , 0 );
    config_set_option_not_exist( "notification.notification.notify_find_rogueap", "0" , 0 );
}

//==========================================================================
static void
check_set_dhcp_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "dhcp", "dnsmasq", "dnsmasq" );
    config_set_option_not_exist( "dhcp.dnsmasq.domainneeded", "1", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.boguspriv", "1", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.filterwin2k", "0", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.localise_queries", "1", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.hijack_protection", "1", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.rebind_localhost", "1", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.local", "/lan/", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.domain", "lan", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.expandhosts", "1", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.nonegcache", "0", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.authoritative", "1", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.readethers", "1", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.leasefile", "/tmp/dhcp.leases", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.resolvfile", "/tmp/resolv.conf.auto", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.localservice", "1", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.conntrack", "0", 0 );
    config_set_option_not_exist( "dhcp.dnsmasq.rebind_protection", "0", 0 );

    config_add_named_section( "dhcp", "dhcp", "lan0_zone0" );
    config_set_option_not_exist( "dhcp.lan0_zone0.interface", "lan0_zone0", 0 );
    config_set_option_not_exist( "dhcp.lan0_zone0.force", "1", 0 );

    if( MODE_ROUTER == device_info.work_mode ) {
        config_add_named_section( "dhcp", "dhcp", "wan0" );
        config_set_option_not_exist( "dhcp.wan0.interface", "wan0", 0 );
        config_set_option_not_exist( "dhcp.wan0.ignore", "1", 0 );
    }
}

//==========================================================================
static void
check_set_network_defvalue(
    void
)
//==========================================================================
{
#ifndef SINGLE_WAN
    char path[LOOKUP_STR_SIZE];
    char temp[BUF_LEN_16];
#endif

    config_add_named_section( "network", "interface", "loopback" );
    config_set_option_not_exist( "network.loopback.ifname", "lo" , 0 );
    config_set_option_not_exist( "network.loopback.proto", "static" , 0 );
    config_set_option_not_exist( "network.loopback.ipaddr", "127.0.0.1" , 0 );
    config_set_option_not_exist( "network.loopback.netmask", "255.0.0.0" , 0 );

#ifndef GWN7052
    config_add_named_section( "network", "interface", "owntraffic" );
    config_set_option_not_exist( "network.owntraffic.ifname", "dummy0" , 0 );
    config_set_option_not_exist( "network.owntraffic.proto", "static" , 0 );
    config_set_option_not_exist( "network.owntraffic.ipaddr", "192.0.2.0" , 0 );
    config_set_option_not_exist( "network.owntraffic.netmask", "255.255.255.255" , 0 );
#endif

    config_add_named_section( "network", "globals", "globals" );
    config_set_option_not_exist( "network.globals.ula_prefix", "auto" , 0 );

    config_add_named_section( "network", "switch", "switch0" );
    config_set_option_not_exist( "network.switch0.name", "switch0" , 0 );
    config_set_option_not_exist( "network.switch0.reset", "1" , 0 );
    config_set_option_not_exist( "network.switch0.enable_vlan", "0" , 0 );

    config_add_named_section( "network", "interface", "lan0_zone0" );
    config_set_option_not_exist( "network.lan0_zone0.type", "bridge" , 0 );
    config_set_option_not_exist( "network.lan0_zone0.force_link", "1" , 0 );
    if( MODE_ROUTER == device_info.work_mode ) {
        config_set_option_not_exist( "network.lan0_zone0.proto", "static" , 0 );

#ifdef SINGLE_WAN
        config_set_option_not_exist( "network.lan0_zone0.ifname", ROUTER_SINGLEWAN_LAN_IFNAME , 0 );
#else
        snprintf( path, sizeof( path ), "%s.wan.wan1Enable", CFMANAGER_CONFIG_NAME );
        memset( temp, 0, sizeof( temp ) );
        config_uci_get_option( path, temp, sizeof( temp ) );
        if( '1' == temp[0] ) {
            config_set_option_not_exist( "network.lan0_zone0.ifname", ROUTER_DOUBWAN_LAN_IFNAME , 0 );
        }
        else if( '0' == temp[0] ) {
            config_set_option_not_exist( "network.lan0_zone0.ifname", ROUTER_SINGLEWAN_LAN_IFNAME , 0 );
        }
        else {
             cfmanager_log_message( L_DEBUG, "error wan1Enable status :%s\n ", temp );
        }
#endif
    }
    else {
        config_set_option_not_exist( "network.lan0_zone0.proto", "dhcp" , 0 );
        config_set_option_not_exist( "network.lan0_zone0.ifname", AP_LAN_IFNAME , 0 );
    }

    config_set_option_not_exist( "network.lan0_zone0.multicast_querier", "0" , 0 );
    config_set_option_not_exist( "network.lan0_zone0.igmp_snooping", "0" , 0 );
    config_set_option_not_exist( "network.lan0_zone0.ieee1905managed", "1" , 0 );

    //Wan0 is not allowed to delete at present
    if( MODE_ROUTER == device_info.work_mode ) {
        config_add_named_section( "network", "interface", "wan0" );
        config_set_option_not_exist( "network.wan0.ifname", WAN_IFNAME , 0 );
        config_set_option_not_exist( "network.wan0.metric", "40" , 0 );
        config_set_option_not_exist( "network.wan0.force_link", "0" , 0 );
        config_set_option_not_exist( "network.wan0.peerdns", "1" , 0 );
    }
}

//==========================================================================
static void
check_set_qos_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "qos", "qos", "qos" );
    config_set_option_not_exist( "qos.qos.enabled", "0" , 0 );

    config_add_named_section( "qos", "qos_sqm", "sqm_wan0" );

    config_add_named_section( "qos", "qos_instance", "qos_instancewan0" );
    config_set_option_not_exist( "qos.qos_instancewan0.qos_type", "sqm" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan0.interface", "wan0" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan0.sqm_script", "simple.qos" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan0.upstream_bandwidth_unit", "Kbit" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan0.downstream_bandwidth_unit", "Kbit" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan0.upstream_qos_enabled", "0" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan0.downstream_qos_enabled", "0" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan0.sqm_qdisc", "fq_codel" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan0.sqm_linklayer", "none" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan0.sqm_qdisc_advanced", "0" , 0 );

    config_add_named_section( "qos", "qos_sqm", "sqm_wan1" );

    config_add_named_section( "qos", "qos_instance", "qos_instancewan1" );
    config_set_option_not_exist( "qos.qos_instancewan1.qos_type", "sqm" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan1.interface", "wan1" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan1.sqm_script", "simple.qos" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan1.upstream_bandwidth_unit", "Kbit" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan1.downstream_bandwidth_unit", "Kbit" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan1.upstream_qos_enabled", "0" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan1.downstream_qos_enabled", "0" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan1.sqm_qdisc", "fq_codel" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan1.sqm_linklayer", "none" , 0 );
    config_set_option_not_exist( "qos.qos_instancewan1.sqm_qdisc_advanced", "0" , 0 );
}

//==========================================================================
static void
check_set_ecm_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "ecm", "ecm", "global" );
    config_set_option_not_exist( "ecm.global.acceleration_engine", "auto" , 0 );
}

#if 0
//==========================================================================
static void
check_set_ddns_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "ddns", "service", "myddns" );
    config_set_option_not_exist( "ddns.myddns.enabled", "0" , 0 );
    config_set_option_not_exist( "ddns.myddns.interface", "wan" , 0 );
    config_set_option_not_exist( "ddns.myddns.use_syslog", "1" , 0 );
    config_set_option_not_exist( "ddns.myddns.service_name", "dyndns.org" , 0 );
    config_set_option_not_exist( "ddns.myddns.domain", "mypersonaldomain.dyndns.org" , 0 );
    config_set_option_not_exist( "ddns.myddns.username", "myusername" , 0 );
    config_set_option_not_exist( "ddns.myddns.password", "mypassword" , 0 );
    config_set_option_not_exist( "ddns.myddns.use_https", "0" , 0 );
    config_set_option_not_exist( "ddns.myddns.force_interval", "72" , 0 );
    config_set_option_not_exist( "ddns.myddns.force_unit", "hours" , 0 );
    config_set_option_not_exist( "ddns.myddns.check_interval", "10" , 0 );
    config_set_option_not_exist( "ddns.myddns.check_unit", "minutes" , 0 );
    config_set_option_not_exist( "ddns.myddns.retry_interval", "60" , 0 );
    config_set_option_not_exist( "ddns.myddns.retry_unit", "seconds" , 0 );
    config_set_option_not_exist( "ddns.myddns.ip_source", "web" , 0 );
    config_set_option_not_exist( "ddns.myddns.ip_url", "http://checkip.dyndns.com/" , 0 );
}
#endif

//==========================================================================
void
check_set_qcacfg80211_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "qcacfg80211", "qcacfg80211", "config" );
    config_set_option_not_exist( "qcacfg80211.config.enable", "1" , 0 );
}

//==========================================================================
static void
check_set_tracks_defvalue(
    void
)
//==========================================================================
{
    /* Do not leave spaces between multiple execution commands
       for example:
       cmd_1&&cmd_2---------->true
       cmd_1 && cmd_2-------->false
       This is not because of the wrong format.
       It's just a little easier to get rid of repetitive commands
    */
    config_add_named_section( "tracks", "network", "network" );
    config_set_option_not_exist( "tracks.network.init", "network" , 0 );
    /* network is handled separately, too many services affected
    */
    //config_uci_del( "tracks.network.affects", 0 );
    //config_uci_add_list( "tracks.network.affects", "dhcp", 0 );
    //config_uci_add_list( "tracks.network.affects", "controller", 0 );

    config_add_named_section( "tracks", "wireless", "wireless" );
    config_set_option_not_exist( "tracks.wireless.exec", "/sbin/wifi reload_legacy&&/etc/init.d/lbd reload", 0 );
    config_uci_del( "tracks.wireless.affects", 0 );
    config_uci_add_list( "tracks.wireless.affects", "wifi_isolate", 0 );
    config_uci_add_list( "tracks.wireless.affects", "samba", 0 );

    config_add_named_section( "tracks", "firewall", "firewall" );
    config_set_option_not_exist( "tracks.firewall.init", "firewall", 0 );
    config_uci_del( "tracks.firewall.affects", 0 );
    config_uci_add_list( "tracks.firewall.affects", "qos", 0 );
    config_uci_add_list( "tracks.firewall.affects", "bwctrl", 0 );

    config_add_named_section( "tracks", "dhcp", "dhcp" );
    config_set_option_not_exist( "tracks.dhcp.init", "dnsmasq" , 0 );
    config_set_option_not_exist( "tracks.dhcp.exec", "/usr/bin/refill_vpn_ipset.sh", 0 );
    //Refine the operation, restart WiFi only when the DHCP address pool changes
    //config_uci_del( "tracks.dhcp.affects", 0 );
    //config_uci_add_list( "tracks.dhcp.affects", "wireless", 0 );

    config_add_named_section( "tracks", "system", "system" );
    config_set_option_not_exist( "tracks.system.init", "system" , 0 );
    config_set_option_not_exist( "tracks.system.exec", "/etc/init.d/sysntpd restart&&/etc/init.d/log reload" , 0 );

    config_add_named_section( "tracks", "schedule", "schedule" );
    config_set_option_not_exist( "tracks.schedule.init", "sch", 0 );

    config_add_named_section( "tracks", "qos", "qos" );
    config_set_option_not_exist( "tracks.qos.init", "qos", 0 );
    // QoS donesn't need to reload mwan3.
    //config_uci_add_list( "tracks.qos.affects", "mwan3", 0 );

    config_add_named_section( "tracks", "blackhole", "blackhole" );
    config_set_option_not_exist( "tracks.blackhole.init", "blackhole", 0 );

    config_add_named_section( "tracks", "mwan3", "mwan3" );
    config_set_option_not_exist( "tracks.mwan3.init", "mwan3" , 0 );

    config_add_named_section( "tracks", "controller", "controller" );
    config_set_option_not_exist( "tracks.controller.init", "controller" , 0 );
    config_set_option_not_exist( "tracks.controller.exec", "/usr/sbin/mode_change", 0 );

    config_add_named_section( "tracks", "upnpd", "upnpd" );
    config_set_option_not_exist( "tracks.upnpd.init", "miniupnpd", 0 );

    config_add_named_section( "tracks", "ddns", "ddns" );
    config_set_option_not_exist( "tracks.ddns.init", "ddns", 0 );

    config_add_named_section( "tracks", "grandstream", "grandstream" );
    config_uci_add_list( "tracks.grandstream.affects", "controller", 0 );

    config_add_named_section( "tracks", "bwctrl", "bwctrl" );
    config_set_option_not_exist( "tracks.bwctrl.init", "bwctrl", 0 );

    config_add_named_section( "tracks", "wifi_isolate", "wifi_isolate" );
    config_set_option_not_exist( "tracks.wifi_isolate.exec", "/usr/sbin/wifi_isolate", 0 );

    config_add_named_section("tracks", "gsportalcfg", "gsportalcfg");
    config_set_option_not_exist( "tracks.gsportalcfg.init", "gsportalcfg", 0 );

    config_add_named_section("tracks", "portalfcgi", "portalfcgi");
    config_set_option_not_exist( "tracks.portalfcgi.init", "portalfcgi", 0 );

    config_add_named_section("tracks", "samba", "samba");
    config_set_option_not_exist( "tracks.samba.init", "samba", 0 );
    config_set_option_not_exist( "tracks.samba.exec", "smb.sh", 0 );

    config_add_named_section("tracks", "tr069", "tr069");
    config_set_option_not_exist( "tracks.tr069.init", "gstr069", 0 );

    config_add_named_section("tracks", "ipsec", "ipsec");
    config_set_option_not_exist( "tracks.ipsec.init", "ipsec", 0 );

    config_add_named_section( "tracks", "ecm", "ecm" );
    config_set_option_not_exist( "tracks.ecm.init", "qca-nss-ecm" , 0 );

    config_add_named_section( "tracks", "email", "email" );
    config_set_option_not_exist( "tracks.email.init", "notification" , 0 );

    config_add_named_section( "tracks", "notification", "notification" );
    config_set_option_not_exist( "tracks.notification.init", "notification" , 0 );
}

//==========================================================================
static void
check_set_fstab_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "fstab", "global", "global" );
    config_set_option_not_exist( "fstab.global.anon_swap", "0" , 0 );
    config_set_option_not_exist( "fstab.global.anon_mount", "0" , 0 );
    config_set_option_not_exist( "fstab.global.auto_swap", "1" , 0 );
    config_set_option_not_exist( "fstab.global.auto_mount", "1" , 0 );
    config_set_option_not_exist( "fstab.global.delay_root", "5" , 0 );
    config_set_option_not_exist( "fstab.global.check_fs", "0" , 0 );
}

//==========================================================================
static void
check_set_lbd_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "lbd", "config", "config" );
    config_set_option_not_exist( "lbd.config.Enable", "1" , 0 );
    config_uci_add_list( "lbd.config.MatchingSSID", "", 0 );
    config_set_option_not_exist( "lbd.config.ForceWextMode", "0" , 0 );
    config_set_option_not_exist( "lbd.config.PHYBasedPrioritization", "1" , 0 );
    config_set_option_not_exist( "lbd.config.BlacklistOtherESS", "0" , 0 );
    config_set_option_not_exist( "lbd.config.InactDetectionFromTx", "0" , 0 );
    config_set_option_not_exist( "lbd.config.WlanSteerMulticast", "0" , 0 );
    config_set_option_not_exist( "lbd.config.ClientClassificationEnable", "0" , 0 );
    config_set_option_not_exist( "lbd.config.EnableAckRSSI", "0" , 0 );
    config_set_option_not_exist( "lbd.config.LogModCounter", "10" , 0 );

    config_add_named_section( "lbd", "IdleSteer", "IdleSteer" );
    config_set_option_not_exist( "lbd.IdleSteer.RSSISteeringPoint_DG", "5" , 0 );
    config_set_option_not_exist( "lbd.IdleSteer.RSSISteeringPoint_UG", "20" , 0 );
    config_set_option_not_exist( "lbd.IdleSteer.NormalInactTimeout", "10" , 0 );
    config_set_option_not_exist( "lbd.IdleSteer.OverloadInactTimeout", "10" , 0 );
    config_set_option_not_exist( "lbd.IdleSteer.InactCheckInterval", "1" , 0 );
    config_set_option_not_exist( "lbd.IdleSteer.AuthAllow", "0" , 0 );

    config_add_named_section( "lbd", "ActiveSteer", "ActiveSteer" );
    config_set_option_not_exist( "lbd.ActiveSteer.TxRateXingThreshold_UG", "50000" , 0 );
    config_set_option_not_exist( "lbd.ActiveSteer.RateRSSIXingThreshold_UG", "30" , 0 );
    config_set_option_not_exist( "lbd.ActiveSteer.TxRateXingThreshold_DG", "6000" , 0 );
    config_set_option_not_exist( "lbd.ActiveSteer.RateRSSIXingThreshold_DG", "0" , 0 );

    config_add_named_section( "lbd", "Offload", "Offload" );
    config_set_option_not_exist( "lbd.Offload.MUAvgPeriod", "60" , 0 );
    config_set_option_not_exist( "lbd.Offload.MUOverloadThreshold_W2", "0" , 0 );
    config_set_option_not_exist( "lbd.Offload.MUOverloadThreshold_W5", "70" , 0 );
    config_set_option_not_exist( "lbd.Offload.MUSafetyThreshold_W2", "0" , 0 );
    config_set_option_not_exist( "lbd.Offload.MUSafetyThreshold_W5", "60" , 0 );
    config_set_option_not_exist( "lbd.Offload.OffloadingMinRSSI", "20" , 0 );

    config_add_named_section( "lbd", "IAS", "IAS" );
    config_set_option_not_exist( "lbd.IAS.Enable_W2", "0" , 0 );
    config_set_option_not_exist( "lbd.IAS.Enable_W5", "0" , 0 );
    config_set_option_not_exist( "lbd.IAS.MaxPollutionTime", "1200" , 0 );
    config_set_option_not_exist( "lbd.IAS.UseBestEffort", "0" , 0 );

    config_add_named_section( "lbd", "StaDB", "StaDB" );
    config_set_option_not_exist( "lbd.StaDB.IncludeOutOfNetwork", "1" , 0 );
    config_set_option_not_exist( "lbd.StaDB.TrackRemoteAssoc", "1" , 0 );
    config_set_option_not_exist( "lbd.StaDB.MarkAdvClientAsDualBand", "0" , 0 );

    config_add_named_section( "lbd", "SteerExec", "SteerExec" );
    config_set_option_not_exist( "lbd.SteerExec.SteeringProhibitTime", "300" , 0 );
    config_set_option_not_exist( "lbd.SteerExec.BTMSteeringProhibitShortTime", "30" , 0 );

    config_add_named_section( "lbd", "APSteer", "APSteer" );
    config_set_option_not_exist( "lbd.APSteer.DisableSteeringInactiveLegacyClients", "1" , 0 );
    config_set_option_not_exist( "lbd.APSteer.DisableSteeringActiveLegacyClients", "1" , 0 );
    config_set_option_not_exist( "lbd.APSteer.DisableSteering11kUnfriendlyClients", "1" , 0 );
    config_set_option_not_exist( "lbd.APSteer.LowRSSIAPSteerThreshold_SIG", "17" , 0 );
    config_set_option_not_exist( "lbd.APSteer.LowRSSIAPSteerThreshold_CAP", "20" , 0 );
    config_set_option_not_exist( "lbd.APSteer.LowRSSIAPSteerThreshold_RE", "45" , 0 );
    config_set_option_not_exist( "lbd.APSteer.APSteerToRootMinRSSIIncThreshold", "5" , 0 );
    config_set_option_not_exist( "lbd.APSteer.APSteerToLeafMinRSSIIncThreshold", "10" , 0 );
    config_set_option_not_exist( "lbd.APSteer.APSteerToPeerMinRSSIIncThreshold", "10" , 0 );
    config_set_option_not_exist( "lbd.APSteer.DownlinkRSSIThreshold_W5", "-65" , 0 );
    config_set_option_not_exist( "lbd.APSteer.APSteerMaxRetryCount", "2" , 0 );

    config_add_named_section( "lbd", "config", "config_Adv" );
    config_set_option_not_exist( "lbd.config_Adv.AgeLimit", "5" , 0 );
    config_set_option_not_exist( "lbd.config_Adv.BackhaulAgeLimit", "60" , 0 );
    config_set_option_not_exist( "lbd.config_Adv.LegacyClientAgeLimit", "20" , 0 );

    config_add_named_section( "lbd", "StaDB", "StaDB_Adv" );
    config_set_option_not_exist( "lbd.StaDB_Adv.AgingSizeThreshold", "100" , 0 );
    config_set_option_not_exist( "lbd.StaDB_Adv.AgingFrequency", "60" , 0 );
    config_set_option_not_exist( "lbd.StaDB_Adv.OutOfNetworkMaxAge", "300" , 0 );
    config_set_option_not_exist( "lbd.StaDB_Adv.InNetworkMaxAge", "5" , 0 );
    config_set_option_not_exist( "lbd.StaDB_Adv.AgingSizeThreshold", "2592000" , 0 );
    config_set_option_not_exist( "lbd.StaDB_Adv.NumNonServingBSSes", "4" , 0 );
    config_set_option_not_exist( "lbd.StaDB_Adv.PopulateNonServingPHYInfo", "1" , 0 );
    config_set_option_not_exist( "lbd.StaDB_Adv.LegacyUpgradeAllowedCnt", "0" , 0 );
    config_set_option_not_exist( "lbd.StaDB_Adv.LegacyUpgradeMonitorDur", "2100" , 0 );
    config_set_option_not_exist( "lbd.StaDB_Adv.MinAssocAgeForStatsAssocUpdate", "150" , 0 );

    config_add_named_section( "lbd", "StaMonitor", "StaMonitor_Adv" );
    config_set_option_not_exist( "lbd.StaMonitor_Adv.RSSIMeasureSamples_W2", "5" , 0 );
    config_set_option_not_exist( "lbd.StaMonitor_Adv.RSSIMeasureSamples_W5", "5" , 0 );

    config_add_named_section( "lbd", "BandMonitor", "BandMonitor_Adv" );
    config_set_option_not_exist( "lbd.BandMonitor_Adv.ProbeCountThreshold", "1" , 0 );
    config_set_option_not_exist( "lbd.BandMonitor_Adv.MUCheckInterval_W2", "10" , 0 );
    config_set_option_not_exist( "lbd.BandMonitor_Adv.MUCheckInterval_W5", "10" , 0 );
    config_set_option_not_exist( "lbd.BandMonitor_Adv.MUReportPeriod", "30" , 0 );
    config_set_option_not_exist( "lbd.BandMonitor_Adv.LoadBalancingAllowedMaxPeriod", "15" , 0 );
    config_set_option_not_exist( "lbd.BandMonitor_Adv.NumRemoteChannels", "3" , 0 );

    config_add_named_section( "lbd", "Estimator_Adv", "Estimator_Adv" );
    config_set_option_not_exist( "lbd.Estimator_Adv.RSSIDiff_EstW5FromW2", "-15" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.RSSIDiff_EstW2FromW5", "5" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.ProbeCountThreshold", "3" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.StatsSampleInterval", "1" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.BackhaulStationStatsSampleInterval", "10" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.Max11kUnfriendly", "10" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.11kProhibitTimeShort", "30" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.11kProhibitTimeLong", "300" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.PhyRateScalingForAirtime", "50" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.EnableContinuousThroughput", "0" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.BcnrptActiveDuration", "50" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.BcnrptPassiveDuration", "200" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.FastPollutionDetectBufSize", "10" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.NormalPollutionDetectBufSize", "10" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.PollutionDetectThreshold", "60" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.PollutionClearThreshold", "40" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.InterferenceAgeLimit", "15" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.IASLowRSSIThreshold", "12" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.IASMaxRateFactor", "88" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.IASMinDeltaBytes", "2000" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.IASMinDeltaPackets", "10" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.ActDetectMinInterval", "30" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.ActDetectMinPktPerSec", "2" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.BackhaulActDetectMinPktPerSec", "40" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.LowPhyRateThreshold", "20" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.HighPhyRateThreshold_W2", "200" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.HighPhyRateThreshold_W5", "750" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.PhyRateScalingFactorLow", "60" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.PhyRateScalingFactorMedium", "85" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.PhyRateScalingFactorHigh", "60" , 0 );
    config_set_option_not_exist( "lbd.Estimator_Adv.PhyRateScalingFactorTCP", "90" , 0 );

    config_add_named_section( "lbd", "SteerExec", "SteerExec_Adv" );
    config_set_option_not_exist( "lbd.SteerExec_Adv.TSteering", "15" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.InitialAuthRejCoalesceTime", "2" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.AuthRejMax", "3" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.SteeringUnfriendlyTime", "600" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.MaxSteeringUnfriendly", "604800" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.TargetLowRSSIThreshold_W2", "5" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.TargetLowRSSIThreshold_W5", "15" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.BlacklistTime", "900" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.BTMResponseTime", "10" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.BTMAssociationTime", "6" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.BTMAlsoBlacklist", "1" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.BTMUnfriendlyTime", "600" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.MaxBTMUnfriendly", "86400" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.MaxBTMActiveUnfriendly", "604800" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.MinRSSIBestEffort", "12" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.LowRSSIXingThreshold", "10" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.StartInBTMActiveState", "0" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.Delay24GProbeRSSIThreshold", "35" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.Delay24GProbeTimeWindow", "0" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.Delay24GProbeMinReqCount", "0" , 0 );
    config_set_option_not_exist( "lbd.SteerExec_Adv.LegacyUpgradeUnfriendlyTime", "21600" , 0 );

    config_add_named_section( "lbd", "SteerAlg_Adv", "SteerAlg_Adv" );
    config_set_option_not_exist( "lbd.SteerAlg_Adv.MinTxRateIncreaseThreshold", "53" , 0 );
    config_set_option_not_exist( "lbd.SteerAlg_Adv.MaxSteeringTargetCount", "1" , 0 );
    config_set_option_not_exist( "lbd.SteerAlg_Adv.ApplyEstimatedAirTimeOnSteering", "1" , 0 );
    config_set_option_not_exist( "lbd.SteerAlg_Adv.UsePathCapacityToSelectBSS", "1" , 0 );

    config_add_named_section( "lbd", "DiagLog", "DiagLog" );
    config_set_option_not_exist( "lbd.DiagLog.EnableLog", "1" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogServerIP", "192.168.1.10" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogServerPort", "7788" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogLevelWlanIF", "2" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogLevelBandMon", "2" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogLevelStaDB", "2" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogLevelSteerExec", "2" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogLevelStaMon", "2" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogLevelEstimator", "2" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogLevelDiagLog", "2" , 0 );
    config_set_option_not_exist( "lbd.DiagLog.LogLevelMultiAP", "2" , 0 );

    config_add_named_section( "lbd", "Persist", "Persist" );
    config_set_option_not_exist( "lbd.Persist.PersistPeriod", "3600" , 0 );
}

//==========================================================================
static void
check_set_upnpd_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "upnpd", "upnpd", "config" );
    config_set_option_not_exist( "upnpd.config.enabled", "0" , 0 );
    config_set_option_not_exist( "upnpd.config.enable_natpmp", "1" , 0 );
    config_set_option_not_exist( "upnpd.config.enable_upnp", "1" , 0 );
    config_set_option_not_exist( "upnpd.config.secure_mode", "1" , 0 );
    config_set_option_not_exist( "upnpd.config.log_output", "0" , 0 );
    config_set_option_not_exist( "upnpd.config.port", "5000" , 0 );
    config_set_option_not_exist( "upnpd.config.upnp_lease_file", "/var/upnp.leases" , 0 );
    config_set_option_not_exist( "upnpd.config.EXTERNAL_iface", "wan0" , 0 );
    config_uci_del( "upnpd.config.internal_iface", 0 );
    config_uci_add_list( "upnpd.config.internal_iface", "lan0_zone0", 0 );

    config_add_named_section( "upnpd", "perm_rule", "allow_high_ports" );
    config_set_option_not_exist( "upnpd.allow_high_ports.action", "allow" , 0 );
    config_set_option_not_exist( "upnpd.allow_high_ports.ext_ports", "1024-65535" , 0 );
    config_set_option_not_exist( "upnpd.allow_high_ports.int_addr", "0.0.0.0/0" , 0 );
    config_set_option_not_exist( "upnpd.allow_high_ports.int_ports", "1024-65535" , 0 );
    config_set_option_not_exist( "upnpd.allow_high_ports.comment", "Allow high ports" , 0 );

    config_add_named_section( "upnpd", "perm_rule", "default_deny" );
    config_set_option_not_exist( "upnpd.default_deny.action", "deny" , 0 );
    config_set_option_not_exist( "upnpd.default_deny.ext_ports", "0-65535" , 0 );
    config_set_option_not_exist( "upnpd.default_deny.int_addr", "0.0.0.0/0" , 0 );
    config_set_option_not_exist( "upnpd.default_deny.int_ports", "0-65535" , 0 );
    config_set_option_not_exist( "upnpd.default_deny.comment", "Default deny" , 0 );
}

//==========================================================================
static void
check_set_ssid_steering_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "ssid-steering", "ssid-steering", "global" );
    config_set_option_not_exist( "ssid-steering.ssid-steering.enable", "0" , 0 );
    config_set_option_not_exist( "ssid-steering.ssid-steering.private_vaps", "ath0" , 0 );
    config_set_option_not_exist( "ssid-steering.ssid-steering.public_vaps", "ath1" , 0 );
}

//==========================================================================
static void
check_set_dropbear_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "dropbear", "dropbear", "dropbear" );
    config_set_option_not_exist( "dropbear.dropbear.PasswordAuth", "on", 0 );
    config_set_option_not_exist( "dropbear.dropbear.RootPasswordAuth", "on", 0 );
    config_set_option_not_exist( "dropbear.dropbear.Port", "22", 0 );
    config_set_option_not_exist( "dropbear.dropbear.BannerFile", "/etc/banner", 0 );
}

//==========================================================================
static void
check_set_hyd_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "hyd", "config", "config" );
    config_set_option_not_exist( "hyd.config.Enable", "0", 0 );
    config_set_option_not_exist( "hyd.config.SwitchInterface", "auto", 0 );
    config_set_option_not_exist( "hyd.config.SwitchLanVid", "1", 0 );
    config_set_option_not_exist( "hyd.config.Control", "manual", 0 );
    config_set_option_not_exist( "hyd.config.DisableSteering", "0", 0 );

    config_add_named_section( "hyd", "hy", "hy" );
    config_set_option_not_exist( "hyd.hy.LoadBalancingSeamless", "1", 0 );
    config_set_option_not_exist( "hyd.hy.ConstrainTCPMedium", "0", 0 );
    config_set_option_not_exist( "hyd.hy.MaxLBReordTimeout", "1500", 0 );
    config_set_option_not_exist( "hyd.hy.HActiveMaxAge", "120000", 0 );

    config_add_named_section( "hyd", "Wlan", "Wlan" );
    config_set_option_not_exist( "hyd.Wlan.WlanCheckFreqInterval", "10", 0 );

    config_add_named_section( "hyd", "Vlanid", "Vlanid1" );
    config_set_option_not_exist( "hyd.Vlanid1.ifname", "eth1", 0 );
    config_set_option_not_exist( "hyd.Vlanid1.vid", "1", 0 );

    config_add_named_section( "hyd", "Vlanid", "Vlanid0" );
    config_set_option_not_exist( "hyd.Vlanid0.ifname", "eth0", 0 );
    config_set_option_not_exist( "hyd.Vlanid0.vid", "2", 0 );

    config_add_named_section( "hyd", "PathChWlan", "PathChWlan" );
    config_set_option_not_exist( "hyd.PathChWlan.UpdatedStatsInterval_W2", "1", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.StatsAgedOutInterval_W2", "30", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.MaxMediumUtilization_W2", "70", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.MediumChangeThreshold_W2", "10", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.LinkChangeThreshold_W2", "10", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.MaxMediumUtilizationForLC_W2", "70", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.CPULimitedTCPThroughput_W2", "0", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.CPULimitedUDPThroughput_W2", "0", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.PHYRateThresholdForMU_W2", "2000", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.ProbePacketInterval_W2", "1", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.ProbePacketSize_W2", "64", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.EnableProbe_W2", "1", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.AssocDetectionDelay_W2", "5", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.UpdatedStatsInterval_W5", "1", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.StatsAgedOutInterval_W5", "30", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.MaxMediumUtilization_W5", "70", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.MediumChangeThreshold_W5", "10", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.LinkChangeThreshold_W5", "10", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.MaxMediumUtilizationForLC_W5", "70", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.CPULimitedTCPThroughput_W5", "0", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.CPULimitedUDPThroughput_W5", "0", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.PHYRateThresholdForMU_W5", "2000", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.ProbePacketInterval_W5", "1", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.ProbePacketSize_W5", "64", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.EnableProbe_W5", "1", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.AssocDetectionDelay_W5", "5", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.ScalingFactorHighRate_W5", "750", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.ScalingFactorLow", "60", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.ScalingFactorMedium", "85", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.ScalingFactorHigh", "60", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.ScalingFactorTCP", "90", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.UseWHCAlgorithm", "1", 0 );
    config_set_option_not_exist( "hyd.PathChWlan.NumUpdatesUntilStatsValid", "3", 0 );

    config_add_named_section( "hyd", "PathChPlc", "PathChPlc" );
    config_set_option_not_exist( "hyd.PathChPlc.MaxMediumUtilization", "80", 0 );
    config_set_option_not_exist( "hyd.PathChPlc.MediumChangeThreshold", "10", 0 );
    config_set_option_not_exist( "hyd.PathChPlc.LinkChangeThreshold", "10", 0 );
    config_set_option_not_exist( "hyd.PathChPlc.StatsAgedOutInterval", "60", 0 );
    config_set_option_not_exist( "hyd.PathChPlc.UpdateStatsInterval", "1", 0 );
    config_set_option_not_exist( "hyd.PathChPlc.EntryExpirationInterval", "120", 0 );
    config_set_option_not_exist( "hyd.PathChPlc.MaxMediumUtilizationForLC", "80", 0 );
    config_set_option_not_exist( "hyd.PathChPlc.LCThresholdForUnreachable", "5", 0 );
    config_set_option_not_exist( "hyd.PathChPlc.LCThresholdForReachable", "10", 0 );
    config_set_option_not_exist( "hyd.PathChPlc.HostPLCInterfaceSpeed", "0", 0 );

    config_add_named_section( "hyd", "Topology", "Topology" );
    config_set_option_not_exist( "hyd.Topology.ND_UPDATE_INTERVAL", "15", 0 );
    config_set_option_not_exist( "hyd.Topology.BD_UPDATE_INTERVAL", "3", 0 );
    config_set_option_not_exist( "hyd.Topology.HOLDING_TIME", "190", 0 );
    config_set_option_not_exist( "hyd.Topology.TIMER_LOW_BOUND", "7", 0 );
    config_set_option_not_exist( "hyd.Topology.TIMER_UPPER_BOUND", "11", 0 );
    config_set_option_not_exist( "hyd.Topology.MSGID_DELTA", "64", 0 );
    config_set_option_not_exist( "hyd.Topology.HA_AGING_INTERVAL", "120", 0 );
    config_set_option_not_exist( "hyd.Topology.ENABLE_TD3", "1", 0 );
    config_set_option_not_exist( "hyd.Topology.ENABLE_BD_SPOOFING", "1", 0 );
    config_set_option_not_exist( "hyd.Topology.NOTIFICATION_THROTTLING_WINDOW", "1", 0 );
    config_set_option_not_exist( "hyd.Topology.PERIODIC_QUERY_INTERVAL", "60", 0 );
    config_set_option_not_exist( "hyd.Topology.ENABLE_NOTIFICATION_UNICAST", "0", 0 );

    config_add_named_section( "hyd", "HSPECEst", "HSPECEst" );
    config_set_option_not_exist( "hyd.HSPECEst.UpdateHSPECInterval", "1", 0 );
    config_set_option_not_exist( "hyd.HSPECEst.NotificationThresholdLimit", "10", 0 );
    config_set_option_not_exist( "hyd.HSPECEst.NotificationThresholdPercentage", "20", 0 );
    config_set_option_not_exist( "hyd.HSPECEst.AlphaNumerator", "3", 0 );
    config_set_option_not_exist( "hyd.HSPECEst.AlphaDenominator", "8", 0 );
    config_set_option_not_exist( "hyd.HSPECEst.LocalFlowRateThreshold", "2000000", 0 );
    config_set_option_not_exist( "hyd.HSPECEst.LocalFlowRatioThreshold", "5", 0 );
    config_set_option_not_exist( "hyd.HSPECEst.MaxHActiveEntries", "8192", 0 );

    config_add_named_section( "hyd", "PathSelect", "PathSelect" );
    config_set_option_not_exist( "hyd.PathSelect.UpdateHDInterval", "10", 0 );
    config_set_option_not_exist( "hyd.PathSelect.LinkCapacityThreshold", "20", 0 );
    config_set_option_not_exist( "hyd.PathSelect.UDPInterfaceOrder", "EP52", 0 );
    config_set_option_not_exist( "hyd.PathSelect.NonUDPInterfaceOrder", "EP52", 0 );
    config_set_option_not_exist( "hyd.PathSelect.SerialflowIterations", "10", 0 );
    config_set_option_not_exist( "hyd.PathSelect.DeltaLCThreshold", "10", 0 );

    config_add_named_section( "hyd", "LogSettings", "LogSettings" );
    config_set_option_not_exist( "hyd.LogSettings.EnableLog", "0", 0 );
    config_set_option_not_exist( "hyd.LogSettings.LogRestartIntervalSec", "10", 0 );
    config_set_option_not_exist( "hyd.LogSettings.LogPCSummaryPeriodSec", "0", 0 );
    config_set_option_not_exist( "hyd.LogSettings.LogServerIP", "192.168.1.10", 0 );
    config_set_option_not_exist( "hyd.LogSettings.LogServerPort", "5555", 0 );
    config_set_option_not_exist( "hyd.LogSettings.EnableLogPCW2", "1", 0 );
    config_set_option_not_exist( "hyd.LogSettings.EnableLogPCW5", "1", 0 );
    config_set_option_not_exist( "hyd.LogSettings.EnableLogPCP", "1", 0 );
    config_set_option_not_exist( "hyd.LogSettings.EnableLogTD", "1", 0 );
    config_set_option_not_exist( "hyd.LogSettings.EnableLogHE", "1", 0 );
    config_set_option_not_exist( "hyd.LogSettings.EnableLogHETables", "1", 0 );
    config_set_option_not_exist( "hyd.LogSettings.EnableLogPS", "1", 0 );
    config_set_option_not_exist( "hyd.LogSettings.EnableLogPSTables", "1", 0 );
    config_set_option_not_exist( "hyd.LogSettings.LogHEThreshold1", "200000", 0 );
    config_set_option_not_exist( "hyd.LogSettings.LogHEThreshold2", "10000000", 0 );

    config_add_named_section( "hyd", "IEEE1905Settings", "IEEE1905Settings" );
    config_set_option_not_exist( "hyd.IEEE1905Settings.StrictIEEE1905Mode", "0", 0 );
    config_set_option_not_exist( "hyd.IEEE1905Settings.GenerateLLDP", "1", 0 );

    config_add_named_section( "hyd", "HCPSettings", "HCPSettings" );
    config_set_option_not_exist( "hyd.HCPSettings.V1Compat", "1", 0 );

    config_add_named_section( "hyd", "SteerMsg", "SteerMsg" );
    config_set_option_not_exist( "hyd.SteerMsg.AvgUtilReqTimeout", "1", 0 );
    config_set_option_not_exist( "hyd.SteerMsg.LoadBalancingCompleteTimeout", "90", 0 );
    config_set_option_not_exist( "hyd.SteerMsg.RspTimeout", "2", 0 );

    config_add_named_section( "hyd", "Monitor", "Monitor" );
    config_set_option_not_exist( "hyd.Monitor.DisableMonitoring", "1", 0 );
    config_set_option_not_exist( "hyd.Monitor.MonitorTimeout", "10", 0 );
    config_set_option_not_exist( "hyd.Monitor.LowRSSIThreshold", "20", 0 );
    config_set_option_not_exist( "hyd.Monitor.HighRSSIThreshold", "30", 0 );
}

//==========================================================================
static void
check_set_nss_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "nss", "nss_firmware", "qca_nss_0" );

    config_add_named_section( "nss", "nss_firmware", "qca_nss_1" );

    config_add_named_section( "nss", "general", "general" );
    config_set_option_not_exist( "nss.general.enable_rps", "1", 0 );
}

//==========================================================================
static void
check_set_rpcd_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "rpcd", "login", "admin" );
    config_set_option_not_exist( "rpcd.admin.username", "admin", 0 );
    config_set_option_not_exist( "rpcd.admin.password", "$p$root", 0 );
    config_uci_del( "rpcd.admin.read", 0 );
    config_uci_del( "rpcd.admin.write", 0 );
    config_uci_add_list( "rpcd.admin.read", "*", 0 );
    config_uci_add_list( "rpcd.admin.write", "*", 0 );

    config_add_named_section( "rpcd", "login", "user" );
    config_set_option_not_exist( "rpcd.user.username", "user", 0 );
    config_set_option_not_exist( "rpcd.user.password", "123", 0 );
    config_uci_del( "rpcd.user.read", 0 );
    config_uci_add_list( "rpcd.user.read", "unauthenticated", 0 );
    config_uci_add_list( "rpcd.user.read", "core", 0 );
    config_uci_add_list( "rpcd.user.read", "overview", 0 );
    config_uci_add_list( "rpcd.user.read", "devices", 0 );
    config_uci_add_list( "rpcd.user.read", "clients", 0 );
    config_uci_add_list( "rpcd.user.read", "network", 0 );
    config_uci_add_list( "rpcd.user.read", "firewall", 0 );
}

//==========================================================================
static void
check_set_grandstream_defvalue(
    void
)
//==========================================================================
{
    char password[32] = {0};
    char temp[BUF_LEN_64] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };

    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "debug", "debug" );
    config_set_option_not_exist( "grandstream.debug.log_level", "4", 0 );
    config_set_option_not_exist( "grandstream.debug.syslog_level", "3", 0 );
    config_set_option_not_exist( "grandstream.debug.syslog_logqueries", "0", 0 );
    config_set_option_not_exist( "grandstream.debug.logserver_file_size_unit", "M", 0 );
    config_set_option_not_exist( "grandstream.debug.logserver_file_size", "5M", 0 );
    config_set_option_not_exist( "grandstream.debug.logserver_file_count", "56", 0 );
    config_set_option_not_exist( "grandstream.debug.logserver_rotate_mode", "1", 0 );
    config_set_option_not_exist( "grandstream.debug.logserver_rotate_rate", "3", 0 );
    config_set_option_not_exist( "grandstream.debug.logserver_rotate_rate_hour", "3", 0 );
    config_set_option_not_exist( "grandstream.debug.external_storage_path", "ramdrive", 0 );
    config_set_option_not_exist( "grandstream.debug.logserver", "0", 0 );

    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "general", "general" );
    util_read_file_content( "/proc/gxp/dev_info/security/ssid_password", password, sizeof( password ) );
    if ( password[0] == '\0' )
        strcpy( password, "admin" );    //set default admin_password
    config_set_option_not_exist( "grandstream.general.admin_password", password, 0 );
    config_set_option_not_exist( "grandstream.general.web_wan_access", "0", 0 );
    config_set_option_not_exist( "grandstream.general.user_password", "123", 0 );
    config_set_option_not_exist( "grandstream.general.password_change_required", "1", 0 );
    config_uci_del( "grandstream.general.ntp_server", 0 );
    config_uci_add_list( "grandstream.general.ntp_server", "0.pool.ntp.org", 0 );
    config_uci_add_list( "grandstream.general.ntp_server", "1.pool.ntp.org", 0 );
    config_uci_add_list( "grandstream.general.ntp_server", "2.pool.ntp.org", 0 );
    config_uci_add_list( "grandstream.general.ntp_server", "3.pool.ntp.org", 0 );
    config_set_option_not_exist( "grandstream.general.date_display", "0", 0 );
    config_set_option_not_exist( "grandstream.general.role", "0", 0 );
    config_set_option_not_exist( "grandstream.general.web_wan_http", "1", 0 );
    config_set_option_not_exist( "grandstream.general.web_port", "443", 0 );
    snprintf( temp, sizeof( temp ), "%d", COUNTRY_DEFVALUE );
    config_set_option_not_exist( "grandstream.general.country", temp, 0 );
    config_set_option_not_exist( "grandstream.general.timezone", "UTC", 0 );
    config_set_option_not_exist( "grandstream.general.outdoor", "0", 0 );
    config_set_option_not_exist( "grandstream.general.config_version", "2", 0 );
    config_set_option_not_exist( "grandstream.general.dfs", "1", 0 );
    config_set_option_not_exist( "grandstream.general.used_port", "14,10014,8080,8443,9443,223,224", 0 );

    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "provision", "provision" );
    config_set_option_not_exist( "grandstream.provision.on_boot", "1", 0 );
    config_set_option_not_exist( "grandstream.provision.auth", "0", 0 );
    config_set_option_not_exist( "grandstream.provision.config_server", "fm.grandstream.com/gs", 0 );
    config_set_option_not_exist( "grandstream.provision.server", "fm.grandstream.com/gs", 0 );
    config_set_option_not_exist( "grandstream.provision.protocol", "1", 0 );
    config_set_option_not_exist( "grandstream.provision.upgrade_mode", "0", 0 );
    config_set_option_not_exist( "grandstream.provision.allow_dhcp_override", "1", 0 );

    snprintf( temp, sizeof( temp ), COMPACT_MACSTR, MAC2STR( device_info.mac_raw ) );
    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "ap", temp );
    snprintf( path, sizeof(path), "grandstream.%s.type", temp );
    config_set_option_not_exist( path, device_info.product_model, 0 );

    //This configuration is required when configuring synchronization
    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "zone", "zone0" );
    config_set_option_not_exist( "grandstream.zone0.name", "Default", 0 );
    config_set_option_not_exist( "grandstream.zone0.id", "zone0", 0 );
    config_set_option_not_exist( "grandstream.zone0.enable", "1", 0 );
    config_set_option_not_exist( "grandstream.zone0.wan", "wan0", 0 );
    config_set_option_not_exist( "grandstream.zone0.ipv4", "0", 0 );
    config_set_option_not_exist( "grandstream.zone0.ipv6", "0", 0 );
    config_set_option_not_exist( "grandstream.zone0.ipv4_dhcp_enable", "0", 0 );
    config_set_option_not_exist( "grandstream.zone0.ipv6_dhcp_enable", "0", 0 );
    config_set_option_not_exist( "grandstream.zone0.dhcpv4_lease_time", "12h", 0 );
}

//==========================================================================
static void
check_set_controller_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( CF_CONFIG_NAME_CONTROLLER, "controller", "main" );
    config_set_option_not_exist( "controller.main.role", "slave", 0 );
    config_set_option_not_exist( "controller.main.workMode", "0", 0 );
}

//==========================================================================
static void
check_set_gsportalcfg_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "gsportalcfg", "general", "general" );

    config_set_option_not_exist( "gsportalcfg.general.fb_app_oauth_domain", "cwp.gwnportal.cloud", 0 );
    config_set_option_not_exist( "grandstream.general.fb_app_oauth_port", "8443", 0 );
}

//==========================================================================
static void
set_ap_option_value(
    const char *section,
    char *mac,
    char *name,
    char *value
)
//==========================================================================
{
    if ( NULL == name ) {
        return;
    }

    memset( name, 0, BUF_LEN_64 );
    snprintf( name, BUF_LEN_64, "%s.%s.%s", CF_CONFIG_NAME_CFMANAGER, mac, section );
    config_set_option_not_exist( name, value, 0 );
}

//==========================================================================
static void
check_set_ap_cfmanager_defvalue(
    void
)
//==========================================================================
{
    char password[BUF_LEN_128] = { 0 };
    char ssid[BUF_LEN_16] = { 0 };
    char guest_ssid[BUF_LEN_16] = { 0 };
    char ssid_password[BUF_LEN_32] = { 0 };
    char mac[BUF_LEN_16] = { 0 };
    char temp[BUF_LEN_64] = { 0 };
    int i = 0,index = 0, has_ssid_passwd = 0;
    FILE *procfs_data;
    int start = sizeof( device_info.mac )/2;

    //get mac
    for ( index = 0; index < sizeof( device_info.mac ); index++ ) {
        if ( device_info.mac[index] != ':' ) {
            mac[i++] = device_info.mac[index];
        }
    }

    //get ssid
    for( i= 0, index = start; index < sizeof( device_info.mac ); index++ ) {
        if ( device_info.mac[index] >= 'a' && device_info.mac[index] <= 'z' ) {
            temp[i++] = device_info.mac[index] - 32;
        }
        else {
            if ( device_info.mac[index] != ':' ) {
                temp[i++] = device_info.mac[index];
            }
        }
    }

    snprintf( ssid, BUF_LEN_16, "GS_%s", temp );
    snprintf( guest_ssid, BUF_LEN_16, "GSGuest_%s", temp );

    //get ssid passwoard
    procfs_data = fopen( "/proc/gxp/dev_info/security/ssid_password", "r" );
    if ( procfs_data ) {
        fgets( ssid_password, BUF_LEN_32, procfs_data );
        has_ssid_passwd = 1;
        fclose( procfs_data );
    }

    //get admin_password from old cfmanager
    config_uci_get_option( "cfmanager.general.admin_password", password, sizeof(password) );

    //remove old cfmanager
    unlink( ck_cfg[CHECK_CFMANAGER].name );
    check_create_cfg( ck_cfg[CHECK_CFMANAGER].name, CHECK_CFMANAGER );

    //SET LAN
    config_add_named_section( CF_CONFIG_NAME_CFMANAGER, "lan", "lan" );
    config_set_option_not_exist( "cfmanager.lan.dhcpEnable", "0", 0 );

    //SET WIRELESS
    config_add_named_section( CF_CONFIG_NAME_CFMANAGER, "wireless", "wireless" );
    config_set_option_not_exist( "cfmanager.wireless.wifiEnable", "1", 0 );
    config_set_option_not_exist( "cfmanager.wireless.2g4Enable", "1", 0 );
    config_set_option_not_exist( "cfmanager.wireless.2g4ChannelWidth", "0", 0 );
    config_set_option_not_exist( "cfmanager.wireless.5gEnable", "1", 0 );
    config_set_option_not_exist( "cfmanager.wireless.2g4Crypto", "0", 0 );
    config_set_option_not_exist( "cfmanager.wireless.5gCrypto", "0", 0 );
    config_set_option_not_exist( "cfmanager.wireless.2g4Channel", "0", 0 );
    config_set_option_not_exist( "cfmanager.wireless.5gChannel", "0", 0 );
    config_set_option_not_exist( "cfmanager.wireless.5gChannelWidth", "2", 0 );
    config_set_option_not_exist( "cfmanager.wireless.mergeRadioEnable", "1", 0 );
    config_set_option_not_exist( "cfmanager.wireless.mumimoEnable", "1", 0 );
    config_set_option_not_exist( "cfmanager.wireless.compatibilityMode", "0", 0 );

    //2g and 5g ssid name and password
    config_set_option_not_exist( "cfmanager.wireless.2g4SsidName", ssid, 0 );
    config_set_option_not_exist( "cfmanager.wireless.5gSsidName", ssid, 0 );
    config_set_option_not_exist( "cfmanager.wireless.2g4Password", ssid_password, 0 );
    config_set_option_not_exist( "cfmanager.wireless.5gPassword", ssid_password, 0 );

    //SET AP
    config_add_named_section( CF_CONFIG_NAME_CFMANAGER, "ap", mac );
    set_ap_option_value( "mac", mac, temp, mac );
    set_ap_option_value( "type", mac, temp, device_info.product_model );
    set_ap_option_value( "mesh", mac, temp, "0" );
    set_ap_option_value( "frequency", mac, temp, "2" );
    set_ap_option_value( "2g4Mode", mac, temp, "2" );
    set_ap_option_value( "2g4Width", mac, temp, "3" );
    set_ap_option_value( "5gWidth", mac, temp, "4" );
    set_ap_option_value( "2g4Power", mac, temp, "4" );
    set_ap_option_value( "5gPower", mac, temp, "4" );
    set_ap_option_value( "2g4Shortgi", mac, temp, "1" );
    set_ap_option_value( "5gShortgi", mac, temp, "1" );
    set_ap_option_value( "bandSteering", mac, temp, "4" );
    set_ap_option_value( "zones", mac, temp, "zone0" );
    set_ap_option_value( "ssids", mac, temp, "ssid0" );
    set_ap_option_value( "tls12", mac, temp, "1" );

    //SET CONTROLLER
    config_add_named_section( CF_CONFIG_NAME_CFMANAGER, "controller", "main" );
    config_set_option_not_exist( "cfmanager.main.role", "slave", 0 );
    config_set_option_not_exist( "cfmanager.main.workMode", "1", 0 );

    //SET GUEST_SSID
    config_add_named_section( CF_CONFIG_NAME_CFMANAGER, "guest_ssid", "guest_ssid" );
    config_set_option_not_exist( "cfmanager.guest_ssid.passwordSsid", ssid_password, 0 );
    config_set_option_not_exist( "cfmanager.guest_ssid.ssidName", guest_ssid, 0 );
    config_set_option_not_exist( "cfmanager.guest_ssid.enable", "0", 0 );
    config_set_option_not_exist( "cfmanager.guest_ssid.encryptoMode", "1", 0 );

    //SET MESH_SSID
    config_add_named_section( CF_CONFIG_NAME_CFMANAGER, "mesh_ssid", "mesh_ssid" );
    config_set_option_not_exist( "cfmanager.mesh_ssid.enable", "0", 0 );
    config_set_option_not_exist( "cfmanager.mesh_ssid.mode", "ap", 0 );
    config_set_option_not_exist( "cfmanager.mesh_ssid.ssid", "MESH-7062", 0 );
    config_set_option_not_exist( "cfmanager.mesh_ssid.bssid", "00:00:00:00:00:00", 0 );
    config_set_option_not_exist( "cfmanager.mesh_ssid.encryption", "1", 0 );
    config_set_option_not_exist( "cfmanager.mesh_ssid.key", "12345678", 0 );
    config_set_option_not_exist( "cfmanager.mesh_ssid.isadded", "0", 0 );

    //SET BASIC
    config_add_named_section( CF_CONFIG_NAME_CFMANAGER, "basic", "basic" );
    config_set_option_not_exist( "cfmanager.basic.led", "1", 0 );
    config_set_option_not_exist( "cfmanager.basic.country", "840", 0 );
    config_set_option_not_exist( "cfmanager.basic.timeZone", "America/Chicago", 0 );

    //SET GENERAL
    if ( password[0] != '\0' ) {
        config_add_named_section( CF_CONFIG_NAME_CFMANAGER, "general", "general" );
        config_set_option_not_exist( "cfmanager.general.admin_password", password, 0 );
    }

    // SET USB SHARE
    config_add_named_section(CF_CONFIG_NAME_CFMANAGER, "usb_share", "usb_share" );
    config_set_option_not_exist("cfmanager.usb_share.enable", "1", 0);
    config_set_option_not_exist("cfmanager.usb_share.anonymity","0", 0);
    config_set_option_not_exist("cfmanager.usb_share.smbUser", "admin", 0);
    if( has_ssid_passwd ) {
        config_set_option_not_exist("cfmanager.usb_share.smbPassword", ssid_password, 0);
    }
    else {
        config_set_option_not_exist("cfmanager.usb_share.smbPassword", "admin", 0);
    }
}

static void
check_set_samba_defvalue(
    void
)
//==========================================================================
{
    FILE *procfs_data;
    char ssid_password[BUF_LEN_32] = { 0 };
    int has_ssid_password = 0;

    config_add_named_section( "samba", "global","global" );
    config_set_option_not_exist("samba.global.name", "GWN7062", 0);
    config_set_option_not_exist("samba.global.description", "Samba3.6.25 on GWN7062", 0);
    config_set_option_not_exist("samba.global.workgroup", "WORKGROUP", 0);
    config_set_option_not_exist("samba.global.interfaces", "lan0_zone0", 0);

    //get default ssid passwoard.
    procfs_data = fopen( "/proc/gxp/dev_info/security/ssid_password", "r" );
    if ( procfs_data ) {
        fgets( ssid_password, BUF_LEN_32, procfs_data );
        has_ssid_password = 1;
        fclose( procfs_data );
    }

    config_add_named_section( "samba", "user","user" );
    config_set_option_not_exist("samba.user.username", "admin", 0);
    if( has_ssid_password ) {
        config_set_option_not_exist("samba.user.password", ssid_password, 0);
    }
    else {
        config_set_option_not_exist("samba.user.password", "admin", 0);
    }
}

//==========================================================================
static void
check_set_tr069_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "tr069", "tr069", "tr069" );
    config_set_option_not_exist( "tr069.tr069.ssid_counts", "4", 0 );
}

//==========================================================================
static void
check_set_clientbridge_defvalue(
    void
)
//==========================================================================
{
    config_add_named_section( "clientbridge", "staoptions", "clientbridge" );
    config_set_option_not_exist( "clientbridge.clientbridge.mode", "disconnected", 0 );
    config_set_option_not_exist( "clientbridge.clientbridge.enable", "0", 0 );
}

//==========================================================================
void
check_set_defvalue(
    check_config config
)
//==========================================================================
{
    switch ( config ) {
        case CHECK_WIRELESS:
            check_set_wireless_glob_defvalue();
            check_set_wireless_defvalue( "wifi0" );
            check_set_wireless_defvalue( "wifi1" );
            config_commit( "wireless", false );
            break;
        case CHECK_DHCP:
            check_set_dhcp_defvalue();
            config_commit( "dhcp", false );
            break;
        case CHECK_NETWORK:
            check_set_network_defvalue();
            config_commit( "network", false );
            break;
        case CHECK_QOS:
            check_set_qos_defvalue();
            config_commit( "qos", false );
            break;
        case CHECK_SYSTEM:
            check_set_system_defvalue();
            config_commit( "system", false );
            break;
        case CHECK_EMAIL:
            check_set_email_defvalue();
            config_commit( "email", false );
            break;
        case CHECK_NOTIFICATION:
            check_set_notification_defvalue();
            config_commit( "notification", false );
            break;
        case CHECK_FIREWALL:
            break;
        case CHECK_SCHEDULE:
            break;
        case CHECK_MWAN3:
            break;
        case CHECK_ECM:
            check_set_ecm_defvalue();
            config_commit( "ecm", false );
            break;
        case CHECK_DDNS:
            break;
        case CHECK_QCACFG80211:
            check_set_qcacfg80211_defvalue();
            config_commit( "qcacfg80211", false );
            break;
        case CHECK_TRACKS:
            check_set_tracks_defvalue();
            config_commit( "tracks", false );
            break;
        case CHECK_FSTAB:
            check_set_fstab_defvalue();
            config_commit( "fstab", false );
            break;
        case CHECK_LBD:
            check_set_lbd_defvalue();
            config_commit( "lbd", false );
            break;
        case CHECK_IPSEC:
            break;
        case CHECK_UPNPD:
            check_set_upnpd_defvalue();
            config_commit( "upnpd", false );
            break;
        case CHECK_SSID_STEERING:
            check_set_ssid_steering_defvalue();
            config_commit( "ssid-steering", false );
            break;
        case CHECK_DROPBEAR:
            check_set_dropbear_defvalue();
            config_commit( "dropbear", false );
            break;
        case CHECK_HYD:
            check_set_hyd_defvalue();
            config_commit( "hyd", false );
            break;
        case CHECK_NSS:
            check_set_nss_defvalue();
            config_commit( "nss", false );
            break;
        case CHECK_RPCD:
            check_set_rpcd_defvalue();
            config_commit( "rpcd", false );
            break;
        case CHECK_SAMBA:
            check_set_samba_defvalue();
            config_commit( "samba", false);
        case CHECK_GRANDSTREAM:
            check_set_grandstream_defvalue();
            config_commit( CF_CONFIG_NAME_GRANDSTREAM, false );
            break;
        case CHECK_CONTROLLER:
            check_set_controller_defvalue();
            config_commit( CF_CONFIG_NAME_CONTROLLER, false );
        case CHECK_BWCTRL:
            break;
        case CHECK_CFMANAGER:
            check_set_ap_cfmanager_defvalue();
            config_commit( CF_CONFIG_NAME_CFMANAGER, false );
            break;
        case CHECK_PORTALCFG:
            check_set_gsportalcfg_defvalue();
            config_commit( "gsportalcfg", false );
            break;
        case CHECK_TR069:
            check_set_tr069_defvalue();
            config_commit( "tr069", false );
            break;
        case CHECK_CLIENTBRIDGE:
            check_set_clientbridge_defvalue();
            config_commit( "clientbridge", false );
            break;
        default:
            cfmanager_log_message( L_ERR, "Parameter error:%d\n", config );
            break;
    }
}
//==========================================================================
void
check_create_all_cfg(
    void
)
//==========================================================================
{
    int i;
    int ret;
    char path_src[LOOKUP_STR_SIZE];
    char path_dest[LOOKUP_STR_SIZE];

    if( MODE_AP == device_info.work_mode ) {
        check_flag |= BIT( CHECK_FIREWALL );
        check_flag |= BIT( CHECK_MWAN3 );
    }

    for( i = 0; i < __CHECK_MAX; i++ ) {

        //Don't check cfmanager when cfmanager restart
        if( CHECK_CFMANAGER == i ) {
            continue;
        }

        switch( ck_cfg[i].rebuild_option ) {
            case NEED_REBUILD:
                unlink( ck_cfg[i].name );
                check_create_cfg( ck_cfg[i].name, i );
                break;
            case NOT_REBUILD:
                ret = access( ck_cfg[i].name, F_OK );
                if ( ret ) {
                    snprintf( path_src, sizeof( path_src ),
                        "%s/%s", UCI_DEFAULT_PATH, ck_cfg[i].name+strlen( CM_CONFIG_PATH )+1 );
                    if( access( path_src, F_OK ) ) {
                        check_create_cfg( ck_cfg[i].name, i );
                    }
                    else {
                        snprintf( path_dest, sizeof( path_dest ),
                            "%s/%s", CM_CONFIG_PATH, ck_cfg[i].name+strlen( CM_CONFIG_PATH )+1 );

                        util_cpy_file( path_src, path_dest );
                    }
                }
                break;
            default:
                cfmanager_log_message( L_ERR, "Unknown rebuild option:%d\n", ck_cfg[i].rebuild_option );
                break;
        }

        check_set_defvalue( i );
    }

    check_flag = 0;
}

