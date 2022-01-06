/****************************************************************************
* *
* * FILENAME:        $RCSfile: ubus.c,v $
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
//=================
//  Includes
//=================
#include "global.h"
#include "cfparse.h"
#include "utils.h"
#include "sgrequest.h"
#include "ubus.h"
#include "ipc.h"
#include "cfmanager.h"
#include "config.h"
#include "gs_ipc_msg.h"
#include "track.h"
#include "usb.h"
#include "smb.h"
#include "vpn.h"

//=================
//  Defines
//=================
#define METHOD_LEN              64
#define HASH_SIZE               65

typedef void ( *handle_sg_request )( struct ubus_context *, struct blob_attr *,
    struct ubus_request_data *, struct sg_params* );

struct sg_handle_policy {
    const char *method_name;
    handle_sg_request cb;
};

struct close_radio_params {
    const char *channel_with;
    bool *need_close_radio;
};

//=================
//  Typedefs
//=================
enum {
    EVENT_CONFIG,
    __EVENT_MAX
};

enum {
    CMD_MNEMONIC,
    CMD_EXTENDED,
    __CMD_MAX
};

enum {
    SG_ID,
    SG_HASHID,
    SG_METHOD,
    SG_PARAMS,

    __SG_MAX
};

enum {
    ME_MAC,

    __ME_MAX
};

enum {
    DEV_STATUS_MAC,

    __DEV_STATUS_MAX
};

//=================
//  Globals
//=================

//=================
//  Locals
//=================
static struct sg_handle_policy sg_hp[] = {
    //ctrl request
    { "vpn service", sgreq_ctrl_vpn_client },

    //set request
    { "set extern log", sgreq_set_extern_sys_log },
    { "set email", sgreq_set_email },
    { "set notification", sgreq_set_notification },
    { "set upgrade auto", sgreq_set_upgrade_auto },
    { "set wan", sgreq_set_wan },
    { "set lan", sgreq_set_lan },
    { "set vlan", sgreq_set_vlan },
    { "set switch ports", sgreq_set_switch_ports },
    { "set wireless", sgreq_set_wireless },
    { "set access", sgreq_set_access },
    { "set global access", sgreq_set_global_access },
    { "set schedule access", sgreq_set_schedule_access },
    { "set guest ssid", sgreq_set_guest_ssid },
    { "set acl", sgreq_set_acl },
    { "set qos", sgreq_set_qos },
    { "set usb share", sgreq_set_usb_share },
    { "set static router", sgreq_set_static_router },
    { "set controller", sgreq_set_controller },
    { "set general", sgreq_set_general },
    { "set basic", sgreq_set_basic },
    { "set manage ap or mesh", sgreq_set_manage_ap_mesh },
    { "set ap", sgreq_set_ap },
    { "set vpn service", sgreq_set_vpn_client },
    { "set vpn split tunneling", sgreq_set_vpn_split_tunneling },
    { "set port mapping", sgreq_set_port_mapping },
    { "set dmz", sgreq_set_dmz },
    { "set upnp", sgreq_set_upnp },
    { "set ddns", sgreq_set_ddns },
    { "set ipv4 static route", sgreq_set_ipv4_static_route },
    { "set mesh ssid", sgreq_set_mesh_ssid },
    { "set client limit", sgreq_set_client_limit },
    { "set firewall dos", sgreq_set_firewall_dos },
    { "set ipv6 static route", sgreq_set_ipv6_static_route },
    { "set tr069", sgreq_set_tr069 },
    { "set hostname", sgreq_set_hostname },
    { "set portal policy", sgreq_set_portal_policy },
    { "set snmp", sgreq_set_snmp },
    { "set vpn server", sgreq_set_vpn_server },
    { "set additional ssid", sgreq_set_additional_ssid },
    { "set radio", sgreq_set_radio },
    { "set acceleration", sgreq_set_acceleration },
    { "set schedule", sgreq_set_schedule },
    { "set dev add ssid", sgreq_set_dev_add_ssid },
    { "set ap batch", sgreq_set_ap_batch },
    { "set ssid wizard", sgreq_set_ssid_wizard },

    //get request
    { "get all", sgreq_get_all },
    { "get wan", sgreq_get_wan },
    { "get lan", sgreq_get_lan },
    { "get vlan", sgreq_get_vlan },
    { "get switch ports", sgreq_get_switch_ports },
    { "get wireless", sgreq_get_wireless },
    { "get client", sgreq_get_client },
    { "get device", sgreq_get_device },
    { "get guest ssid", sgreq_get_guest_ssid },
    { "get acl", sgreq_get_acl },
    { "get qos", sgreq_get_qos },
    { "get network interface", sgreq_get_network_interface },
    { "get controller", sgreq_get_controller },
    { "get access", sgreq_get_access },
    { "get upgrade auto", sgreq_get_upgrade_auto },
    { "get basic", sgreq_get_basic },
    { "get extern log", sgreq_get_extern_sys_log },
    { "get email", sgreq_get_email },
    { "get notification", sgreq_get_notification},
    { "get ap", sgreq_get_ap },
    { "get vpn service", sgreq_get_vpn_client },
    { "get vpn split tunneling", sgreq_get_vpn_split_tunneling },
    { "get port mapping", sgreq_get_port_mapping },
    { "get dmz", sgreq_get_dmz },
    { "get upnp", sgreq_get_upnp },
    { "get ddns", sgreq_get_ddns },
    { "get ipv4 static route", sgreq_get_ipv4_static_route },
    { "get mesh ssid", sgreq_get_mesh_ssid },
    { "get paired devices", sgreq_get_paired_devices },
    { "get client limit", sgreq_get_client_limit },
    { "get overview", sgreq_get_overview },
    { "get channel", sgreq_get_channel },
    { "get usb share", sgreq_get_usb_share },
    { "get firewall dos", sgreq_get_firewall_dos },
    { "get wan ip", sgreq_get_wan_ip },
    { "get ipv6 static route", sgreq_get_ipv6_static_route },
    { "get tr069", sgreq_get_tr069 },
    { "get portal policy", sgreq_get_portal_policy },
    { "get snmp", sgreq_get_snmp },
    { "get vpn server", sgreq_get_vpn_server },
    { "get additional ssid", sgreq_get_additional_ssid },
    { "get radio", sgreq_get_radio },
    { "get acceleration", sgreq_get_acceleration },
    { "get schedule", sgreq_get_schedule },
    { "get ap status", sgreq_get_ap_status },
    { "get aps simple info", sgreq_get_aps_simple_info },
    { "get ssid wizard", sgreq_get_ssid_wizard },

    //ext request
    { "ext unauthenticated info", sgreq_ext_unauthenticated_info },
};

static int sg_handle_policy_sz = ARRAY_SIZE( sg_hp );
/*
 * Private Functions
 */
