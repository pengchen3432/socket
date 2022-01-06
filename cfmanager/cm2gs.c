/****************************************************************************
* *
* * FILENAME:        $RCSfile: cm2gs.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/11/21
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
#include "gs_utils.h"
#include "sgrequest.h"
#include "grandstream.h"
#include "cfmanager.h"
#include "cm2gs.h"

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================

//=================
//  Globals
//=================
extern const struct blobmsg_policy cm_email_policy[_CM_EMAIL_MAX];
extern const struct blobmsg_policy cm_extern_sys_log_policy[_EXTERNAL_LOG_MAX];
extern const struct blobmsg_policy cm_notification_policy[_CM_NOTIFY_MAX];
extern const struct blobmsg_policy cm_ap_policy[__CM_AP_MAX];
extern const struct blobmsg_policy cm_addit_ssid_policy[__CM_ADDIT_SSID_MAX];
extern const struct blobmsg_policy cm_radio_policy[__CM_RADIO_MAX];
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

/*
    This function can be called if the value in cfmanager is the same as grandstream
*/
//=============================================================================
static void
cm2gs_set_gs_config(
    const char *gs_section_name,
    struct blob_attr **cm_attr,
    const struct blobmsg_policy *gs_policy,
    const struct blobmsg_policy *cm_policy,
    int gs_index,
    int cm_index
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };
    struct blob_attr *cm_data = cm_attr[cm_index];

    if( !gs_section_name || !gs_policy ||!cm_policy ) {
        return;
    }

    snprintf( path, sizeof(path), "%s.%s.%s", CF_CONFIG_NAME_GRANDSTREAM,
            gs_section_name, gs_policy[gs_index].name );
    config_set_by_blob( cm_data, path, cm_policy[cm_index].type );
}

//=============================================================================
int
cm2gs_extern_sys_log_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
)
//=============================================================================
{
    char *section_name = extend->section_name;
    int rc = 0;
    
    rc |= BIT( CM_CFG_GRANDSTREAM );
    cm2gs_set_gs_config( section_name, cm_attr, gs_extern_sys_log_policy, cm_extern_sys_log_policy, GS_ISSUE_AP_LIST, CM_ISSUE_AP_LIST );
    cm2gs_set_gs_config( section_name, cm_attr, gs_extern_sys_log_policy, cm_extern_sys_log_policy, GS_EXTERN_LOG_URI, CM_EXTERN_LOG_URI );
    cm2gs_set_gs_config( section_name, cm_attr, gs_extern_sys_log_policy, cm_extern_sys_log_policy, GS_EXTERN_LOG_LEVEL, CM_EXTERN_LOG_LEVEL );
    cm2gs_set_gs_config( section_name, cm_attr, gs_extern_sys_log_policy, cm_extern_sys_log_policy, GS_EXTERN_LOG_PROTOCOL, CM_EXTERN_LOG_PROTOCOL );
    return rc;
}

//=============================================================================
int
cm2gs_email_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
)
//=============================================================================
{
    struct grandstream_config_parse *gs_cfg = NULL;
    char *section_name = extend->section_name;
    int rc = 0;

    gs_cfg = util_get_vltree_node( &grandstream_email_vltree, VLTREE_GRANDSTREAM, section_name );
    if( gs_cfg ) {
        config_del_named_section( CF_CONFIG_NAME_GRANDSTREAM, "email", section_name );
    }

    rc |= BIT( CM_CFG_GRANDSTREAM );

    if( VLTREE_ACTION_DEL == extend->action ) {
        return rc;
    }

    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "email", section_name );
    cm2gs_set_gs_config( section_name, cm_attr, gs_email_policy, cm_email_policy, GS_EMAIL_PORT, CM_EMAIL_PORT );
    cm2gs_set_gs_config( section_name, cm_attr, gs_email_policy, cm_email_policy, GS_EMAIL_HOST, CM_EMAIL_HOST );
    cm2gs_set_gs_config( section_name, cm_attr, gs_email_policy, cm_email_policy, GS_EMAIL_USER, CM_EMAIL_USER );
    cm2gs_set_gs_config( section_name, cm_attr, gs_email_policy, cm_email_policy, GS_EMAIL_PASSWORD, CM_EMAIL_PASSWORD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_email_policy, cm_email_policy, GS_EMAIL_DO_NOT_VALIDATE, CM_EMAIL_DO_NOT_VALIDATE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_email_policy, cm_email_policy, GS_EMAIL_ENABLE_NOTIFICATION, CM_EMAIL_ENABLE_NOTIFICATION );
    cm2gs_set_gs_config( section_name, cm_attr, gs_email_policy, cm_email_policy, GS_EMAIL_FROM_ADDRESS, CM_EMAIL_FROM_ADDRESS );
    cm2gs_set_gs_config( section_name, cm_attr, gs_email_policy, cm_email_policy, GS_EMAIL_FROM_NAME, CM_EMAIL_FROM_NAME );
    cm2gs_set_gs_config( section_name, cm_attr, gs_email_policy, cm_email_policy, GS_EMAIL_EMAILADDRESS, CM_EMAIL_EMAILADDRESS );

    return rc;
}

