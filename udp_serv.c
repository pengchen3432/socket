#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
int main()
{
    int ser_fd;
    const int one = 1;
    struct sockaddr_in in;
    struct sockaddr_in cli_in;
    socklen_t len = sizeof(cli_in);
    int n;
    char buf[1024];
    ser_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ser_fd < 0)
    {
        printf("%s\n", strerror(errno));
    }

    setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.80.139");
    bind(ser_fd, (void *)&in, sizeof(in));

    while (1)
    {
        memset(buf, 0x00, sizeof(buf));
        n = recvfrom(ser_fd, buf, sizeof(buf), 0, (void *)&cli_in, &len);
        if (n > 0)
        {
            printf("clientaddr:%s\n", inet_ntoa(cli_in.sin_addr));
            printf("clientport:%d\n", ntohs(cli_in.sin_port));
            printf("message:%s\n", buf);
            sendto(ser_fd, "hello\n", strlen("hello\n"), 0, (void *)&cli_in, sizeof(cli_in));
        }
    }
}
