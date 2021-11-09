#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
static char addr[] = "192.168.2.11";
int main()
{
    int fd;
    const int one = 1;
    struct sockaddr_in servaddr;
    int client[FD_SETSIZE];
    int max_fd, max_i, i, cli_fd;
    int sockfd;
    int n;
    fd_set all, rset;
    char buf[] = "hello!\n";

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("%s\n", strerror(errno));
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999);
    servaddr.sin_addr.s_addr = inet_addr(addr);

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    bind(fd, (void *)&servaddr, sizeof(servaddr));
    listen(fd, 64);

    max_fd = fd;
    max_i = -1;
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;

    FD_ZERO(&all);
    FD_SET(fd, &all);

    for (;;)
    {
        rset = all;
        n = select(max_fd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(fd, &rset))
        {
            cli_fd = accept(fd, NULL, NULL);
            printf("fd = %d\n", cli_fd);
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
                printf("fd too many\n");
                break;
            }

            FD_SET(cli_fd, &all);
            if (cli_fd > max_fd)
            {
                max_fd = cli_fd;
            }
            if (i > max_i)
            {
                max_i = i;
            }
            if (--n <= 0)
            {
                continue;
            }
        }

        for (i = 0; i <= max_i; i++)
        {
            if ((sockfd = client[i]) < 0)
            {
                continue;
            }
            if (FD_ISSET(sockfd, &rset))
            {
                if ((n = read(sockfd, buf, sizeof(buf))) == 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &all);
                    client[i] = -1;
                }
                else
                {
                    write(sockfd, buf, n);
                }
                if (--n <= 0)
                {
                    break;
                }
            }
        }
    }
}