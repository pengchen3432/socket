/****************************************************************************
* *
* * FILENAME:        $RCSfile: config.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/27
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
#include "cfparse.h"
#include "utils.h"
#include "ubus.h"
#include "config.h"
#include "wireless.h"
#include "sgrequest.h"
#include "network.h"
#include "dhcp.h"
#include "cfmanager.h"
#include "schedule.h"
#include "grandstream.h"
#include "controller.h"
#include "vpn.h"
#include "bwctrl.h"

//=================
//  Defines
//=================
#define SCHEDULE_COUNT_MAX          50

//=================
//  Typedefs
//=================
enum {
    ADDR_ATTR_ADDRESS,
    ADDR_ATTR_MASK,

    __ADDR_ATTR_MAX
};
//=================
//  Globals
//=================
extern const char *app_name;
extern const struct blobmsg_policy network_attrs_policy[__NETWORK_ATTR_MAX];
extern const struct blobmsg_policy wan_policy[__WAN_MAX];
extern const struct blobmsg_policy wireless_policy[__WIRELESS_MAX];
extern const struct blobmsg_policy lan_policy[__LAN_MAX];
extern const struct blobmsg_policy vlan_policy[__VLAN_MAX];
extern const struct blobmsg_policy switch_port_policy[__SWITCH_PORT_MAX];
extern const struct blobmsg_policy upgrade_auto_policy[__UPGRADE_AUTO_MAX];
extern const struct blobmsg_policy guest_ssid_policy[__GUEST_SSID_MAX];
extern const struct cm_vltree_info cm_vltree_info[__CM_VLTREE_MAX];
extern const struct blobmsg_policy cm_ap_policy[__CM_AP_MAX];
extern const struct blobmsg_policy cm_access_parse_policy[__CM_PARSE_ACCESS_MAX];
extern const struct blobmsg_policy cm_addit_ssid_policy[__CM_ADDIT_SSID_MAX];
//=================
//  Locals
//=================

/*
 * Private Functions
 */

static const struct blobmsg_policy ipv4_address_attrs_policy[__ADDR_ATTR_MAX] = {
    [ADDR_ATTR_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
    [ADDR_ATTR_MASK] = { .name = "mask", .type = BLOBMSG_TYPE_INT32 },
};

const struct blobmsg_policy route_attrs_policy[__ROUTE_ATTR_MAX] = {
    [ROUTE_ATTR_NEXTHOP] = { .name = "nexthop", .type = BLOBMSG_TYPE_STRING },
};

/*
 * Private Data
 */

//=================
//  Functions
//=================

//=============================================================================
int
config_init(
    void
)
//=============================================================================
{
    int ret = 0;
    char path_src[LOOKUP_STR_SIZE] = { 0 };
    char path_dest[LOOKUP_STR_SIZE] = { 0 };

    ret = access( CM_FOLDER, F_OK );
    if ( ret ) {
        mkdir( CM_FOLDER, 666 );
    }

    ret = access( CM_CONFIG_PATH, F_OK );
    if ( ret ) {
        mkdir( CM_CONFIG_PATH, 666 );
    }

    snprintf( path_src, sizeof( path_src ), "%s/%s",
        UCI_DEFAULT_PATH, CFMANAGER_CONFIG_NAME );
    snprintf( path_dest, sizeof( path_dest ), "%s/%s",
        CM_CONFIG_PATH, CFMANAGER_CONFIG_NAME );
    //Restore configuration
    util_cpy_file( path_src, path_dest );

    return 0;
}
//=============================================================================
int
config_uci_set(
    char* path,
    char* value,
    int commit
)
//=============================================================================
{
    struct uci_ptr ptr;
    char path_dup[LOOKUP_STR_SIZE] = { 0 };

    if( NULL == path || NULL == value ) {
        cfmanager_log_message( L_ERR, "Illegal parameter\n" );
        return -1;
    }

    /* To prevent configuration loss,
     * if this value is empty then it will not be set,
     * otherwise uci will delete this option
     */
    if( '\0' == value[0] ) {
        cfmanager_log_message( L_DEBUG, "path:%s  value:%s, value is empty, not set\n",
            path, value );
        return 0;
    }

    strncpy( path_dup, path, sizeof( path_dup ) -1 );
    memset( &ptr, 0, sizeof( struct uci_ptr ) );

    if ( uci_lookup_ptr( uci_ctx, &ptr, path_dup, false ) == UCI_OK ) {
        ptr.value = value;
        uci_set( uci_ctx, &ptr );
        uci_save( uci_ctx, ptr.p );
    }

    cfmanager_log_message( L_DEBUG, "path:%s   value:%s\n", path, value );
    if ( commit == 1 ) {
        uci_commit( uci_ctx, &ptr.p, false );
    }

    return 0;
}

//=============================================================================
int
config_set_option_not_exist(
    char* path,
    char* value,
    int commit
)
//=============================================================================
{
    struct uci_ptr ptr;
    struct uci_package* package = NULL;
    char path_dup[LOOKUP_STR_SIZE] = { 0 };
    const char * option = NULL;
    unsigned char need_set = 0;
    char package_name[BUF_LEN_32] = { 0 };
    char section_name[BUF_LEN_32] = { 0 };
    char option_name[BUF_LEN_32] = { 0 };
    int i = 0;
    int pos = 0;
    int package_pos = 0;

    if( NULL == path || NULL == value ) {
        cfmanager_log_message( L_ERR, "Illegal parameter\n" );
        return -1;
    }

    strncpy( path_dup, path, sizeof( path_dup ) -1 );

    while( '\0' != path_dup[i] && i < LOOKUP_STR_SIZE ) {
        if( '.' == path_dup[i] ) {
            if( 0 == pos ) {
                strncpy( package_name, path_dup, i );
                package_pos = i;
            }
            else if( 1 == pos ) {
                strncpy( section_name, path_dup + package_pos + 1, i - package_pos -1 );
                strncpy( option_name, path_dup + i + 1, sizeof( option_name ) -1 );
                break;
            }

            pos++;
        }

        i++;
    }

    memset( &ptr, 0, sizeof( struct uci_ptr ) );

    //Reload package, the config might have changed in the background
    package = cfparse_init_package( package_name );
    if ( !package ) {
        cfmanager_log_message( L_DEBUG, "Error loading %s\n", package_name );
        return -1;
    }

    if ( uci_lookup_ptr( uci_ctx, &ptr, path_dup, false ) == UCI_OK ) {
        struct uci_section *section = uci_lookup_section( uci_ctx, ptr.p, section_name );
        if ( section ) {
            option = uci_lookup_option_string( uci_ctx, section, option_name );
            if( !option ) {
                need_set = 1;
            }
        }
        else {
            need_set = 1;
        }

        if( !need_set ) {
            return 0;
        }

        ptr.value = value;
        uci_set( uci_ctx, &ptr );
        uci_save( uci_ctx, ptr.p );
    }

    if ( commit == 1 ) {
        uci_commit( uci_ctx, &ptr.p, false );
    }

    return 0;
}

//=============================================================================
int
config_set_by_blob(
    struct blob_attr *data,
    char *path,
    int blob_type
)
//=============================================================================
{
    char value[BUF_LEN_128] = { 0 };
    unsigned int rem;
    struct blob_attr *cur;
    if( NULL == path ) {
        return -1;
    }

    switch ( blob_type ) {
        case BLOBMSG_TYPE_BOOL:
            snprintf( value, sizeof( value ), "%d", util_blobmsg_get_bool( data, false ) );
            config_uci_set( path, value, 0 );
            break;
        case BLOBMSG_TYPE_STRING:
            strncpy( value, util_blobmsg_get_string( data, "" ), sizeof( value ) -1 );
            if( '\0' == value[0] ) {
                //Do not set empty data, delete if data is empty
                config_uci_del( path, 0 );
            }
            else {
                config_uci_set( path, blobmsg_get_string( data ), 0 );
            }
            break;
        case BLOBMSG_TYPE_INT32:
            snprintf( value, sizeof( value ), "%d", util_blobmsg_get_int( data, 0 ) );
            config_uci_set( path, value, 0 );
            break;
        case BLOBMSG_TYPE_ARRAY:
            config_uci_del( path, 0 );
            blobmsg_for_each_attr(cur, data, rem) {
                config_uci_add_list( path, blobmsg_get_string( cur ), 0 );
            }
            break;
        default:
            break;
    }

    return 0;
}

//=============================================================================
int
config_uci_del(
    char* path,
    int commit
)
//=============================================================================
{
    int ret = -1;
    struct uci_ptr ptr;
    char *path_dup = strdup( path );

    memset( &ptr, 0, sizeof( struct uci_ptr ) );

    if ( uci_lookup_ptr( uci_ctx, &ptr, path_dup, false ) == UCI_OK &&
         ( ptr.flags & UCI_LOOKUP_COMPLETE ) ) {
        uci_delete( uci_ctx, &ptr );
        ret = 0;
    }

    if ( commit == 1 ) {
        uci_commit( uci_ctx, &ptr.p, false );
    }

    cfmanager_log_message( L_DEBUG, "path:%s", path  )

    SAFE_FREE( path_dup );
    return ret;
}

//=============================================================================
int
config_uci_get_option(
    char* path,
    char* value,
    int size
)
//=============================================================================
{
    int ret = -1;
    struct uci_ptr ptr;
    char *path_dup = NULL;

    if ( NULL == path || NULL == value || 0 == size ) {
        cfmanager_log_message( L_ERR, "invalid arguments \n" );
        return ret;
    }

    path_dup = strdup( path );
    if ( uci_lookup_ptr( uci_ctx, &ptr, path_dup, false ) == UCI_OK ) {
        if ( NULL != ptr.o ){
            cfmanager_log_message( L_DEBUG, "value: %s, path:%s \n",
                ptr.o->v.string, path );
            strncpy( value, ptr.o->v.string, size - 1 );
            ret = 0;
        }
    }
    else {
        cfmanager_log_message( L_ERR, "error looking up path %s", path );
    }

    SAFE_FREE( path_dup );
    return ret;
}

//=============================================================================
int
config_get_uci_list(
    const char *package_name,
    const char *section_name,
    const char *option_name,
    struct blob_buf *b,
    const char *key,
    void (*cb)(struct blob_buf *b, const char *key, const char *value, const char *option_name)
)
//=============================================================================
{
    struct uci_package *package = NULL;
    struct uci_element *e = NULL;
    struct uci_element *list_element = NULL;
    struct uci_option *option = NULL;
    int ret = -1;
    char value[BUF_LEN_64];

    if( NULL == package_name || NULL == section_name || NULL == option_name  || NULL == b )
        return -1;

    package = cfparse_init_package( package_name );
    if( !package )
        return ret;

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( strcmp(s->e.name, section_name ) == 0 ) {
            option = uci_lookup_option(uci_ctx, s, option_name);

            if ( !option || option->type != UCI_TYPE_LIST )
                return ret;

            uci_foreach_element( &option->v.list, list_element ) {
                memset( value, 0, sizeof(value) );
                strncpy( value, list_element->name, sizeof(value)-1 );

                cb( b, key, value, section_name );
            }
        }
    }

    ret = 0;
    return ret;
}

//=============================================================================
int
config_set_section(
   const char *config,
   char *section,
   char *section_name
)
//=============================================================================
{
    struct uci_ptr ptr;
    struct uci_package* package = NULL;
    int ret = 0;
    char *section_name_dup = strdup( section_name );
    char path[LOOKUP_STR_SIZE] = { 0 };

    memset( &ptr, 0, sizeof( ptr ) );
    snprintf( path, sizeof( path ), "%s", config );

    if ( uci_lookup_ptr( uci_ctx, &ptr, path, false ) == UCI_OK ) {
        package = ptr.p;
    }

    if( !package ) {
        cfmanager_log_message( L_ERR, "package is NULL\n" );
        goto out;
    }

    ret = uci_add_section_named( uci_ctx, package, section, &ptr.s, section_name_dup );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_ERR, "Create %s section :%s failed\n",
            section, section_name_dup );
        goto out;
    }

    SAFE_FREE( section_name_dup );
    uci_commit( uci_ctx, &ptr.p, false );
    return 0;

