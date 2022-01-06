/****************************************************************************
* *
* * FILENAME:        $RCSfile: wireless.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/12
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
#include "wireless.h"
#include "track.h"
#include "initd.h"
#include "global.h"
#include "utils.h"
#include "cfparse.h"
#include "time.h"
#include "apply.h"
#include "check.h"

//=================
//  Defines
//=================
#define WIRELESS_SCRIPT_NAME SPLICE_STR(CM_FOLDER,/wireless.sh)
#define BUFF_SIZE 1024

//=================
//  Typedefs
//=================
enum
{
    OPTION_FLAGS_MIN,
    OPTION_FLAGS_NEED_CHECK,
    OPTION_FLAGS_CHECK_COMPL,
    OPTION_FLAGS_HAVE_DOWN,
    OPTION_FLAGS_NEED_UP,
    OPTION_FLAGS_START_8021XD,

    __OPTION_FLAGS_MAX
};
//=================
//  Globals
//=================
struct default_opt_value {
    const char *name;
    const char *defvalue;
};

const struct blobmsg_policy wireless_iface_policy[__WIRELESS_IFACE_MAX] = {
    [WIRELESS_IFACE_DISABLED] = { .name = "disabled", .type = BLOBMSG_TYPE_BOOL },
    [WIRELESS_IFACE_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_NETWORK] = { .name = "network", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_MODE] = { .name = "mode", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_MCAST_TO_UCAST] = { .name = "mcast_to_ucast", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_BINTVAL] = { .name = "bintval", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_DTIM_PERIOD] = { .name = "dtim_period", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_PROXYARP] = { .name = "proxyarp", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_MCASTENHANCE] = { .name = "mcastenhance", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_WDS] = { .name = "wds", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_DISABLECOEXT] = { .name = "disablecoext", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_HIDDEN] = { .name = "hidden", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_MAXSTA] = { .name = "maxsta", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_MACLIST] = { .name = "maclist", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_MACFILTER] = { .name = "macfilter", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_WMM] = { .name = "wmm", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_VHTSUBFER] = { .name = "vhtsubfer", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_VHTMUBFER] = { .name = "vhtmubfer", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_RSSI_ENABLE] = { .name = "rssi_enable", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_RSSI_THRESHOLD] = { .name = "rssi_threshold", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_VOICE_ENTERPRISE] = { .name = "voice_enterprise", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_11R] = { .name = "11R", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_11K] = { .name = "11K", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_WNM] = { .name = "wnm", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_ISOLATE] = { .name = "isolate", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_ISOLATION_MODE] = { .name = "isolation_mode", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_GATEWAY_MAC] = { .name = "gateway_mac", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_PORTAL_ENABLE] = { .name = "portal_enable", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_PORTAL_POLICY] = { .name = "portal_policy", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_VLAN_TRUNK] = { .name = "vlan_trunk", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_MINIRATE_ENABLE] = { .name = "minirate_enable", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_MINIRATE_THRESHOLD] = { .name = "minirate_threshold", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_VLAN] = { .name = "vlan", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_INACT] = { .name = "inact", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_11W] = { .name = "ieee80211w", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_ENCRYPTION] = { .name = "encryption", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_KEY] = { .name = "key", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_AUTH_SERVER] = { .name = "auth_server", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_DYNAMIC_VLAN] = { .name = "dynamic_vlan", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_AUTH_PORT] = { .name = "auth_port", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_AUTH_SECRET] = { .name = "auth_secret", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_ACCT_SERVER] = { .name = "acct_server", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_ACCT_PORT] = { .name = "acct_port", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_ACCT_SECRET] = { .name = "acct_secret", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_NASID] = { .name = "nasid", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_NETWORK_TYPE] = { .name = NET_TYPE_OPTION_NAME, .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_BSSID] = { .name = "bssid", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_SAE] = { .name = "sae", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_SUITE_B] = { .name = "suite_b", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_UAPSD] = { .name = "uapsd", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFACE_ID] = { .name = "ssid_id", .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy wireless_device_policy[__WIRELESS_DEVICE_MAX] = {
    [WIRELESS_DEVICE_DISABLED] = { .name = "disabled", .type = BLOBMSG_TYPE_BOOL },
    [WIRELESS_DEVICE_PHY] = { .name = "phy", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_OUTDOOR] = { .name = "outdoor", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_PUREG] = { .name = "pureg", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_HWMODE] = { .name = "hwmode", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_HTMODE] = { .name = "htmode", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_SHORTGI] = { .name = "shortgi", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_TXCHAINMASK] = { .name = "txchainmask", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_RXCHAINMASK] = { .name = "rxchainmask", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_DBDC_ENABLE] = { .name = "dbdc_enable", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_CHANNEL] = { .name = "channel", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_TXPOWER] = { .name = "txpower", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_MUMIMO] = { .name = "mumimo", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_DEVICE_GEOANALYTIC] = { .name = "geoanalytic", .type = BLOBMSG_TYPE_STRING },
};

struct vlist_tree wireless_vltree;
struct vlist_tree wireless_global_vltree;
//=================
//  Locals
//=================

/*
 * Private Functions
 */

/*
 * Private Data
 */

static const struct uci_blob_param_list wireless_iface_list = {
    .n_params = __WIRELESS_IFACE_MAX,
    .params = wireless_iface_policy,
};

static const struct uci_blob_param_list wireless_device_list = {
    .n_params = __WIRELESS_DEVICE_MAX,
    .params = wireless_device_policy,
};

static const struct blobmsg_policy wireless_global_policy[__WIRELESS_GLOBAL_MAX] = {
    [WIRELESS_GLOBAL_ATF_MODE] = { .name = "atf_mode", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_GLOBAL_DFS] = { .name = "dfs", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_GLOBAL_COUNTRY] = { .name = "country", .type = BLOBMSG_TYPE_STRING },
};

static const struct uci_blob_param_list wireless_global_list = {
    .n_params = __WIRELESS_GLOBAL_MAX,
    .params = wireless_global_policy,
};

static struct default_opt_value global_opt_defvalue[] =
{
    {"atf_mode", "0"},
    {"dfs", "0"},
    {"country", "840"},
};

static struct default_opt_value device_opt_defvalue[] =
{
    {"disabled", "0"},
    {"phy", "1" },
    {"outdoor", "0" },
    {"pureg", "1" },
    {"hwmode", "auto" },
    {"htmode", "auto" },
    {"shortgi", "1" },
    {"txchainmask", "0" },
    {"rxchainmask", "0" },
    {"dbdc_enable", "1"},
    {"channel", "1" },
    {"txpower", "30" },
    {"mumimo", "1" },
    {"geoanalytic", "0" },
};

static struct default_opt_value iface_opt_defvalue[] =
{
    { "disabled", "0" },
    { "ifname", "" },
    { "network", "" },
    { "mode", "" },
    { "mcast_to_ucast", "0" },
    { "bintval", "100" },
    { "dtim_period", "1" },
    { "proxyarp", "0" },
    { "mcastenhance", "0" },
    { "wds", "0" },
    { "disablecoext", "0" },
    { "hidden", "0" },
    { "maxsta", "127" },
    { "maclist", "" },
    { "macfilter", "deny" },
    { "wmm", "1" },
    { "vhtsubfer", "1" },
    { "vhtmubfer", "1" },
    { "rssi_enable", "1" },
    { "rssi_threshold", "1" },
    { "voice_enterprise", "1" },
    { "11R", "0" },
    { "11K", "0" },
    { "wnm", "0" },
    { "isolate", "0" },
    { "isolation_mode", "0" },
    { "gateway_mac", "0" },
    { "portal_enable", "1" },
    { "portal_policy", "1" },
    { "vlan_trunk", "0" },
    { "minirate_enable", "" },
    { "minirate_threshold", "" },
    { "vlan", "0" },
    { "inact", "300" },
    { "ieee80211w", "0" },
    { "ssid", "" },
    { "encryption", "none" },
    { "key", "" },
    { "auth_server", "" },
    { "dynamic_vlan", "" },
    { "auth_port", "" },
    { "auth_secret", "" },
    { "acct_server", "" },
    { "acct_port", "" },
    { "acct_secret", "" },
    { "nasid", "" },
    {"suite_b", "" },
    {"uapsd", "1" },
};

static struct blob_buf b;
static FILE *fp;            //Wireless all the commands that need to be processed are put into one file
static uint32_t options;
#ifdef CONFIG_MTK
static char country_id[BUF_LEN_8] = { "840" };
static char country_code[BUF_LEN_8] = { "US" };
static char country_region[BUF_LEN_8] = { "0" };
static char country_region_aband[BUF_LEN_8] = { "17" };
static char htmode_2g[BUF_LEN_8] = { 0 };
static char htmode_5g[BUF_LEN_8] = { 0 };
static const char *ifname_2g = NULL;
static const char *ifname_5g = NULL;
#endif
//=================
//  Functions
//=================

//=============================================================================
static const char*
cfparse_get_tx_power_override(
    int radio
)
//=============================================================================
{
    struct uci_package *p;
    struct blob_attr *glob_tb[__WIRELESS_GLOBAL_MAX];
    struct cfparse_wifi_global *cfwgl;

    if ( !uci_ctx ) {
        uci_ctx = uci_alloc_context();
        uci_ctx->flags &= ~UCI_FLAG_STRICT;
    }

    p = cfparse_init_package( "wireless_limits_override" );
    if ( !p ) {
        return NULL;
    }

    cfwgl= util_get_vltree_node( &wireless_global_vltree, VLTREE_DEV_GLOB, "qcawifi" );

    if( !cfwgl ) {
        return NULL;
    }

    blobmsg_parse( wireless_global_policy,
        __WIRELESS_GLOBAL_MAX,
        glob_tb,
        blob_data( cfwgl->cf_section.config ),
        blob_len( cfwgl->cf_section.config ) );

    if ( radio == RADIO_2G ) {
        return uci_lookup_option_string_from_package_and_section ( uci_ctx, p,
            blobmsg_get_string( glob_tb[WIRELESS_GLOBAL_COUNTRY] ), "2ghz_power_max" );
    }
    else {
        return uci_lookup_option_string_from_package_and_section ( uci_ctx, p,
            blobmsg_get_string( glob_tb[WIRELESS_GLOBAL_COUNTRY] ), "5ghz_power_max" );
    }

}

#ifdef CONFIG_MTK
//=============================================================================
static void
cfparse_wireless_down_iface(
    struct cfparse_wifi_interface *vif,
    FILE *fp
)
//=============================================================================
{
    if( NULL == vif || NULL == fp ) {
        cfmanager_log_message(  LOG_DEBUG, "Illegal parameter\n" );
        return;
    }

    if( vif->flags & BIT(OPTION_FLAGS_HAVE_DOWN) ) {
        return;
    }

    fprintf( fp, "ifconfig %s down\n", vif->cf_section.name );
    vif->flags |= BIT(OPTION_FLAGS_HAVE_DOWN);
    vif->flags |= BIT(OPTION_FLAGS_NEED_UP);
}

//=============================================================================
static int
parse_country(
    void
)
//=============================================================================
{
    FILE * fp = NULL;
    char * p = NULL;
    char * value = NULL;
    char buff[128] = {0};
    int len = 0;
    int ret = -1;

    fp = popen( "/lib/wifi/country.sh", "r" );
    if( !fp ) {
        printf( "%s(), error %d\n", __FUNCTION__, __LINE__ );
        return -1;
    }

    if ( fgets( buff, sizeof( buff ), fp ) ) {
        len = strlen( buff );
        if ( buff[len-1] == '\n' )
            buff[len-1] = '\0';

        p = buff;
        value = p;
        p = strchr( p, ':' );
        if (p == NULL)
            goto end;
        *p = '\0';
        if ( strlen( value ) > 0 ) {
            memset( country_id, 0, sizeof( country_id ) );
            strncpy( country_id, value, sizeof( country_id ) -1 );
        }

        p++;
        value = p;
        p = strchr( p, ':' );
        if (p == NULL)
            goto end;
        *p = '\0';
        if ( strlen( value ) > 0 ) {
            memset( country_code, 0, sizeof( country_code ) );
            strncpy( country_code, value, sizeof( country_code ) -1 );
        }

        p++;
        value = p;
        p = strchr( p, ':' );
        if (p == NULL)
            goto end;
        *p = '\0';
        if ( strlen( value ) > 0 ) {
            memset( htmode_2g, 0, sizeof( htmode_2g ) );
            strncpy( htmode_2g, value, sizeof( htmode_2g ) -1 );
        }

        p++;
        value = p;
        p = strchr( p, ':' );
        if (p == NULL)
            goto end;
        *p = '\0';
        if ( strlen( value ) > 0 ) {
            memset( country_region, 0, sizeof( country_region ) );
            strncpy( country_region, value, sizeof( country_region ) -1 );
        }

        p++;
        value = p;
        p = strchr( p, ':' );
        if ( p == NULL )
            goto end;
        *p = '\0';
        if ( strlen( value ) > 0 ) {
            memset( htmode_5g, 0, sizeof( htmode_5g ) );
            strncpy( htmode_5g, value, sizeof( htmode_5g ) -1 );
        }

        p++;
        value = p;
        if ( strlen( value ) > 0 ) {
            memset( country_region_aband, 0, sizeof( country_region_aband ) );
            strncpy( country_region_aband, value, sizeof( country_region_aband ) -1 );
        }
    }

    ret = 0;
end:
    pclose( fp );
    return ret;
}

