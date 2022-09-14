#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#define DST_IP "192.168.40.90"
#define DST_PORT 8888
#define BIND_PORT 57888
#define BIND_IP "192.168.40.139"
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
    dst_addr.sin_port = htons(DST_PORT);
    dst_addr.sin_addr.s_addr = inet_addr(DST_IP);

    while (1) {
        if ( fgets(buf, sizeof(buf), stdin) == NULL ) {
            break;
        }
        n = sendto(fd, buf, sizeof(buf), 0, (void *)&dst_addr, sizeof(dst_addr));
        n = recvfrom(fd, buf, sizeof(buf), 0, NULL, 0);
        if (  n < 0 ) {
            printf("%s\n", strerror(errno));
        }
    }
}
