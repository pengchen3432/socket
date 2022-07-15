#include <stdio.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <netinet/in.h>
#define path "/tmp/local.sock"
static int idx = 3;
static struct epoll_event events[64] = {0};
struct obj{
    int fd;
    char msg[64];
};
struct obj creat_obj(int fd) {
    struct obj o;
    o.fd = fd;
    sprintf( o.msg, "my index id %d", idx++ );
    return o;
}

int epoll_init() {
    int ep_fd;
    ep_fd = epoll_create( 64 );
    if ( ep_fd < 0 ) {
        printf("epoll init failed");
        exit(1);
    }
    return ep_fd;
}
int epoll_add_fd(int ep_fd, int fd, void *data) {
    struct epoll_event ev;
    ev.data.ptr = data;
    ev.events = EPOLLIN;
    if ( epoll_ctl( ep_fd, EPOLL_CTL_ADD, fd, &ev ) < 0) {
        printf("add fd failed\n");
        exit(1);
    }
    return 1;
}

int local_sock_init() {
    int lo_fd;

    lo_fd = socket( AF_LOCAL, SOCK_STREAM, 0 );
    if ( lo_fd < 0 ) {
        printf("local init failed\n");
        exit(1);
    }
    return lo_fd;
}
void local_bind_sock( int fd, int family, const char *file ) {
    struct sockaddr_un un;
    int ret = -1;

    un.sun_family = family;
    unlink( file );
    strcpy( un.sun_path, file );
    ret = bind( fd, (void *)&un, sizeof(un) );
    if ( ret < 0 ) {
        printf( "bind failed\n" );
        exit(0);
    }
}

void local_listen_sock( int fd, int n ) {
    listen( fd, 64 );
}

void run( int ep_fd, int local_fd ) {
    int nfds, n, i, cli_fd, res_fd;
    struct obj *obj;
    struct obj cli_obj;
    char buf[1024];
    for ( ;; ) {
        nfds = epoll_wait( ep_fd, events, 64, -1 );
        if ( nfds < 0 ) {
            printf("epoll_wait err\n");
            return;
        }
        for ( i = 0; i < nfds; i++ ) {
            obj = (struct obj*)events[i].data.ptr;
            res_fd = obj->fd;
            printf("message:%s\n", obj->msg);
            if ( local_fd == res_fd ) {
                cli_fd = accept( local_fd, NULL, NULL );
                cli_obj = creat_obj( cli_fd );
                epoll_add_fd( ep_fd, cli_fd, (void *)&cli_obj );
            }
            else {
                memset( buf, 0, sizeof( buf ) );
                n = read( res_fd, buf, sizeof( buf ) );
                if ( n > 0 ) {
                    printf("buf:%s\n", buf);
                }
                else {
                    printf("cli exit\n");
                    close( res_fd );
                    epoll_ctl( ep_fd, EPOLL_CTL_DEL, res_fd, &events[i] );
                    idx--;
                }
            }
        }
    }
}

int main()
{
    int ep_fd, local_fd;
    struct obj o;

    local_fd = local_sock_init();
    local_bind_sock( local_fd, AF_LOCAL, path );
    local_listen_sock( local_fd, 64 );

    ep_fd = epoll_init();
    o = creat_obj( local_fd );
    epoll_add_fd( ep_fd, local_fd, (void *)&o );

    run( ep_fd, local_fd );

    close(ep_fd);
    close(local_fd);
}