out:
    SAFE_FREE( section_name_dup );
    return -1;
}

//=============================================================================
int
config_set_ssid_section(
    int network_type,
    int radio
)
//=============================================================================
{
    struct uci_package* package = NULL;
    struct uci_element* e = NULL;
    int i = 0;
    int flag = 0;
    const char *iface = NULL;
    char temp[BUF_LEN_8] = { 0 };
    char iface_name[BUF_LEN_8] = { 0 };

    //Save the configuration of the package before unloading it
    package = uci_lookup_package( uci_ctx, "wireless" );
    if( package ) {
        config_commit( "wireless", false );

        //Reload package
        package = NULL;
        package = uci_lookup_package( uci_ctx, "wireless" );
        if ( package ) {
            uci_unload( uci_ctx, package );
        }
    }

    if ( uci_load( uci_ctx, "wireless", &package ) ) {
        cfmanager_log_message( L_DEBUG, "uci_load wireless failed\n" );
        return -1;
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section(e);

        if ( strcmp(s->type, "wifi-iface") == 0 ) {
            iface = uci_lookup_option_string( uci_ctx, s, "ifname" );
            if ( !iface ) {
               continue;
            }

            if ( network_type == NET_TYPE_MESH && strncmp( iface, "mesh", 4 ) != 0 )
                continue;

            if ( network_type != NET_TYPE_MESH && strncmp( iface, "mesh", 4 ) == 0 )
                continue;

            sscanf( iface, "%[a-z]%d", temp, &i );

            flag |= BIT(i);
        }
    }

    for( i = 0; i < SSID_COUNT_MAX; i++ ) {
        //Get unused ifname number
        if( ! ( flag & BIT(i) ) ) {
            break;
        }
    }

    if( i >= SSID_COUNT_MAX ) {
        return -1;
    }

    if ( network_type == NET_TYPE_MESH ) {
        sprintf( iface_name, "mesh%d", i );
    }
    else {
        sprintf( iface_name, "ath%d", i );
#ifdef CONFIG_MTK
        if( RADIO_5G == radio ) {
            sprintf( iface_name, "athi%d", i );
        }
#endif
    }

    config_set_section( "wireless", "wifi-iface", iface_name );

    if ( network_type != NET_TYPE_MESH )
        config_set_section( "gsportalcfg", "wifi_portal", iface_name );

    return i;
}

//=============================================================================
int
config_lookup_list(
    char *package_name,
    char *list_name
)
//=============================================================================
{
    struct uci_ptr ptr;
    struct uci_package *package = NULL;
    struct uci_element *e;
    char path[LOOKUP_STR_SIZE];

    if( NULL == package_name || NULL == list_name ) {
        return CFG_FIND_NOT_EXIST;
    }

    memset( &ptr, 0, sizeof( ptr ) );
    snprintf( path, sizeof( path ), "%s", package_name );

    if ( uci_lookup_ptr( uci_ctx, &ptr, path, false ) == UCI_OK ) {
        package = ptr.p;
    }

    if ( !package ) {
        return CFG_FIND_NOT_EXIST;
    }

    uci_foreach_element( &package->sections, e ) {
        if( !strcmp( e->name, list_name ) ) {
            return CFG_FIND_EXIST;
        }
    }

    return CFG_FIND_NOT_EXIST;
}

//=============================================================================
int
config_uci_add_list(
    char *path,
    char *value,
    int commit
)
//=============================================================================
{
    int ret = -1;
    struct uci_ptr ptr;
    char *path_dup = strdup( path );

    memset( &ptr, 0, sizeof( ptr ) );

    if ( uci_lookup_ptr( uci_ctx, &ptr, path_dup, false ) == UCI_OK ) {
        ptr.value = value;
        uci_add_list(uci_ctx, &ptr);
        uci_save( uci_ctx, ptr.p );
        ret = 0;
    }

    if ( commit == 1 ) {
        uci_commit( uci_ctx, &ptr.p, false );
    }

    cfmanager_log_message( L_ERR, "add list path:%s value:%s", path, value );
    SAFE_FREE( path_dup );
    return ret;
}

//=============================================================================
int
config_uci_del_list(
    char *path,
    char *value,
    int commit
)
//=============================================================================
{
    int ret = -1;
    struct uci_ptr ptr;
    char *path_dup = strdup( path );

    memset( &ptr, 0, sizeof( ptr ) );

    if ( uci_lookup_ptr( uci_ctx, &ptr, path_dup, false ) == UCI_OK ) {
        ptr.value = value;
        uci_del_list( uci_ctx, &ptr );
        uci_save( uci_ctx, ptr.p );
        ret = 0;
    }

    if( commit == 1 ) {
        uci_commit( uci_ctx, &ptr.p, false );
    }

    cfmanager_log_message( L_ERR, "del list path:%s value:%s", path, value );
    SAFE_FREE( path_dup );
    return ret;
}

//=============================================================================
int
config_commit(
    const char *package_name,
    int overwrite
)
//=============================================================================
{
    struct uci_package *package = NULL;
    struct uci_ptr ptr;
    char path[LOOKUP_STR_SIZE];
    char path_dest[LOOKUP_STR_SIZE];
    int ret = 0;

    memset( &ptr, 0, sizeof( ptr ) );
    snprintf( path, sizeof( path ), "%s", package_name );

    if ( uci_lookup_ptr( uci_ctx, &ptr, path, false ) == UCI_OK ) {
        package = ptr.p;
    }

    if ( !package ) {
        cfmanager_log_message( L_DEBUG, "The package %s is NULL!\n", package_name );
        return ERRCODE_INTERNAL_ERROR;
    }

    ret = uci_save( uci_ctx, package );
    if ( ret != UCI_OK ) {
        cfmanager_log_message( L_DEBUG, "save %s failed\n", package_name );
        return ERRCODE_INTERNAL_ERROR;
    }

    ret = uci_commit( uci_ctx, &package, overwrite );
    if ( ret != UCI_OK  ) {
        cfmanager_log_message( L_DEBUG, "commit %s failed\n", package_name );
        return ERRCODE_INTERNAL_ERROR;
    }

    if( 0 == strcmp( package_name, CFMANAGER_CONFIG_NAME ) ) {
        //Save to flash
        snprintf( path, sizeof( path ), "%s/%s", CM_CONFIG_PATH, CFMANAGER_CONFIG_NAME );
        snprintf( path_dest, sizeof( path_dest ), "%s/%s", UCI_DEFAULT_PATH, CFMANAGER_CONFIG_NAME );
        util_cpy_file( path, path_dest );

        cfmanager_log_message( L_DEBUG, "synchronize files :%s", CFMANAGER_CONFIG_NAME );
    }
    else if( 0 == strcmp( package_name, CF_CONFIG_NAME_GRANDSTREAM ) ) {
        //Save to flash
        snprintf( path, sizeof( path ), "%s/%s", CM_CONFIG_PATH, CF_CONFIG_NAME_GRANDSTREAM );
        snprintf( path_dest, sizeof( path_dest ), "%s/%s", UCI_DEFAULT_PATH, CF_CONFIG_NAME_GRANDSTREAM );
        util_cpy_file( path, path_dest );

        cfmanager_log_message( L_DEBUG, "synchronize files :%s", CF_CONFIG_NAME_GRANDSTREAM );
    }

    return ERRCODE_SUCCESS;
}

//=============================================================================
int
config_add_named_section(
    const char *package_name,
    const char *section_type,
    const char *section_name
)
//=============================================================================
{
    struct uci_ptr ptr;
    struct uci_element* e = NULL;
    struct uci_package* p = NULL;
    struct uci_section* s = NULL;
    char path[LOOKUP_STR_SIZE];

    //Reload package, the config might have changed in the background
    p = uci_lookup_package( uci_ctx, package_name );
    if ( p )
        uci_unload( uci_ctx, p );

    if ( uci_load( uci_ctx, package_name, &p ) ) {
        cfmanager_log_message( L_DEBUG, "Error loading %s\n", package_name );
        return -1;
    }

    memset( &ptr, 0, sizeof( ptr ) );
    snprintf( path, sizeof( path ), package_name );

    if ( uci_lookup_ptr( uci_ctx, &ptr, path, false ) == UCI_OK
        && ( ptr.flags & UCI_LOOKUP_COMPLETE ) && ( p = ptr.p ) ) {
        uci_foreach_element( &p->sections, e ) {
            s = uci_to_section( e );
            if ( strcmp( s->type, section_type ) == 0
                && strcmp( section_name, s->e.name ) == 0 ) {
                break;
            }
            else
                s = NULL;
        }

        if ( s == NULL ) {
            uci_add_section_named( uci_ctx, p, section_type, &s, (char *)section_name );

            uci_save( uci_ctx, p );
            return 0;
        }
    }

    return -1;
}

//=============================================================================
int
config_del_named_section(
    const char *package_name,
    const char *section_type,
    const char *section_name
)
//=============================================================================
{
    struct uci_ptr ptr;
    int dummy;

    memset( &ptr,0,sizeof( ptr ) );

    char path[LOOKUP_STR_SIZE];
    snprintf( path, sizeof( path ), "%s.%s", package_name, section_name );
    if ( uci_lookup_ptr( uci_ctx, &ptr, path, true ) != UCI_OK )
        return -1;

    if ( ptr.value && !sscanf( ptr.value, "%d", &dummy ) )
        return -1;

    uci_delete( uci_ctx, &ptr );

    uci_save( uci_ctx, ptr.p );

    return 0;
}

//=============================================================================
int
config_set_schedule_section(
    void
)
//=============================================================================
{
    struct schedule_config_parse *schcfpar = NULL;
    int i = 0;
    int flag = 0;
    char *p = NULL;
    char buf[BUF_LEN_16] = { 0 };

    vlist_for_each_element( &sch_vltree, schcfpar, node ) {
        p = strstr( schcfpar->cf_section.name, "schedule" );
        if( !p ) {
            continue;
        }

        //Offset 8 bit to get the number after schedule
        i = atoi( p+8 );
        flag |= BIT(i);
    }

    for( i = 0; i < SCHEDULE_COUNT_MAX; i++ ) {
        //Get unused schedule number
        if( ! ( flag & BIT(i) ) )
            break;
    }

    if( i >= SCHEDULE_COUNT_MAX )
        return -1;

    sprintf( buf, "schedule%d", i );

    config_set_section( "schedule", "schedule", buf );

    return i;
}

