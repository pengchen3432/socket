#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
int main()
{
    int cli_fd;
    struct sockaddr_in ser_in, cli_in;
    socklen_t len = sizeof(ser_in);
    char buf[1024] = {0};
    char recvline[1024] = {0};
    int n, ret;
    cli_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (cli_fd < 0)
    {
        printf("%s\n", strerror(errno));
    }

    ser_in.sin_family = AF_INET;
    ser_in.sin_port = htons(9999);
    ser_in.sin_addr.s_addr = inet_addr("192.168.2.11");

    cli_in.sin_family = AF_INET;
    cli_in.sin_port = htons(9898);
    cli_in.sin_addr.s_addr = inet_addr("192.168.2.11");
    ret = bind(cli_fd, (void *)&cli_fd, sizeof(cli_fd));
    if (ret < 0)
    {
        printf("bind err\n");
    }

    while ((fgets(buf, sizeof(buf), stdin)) != NULL)
    {
        n = sendto(cli_fd, buf, strlen(buf), 0, (void *)&ser_in, sizeof(ser_in));
        n = recvfrom(cli_fd, recvline, sizeof(recvline), 0, (void *)&ser_in, &len);
        fputs(recvline, stdout);
    }
}