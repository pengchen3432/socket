/****************************************************************************
* *
* * FILENAME:        $RCSfile: utils.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/13
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "global.h"
#include "utils.h"
#include "network.h"
#include "dhcp.h"
#include "schedule.h"
#include "cfmanager.h"
#include "firewall.h"
#include "wireless.h"
#include "upnpd.h"
#include "bwctrl.h"
#include "grandstream.h"
#include "system.h"
#include "controller.h"
#include "tr069.h"
#include "gsportalcfg.h"


//=================
//  Defines
//=================

//=================
//  Typedefs
//=================

//=================
//  Globals
//=================
bool use_syslog = true;

//=================
//  Locals
//=================

/*
 * Private Functions
 */

/*
 * Private Data
 */
static int log_level = DEFAULT_LOG_LEVEL;

static const int log_class[] = {
    [L_EMERG] = LOG_EMERG,
    [L_ALERT] = LOG_ALERT,
    [L_CRIT] = LOG_CRIT,
    [L_ERR] = LOG_ERR,
    [L_WARNING] = LOG_WARNING,
    [L_NOTICE] = LOG_NOTICE,
    [L_INFO] = LOG_INFO,
    [L_DEBUG] = LOG_DEBUG,
};
//=================
//  Functions
//=================

//=============================================================================
void
cfmanager_log(
    int priority,
    const char *format,
    ...
)
//=============================================================================
{
    va_list vl;

    if ( priority > log_level )
        return;

    va_start(vl, format);
    if ( use_syslog )
        vsyslog(log_class[priority], format,vl);
    else
        vfprintf(stderr, format, vl);
    va_end(vl);
}

//=============================================================================
void
cfmanager_set_log_level(
    int level
)
//=============================================================================
{
    log_level = level;

    if ( log_level >= ARRAY_SIZE( log_class ) )
        log_level = ARRAY_SIZE( log_class ) - 1;
}

//=============================================================================
int
util_read_file_content(
    const char *path,
    char *value,
    int value_size
)
//=============================================================================
{
    FILE *fp = NULL;

    if( NULL == path || NULL == value || 0 >= value_size )
        return -1;

    fp = fopen( path, "r" );

    if( NULL == fp )
        return -1;

    fread(value, 1, value_size-1, fp);

    if( fp ) {
        fclose( fp );
        fp = NULL;
    }

    return 0;
}

//=============================================================================
void
util_convert_mask_to_str(
    int mask,
    char *mask_str,
    int size
)
//=============================================================================
{
    struct in_addr in;
    in_addr_t addr;

    if( NULL == mask_str )
        return;

    addr = ~( ( 1 << ( 32 - mask ) ) -1 );
    in.s_addr = htonl( addr );

    strncpy( mask_str, inet_ntoa(in), size-1 );
}

//=============================================================================
int
util_format_macaddr(
    char *in,
    char *out
)
//=============================================================================
{
    char mac[32] = {0};
    size_t i = 0, j = 0;

    strncpy( mac, in, sizeof( mac ) - 1 );
    while ( i < strlen( mac ) ) {
        if ( mac[i] == ':' ) {
            i++;
            continue;
        }
        else if ( 'A' <= mac[i] && mac[i] <= 'Z' ) {
            out[j] = mac[i] + 32;
        }
        else {
            out[j] = mac[i];
        }
        i++;
        j++;
    }

    return 0;
}

//=============================================================================
int
util_str_erase_colon(
    const char *in_str,
    char *out_str
)
//=============================================================================
{
    int i = 0, j = 0, len = 0;

    if ( in_str == NULL || out_str == NULL ) {
        return -1;
    }

    len = strlen( in_str );
    for ( ; i < len; ) {
        if ( in_str[i] == ':' ) {
            i++;
            continue;
        }
        else {
            out_str[j++] = in_str[i++];
        }
    }

    out_str[j] = '\0';

    return 0;
}

//=============================================================================
void
util_formatted_mac_with_colo(
    const char *mac,
    char *output
)
//=============================================================================
{
    snprintf( output, MAC_STR_MAX_LEN+1, "%.2s:%.2s:%.2s:%.2s:%.2s:%.2s",
        mac + 0, mac + 2, mac + 4,
        mac + 6, mac + 8, mac + 10);
}

//=============================================================================
int
util_str_to_low_case(
    char *str
)
//=============================================================================
{
    int i = 0, len = 0;

    if ( str == NULL ) {
        return -1;
    }

    len = strlen( str );
    for ( ; i < len; i++ ) {
        if ( str[i] >= 'A' &&
             str[i] <= 'Z' ) {
            str[i] += ('a' - 'A');
        }
    }

    return 0;
}