//=============================================================================
static void
config_set_ssid_basic_info(
    char *dev_name,
    char *iface,
    int network_type,
    char *ssid_id
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( NULL == iface || NULL == ssid_id || NULL == dev_name ) {
        return;
    }

    snprintf( path, sizeof( path ), "wireless.%s.device", iface );
    config_uci_set( path, dev_name, 0 );

    snprintf( path, sizeof( path ), "wireless.%s.ifname", iface );
    config_uci_set( path, iface, 0 );

    snprintf( path, sizeof( path ), "wireless.%s.zone", iface );
    config_uci_set( path, "zone0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.network", iface );
    config_uci_set( path, "lan0_zone0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.mode", iface );
    config_uci_set( path, "ap", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.mcast_to_ucast", iface );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.bintval", iface );
    config_uci_set( path, "100", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.bms", iface );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.dtim_period", iface );
    config_uci_set( path, "1", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.macfilter", iface );
    config_uci_set( path, "deny", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.wmm", iface );
    config_uci_set( path, "1", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.voice_enterprise", iface );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.11R", iface );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.11K", iface );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.wnm", iface );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.portal_enable", iface );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.ClientIPAssignment", iface );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "wireless.%s.inact", iface );
    config_uci_set( path, "300", 0 );

    if ( NET_TYPE_MESH == network_type ) {
        snprintf( path, sizeof( path ), "wireless.%s.hidden", iface );
        config_uci_set( path, "1", 0 );
        snprintf( path, sizeof( path ), "wireless.%s.wds", iface );
        config_uci_set( path, "1", 0 );
    }

    snprintf( path, sizeof( path ), "wireless.%s.%s", iface, NET_TYPE_OPTION_NAME );
    switch( network_type ) {
        case NET_TYPE_MASTER:
            config_uci_set( path, NET_MASTER, 0 );
            break;
        case NET_TYPE_GUEST:
            config_uci_set( path, NET_GUEST, 0 );
            break;
        case NET_TYPE_MESH:
            config_uci_set( path, NET_MESH, 0 );
            break;
        case NET_TYPE_ADDIT:
            config_uci_set( path, NET_ADDIT, 0 );
            break;
        default:
            cfmanager_log_message( L_DEBUG, "Error network type:%d\n", network_type );
            break;
    }

    snprintf( path, sizeof( path ), "wireless.%s.%s", iface, wireless_iface_policy[WIRELESS_IFACE_ID].name );
    config_uci_set( path, ssid_id, 0 );

}

//=============================================================================
static char*
config_blacklist_get(
    void
)
//=============================================================================
{
    char *blacklist = NULL;
    char *more_blacklist = NULL;

    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb1[__CM_PARSE_ACCESS_MAX];

    int len = 0;
    int total_len = 0;
    char buffer[32];

    vlist_for_each_element( &cm_access_vltree, cm_cfg, node ) {
        const char *mac = NULL;
        const char *black = NULL;

        memset( buffer, 0, sizeof( buffer ) );
        blobmsg_parse( cm_access_parse_policy,
            __CM_PARSE_ACCESS_MAX,
            tb1,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        if ( !tb1[ CM_PARSE_ACCESS_MAC ] )
            continue;

        mac = blobmsg_get_string( tb1[ CM_PARSE_ACCESS_MAC ] );

        if ( !tb1[ CM_PARSE_ACCESS_BLACK ] )
            continue;

        black = blobmsg_get_string( tb1[ CM_PARSE_ACCESS_BLACK ] );

        if ( black && black[0] == '1' ) {
            if ( blacklist == NULL ) {
                len = strlen( mac );

                blacklist = (char *)malloc( len + 1 );
                if ( !blacklist )
                    return NULL;

                memcpy( blacklist, mac, len );
                total_len = len;
            }
            else {
                snprintf( buffer, sizeof( buffer ), " %s", mac );
                len = strlen( buffer );

                if ( !( more_blacklist =
                        realloc( blacklist, total_len + len + 1 ) ) ) {
                    free( blacklist );
                    return NULL;
                }
                else
                    blacklist = more_blacklist;

                memcpy( blacklist + total_len, buffer, len );
                total_len += len;
            }
        }
    }

    if ( blacklist )
        blacklist[total_len] = '\0';

    return blacklist;
}

//=============================================================================
int
config_create_ssid(
    char *ifname,
    int ifname_size,
    int radio,
    int network_type,
    char *ssid_id
)
//=============================================================================
{
    int iface_num = 0;
    char *dev_name = NULL;
    char *blacklist = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( NULL == ifname || 0 >= ifname_size || NULL == ssid_id ) {
        cfmanager_log_message( L_ERR, "Wrong parameters\n" );
        return ERRCODE_INTERNAL_ERROR;
    }

    if( RADIO_2G == radio ) {
        dev_name = "wifi0";
    }
    else if( RADIO_5G == radio ) {
        dev_name = "wifi1";
    }
    else {
        cfmanager_log_message( L_DEBUG, "error radio type:%d\n", radio );
        return ERRCODE_INTERNAL_ERROR;
    }

    iface_num = config_set_ssid_section( network_type, radio );
    if( 0 > iface_num ) {
        cfmanager_log_message( L_DEBUG, "Failed to create iface section\n" );
        return ERRCODE_INTERNAL_ERROR;
    }

    if ( network_type == NET_TYPE_MESH ) {
        snprintf( ifname, ifname_size, "mesh%d", iface_num );
    }
    else {
        snprintf( ifname, ifname_size, "ath%d", iface_num );
#ifdef CONFIG_MTK
        if( RADIO_5G == radio ) {
            snprintf( ifname, ifname_size, "athi%d", iface_num );
        }
#endif
    }

    //Add essential information
    config_set_ssid_basic_info( dev_name, ifname, network_type, ssid_id );

    blacklist = config_blacklist_get();
    if ( blacklist ) {
        snprintf( path, sizeof( path ), "wireless.%s.maclist", ifname );
        config_uci_set( path, blacklist, 0 );
        snprintf( path, sizeof( path ), "wireless.%s.macfilter", ifname );
        config_uci_set( path, "deny", 0 );
        free( blacklist );
    }

    config_commit( "wireless", false );
    if ( network_type != NET_TYPE_MESH )
        config_commit( "gsportalcfg", false );

    return ERRCODE_SUCCESS;
}

//=============================================================================
static void
config_del_wan_proto_info(
    int proto,
    const char *wan_str
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE];
    char *vpn_section = NULL;

    if( 0 == strcmp( "wan0", wan_str ) ) {
        vpn_section = WAN0_VPN_SECTION_NAME;
    }
    else {
        vpn_section = WAN1_VPN_SECTION_NAME;
    }

    switch ( proto ) {
        case WANTYPE_DHCP:
            break;
        case WANTYPE_STATIC:
            sprintf( path, "network.%s.ipaddr", wan_str );
            config_uci_del( path, 0 );

            sprintf( path, "network.%s.netmask", wan_str );
            config_uci_del( path, 0 );

            sprintf( path, "network.%s.gateway", wan_str );
            config_uci_del( path, 0 );
            break;
        case WANTYPE_PPTP:
            sprintf( path, "/etc/ppp/options.pptp-%s", vpn_section );
            unlink( path );
        case WANTYPE_L2TP:
            sprintf( path, "network.%s", vpn_section );
            config_uci_del( path, 0 );
            break;
        case WANTYPE_PPPOE:
            sprintf( path, "network.%s.username", wan_str );
            config_uci_del( path, 0 );

            sprintf( path, "network.%s.password", wan_str );
            config_uci_del( path, 0 );

            sprintf( path, "network.%s.pppd_options", wan_str );
            config_uci_del( path, 0 );
            break;
        default:
            cfmanager_log_message( L_DEBUG, "Unknown proto:%d\n", proto );
            break;
    };
}

#if 0
//=============================================================================
static void
config_set_pptp_special(
    const char *vpn_name
)
//=============================================================================
{
    FILE *pptp_fp = NULL;
    char pptp_opts[BUF_LEN_64] = { 0 };

    snprintf(pptp_opts, sizeof(pptp_opts), "/etc/ppp/options.pptp-%s", vpn_name);
    uci_clean_package(pptp_opts);

    pptp_fp = fopen(pptp_opts, "w");
    if (pptp_fp == (FILE *)NULL) {
        perror(pptp_opts);
        return;
    }
    /* default options */
    fputs("noipdefault\n", pptp_fp);
    fputs("noauth\n", pptp_fp);
    fputs("nopcomp\n", pptp_fp);
    fputs("noaccomp\n", pptp_fp);
    fputs("# noccp\n", pptp_fp);
    fputs("# novj\n", pptp_fp);
    fputs("nobsdcomp\n", pptp_fp);
    fputs("nodeflate\n", pptp_fp);
    fputs("maxfail 10\n", pptp_fp);
    fputs("lcp-echo-failure 3\n", pptp_fp);
    fputs("lcp-echo-interval 20\n", pptp_fp);
    fputs("idle 0\n", pptp_fp);

    fclose(pptp_fp);
}
#endif

//=============================================================================
static void
config_set_wan_vpn(
    struct blob_attr **new_config,
    char *wan_name,
    char *proto
)
//=============================================================================
{
    char *vpn_section_name = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char *metric = NULL;

    if( NULL == wan_name || NULL == proto ) {
        return;
    }

    if( 0 == strcmp( wan_name, "wan0" ) ) {
        vpn_section_name = WAN0_VPN_SECTION_NAME;
        metric = "60";
    }
    else {
        vpn_section_name = WAN1_VPN_SECTION_NAME;
        metric = "61";
    }

    config_set_section( "network", "interface", vpn_section_name );

    snprintf( path, sizeof( path ), "network.%s.metric", vpn_section_name );
    config_uci_set( path, metric, 0 );

    snprintf( path, sizeof( path ), "network.%s.proto", vpn_section_name );
    config_uci_set( path, proto, 0 );

    snprintf( path, sizeof( path ), "network.%s.delegate", vpn_section_name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "network.%s.interface", vpn_section_name );
    config_uci_set( path, wan_name, 0 );

    // Disable IPv6 negotiation
    snprintf( path, sizeof( path ), "network.%s.ipv6", vpn_section_name );
    config_uci_set( path, "0", 0 );

    if( 0 == strcmp( "pptp", proto ) ) {
        /* Done this in ppp.sh */
        // config_set_pptp_special( vpn_section_name );
        bool mppe = false;

        if ( !strcmp(wan_name, "wan0") ) {
            mppe = util_blobmsg_get_bool(new_config[WAN_PPTPMPPE], false);
        }
#ifdef DOUBLE_WAN
        else {
            mppe = util_blobmsg_get_bool(new_config[WAN1_PPTPMPPE], false);
        }
#endif
        snprintf( path, sizeof(path), "network.%s.mppe", vpn_section_name );
        if ( mppe ) {
            config_uci_set( path, "1", 0 );
        }
        else {
            config_uci_set( path, "0", 0 );
        }
    }
    else if( 0 == strcmp( "l2tp", proto ) ) {
        snprintf( path, sizeof( path ), "network.%s.checkup_interval", vpn_section_name );
        config_uci_set( path, "10", 0 );
    }
}

//=============================================================================
void
config_edit_wan_type(
    struct blob_attr **new_config,
    int new_type,
    char *wan_name,
    int commit
)
//=============================================================================
{
    //If the protocol is equal and return, then VPN and other protocols have a priority
    struct network_config_parse *nwcfpar = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };
    char proto_new[BUF_LEN_16] = { 0 };
    char *vpn_str = NULL;
    int vpn_type = -1;
    int wan_type = -1;

    if( WANTYPE_DHCP > new_type || __WANTYPE_MAX <= new_type ) {
        cfmanager_log_message( L_DEBUG, "Unknown wan type:%d\n", new_type );
        return;
    }

    if( 0 == strcmp ( "wan0", wan_name ) ) {
        vpn_str = WAN0_VPN_SECTION_NAME;
    }
    else {
        vpn_str = WAN1_VPN_SECTION_NAME;
    }

    nwcfpar = util_get_vltree_node( &network_interface_vltree, VLTREE_NETWORK, vpn_str );
    if( nwcfpar ) {
        vpn_type = nwcfpar->proto;
    }

    nwcfpar = util_get_vltree_node( &network_interface_vltree, VLTREE_NETWORK, wan_name );
    if( nwcfpar ) {
        wan_type = nwcfpar->proto;
    }

    sgreq_convert_sg_proto( new_type, proto_new, sizeof( proto_new ) );

    if( WANTYPE_PPTP == new_type || WANTYPE_L2TP == new_type ) {
        /* If it is a VPN, do not delete the original WAN port data.
         * The WAN port guarantees an IP address.
         * In fact, the configuration of VPN in WAN port rely on the configuration of WAN port last time.
         */
        if( 0 <= vpn_type ) {
            config_del_wan_proto_info( vpn_type, wan_name );
        }

        config_set_wan_vpn( new_config, wan_name, proto_new );
    }
    else {
        if( 0 <= wan_type ) {
            config_del_wan_proto_info( wan_type, wan_name );
        }

        if( 0 <= vpn_type ) {
            config_del_wan_proto_info( vpn_type, wan_name );
        }

        snprintf( path, sizeof( path ), "network.%s.proto", wan_name );
        config_uci_set( path, proto_new, 0 );
    }

    //Fixed value
    if( WANTYPE_PPPOE == new_type ) {
        snprintf( path, sizeof( path ), "network.%s.pppd_options", wan_name );
        config_uci_set( path, "debug", 0 );
    }

    cfmanager_log_message( L_DEBUG, "vpn_type:%d,wan_type:%d,new_type:%d,wan_name:%s\n",
        vpn_type, wan_type, new_type, wan_name );

    if ( commit == 1 ) {
        config_commit( "network", false );
    }
}

