/****************************************************************************
* *
* * FILENAME:        $RCSfile: ipc.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/18
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
#include "ipc.h"
#include "global.h"
#include "utils.h"
#include "gs_ipc_msg.h"
#include "network.h"
#include "config.h"

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
enum {
    IPC_INFO_TYPE,
    IPC_INFO_CONTENT,

    __IPC_INFO_MAX
};

enum {
    IPC_ME_MAC,
    IPC_ME_WAN,

    __IPC_ME_MAX
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
static struct uloop_fd uloop_fd;
static gs_ipc_srv_t gs_ipc_srv_cm;

static const struct blobmsg_policy sg_ipc_info_policy[__IPC_INFO_MAX] = {
    [IPC_INFO_TYPE] = { .name = "content_type", .type = BLOBMSG_TYPE_INT32 },
    [IPC_INFO_CONTENT] = { .name = "content", .type = BLOBMSG_TYPE_TABLE },
};

//=================
//  Functions
//=================

//=============================================================================
static int
cfmanager_ipc_handle_sg_info(
    uint32 size,
    void *data
)
//=============================================================================
{
    struct blob_attr *attr = NULL;
    struct blob_attr *tb[__IPC_INFO_MAX];
    int type = 0;
    int ret = 0;

    if( !data ) {
        cfmanager_log_message( L_ERR, "receive sg ipc msg is NULL\n" );
        return -1;
    }

    attr = ( struct blob_attr * )data;
    blobmsg_parse( sg_ipc_info_policy,
        __IPC_INFO_MAX,
        tb,
        blob_data( attr ),
        blob_len( attr ) );

    type = blobmsg_get_u32( tb[IPC_INFO_TYPE] );
    switch( type ) {
        default:
            cfmanager_log_message( L_DEBUG,
                "recv unknown sg info type:%d\n", type );

            ret = -1;
            break;
    }

    return ret;
}

//=============================================================================
static void
cfmanager_ipc_proc_msg(
    uint32 msg_type,
    uint32 size,
    void *data
)
//=============================================================================
{
    switch (msg_type) {
        case IPC_MSG_SG:
            cfmanager_ipc_handle_sg_info( size, data );
            cfmanager_log_message( L_ERR,
                "receive sg ipc msg (%d)\n", msg_type );
            break;
        default:
            cfmanager_log_message( L_ERR,
                "receive unknown ipc msg (%d)\n", msg_type );
            break;
    }
}

//=============================================================================
static int
cfmanager_ipc_sync_msg_cb(
    char *data,
    size_t len
)
//=============================================================================
{
    int ret = 0;
    ipc_msg_t *msg = (ipc_msg_t *)data;
    ipc_msg_hdr_ntoh( msg );

    cfmanager_log_message( L_DEBUG, "sync recv msg_type=%d, msg_len=%d\n",
        msg->msg_type, msg->msg_len);
    cfmanager_ipc_proc_msg( msg->msg_type, msg->msg_len, msg->data );

    return ret;
}

//=============================================================================
static void
cfmanager_ipc_recv_cb(
    struct uloop_fd* ufd,
    unsigned int events
)
//=============================================================================
{
    uint32_t    res;

    /* It's stranged that client address can not be returned from recvfrom(),
       it's seems to be a known issue for unix domain socket, refer to the following link:
       http://stackoverflow.com/questions/31755790/sockets-unix-domain-udp-c-recvfrom-fail-to-populate-the-source-address.
       temporary solution is adding a additional header to carry the sender address, so we can reply to it.
    */
    res = recvfrom( uloop_fd.fd, gs_ipc_srv_cm.recv_buf,
        sizeof(gs_ipc_srv_cm.recv_buf) - 1, 0, NULL, NULL);

    if (res < 0) {
        cfmanager_log_message( L_ERR,
            "recvfrom error: %s\n", strerror(errno) );
        return;
    }

    if (res < sizeof(ipc_msg_t)) {
        cfmanager_log_message( L_ERR, "recvfrom res(%d) < ipc hdr len(%d)\n",
            res, (uint32_t)sizeof(ipc_msg_t));
        return;
    }

    ipc_msg_t *msg = (ipc_msg_t *)gs_ipc_srv_cm.recv_buf;
    ipc_msg_hdr_ntoh(msg);

    cfmanager_log_message( L_DEBUG, "recv msg_type=%d, msg_len=%d, total_len=%d\n",
        msg->msg_type, msg->msg_len, res );

    if ( res < (sizeof(ipc_msg_t) + msg->msg_len) ) {
        cfmanager_log_message( L_ERR, "recvfrom res(%d) < ipc total len(%d)",
            res, (uint32_t)(sizeof(ipc_msg_t) + msg->msg_len) );
        return;
    }

    cfmanager_ipc_proc_msg( msg->msg_type, msg->msg_len, msg->data );

}