//=============================================================================
int
util_device_is_world_model(
    void
)
//=============================================================================
{
    char buf[32];
    static int flag = -1;

    if ( flag >= 0 )
        return flag;

    memset(buf, 0, sizeof(buf));
    int fd;
    fd = open( WORLD_MODLE_PATH, O_RDONLY );
    if ( fd < 0 ) {
        return 1;
    }
    read( fd, buf, sizeof ( buf ) );
    close( fd );
    if( 0 == strncmp( buf, STR_WORLD_MODLE, sizeof(STR_WORLD_MODLE)-1) ) {
        flag = 1;
    }
    else {
        flag = 0;
    }

    return flag;
}

//=============================================================================
void
util_escape_single_quote(
    const char *src,
    char *dest,
    int len
)
//=============================================================================
{
    int i = 0;
    int j = 0;

    if ( !src || !dest )
        return;

    for ( i = 0, j = 0; i < len - 1; i++, j++ ) {
        if ( src[j] == '\0' ) {
            dest[i] = '\0';
            break;
        }

        if ( src[j] == '\'' ) {
            dest[i++] = '\'';
            dest[i++] = '\\';
            dest[i++] = '\'';
            dest[i] = '\'';
        }
        else {
            dest[i] = src[j];
        }
    }

    return;
}

//=============================================================================
const char*
uci_lookup_option_string_from_package_and_section (
    struct uci_context *uci_ctx,
    struct uci_package *p,
    const char *section_str,
    const char *option_name
)
//=============================================================================
{
    const char * option = NULL;
    struct uci_section *section = uci_lookup_section( uci_ctx, p, section_str );
    if ( section ) {
        option = uci_lookup_option_string( uci_ctx, section, option_name );
    }

    return option;
}