//=============================================================================
void
config_del_wan_dns_info(
    const char *wan_name,
    int commit
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( NULL == wan_name ) {
        return;
    }

    sprintf( path, "network.%s.dns", wan_name );

    config_uci_del( path, commit );
}

//=============================================================================
void
config_set_wan_link_speed(
    const char *wan_name,
    int link
)
//=============================================================================
{
    char cmd[BUF_LEN_64] = { 0 };
    int port = 0;

    if( NULL == wan_name ) {
        return;
    }

    if( 0 == strcmp( wan_name, "wan0" ) ) {
        port = WAN_LINK_PORT;
    }
    else if( 0 == strcmp( wan_name, "wan1" ) ) {
        port = WAN1_LINK_PORT;
    }
    else {
        cfmanager_log_message( L_DEBUG, "Error wan name :%s", wan_name );
        return;
    }

    if( WANLINKSPEED_AUTO == link ) {
        sprintf( cmd, "ssdk_sh port autoAdv set %d 0X023F", port );
    }
    else if( WANLINKSPEED_100M == link ) {
        sprintf( cmd, "ssdk_sh port autoAdv set %d 0X003C", port );
    }
    else if( WANLINKSPEED_1000M == link ) {
        sprintf( cmd, "ssdk_sh port autoAdv set %d 0X0230", port );
    }
    else {
        cfmanager_log_message( L_DEBUG, "Error link type:%d\n", link );
        return;
    }

    system( cmd );

    sprintf( cmd, "ssdk_sh port autoNeg restart %d", port );
    system( cmd );
}

//=============================================================================
void
config_get_ifname_by_ssid_name(
    const char *ssid_name,
    const char *dev_name,
    char *iface,
    int iface_size
)
//=============================================================================
{
    struct uci_ptr ptr;
    struct uci_package* package = NULL;
    struct uci_element* e = NULL;
    const char *value = NULL;
    char path[LOOKUP_STR_SIZE];

    if( NULL == ssid_name || NULL == dev_name || NULL == iface ) {
        cfmanager_log_message( L_DEBUG,
            "Some parameters are illegal\n" );

        return;
    }

    memset( &ptr, 0, sizeof( struct uci_ptr ) );
    snprintf( path, sizeof( path ), "%s", "wireless" );

    if ( uci_lookup_ptr( uci_ctx, &ptr, path, false ) == UCI_OK ) {
        package = ptr.p;
    }

    if ( !package ) {
        cfmanager_log_message( L_DEBUG, "The package 'wireless' is NULL\n" );
        return;
    }

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        if ( 0 != strcmp(s->type, "wifi-iface") ) {
            continue;
        }

        value = uci_lookup_option_string( uci_ctx, s, "ssid" );
        if( !value ) {
            continue;
        }

        if( 0 != strcmp( value, ssid_name ) ) {
            continue;
        }

        value = uci_lookup_option_string( uci_ctx, s, "device" );
        if( !value ) {
            continue;
        }

        if( 0 != strcmp( value, dev_name ) ) {
            continue;
        }

        strncpy( iface, s->e.name, iface_size );

        cfmanager_log_message( L_DEBUG, "find iface :%s\n", s->e.name );

        return;
    }
}

//=============================================================================
void
config_get_ifname(
    char *iface,
    int iface_size,
    int radio,
    int network_type,
    const char *ssid_id
)
//=============================================================================
{
    struct cfparse_wifi_device *cfwdev = NULL;
    struct cfparse_wifi_interface *vif = NULL;
    char *dev = NULL;

    if( NULL == iface ) {
        cfmanager_log_message( L_DEBUG,
            "Some parameters are illegal\n" );

        return;
    }

    if( RADIO_2G == radio ) {
        dev = "wifi0";
    }
    else if( RADIO_5G == radio ) {
        dev = "wifi1";
    }
    else {
        cfmanager_log_message( L_DEBUG, "Error radio:%d\n", radio );
        return;
    }

    cfwdev= util_get_vltree_node( &wireless_vltree, VLTREE_DEV, dev );
    if( !cfwdev ) {
        cfmanager_log_message( L_DEBUG, "not find %s\n", dev );
        return;
    }

    vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
        if( network_type != vif->network_type ) {
            continue;
        }

        if( NULL == ssid_id ) {
            strncpy( iface, vif->cf_section.name, iface_size -1 );
        }
        else {
            if( 0 == strcmp( ssid_id, vif->ssid_id ) ) {
                strncpy( iface, vif->cf_section.name, iface_size -1 );
            }
        }

        return;
    }
}

//=============================================================================
int
config_get_channel_width(
    int radio
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__WIRELESS_MAX];
    char *channel_width_str = NULL;

    cm_cfg = util_get_vltree_node( &cm_wireless_vltree, VLTREE_CM_TREE, "wireless" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_ERR, "not find wireless vltree in cfmanager\n" );
        return -1;
    }

    blobmsg_parse( wireless_policy,
        __WIRELESS_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    if( RADIO_2G == radio ) {
        channel_width_str = util_blobmsg_get_string( tb[WIRELESS_2G4CHANNELWIDTH], "" );
    }
    else {
        channel_width_str = util_blobmsg_get_string( tb[WIRELESS_5GCHANNELWIDTH], "" );
    }

    cfmanager_log_message( L_DEBUG, "channel_width_str:%s   radio:%d\n", channel_width_str, radio );

    return atoi( channel_width_str );
}

//=============================================================================
static void
config_set_2g_ht40(
    int channel_location
)
//=============================================================================
{
    switch( channel_location ) {
        case CL_AUTO:
            config_uci_set( "wireless.wifi0.htmode", "HT40", 0 );
            break;
        case CL_SECONDARY_BELOW_PRIMARY:
            config_uci_set( "wireless.wifi0.htmode", "HT40-", 0 );
            break;
        case CL_PRIMARY_BELOW_SECONDARY:
            config_uci_set( "wireless.wifi0.htmode", "HT40+", 0 );
            break;
        default:
            break;
    }
}

//=============================================================================
void
config_set_channel_width(
    int channel_width,
    int channel_location,
    int radio
)
//=============================================================================
{
    struct cfparse_wifi_global *cfwgl = NULL;
    char *channel_width_str = NULL;
    struct cfparse_wifi_device *cfwdev = NULL;
    struct cfparse_wifi_interface *vif = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( RADIO_2G == radio ) {
        cfwdev= util_get_vltree_node( &wireless_vltree, VLTREE_DEV, "wifi0" );
        if( !cfwdev ) {
            cfmanager_log_message( L_ERR, "not find wifi0\n" );
            return;
        }

        if( CW_2G_HT20 == channel_width ) {
            config_uci_set( "wireless.wifi0.htmode", "HT20", 0 );

            vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
                if( vif->disablecoext ) {
                    snprintf( path, sizeof( path ), "wireless.%s.disablecoext", vif->cf_section.name );
                    config_uci_del( path, 0 );
                }
            }
        }
        else if( CW_2G_HT20_HT40 == channel_width ) {
            config_set_2g_ht40( channel_location );

            vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
                snprintf( path, sizeof( path ), "wireless.%s.disablecoext", vif->cf_section.name );
                config_uci_set( path, "0", 0 );
            }
        }
        else {
            config_set_2g_ht40( channel_location );

            vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
                snprintf( path, sizeof( path ), "wireless.%s.disablecoext", vif->cf_section.name );
                config_uci_set( path, "1", 0 );
            }
        }
    }
    else {
        cfwgl = util_get_vltree_node( &wireless_global_vltree, VLTREE_DEV_GLOB, "qcawifi" );

        if( CW_5G_HT20 == channel_width ) {
            channel_width_str = "HT20";
        }
        else if( CW_5G_HT40 == channel_width ) {
            channel_width_str = "HT40";
        }
        else if( CW_5G_HT80 == channel_width ) {
            channel_width_str = "HT80";
        }
        else {
            channel_width_str = "HT160";
        }

        config_uci_set( "wireless.wifi1.htmode", channel_width_str, 0 );
        if( cfwgl ) {
            config_set_radio_enable( cfwgl->country, channel_width_str );
        }
    }
}

//=============================================================================
void
config_del_lan_dhcp(
    int commit
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };

    sprintf( path, "dhcp.lan0_zone0.start" );
    config_uci_del( path, 0 );

    sprintf( path, "dhcp.lan0_zone0.limit" );
    config_uci_del( path, 0 );

    sprintf( path, "dhcp.lan0_zone0.dhcp_option" );
    config_uci_del( path, 0 );

    sprintf( path, "dhcp.lan0_zone0.leasetime" );
    config_uci_del( path, 0 );

    if( 1 == commit ) {
        config_commit( "dhcp", false );
    }
}

//=============================================================================
void
config_del_lan_dns(
    char *section_name,
    int commit
)
//=============================================================================
{
    struct blob_attr *dp_tb[__DHCP_MAX];
    struct dhcp_config_parse *dpcfparse = NULL;
    struct blob_attr *cur;
    char path[BUF_LEN_64];
    char value[BUF_LEN_64];
    char *p = NULL;
    int i = 0;

    dpcfparse = util_get_vltree_node( &dhcp_vltree, VLTREE_DHCP, section_name );
    if( dpcfparse ) {
        blobmsg_parse( dhcp_policy,
            __DHCP_MAX,
            dp_tb,
            blob_data( dpcfparse->cf_section.config ),
            blob_len( dpcfparse->cf_section.config ) );
    }
    else {
        cfmanager_log_message( L_DEBUG, "Missing lan information\n" );
        return;
    }

    if( !dp_tb[DHCP_DHCP_OPTION] ) {
        cfmanager_log_message( L_DEBUG, "No lan dhcp_option information\n" );
        return;
    }

    blobmsg_for_each_attr( cur, dp_tb[DHCP_DHCP_OPTION], i ) {
        if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING ) {
            continue;
        }
        memset( path, 0, sizeof( path ) );
        snprintf( path, sizeof( path ), "dhcp.%s.dhcp_option", section_name );

        memset( value, 0, sizeof( value ) );
        strncpy( value, blobmsg_get_string( cur ), sizeof ( value ) - 1 );

        p = strtok( value, "," );
        if( NULL != p && 0 == strcmp( p, "6" ) ) {

            config_uci_del_list( path,
                blobmsg_get_string( cur ), commit );

            cfmanager_log_message( L_DEBUG, "Delete dhcp_option information:%s\n ",
                blobmsg_get_string( cur ) );

            return;
        }
    }
}

