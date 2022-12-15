#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#define LOCAL_UNIX "./steam.sock"
struct ipc_msg {
    int len;
    char data[0];
};

int recv_msg(int fd, struct iovec *iov) {

    struct msghdr msghdr;
    msghdr.msg_iov = iov;
    msghdr.msg_iovlen = 1;
    int len;
    int total = 0;
    while ( iov->iov_len > 0 ) {
        len = recvmsg(fd, &msghdr, 0);
        printf("recv = %d\n", len);
        if ( len <= 0 ) {
            exit(0);
        }
        total += len;
        iov->iov_base += len;
        iov->iov_len -= len;
    }
    return total;
}

int creat_socket()
{
    int ser_fd, ret;
    struct sockaddr_un un;
    const int opt = 1;
    int recv_buff = 0;
    int send_buff = 0;
    int optlen = sizeof(int);
    ser_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( ser_fd < 0 ) {
        perror("sock");
    }

    ret = setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if ( ret < 0 ) {
        perror("setsocketopt");
    }

    ret = getsockopt(ser_fd, SOL_SOCKET, SO_RCVBUF, &recv_buff, &optlen);
    if ( ret < 0 ) {
        perror("setsocketopt");
    }
    printf("recv_buf = %d\n", recv_buff);

    ret = getsockopt(ser_fd, SOL_SOCKET, SO_SNDBUF, &send_buff, &optlen);
    if ( ret < 0 ) {
        perror("setsocketopt");
    }
    printf("send_buf = %d\n", send_buff);


    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, LOCAL_UNIX);

    ret = bind(ser_fd, (void *)&un, sizeof(un));
    if ( ret < 0 ) {
        perror("bind");
    }

    ret = listen(ser_fd, 64);

    if ( ret < 0 ) {
        perror("listen");
    }
    return ser_fd;
}
int main()
{
    unlink(LOCAL_UNIX);
    int ser_fd, cli_fd;
    struct iovec iov = {0};
    struct ipc_msg ipc_msg = {0};
    char *p = NULL;
    ser_fd = creat_socket();
    if ( ser_fd < 0 ) {
        return -1;
    }
    cli_fd = accept(ser_fd, NULL, NULL);


    iov.iov_base = (void *)&ipc_msg;
    iov.iov_len = sizeof(ipc_msg);
    recv_msg(cli_fd, &iov);
    if ( ipc_msg.len ) {
        iov.iov_len = ipc_msg.len;
        printf("len = %d\n", ipc_msg.len);
        p = calloc(1, ipc_msg.len);
        iov.iov_base = p;
        recv_msg(cli_fd, &iov);
    }

    if ( p ) {
        //printf("%s\n", p);
        free(p);
    }
}