//=============================================================================
int
interface_is_up(
    const char *devname
)
//=============================================================================
{
    int      ret = 1;
    int      fd;
    struct ifreq ifr;

    memset( &ifr, 0, sizeof( ifr ) );
    snprintf( ifr.ifr_name, sizeof( ifr.ifr_name ), "%s", devname );
    if ( ( fd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
        cfmanager_log_message( LOG_ERR, "socket() error: %s\n", strerror( errno ) );
        ret = 0;
        goto done;
    }

    if ( ioctl( fd, SIOCGIFFLAGS, &ifr ) < 0 ) {
        cfmanager_log_message( LOG_ERR, "ioctl() SIOCGIFFLAGS error: %s\n", strerror( errno ) );
        ret = 0;

        goto done;
    }

    ret = ( ( ifr.ifr_flags & IFF_UP ) > 0 ? 1 : 0 );

done:
    if ( fd > 0 ) close( fd );
    return ret;
}

//=============================================================================
void*
util_get_vltree_node(
    struct vlist_tree *vltree,
    int vltree_type,
    const char *key
)
//=============================================================================
{
    switch ( vltree_type ) {
        case VLTREE_CM_TREE: {
            struct cm_config *cm_cfg = NULL;

            cm_cfg = vlist_find( vltree, key, cm_cfg, node );
            return cm_cfg;
        }
        break;

        case VLTREE_DEV: {
            struct cfparse_wifi_device *cfwdev = NULL;

            cfwdev = vlist_find( vltree, key, cfwdev, node );
            return cfwdev;
        }
        break;

        case VLTREE_IFACE: {
            struct cfparse_wifi_interface *vif = NULL;

            vif = vlist_find( vltree, key, vif, node );
            return vif;
        }
        break;

        case VLTREE_DEV_GLOB: {
            struct cfparse_wifi_global *cfwgl = NULL;

            cfwgl = vlist_find( vltree, key, cfwgl, node );
            return cfwgl;
        }
        break;

        case VLTREE_NETWORK: {
            struct network_config_parse *nwcfpar = NULL;

            nwcfpar = vlist_find( vltree, key, nwcfpar, node );
            return nwcfpar;
        }
        break;

        case VLTREE_DHCP: {
            struct dhcp_config_parse *dpcfparse = NULL;

            dpcfparse = vlist_find( vltree, key, dpcfparse, node );
            return dpcfparse;
        }
        break;

        case VLTREE_SYS: {
            struct sys_config_parse *sys_cfg = NULL;
            sys_cfg = vlist_find( vltree, key, sys_cfg, node );
            return sys_cfg;
        }
        break;

        case VLTREE_CONTR: {
            struct controller_config_parse *concfparse = NULL;
            concfparse = vlist_find( vltree, key, concfparse, node );
            return concfparse;
        }
        break;

        case VLTREE_GRANDSTREAM: {
            struct grandstream_config_parse *control_cfg = NULL;
            control_cfg = vlist_find( vltree, key, control_cfg, node );
            return control_cfg;
        }
        break;

        case VLTREE_UPNPD: {
            struct upnpd_config_parse *upnpd_cfg = NULL;
            upnpd_cfg = vlist_find( vltree, key, upnpd_cfg, node );
            return upnpd_cfg;
        }
        break;
        case VLTREE_BWCTRL: {
            struct bwctrl_config_parse *bwctrl_cfpar = NULL;
            bwctrl_cfpar = vlist_find( vltree, key, bwctrl_cfpar, node );
            return bwctrl_cfpar;
        };
        break;
        case VLTREE_TR069: {
            struct tr069_config_parse *tr069_cfg = NULL;

            tr069_cfg = vlist_find( vltree, key, tr069_cfg, node );
            return tr069_cfg;
        }
        break;
        case VLTREE_SCHEDULE: {
            struct schedule_config_parse *schedule_cfg = NULL;

            schedule_cfg = vlist_find( vltree, key, schedule_cfg, node );
            return schedule_cfg;
        }
        break;

        default:
            return NULL;
    }
}

//=============================================================================
int
util_blobmsg_get_int(
    struct blob_attr *attr,
    int defalut_value
)
//=============================================================================
{
    const char *value = NULL;

    if ( !attr )
        return defalut_value;

    value = blobmsg_get_string( attr );
    if ( value == NULL )
        return defalut_value;

    if ( strlen( value ) == 0 )
        return defalut_value;

    return atoi( value );
}

//=============================================================================
char*
util_blobmsg_get_string(
    struct blob_attr *attr,
    char *defalut_value
)
//=============================================================================
{
    if ( !attr )
        return defalut_value;

    return blobmsg_get_string( attr );
}

//=============================================================================
bool
util_blobmsg_get_bool(
    struct blob_attr *attr,
    bool defalut_value
)
//=============================================================================
{
    if ( !attr )
        return defalut_value;

    return blobmsg_get_bool( attr );
}

//=============================================================================
int
util_split_string_data(
    json_object *data_obj,
    const char *in,
    char seperator,
    char assign
)
//=============================================================================
{
    char key[32] = { 0 };
    char value[128] = { 0 };
    char *p = key;
    unsigned int i = 0;

    for ( ; i < strlen( in ); i++ ) {
        if ( in[i] == assign ) {
            p = value;
            memset( value, 0x0, sizeof(value));
        } else if ( in[i] == seperator ) {
            json_object_object_add( data_obj, (const char *)key, json_object_new_string( value ) );
            memset( key, 0x0, sizeof(key));
            p = key;
        } else {
            *p = in[i];
            p++;
        }
    }
    json_object_object_add( data_obj, (const char *)key, json_object_new_string( value ) );

    return 0;
}

//=============================================================================
char *
util_rm_substr(
    char *str,
    const char *sub
)
//=============================================================================
{
    char *p, *q, *r;
    if ( ( q = r = strstr( str, sub ) ) != NULL ) {
        int len = strlen( sub );
        while ( ( r = strstr( p = r + len, sub ) ) != NULL ) {
            while ( p < r )
                *q++ = *p++;
        }
        while ( ( *q++ = *p++ ) != '\0' )
            continue;
    }
    return str;
}

//=============================================================================
int
util_parse_string_data(
    char *data,
    char *divider,
    const char *match,
    int pos,
    char *out,
    int out_size
)
//=============================================================================
{
    char *buf = NULL;
    char *p = NULL;
    int ret = -1;

    if( !data || !divider || !match || !out ) {
        return -1;
    }

    buf = strdup( data );

    p = strtok( buf, divider );
    if( !p ) {
        ret = -1;
        goto out;
    }
    if( strstr( p, match ) ) {
        strncpy( out, p + pos, out_size -1 );
        ret = 0;
        goto out;
    }

    while ( p ) {
        p = strtok( NULL, divider );
        if( !p ) {
            continue;
        }

        if( strstr( p, match ) ) {
            strncpy( out, p + pos, out_size -1 );
            ret = 0;
            goto out;
        }
    }

out:
    SAFE_FREE( buf );
    return ret;
}

//=============================================================================
void
util_sha256sum(
    const char *str,
    char *sum,
    int   size
)
//=============================================================================
{
    char cmd[512] = {0};
    char buf[128] = {0};
    FILE *pipe = NULL;
    int  len = 0;

    snprintf( cmd,  sizeof( cmd ), "echo -n \"%s\" | sha256sum | awk '{print $1}'", str );
    pipe = popen( cmd, "r" );
    if ( NULL == pipe ) {
        return;
    }

    fgets( buf, sizeof( buf ), pipe );
    pclose( pipe );

    len = strlen( buf );
    if ( buf[len - 1] == '\n' )
        buf[len - 1] = '\0';

    snprintf( sum, size, "%s", buf );
}

//=============================================================================
int
util_validate_ipv4(
    char *value
)
//=============================================================================
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, value, &(sa.sin_addr));
    return result != 0;
}