//=============================================================================
void
config_set_upgrade_auto_config(
    char *time,
    const char *sch_name,
    unsigned char sync_slave_enable
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };
    struct blob_attr *cm_tb[__CONTROLLER_MAX];
    struct cm_config *cm_cfg = NULL;
    char temp[BUF_LEN_16] = { 0 };
    int i = 0;

    if( NULL == time || NULL == sch_name ) {
        return;
    }

    cm_cfg = util_get_vltree_node( &cm_controller_vltree, VLTREE_CM_TREE, "main" );
    blobmsg_parse( controller_policy,
        __CONTROLLER_MAX,
        cm_tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config  ) );

    if ( ( device_info.work_mode == MODE_AP &&
        strncmp( blobmsg_get_string( cm_tb[CONTROLLER_ROLE] ), "slave", 5 ) == 0 ) &&
        !sync_slave_enable ) {
        return;
    }

    sprintf( path, "schedule.%s.upgrade", sch_name );
    config_uci_set( path, "1", 0 );

    //It needs to be updated automatically at this time every day
    for( i = 1;i <= 7; i++ ) {
        sprintf( temp, "wtime%d", i );
        sprintf( path, "schedule.%s.%s", sch_name, temp );
        config_uci_set( path, time, 0 );
    }
}

//=============================================================================
int
config_get_mtu_defvalue(
    int proto
)
//=============================================================================
{
    switch ( proto ) {
        case WANTYPE_DHCP:
            return DHCP_MTU_DEVALUE;
        case WANTYPE_STATIC:
            return STATIC_MTU_DEVALUE;
        case WANTYPE_PPPOE:
            return PPPOE_MTU_DEVALUE;
        case WANTYPE_PPTP:
            return PPTP_MTU_DEVALUE;
        case WANTYPE_L2TP:
            return L2TP_MTU_DEVALUE;
        default:
            cfmanager_log_message( L_DEBUG, "unknown proto:%d\n", proto );
            return -1;
    }
}

//=============================================================================
void
config_parse_ipv4_addr_attr(
    struct blob_attr *msg,
    char *mask_str,
    char *ip
)
//=============================================================================
{
    struct blob_attr *tb[__ADDR_ATTR_MAX];
    int mask = 0;

    if( NULL == msg || NULL == mask_str || NULL == ip ) {
        return;
    }

    blobmsg_parse( ipv4_address_attrs_policy,
        __ADDR_ATTR_MAX,
        tb,
        blobmsg_data( msg ),
        blobmsg_len( msg ) );

    if( tb[ADDR_ATTR_ADDRESS] ) {
        strncpy( ip, blobmsg_get_string( tb[ADDR_ATTR_ADDRESS] ), IP4ADDR_MAX_LEN );
    }

    if( tb[ADDR_ATTR_MASK] ) {
        mask = blobmsg_get_u32( tb[ADDR_ATTR_MASK] );
        util_convert_mask_to_str( mask, mask_str, IP4ADDR_MAX_LEN+1 );
    }
}

//=============================================================================
void
config_parse_route_attr(
    struct blob_attr *msg,
    char *nexthop
)
//=============================================================================
{
    struct blob_attr *tb[__ROUTE_ATTR_MAX];

    if( NULL == msg || NULL == nexthop )
        return;

    blobmsg_parse( route_attrs_policy,
        __ROUTE_ATTR_MAX,
        tb,
        blobmsg_data( msg ),
        blobmsg_len( msg ) );

    if( tb[ROUTE_ATTR_NEXTHOP] )
        strncpy( nexthop, blobmsg_get_string( tb[ROUTE_ATTR_NEXTHOP] ), IP4ADDR_MAX_LEN );
}

//=============================================================================
const char*
config_get_upgrade_auto_sch(
    void
)
//=============================================================================
{
    struct schedule_config_parse *schcfpar = NULL;

    vlist_for_each_element( &sch_vltree, schcfpar, node ) {
        if( schcfpar->sch_option & BIT( SCH_UPGRADE ) ) {
            return schcfpar->cf_section.name;
        }
    }

    return NULL;
}

//=============================================================================
int
config_creat_no_exist_section(
    const char *package_name,
    char *section_type,
    char *section_name,
    struct vlist_tree *vtree,
    int vtree_type
)
//=============================================================================
{
    int ret = 0;
    vtree_config_parse *cfg = NULL;

    cfg = util_get_vltree_node( vtree, vtree_type, section_name );
    if( !cfg ) {
        cfmanager_log_message( L_DEBUG, "No information found in %s\n", section_name );
        ret = config_add_named_section( package_name, section_type, section_name );
    }

    return ret;
}

//=============================================================================
int
config_add_ap_default(
    char *package_name,
    char *mac,
    char *type
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };

    config_add_named_section( package_name, "ap", mac );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_MAC].name );
    config_set_option_not_exist( path, mac, 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_TYPE].name );
    config_set_option_not_exist( path, type, 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_FREQUENCY].name );
    config_set_option_not_exist( path, "2", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_2G4_MODE].name );
    config_set_option_not_exist( path, "2", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_2G4_WIDTH].name );
    config_set_option_not_exist( path, "3", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_5G_WIDTH].name );
    config_set_option_not_exist( path, "4", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_2G4_POWER].name );
    config_set_option_not_exist( path, "4", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_5G_POWER].name );
    config_set_option_not_exist( path, "4", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_2G4_SHORTGI].name );
    config_set_option_not_exist( path, "1", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_5G_SHORTGI].name );
    config_set_option_not_exist( path, "1", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_BAND_STEERING].name );
    config_set_option_not_exist( path, "4", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_ZONES].name );
    config_set_option_not_exist( path, "zone0", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_SSIDS].name );
    config_set_option_not_exist( path, "ssid0", 0 );
    snprintf( path, LOOKUP_STR_SIZE, "%s.%s.%s", package_name, mac, cm_ap_policy[CM_AP_TLS1_2].name );
    config_set_option_not_exist( path, "1", 0 );

    return 0;
}

//=============================================================================
void
config_random_string(
    char* target,
    int size
)
//=============================================================================
{
    int i;
    unsigned int val;
    FILE *urand;
    const char available_chars[] =
        " !\"#$%&()*+,-./"
        "0123456789"
        ":;<=>?@"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "[\\]^_`"
        "abcdefghijklmnopqrstuvwxyz"
        "{|}~";

    memset( target, 0, size );

    urand = fopen( "/dev/urandom", "r" );
    for ( i = 0; i < size; i++ ) {
        fread( &val, sizeof( val ), 1, urand );
        target[i] = available_chars[val % ( sizeof( available_chars ) - 1 )];
    }
    fclose( urand );

    target[size - 1] = '\0';
}

//=============================================================================
static void
config_set_bwctrl(
    char *section_name,
    int size
)
//=============================================================================
{
    struct bwctrl_config_parse *bwctrl_node = NULL;
    int flag = 0;
    int i = 0;
    char temp[BUF_LEN_16] = { 0 };

    vlist_for_each_element( &bwctrl_rule_vltree, bwctrl_node, node ) {
        if( strstr( bwctrl_node->cf_section.name, "rule" ) ) {

            sscanf( bwctrl_node->cf_section.name, "%[a-z]%d", temp, &i );
            flag |= BIT(i);
        }
    }

    for( i = 0; i < SSID_COUNT_MAX; i++ ) {
        //Get unused ifname number
        if( ! ( flag & BIT(i) ) ) {
            break;
        }
    }

    snprintf( section_name, size, "rule%d", i );
    config_add_named_section( "bwctrl", "bwctrl-rule", section_name );
}

//=============================================================================
static int
config_get_bwctrl_section_name(
    const char *cm_id,
    char *section_name,
    int size
)
//=============================================================================
{
    struct bwctrl_config_parse *bwctrl_node = NULL;

    if( NULL == cm_id || NULL == section_name ) {
        cfmanager_log_message( L_ERR, "Illegal parameter\n" );
        return -1;
    }

    vlist_for_each_element( &bwctrl_rule_vltree, bwctrl_node, node ) {
        if( 0 == strcmp( bwctrl_node->id, cm_id ) ) {
            strncpy( section_name, bwctrl_node->cf_section.name, size-1 );
            return 0;
        }
    }

    return -1;
}

//=============================================================================
int
config_set_wireless_limit(
    struct bwctrl_param *bwctrl
)
//=============================================================================
{
    char up_rate_str[BUF_LEN_32];
    char down_rate_str[BUF_LEN_32];
    char path[LOOKUP_STR_SIZE];
    char section_name[BUF_LEN_16] = { 0 };
    int ret = 0;
    int up_rate = 0;
    int down_rate = 0;
    char *cm_id = NULL;
    char *iface_2g = NULL;
    char *iface_5g = NULL;

    if( !bwctrl ) {
        cfmanager_log_message( L_ERR, "Illegal parameter\n" );
    }

    up_rate = bwctrl->up_rate;
    down_rate = bwctrl->down_rate;
    cm_id = bwctrl->cm_id;
    iface_2g = bwctrl->iface_2g;
    iface_5g = bwctrl->iface_5g;

    ret = config_get_bwctrl_section_name( cm_id, section_name, sizeof( section_name ) -1 );
    if( 0 > ret ) {
        config_set_bwctrl( section_name, sizeof( section_name ) );
        snprintf( path, sizeof( path ), "bwctrl.%s.cm_id", section_name );
        config_uci_set( path, cm_id, 0 );
    }

    if( 0 == up_rate && 0 == down_rate ) {
        snprintf( path, sizeof( path ), "bwctrl.%s.enabled", section_name );
        config_uci_set( path, "0", 0 );
    }
    else if( 0 == up_rate || 0 == down_rate ) {
        snprintf( path, sizeof( path ), "bwctrl.%s.enabled", section_name );
        config_uci_set( path, "1", 0 );

        if( 0 == up_rate ) {
        snprintf( path, sizeof( path ), "bwctrl.%s.urate", section_name );
            config_uci_del( path, 0 );

            snprintf( path, sizeof( path ), "bwctrl.%s.drate", section_name );
            snprintf( down_rate_str, sizeof( down_rate_str ),
                 "%dKbps", down_rate );
            config_uci_set( path, down_rate_str, 0 );
        }
        else {
            snprintf( path, sizeof( path ), "bwctrl.%s.drate", section_name );
            config_uci_del( path, 0 );

            snprintf( path, sizeof( path ), "bwctrl.%s.urate", section_name );
            snprintf( up_rate_str, sizeof( up_rate_str ),
                "%dKbps", up_rate );
            config_uci_set( path, up_rate_str, 0 );
        }
    }
    else {
        snprintf( path, sizeof( path ), "bwctrl.%s.enabled", section_name );
        config_uci_set( path, "1", 0 );

        snprintf( path, sizeof( path ), "bwctrl.%s.urate", section_name );
        snprintf( up_rate_str, sizeof( up_rate_str ),
            "%dKbps", up_rate );
        config_uci_set( path, up_rate_str, 0 );

        snprintf( path, sizeof( path ), "bwctrl.%s.drate", section_name );
        snprintf( down_rate_str, sizeof( down_rate_str ), "%dKbps", down_rate );
        config_uci_set( path, down_rate_str, 0 );
    }

    snprintf( path, sizeof( path ), "bwctrl.%s.type", section_name );
    config_uci_set( path, "ssid", 0 );

    snprintf( path, sizeof( path ), "bwctrl.%s.dev", section_name );
    config_uci_del( path, 0 );

    if( '\0' != iface_2g[0] ) {
        config_uci_add_list( path, iface_2g, 0 );
    }
    if( '\0' != iface_5g[0] ) {
        config_uci_add_list( path, iface_5g, 0 );
    }

    return 0;
}

//=============================================================================
char*
config_get_iface_by_ssid_name(
    const char *ssid_name,
    int radio
)
//=============================================================================
{
    struct cfparse_wifi_device *cfwdev = NULL;
    struct cfparse_wifi_interface *vif = NULL;
    char *dev = NULL;

    if( NULL == ssid_name ) {
        cfmanager_log_message( L_DEBUG,
            "Some parameters are illegal\n" );

        return NULL;
    }

    if( RADIO_2G == radio ) {
        dev = "wifi0";
    }
    else if( RADIO_5G == radio ) {
        dev = "wifi1";
    }
    else {
        cfmanager_log_message( L_DEBUG, "Error radio:%d\n", radio );
        return NULL;
    }

    cfwdev= util_get_vltree_node( &wireless_vltree, VLTREE_DEV, dev );
    if( !cfwdev ) {
        cfmanager_log_message( L_DEBUG, "not find %s\n", dev );
        return NULL;
    }

    vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
        if( 0 == strcmp( vif->ssid_name, ssid_name ) ) {
            return vif->cf_section.name;
        }
    }

    return NULL;
}

