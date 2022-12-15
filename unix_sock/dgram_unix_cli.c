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
#define LOCAL_UNIX "./dgram.sock"
struct ipc_msg {
    int len;
    char data[0];
};
int socket_creat(){
    int cli_fd, ret;
    struct sockaddr_un un = {0};
    cli_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if ( cli_fd < 0 ) {
        printf("socket creat er\n");
        return -1;
    }

    un.sun_family = AF_UNIX;
    strncpy(un.sun_path, LOCAL_UNIX, sizeof(un.sun_path));
    ret = connect( cli_fd, (void *)&un, sizeof(un) );
    if ( ret < 0 ) {
        perror("connect");
        return -1;
    }
    return cli_fd;
}

int send_msg(int fd, char *msg, int total)
{
    struct msghdr msghdr = {0};
    struct iovec iov = {0};
    iov.iov_base = msg;
    iov.iov_len = total;
    msghdr.msg_iov = &iov;
    msghdr.msg_iovlen = 1;

    int len = 0;
    do {
        int cur_len = 0;
        cur_len = sendmsg(fd, &msghdr, 0);
        printf("cur = %d\n", cur_len);
        if ( cur_len < 0 ) {
            perror("err");
            return -1;
        }

        len += cur_len;
        iov.iov_base += cur_len;
        iov.iov_len -= cur_len;
        msghdr.msg_iov = &iov;
        msghdr.msg_iovlen = 1;
    } while (len < total);
    return len;
}
#define SIZE 65535
int main()
{
    int cli_fd, ret;
    struct ipc_msg *ipc_msg = NULL;
    int total_len = 0;
    cli_fd = socket_creat();
    if ( cli_fd < 0 ) {
        printf("creat err\n");
    }

    char buf[SIZE] = {0};
    memset(buf, '1', sizeof(buf));
    buf[SIZE - 1] = '2';
    buf[SIZE - 2] = '3';
    total_len = sizeof(struct ipc_msg) + sizeof(buf);
    ipc_msg = (struct ipc_msg *)calloc(1, total_len);
    if ( !ipc_msg ) {
        printf("msg malloc err\n");
        return -1;
    }
    ipc_msg->len = sizeof(buf);
    memcpy(ipc_msg->data, buf, sizeof(buf));
    ret = send_msg(cli_fd, (void *)ipc_msg, total_len);
}