//=============================================================================
int
cm2gs_notification_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
)
//=============================================================================
{
    struct grandstream_config_parse *gs_cfg = NULL;
    char *section_name = extend->section_name;
    int rc = 0;

    gs_cfg = util_get_vltree_node( &grandstream_notification_vltree, VLTREE_GRANDSTREAM, section_name );
    if( gs_cfg ) {
        config_del_named_section( CF_CONFIG_NAME_GRANDSTREAM, "notification", section_name );
    }

    rc |= BIT( CM_CFG_GRANDSTREAM );

    if( VLTREE_ACTION_DEL == extend->action ) {
        return rc;
    }

    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "notification", section_name );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_NOTIFY_CM_MEMORY_USAGE, CM_NOTIFY_CM_MEMORY_USAGE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_MEMORY_USAGE_THRESHOLD, CM_MEMORY_USAGE_THRESHOLD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_NOTIFY_AP_THROUGHPUT, CM_NOTIFY_AP_THROUGHPUT );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_AP_THROUGHPUT_THRESHOLD, CM_AP_THROUGHPUT_THRESHOLD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_NOTIFY_SSID_THROUGHPUT, CM_NOTIFY_SSID_THROUGHPUT );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_SSID_THROUGHPUT_THRESHOLD, CM_SSID_THROUGHPUT_THRESHOLD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_NOTIFY_PASSWORD_CHANGE, CM_NOTIFY_PASSWORD_CHANGE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_NOTIFY_FIRMWARE_UPGRADE, CM_NOTIFY_FIRMWARE_UPGRADE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_NOTIFY_AP_OFFLINE, CM_NOTIFY_AP_OFFLINE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_notification_policy, cm_notification_policy, GS_NOTIFY_FIND_ROGUEAP, CM_NOTIFY_FIND_ROGUEAP );
      
    return rc;
}

//=============================================================================
int
cm2gs_tr069_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
)
//=============================================================================
{
    return 0;
}

