#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
int main()
{
    int fd, cli_fd;
    struct sockaddr_in servaddr;
    int flag = 0;
    int n;
    const int one = 1;
    char buf[1024];
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("%s\n", strerror(errno));
    }
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999);
    servaddr.sin_addr.s_addr = inet_addr("192.168.2.11");

    bind(fd, (void *)&servaddr, sizeof(servaddr));
    listen(fd, 64);

    cli_fd = accept(fd, NULL, NULL);

    flag = fcntl(cli_fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(cli_fd, F_SETFL, flag);
    while (1)
    {
        memset(buf, 0x00, sizeof(buf));
        n = read(cli_fd, buf, sizeof(buf));
        if (n > 0)
        {
            printf("buf: %s", buf);
        }
        printf("hello\n");
        sleep(1);
    }
    
}