//=============================================================================
static int
cfparse_wireless_iface_hooker(
    FILE *fp,
    struct cfparse_wifi_interface *vif,
    struct blob_attr **new_attr,
    int index
)
//=============================================================================
{
    int rc = 0;
    const char *value = NULL;
    const char *prefix = "";
    struct cfparse_wifi_interface *temp_vif;
    struct blob_attr *new_config = new_attr[index];
    char temp[BUFF_SIZE] = { 0 };
    char *tmp = NULL;

    value = blobmsg_get_string( new_config );
    if( !value ) {
        value = iface_opt_defvalue[index].defvalue;
    }

    if( 0 == strcmp( util_blobmsg_get_string( new_attr[WIRELESS_IFACE_MODE], "" ), "sta" ) ) {
        prefix = "ApCli";
    }

    switch( index ) {
        case WIRELESS_IFACE_DISABLED:
            break;
        case WIRELESS_IFACE_IFNAME:
            break;
        case WIRELESS_IFACE_NETWORK:
            vif->option_flags[WIRELESS_IFACE_SSID] = OPTION_FLAGS_NEED_CHECK;
            vif->option_flags[WIRELESS_IFACE_VLAN] = OPTION_FLAGS_NEED_CHECK;
            rc = OPTION_FLAGS_NEED_RELOAD;
            break;
        case WIRELESS_IFACE_MODE:
            rc = OPTION_FLAGS_NEED_RELOAD;
            break;
        case WIRELESS_IFACE_MCAST_TO_UCAST:
            break;
        case WIRELESS_IFACE_BINTVAL:
            {
                const char *bintval_str = NULL;
                int bintval = 0, max_bintval = 0;

                vlist_for_each_element( &vif->cfwdev->interfaces, temp_vif, node ) {
                    if ( !temp_vif->disabled ) {
                        continue;
                    }

                    if ( blobmsg_get_string ( new_attr[WIRELESS_IFACE_BINTVAL] ) ) {
                        bintval_str = blobmsg_get_string ( new_attr[WIRELESS_IFACE_BINTVAL] );
                    }
                    else {
                        bintval_str = iface_opt_defvalue[WIRELESS_IFACE_BINTVAL].defvalue;
                    }
                    bintval = atoi( bintval_str );
                    if ( bintval > max_bintval ) {
                        max_bintval = bintval;
                        value = bintval_str;
                    }
                }

                cfparse_wireless_down_iface( vif, fp );
                fprintf( fp, "iwpriv %s set BeaconPeriod=%s\n", vif->cf_section.name, value );
            }
            break;
        case WIRELESS_IFACE_DTIM_PERIOD:
            cfparse_wireless_down_iface( vif, fp );
            fprintf( fp, "iwpriv %s set DtimPeriod=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_PROXYARP:
            fprintf( fp, "iwpriv %s set ProxyARPEnable=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_MCASTENHANCE:
            cfparse_wireless_down_iface( vif, fp );
            fprintf( fp, "iwpriv %s set IgmpSnEnable=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_WDS:
            break;
        case WIRELESS_IFACE_DISABLECOEXT:
            if( 0 == strcmp(value, "0") ) {
                value = "1";
            }
            else {
                value = "0";
            }
            cfparse_wireless_down_iface( vif, fp );
            fprintf( fp, "iwpriv %s set HtBssCoex=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_HIDDEN:
            fprintf( fp, "iwpriv %s set HideSSID=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_MAXSTA:
            fprintf( fp, "iwpriv %s set MbssMaxStaNum=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_MACLIST:
            fprintf( fp, "iwpriv %s set ACLClearAll=1\n", vif->cf_section.name );
            fprintf( fp, "maclist=\"%s\"\n"
                "for mac in $maclist; do\n"
                "\tiwpriv %s set ACLAddEntry=\"$mac\"\n"
                "done\n", value, vif->cf_section.name );
        case WIRELESS_IFACE_MACFILTER:
            if ( OPTION_FLAGS_CHECK_COMPL == vif->option_flags[WIRELESS_IFACE_MACFILTER] ) {
                break;
            }
            vif->option_flags[WIRELESS_IFACE_MACFILTER] = OPTION_FLAGS_CHECK_COMPL;

            if ( !value ) {
                value = iface_opt_defvalue[WIRELESS_IFACE_MACFILTER].defvalue;
            }

            if ( 0 == strcmp( "allow", value ) ) {
                fprintf( fp, "iwpriv %s set AccessPolicy=1\n", vif->cf_section.name );
            }
            else if ( 0 == strcmp( "deny", value ) ) {
                fprintf( fp, "iwpriv %s set AccessPolicy=2\n", vif->cf_section.name );
            }
            else {
                fprintf( fp, "iwpriv %s set AccessPolicy=0\n", vif->cf_section.name );
            }
            break;
        case WIRELESS_IFACE_WMM:
            fprintf( fp, "iwpriv %s set WmmCapable=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_VHTSUBFER:
            break;
        case WIRELESS_IFACE_VHTMUBFER:
            break;
        case WIRELESS_IFACE_RSSI_ENABLE:
            break;
        case WIRELESS_IFACE_RSSI_THRESHOLD:
            break;
        case WIRELESS_IFACE_VOICE_ENTERPRISE:
            break;
        case WIRELESS_IFACE_11R:
            fprintf( fp, "iwpriv %s set FtSupport=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_11K:
            fprintf( fp, "iwpriv %s set RRMEnable=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_WNM:
            fprintf( fp, "iwpriv %s set WNMEnable=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_ISOLATE:
            fprintf( fp, "iwpriv %s set NoForwarding=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_ISOLATION_MODE:
            break;
        case WIRELESS_IFACE_GATEWAY_MAC:
            break;
        case WIRELESS_IFACE_PORTAL_ENABLE:
            break;
        case WIRELESS_IFACE_PORTAL_POLICY:
            break;
        case WIRELESS_IFACE_VLAN_TRUNK:
            break;
        case WIRELESS_IFACE_MINIRATE_ENABLE:
        case WIRELESS_IFACE_MINIRATE_THRESHOLD:
            {
                const char *enable = NULL;
                const char *rate = NULL;

                if ( OPTION_FLAGS_CHECK_COMPL == vif->option_flags[WIRELESS_IFACE_MINIRATE_ENABLE] ) {
                    break;
                }
                vif->option_flags[WIRELESS_IFACE_MINIRATE_ENABLE] = OPTION_FLAGS_CHECK_COMPL;

                if ( blobmsg_get_string ( new_attr[WIRELESS_IFACE_MINIRATE_ENABLE] ) ) {
                    enable = blobmsg_get_string (new_attr[WIRELESS_IFACE_MINIRATE_ENABLE] );
                }
                else{
                    enable = iface_opt_defvalue[WIRELESS_IFACE_MINIRATE_ENABLE].defvalue;
                }
                if ( blobmsg_get_string ( new_attr[WIRELESS_IFACE_MINIRATE_THRESHOLD] ) ) {
                    rate = blobmsg_get_string (new_attr[WIRELESS_IFACE_MINIRATE_THRESHOLD] );
                }
                else {
                    rate = iface_opt_defvalue[WIRELESS_IFACE_MINIRATE_THRESHOLD].defvalue;
                }

                cfparse_wireless_down_iface( vif, fp );
                if ( 0 == strcmp( "1", enable ) )
                    fprintf( fp, "iwpriv %s set MinDataRate=%s\n", vif->cf_section.name, rate );
                else
                    fprintf( fp, "iwpriv %s set MinDataRate=0\n", vif->cf_section.name );
            }
            break;
        case WIRELESS_IFACE_VLAN:
            cfparse_wireless_down_iface( vif, fp );
            value = blobmsg_get_string ( new_attr[WIRELESS_IFACE_NETWORK] );
            fprintf( fp, "ubus call network.interface.$( ovs-vsctl port-to-br %s | sed s/br-// ) remove_device '{\"name\": \"%s\"}'\n", vif->cf_section.name, vif->cf_section.name );
            if ( value && strlen( value ) > 0 ) {
                fprintf( fp, "ubus -t 10 wait_for network.interface.%s\n", value );
                fprintf( fp, "ubus call network.interface.%s add_device '{\"name\": \"%s\"}'\n", value, vif->cf_section.name );
            }
            fprintf( fp, "iwpriv %s set VlanID=%s\n", vif->cf_section.name, value );
            goto update_hostapd;
            break;
        case WIRELESS_IFACE_INACT:
            cfparse_wireless_down_iface( vif, fp );
            fprintf( fp, "iwpriv %s set IdleTimeout=%s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_11W:
            cfparse_wireless_down_iface( vif, fp );
            if ( value && ( 0 == strcmp( "1", value ) ) ) {
                fprintf( fp, "iwpriv %s set %sPMFMFPC=1\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sPMFMFPR=0\n", vif->cf_section.name, prefix );
            }
            else if ( value && ( 0 == strcmp( "2", value ) ) ) {
                fprintf( fp, "iwpriv %s set %sPMFMFPC=1\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sPMFMFPR=1\n", vif->cf_section.name, prefix );
            }
            else {
                fprintf( fp, "iwpriv %s set %sPMFMFPC=0\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sPMFMFPR=0\n", vif->cf_section.name, prefix );
            }
            break;
        case WIRELESS_IFACE_BSSID:
        case WIRELESS_IFACE_SSID:
            util_escape_single_quote( value, temp, sizeof( temp ) );
            tmp = util_blobmsg_get_string( new_attr[WIRELESS_IFACE_MODE], "" );
            if ( ( 0 == strcmp( "sta",  tmp ) ) ){
                fprintf( fp, "iwpriv %s set ApCliSsid='%s'\n", vif->cf_section.name, temp );
            }
            else {
                fprintf( fp, "iwpriv %s set SSID='%s'\n", vif->cf_section.name, temp );
            }
            goto update_hostapd;
        case WIRELESS_IFACE_ENCRYPTION:
            if ( 0 == strcmp( "none", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=OPEN\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=NONE\n", vif->cf_section.name, prefix );
            }
            else if ( 0 == strcmp( "wep", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=OPEN\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=WEP\n", vif->cf_section.name, prefix );
            }
            else if ( 0 == strcmp( "psk+aes", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=WPAPSKWPA2PSK\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=AES\n", vif->cf_section.name, prefix );
            }
            else if ( 0 == strcmp( "psk+tkip+aes", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=WPAPSKWPA2PSK\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=TKIPAES\n", vif->cf_section.name, prefix );
            }
            else if ( 0 == strcmp( "wpa+aes", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=WPA1WPA2\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=AES\n", vif->cf_section.name, prefix );
                vif->cfwdev->flags |= BIT( OPTION_FLAGS_START_8021XD );
            }
            else if ( 0 == strcmp( "wpa+tkip+aes", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=WPA1WPA2\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=TKIPAES\n", vif->cf_section.name, prefix );
                vif->cfwdev->flags |= BIT( OPTION_FLAGS_START_8021XD );
            }
            else if ( 0 == strcmp( "wpa2+aes", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=WPA2\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=AES\n", vif->cf_section.name, prefix );
                vif->cfwdev->flags |= BIT( OPTION_FLAGS_START_8021XD );
            }
            else if ( 0 == strcmp( "wpa2+tkip+aes", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=WPA2\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=TKIPAES\n", vif->cf_section.name, prefix );
                vif->cfwdev->flags |= BIT( OPTION_FLAGS_START_8021XD );
            }
            else if ( 0 == strcmp( "psk2+aes", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=WPA2PSK\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=AES\n", vif->cf_section.name, prefix );
            }
            else if ( 0 == strcmp( "psk2+tkip+aes", value ) ) {
                fprintf( fp, "iwpriv %s set %sAuthMode=WPA2PSK\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=TKIPAES\n", vif->cf_section.name, prefix );
            }
            else if(0 == strncmp("wpa3", value, strlen("wpa3")) ||
                    0 == strncmp("sae", value, strlen("sae"))) {
                const char *p = NULL;
                const char *cipher = NULL;
                const char *authmode = NULL;
                const char *encryptype = NULL;

                if (0 == strncmp("wpa3-mixed", value, strlen("wpa3-mixed"))) {
                    authmode = "WPA2WPA3";
                    p = value+strlen("wpa3-mixed");

                } else if (0 == strncmp("wpa3", value, strlen("wpa3"))) {
                    if (new_attr[WIRELESS_IFACE_SUITE_B]&&
                        0 == strcasecmp(blobmsg_get_string(new_attr[WIRELESS_IFACE_SUITE_B]), "192")) {
                        authmode="WPA3-192";
                    } else 
                        authmode="WPA3";

                    p = value+strlen("wpa3");

                } else if (0 == strncmp("sae-mixed", value, strlen("sae-mixed"))) {
                    authmode = "WPA2PSKWPA3PSK";
                    p = value+strlen("sae-mixed");

                } else if (0 == strncmp("sae", value, strlen("sae"))) {
                    authmode = "WPA3PSK";
                    p = value+strlen("sae");
                }

                if (*p == '+' && *(p+1) != 0)
                    cipher = p+1;
                else if (new_attr[WIRELESS_IFACE_SUITE_B] &&
                         0 == strncmp( blobmsg_get_string(new_attr[WIRELESS_IFACE_SUITE_B]), "192", strlen( "192" ) ) )
                    cipher = "gcmp-256";
                else if (0 == strncmp("wpa3", value, strlen("wpa3")))
                    cipher = "ccmp";
                else
                    cipher = "tkip+ccmp";

                if (0 == strcasecmp(cipher, "ccmp")
                     || 0 == strcasecmp(cipher, "aes")
                     || 0 == strcasecmp(cipher, "ccmp-128"))
                    encryptype = "AES";
                else if (0 == strcasecmp(cipher, "tkip"))
                    encryptype = "TKIP";
                else if (0 == strcasecmp(cipher, "ccmp+tkip")
                         || 0 == strcasecmp(cipher, "tkip+ccmp")
                         || 0 == strcasecmp(cipher, "tkip+aes")
                         || 0 == strcasecmp(cipher, "aes+tkip"))
                    encryptype = "TKIPAES";
                else if (0 == strcasecmp(cipher, "gcmp")
                         || 0 == strcasecmp(cipher, "gcmp-128"))
                    encryptype = "GCMP128";
                else if (0 == strcasecmp(cipher, "ccmp-256"))
                    encryptype = "CCMP256";
                else if (0 == strcasecmp(cipher, "gcmp-256"))
                    encryptype = "GCMP256";
                else if (0 == strcasecmp(cipher, "gcmp"))
                    encryptype = "GCMP128";
                else if (0 == strcasecmp(cipher, "ccmp-256"))
                    encryptype = "CCMP256";
                else
                    encryptype = "NONE";

                fprintf( fp, "iwpriv %s set %sAuthMode=%s\n", vif->cf_section.name, prefix, authmode );
                fprintf( fp, "iwpriv %s set %sEncrypType=%s\n", vif->cf_section.name, prefix, encryptype );
            }
            else {
                fprintf( fp, "iwpriv %s set %sAuthMode=OPEN\n", vif->cf_section.name, prefix );
                fprintf( fp, "iwpriv %s set %sEncrypType=NONE\n", vif->cf_section.name, prefix );
            }
            /* fall through */
        case WIRELESS_IFACE_KEY:
            {
                const char *enc = blobmsg_get_string( new_attr[WIRELESS_IFACE_ENCRYPTION] );
                const char *key = blobmsg_get_string( new_attr[WIRELESS_IFACE_KEY] );

                vif->option_flags[WIRELESS_IFACE_KEY] = OPTION_FLAGS_CHECK_COMPL;
                cfparse_wireless_down_iface( vif, fp );
                if ( !key ) {
                    key = iface_opt_defvalue[WIRELESS_IFACE_KEY].defvalue;
                }

                util_escape_single_quote( key, temp, sizeof( temp ) );
                if (enc && 0 == strcmp( "wep", enc ) ) {
                    fprintf( fp, "iwpriv %s set %sDefaultKeyID=1\n", vif->cf_section.name, prefix );
                    fprintf( fp, "iwpriv %s set %sKey1='%s'\n", vif->cf_section.name, prefix, temp  );
                }
                else {
                    fprintf( fp, "iwpriv %s set %sWPAPSK='%s'\n", vif->cf_section.name, prefix, temp  );
                }
            }
            goto update_hostapd;
        case WIRELESS_IFACE_AUTH_SERVER:
        case WIRELESS_IFACE_DYNAMIC_VLAN:
            cfparse_wireless_down_iface( vif, fp );
            fprintf( fp, "iwpriv %s set DynamicVlan=%s\n", vif->cf_section.name, value );
            vif->cfwdev->flags |= BIT( OPTION_FLAGS_START_8021XD );
            goto update_hostapd;
        case WIRELESS_IFACE_AUTH_PORT:
        case WIRELESS_IFACE_AUTH_SECRET:
        case WIRELESS_IFACE_ACCT_SERVER:
        case WIRELESS_IFACE_ACCT_PORT:
        case WIRELESS_IFACE_ACCT_SECRET:
            rc = OPTION_FLAGS_NEED_RELOAD;
            return rc;
        case WIRELESS_IFACE_NASID:
            {
                cfparse_wireless_down_iface( vif, fp );
                fprintf( fp, "iwpriv %s set NasId=%s\n", vif->cf_section.name, value );
                vif->cfwdev->flags |= BIT(OPTION_FLAGS_START_8021XD);
update_hostapd:
                if ( OPTION_FLAGS_CHECK_COMPL == vif->option_flags[WIRELESS_IFACE_VLAN] ) {
                    break;
                }
                vif->option_flags[WIRELESS_IFACE_VLAN] = OPTION_FLAGS_CHECK_COMPL;
                if ( blobmsg_get_string ( new_attr[WIRELESS_IFACE_MODE] ) ) {
                    value = blobmsg_get_string ( new_attr[WIRELESS_IFACE_MODE] );
                }
                else {
                    value = iface_opt_defvalue[WIRELESS_IFACE_MODE].defvalue;
                }

                if ( blobmsg_get_string( new_attr[WIRELESS_IFACE_NETWORK] ) &&
                    strlen( blobmsg_get_string ( new_attr[WIRELESS_IFACE_NETWORK] ) )> 0 ) {

                    fprintf( fp, "config_set \"%s\" bridge \"br-%s\"\n", vif->cf_section.name, blobmsg_get_string ( new_attr[WIRELESS_IFACE_NETWORK] ) );
                }

                if ( 0 == strcmp( "ap", value ) ) {
                    fprintf( fp, "wpa_cli -g /var/run/hostapd/global raw REMOVE %s\n", vif->cf_section.name );
                    fprintf( fp, "hostapd_setup_vif %s mtk7621 no_nconfig\n", vif->cf_section.name );
                }
            }
            break;
        case WIRELESS_IFACE_UAPSD:
            fprintf( fp, "iwpriv %s set UAPSDCapable=%s\n", vif->cf_section.name, value );
            break;
        default:
            break;
    }

    rc |= OPTION_FLAGS_CONFIG_CHANGED;
    return rc;
}

//=============================================================================
static int
cfparse_wireless_device_hooker(
    FILE *fp,
    struct cfparse_wifi_device *cfwdev,
    struct blob_attr **new_attr,
    int index
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case WIRELESS_DEVICE_DISABLED:
            rc = OPTION_FLAGS_NEED_RELOAD;
            break;
        case WIRELESS_DEVICE_PHY:
            break;
        case WIRELESS_DEVICE_OUTDOOR:
            if ( util_device_is_world_model( ) ) {
                if ( parse_country( ) ) {
                    rc = OPTION_FLAGS_NEED_RELOAD;
                    break;
                }
                fprintf( fp, "iwpriv %s set CountryRegion=%s\n", cfwdev->cf_section.name, country_region );
                fprintf( fp, "iwpriv %s set CountryRegionABand=%s\n", cfwdev->cf_section.name, country_region_aband );
            }
            break;
        case WIRELESS_DEVICE_PUREG:
        case WIRELESS_DEVICE_HWMODE:
            cfwdev->option_flags[WIRELESS_DEVICE_HWMODE] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
        case WIRELESS_DEVICE_HTMODE:
            cfwdev->option_flags[WIRELESS_DEVICE_HTMODE] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->option_flags[WIRELESS_DEVICE_CHANNEL] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
        case WIRELESS_DEVICE_SHORTGI:
            cfwdev->option_flags[WIRELESS_DEVICE_SHORTGI] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
        case WIRELESS_DEVICE_TXCHAINMASK:
            cfwdev->option_flags[WIRELESS_DEVICE_TXCHAINMASK] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
        case WIRELESS_DEVICE_RXCHAINMASK:
            cfwdev->option_flags[WIRELESS_DEVICE_RXCHAINMASK] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
        case WIRELESS_DEVICE_DBDC_ENABLE:
            break;
        case WIRELESS_DEVICE_CHANNEL:
            cfwdev->option_flags[WIRELESS_DEVICE_CHANNEL] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->option_flags[WIRELESS_DEVICE_TXPOWER] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
        case WIRELESS_DEVICE_TXPOWER:
            cfwdev->option_flags[WIRELESS_DEVICE_TXPOWER] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
        case WIRELESS_DEVICE_GEOANALYTIC:
            cfwdev->option_flags[WIRELESS_DEVICE_GEOANALYTIC] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
        default:
            break;
    }

    rc |= OPTION_FLAGS_CONFIG_CHANGED;

    return rc;
}

//=============================================================================
static void
cfparse_set_device_by_iface(
    FILE *fp,
    struct cfparse_wifi_interface *vif
)
//=============================================================================
{
    struct blob_attr *tb[__WIRELESS_DEVICE_MAX];
    const char *value = NULL;
    int i = 0;

    if( !fp || !vif ) {
        return;
    }

    const char *dev_name = vif->cfwdev->cf_section.name;
    const char *iface_name = vif->cf_section.name;
    struct cfparse_wifi_device *cfwdev = vif->cfwdev;

    blobmsg_parse( wireless_device_policy,
        __WIRELESS_DEVICE_MAX,
        tb,
        blob_data( cfwdev->cf_section.config ),
        blob_len( cfwdev->cf_section.config ) );

    for ( i = 0; i < __WIRELESS_DEVICE_MAX;  i++ ) {
        if( OPTION_FLAGS_NEED_CHECK != cfwdev->option_flags[i] ) {
            continue;
        }
        switch ( i ) {
            case WIRELESS_DEVICE_PUREG:
            case WIRELESS_DEVICE_HWMODE:
            {
                const char *pureg = NULL;
                const char *hwmode = NULL;
                const char *basicrate = "15"; //0x1f

                if ( tb[WIRELESS_DEVICE_PUREG] )
                    pureg = blobmsg_get_string( tb[WIRELESS_DEVICE_PUREG] );
                else
                    pureg = device_opt_defvalue[WIRELESS_DEVICE_PUREG].defvalue;
                if ( tb[WIRELESS_DEVICE_HWMODE] )
                    hwmode = blobmsg_get_string( tb[WIRELESS_DEVICE_HWMODE] );
                else
                    hwmode = device_opt_defvalue[WIRELESS_DEVICE_HWMODE].defvalue;

                value = pureg;
                if( 0 == strcasecmp( hwmode, "11b" ) )
                    value = "1";
                else if( 0 == strcasecmp( hwmode, "11g" ) )
                {
                    if ( value && ( 0 == strcasecmp( pureg, "1" ) ) ) {
                        value = "4";
                        basicrate = "351";//0x15f
                    }
                    else
                        value = "0";
                }
                else if( 0 == strcasecmp( hwmode, "11ng" ) )
                {
                    if ( value && ( 0 == strcasecmp( pureg, "1" ) ) ) {
                        value = "7";
                        basicrate = "351";//0x15f
                    }
                    else
                        value = "9";
                }
                else if( 0 == strcasecmp( hwmode, "11ac" ) )
                    value = "14";
                else
                    value = "5";

                cfparse_wireless_down_iface( vif, fp );
                fprintf( fp, "iwpriv %s set BasicRate=%s\n", iface_name, basicrate );
                fprintf( fp, "iwpriv %s set WirelessMode=%s\n", iface_name, value );
            }
                break;
            case WIRELESS_DEVICE_HTMODE:
            {
                const char *ht_extcha = NULL;
                const char *ht_bw = NULL;
                const char *vht_bw = NULL;

                if( tb[WIRELESS_DEVICE_HTMODE] ) {
                    value = blobmsg_get_string( tb[WIRELESS_DEVICE_HTMODE] );
                }
                else {
                    value = device_opt_defvalue[WIRELESS_DEVICE_HTMODE].defvalue;
                }

                if( 0 == strcasecmp( value, "HT40-" ) )
                    ht_extcha = "0";
                else if( 0 == strcasecmp( value, "HT40+" ) )
                    ht_extcha = "1";
                else
                    ht_extcha = "2";

                if ( 0 == strcmp( "wifi0", cfwdev->cf_section.name ) && '\0' != htmode_2g[0] )
                    value = &htmode_2g[0];

                if ( 0 == strcmp( "wifi1", cfwdev->cf_section.name ) && '\0' != htmode_5g[0] )
                    value = &htmode_5g[0];

                if( 0 == strcasecmp( value, "HT20" ) )
                    ht_bw = "0";
                else
                    ht_bw = "1";

                if( 0 == strcasecmp( value, "HT80" ) )
                    vht_bw = "1";
                else if ( 0 == strcasecmp( value, "HT160" ) )
                    vht_bw = "2";
                else if ( 0 == strcasecmp( value, "HT80_80" ) )
                    vht_bw = "3";
                else
                    vht_bw = "0";

                cfparse_wireless_down_iface( vif, fp );
                fprintf( fp, "iwpriv %s set HtExtcha=%s\n", iface_name, ht_extcha );
                fprintf( fp, "iwpriv %s set HtBw=%s\n", iface_name, ht_bw );
                fprintf( fp, "iwpriv %s set VhtBw=%s\n", iface_name, vht_bw );

                if ( !parse_country( ) ) {
                    fprintf( fp, "iwpriv %s set CountryRegion=%s\n", dev_name, country_region );
                    fprintf( fp, "iwpriv %s set CountryRegionABand=%s\n", dev_name, country_region_aband );
                }
            }
                break;
            case WIRELESS_DEVICE_SHORTGI:
                value = blobmsg_get_string( tb[WIRELESS_DEVICE_SHORTGI] );
                if( !value ) {
                    value = device_opt_defvalue[WIRELESS_DEVICE_SHORTGI].defvalue;
                }

                fprintf( fp, "iwpriv %s set HtGi=%s\n", iface_name, value );
                fprintf( fp, "iwpriv %s set VhtGi=%s\n", iface_name, value );
                break;
            case WIRELESS_DEVICE_TXCHAINMASK:
                {
                    const char *stream = NULL;

                    value = blobmsg_get_string( tb[WIRELESS_DEVICE_TXCHAINMASK] );
                    if( !value ) {
                        value = device_opt_defvalue[WIRELESS_DEVICE_TXCHAINMASK].defvalue;
                    }

                    if( 0 == strcmp( value, "15" ) )
                        stream = "4";
                    else if( 0 == strcmp( value, "7" ) )
                        stream = "3";
                    else if( 0 == strcmp( value, "3" ) )
                        stream = "2";
                    else if( 0 == strcmp( value, "1" ) )
                        stream = "1";
                    else
                        stream = "4";

                    cfparse_wireless_down_iface( vif, fp );
                    fprintf( fp, "iwpriv %s set HtTxStream=%s\n", iface_name, stream );
                }
                break;
            case WIRELESS_DEVICE_RXCHAINMASK:
                {
                    const char *stream = NULL;

                    value = blobmsg_get_string( tb[WIRELESS_DEVICE_TXCHAINMASK] );
                    if( !value ) {
                        value = device_opt_defvalue[WIRELESS_DEVICE_TXCHAINMASK].defvalue;
                    }

                    if( 0 == strcmp( value, "15" ) )
                        stream = "4";
                    else if( 0 == strcmp( value, "7" ) )
                        stream = "3";
                    else if( 0 == strcmp( value, "3" ) )
                        stream = "2";
                    else if( 0 == strcmp( value, "1" ) )
                        stream = "1";
                    else
                        stream = "4";

                    cfparse_wireless_down_iface( vif, fp );
                    fprintf( fp, "iwpriv %s set HtRxStream=%s\n", iface_name, stream );
                }
                break;
            case WIRELESS_DEVICE_CHANNEL:
                value = blobmsg_get_string( tb[WIRELESS_DEVICE_CHANNEL] );
                if( !value ) {
                    value = device_opt_defvalue[WIRELESS_DEVICE_CHANNEL].defvalue;
                }

                if ( 0 == strcmp( "auto", value ) ) {
#if (defined GWN7052)
                    fprintf( fp, "for i in $(seq 1 5) ; do iwpriv %s set AutoChannelSel=3 && break; sleep 1 ; done\n", iface_name);
#else
                    fprintf( fp, "iwpriv %s set AutoChannelSel=3\n", iface_name);
#endif
                    fprintf( fp, "sleep 2\n");
                }
                else {
                    int interface_down_flags = 0;

                    if ( vif->flags & BIT(OPTION_FLAGS_HAVE_DOWN) || !interface_is_up( iface_name ) ) {
                        fprintf( fp, "ifconfig %s up\n", iface_name );
                        interface_down_flags = 1;
                    }

                    fprintf( fp, "iwconfig %s channel %s\n", iface_name, value );

                    if ( interface_down_flags ) {
                        fprintf( fp, "ifconfig %s down\n", iface_name );
                    }
                }
                break;
            case WIRELESS_DEVICE_TXPOWER: {
                const char *txpower_override_str = NULL;
                int radio = 0, txpower = 0, txpower_override  = 0;

                value = blobmsg_get_string( tb[WIRELESS_DEVICE_TXPOWER] );
                if( !value ) {
                    value = device_opt_defvalue[WIRELESS_DEVICE_TXPOWER].defvalue;
                }

                if ( strcmp( dev_name, "wifi0" ) == 0 ) {
                    radio = RADIO_2G;
                }
                else {
                    radio = RADIO_5G;
                }

                txpower_override_str = cfparse_get_tx_power_override( radio );
                txpower = atoi( value );
                if ( txpower_override_str ) {
                    txpower_override = atoi( txpower_override_str );
                }
                if ( ( txpower_override > 0 ) && ( txpower > txpower_override ) ) {
                    value = txpower_override_str;
                }

                if ( strcmp("auto", value) == 0 ) {
                    value = (txpower_override > 0 ? txpower_override_str : "29");
                }

                fprintf( fp, "iwconfig %s txpower %s\n", iface_name, value );
            }
                break;
            case WIRELESS_DEVICE_GEOANALYTIC:

                value = blobmsg_get_string( tb[WIRELESS_DEVICE_GEOANALYTIC] );
                if( !value ) {
                    value = device_opt_defvalue[WIRELESS_DEVICE_GEOANALYTIC].defvalue;
                }
                fprintf( fp, "iwpriv %s set Monitor_Enable=%s\n", iface_name, value);
                break;
            default:
                break;
        }
    }
}

//=============================================================================
static int
cfparse_wireless_global_hooker(
    FILE *fp,
    struct cfparse_wifi_global *cfwgl,
    struct blob_attr *new_config,
    int index
)
//=============================================================================
{
    int rc = 0;
    const char *value = NULL;
    struct cfparse_wifi_device *cfwdev;

    value = blobmsg_get_string( new_config );
    if( !value ) {
        value = global_opt_defvalue[index].defvalue;
    }

    switch( index ) {
        case WIRELESS_GLOBAL_ATF_MODE:
            fprintf( fp, "iwpriv %s set ATFEnable=%s\n", ifname_2g, value );
            fprintf( fp, "iwpriv %s set ATFEnable=%s\n", ifname_5g, value );
            break;
        case WIRELESS_GLOBAL_DFS:
            cfwgl->option_flags[WIRELESS_GLOBAL_COUNTRY] |= OPTION_FLAGS_NEED_CHECK;
            break;
        case WIRELESS_GLOBAL_COUNTRY:
            if( util_device_is_world_model( ) ) {
                if ( parse_country() ) {
                    return -1;
                }

                fprintf( fp, "iwpriv %s set CountryID=%s\n", ifname_2g, country_id );
                fprintf( fp, "iwpriv %s set CountryRegion=%s\n", ifname_2g, country_region );
                fprintf( fp, "iwpriv %s set CountryRegionABand=%s\n", ifname_2g, country_region_aband );
                fprintf( fp, "iwpriv %s set CountryID=%s\n", ifname_5g, country_id );
                fprintf( fp, "iwpriv %s set CountryRegion=%s\n", ifname_5g, country_region );
                fprintf( fp, "iwpriv %s set CountryRegionABand=%s\n", ifname_5g, country_region_aband );

                vlist_for_each_element( &wireless_vltree, cfwdev, node ) {
                    cfwdev->option_flags[WIRELESS_DEVICE_TXPOWER] = OPTION_FLAGS_NEED_CHECK;
                    cfwdev->option_flags[WIRELESS_DEVICE_HTMODE] = OPTION_FLAGS_NEED_CHECK;
                    cfwdev->option_flags[WIRELESS_DEVICE_CHANNEL] = OPTION_FLAGS_NEED_CHECK;
                }
            }
            break;
    };

    rc |= OPTION_FLAGS_CONFIG_CHANGED;
    return rc;
}

#else
//=============================================================================
static char*
cfparse_get_wlan_mode(
    const char *hwmode,
    const char *htmode
)
//=============================================================================
{
    if ( 0 == strcmp( hwmode, "11b" ) ) {
        return "11B";
    }
    else if ( 0 == strcmp( hwmode, "11bg" ) || 0 == strcmp( hwmode, "11g" ) ) {
        return "11G";
    }
    else if ( 0 == strcmp( hwmode, "11a" ) ) {
        return "11A";
    }
    else if ( 0 == strcmp( hwmode, "11ng" ) ) {
        if ( 0 == strcmp( htmode, "HT40-" ) ) {
            return "11NGHT40MINUS";
        }
        else if ( 0 == strcmp( htmode, "HT40+" ) ) {
            return "11NGHT40PLUS";
        }
        else if ( 0 == strcmp( htmode, "HT40" ) ) {
            return "11NGHT40";
        }
        else {
            return "11NGHT20";
        }
    }
    else if ( 0 == strcmp( hwmode, "11na" ) ) {
        if ( 0 == strcmp( htmode, "HT40-" ) ) {
            return "11NAHT40MINUS";
        }
        else if ( 0 == strcmp( htmode, "HT40+" ) ) {
            return "11NAHT40PLUS";
        }
        else if ( 0 == strcmp( htmode, "HT20" ) ) {
            return "11NAHT20";
        }
        else {
            return "11NAHT40";
        }
    }
    else if ( 0 == strcmp( hwmode, "11ac" ) ) {
        if ( 0 == strcmp( htmode, "HT20" ) ) {
            return "11ACVHT20";
        }
        else if ( 0 == strcmp( htmode, "HT40-" ) ) {
            return "11ACVHT40MINUS";
        }
        else if ( 0 == strcmp( htmode, "HT40+" ) ) {
            return "11ACVHT40PLUS";
        }
        else if ( 0 == strcmp( htmode, "HT40" ) ) {
            return "11ACVHT40";
        }
        else if ( 0 == strcmp( htmode, "HT160" ) ) {
            return "11ACVHT160";
        }
        else if ( 0 == strcmp( htmode, "HT80_80" ) ) {
            return "11ACVHT80_80";
        }
        else {
            return "11ACVHT80";
        }
    }
    else if ( 0 == strcmp( hwmode, "11axg" ) ) {
        if ( 0 == strcmp( htmode, "HT40-" ) ) {
            return "11GHE40MINUS";
        }
        else if ( 0 == strcmp( htmode, "HT40+" ) ) {
            return "11GHE40PLUS";
        }
        else if ( 0 == strcmp( htmode, "HT40" ) ) {
            return "11GHE40";
        }
        else {
            return "11GHE20";
        }
    }
    else if ( 0 == strcmp( hwmode, "11axa" ) ) {
        if ( 0 == strcmp( htmode, "HT20" ) ) {
            return "11AHE20";
        }
        else if ( 0 == strcmp( htmode, "HT40-" ) ) {
            return "11AHE40MINUS";
        }
        else if ( 0 == strcmp( htmode, "HT40+" ) ) {
            return "11AHE40PLUS";
        }
        else if ( 0 == strcmp( htmode, "HT40" ) ) {
            return "11AHE40";
        }
        else if ( 0 == strcmp( htmode, "HT160" ) ) {
            return "11AHE160";
        }
        else if ( 0 == strcmp( htmode, "HT80_80" ) ) {
            return "11AHE80_80";
        }
        else {
            return "11AHE80";
        }
    }
    else {
            return "AUTO";
    }
}

//=============================================================================
static void
cfparse_set_device_by_iface(
    FILE *fp,
    struct cfparse_wifi_interface *vif
)
//=============================================================================
{
    struct blob_attr *tb[__WIRELESS_DEVICE_MAX];
    const char *value;
    int i = 0;

    if( !fp || !vif ) {
        return;
    }

    const char *dev_name = vif->cfwdev->cf_section.name;
    const char *iface_name = vif->cf_section.name;
    struct cfparse_wifi_device *cfwdev = vif->cfwdev;

    blobmsg_parse( wireless_device_policy,
        __WIRELESS_DEVICE_MAX,
        tb,
        blob_data( cfwdev->cf_section.config ),
        blob_len( cfwdev->cf_section.config ) );

    for ( i = 0; i < __WIRELESS_DEVICE_MAX;  i++ ) {
        if( OPTION_FLAGS_NEED_CHECK != cfwdev->option_flags[i] ) {
            continue;
        }
        switch ( i ) {
            case WIRELESS_DEVICE_PUREG:
                value = blobmsg_get_string( tb[WIRELESS_DEVICE_PUREG] );
                if( !value ) {
                    value = device_opt_defvalue[WIRELESS_DEVICE_PUREG].defvalue;
                }
                fprintf( fp, "iwpriv %s pureg %s\n", iface_name, value );
                break;
            case WIRELESS_DEVICE_HWMODE: {
                const char *hwmode = NULL;
                const char *htmode = NULL;

                hwmode = blobmsg_get_string( tb[WIRELESS_DEVICE_HWMODE] );
                if( !hwmode ) {
                    hwmode = device_opt_defvalue[WIRELESS_DEVICE_HWMODE].defvalue;
                }

                htmode = blobmsg_get_string( tb[WIRELESS_DEVICE_HTMODE] );
                if( !htmode ) {
                    htmode = device_opt_defvalue[WIRELESS_DEVICE_HTMODE].defvalue;
                }

                value = cfparse_get_wlan_mode( hwmode, htmode );
                fprintf( fp, "iwpriv %s mode %s\n", iface_name, value );

                vif->option_flags[WIRELESS_IFACE_DISABLECOEXT] = OPTION_FLAGS_NEED_CHECK;
                break;
            }
            case WIRELESS_DEVICE_SHORTGI:
                value = blobmsg_get_string( tb[WIRELESS_DEVICE_SHORTGI] );
                if( !value ) {
                    value = device_opt_defvalue[WIRELESS_DEVICE_SHORTGI].defvalue;
                }

                fprintf( fp, "iwpriv %s shortgi %s\n", iface_name, value );
#ifdef USE_CFG80211
                if ( 0 == strcmp( "0", value ) ) {
                    fprintf( fp, "cfg80211tool %s he_ar_gi_ltf 0x8ff\n", iface_name );
                }
                else {
                    fprintf( fp, "cfg80211tool %s he_ar_gi_ltf 0xffff\n", iface_name );
                }
#endif
                break;
            case WIRELESS_DEVICE_CHANNEL:
                value = blobmsg_get_string( tb[WIRELESS_DEVICE_CHANNEL] );
                if( !value ) {
                    value = device_opt_defvalue[WIRELESS_DEVICE_CHANNEL].defvalue;
                }
                fprintf( fp, "iwconfig %s channel %s\n", iface_name, value );
                fprintf( fp, "sleep 2\n");
#ifdef USE_CFG80211
                vif->option_flags[WIRELESS_IFACE_SSID] = OPTION_FLAGS_NEED_CHECK;
#endif
                break;
            case WIRELESS_DEVICE_TXPOWER: {
                const char *txpower_override_str = NULL;
                int radio = 0, txpower = 0, txpower_override  = 0;

                value = blobmsg_get_string( tb[WIRELESS_DEVICE_TXPOWER] );
                if( !value ) {
                    value = device_opt_defvalue[WIRELESS_DEVICE_TXPOWER].defvalue;
                }

                if ( strcmp( dev_name, "wifi0" ) == 0 ) {
                    radio = RADIO_2G;
                }
                else {
                    radio = RADIO_5G;
                }

                txpower_override_str = cfparse_get_tx_power_override( radio );
                txpower = atoi( value );
                if ( txpower_override_str ) {
                    txpower_override = atoi( txpower_override_str );
                }
                if ( ( txpower_override > 0 ) && ( txpower > txpower_override ) ) {
                    value = txpower_override_str;
                }

                fprintf( fp, "iwconfig %s txpower %s\n", iface_name, value );
            }
                break;
            case WIRELESS_DEVICE_MUMIMO:
                value = blobmsg_get_string( tb[WIRELESS_DEVICE_MUMIMO] );
                if( !value ) {
                    value = device_opt_defvalue[WIRELESS_DEVICE_MUMIMO].defvalue;
                }
                fprintf( fp, "iwpriv %s he_mubfer %s\n", iface_name, value );
                fprintf( fp, "iwpriv %s vhtmubfer %s\n", iface_name, value );
                fprintf( fp, "iwpriv %s he_ulmumimo %s\n", iface_name, value );
                break;
            default:
                break;
        }
    }
}

//=============================================================================
static int
cfparse_wireless_iface_hooker(
    FILE *fp,
    struct cfparse_wifi_interface *vif,
    struct blob_attr **new_attr,
    int index
)
//=============================================================================
{
    int rc = 0;
    const char *value = NULL;
    struct cfparse_wifi_interface *temp_vif;
    struct blob_attr *new_config = new_attr[index];

    value = blobmsg_get_string( new_config );
    if( !value ) {
        value = iface_opt_defvalue[index].defvalue;
    }

    switch( index ) {
        case WIRELESS_IFACE_DISABLED:
            break;
        case WIRELESS_IFACE_IFNAME:
            break;
        case WIRELESS_IFACE_NETWORK:
            vif->option_flags[WIRELESS_IFACE_SSID] = OPTION_FLAGS_NEED_CHECK;
            vif->option_flags[WIRELESS_IFACE_VLAN] = OPTION_FLAGS_NEED_CHECK;
            break;
        case WIRELESS_IFACE_MODE:
            rc = OPTION_FLAGS_NEED_RELOAD;
            break;
        case WIRELESS_IFACE_MCAST_TO_UCAST:
            break;
        case WIRELESS_IFACE_BINTVAL:
            {
                const char * bintval_str = NULL;
                int bintval = 0, max_bintval = 0;

                vlist_for_each_element( &vif->cfwdev->interfaces, temp_vif, node ) {
                    if ( !temp_vif->disabled ) {
                        continue;
                    }

                    if ( blobmsg_get_string ( new_attr[WIRELESS_IFACE_BINTVAL] ) ) {
                        bintval_str = blobmsg_get_string ( new_attr[WIRELESS_IFACE_BINTVAL] );
                    }
                    else {
                        bintval_str = iface_opt_defvalue[WIRELESS_IFACE_BINTVAL].defvalue;
                    }
                    bintval = atoi( bintval_str );
                    if ( bintval > max_bintval ) {
                        value = bintval_str;
                    }
                }

                fprintf( fp, "iwpriv %s bintval %s\n", vif->cf_section.name, value );
#ifdef USE_CFG80211
                vif->option_flags[WIRELESS_IFACE_SSID] = OPTION_FLAGS_NEED_CHECK;
#endif
            }
            break;
        case WIRELESS_IFACE_DTIM_PERIOD:
            fprintf( fp, "iwpriv %s dtim_period %s\n", vif->cf_section.name, value );
#ifdef USE_CFG80211
            vif->option_flags[WIRELESS_IFACE_SSID] = OPTION_FLAGS_NEED_CHECK;
#endif
            break;
        case WIRELESS_IFACE_PROXYARP:
            fprintf( fp, "iwpriv %s proxyarp %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_MCASTENHANCE:
            fprintf( fp, "iwpriv %s mcastenhance %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_WDS:
            fprintf( fp, "iwpriv %s wds %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_DISABLECOEXT:
            fprintf( fp, "iwpriv %s disablecoext %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_HIDDEN:
            fprintf( fp, "iwpriv %s hide_ssid %s\n", vif->cf_section.name, value );
#ifdef USE_CFG80211
            vif->option_flags[WIRELESS_IFACE_SSID] = OPTION_FLAGS_NEED_CHECK;
#endif
            break;
        case WIRELESS_IFACE_MAXSTA:
            fprintf( fp, "iwpriv %s maxsta %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_MACLIST:
            fprintf( fp, "iwpriv %s maccmd_sec 3\n", vif->cf_section.name );
            fprintf( fp, "maclist=\"%s\"\n"
                "for mac in $maclist; do\n"
                "\tiwpriv %s addmac_sec \"$mac\"\n"
                "\tiwpriv %s kickmac \"$mac\"\n"
                "done\n", value, vif->cf_section.name, vif->cf_section.name );
        case WIRELESS_IFACE_MACFILTER:
            if ( OPTION_FLAGS_CHECK_COMPL == vif->option_flags[WIRELESS_IFACE_MACFILTER] ) {
                break;
            }
            vif->option_flags[WIRELESS_IFACE_MACFILTER] = OPTION_FLAGS_CHECK_COMPL;

            if ( !value ) {
                value = iface_opt_defvalue[WIRELESS_IFACE_MACFILTER].defvalue;
            }

            if ( 0 == strcmp( "allow", value ) ) {
                fprintf( fp, "iwpriv %s maccmd_sec 1\n", vif->cf_section.name );
            }
            else if ( 0 == strcmp( "deny", value ) ) {
                fprintf( fp, "iwpriv %s maccmd_sec 2\n", vif->cf_section.name );
            }
            else {
                fprintf( fp, "iwpriv %s maccmd_sec 2\n", vif->cf_section.name );
            }
            break;
        case WIRELESS_IFACE_WMM:
            fprintf( fp, "iwpriv %s wmm %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_VHTSUBFER:
            fprintf( fp, "iwpriv %s vhtsubfer %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_VHTMUBFER:
            fprintf( fp, "iwpriv %s vhtmubfer %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_RSSI_ENABLE:
            break;
        case WIRELESS_IFACE_RSSI_THRESHOLD:
            break;
        case WIRELESS_IFACE_VOICE_ENTERPRISE:
            break;
        case WIRELESS_IFACE_11R:
            break;
        case WIRELESS_IFACE_11K:
            break;
        case WIRELESS_IFACE_WNM:
            fprintf( fp, "iwpriv %s wnm %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_ISOLATE:
            if ( 0 == strcmp( "1", value ) )
                fprintf( fp, "iwpriv %s ap_bridge 0\n", vif->cf_section.name );
            else
                fprintf( fp, "iwpriv %s ap_bridge 1\n", vif->cf_section.name );
            fprintf( fp, "/usr/sbin/wifi_isolate \n" );
            break;
        case WIRELESS_IFACE_ISOLATION_MODE:
            break;
        case WIRELESS_IFACE_GATEWAY_MAC:
            break;
        case WIRELESS_IFACE_PORTAL_ENABLE:
            break;
        case WIRELESS_IFACE_PORTAL_POLICY:
            break;
        case WIRELESS_IFACE_VLAN_TRUNK:
            fprintf( fp, "iwpriv %s vlan_trunk %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_MINIRATE_ENABLE:
        case WIRELESS_IFACE_MINIRATE_THRESHOLD:
            {
                const char *enable = NULL;
                const char *rate = NULL;

                if ( OPTION_FLAGS_CHECK_COMPL == vif->option_flags[WIRELESS_IFACE_MINIRATE_ENABLE] ) {
                    break;
                }
                vif->option_flags[WIRELESS_IFACE_MINIRATE_ENABLE] = OPTION_FLAGS_CHECK_COMPL;

                if ( blobmsg_get_string ( new_attr[WIRELESS_IFACE_MINIRATE_ENABLE] ) ) {
                    enable = blobmsg_get_string (new_attr[WIRELESS_IFACE_MINIRATE_ENABLE] );
                }
                else{
                    enable = iface_opt_defvalue[WIRELESS_IFACE_MINIRATE_ENABLE].defvalue;
                }
                if ( blobmsg_get_string ( new_attr[WIRELESS_IFACE_MINIRATE_THRESHOLD] ) ) {
                    rate = blobmsg_get_string (new_attr[WIRELESS_IFACE_MINIRATE_THRESHOLD] );
                }
                else {
                    rate = iface_opt_defvalue[WIRELESS_IFACE_MINIRATE_THRESHOLD].defvalue;
                }

                if ( 0 == strcmp( "1", enable ) )
                    fprintf( fp, "iwpriv %s min_datarate %s\n", vif->cf_section.name, rate );
                else
                    fprintf( fp, "iwpriv %s min_datarate 0\n", vif->cf_section.name );
            }
            break;
        case WIRELESS_IFACE_VLAN:

            value = blobmsg_get_string ( new_attr[WIRELESS_IFACE_NETWORK] );
            fprintf( fp, "ubus call network.interface.$( ovs-vsctl port-to-br %s | sed s/br-// ) remove_device '{\"name\": \"%s\"}'\n", vif->cf_section.name, vif->cf_section.name );
            if ( value && strlen( value ) > 0 ) {
                fprintf( fp, "ubus -t 10 wait_for network.interface.%s\n", value );
                fprintf( fp, "ubus call network.interface.%s add_device '{\"name\": \"%s\"}'\n", value, vif->cf_section.name );
            }
            vif->option_flags[WIRELESS_IFACE_VLAN] = OPTION_FLAGS_CHECK_COMPL;
            vif->option_flags[WIRELESS_IFACE_SSID] = OPTION_FLAGS_NEED_CHECK;
#ifdef GWN7062
            fprintf( fp, "iwpriv %s vlan_id %s\n", vif->cf_section.name,
                ( blobmsg_get_string ( new_attr[WIRELESS_IFACE_VLAN] ) ? blobmsg_get_string ( new_attr[WIRELESS_IFACE_VLAN] ) :
                iface_opt_defvalue[WIRELESS_IFACE_VLAN].defvalue ? iface_opt_defvalue[WIRELESS_IFACE_VLAN].defvalue : "0" ) );
#endif
            break;
        case WIRELESS_IFACE_INACT:
            fprintf( fp, "iwpriv %s inact %s\n", vif->cf_section.name, value );
            break;
        case WIRELESS_IFACE_11W:
        case WIRELESS_IFACE_SSID:
        case WIRELESS_IFACE_BSSID:
        case WIRELESS_IFACE_ENCRYPTION:
        case WIRELESS_IFACE_KEY:
        case WIRELESS_IFACE_AUTH_SERVER:
        case WIRELESS_IFACE_DYNAMIC_VLAN:
        case WIRELESS_IFACE_AUTH_PORT:
        case WIRELESS_IFACE_AUTH_SECRET:
        case WIRELESS_IFACE_ACCT_SERVER:
        case WIRELESS_IFACE_ACCT_PORT:
        case WIRELESS_IFACE_ACCT_SECRET:
        case WIRELESS_IFACE_NASID:
            {
                const char *key = NULL;
                const char *enc = NULL;

                key = blobmsg_get_string ( new_attr[WIRELESS_IFACE_KEY] );
                enc = blobmsg_get_string ( new_attr[WIRELESS_IFACE_ENCRYPTION] );
                if ( OPTION_FLAGS_CHECK_COMPL == vif->option_flags[WIRELESS_IFACE_SSID] ) {
                    break;
                }
                vif->option_flags[WIRELESS_IFACE_SSID] = OPTION_FLAGS_CHECK_COMPL;
                if ( blobmsg_get_string ( new_attr[WIRELESS_IFACE_MODE] ) ) {
                    value = blobmsg_get_string ( new_attr[WIRELESS_IFACE_MODE] );
                }
                else {
                    value = iface_opt_defvalue[WIRELESS_IFACE_MODE].defvalue;
                }

                if ( blobmsg_get_string( new_attr[WIRELESS_IFACE_NETWORK] ) &&
                    strlen( blobmsg_get_string ( new_attr[WIRELESS_IFACE_NETWORK] ) )> 0 )

                    fprintf( fp, "config_set \"%s\" bridge \"br-%s\"\n", vif->cf_section.name, blobmsg_get_string ( new_attr[WIRELESS_IFACE_NETWORK] ) );

                if ( 0 == strcmp( "ap", value ) ) {
                    fprintf( fp, "wpa_cli -g /var/run/hostapd/global raw REMOVE %s\n", vif->cf_section.name );
#ifdef USE_CFG80211
                    fprintf( fp, "hostapd_setup_vif %s nl80211 no_nconfig\n", vif->cf_section.name );
#else
                    fprintf( fp, "hostapd_setup_vif %s atheros no_nconfig\n", vif->cf_section.name );
#endif
                }
                else if ( 0 == strcmp( "sta", value ) ) {
                    fprintf( fp, "wpa_cli -g /var/run/wpa_supplicantglobal interface_remove %s\n", vif->cf_section.name );
#ifdef USE_CFG80211
                    fprintf( fp, "wpa_supplicant_setup_vif %s nl80211\n", vif->cf_section.name );
#else
                    fprintf( fp, "wpa_supplicant_setup_vif %s athr\n", vif->cf_section.name );
#endif
                }
                if ( enc && 0 == strcmp( "wep", enc ) ) {
                    char buf[BUFF_SIZE] = "";

                    util_escape_single_quote( key, buf, BUFF_SIZE );
                    fprintf( fp, "iwconfig %s enc '%s'\n", vif->cf_section.name, buf );
                }
            }
            break;
        case WIRELESS_IFACE_UAPSD:
            fprintf( fp, "iwpriv %s uapsd %s\n", vif->cf_section.name, value );
            break;
        default:
            break;
    }

    rc |= OPTION_FLAGS_CONFIG_CHANGED;
    return rc;
}

//=============================================================================
static int
cfparse_wireless_device_hooker(
    FILE *fp,
    struct cfparse_wifi_device *cfwdev,
    struct blob_attr **new_attr,
    int index
)
//=============================================================================
{
    int rc = 0, country = 0;
    const char *value;
    struct blob_attr *glob_tb[__WIRELESS_GLOBAL_MAX];
    struct cfparse_wifi_global *cfwgl;
    struct blob_attr *new_config = new_attr[index];

    value = blobmsg_get_string( new_config );
    if( !value ) {
        value = device_opt_defvalue[index].defvalue;
    }

    switch( index ) {
        case WIRELESS_DEVICE_DISABLED:
            rc = OPTION_FLAGS_NEED_RELOAD;
            break;
        case WIRELESS_DEVICE_PHY:
            break;
        case WIRELESS_DEVICE_OUTDOOR:
            fprintf( fp, "iwpriv %s outdoor %s\n", cfwdev->cf_section.name, value );
            if ( util_device_is_world_model() ) {

                cfwgl= util_get_vltree_node( &wireless_global_vltree, VLTREE_DEV_GLOB, "qcawifi" );
                if( cfwgl ) {
                    blobmsg_parse( wireless_global_policy,
                        __WIRELESS_GLOBAL_MAX,
                        glob_tb,
                        blob_data( cfwgl->cf_section.config  ),
                        blob_len( cfwgl->cf_section.config  ) );

                    if( blobmsg_get_string( glob_tb[WIRELESS_GLOBAL_COUNTRY] ) )
                        country = atoi( blobmsg_get_string( glob_tb[WIRELESS_GLOBAL_COUNTRY] ) );
                    else
                        country = atoi( global_opt_defvalue[WIRELESS_GLOBAL_COUNTRY].defvalue );
                }
                else {
                    country = atoi( global_opt_defvalue[WIRELESS_GLOBAL_COUNTRY].defvalue );
                }
                fprintf( fp, "iwpriv %s setCountryID %d\n", cfwdev->cf_section.name, country );
            }
            break;
        case WIRELESS_DEVICE_PUREG: {
            cfwdev->option_flags[WIRELESS_DEVICE_PUREG] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;

            break;
        }
        case WIRELESS_DEVICE_HWMODE:
        case WIRELESS_DEVICE_HTMODE:
            cfwdev->option_flags[WIRELESS_DEVICE_HWMODE] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->option_flags[WIRELESS_DEVICE_CHANNEL] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;

            break;
        case WIRELESS_DEVICE_SHORTGI:
            cfwdev->option_flags[WIRELESS_DEVICE_SHORTGI] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;

            break;
        case WIRELESS_DEVICE_TXCHAINMASK:
            cfwdev->option_flags[WIRELESS_DEVICE_CHANNEL] = OPTION_FLAGS_NEED_CHECK;
            fprintf( fp, "iwpriv %s txchainmask %s\n", cfwdev->cf_section.name, value );
            break;
        case WIRELESS_DEVICE_RXCHAINMASK:
            cfwdev->option_flags[WIRELESS_DEVICE_CHANNEL] = OPTION_FLAGS_NEED_CHECK;
            fprintf( fp, "iwpriv %s rxchainmask %s\n", cfwdev->cf_section.name, value );
            break;
        case WIRELESS_DEVICE_DBDC_ENABLE:
            fprintf( fp, "iwpriv %s dbdc_enable %s\n", cfwdev->cf_section.name, value );
            break;
        case WIRELESS_DEVICE_CHANNEL:
            cfwdev->option_flags[WIRELESS_DEVICE_CHANNEL] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;

            cfwdev->option_flags[WIRELESS_DEVICE_TXPOWER] = OPTION_FLAGS_NEED_CHECK;
            break;
        case WIRELESS_DEVICE_TXPOWER:
            cfwdev->option_flags[WIRELESS_DEVICE_TXPOWER] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
        case WIRELESS_DEVICE_MUMIMO:
            cfwdev->option_flags[WIRELESS_DEVICE_MUMIMO] = OPTION_FLAGS_NEED_CHECK;
            cfwdev->need_set_iface = true;
            break;
    }

    rc |= OPTION_FLAGS_CONFIG_CHANGED;

    return rc;
}

//=============================================================================

static int
cfparse_wireless_global_hooker(
    FILE *fp,
    struct cfparse_wifi_global *cfwgl,
    struct blob_attr *new_config,
    int index
)
//=============================================================================
{
    int rc = 0;
    const char *value = NULL;
    struct cfparse_wifi_device *cfwdev;

    value = blobmsg_get_string( new_config );

    switch( index ) {
        case WIRELESS_GLOBAL_ATF_MODE:
            break;
        case WIRELESS_GLOBAL_DFS:
            if ( 0 == strcmp( "1", value ) )
                fprintf( fp, "iwpriv wifi1 blockdfslist 0\n" );
            else
                fprintf( fp, "iwpriv wifi1 blockdfslist 1\n" );
            cfwgl->option_flags[WIRELESS_GLOBAL_COUNTRY] |= OPTION_FLAGS_NEED_CHECK;
            break;
        case WIRELESS_GLOBAL_COUNTRY:
            if( util_device_is_world_model( ) ) {
                fprintf( fp, "[ -e /lib/wifi/country.sh ] && . /lib/wifi/country.sh\n");
                fprintf( fp, "config_set_dfs wifi0 %s\n", value );
                fprintf( fp, "config_set_dfs wifi1 %s\n", value );
                while ( *value == '0' ) {
                    value++;
                }
                fprintf( fp, "iwpriv wifi0 setCountryID %s\n", value );
                fprintf( fp, "iwpriv wifi1 setCountryID %s\n", value );
                vlist_for_each_element( &wireless_vltree, cfwdev, node ) {
                    cfwdev->option_flags[WIRELESS_DEVICE_TXPOWER] = OPTION_FLAGS_NEED_CHECK;
                }
            }
            break;
    };

    rc |= OPTION_FLAGS_CONFIG_CHANGED;
    return rc;
}

#endif

//=============================================================================
static void
cfparse_wifi_global_update(
    struct cfparse_wifi_global *old_cfwgl,
    struct cfparse_wifi_global *new_cfwgl
)
//=============================================================================
{
    int i = 0;
    struct blob_attr *new_config = new_cfwgl->cf_section.config;
    struct blob_attr *old_config = old_cfwgl->cf_section.config;
    struct blob_attr *tb_old[__WIRELESS_GLOBAL_MAX];
    struct blob_attr *tb_new[__WIRELESS_GLOBAL_MAX];

    blobmsg_parse( wireless_global_policy,
        __WIRELESS_GLOBAL_MAX,
        tb_new,
        blob_data( new_config ),
        blob_len( new_config ) );

    blobmsg_parse( wireless_global_policy,
        __WIRELESS_GLOBAL_MAX,
        tb_old,
        blob_data( old_config ),
        blob_len( old_config ) );

    for ( i = 0; i < __WIRELESS_GLOBAL_MAX; i++ ) {
        if ( !blob_attr_equal( tb_old[i], tb_new[i] ) || OPTION_FLAGS_NEED_CHECK == new_cfwgl->option_flags[i] ) {
            options |= cfparse_wireless_global_hooker( fp, new_cfwgl, tb_new[i], i );
        }
    }
}

//=============================================================================
static void
cfparse_wifi_device_update(
    struct cfparse_wifi_device *cfwdev_old,
    struct cfparse_wifi_device *cfwdev_new
)
//=============================================================================
{
    int i = 0;
    struct blob_attr *new_config = cfwdev_new->cf_section.config;
    struct blob_attr *old_config = cfwdev_old->cf_section.config;

    struct blob_attr *tb_new[__WIRELESS_DEVICE_MAX];
    struct blob_attr *tb_old[__WIRELESS_DEVICE_MAX];

    blobmsg_parse( wireless_device_policy,
                   __WIRELESS_DEVICE_MAX,
                   tb_new,
                   blob_data( new_config ),
                   blob_len( new_config ) );


    blobmsg_parse( wireless_device_policy,
                   __WIRELESS_DEVICE_MAX,
                   tb_old,
                   blob_data( old_config ),
                   blob_len( old_config ) );


    for ( i = 0; i < __WIRELESS_DEVICE_MAX; i++ ) {

        if ( !blob_attr_equal( tb_old[i], tb_new[i] ) || OPTION_FLAGS_NEED_CHECK == cfwdev_new->option_flags[i] ) {
            options |= cfparse_wireless_device_hooker( fp, cfwdev_new, tb_new, i );
        }
    }

#ifdef CONFIG_MTK
    if( 0 == strcmp( "wifi0", cfwdev_new->cf_section.name ) && cfwdev_new->flags & BIT(OPTION_FLAGS_START_8021XD) ) {
        fprintf( fp, "killall 8021xd > /dev/null; 8021xd -d 9 >> /tmp/802.wifi0.log 2>&1\n" );
    }
    if( 0 == strcmp( "wifi1", cfwdev_new->cf_section.name ) && cfwdev_new->flags & BIT(OPTION_FLAGS_START_8021XD) ) {
        fprintf( fp, "killall 8021xdi > /dev/null; 8021xdi -d 9 >> /tmp/802.wifi1.log 2>&1\n" );
    }
#endif
}

//=============================================================================
static void
cfparse_wifi_iface_update(
    struct cfparse_wifi_interface *vif_old,
    struct cfparse_wifi_interface *vif_new,
    bool need_set_device
)
//=============================================================================
{
    int i = 0;
    struct blob_attr *new_config = vif_new->cf_section.config;
    struct blob_attr *old_config = vif_old->cf_section.config;
    struct blob_attr *tb_new[__WIRELESS_IFACE_MAX];
    struct blob_attr *tb_old[__WIRELESS_IFACE_MAX];

    blobmsg_parse( wireless_iface_policy,
                   __WIRELESS_IFACE_MAX,
                   tb_new,
                   blob_data( new_config ),
                   blob_len( new_config ) );


    blobmsg_parse( wireless_iface_policy,
                   __WIRELESS_IFACE_MAX,
                   tb_old,
                   blob_data( old_config ),
                   blob_len( old_config ) );

    if( need_set_device ) {
        //Bug 192689
        cfparse_set_device_by_iface( fp, vif_new );
        options |= OPTION_FLAGS_CONFIG_CHANGED;
    }

    for ( i = 0; i < __WIRELESS_IFACE_MAX; i++ ) {
        if ( !blob_attr_equal( tb_old[i], tb_new[i] ) || OPTION_FLAGS_NEED_CHECK == vif_new->option_flags[i] ) {
            options |= cfparse_wireless_iface_hooker( fp, vif_new, tb_new, i );
        }
    }

#ifdef CONFIG_MTK
    if( vif_new->flags & BIT(OPTION_FLAGS_NEED_UP) ) {
        fprintf( fp, "ifconfig %s up\n", vif_new->cf_section.name );
    }
#endif
}

//=============================================================================
void
cfparse_vif_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct cfparse_wifi_interface *vif_old = NULL;
    struct cfparse_wifi_interface *vif_new = NULL;
    struct cfparse_wifi_device *cfwdev = NULL;

    if ( node_old )
        vif_old =
            container_of(node_old, struct cfparse_wifi_interface, node);

    if ( node_new )
        vif_new =
            container_of(node_new, struct cfparse_wifi_interface, node);

    if ( vif_old )
        cfwdev = vif_old->cfwdev;
    else
        cfwdev = vif_new->cfwdev;

    if ( vif_old && vif_new ) {

        if ( blob_attr_equal( vif_old->cf_section.config, vif_new->cf_section.config ) &&
                !vif_new->cfwdev->need_set_iface ) {
            SAFE_FREE( vif_new->maclist );
            free( vif_new->cf_section.config );
            free( vif_new );
            return;
        }

        cfmanager_log_message( L_WARNING,
            "Update wireless interface %s on device %s\n", vif_new->cf_section.name, cfwdev->cf_section.name );

        //free( vif_old->cf_section.config );
        //vif_old->config = blob_memdup( vif_new->config );

        cfparse_wifi_iface_update( vif_old, vif_new, vif_new->cfwdev->need_set_iface );

        SAFE_FREE( vif_old->maclist );
        if ( vif_new->maclist )
            vif_old->maclist = strdup( vif_new->maclist );

        SAFE_FREE( vif_old->cf_section.config );
        vif_old->cf_section.config = blob_memdup( vif_new->cf_section.config );
        vif_old->disabled = vif_new->disabled;
        vif_old->network_type = vif_new->network_type;
        snprintf( vif_old->ssid_name, sizeof( vif_old->ssid_name ), "%s", vif_new->ssid_name );
        vif_old->sae = vif_new->sae;
        vif_old->disablecoext = vif_new->disablecoext;
        memset( vif_old->ssid_id, 0, sizeof( vif_old->ssid_id ) );
        strncpy( vif_old->ssid_id, vif_new->ssid_id, sizeof( vif_old->ssid_id ) -1 );
        vif_old->rssi_enable = vif_new->rssi_enable;
        vif_old->minirate_enable = vif_new->minirate_enable;

        SAFE_FREE( vif_new->maclist );
        free( vif_new->cf_section.config );
        free( vif_new );
    }
    else if ( vif_new ) {
        options |= OPTION_FLAGS_NEED_RELOAD;

        cfmanager_log_message( L_WARNING,
            "Create new wireless interface %s on device %s\n", vif_new->cf_section.name, cfwdev->cf_section.name );
    }
    else if ( vif_old ) {
        options |= OPTION_FLAGS_NEED_RELOAD;

        cfmanager_log_message( L_WARNING,
            "Delete wireless interface %s on device %s\n", vif_old->cf_section.name, cfwdev->cf_section.name );

        SAFE_FREE( vif_old->maclist );
        free( vif_old->cf_section.config );
        free( vif_old );
    }
}

//=============================================================================
static void
cfparse_wifi_device_free(
    struct cfparse_wifi_device *cfwdev
)
//=============================================================================
{
    SAFE_FREE( cfwdev->cf_section.config );
    SAFE_FREE( cfwdev );
}

//=============================================================================
static void
cfparse_wifi_global_free(
    struct cfparse_wifi_global *cfwgl
)
//=============================================================================
{
    SAFE_FREE( cfwgl->cf_section.config );
    SAFE_FREE( cfwgl );
}

//=============================================================================
static void
cfparse_wireless_tree_free(
    struct cfparse_wifi_device *cfwdev
)
//=============================================================================
{
    vlist_flush_all( &cfwdev->interfaces );
    avl_delete( &wireless_vltree.avl, &cfwdev->node.avl );

    cfparse_wifi_device_free( cfwdev );
}

//=============================================================================
static void
cfparse_wireless_global_tree_free(
    struct cfparse_wifi_global *cfwgl
)
//=============================================================================
{
    avl_delete( &wireless_global_vltree.avl, &cfwgl->node.avl );

    cfparse_wifi_global_free( cfwgl );
}

//=============================================================================
void
cfparse_wireless_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct cfparse_wifi_device *cfwdev_old = NULL;
    struct cfparse_wifi_device *cfwdev_new = NULL;

    if ( node_old )
        cfwdev_old =
            container_of(node_old, struct cfparse_wifi_device, node);

    if ( node_new )
        cfwdev_new =
            container_of(node_new, struct cfparse_wifi_device, node);

    if ( cfwdev_old && cfwdev_new ) {
        cfmanager_log_message( L_WARNING,
            "update wireless config section '%s'\n", cfwdev_old->cf_section.name );

        if ( blob_attr_equal( cfwdev_old->cf_section.config, cfwdev_new->cf_section.config ) ) {
            cfparse_wifi_device_free( cfwdev_new );
            return;
        }

        if ( cfwdev_old->disabled == cfwdev_new->disabled
            && cfwdev_old->disabled ) {
            //Does it need to be set in closed state?
        }
        else {
            cfwdev_old->disabled = cfwdev_new->disabled;
            cfparse_wifi_device_update( cfwdev_old, cfwdev_new );
        }

        SAFE_FREE( cfwdev_old->cf_section.config );
        cfwdev_old->cf_section.config = blob_memdup( cfwdev_new->cf_section.config );
        cfwdev_old->disabled = cfwdev_new->disabled;
        memcpy( cfwdev_old->option_flags, cfwdev_new->option_flags, __WIRELESS_DEVICE_MAX );
        cfwdev_old->need_set_iface = cfwdev_new->need_set_iface;
        memset( cfwdev_old->htmode, 0, sizeof( cfwdev_old->htmode ) ) ;
        strncpy( cfwdev_old->htmode, cfwdev_new->htmode, sizeof( cfwdev_old->htmode ) -1 );

        cfparse_wifi_device_free( cfwdev_new );
    }
    else if ( cfwdev_old ) {
        options |= OPTION_FLAGS_NEED_RELOAD;

        cfmanager_log_message( L_WARNING,
            "Delete wireless config section '%s'\n", cfwdev_old->cf_section.name );

        cfparse_wireless_tree_free( cfwdev_old );
    }
    else if ( cfwdev_new ) {
        options |= OPTION_FLAGS_NEED_RELOAD;
        cfmanager_log_message( L_WARNING,
                "New wireless config section '%s'\n", cfwdev_new->cf_section.name );

    }
}

//=============================================================================
void
cfparse_wireless_global_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct cfparse_wifi_global *cfwgl_old = NULL;
    struct cfparse_wifi_global *cfwgl_new = NULL;

    if ( node_old )
        cfwgl_old =
            container_of(node_old, struct cfparse_wifi_global, node);

    if ( node_new )
        cfwgl_new =
            container_of(node_new, struct cfparse_wifi_global, node);

    if ( cfwgl_old && cfwgl_new ) {
        cfmanager_log_message( L_WARNING,
            "update wireless config section '%s'\n", cfwgl_old->cf_section.name );

        if ( blob_attr_equal( cfwgl_old->cf_section.config, cfwgl_new->cf_section.config ) ) {
            cfparse_wifi_global_free( cfwgl_new );
            return;
        }

        cfparse_wifi_global_update( cfwgl_old, cfwgl_new );

        SAFE_FREE( cfwgl_old->cf_section.config );
        cfwgl_old->cf_section.config = blob_memdup( cfwgl_new->cf_section.config );

        memset( cfwgl_old->country, 0, sizeof( cfwgl_old->country ) );
        strncpy( cfwgl_old->country, cfwgl_new->country, sizeof( cfwgl_old->country ) -1 );

        cfparse_wifi_global_free( cfwgl_new );
    }
    else if ( cfwgl_old ) {
        options |= OPTION_FLAGS_NEED_RELOAD;
        cfmanager_log_message( L_WARNING,
            "Delete wireless config section '%s'\n", cfwgl_old->cf_section.name );

        cfparse_wireless_global_tree_free( cfwgl_old );
    }
    else if ( cfwgl_new ) {
        options |= OPTION_FLAGS_NEED_RELOAD;
        cfmanager_log_message( L_WARNING,
                "New wireless config section '%s'\n", cfwgl_new->cf_section.name );

    }
}

//=============================================================================
void
cfparse_wifi_global_parse(
    struct blob_attr *data,
    const char *section
)
//=============================================================================
{
    struct cfparse_wifi_global *cfwgl;
    char *name_buf;
    struct blob_attr *tb[__WIRELESS_GLOBAL_MAX];

    cfwgl = calloc_a( sizeof( *cfwgl ), &name_buf, strlen( section ) + 1 );
    if ( !cfwgl ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc cfparse_wifi_global '%s'\n", section );

        return;
    }

    blobmsg_parse( wireless_global_policy,
        __WIRELESS_GLOBAL_MAX,
        tb,
        blob_data( data ),
        blob_len( data ) );

    if( tb[WIRELESS_GLOBAL_COUNTRY] ) {
        strncpy( cfwgl->country, blobmsg_get_string( tb[WIRELESS_GLOBAL_COUNTRY] ),
            sizeof( cfwgl->country ) -1 );
    }
    cfwgl->cf_section.name = strcpy( name_buf, section );
    cfwgl->cf_section.config = blob_memdup( data );

    vlist_add( &wireless_global_vltree, &cfwgl->node, cfwgl->cf_section.name );
}

//=============================================================================
static void
cfparse_wifi_device_parse(
    struct blob_attr *data,
    const char *section
)
//=============================================================================
{
    struct cfparse_wifi_device *cfwdev;
    char *name_buf;

    struct blob_attr *tb[__WIRELESS_DEVICE_MAX];

    blobmsg_parse( wireless_device_policy,
                   __WIRELESS_DEVICE_MAX,
                   tb,
                   blob_data( data ),
                   blob_len( data ) );


    cfwdev = calloc_a( sizeof( *cfwdev ), &name_buf, strlen( section ) + 1 );
    if ( !cfwdev ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc cfparse_wifi_device '%s'\n", section );

        return;
    }

    if ( tb[WIRELESS_DEVICE_DISABLED] ) {
        cfwdev->disabled = blobmsg_get_bool( tb[WIRELESS_DEVICE_DISABLED] );
    }

    if( tb[WIRELESS_DEVICE_HTMODE] ) {
        strncpy( cfwdev->htmode, blobmsg_get_string( tb[WIRELESS_DEVICE_HTMODE] ),
            sizeof( cfwdev->htmode ) -1 );
    }

    cfwdev->cf_section.name = strcpy( name_buf, section );
    cfwdev->cf_section.config = blob_memdup( data );

    vlist_init( &cfwdev->interfaces, avl_strcmp, cfparse_vif_update );
    cfwdev->interfaces.keep_old = true;

    vlist_add( &wireless_vltree, &cfwdev->node, cfwdev->cf_section.name );
}

//=============================================================================
static void
cfparse_wifi_iface_parse(
    struct cfparse_wifi_device *cfwdev,
    struct blob_attr *data,
    const char *section
)
//=============================================================================
{
    struct cfparse_wifi_interface *vif;
    struct blob_attr *tb[__WIRELESS_IFACE_MAX];
    struct blob_attr *cur;
    char *name_buf;

    blobmsg_parse( wireless_iface_policy,
                   __WIRELESS_IFACE_MAX,
                   tb,
                   blob_data( data ),
                   blob_len( data ) );


    vif = calloc_a( sizeof( *vif ),
               &name_buf, strlen( section ) + 1 );
    if ( !vif ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc cfparse_wifi_interface '%s'\n", section );

        return;
    }

    vif->cf_section.name = strcpy( name_buf, section );

    cfwdev->vif_idx++;
    cur = tb[WIRELESS_IFACE_DISABLED];
    if ( cur ) {
        vif->disabled = blobmsg_get_bool( tb[WIRELESS_IFACE_DISABLED] );
    }

    cur = tb[WIRELESS_IFACE_NETWORK_TYPE];
    if( cur ) {
        if( 0 == strcmp( blobmsg_get_string( cur ), NET_MASTER ) ) {
            vif->network_type = NET_TYPE_MASTER;
        }
        else if( 0 == strcmp( blobmsg_get_string( cur ), NET_GUEST ) ) {
            vif->network_type = NET_TYPE_GUEST;
        }
        else if( 0 == strcmp( blobmsg_get_string( cur ), NET_MESH ) ) {
            vif->network_type = NET_TYPE_MESH;
        }
        else if( 0 == strcmp( blobmsg_get_string( cur ), NET_ADDIT ) ) {
            vif->network_type = NET_TYPE_ADDIT;
        }
    }

    cur = tb[WIRELESS_IFACE_SSID];
    if( cur ) {
        strncpy( vif->ssid_name, blobmsg_get_string( cur ), SSID_NAME_MAX_LEN );
    }

    cur = tb[WIRELESS_IFACE_SAE];
    if( cur ) {
        vif->sae = true;
    }

    cur = tb[WIRELESS_IFACE_MACLIST];
    if ( cur ) {
        vif->maclist = strdup( blobmsg_get_string( cur ) );
    }

    cur = tb[WIRELESS_IFACE_MACFILTER];
    if ( cur ) {
        strncpy( vif->macfilter, blobmsg_get_string( cur ), MACFILTER_MAX_LEN );
    }

    cur = tb[WIRELESS_IFACE_DISABLECOEXT];
    if( cur ) {
        vif->disablecoext = true;
    }

    cur = tb[WIRELESS_IFACE_ID];
    if( cur ) {
        strncpy( vif->ssid_id, blobmsg_get_string( cur ), sizeof( vif->ssid_id ) -1 );
    }

    cur = tb[WIRELESS_IFACE_RSSI_ENABLE];
    if( cur && 0 == strcmp( blobmsg_get_string( cur ), "1" ) ) {
        vif->rssi_enable = true;
    }
    else {
        vif->rssi_enable = false;
    }

    cur = tb[WIRELESS_IFACE_MINIRATE_ENABLE];
    if( cur && 0 == strcmp( blobmsg_get_string( cur ), "1" ) ) {
        vif->minirate_enable = true;
    }
    else {
        vif->minirate_enable = false;
    }

    vif->cfwdev = cfwdev;
    vif->cf_section.config = blob_memdup( data );

#ifdef CONFIG_MTK
    if( 0 == strcmp( "wifi0", cfwdev->cf_section.name ) ) {
        ifname_2g = vif->cf_section.name;
    }
    else {
        ifname_5g = vif->cf_section.name;
    }
#endif

    vlist_add( &cfwdev->interfaces, &vif->node, vif->cf_section.name );
}

//=============================================================================
static void
cfparse_wireless_global(
    struct uci_section *s
)
//=============================================================================
{
    blob_buf_init( &b, 0 );
    uci_to_blob( &b, s, &wireless_global_list );
    cfparse_wifi_global_parse( b.head, s->e.name );
    blob_buf_free( &b );
}

//=============================================================================
static void
cfparse_wireless_device(
    struct uci_section *s
)
//=============================================================================
{
    blob_buf_init( &b, 0 );
    uci_to_blob( &b, s, &wireless_device_list );
    cfparse_wifi_device_parse( b.head, s->e.name );
    blob_buf_free( &b );
}

//=============================================================================
static void
cfparse_wireless_interface(
    struct cfparse_wifi_device *cfwdev,
    struct uci_section *s
)
//=============================================================================

{
    blob_buf_init( &b, 0 );
    uci_to_blob( &b, s, &wireless_iface_list );
    cfparse_wifi_iface_parse( cfwdev, b.head, s->e.name );
    blob_buf_free( &b );
}

//=============================================================================
int
cfparse_load_wireless(
    void
)
//=============================================================================
{
    struct uci_element *e;
    struct cfparse_wifi_device *cfwdev;
    const char *dev_name = NULL;
    struct uci_package *package = NULL;
    char command[COMMAND_LEN] = { 0 };

    check_set_defvalue( CHECK_WIRELESS );

    package = cfparse_init_package( "wireless" );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load wireless package failed\n" );
        return -1;
    }

    fp = fopen( WIRELESS_SCRIPT_NAME, "wb" );
    if( !fp ) {
        cfmanager_log_message( L_ERR, "create file: %s failed\n", WIRELESS_SCRIPT_NAME );
        options |= OPTION_FLAGS_NEED_RELOAD;
        goto out;
    }

    fprintf( fp, "#!/bin/sh\n"
             ". /lib/functions.sh\n"
             "[ -e /lib/wifi/hostapd.sh ] && . /lib/wifi/hostapd.sh\n"
             "[ -e /lib/wifi/wpa_supplicant.sh ] && . /lib/wifi/wpa_supplicant.sh\n"
             "[ -e /lib/netifd/netifd-wireless.sh ] && . /lib/netifd/netifd-wireless.sh\n"
             "lock /var/run/wifi.lock\n"
             "config_load wireless\n" );

    vlist_update( &wireless_vltree );
    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( strcmp(s->type, "wifi-device") != 0 )
            continue;
        cfparse_wireless_device( s );
    }

    vlist_flush( &wireless_vltree );

    vlist_for_each_element( &wireless_vltree, cfwdev, node ) {
        cfwdev->vif_idx = 0;
        vlist_update( &cfwdev->interfaces );
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if (strcmp(s->type, "wifi-iface") != 0)
            continue;

        dev_name = uci_lookup_option_string( uci_ctx, s, "device" );
        if ( !dev_name )
            continue;

        cfwdev = vlist_find( &wireless_vltree, dev_name, cfwdev, node );
        if ( !cfwdev ) {
            cfmanager_log_message( L_ERR,
                "device %s not found!\n", dev_name );

            continue;
        }

        cfparse_wireless_interface( cfwdev, s );
    }

    vlist_update( &wireless_global_vltree );
    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( strcmp( s->type, "qcawifi") == 0 ) {
            cfparse_wireless_global( s );
            break;
        }
    }
    vlist_flush( &wireless_global_vltree );

    vlist_for_each_element( &wireless_vltree, cfwdev, node ) {
        vlist_flush( &cfwdev->interfaces );
        /* Here, the corresponding device modification has taken effect,
         * and the corresponding flag should be cleared
         */
        memset( cfwdev->option_flags, 0, sizeof( cfwdev->option_flags ) );
        cfwdev->need_set_iface = false;
    }

out:
    if( fp ) {
        fprintf( fp, "/etc/init.d/lbd reload\n" );
        fprintf( fp, "lock -u /var/run/wifi.lock\n" );
        fclose( fp );
    }

    if( options & OPTION_FLAGS_NEED_RELOAD ) {
        apply_add( "wireless" );
        apply_timer_start();
    }
    else if ( options & OPTION_FLAGS_CONFIG_CHANGED ) {
        snprintf( command, sizeof( command ), "sh -x %s;", WIRELESS_SCRIPT_NAME );

        config_sync();

        cfmanager_log_message( L_DEBUG, "system( %s )\n",  command );
        system( command );
    }

    options = 0;

    return 0;
}

//=============================================================================
void
cfparse_wireless_init(
    void
)
//=============================================================================
{
    vlist_init( &wireless_vltree, avl_strcmp, cfparse_wireless_update );
    wireless_vltree.keep_old = true;
    wireless_vltree.no_delete = true;


    vlist_init( &wireless_global_vltree, avl_strcmp, cfparse_wireless_global_update );
    wireless_global_vltree.keep_old = true;
    wireless_global_vltree.no_delete = true;
}

//=============================================================================
void
cfparse_wireless_deinit(
    void
)
//=============================================================================
{
    struct cfparse_wifi_device *cfwdev;

    vlist_for_each_element( &wireless_vltree, cfwdev, node ) {
        vlist_flush_all( &cfwdev->interfaces );
    }
    vlist_flush_all( &wireless_vltree );

    vlist_flush_all( &wireless_global_vltree );
}

