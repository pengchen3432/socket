#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/io.h>
#define BUFF_LEN 1024
int main()
{
    int fd;
    struct sockaddr_in src_addr, dst_addr;
    struct iovec iov;
    struct msghdr msg;
    int state;
    char buf[BUFF_LEN] = {0};
    fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( fd < 0 ) {
        printf( "socket failed\n" );
        return 0;
    }

    bzero(&dst_addr, sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(9999);
    dst_addr.sin_addr.s_addr = inet_addr("192.168.80.139");
    iov.iov_base = buf;
    iov.iov_len = BUFF_LEN;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &dst_addr;
    msg.msg_namelen = sizeof(dst_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;

    while ( 1 ) {
        memset(buf, 0, BUFF_LEN);
        if ( fgets(buf, sizeof(buf), stdin) == NULL ) {
            printf("fegt err\n");
            return -1;
        }
        state = sendmsg(fd, &msg, 0);
        if ( state < 0 ) {
            return -1;
        }
        state = recvmsg(fd, &msg, 0);
        printf("recv:%s\n", buf);
    }
    return 0;
}