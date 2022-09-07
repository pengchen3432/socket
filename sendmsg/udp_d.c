#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/io.h>
#include <errno.h>
#define BUFF_LEN 1024
int main()
{
    int fd, ret;
    struct sockaddr_in src_addr;
    struct sockaddr_in *send_addr;
    struct iovec iov;
    struct msghdr msg;
    int state;
    char buf[BUFF_LEN] = {0};
    fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( fd < 0 ) {
        printf( "socket failed\n" );
        return 0;
    }

    bzero(&src_addr, sizeof(src_addr));
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(9999);
    src_addr.sin_addr.s_addr = inet_addr("192.168.80.139");
    ret =  bind( fd, (void *)&src_addr, sizeof(src_addr) );
    if ( ret < 0 ) {
        printf("bind err\n");
        return -1;
    }
    iov.iov_base = buf;
    iov.iov_len = BUFF_LEN;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = send_addr;
    msg.msg_namelen = sizeof(send_addr);

    while (1) {
        state = recvmsg( fd, &msg, 0 );
        send_addr = msg.msg_name;
        printf("send_addr:%s\n", inet_ntoa(send_addr->sin_addr));
        printf("send_port:%d\n", ntohs(send_addr->sin_port));
        if ( state < 0 ) {
            printf("err:%s\n", strerror(errno));
            continue;
        }
        printf("buf:%s\n", buf);
    
        sendmsg(fd, &msg, 0);
    }
    return 0;
}