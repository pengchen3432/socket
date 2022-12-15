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

int creat_socket()
{
    int cli_fd, ret;
    struct sockaddr_un un;
    const int opt = 1;
    int recv_buff = 0;
    int send_buff = 0;
    int optlen = sizeof(int);
    cli_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( cli_fd < 0 ) {
        perror("sock");
    }

    ret = setsockopt(cli_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if ( ret < 0 ) {
        perror("setsocketopt");
    }

    ret = getsockopt(cli_fd, SOL_SOCKET, SO_RCVBUF, &recv_buff, &optlen);
    if ( ret < 0 ) {
        perror("setsocketopt");
    }
    printf("recv_buf = %d\n", recv_buff);

    ret = getsockopt(cli_fd, SOL_SOCKET, SO_SNDBUF, &send_buff, &optlen);
    if ( ret < 0 ) {
        perror("setsocketopt");
    }
    printf("send_buf = %d\n", send_buff);

    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, LOCAL_UNIX);

    ret = connect(cli_fd, (void *)&un, sizeof(un));
    if ( ret < 0 ) {
        perror("connect");
    }

    return cli_fd;
}
int main()
{
    int cli_fd, ret;
    int total_len;
    struct iovec iov = {0};
    struct ipc_msg *ipc_msg = NULL;
    char *p = NULL;
    cli_fd = creat_socket();
    if ( cli_fd < 0 ) {
        return -1;
    }
    
    char buf[222992] = {0};
    memset(buf, '1', sizeof(buf));
    buf[222990] = '2';
    buf[222991] = '3';
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