static int
cfubus_handle_command(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
);

static int
cfubus_handle_async_reload_service(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
);

static int
cfubus_handle_sg_info(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
);

static int
cfubus_load_cfmanager(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
);

static int
cfubus_tr069_apply_modify(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
);

/*
 * Private Data
 */

static struct ubus_context *ubus_ctx = NULL;
static struct ubus_subscriber main_subscribe_event;
static struct ubus_event_handler cf_ubus_listener;
static const char *ubus_path;
static struct blob_buf reply;

/* --- Policy --- */
static const struct blobmsg_policy event_policy[__EVENT_MAX] = {
    [EVENT_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_STRING },
};

static struct blobmsg_policy cfubus_command_policy[__CMD_MAX] = {
        [CMD_MNEMONIC] = { .name = "mnemonic", .type = BLOBMSG_TYPE_STRING },
        [CMD_EXTENDED] = { .name = "extended", .type = BLOBMSG_TYPE_STRING },
};

static struct blobmsg_policy cfubus_sg_info_policy[__SG_MAX] = {
    [SG_ID] = { .name = "id", .type = BLOBMSG_TYPE_INT32 },
    [SG_HASHID] = { .name = "hashID", .type = BLOBMSG_TYPE_STRING },
    [SG_METHOD] = { .name = "method", .type = BLOBMSG_TYPE_STRING },
    [SG_PARAMS] = { .name = "params", .type = BLOBMSG_TYPE_TABLE },
};