//=============================================================================
int
config_set_client_limit_iface(
    char *path,
    int radio
)
//=============================================================================
{
    struct cfparse_wifi_device *cfwdev = NULL;
    struct cfparse_wifi_interface *vif = NULL;
    char *dev = NULL;

    if( NULL == path ) {
        return -1;
    }

    if( RADIO_2G == radio ) {
        dev = "wifi0";
    }
    else if( RADIO_5G == radio ) {
        dev = "wifi1";
    }
    else {
        cfmanager_log_message( L_DEBUG, "Error radio:%d\n", radio );
        return -1;
    }

    cfwdev= util_get_vltree_node( &wireless_vltree, VLTREE_DEV, dev );
    if( !cfwdev ) {
        cfmanager_log_message( L_DEBUG, "not find %s\n", dev );
        return -1;
    }

    vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
        //Speed limit for all SSIDs
        config_uci_add_list( path, vif->cf_section.name, 0 );
    }

    return 0;
}

//=============================================================================
void
config_del_all_iface(
    int radio
)
//=============================================================================
{
    struct cfparse_wifi_device *cfwdev = NULL;
    struct cfparse_wifi_interface *vif = NULL;
    char *dev = NULL;
    char path[LOOKUP_STR_SIZE];

    if( RADIO_2G == radio ) {
        dev = "wifi0";
    }
    else if( RADIO_5G == radio ) {
        dev = "wifi1";
    }
    else {
        cfmanager_log_message( L_DEBUG, "Error radio:%d\n", radio );
        return;
    }

    cfwdev= util_get_vltree_node( &wireless_vltree, VLTREE_DEV, dev );
    if( !cfwdev ) {
        cfmanager_log_message( L_DEBUG, "not find %s\n", dev );
        return;
    }

    vlist_for_each_element( &cfwdev->interfaces, vif, node ) {
        snprintf( path, sizeof( path ), "wireless.%s", vif->cf_section.name );
        config_uci_del( path, 0 );
    }
}

//=============================================================================
void
config_set_radio_enable(
    const char *country,
    const char *channel_with
)
//=============================================================================
{
    bool need_close_radio = false;

    if( !country || !channel_with ) {
        return;
    }

    need_close_radio = cfubus_judge_need_close_radio( country, channel_with );
    if( need_close_radio ) {
        //At present, only 5g radio may need to be shut down
        config_uci_set( "wireless.wifi1.disabled", "1", 0 );
    }
    else {
        config_uci_set( "wireless.wifi1.disabled", "0", 0 );
    }
}

//=============================================================================
void
config_sync(
    void
)
//=============================================================================
{
    char path_src[LOOKUP_STR_SIZE] = { 0 };
    char path_dest[LOOKUP_STR_SIZE] = { 0 };
    DIR *dir = NULL;
    struct dirent *dent = NULL;

    dir = opendir( CM_CONFIG_PATH );
    if ( dir == NULL ) {
        cfmanager_log_message( L_ERR, "opendir %s failed\n", CM_CONFIG_PATH );
        return;
    }

    for ( dent = readdir(dir); dent != NULL; dent = readdir(dir) ) {
        if ( dent->d_name[0] == '.' ) {
            continue;
        }
        if ( dent->d_type != DT_REG ) {
            continue;
        }

        //It was synchronized when it was saved
        if ( strcmp( dent->d_name, CFMANAGER_CONFIG_NAME ) == 0 ||
            strcmp( dent->d_name, CF_CONFIG_NAME_GRANDSTREAM ) == 0 ) {

            continue;
        }

        snprintf( path_src, sizeof( path_src ), "%s/%s", CM_CONFIG_PATH, dent->d_name );
        snprintf( path_dest, sizeof( path_dest ), "%s/%s", UCI_DEFAULT_PATH, dent->d_name );
        util_cpy_file( path_src, path_dest );
    }

    vpn_prepare_reload_ipsec();

    closedir( dir );

    cfmanager_log_message( L_DEBUG, "config sync\n" );
}

//=============================================================================
void
config_set_wan_alias_default_cfg(
    const char *section_name,
    int wan_type
)
//=============================================================================
{
    unsigned int netmask = 0;
    char value[BUF_LEN_64] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    char *wan_str = NULL;

    if( NULL == section_name ) {
        cfmanager_log_message( L_ERR, "the section name is NULL\n" );
        return;
    }

    if( WAN0 == wan_type ) {
        wan_str = "wan0";
    }
    else {
        wan_str = "wan1";
    }

    netmask = cfubus_get_netmask( wan_type );
    if( !netmask ) {
        cfmanager_log_message( L_ERR, "get netmask failed\n" );
        return;
    }

    //netmask is required
    util_convert_mask_to_str( netmask, value, sizeof( value ) );
    snprintf( path, sizeof( path ), "network.%s.netmask", section_name );
    config_uci_set( path, value, 0 );

    snprintf( path, sizeof( path ), "network.%s.proto", section_name );
    //The WAN alias only supports static protocols
    config_uci_set( path, "static", 0 );

    //interface is required
    snprintf( path, sizeof( path ), "network.%s.interface", section_name );
    config_uci_set( path, wan_str, 0 );

    //If no gateway is set, the default is used
}

