#include <stdio.h>
#include <uci.h>
#include <uci_blob.h>
#include <libubus.h>
#include <libubox/list.h>
#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubox/vlist.h>
#include <libubox/utils.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
const char *app_name = "local";
static struct ubus_context *ubus_ctx = NULL;
static struct blob_buf b;
enum {
    LOCAL_READ_NAME,
    LOCAL_READ_AGE,
    LOCAL_READ_LIST,
    __LOCAL_READ_MAX,
};

enum {
    LOCAL_LIST_FIR,
    LOCAL_LIST_SEC,
    __LOCAL_LIST_MAX,
};

static int
local_write(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
);

static int
local_read(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
);
static void parse_local_list( 
    struct blob_attr *msg,
    struct blob_buf *b
);


struct blobmsg_policy local_read_policy[ __LOCAL_READ_MAX ] = {
    [LOCAL_READ_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [LOCAL_READ_AGE] = { .name = "age", .type = BLOBMSG_TYPE_INT32 },
    [LOCAL_READ_LIST] = { .name = "list", .type = BLOBMSG_TYPE_ARRAY },
};

struct blobmsg_policy local_list_policy[ __LOCAL_LIST_MAX ] = {
    [LOCAL_LIST_FIR] = { .name = "first", .type = BLOBMSG_TYPE_STRING },
    [LOCAL_LIST_SEC] = { .name = "second", .type = BLOBMSG_TYPE_STRING },
};

struct ubus_method local_methods[] = {
    UBUS_METHOD_NOARG( "write", local_write ),
    UBUS_METHOD( "read", local_read, local_read_policy ),
};

struct ubus_object_type local_type = 
    UBUS_OBJECT_TYPE( "local", local_methods );
struct ubus_object local_obj = {
    .name = "local",
    .type = &local_type,
    .methods = local_methods,
    .n_methods = ARRAY_SIZE( local_methods ),
};


int main()
{
    uloop_init();
    ubus_ctx = ubus_connect( NULL );
    ubus_add_uloop( ubus_ctx );
    ubus_add_object( ubus_ctx, &local_obj );

    uloop_run();
    
    return 0;
}


static int
local_write(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
)
{
    int fd;
    fd = open( "/tmp/local", O_CREAT | O_RDWR | O_EXCL | O_APPEND, 0666 );
    if ( fd < 0 ) {
        if ( errno == EEXIST ) {
            syslog( LOG_ERR, "file exit\n" );
        }
        else {
            syslog( LOG_ERR, "local open failed\n" );
        }
    }
    write( fd, "hello world", strlen( "hello world" ) );
    close(fd);
    return 0;
}

static int
local_read(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg
)
{
    char *value = NULL;
    int age = 0, i;
    struct blob_attr *tb[__LOCAL_READ_MAX] = {0};
    struct blob_attr *cur;
    int rem;
    int type;
    void *c;
    blob_buf_init( &b, 0 );
    blobmsg_parse( local_read_policy,
        __LOCAL_READ_MAX,
        tb,
        blobmsg_data( msg ),
        blobmsg_len( msg )
    );
    for ( i = 0; i < __LOCAL_READ_MAX; i++ ) {
        if ( !tb[i] ) {
            blobmsg_add_string( &b, local_read_policy[i].name, "" );
            syslog( LOG_ERR, "%s pararm %s is NULL\n", app_name, local_read_policy[i].name );
            continue;
        }
        if ( local_read_policy[i].type == BLOBMSG_TYPE_STRING ) {
            value = blobmsg_get_string( tb[i] );
            blobmsg_add_string( &b, local_read_policy[i].name, value );
            syslog( LOG_DEBUG, "%s: %s=%s", app_name, local_read_policy[i].name, value );
        }
        else if ( local_read_policy[i].type == BLOBMSG_TYPE_INT32 ) {
            age = blobmsg_get_u32( tb[i] );
            blobmsg_add_u32( &b, local_read_policy[i].name, age );
            syslog( LOG_DEBUG, "%s: %s=%d", app_name, local_read_policy[i].name, age );
        }
        else {
            c = blobmsg_open_array( &b, "list" );

            blobmsg_for_each_attr( cur, tb[i], rem ) {
                type = blobmsg_type( cur );
                syslog( LOG_DEBUG, "%s: type=%d", app_name, type );
                parse_local_list( cur, &b );
            }
            blobmsg_close_array( &b, c );
        }
    }
    ubus_send_reply( ctx, req, b.head );
    blob_buf_free( &b );
    return 0;
}
static void parse_local_list( 
    struct blob_attr *msg,
    struct blob_buf *b
)
{
    struct blob_attr *tb[__LOCAL_LIST_MAX] = {0};
    int i;
    char *value = NULL;
    blobmsg_parse( local_list_policy,
        __LOCAL_LIST_MAX,
        tb,
        blobmsg_data( msg ),
        blobmsg_len( msg )
    );
    void *d;
    d = blobmsg_open_table( b, NULL );
    for ( i = 0; i < __LOCAL_LIST_MAX; ++i ) {
        if ( !tb[i] ) {
            blobmsg_add_string( b, local_list_policy[i].name, "" );  
            syslog( LOG_ERR, "%s pararm %s is NULL\n", app_name, local_list_policy[i].name );
            continue;
        }
        if ( local_list_policy[i].type == BLOBMSG_TYPE_STRING ) {
            value = blobmsg_get_string( tb[i] );
            blobmsg_add_string( b, local_list_policy[i].name, value );  
            syslog( LOG_DEBUG, "%s: %s=%s", app_name, local_list_policy[i].name, value );
        }
    }
    blobmsg_close_table( b, d );
}