static struct blobmsg_policy cfubus_me_info_policy[__ME_MAX] = {
    [ME_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
};

static const struct blobmsg_policy network_dev_policy[__DEV_STATUS_MAX] = {
    [DEV_STATUS_MAC] = { .name = "macaddr", .type = BLOBMSG_TYPE_STRING },
};

static const struct blobmsg_policy parse_channel_policy[] = {
    [0] = { .name = "channels_5g", .type = BLOBMSG_TYPE_ARRAY }
};
int parse_channel_policy_sz = ARRAY_SIZE( parse_channel_policy );

static const struct blobmsg_policy parse_bandwidths_policy[] = {
    [0] = { .name = "bandwidths", .type = BLOBMSG_TYPE_ARRAY }
};
int parse_bandwidths_policy_sz = ARRAY_SIZE( parse_bandwidths_policy );

static const struct blobmsg_policy clients_list_policy[] = {
    [0] = { .name = "clients_list", .type = BLOBMSG_TYPE_ARRAY }
};
int clients_list_policy_sz = ARRAY_SIZE( clients_list_policy );

static const struct blobmsg_policy network_interface_attr_policy[] = {
    [0] = { .name = "ipv4-address", .type = BLOBMSG_TYPE_ARRAY }
};
int network_interface_attr_policy_sz = ARRAY_SIZE( network_interface_attr_policy );

static const struct blobmsg_policy ipv4_address_attr_policy[] = {
    [0] = { .name = "mask", .type = BLOBMSG_TYPE_INT32 }
};
int ipv4_address_attr_policy_sz = ARRAY_SIZE( ipv4_address_attr_policy );

static const struct blobmsg_policy async_reload_service_policy[] = {
    [0] = { .name = "service", .type = BLOBMSG_TYPE_STRING }
};
int async_reload_service_policy_sz = ARRAY_SIZE( async_reload_service_policy );

/* --- Methods --- */
static struct ubus_method main_object_methods[] = {
    UBUS_METHOD( "command", cfubus_handle_command, cfubus_command_policy ),
    UBUS_METHOD( "async_reload_service", cfubus_handle_async_reload_service, async_reload_service_policy ),
};

static struct ubus_method cfmanager_sg_object_methods[] = {
    UBUS_METHOD( "handle_sg_info", cfubus_handle_sg_info, cfubus_sg_info_policy ),
};

static struct ubus_method cfmanager_ctrl_object_methods[] = {
    UBUS_METHOD_NOARG( "load_cfmanager_cfg", cfubus_load_cfmanager ),
};

static struct ubus_method cfmanager_tr069_object_methods[] = {
    UBUS_METHOD_NOARG( "apply", cfubus_tr069_apply_modify ),
};

/* --- Object Type --- */
static struct ubus_object_type main_object_type =
    UBUS_OBJECT_TYPE("cfmanager", main_object_methods);

static struct ubus_object_type cfmanager_sg_object_type =
    UBUS_OBJECT_TYPE( "cfmanager.sg", cfmanager_sg_object_methods );

static struct ubus_object_type cfmanager_ctrl_object_type =
    UBUS_OBJECT_TYPE( "cfmanager.ctrl", cfmanager_ctrl_object_methods );

static struct ubus_object_type cfmanager_tr069_object_type =
    UBUS_OBJECT_TYPE( "cfmanager.tr069", cfmanager_tr069_object_methods );

/* --- Object --- */
static struct ubus_object main_object = {
    .name = "cfmanager",
    .type = &main_object_type,
    .methods = main_object_methods,
    .n_methods = ARRAY_SIZE(main_object_methods),
};

static struct ubus_object cfmanager_sg_object = {
    .name = "cfmanager.sg",
    .type = &cfmanager_sg_object_type,
    .methods = cfmanager_sg_object_methods,
    .n_methods = ARRAY_SIZE(cfmanager_sg_object_methods),
};

static struct ubus_object cfmanager_ctrl_object = {
    .name = "cfmanager.ctrl",
    .type = &cfmanager_ctrl_object_type,
    .methods = cfmanager_ctrl_object_methods,
    .n_methods = ARRAY_SIZE(cfmanager_ctrl_object_methods),
};

static struct ubus_object cfmanager_tr069_object = {
    .name = "cfmanager.tr069",
    .type = &cfmanager_tr069_object_type,
    .methods = cfmanager_tr069_object_methods,
    .n_methods = ARRAY_SIZE(cfmanager_tr069_object_methods),
};

//=================
//  Functions
//=================

//=============================================================================
static int
cfubus_add_object(
    struct ubus_object *obj
)
//=============================================================================
{
    int ret = ubus_add_object( ubus_ctx, obj );

    if ( ret != 0 ) {
        cfmanager_log_message( L_ERR,
            "Failed to publish object '%s': %s\n",
            obj->name, ubus_strerror( ret ) );
        return -1;
    }

    return 0;
}

//=============================================================================
static int
cfubus_main_subscribe_cb(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_attr *tb[ARRAY_SIZE(event_policy)];
    const char *config = NULL;

    blobmsg_parse( event_policy,
                   ARRAY_SIZE(event_policy),
                   tb,
                   blob_data( msg ),
                   blob_len( msg ) );

    config = blobmsg_get_string(tb[EVENT_CONFIG]);

    if ( !config ) {
        return 0;
    }

    cfmanager_log_message( L_DEBUG, "Config change: %s\n", config );

    //cfparse_load_file( config, LOAD_ALL_SECTION );

    return 0;
}

//=============================================================================
static void
cfubus_main_subscribe_remove_cb(
    struct ubus_context *ctx,
    struct ubus_subscriber *obj,
    uint32_t id
)
//=============================================================================
{
    cfmanager_log_message( L_DEBUG, "Object %08x went away\n", id );
    ubus_unsubscribe( ctx, obj, id );
}

//=============================================================================
static int
cfubus_register_config_events(
    struct ubus_subscriber *event
)
//=============================================================================
{
    int ret = 0;

    ret = ubus_register_subscriber( ubus_ctx, event );
    if ( ret ) {
        cfmanager_log_message( L_ERR,
            "Register config event to ubus failed: %s", ubus_strerror( ret ) );
        return -1;
    }

    cfmanager_log_message( L_DEBUG, "Register config event to ubus success" );
    return 0;
}

//=============================================================================
static int
cfubus_subscribe_object(
    struct ubus_subscriber *event
)
//=============================================================================
{
    int ret = 0;
    unsigned int id;

    ret = ubus_lookup_id( ubus_ctx, "service", &id );
    if ( ret ) {
        cfmanager_log_message( L_ERR,
            "Failed to lookup service id: %s",
            ubus_strerror( ret ) );
        return -1;
    }

    ret = ubus_subscribe( ubus_ctx, event, id );
    if ( ret ) {
        cfmanager_log_message( L_ERR,
            "Failed to subscriber config.change event: %s",
            ubus_strerror( ret ) );
        return -1;
    }

    cfmanager_log_message( L_DEBUG, "Subscriber config.change to ubus success" );
    return 0;
}

//=============================================================================
static void
cfubus_reconnect_timer(
    struct uloop_timeout *timeout
)
//=============================================================================
{
    static struct uloop_timeout retry = {
        .cb = cfubus_reconnect_timer,
    };
    int t = 3;

    if ( ubus_reconnect( ubus_ctx, ubus_path ) != 0) {
        cfmanager_log_message( L_ERR, "Failed to reconnect, trying again in %d seconds\n", t );
        uloop_timeout_set( &retry, t * 1000 );
        return;
    }

    cfmanager_log_message( L_DEBUG, "reconnected to ubus, new id: %08x\n", ubus_ctx->local_id );
    ubus_add_uloop( ubus_ctx );
}

//=============================================================================
static void
cfubus_connection_lost(
    struct ubus_context *ctx
)
//=============================================================================
{
    if (ctx->sock.fd >= 0) {
        if ( ctx->sock.registered )
            uloop_fd_delete( &ctx->sock );

        close( ctx->sock.fd );
        cfmanager_log_message( L_ERR, "Before reconnect to ubus, close socket\n" );
    }
    cfubus_reconnect_timer(NULL);
}

//=============================================================================
static void
cf_ubus_handle_receive_event(
    struct ubus_context *ctx,
    struct ubus_event_handler *ev,
    const char *type,
    struct blob_attr *msg
)
//=============================================================================
{
    char * message = NULL;

    message = blobmsg_format_json(msg, true);
    cfmanager_log_message(L_DEBUG, "cfmanager receive ubus event[%s]: %s.", type, message);
    SAFE_FREE( message );

    if( 0 == strcmp(type, "usb storage partition") ) {
        cfparse_update_share(msg);
    }

}

//=============================================================================
int
cfubus_init(
    const char *path
)
//=============================================================================
{
    int ret = 0;
    ubus_path = path;

    ubus_ctx = ubus_connect( path );
    if ( !ubus_ctx )
        return -EIO;

    cfmanager_log_message( L_DEBUG, "connected as %08x\n", ubus_ctx->local_id );
    ubus_ctx->connection_lost = cfubus_connection_lost;
    ubus_add_uloop( ubus_ctx );

    ret = cfubus_add_object( &main_object );
    if ( ret ) {
        cfmanager_log_message( L_ERR, "Failed to add main_object\n" );
        return -1;
    }

    ret = cfubus_add_object( &cfmanager_sg_object );
    if ( ret ) {
        cfmanager_log_message( L_ERR, "Failed to add cfmanager_sg_object\n" );
        return -1;
    }

    ret = cfubus_add_object( &cfmanager_ctrl_object );
    if ( ret ) {
        cfmanager_log_message( L_ERR, "Failed to add cfmanager_ctrl_object\n" );
        return -1;
    }

    ret = cfubus_add_object( &cfmanager_tr069_object );
    if ( ret ) {
        cfmanager_log_message( L_ERR, "Failed to add cfmanager_tr069_object\n" );
        return -1;
    }

    main_subscribe_event.cb = cfubus_main_subscribe_cb;
    main_subscribe_event.remove_cb = cfubus_main_subscribe_remove_cb;
    ret = cfubus_register_config_events( &main_subscribe_event );
    if ( ret ) {
        cfmanager_log_message( L_ERR,
            "Failed to register ubus subscriber: %s\n", ubus_strerror( ret ) );
        return -1;
    }

    ret = cfubus_subscribe_object( &main_subscribe_event );
    if ( ret ) {
        cfmanager_log_message( L_ERR,
            "Failed to subscribe: %s\n", ubus_strerror( ret ) );
        return -1;
    }

    memset( &cf_ubus_listener, 0, sizeof ( cf_ubus_listener ) );

    cf_ubus_listener.cb = cf_ubus_handle_receive_event;

    // cfmanager subscribe to usb storage partition event from ubus.
    ret = ubus_register_event_handler( ubus_ctx, &cf_ubus_listener,
                                       "usb storage partition" );
    if( ret ) {
        cfmanager_log_message( L_ERR,
            "cfmanager subscribe to usb storage partition event is error: %s\n",
            ubus_strerror(ret));
    } else {
        cfmanager_log_message( L_DEBUG,
            "cfmanager subscribe to usb storage partition event is successful.");
    }

    return 0;
}

//=============================================================================
void
send_uns_notification(
    struct ubus_context *ctx,
    const char* msg,
    int priority
)
//=============================================================================
{
   static struct blob_buf ubus_send_buf;

    json_object* CM_NOTIFY_obj;
    blob_buf_init( &ubus_send_buf, 0 );

    CM_NOTIFY_obj = json_object_new_string( "cfmanager" );
    blobmsg_add_json_element( &ubus_send_buf, "module",
        CM_NOTIFY_obj );
    json_object_put( CM_NOTIFY_obj );

    CM_NOTIFY_obj = json_object_new_string( msg );
    blobmsg_add_json_element( &ubus_send_buf, "msg",
        CM_NOTIFY_obj );
    json_object_put( CM_NOTIFY_obj );

    CM_NOTIFY_obj = json_object_new_int( priority );
    blobmsg_add_json_element( &ubus_send_buf, "pri",
        CM_NOTIFY_obj );
    json_object_put( CM_NOTIFY_obj );

    ubus_send_event( ctx, "grandstream", ubus_send_buf.head );

    blob_buf_free( &ubus_send_buf );
}


//=============================================================================
void
cfubus_done(
    void
)
//=============================================================================
{
    ubus_free( ubus_ctx );
}

//=============================================================================
static void
cfubus_handle_log(
    const char *data
)
//=============================================================================
{
    if ( !data )
        return;
    switch ( data[0] ) {
        case 'D':
        case 'd':
        case '7':
            cfmanager_set_log_level( L_DEBUG );
            break;
        case 'I':
        case 'i':
        case '6':
            cfmanager_set_log_level( L_INFO );
            break;
        case 'N':
        case 'n':
        case '5':
            cfmanager_set_log_level( L_NOTICE );
            break;
        case 'W':
        case 'w':
        case '4':
            cfmanager_set_log_level( L_WARNING );
            break;
        case '3':
            cfmanager_set_log_level( L_ERR );
            break;
        case 'C':
        case 'c':
        case '2':
            cfmanager_set_log_level( L_CRIT );
            break;
        case 'A':
        case 'a':
        case '1':
            cfmanager_set_log_level( L_ALERT );
            break;
        case '0':
            cfmanager_set_log_level( L_EMERG );
            break;
        case 'E':
        case 'e':
            if ( strncasecmp( data, "ERR", strlen( "ERR" ) ) == 0 ) {
                cfmanager_set_log_level( L_ERR );
            }
            else if ( strncasecmp( data, "EMERG", strlen( "EMERG" ) ) == 0 ) {
                cfmanager_set_log_level( L_EMERG );
            }
            else {
                cfmanager_set_log_level( L_ERR );
            }
            break;
    }
}

//=============================================================================
static int
cfubus_handle_command(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_attr *tb[__CMD_MAX];
    const char *mnemonic = NULL;
    const char *extended = NULL;

    blobmsg_parse( cfubus_command_policy,
                   __CMD_MAX,
                   tb,
                   blob_data( msg ),
                   blob_len( msg ) );

    if ( !tb[CMD_MNEMONIC] )
        return UBUS_STATUS_INVALID_ARGUMENT;

    mnemonic = blobmsg_get_string( tb[CMD_MNEMONIC] );

    if ( tb[CMD_EXTENDED] )
        extended = blobmsg_get_string( tb[CMD_EXTENDED] );

    cfmanager_log_message( L_DEBUG, "CMD_MNEMONIC: '%s'\n", mnemonic );
    if ( strncmp( mnemonic, "log", strlen( "log" ) ) == 0 ) {
        cfubus_handle_log( extended );
    }

    return UBUS_STATUS_OK;
}

//=============================================================================
static int
cfubus_handle_async_reload_service(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
)
//=============================================================================
{
    char *service = NULL;
    struct blob_attr *tb[async_reload_service_policy_sz];

    blobmsg_parse( async_reload_service_policy,
                   async_reload_service_policy_sz,
                   tb,
                   blob_data( msg ),
                   blob_len( msg ) );

    if ( !tb[0] ) {
        cfmanager_log_message( L_ERR, "Missing argument %s!\n", 
            async_reload_service_policy[0].name );
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    service = blobmsg_get_string( tb[0] );
    if ( !strcmp( service, "ipsec" ) ) {
        vpn_aync_reload_ipsec();
    }

    return UBUS_STATUS_OK;
}

//=============================================================================
void
cfubus_wss_error_state_handle(
    int error_state,
    char *erro_buf,
    int error_buf_size
)
//=============================================================================
{
    if ( NULL == erro_buf || 0 >= error_buf_size )
        return;

    switch ( error_state ) {
        case ERRCODE_SUCCESS:
            snprintf( erro_buf, error_buf_size, "data processing successful" );
            break;
        case ERRCODE_SESSION_TIMEOUT:
            snprintf( erro_buf, error_buf_size, "user ID timeout" );
            break;
        case ERRCODE_SESSION_INACTIVE:
            snprintf( erro_buf, error_buf_size, "inactive userID" );
            break;
        case ERRCODE_COMMAND_INVALID:
            snprintf( erro_buf, error_buf_size, "command invalid" );
            break;
        case ERRCODE_PARAMETER_ERROR:
            snprintf( erro_buf, error_buf_size, "error parameter" );
            break;
        case ERRCODE_PROTOCOL_IMCOMPAT:
            snprintf( erro_buf, error_buf_size, "protocol incompatibility" );
            break;
        case ERRCODE_METHOD_NOT_FOUND:
            snprintf( erro_buf, error_buf_size, "method not found" );
            break;
        case ERRCODE_INTERNAL_ERROR:
            snprintf( erro_buf, error_buf_size, "internal error" );
            break;
        case ERRCODE_MESSAGE_WRONG_FROMAT:
            snprintf( erro_buf, error_buf_size, "Wrong message format" );
            break;
        case ERRCODE_WRONG_PASSWORD:
            snprintf( erro_buf, error_buf_size, "error passord" );
            break;
        case ERRCODE_LASTEST_VERSION:
            snprintf( erro_buf, error_buf_size, "The current firmware version is already the latest version" );
            break;
        case ERRCODE_SERVER_ERROR:
            snprintf( erro_buf, error_buf_size, "The firmware server cannot be connected or is not responding" );
            break;
        case ERRCODE_SAME_PASSWORD:
            snprintf( erro_buf, error_buf_size, "The password is the same as the original password" );
            break;
        case ERRCODE_UPGRADE_TIMEOUT:
            snprintf( erro_buf, error_buf_size, "Upgrade timeout" );
            break;
        case ERRCODE_SSID_NAME_TOO_LONG:
            snprintf( erro_buf, error_buf_size, "The length of ssid name is too long\n" );
            break;
        default:
            cfmanager_log_message( L_DEBUG, "unknown error:%d\n", error_state );
            snprintf( erro_buf, error_buf_size, "Unknown error" );
            break;
        }
}

//=============================================================================
static int
cfubus_handle_sg_info(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
)
//=============================================================================
{
    struct blob_attr *tb[__SG_MAX];
    struct blob_attr *params_attr;
    struct sg_params sg_params;
    int id = 0;
    int i = 0;
    int find = 0;
    int error = 0;
    char method_str[METHOD_LEN] = { 0 };
    char hashID[HASH_SIZE] = { 0 };
    char temp_buf[BUF_LEN_128] = { 0 };

    blobmsg_parse( cfubus_sg_info_policy,
            __SG_MAX,
            tb,
            blob_data( msg ),
            blob_len( msg ) );

    if( tb[SG_ID] )
        id = blobmsg_get_u32( tb[SG_ID] );
    if( tb[SG_HASHID] )
        strncpy( hashID, blobmsg_get_string( tb[SG_HASHID] ), sizeof( hashID ) - 1 );
    if( tb[SG_METHOD] )
        strncpy( method_str, blobmsg_get_string( tb[SG_METHOD] ), sizeof( method_str ) - 1 );
    if( tb[SG_PARAMS] )
        params_attr = tb[SG_PARAMS];

    cfmanager_log_message( L_DEBUG, "recv id:%d,hashID:%s,method:%s\n", id, hashID, method_str );

    if( !id || hashID[0] == '\0' || method_str[0] == '\0' ) {
        error = ERRCODE_MESSAGE_WRONG_FROMAT;
        goto error_out;
    }

    memset( &sg_params, 0, sizeof( sg_params ) );
    sg_params.id = id;
    sg_params.hashID = hashID;
    sg_params.method = method_str;

    for( i = 0; i < sg_handle_policy_sz; i++ ) {
        if( 0 == strcmp( sg_hp[i].method_name, method_str ) ) {
            sg_hp[i].cb( ctx, params_attr, req, &sg_params );
            find = 1;
        }
    }

    if( !find ) {
        error = ERRCODE_METHOD_NOT_FOUND;
        cfmanager_log_message( L_ERR, "No corresponding %s processing policy was found", method_str );
        goto error_out;
    }

    return UBUS_STATUS_OK;

error_out:
    blob_buf_init( &reply, 0 );

    blobmsg_add_string( &reply, "hashID", hashID );
    blobmsg_add_u32( &reply, "id", id );

    cfubus_wss_error_state_handle( error, temp_buf, sizeof( temp_buf ) );
    blobmsg_add_u32( &reply, "retVal", error );
    blobmsg_add_string( &reply, "retMsg", temp_buf );

    ubus_send_reply( ctx, req, reply.head );
    blob_buf_free( &reply );

    return UBUS_STATUS_INVALID_ARGUMENT;
}

//=============================================================================
static void
cfubus_load_all_cfmanager_section(
    void
)
//=============================================================================
{
    char path_src[LOOKUP_STR_SIZE] = { 0 };
    char path_dest[LOOKUP_STR_SIZE] = { 0 };

    /* The directory to configure synchronization is /etc/config,
     * which needs to be copied to tmp directory
     */
    snprintf( path_src, sizeof( path_src ), "%s/%s", UCI_DEFAULT_PATH, CFMANAGER_CONFIG_NAME );
    snprintf( path_dest, sizeof( path_dest ), "%s/%s", CM_CONFIG_PATH, CFMANAGER_CONFIG_NAME );
    util_cpy_file( path_src, path_dest );

    cfparse_load_file( CFMANAGER_CONFIG_NAME, LOAD_ALL_SECTION, false );
}

//=============================================================================
static int
cfubus_load_cfmanager(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
)
//=============================================================================
{
    cfubus_load_all_cfmanager_section();

    return UBUS_STATUS_OK;
}

//=============================================================================
static int
cfubus_tr069_apply_modify(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
)
//=============================================================================
{
    cfubus_load_all_cfmanager_section();

    return UBUS_STATUS_OK;
}

//=============================================================================
static void
cfubus_handle_vpn_status_data_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    int *status = req->priv;
    bool up = false;
    bool autostart = true;

    enum {
        INTF_STATUS_ATTR_UP,
        INTF_STATUS_ATTR_AUTOSTART,

        __INTF_STATUS_ATTR_MAX
    };

    struct blobmsg_policy intf_status_policy[__INTF_STATUS_ATTR_MAX] = {
        [INTF_STATUS_ATTR_UP] = { .name = "up", .type = BLOBMSG_TYPE_BOOL },
        [INTF_STATUS_ATTR_AUTOSTART] = { .name = "autostart", .type = BLOBMSG_TYPE_BOOL },
    };
    struct blob_attr *tb[__INTF_STATUS_ATTR_MAX];

    blobmsg_parse( intf_status_policy, __INTF_STATUS_ATTR_MAX, tb, blob_data(msg), blob_len(msg) );
    if ( tb[INTF_STATUS_ATTR_UP] ) {
        up = blobmsg_get_bool( tb[INTF_STATUS_ATTR_UP] );
    }

    if ( tb[INTF_STATUS_ATTR_AUTOSTART] ) {
        autostart = blobmsg_get_bool( tb[INTF_STATUS_ATTR_AUTOSTART] );
    }

    if ( !up && !autostart ) {
        *status = VPN_CONN_STATUS_DISCONNECTED;
    }
    else if ( !up ) {
        *status = VPN_CONN_STATUS_CONNECTING;
    }
    else {
        *status = VPN_CONN_STATUS_CONNECTED;
    }

    cfmanager_log_message( L_DEBUG, "Get interface status %d from ubus!\n", *status );

    return;
}

//=============================================================================
int
cfubus_get_vpn_status(
    char *intf,
    int *status
)
//=============================================================================
{
    uint32_t id;
    char obj_path[BUF_LEN_64] = { 0 };
    *status = VPN_CONN_STATUS_CONNECTING ;

    snprintf( obj_path, sizeof(obj_path), "network.interface.%s", intf );
    if ( ubus_lookup_id ( ubus_ctx, obj_path, &id ) ) {
        return -1;
    }
    cfmanager_log_message( L_DEBUG, "Request interface %s status by ubus!\n", intf );

    return ubus_invoke( ubus_ctx, id, "status", NULL, cfubus_handle_vpn_status_data_cb, status, 3000);;        
}

//=============================================================================
int
cfubus_ctrl_vpn_status(
    char *intf,
    int action
)
//=============================================================================
{
    uint32_t id;
    char obj_path[BUF_LEN_64] = { 0 };
    int ret = 0;
    char *action2str[__VPN_CLIENT_ACTION_MAX] = {
        "connect",
        "concel",
        "disconnect"
    };

    snprintf( obj_path, sizeof(obj_path), "network.interface.%s", intf );
    if ( ubus_lookup_id ( ubus_ctx, obj_path, &id ) ) {
        return -1;
    }

    cfmanager_log_message( L_DEBUG, "Control interface %s to %s!\n", intf, action2str[action] );
    if ( action == VPN_CLIENT_ACTION_CONNECT ) {
        ret = ubus_invoke( ubus_ctx, id, "up", NULL, NULL, NULL, 3000);
    }
    else {
        ret = ubus_invoke( ubus_ctx, id, "down", NULL, NULL, NULL, 3000);
    }

    return ret;
}

//=============================================================================
void
cfubus_event_config_change(
    void
)
//=============================================================================
{
    static struct blob_buf b;
    blob_buf_init( &b, 0 );
    ubus_send_event( ubus_ctx, UBUS_EVENT_CONFIG_CHANGE, b.head );
    blob_buf_free( &b );

    return;
}

//=============================================================================
static void
cfubus_judge_need_close_radio_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct close_radio_params *params = (struct close_radio_params *)req->priv;
    struct blob_attr *tb[parse_channel_policy_sz];
    struct blob_attr *tb_tmp[parse_bandwidths_policy_sz];
    struct blob_attr *cur = NULL;
    struct blob_attr *attr = NULL;
    int i = 0;
    int j = 0;