//=============================================================================
int
util_validate_ipv6(
    char *value
)
//=============================================================================
{
    struct sockaddr_in6 sa;
    int result = inet_pton(AF_INET6, value, &(sa.sin6_addr));
    return result != 0;
}

/*
 * Runs a shell command and copies the output to buffer.
 * Returns 0 on success, 1 otherwise
 */
//=============================================================================
int util_run_shell_command(
    const char * command,
    char * output,
    size_t len
)
//=============================================================================
{
    FILE * pipe;
    int status = 0;
    int offset = 0;
    int readlen = 0;

    cfmanager_log_message( L_DEBUG, "Executing: %s", command);
    pipe = popen(command, "r");

    if ( pipe != NULL ) {
        while ( 1 ) {
            readlen = fread(output+offset, 1, len - offset, pipe);
            if(readlen > 0)
            {
                offset+=readlen;
            }
            else if(readlen == 0)
            {
                break;
            }
            else
            {
                status = 1;
                cfmanager_log_message( L_ERR, "Failed to read the result of shell" );
                break;
            }
        }
        pclose(pipe);
        if ((output != NULL) && offset ) {                 // Add null terminator at the end of output buffer if it was provided
            output[offset+1] = '\0';
        }
    }
    else {
        cfmanager_log_message( L_DEBUG, "Failed to open pipe for executing shell" );
        status = 1;
    }

    if(output)
    {
        cfmanager_log_message( L_DEBUG, "Shell returned %zu bytes", strlen(output) );
    }
    return status;
}

//=============================================================================
uint32_t
utils_addr_to_int(
    const char *addr_string
)
//=============================================================================
{
    struct in_addr addr;
    inet_pton(AF_INET, addr_string, &addr);

    return ntohl(addr.s_addr);
}

//===============================================================================
void
utils_time_value2time_str(
    time_t time_val,
    char *time_str,
    int size
)
//=============================================================================
{
    time_t t = time_val;
    struct tm local_time;

    if ( NULL == time_str ) {
        cfmanager_log_message( L_ERR, " time_str is null !\n" );
        return;
    }

    memset( time_str, 0, size );

    localtime_r( &t, &local_time );

    snprintf( time_str, size, "%4d-%02d-%02d %02d:%02d",
        local_time.tm_year + 1900,
        local_time.tm_mon + 1,
        local_time.tm_mday,
        local_time.tm_hour,
        local_time.tm_min
        );

}

//=============================================================================
void
util_del_dup_cmd(
    char *cmd,
    int cmd_size
)
//=============================================================================
{
    //The format that comes in: cmd1&&cmd2&&cmd3&&
    char temp[BUF_LEN_32] = { 0 };
    char cmd_new[512] = { 0 };
    int i = 0;
    int pre = 0;

    if( !cmd ) {
        cfmanager_log_message( L_DEBUG, "cmd is NULL\n" );
        return;
    }

    while( cmd[i] != '\0' && i < cmd_size ) {
        if( cmd[i] == '&' ) {
            memset( temp, 0, sizeof( temp ) );
            //Take out one command
            if( pre ) {
                strncpy( temp, cmd+pre+2, i-pre-2 );
                if( !strstr( cmd_new, temp ) ) {
                    snprintf( cmd_new + strlen( cmd_new ),
                        sizeof( cmd_new ) - strlen( cmd_new ), "%s&&", temp );
                }

            }
            else {
                //The first command
                strncpy( temp, cmd, i );
                snprintf( cmd_new, sizeof( cmd_new ), "%s&&", temp );
            }

            pre = i;
            //'cmd1&&cmd2' skip'&'
            i += 2;
            continue;
        }

        i++;
    }

    memset( cmd, 0, cmd_size );
    strncpy( cmd, cmd_new, cmd_size );
}