//=============================================================================
int
config_vlan_is_enable(
    void
)
//=============================================================================
{
    struct blob_attr *tb[__VLAN_MAX];
    struct cm_config *cm_cfg = NULL;
    int vlan_enable = 0;

    vlist_for_each_element( &cm_vlan_vltree, cm_cfg, node ) {
        blobmsg_parse( vlan_policy,
            __VLAN_MAX,
            tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        if ( tb[VLAN_ID] && atoi( blobmsg_get_string( tb[VLAN_ID] ) ) > 1 ) {
            vlan_enable = 1;
            break;
        }
    }

    return vlan_enable;
}

//=============================================================================
bool
config_wan1_is_enable(
    void
)
//=============================================================================
{
    struct blob_attr *tb[__WAN_MAX];
    struct cm_config *cm_cfg = NULL;
    bool wan1_enable = false;

    cm_cfg = util_get_vltree_node( &cm_wan_vltree, VLTREE_CM_TREE, "wan" );
    if( !cm_cfg ) {
        cfmanager_log_message( L_ERR, "No wan found in %s, system error !", CFMANAGER_CONFIG_NAME );
    }
    else {
        blobmsg_parse( wan_policy,
            __WAN_MAX,
            tb,
            blob_data( cm_cfg->cf_section.config ),
            blob_len( cm_cfg->cf_section.config  ) );

        wan1_enable = util_blobmsg_get_bool( tb[WAN1_ENABLE], false );
    }

    return wan1_enable;
}

//=============================================================================
void
config_set_wireless(
    int option,
    char *iface,
    char *value,
    int radio
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( NULL == iface || NULL == value ) {
        cfmanager_log_message( L_ERR, "Illegal parameter\n" );
        return;
    }

    if( '\0' == iface[0] ) {
        return;
    }

    switch( option ) {
        case CM_ADDIT_SSID_NAME:
            snprintf( path, sizeof( path ), "wireless.%s.ssid", iface );
            config_uci_set( path, value, 0 );

            snprintf( path, sizeof( path ), "gsportalcfg.%s.ssid_name", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_SSIDHIDEENABLE:
            snprintf( path, sizeof( path ), "wireless.%s.hidden", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_DTIM_PERIOD:
            snprintf( path, sizeof( path ), "wireless.%s.dtim_period", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_CLIENT_LIMIT:
            snprintf( path, sizeof( path ), "wireless.%s.maxsta", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_STA_IDLE_TIMEOUT:
            snprintf( path, sizeof( path ), "wireless.%s.inact", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_UAPSD:
            snprintf( path, sizeof( path ), "wireless.%s.uapsd", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_PROXY_ARP:
            snprintf( path, sizeof( path ), "wireless.%s.proxyarp", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_MCAST_TO_UCAST:
            snprintf( path, sizeof( path ), "wireless.%s.mcast_to_ucast", iface );
            config_uci_set( path, value, 0 );
            snprintf( path, sizeof( path ), "wireless.%s.mcastenhance", iface );
            if( 0 == strcmp( value, "0" ) ) {
                config_uci_del( path, 0 );
            }
            else {
                config_uci_set( path, "2", 0 );
            }
            break;
        case CM_ADDIT_SSID_BMS:
            snprintf( path, sizeof( path ), "wireless.%s.bms", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_BRIDGE_ENABLE:
            snprintf( path, sizeof( path ), "wireless.%s.wds", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_ISOLATEMODE:
            snprintf( path, sizeof( path ), "wireless.%s.isolate", iface );
            switch( atoi( value ) ) {
                case ISOLATE_MODE_CLOSE:
                    config_uci_del( path, 0 );
                    snprintf( path, sizeof( path ), "wireless.%s.isolation_mode", iface );
                    config_uci_del( path, 0 );
                    snprintf( path, sizeof( path ), "wireless.%s.gateway_mac", iface );
                    config_uci_del( path, 0 );
                    break;
                case ISOLATE_MODE_RADIO:
                    config_uci_set( path, "1", 0 );
                    snprintf( path, sizeof( path ), "wireless.%s.isolation_mode", iface );
                    config_uci_set( path, "radio", 0 );
                    break;
                case ISOLATE_MODE_INTERNET:
                    config_uci_set( path, "1", 0 );
                    snprintf( path, sizeof( path ), "wireless.%s.isolation_mode", iface );
                    config_uci_set( path, "internet", 0 );
                    break;
                case ISOLATE_MODE_GATEWAY_MAC:
                    config_uci_set( path, "1", 0 );
                    snprintf( path, sizeof( path ), "wireless.%s.isolation_mode", iface );
                    config_uci_set( path, "gateway_mac", 0 );
                    break;
                default:
                    break;
            };
            break;
        case CM_ADDIT_SSID_GATEWAYMAC:
            snprintf( path, sizeof( path ), "wireless.%s.gateway_mac", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_80211W:
            snprintf( path, sizeof( path ), "wireless.%s.ieee80211w", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_VLAN_ID:
            snprintf( path, sizeof( path ), "wireless.%s.network", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_PORTAL_ENABLE:
            snprintf( path, sizeof( path ), "wireless.%s.portal_enable", iface );
            config_uci_set( path, value, 0 );
            break;
        case CM_ADDIT_SSID_PORTAL_POLICY:
            snprintf( path, sizeof( path ), "wireless.%s.portal_policy", iface );
            config_uci_set( path, value, 0 );
            break;
        default:
            break;
    }
}

//=============================================================================
void
config_set_rssi(
    bool enable,
    char *rssi_value,
    int radio
)
//=============================================================================
{
    struct cfparse_wifi_device *dev = NULL;
    struct cfparse_wifi_interface *iface = NULL;
    const char *dev_name = NULL;
    char value[BUF_LEN_8] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( RADIO_2G == radio ) {
        dev_name = "wifi0";
    }
    else {
        dev_name = "wifi1";
    }

    dev = util_get_vltree_node( &wireless_vltree, VLTREE_DEV, dev_name );
    if( !dev ) {
        cfmanager_log_message( L_ERR, "Missing %s node information\n", dev_name );
        return;
    }

    snprintf( value, sizeof( value ), "%d", enable );
    if( enable ) {
        vlist_for_each_element( &dev->interfaces, iface, node ) {
            snprintf( path, sizeof( path ),
                "wireless.%s.rssi_enable", iface->cf_section.name );
            config_uci_set( path, value, 0 );
            snprintf( path, sizeof( path ),
                "wireless.%s.rssi_threshold", iface->cf_section.name );
            config_uci_set( path, rssi_value, 0 );
        }
    }
    else {
        vlist_for_each_element( &dev->interfaces, iface, node ) {
            if( !iface->rssi_enable ) {
                continue;
            }
            snprintf( path, sizeof( path ),
                "wireless.%s.rssi_enable", iface->cf_section.name );
            config_uci_del( path, 0 );
            snprintf( path, sizeof( path ),
                "wireless.%s.rssi_threshold", iface->cf_section.name );
            config_uci_del( path, 0 );
        }
    }
}

//=============================================================================
void
config_set_radio_access_rate(
    bool enable,
    char *value,
    int radio
)
//=============================================================================
{
    struct cfparse_wifi_device *dev = NULL;
    struct cfparse_wifi_interface *iface = NULL;
    const char *dev_name = NULL;
    char temp[BUF_LEN_8] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( RADIO_2G == radio ) {
        dev_name = "wifi0";
    }
    else {
        dev_name = "wifi1";
    }

    dev = util_get_vltree_node( &wireless_vltree, VLTREE_DEV, dev_name );
    if( !dev ) {
        cfmanager_log_message( L_ERR, "Missing %s node information\n", dev_name );
        return;
    }

    snprintf( temp, sizeof( temp ), "%d", enable );
    if( enable ) {
        vlist_for_each_element( &dev->interfaces, iface, node ) {
            snprintf( path, sizeof( path ),
                "wireless.%s.minirate_enable", iface->cf_section.name );
            config_uci_set( path, temp, 0 );
            snprintf( path, sizeof( path ),
                "wireless.%s.minirate_threshold", iface->cf_section.name );
            config_uci_set( path, value, 0 );
        }
    }
    else {
        vlist_for_each_element( &dev->interfaces, iface, node ) {
            if( !iface->minirate_enable ) {
                continue;
            }
            snprintf( path, sizeof( path ),
                "wireless.%s.minirate_enable", iface->cf_section.name );
            config_uci_del( path, 0 );
            snprintf( path, sizeof( path ),
                "wireless.%s.minirate_threshold", iface->cf_section.name );
            config_uci_del( path, 0 );
        }
    }
}

//=============================================================================
void
config_set_beacon_interval(
    char *value,
    int radio
)
//=============================================================================
{
    struct cfparse_wifi_device *dev = NULL;
    struct cfparse_wifi_interface *iface = NULL;
    const char *dev_name = NULL;
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( RADIO_2G == radio ) {
        dev_name = "wifi0";
    }
    else {
        dev_name = "wifi1";
    }

    dev = util_get_vltree_node( &wireless_vltree, VLTREE_DEV, dev_name );
    if( !dev ) {
        cfmanager_log_message( L_ERR, "Missing %s node information\n", dev_name );
        return;
    }

    vlist_for_each_element( &dev->interfaces, iface, node ) {
        snprintf( path, sizeof( path ),
            "wireless.%s.bintval", iface->cf_section.name );
        config_uci_set( path, value, 0 );
    }
}

//=============================================================================
int
config_del_iface_section(
    char *ifname
)
//=============================================================================
{
    int ret = 0;

    if( !ifname ) {
        cfmanager_log_message( L_ERR, "ifname is NULL\n" );
        return -1;
    }

    ret = config_del_named_section( "wireless", "wifi-iface", ifname );
    if( ret < 0 ) {
        return ret;
    }

    ret = config_del_named_section( "gsportalcfg", "wifi_portal", ifname );
    if( ret < 0 ) {
        return ret;
    }

    return ret;
}

//=============================================================================
static void
config_del_old_crypto_option(
    int crypto_new,
    int crypto_old,
    char *ifname
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE];

    if( crypto_new == crypto_old ) {
        cfmanager_log_message( L_DEBUG, "Consistent encryption\n" );
        return;
    }

    switch( crypto_old ) {
        case CRYPTO_WPA_WPA2_PERSONAL:
        case CRYPTO_WPA2:
            snprintf( path, sizeof(path), "wireless.%s.encryption", ifname );
            config_uci_del( path, 0 );
            snprintf( path, sizeof(path), "wireless.%s.key", ifname );
            config_uci_del( path, 0 );
            break;
        case CRYPTO_WPA2_WPA3_PERSONAL:
        case CRYPTO_WPA3_PERSONAL:
            snprintf( path, sizeof(path), "wireless.%s.encryption", ifname );
            config_uci_del( path, 0 );
#if defined(GWN7062)
            snprintf( path, sizeof(path), "wireless.%s.sae", ifname );
            config_uci_del( path, 0 );
            snprintf( path, sizeof(path), "wireless.%s.sae_password", ifname );
            config_uci_del( path, 0 );
            snprintf( path, sizeof(path), "wireless.%s.add_sha256", ifname );
            config_uci_del( path, 0 );
#endif
            snprintf( path, sizeof(path), "wireless.%s.key", ifname );
            config_uci_del( path, 0 );
            break;
        case CRYPTO_OSEN:
            snprintf( path, sizeof(path), "wireless.%s.encryption", ifname );
            config_uci_del( path, 0 );
            snprintf( path, sizeof(path), "wireless.%s.osen", ifname );
            config_uci_del( path, 0 );
            break;
        case CRYPTO_OPEN:
            break;
        case CRYPTO_WPA3_192:
            snprintf( path, sizeof(path), "wireless.%s.encryption", ifname );
            config_uci_del( path, 0 );
            snprintf( path, sizeof(path), "wireless.%s.suite_b", ifname );
            config_uci_del( path, 0 );
            break;
        default:
            cfmanager_log_message( L_DEBUG, "Unknown encryption methods:%d", crypto_old );
            break;
    }
}

//=============================================================================
void
config_set_wireless_crypto(
    struct wireless_crypto_parameter *crypto_param
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE];
    char s_encryption[BUF_LEN_64];
    char s_encryption_bak[BUF_LEN_64];

    if( !crypto_param ) {
        cfmanager_log_message( L_ERR, "Parameter not legal" );
        return;
    }

    if( '\0' == crypto_param->ifname[0] ) {
        cfmanager_log_message( L_DEBUG, "radio:%d\n", crypto_param->radio );
        return;
    }

    config_del_old_crypto_option( crypto_param->crypto, crypto_param->crypto_old, crypto_param->ifname );

    switch( crypto_param->crypto ) {
        case CRYPTO_WPA_WPA2_PERSONAL:
        case CRYPTO_WPA2:
            if ( '1' == crypto_param->wpa_key_mode[0] ) {
                snprintf( s_encryption_bak, sizeof(s_encryption_bak), "wpa" );
            }
            else {
                snprintf( s_encryption_bak, sizeof(s_encryption_bak), "psk" );
            }

            if( CRYPTO_WPA2 == crypto_param->crypto ) {
                snprintf( s_encryption, sizeof( s_encryption ), "%s2", s_encryption_bak );
                strncpy( s_encryption_bak, s_encryption, sizeof(s_encryption_bak)-1 );
            }

            if ( '1' == crypto_param->wpa_crypto_type[0] ) {
                snprintf( s_encryption, sizeof( s_encryption ),"%s+tkip+aes", s_encryption_bak );
            }
            else {
                snprintf( s_encryption, sizeof( s_encryption ),"%s+aes", s_encryption_bak );
                strncpy( s_encryption_bak, s_encryption, sizeof(s_encryption_bak)-1 );
            }

            snprintf( path, sizeof(path), "wireless.%s.encryption", crypto_param->ifname );
            config_uci_set( path, s_encryption, 0 );

            snprintf( path, sizeof(path), "wireless.%s.key", crypto_param->ifname );
            config_uci_set( path, crypto_param->password, 0 );
            break;
        case CRYPTO_WPA2_WPA3_PERSONAL:
        case CRYPTO_WPA3_PERSONAL:
            if ( '1' == crypto_param->wpa_key_mode[0] ) {
#if defined(GWN7062)
                snprintf( s_encryption_bak, sizeof(s_encryption_bak), "wpa2" );
#else
                snprintf( s_encryption_bak, sizeof(s_encryption_bak), "wpa3" );
#endif
            }
            else {
                snprintf( s_encryption_bak, sizeof(s_encryption_bak), "sae" );
            }

            if ( CRYPTO_WPA2_WPA3_PERSONAL == crypto_param->crypto ) {
#if defined(GWN7062)
                if ( '0' == crypto_param->wpa_key_mode[0] ) {
                    snprintf( s_encryption, sizeof(s_encryption), "%s-psk2", s_encryption_bak );
                }
                else {
                    snprintf( s_encryption, sizeof(s_encryption), "%s", s_encryption_bak );
                }
#else
                snprintf( s_encryption, sizeof(s_encryption), "%s-mixed", s_encryption_bak );
#endif
                strncpy( s_encryption_bak, s_encryption, sizeof(s_encryption_bak)-1 );
            }

            if ( '1' == crypto_param->wpa_crypto_type[0] ) {
                snprintf( s_encryption, sizeof(s_encryption), "%s+tkip+aes", s_encryption_bak );
            }
            if ( '2' == crypto_param->wpa_crypto_type[0] ) {
                snprintf( s_encryption, sizeof(s_encryption), "%s+gcmp", s_encryption_bak );
            }
            if ( '3' == crypto_param->wpa_crypto_type[0] ) {
                snprintf( s_encryption, sizeof(s_encryption), "%s+gcmp-256", s_encryption_bak );
            }
            if ( '4' == crypto_param->wpa_crypto_type[0] ) {
                snprintf( s_encryption, sizeof(s_encryption), "%s+ccmp-256", s_encryption_bak );
            }
            else {
                snprintf( s_encryption, sizeof(s_encryption), "%s+aes", s_encryption_bak );
                strncpy( s_encryption_bak, s_encryption, sizeof(s_encryption_bak)-1 );
            }

            snprintf( path, sizeof(path), "wireless.%s.encryption", crypto_param->ifname );
            config_uci_set( path, s_encryption, 0 );

#if defined(GWN7062)
            if ( '0' == crypto_param->wpa_key_mode[0] ) {
                snprintf( path, sizeof(path), "wireless.%s.sae", crypto_param->ifname );
                config_uci_set( path, "1", 0 );

                if( '\0' != crypto_param->password[0] ) {
                    snprintf( path, sizeof(path), "wireless.%s.sae_password", crypto_param->ifname );
                    config_uci_set( path, crypto_param->password, 0 );
                }
            }

            snprintf( path, sizeof(path), "wireless.%s.add_sha256", crypto_param->ifname );
            config_uci_set( path, "1", 0 );
#endif
            if( '\0' != crypto_param->password[0] ) {
                snprintf( path, sizeof(path), "wireless.%s.key", crypto_param->ifname );
                config_uci_set( path, crypto_param->password, 0 );
            }
            break;
        case CRYPTO_OSEN:
            snprintf( s_encryption_bak, sizeof(s_encryption_bak), "wpa" );
            snprintf( s_encryption, sizeof(s_encryption), "%s2", s_encryption_bak );
            strncpy( s_encryption_bak, s_encryption, sizeof(s_encryption_bak)-1 );

            if ( '1' == crypto_param->wpa_crypto_type[0]) {
                snprintf( s_encryption, sizeof(s_encryption), "%s+tkip+aes", s_encryption_bak );
            }
            else {
                snprintf( s_encryption, sizeof(s_encryption),  "%s+aes", s_encryption_bak );
                strncpy( s_encryption_bak, s_encryption, sizeof(s_encryption_bak)-1 );
            }

            snprintf( path, sizeof(path), "wireless.%s.encryption", crypto_param->ifname );
            config_uci_set( path, s_encryption, 0 );

            // add ssid osen
            snprintf( path, sizeof(path), "wireless.%s.osen", crypto_param->ifname );
            config_uci_set( path, "1", 0 );
            break;
        case CRYPTO_OPEN:
            break;
        case CRYPTO_WPA3_192:
#if defined(GWN7062)
            snprintf( s_encryption_bak, sizeof(s_encryption_bak), "wpa2" );
#else
            snprintf( s_encryption_bak, sizeof(s_encryption_bak), "wpa3" );
#endif
            if ( '4' == crypto_param->wpa_crypto_type[0] ) {
                snprintf( s_encryption, sizeof(s_encryption), "%s+ccmp-256", s_encryption_bak );
            }
            else {
                snprintf( s_encryption, sizeof(s_encryption),"%s+gcmp-256", s_encryption_bak );
            }

            snprintf( path, sizeof(path), "wireless.%s.encryption", crypto_param->ifname );
            config_uci_set( path, s_encryption, 0 );

            snprintf( path, sizeof(path), "wireless.%s.suite_b", crypto_param->ifname );
            config_uci_set( path, "192", 0 );
            break;
        default:
            cfmanager_log_message( L_DEBUG, "Unknown encryption methods:%d", crypto_param->crypto );
            break;
    }
}

//=============================================================================
int
config_del_bwctrl_section(
    const char *cm_id
)
//=============================================================================
{
    int ret = 0;
    char section_name[BUF_LEN_16] = { 0 };

    if( !cm_id ) {
        return -1;
    }

    ret = config_get_bwctrl_section_name( cm_id, section_name, sizeof( section_name ) -1 );
    if( ret < 0 ) {
        return ret;
    }

    ret = config_del_named_section( "bwctrl", "bwctrl-rule",  section_name );
    return ret;
}

//=============================================================================
void
config_add_dev_ssid_id(
    const char *mac,
    const char *ssid_id
)
//=============================================================================
{
    char ssids[BUF_LEN_256] = { 0 };
    char temp[BUF_LEN_256] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( !mac || !ssid_id ) {
        return;
    }

    config_get_cm_ap_ssids( mac, ssids, sizeof(ssids) );
    if( '\0' == ssids[0] ) {
        snprintf( temp, sizeof(temp), "%s", ssid_id );
    }
    else {
        snprintf( temp, sizeof(temp), "%s %s", ssids, ssid_id );
    }

    snprintf( path, sizeof(path), "%s.%s.%s", CFMANAGER_CONFIG_NAME, mac, cm_ap_policy[CM_AP_SSIDS].name );
    config_uci_set( path, temp, 0 );

}

//=============================================================================
void
config_del_dev_ssid_id(
    const char *mac,
    const char *ssid_id
)
//=============================================================================
{
    char ssids[BUF_LEN_256] = { 0 };
    char temp[BUF_LEN_256] = { 0 };
    char path[LOOKUP_STR_SIZE] = { 0 };
    char substr[BUF_LEN_16] = { 0 };

    if( !mac || !ssid_id ) {
        return;
    }

    config_get_cm_ap_ssids( mac, ssids, sizeof(ssids) );
    if( '\0' == ssids[0] ) {
        return;
    }

    snprintf( temp, sizeof(temp), "%s ", ssids );
    snprintf( substr, sizeof(substr), "%s ", ssid_id );

    util_rm_substr( temp, substr );
    if( ' ' == temp[strlen(temp)-1] ) {
        temp[strlen(temp)-1] = '\0';
    }

    snprintf( path, sizeof(path), "%s.%s.%s", CFMANAGER_CONFIG_NAME, mac, cm_ap_policy[CM_AP_SSIDS].name );
    if( '\0' == temp[0] ) {
        config_uci_del( path, 0 );
    }
    else {
        config_uci_set( path, temp, 0 );
    }
}

//=============================================================================
void
config_get_cm_ap_ssids(
    const char *ap_mac,
    char *buf,
    int buf_size
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__CM_AP_MAX];

    if( !ap_mac || !buf ) {
        return;
    }

    cm_cfg = util_get_vltree_node( &cm_ap_vltree, VLTREE_CM_TREE, ap_mac );
    if( !cm_cfg ) {
        cfmanager_log_message( L_ERR, "not find dev:%s\n", ap_mac );
        return;
    }

    blobmsg_parse( cm_ap_policy,
        __CM_AP_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    if( !tb[CM_AP_SSIDS] ) {
        return;
    }

    strncpy( buf, blobmsg_get_string( tb[CM_AP_SSIDS] ), buf_size -1 );
}

//=============================================================================
int
config_get_schedule_ssids(
    int schedule_id,
    char *schedule_ssids
)
//=============================================================================
{
    struct blob_attr *tb[__CM_ADDIT_SSID_MAX];
    struct cm_config *cm_cfg = NULL;
    int len = 0;

    vlist_for_each_element( &cm_addit_ssid_vltree, cm_cfg, node ) {
        blobmsg_parse( cm_addit_ssid_policy,
            __CM_ADDIT_SSID_MAX,
            tb,
            blobmsg_data( cm_cfg->cf_section.config ),
            blobmsg_len( cm_cfg->cf_section.config ) );

        if ( !tb[CM_ADDIT_SSID_SCHEDULE_ENABLE] || !tb[CM_ADDIT_SSID_SCHEDULE_ID] )
            continue;

        if ( blobmsg_get_bool( tb[CM_ADDIT_SSID_SCHEDULE_ENABLE] ) ) {
            if ( schedule_id == atoi( blobmsg_get_string( tb[CM_ADDIT_SSID_SCHEDULE_ID] ) ) ) {
                len += sprintf( schedule_ssids + len, "%s,", blobmsg_get_string( tb[CM_ADDIT_SSID_ID] ) );
            }
        }
    }
    if ( schedule_ssids[len-1] == ',' ) {
        schedule_ssids[len-1] = '\0';
        len = len-1;
    }

    return len;
}

//=============================================================================
void
config_set_txpower(
    int txpower,
    int radio,
    char *custom_txpower
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE];

    if( RADIO_2G == radio ) {
        snprintf( path, sizeof( path ), "wireless.wifi0.txpower" );
    }
    else {
        snprintf( path, sizeof( path ), "wireless.wifi1.txpower" );
    }

    switch ( txpower ) {
        case TXPOWER_LOW:
            config_uci_set( path, POWER_LOW_STR, 0 );
            break;
        case TXPOWER_MEDIUM:
            config_uci_set( path, POWER_MEDIUM_STR , 0 );
            break;
        case TXPOWER_HIGH:
            config_uci_set( path, POWER_HIGH_STR , 0 );
            break;
        case TXPOWER_CUSTOM:
            if( custom_txpower ) {
                config_uci_set( path, custom_txpower , 0 );
            }
            else {
                cfmanager_log_message( L_ERR, "No custom power, default setting high power\n" );
                config_uci_set( path, POWER_HIGH_STR , 0 );
            }
            break;
        case TXPOWER_RRM:
            break;
        case TXPOWER_AUTO:
            config_uci_set( path, "auto" , 0 );
            break;
        default:
            cfmanager_log_message( L_DEBUG, "Unknown power value:%d\n", txpower );
            break;
    }
}

//=============================================================================
bool
config_get_ssid_enable(
    const char *ssid_id
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__CM_ADDIT_SSID_MAX];

    if( !ssid_id ) {
        return false;
    }

    cm_cfg = util_get_vltree_node( &cm_addit_ssid_vltree, VLTREE_CM_TREE, ssid_id );
    if( !cm_cfg ) {
        cfmanager_log_message( L_ERR, "not find ssid:%s\n", ssid_id );
        return false;
    }

    blobmsg_parse( cm_addit_ssid_policy,
        __CM_ADDIT_SSID_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    return util_blobmsg_get_bool( tb[CM_ADDIT_SSID_ENABLE], false );
}

//=============================================================================
void
config_get_ssid_name(
    const char *ssid_id,
    char *buf,
    int buf_size
)
//=============================================================================
{
    struct cm_config *cm_cfg = NULL;
    struct blob_attr *tb[__CM_ADDIT_SSID_MAX];

    if( !ssid_id || !buf || 0 > buf_size ) {
        return;
    }

    cm_cfg = util_get_vltree_node( &cm_addit_ssid_vltree, VLTREE_CM_TREE, ssid_id );
    if( !cm_cfg ) {
        cfmanager_log_message( L_ERR, "not find ssid:%s\n", ssid_id );
        return;
    }

    blobmsg_parse( cm_addit_ssid_policy,
        __CM_ADDIT_SSID_MAX,
        tb,
        blob_data( cm_cfg->cf_section.config ),
        blob_len( cm_cfg->cf_section.config ) );

    strncpy( buf, util_blobmsg_get_string( tb[CM_ADDIT_SSID_NAME], "" ), buf_size -1 );
}

//=============================================================================
bool
config_addit_ssid_set_dev(
    struct blob_attr *mac_attr,
    int type,
    const char *ssid_id
)
//=============================================================================
{
    struct blob_attr *cur = NULL;
    int rem = 0;
    char mac_str[MAC_STR_MAX_LEN+1];
    char ssids[BUF_LEN_256];
    bool config_change = false;

    if( !mac_attr || !ssid_id ) {
        cfmanager_log_message( L_ERR, "Parameter not legal\n" );
        return config_change;
    }

    if( DEV_MEMBER == type ) {
        blobmsg_for_each_attr( cur, mac_attr, rem ) {
            if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING ) {
                continue;
            }

            memset( mac_str, 0, sizeof( mac_str ) );
            memset( ssids, 0, sizeof( ssids ) );
            strncpy( mac_str, util_blobmsg_get_string( cur, ""), sizeof( mac_str ) -1 );

            config_get_cm_ap_ssids( mac_str, ssids, sizeof( ssids ) );

            //If the ssid id does not match in the added devices, the device need add this ssid id
            if( !util_match_ssids( ssids, ssid_id ) ) {
                config_add_dev_ssid_id( mac_str, ssid_id );
                config_change = true;
            }
         }
    }
    else {
        blobmsg_for_each_attr( cur, mac_attr, rem ) {
            if ( blobmsg_type( cur ) != BLOBMSG_TYPE_STRING ) {
                continue;
            }

            memset( mac_str, 0, sizeof( mac_str ) );
            memset( ssids, 0, sizeof( ssids ) );
            strncpy( mac_str, util_blobmsg_get_string( cur, ""), sizeof( mac_str ) -1 );

            config_get_cm_ap_ssids( mac_str, ssids, sizeof( ssids ) );

            if( !util_match_ssids( ssids, ssid_id ) ) {
                continue;
            }

            //If the ssid id is matched in an addable device, the ap should remove the ssid id
            config_del_dev_ssid_id( mac_str, ssid_id );
            config_change = true;
        }
    }

    return config_change;
}

//=============================================================================
void
config_set_cm_ssid_default(
    char *ssid_id
)
//=============================================================================
{
    char path[LOOKUP_STR_SIZE] = { 0 };

    if( !ssid_id ) {
        cfmanager_log_message( L_ERR, "miss ssid id" );
        return;
    }

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_ID].name );
    config_uci_set( path, ssid_id, 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_ENABLE].name );
    config_uci_set( path, "1", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_BAND].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_WPA_KEY_MODE].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_WPA_CRYPTO_TYPE].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_SSIDHIDEENABLE].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_ISOLATEMODE].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_DTIM_PERIOD].name );
    config_uci_set( path, "1", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_STA_IDLE_TIMEOUT].name );
    config_uci_set( path, "300", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_UAPSD].name );
    config_uci_set( path, "1", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_PROXY_ARP].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_MCAST_TO_UCAST].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_BMS].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_80211W].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_PORTAL_ENABLE].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_SCHEDULE_ENABLE].name );
    config_uci_set( path, "0", 0 );

    snprintf( path, sizeof( path ), "%s.%s.%s",
        CFMANAGER_CONFIG_NAME, ssid_id, cm_addit_ssid_policy[CM_ADDIT_SSID_VLAN_ENABLE].name );
    config_uci_set( path, "0", 0 );
}