    blobmsg_parse( parse_channel_policy,
        parse_channel_policy_sz,
        tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( !tb[0] ) {
        return;
    }

    blobmsg_for_each_attr( cur, tb[0], i ) {
        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_TABLE ) {
            continue;
        }

        blobmsg_parse( parse_bandwidths_policy,
            parse_bandwidths_policy_sz,
            tb_tmp,
            blobmsg_data( cur ),
            blobmsg_data_len( cur ) );

        blobmsg_for_each_attr( attr, tb_tmp[0], j ) {
                if ( blobmsg_type( attr ) != BLOBMSG_TYPE_STRING ) {
                    continue;
                }

                if( blobmsg_get_string( attr ) ) {
                    *params->need_close_radio = false;
                    return;
                }
        }
    }

    //It means that the channel information is not resolved
    *params->need_close_radio = true;
}

//=============================================================================
bool
cfubus_judge_need_close_radio(
    const char *country,
    const char *channel_with
)
//=============================================================================
{
    struct close_radio_params params = { 0 };
    bool need_close_radio = false;
    uint32_t search_id;

    if( !country || !channel_with ) {
        return need_close_radio;
    }

    params.channel_with = channel_with;
    params.need_close_radio = &need_close_radio;

    if ( ubus_lookup_id( ubus_ctx, "iwpriv_dfs_wifi_cap", &search_id ) ) {
        cfmanager_log_message( L_DEBUG, "not find iwpriv_dfs_wifi_cap\n" );
        return need_close_radio;
    }

    blob_buf_init( &reply, 0 );

    blobmsg_add_string( &reply, "country", country );
    blobmsg_add_string( &reply, "model", device_info.product_model );
    ubus_invoke( ubus_ctx, search_id, "get_channels", reply.head, cfubus_judge_need_close_radio_cb,
        &params, 2000 );

    blob_buf_free( &reply );

    return need_close_radio;
}