//=============================================================================
void
util_send_uns_notification(
    struct ubus_context *ctx,
    const char* msg,
    int priority
)
//=============================================================================
{
   static struct blob_buf ubus_send_buf;

    json_object* notify_obj;
    blob_buf_init( &ubus_send_buf, 0 );

    notify_obj = json_object_new_string( "cfmanager" );
    blobmsg_add_json_element( &ubus_send_buf, "module",
        notify_obj );
    json_object_put( notify_obj );

    notify_obj = json_object_new_string( msg );
    blobmsg_add_json_element( &ubus_send_buf, "msg",
        notify_obj );
    json_object_put( notify_obj );

    notify_obj = json_object_new_int( priority );
    blobmsg_add_json_element( &ubus_send_buf, "pri",
        notify_obj );
    json_object_put( notify_obj );

    ubus_send_event( ctx, "grandstream", ubus_send_buf.head );

    blob_buf_free( &ubus_send_buf );
}

//=============================================================================
int
util_validate_legal(
    void *param,
    util_check_param type,
    int err_code
)
//=============================================================================
{
    int ret = ERRCODE_SUCCESS;
    const char *tmp = NULL;

    if( !param ) {
        cfmanager_log_message( L_DEBUG, "Illegal parameter\n" );
        return err_code;
    }

    switch( type ) {
        case CEK_SSID_NAME:
            tmp = util_blobmsg_get_string( (struct blob_attr*)param, "" );
            if( SSID_NAME_MAX_LEN < strlen( tmp ) ) {
                ret = err_code;
            }
            else {
                ret = ERRCODE_SUCCESS;
            }
            break;
        default:
            ret = err_code;

            cfmanager_log_message( L_DEBUG, "Unknown type:%d\n", type );
            break;
    }

    return ret;
}

//=============================================================================
int
util_cpy_file (
    const char *src,
    const char *dest
)
//=============================================================================
{
    int in_fd, out_fd, n_chars;
    char buf[4096] = { 0 };

    if ( NULL == src || NULL == dest ) {
        return -1;
    }

    if ( ( in_fd = open( src, O_RDONLY ) ) == -1 ){
        cfmanager_log_message( L_ERR, "Cannot open %s", src );
        return -1;
    }
    if ( ( out_fd = creat( dest, 0644 ) ) == -1 ){
        cfmanager_log_message( L_ERR, "Cannot creat %s", dest );
        return -1;
    }

    while ( ( n_chars = read( in_fd , buf, sizeof( buf ) ) ) > 0 ){
        if ( write( out_fd, buf, n_chars ) != n_chars ){
            cfmanager_log_message( L_ERR, "Write error to %s", dest );
        }
    }
    if ( n_chars == -1 ){
        cfmanager_log_message(L_ERR, "Read error from %s", src);
    }

    fsync( out_fd );

    if ( close( in_fd ) == -1 || close( out_fd ) == -1 ) {
        cfmanager_log_message( L_ERR, "Error closing files" );
        return -1;
    }

    return 0;
}

//=============================================================================
int
util_convert_specific_char(
        const char *src,
        char *dest,
        unsigned size,
        char old_char,
        char new_char
)
//=============================================================================
{
    int i = 0;
    int ret = 0;

    if(dest == NULL || src == NULL ) {
        return ret;
    }

    for( i = 0; i < (size-1); i++ ) {
        if( *(src+i) == '\0' ) {
            break;
        } else if( *(src+i) == old_char ) {
            *(dest+i) = new_char;
            ret++;
        } else {
            *(dest+i) = *(src+i);
        }
    }

    *(dest+i) = '\0';
    return ret;
}

//=============================================================================
bool
util_match_ssids(
    char *ssids,
    char *ssid
)
//=============================================================================
{
    char totalstr[BUF_LEN_256] = { 0 };
    char substr[BUF_LEN_16] = { 0 };

    if( !ssids || !ssid || BUF_LEN_256 < strlen(ssids) ) {
        return false;
    }

    strncpy( totalstr, ssids, sizeof( totalstr ) -1 );
    snprintf( totalstr, sizeof(totalstr), "%s ", ssids );
    snprintf( substr, sizeof(substr), "%s ", ssid );

    if( strstr( totalstr, substr ) ) {
        return true;
    }
    else {
        return false;
    }
}

//=============================================================================
bool
util_convert_string_to_bool(
    char *value
)
//=============================================================================
{
    if( !value ) {
        return false;
    }

    if( '0' == value[0] ) {
        return false;
    }
    else {
        return true;
    }
}

//=============================================================================
bool
utils_device_is_me(
    const char *mac
)
//=============================================================================
{
    char mac1[18] = { 0 };
    char mac2[18] = { 0 };

    if( !mac )
        return false;

    util_str_erase_colon( device_info.mac, mac1 );
    util_str_erase_colon( mac, mac2 );

    if( 0 == strcasecmp( mac1, mac2 ) )
        return true;
    else
        return false;
}

