#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#define DST_IP "192.168.40.139"
#define DST_PORT 9999
#define BIND_PORT 57889
#define BIND_IP "192.168.40.139"
// udp connect 连接到的端口不存在会发送icmp错误报文，连接到错误的ip无法回复。
// 如果我们不进行 connect 操作，建立（UDP 套接字——目的地址 + 端口）之间的映射关系，
//操作系统内核就没有办法把 ICMP 不可达的信息和 UDP 套接字进行关联，也就没有办法将 ICMP 信息通知给应用程序。
int main()
{
    int fd, ret, n;
    struct sockaddr_in dst_addr, src_addr;
    char buf[1024] = {0};
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( fd < 0 ) {
        printf("socket err\n");
    }
    bzero(&src_addr, sizeof(src_addr));
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(BIND_PORT);
    src_addr.sin_addr.s_addr = inet_addr(BIND_IP);
    ret = bind( fd, (void *)&src_addr, sizeof(src_addr) );
    if ( ret < 0 ) {
        printf("clie bind failed\n");
    }

    bzero(&dst_addr, sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(9999);
    dst_addr.sin_addr.s_addr = inet_addr(DST_IP);

    ret = connect(fd, (void*)&dst_addr, sizeof(dst_addr));
    if ( ret < 0 ) {
        printf("connect failed\n");
    }
    while (1) {
        if ( fgets(buf, sizeof(buf), stdin) == NULL ) {
            break;
        }
        write(fd, buf, sizeof(buf));
        // 如果端口不可达，返回错误
        n = read(fd, buf, sizeof(buf));
        if (  n < 0 ) {
            printf("%s\n", strerror(errno));
        }
    }
}