//=============================================================================
static void
cfubus_get_me_info_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    struct me_info *meif = (struct me_info *)req->priv;
    struct blob_attr *clients_tb[clients_list_policy_sz];
    struct blob_attr *tb[__ME_MAX];
    struct blob_attr *cur;
    int i;

    blobmsg_parse( clients_list_policy,
        clients_list_policy_sz,
        clients_tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( !clients_tb[0] ) {
        cfmanager_log_message( L_DEBUG, "Can't get 'me' information\n" );
        return;
    }

    blobmsg_for_each_attr( cur, clients_tb[0], i ) {
        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_TABLE ) {
            continue;
        }

        blobmsg_parse( cfubus_me_info_policy,
            __ME_MAX,
            tb,
            blobmsg_data( cur ),
            blobmsg_len( cur ) );

        if( tb[ME_MAC] ) {
            strncpy( meif->mac, blobmsg_get_string( tb[ME_MAC] ),
                MAC_STR_MAX_LEN );
        }

        //me info is only one
        return;
    }
}

//=============================================================================
void
cfubus_get_me_info(
    struct me_info *meif
)
//=============================================================================
{
    uint32_t search_id = 0;
    char cond_str[BUF_LEN_32] = { "{\"getme\":1}" };

    if( !meif ) {
        return;
    }

    if ( ubus_lookup_id( ubus_ctx, "clients_center", &search_id ) ) {
        cfmanager_log_message( L_ERR, "not find clients_center\n" );
        return;
    }

    blob_buf_init( &reply, 0 );
    blobmsg_add_string( &reply, "cond", cond_str );

    ubus_invoke( ubus_ctx, search_id,
        "get_clients_list", reply.head, cfubus_get_me_info_cb, meif, 1000 );

    blob_buf_free( &reply );
}

