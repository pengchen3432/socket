#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#define DST_IP "192.168.80.140"
#define DST_PORT 9999
#define BIND_PORT 57888
#define BIND_IP "192.168.80.140"

int main()
{
    int fd, ret;
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
    }
}
