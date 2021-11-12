#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    if (fork() > 0)
    {
        exit(0);
    }
    setsid();

    int ser_fd, cli_fd;
    const int one = 1;
    struct sockaddr_in in;
    int i, nread, n;
    struct sockaddr_in cli_in;
    fd_set all, rset;
    int maxfd, maxi;
    int sockfd;
    char buf[1024];
    int client[FD_SETSIZE];
    socklen_t len = sizeof(cli_in);

    ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_fd < 0)
    {
        printf("%s\n", strerror(errno));
    }
    setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.2.11");

    bind(ser_fd, (void *)&in, sizeof(in));

    listen(ser_fd, 64);

    for (i = 0; i < FD_SETSIZE; i++)
    {
        client[i] = -1;
    }

    FD_ZERO(&all);
    FD_SET(ser_fd, &all);
    maxfd = ser_fd;
    maxi = -1;
    for (;;)
    {
        rset = all;
        nread = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(ser_fd, &rset))
        {
            cli_fd = accept(ser_fd, (void *)&cli_in, &len);
            printf("cli = %d\n", cli_fd);
            printf("cliaddr = %s\n", inet_ntoa(cli_in.sin_addr));
            printf("cliprot = %d\n", ntohs(cli_in.sin_port));

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
                printf("to many\n");
                return -1;
            }
            FD_SET(cli_fd, &all);
            if (i > maxi)
            {
                maxi = i;
            }
            if (cli_fd > maxfd)
            {
                maxfd = cli_fd;
            }
            if (--nread <= 0)
            {
                continue;
            }
        }
        for (i = 0; i < FD_SETSIZE; i++)
        {
            if ((sockfd = client[i]) < 0)
            {
                continue;
            }
            if (FD_ISSET(sockfd, &rset))
            {
                memset(buf, 0x00, sizeof(buf));
                n = read(sockfd, buf, sizeof(buf));
                if (n <= 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &all);
                    client[i] = -1;
                    break;
                }
                else
                {
                    printf("client %d:%s", sockfd, buf);
                }

                if (--nread <= 0)
                {
                    break;
                }
            }
        }
    }
}