#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
int main()
{
    int fd, ret, len;
    struct sockaddr_in in, cli_addr;
    char buf[1024] = {0};
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( fd < 0 ) {
        printf("socket err\n");
    }
    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.40.139");
    len = sizeof(cli_addr);
    bind(fd, (void *)&in, sizeof(in));
    while (1) {
        recvfrom(fd, buf, sizeof(buf), 0, (void *)&cli_addr, &len);
        printf("cli_addr:ip:%s\n", inet_ntoa(cli_addr.sin_addr));
        printf("cli_addr:port:%d\n", ntohs(cli_addr.sin_port));
        printf("buf:%s\n", buf);
    }
}