//=============================================================================
struct ubus_context *
cfubus_get_ubus_context(
    void
)
//=============================================================================
{
    return ubus_ctx;
}

//=============================================================================
static void
cfubus_get_netmask_cb(
    struct ubus_request *req,
    int type,
    struct blob_attr *msg
)
//=============================================================================
{
    unsigned int *netmask = (unsigned int*)req->priv;
    struct blob_attr *tb[network_interface_attr_policy_sz];
    struct blob_attr *ip_tb[ipv4_address_attr_policy_sz];
    int i = 0;
    struct blob_attr *cur_attr = NULL;

    blobmsg_parse( network_interface_attr_policy,
        network_interface_attr_policy_sz,
        tb,
        blob_data( msg ),
        blob_len( msg ) );

    if( !tb[0] ) {
        cfmanager_log_message( L_ERR, "No data parsed to 'ipv4-address'\n" );
        return;
    }

    blobmsg_for_each_attr( cur_attr, tb[0], i ) {
        if( BLOBMSG_TYPE_TABLE != blobmsg_type( cur_attr ) ) {
            continue;
        }

        blobmsg_parse( ipv4_address_attr_policy,
            ipv4_address_attr_policy_sz,
            ip_tb,
            blobmsg_data( cur_attr ),
            blobmsg_len( cur_attr ) );

        if( !ip_tb[0] ) {
            cfmanager_log_message( L_ERR, "No data parsed to 'mask'\n" );
            continue;
        }

        *netmask = blobmsg_get_u32( ip_tb[0] );
        break;
    }
}

//=============================================================================
unsigned int
cfubus_get_netmask(
    int wan_type
)
//=============================================================================
{
    const char *wan_str = NULL;
    uint32_t search_id = 0;
    unsigned int netmask = 0;
    char path[LOOKUP_STR_SIZE];

    if( WAN0 == wan_type ) {
        wan_str = "wan0";
    }
    else{
        wan_str = "wan1";
    }

    snprintf( path, sizeof( path ), "network.interface.%s", wan_str );
    if( ubus_lookup_id( ubus_ctx, path, &search_id ) ) {
        cfmanager_log_message( L_ERR, "ubus call network.interface.wan0 failed\n" );
        return 0;
    }

    ubus_invoke( ubus_ctx, search_id, "status", NULL, cfubus_get_netmask_cb, &netmask, 1000 );

    return netmask;
}

//=============================================================================
void
cfubus_resync_slave_cfg(
    void
)
//=============================================================================
{
    uint32_t search_id = 0;

    if( ubus_lookup_id( ubus_ctx, "controller.config", &search_id ) ) {
        cfmanager_log_message( L_ERR, "ubus call controller.config failed\n" );
        return;
    }

    ubus_invoke( ubus_ctx, search_id, "provision_aps", NULL, NULL, NULL, 5000 );
}
