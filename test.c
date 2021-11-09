#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
int main()
{
    int serv_fd;
    int cli_fd;
    int client[FD_SETSIZE];
    const int one = 1;
    int maxfd, maxi;
    int sockfd;
    fd_set all, rset;
    struct sockaddr_in servaddr;
    int i, n, bytes;
    char buf[1024];

    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( serv_fd < 0 ) {
        printf("%s\n", strerror(errno));
    } 

    setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999);
    servaddr.sin_addr.s_addr = inet_addr("192.168.58.130");

    bind(serv_fd, (void *)&servaddr, sizeof(servaddr));
    listen(serv_fd, 64);

    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }

    maxfd = serv_fd;
    maxi = -1;
    FD_ZERO(&all);
    FD_SET(serv_fd, &all);

    for (;;)
    {
        rset = all;
        n = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if ( FD_ISSET(serv_fd, &rset) ) {
            cli_fd = accept(serv_fd, NULL, NULL);
            printf("cli_fd:%d\n", cli_fd);
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
            }
            FD_SET(cli_fd, &all);
            if (cli_fd > maxfd)
            {
                maxfd =cli_fd;
            }
            if (i > maxi) {
                maxi = i;
            }

            if (--n <= 0) {
                continue;
            }
        }
        printf("cococ\n");
        for (i = 0; i <= maxi; i++)
        {
            if ((sockfd = client[i]) < 0 )
            {
                continue;
            }
            if ( FD_ISSET(sockfd, &rset) ) 
            {
                bytes = read(sockfd, buf, sizeof(buf));
                printf("bytes:%d\n", bytes);
                if ( bytes <= 0 ) 
                {
                    close(sockfd);
                    FD_CLR(sockfd, &all);
                    client[i] = -1;
                }
                else 
                {
                    write(sockfd, buf, bytes);
                }
                if (--n <= 0)
                {
                    break;
                }
            }
            
        }
    }


}