#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
int main()
{
    int ser_fd, cli_fd;
    const int one = 1;
    int ret, i;
    int maxfd, maxi;
    struct sockaddr_in in;
    char buf[1024];
    fd_set all, rset;
    int n, nready, sockfd;
    int client[FD_SETSIZE];
    ser_fd = socket(AF_INET, SOL_SOCKET, 0);
    if (ser_fd < 0)
    {
        printf("%s\n", strerror(errno));
    }

    ret = setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (ret < 0)
    {
        printf("%s\n", strerror(errno));
    }

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.2.11");

    bind(ser_fd, (void *)&in, sizeof(in));
    listen(ser_fd, 64);

    maxfd = ser_fd;
    maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++)
    {
        client[i] = -1;
    }
    FD_ZERO(&all);
    FD_SET(ser_fd, &all);

    for (;;)
    {
        rset = all;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        printf("nready = %d\n", nready);
        if (FD_ISSET(ser_fd, &rset))
        {
            cli_fd = accept(ser_fd, NULL, NULL);
            printf("cli_fd: %d\n", cli_fd);
            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0)
                {
                    client[i] = cli_fd;
                    break;
                }
            }
            if (i == FD_SETSIZE)
            {
                printf("fd to many\n");
                return -1;
            }

            FD_SET(cli_fd, &all);

            if (cli_fd > maxfd)
            {
                maxfd = cli_fd;
            }

            if (i > maxi)
            {
                maxi = i;
            }

            if (--nready <= 0)
            {
                continue;
            }
        }

        for (i = 0; i <= maxi; i++)
        {
            if ((sockfd = client[i]) < 0)
            {
                continue;
            }

            if (FD_ISSET(sockfd, &rset))
            {
                n = read(sockfd, buf, sizeof(buf));
                printf("n = %d\n", n);
                if (n <= 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &all);
                    client[i] = -1;
                }
                else
                {
                    write(sockfd, "hello!\n", strlen("hello!\n"));
                }
                if (--nready <= 0)
                {
                    break;
                }
            }
        }
    }
}