//=============================================================================
int
cm2gs_ap_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
)
//=============================================================================
{
    struct grandstream_config_parse *gs_cfg = NULL;
    char *section_name = extend->section_name;
    int rc = 0;

    gs_cfg = util_get_vltree_node( &grandstream_ap_vltree, VLTREE_GRANDSTREAM, section_name );
    if( gs_cfg ) {
        config_del_named_section( CF_CONFIG_NAME_GRANDSTREAM, "ap", section_name );
    }

    rc |= BIT( CM_CFG_GRANDSTREAM );

    if( VLTREE_ACTION_DEL == extend->action ) {
        return rc;
    }

    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "ap", section_name );

    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_MAC, CM_AP_MAC );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_TYPE, CM_AP_TYPE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_FREQUENCY, CM_AP_FREQUENCY );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_BAND_STEERING, CM_AP_BAND_STEERING );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_ZONES, CM_AP_ZONES );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_SSIDS, CM_AP_SSIDS );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_TLS1_2, CM_AP_TLS1_2 );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_NAME, CM_AP_NAME );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_NETPORT_TYPE, CM_AP_NETPORT_TYPE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV4_ENABLE, CM_AP_IPV4_ENABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV4_IP, CM_AP_IPV4_IP );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV4_NETMASK, CM_AP_IPV4_NETMASK );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV4_GATEWAY, CM_AP_IPV4_GATEWAY );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV4_FIRST_DNS, CM_AP_IPV4_FIRST_DNS );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV4_SECOND_DNS, CM_AP_IPV4_SECOND_DNS );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV6_ENABLE, CM_AP_IPV6_ENABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV6_IP, CM_AP_IPV6_IP );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV6_PREFIX_LENGTH, CM_AP_IPV6_PREFIX_LENGTH );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV6_GATEWAY, CM_AP_IPV6_GATEWAY );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV6_FIRST_DNS, CM_AP_IPV6_FIRST_DNS );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_IPV6_SECOND_DNS, CM_AP_IPV6_SECOND_DNS );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_DISABLE, CM_AP_2G4_DISABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_WIDTH, CM_AP_2G4_WIDTH );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_CHANNEL_LOCATION, CM_AP_2G4_CHANNEL_LOCATION );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_CHANNEL, CM_AP_2G4_CHANNEL );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_POWER, CM_AP_2G4_POWER );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_CUSTOM_TXPOWER, CM_AP_2G4_CUSTOM_TXPOWER );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_RSSI_TYPE, CM_AP_2G4_RSSI_TYPE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_RSSI_THRESHOLD, CM_AP_2G4_RSSI_THRESHOLD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_MODE, CM_AP_2G4_MODE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_RATE_LIMIT_TYPE, CM_AP_2G4_RATE_LIMIT_TYPE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_MINI_RATE, CM_AP_2G4_MINI_RATE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_SHORTGI, CM_AP_2G4_SHORTGI );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_2G4_ALLOW_LEGACY_DEV, CM_AP_2G4_ALLOW_LEGACY_DEV );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_DISABLE, CM_AP_5G_DISABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_WIDTH, CM_AP_5G_WIDTH );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_CHANNEL, CM_AP_5G_CHANNEL );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_POWER, CM_AP_5G_POWER );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_CUSTOM_TXPOWER, CM_AP_5G_CUSTOM_TXPOWER );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_RSSI_TYPE, CM_AP_5G_RSSI_TYPE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_RSSI_THRESHOLD, CM_AP_5G_RSSI_THRESHOLD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_RATE_LIMIT_TYPE, CM_AP_5G_RATE_LIMIT_TYPE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_MINI_RATE, CM_AP_5G_MINI_RATE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_ap_policy, cm_ap_policy, GS_AP_5G_SHORTGI, CM_AP_5G_SHORTGI );

    return rc;
}

//=============================================================================
int
cm2gs_additional_ssid_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };
    char value[BUF_LEN_128] = { 0 };
    char *section_name = extend->section_name;
    struct grandstream_config_parse *gs_cfg = NULL;
    int rc = 0;

    gs_cfg = util_get_vltree_node( &grandstream_ssid_vltree, VLTREE_GRANDSTREAM, section_name );
    if( gs_cfg ) {
        config_del_named_section( CF_CONFIG_NAME_GRANDSTREAM, "additional_ssid", section_name );
    }

    rc |= BIT( CM_CFG_GRANDSTREAM );

    if( VLTREE_ACTION_DEL == extend->action ) {
        return rc;
    }

    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "additional_ssid", section_name );

    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_ID, CM_ADDIT_SSID_ID );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_ENABLE, CM_ADDIT_SSID_ENABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_BAND, CM_ADDIT_SSID_BAND );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_NAME, CM_ADDIT_SSID_NAME );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_CRYPTO, CM_ADDIT_SSID_CRYPTO );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_WPA_KEY_MODE, CM_ADDIT_SSID_WPA_KEY_MODE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_WPA_CRYPTO_TYPE, CM_ADDIT_SSID_WPA_CRYPTO_TYPE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_PASSWORD, CM_ADDIT_SSID_PASSWORD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_SSIDHIDEENABLE, CM_ADDIT_SSID_SSIDHIDEENABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_ISOLATEMODE, CM_ADDIT_SSID_ISOLATEMODE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_DTIM_PERIOD, CM_ADDIT_SSID_DTIM_PERIOD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_CLIENT_LIMIT, CM_ADDIT_SSID_CLIENT_LIMIT );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_STA_IDLE_TIMEOUT, CM_ADDIT_SSID_STA_IDLE_TIMEOUT );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_UAPSD, CM_ADDIT_SSID_UAPSD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_PROXY_ARP, CM_ADDIT_SSID_PROXY_ARP );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_MCAST_TO_UCAST, CM_ADDIT_SSID_MCAST_TO_UCAST );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_BMS, CM_ADDIT_SSID_BMS );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_BRIDGE_ENABLE, CM_ADDIT_SSID_BRIDGE_ENABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_80211W, CM_ADDIT_SSID_80211W );
    cm2gs_set_gs_config( section_name, cm_attr, gs_addit_ssid_policy, cm_addit_ssid_policy, GS_ADDIT_SSID_VLAN, CM_ADDIT_SSID_VLAN );

    if( cm_attr[CM_ADDIT_SSID_GATEWAYMAC] ) {
        snprintf( path, sizeof( path ), "%s.%s.%s",
            CF_CONFIG_NAME_GRANDSTREAM, section_name, gs_addit_ssid_policy[CM_ADDIT_SSID_GATEWAYMAC].name );
        util_formatted_mac_with_colo( blobmsg_get_string( cm_attr[CM_ADDIT_SSID_GATEWAYMAC] ), value );
        config_uci_set( path, value, 0 );
    }

    return rc;
}