#if 0
//=============================================================================
void
cfmanager_ipc_test(
    struct uloop_timeout *timeout
)
//=============================================================================

{
    static struct uloop_timeout retry = {
        .cb = cfmanager_ipc_test,
    };
    int t = 10;

    static struct blob_buf tx_msg;
    static int count = 1;
    struct blob_attr *msg = NULL;
    json_object *content = NULL;

    blob_buf_init( &tx_msg, 0 );

    content = json_object_new_object();
    json_object_object_add( content, "test", json_object_new_string( "cm2sg test" ) );

    blobmsg_add_u32( &tx_msg, "count", count++ );
    blobmsg_add_json_element( &tx_msg, "context", content );

    msg = tx_msg.head;
    cfmanager_ipc_tx_msg( ( void * )msg, blob_raw_len( msg ) );

    uloop_timeout_set( &retry, t * 1000 );
}
#endif

//=============================================================================
int
cfmanager_ipc_init(
    void
)
//=============================================================================
{
    memset( &gs_ipc_srv_cm, 0, sizeof( gs_ipc_srv_t ) );
    gs_ipc_srv_cm.cliaddr_len = sizeof( gs_ipc_srv_cm.cliaddr );

    memset( &uloop_fd, 0, sizeof( struct uloop_fd ) );
    uloop_fd.cb = cfmanager_ipc_recv_cb;
    uloop_fd.fd = create_socket();
    if ( uloop_fd.fd < 0 ) {
        cfmanager_log_message( L_ERR,
            "socket(PF_UNIX): %s", strerror(errno) );
        goto fail;
    }

    if ( gs_ipc_srv_init( uloop_fd.fd, MODULE_CM ) < 0) {
        goto fail;
    }

    uloop_fd_add( &uloop_fd, ULOOP_READ );

#if 0
    cfmanager_ipc_test( NULL );
#endif
    return 0;

fail:

    if (uloop_fd.fd >= 0) {
        close(uloop_fd.fd);
    }

    return -1;
}

//=============================================================================
int
cfmanager_ipc_tx_msg(
    void *buf,
    int len,
    bool async
)
//=============================================================================
{
    ipc_msg_t *msg = NULL;
    int total_len = 0;
    int ret = 0;
    char *p = NULL;

    if ( ( NULL == buf ) || ( 0 == len ) ) {
        cfmanager_log_message( L_ERR, "null msg" );
        return -1;
    }

    total_len = sizeof( ipc_msg_t ) + len;
    msg = ( ipc_msg_t * )malloc( total_len );
    if ( NULL == msg ) {
        cfmanager_log_message( L_ERR, "malloc memory failed" );
        return -1;
    }

    /* fill msg */
    memset( msg, 0x0, total_len );
    if( async ) {
        msg->msg_type = htonl( IPC_MSG_CM_ASYNC );
    }
    else {
        msg->msg_type = htonl( IPC_MSG_CM_SYNC );
    }
    msg->msg_len = htonl(len);
    p = ( char * )msg->data;
    memcpy( p, buf, len );

    if( async ) {
        if ( gs_msg_cm2sg_async( msg, total_len ) < 0 ) {
            cfmanager_log_message( L_ERR, "send msg to sg failed" );
            ret = -1;
        }
    }
    else {
        if ( gs_msg_cm2sg_sync( msg, total_len, cfmanager_ipc_sync_msg_cb ) < 0 ) {
            cfmanager_log_message( L_ERR, "recv sync msg from sg failed" );
            ret = -1;
        }
    }

    free(msg);

    return ret;
}