//=============================================================================
int
cm2gs_radio_hooker(
    struct blob_attr **cm_attr,
    struct cm2gs_extend *extend
)
//=============================================================================
{
    struct grandstream_config_parse *gs_cfg = NULL;
    char *section_name = "radio";
    int rc = 0;

    gs_cfg = util_get_vltree_node( &grandstream_radio_vltree, VLTREE_GRANDSTREAM, "radio" );
    if( gs_cfg ) {
        config_del_named_section( CF_CONFIG_NAME_GRANDSTREAM, "radio", "radio" );
    }

    rc |= BIT( CM_CFG_GRANDSTREAM );

    if( VLTREE_ACTION_DEL == extend->action ) {
        return rc;
    }

    config_add_named_section( CF_CONFIG_NAME_GRANDSTREAM, "radio", "radio" );

    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4CHANNEL, CM_RADIO_2G4CHANNEL );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4CHANNELWIDTH, CM_RADIO_2G4CHANNELWIDTH );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4CHANNEL_LOCATION, CM_RADIO_2G4CHANNEL_LOCATION );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4TXPOWER, CM_RADIO_2G4TXPOWER );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4CUSTOM_TXPOWER, CM_RADIO_2G4CUSTOM_TXPOWER );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4SHORTGI, CM_RADIO_2G4SHORTGI );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4ALLOW_LEGACY_DEV, CM_RADIO_2G4ALLOW_LEGACY_DEV );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4RSSI_ENABLE, CM_RADIO_2G4RSSI_ENABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4RSSI_THRESHOLD, CM_RADIO_2G4RSSI_THRESHOLD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4RATE_LIMIT_ENABLE, CM_RADIO_2G4RATE_LIMIT_ENABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_2G4MINI_RATE, CM_RADIO_2G4MINI_RATE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_5GCHANNEL, CM_RADIO_5GCHANNEL );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_5GCHANNELWIDTH, CM_RADIO_5GCHANNELWIDTH );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_5GTXPOWER, CM_RADIO_5GTXPOWER );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_5GCUSTOM_TXPOWER, CM_RADIO_5GCUSTOM_TXPOWER );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_5GSHORTGI, CM_RADIO_5GSHORTGI );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_5GRSSI_ENABLE, CM_RADIO_5GRSSI_ENABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_5GRSSI_THRESHOLD, CM_RADIO_5GRSSI_THRESHOLD );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_5GRATE_LIMIT_ENABLE, CM_RADIO_5GRATE_LIMIT_ENABLE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_5GMINI_RATE, CM_RADIO_5GMINI_RATE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_BEACON_INTERVAL, CM_RADIO_BEACON_INTERVAL );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_ATF_MODE, CM_RADIO_ATF_MODE );
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_MUMIMOENABLE, CM_RADIO_MUMIMOENABLE );
#if defined(GWN7062)
    cm2gs_set_gs_config( section_name, cm_attr, gs_radio_policy, cm_radio_policy, GS_RADIO_COMPATIBILITYMODE, CM_RADIO_COMPATIBILITYMODE );
#endif

    return rc;
}
