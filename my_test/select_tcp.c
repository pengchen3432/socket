#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
int main()
{
    int ser_fd, ret, nread, n, max_fd, cli_fd, i, max_i, sockfd;
    const int one = 1;
    struct sockaddr_in in;
    fd_set all, reset;
    int client[FD_SETSIZE];
    char buf[5];
    memset(buf, 0x00, sizeof(buf));
    ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_fd < 0) {
        printf("socket err\n");
        return -1;
    }

    ret = setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if ( ret < 0 ) {
        printf("set opt err\n");
        return -1;
    }
    
    bzero(&in, 0);
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.2.11");

    bind(ser_fd, (void *)&in, sizeof(in));
    listen(ser_fd, 64);

    FD_ZERO(&all);
    FD_SET(ser_fd, &all);
    max_fd = ser_fd;
    max_i = -1;
    for (int i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }
    while (1) {
        reset = all;
        nread = select(max_fd + 1, &reset, NULL, NULL, NULL);
        if (FD_ISSET(ser_fd, &reset)) {
            cli_fd = accept(ser_fd, NULL, NULL);
            if (cli_fd > max_fd) {
                max_fd = cli_fd;
            }
            printf("%d\n", cli_fd);
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] == -1) {
                    client[i] = cli_fd;
                    break;
                }
            }
            if (i == FD_SETSIZE) {
                printf("fp is too smail\n");
                return -1;
            }
            FD_SET(cli_fd, &all);
            if (i > max_i) {
                max_i = i;
            }
            if (--nread <= 0) {
                continue;
            }
        }

        for (i = 0; i <= max_i; i++) {
            if (client[i] < 0) {
                continue;
            }
            sockfd = client[i];
            if (FD_ISSET(sockfd, &reset)) {
                n = read(sockfd, buf, sizeof(buf));
                printf("n = %d\n", n);
                if (n > 0) {
                    printf("buf = %s\n", buf);
                    memset(buf, 0x00, n);
                    write(sockfd, buf, sizeof(buf));
                }
                else {
                    printf("client exit\n");
                    close(sockfd);
                    FD_CLR(sockfd, &all);
                    client[i] = -1;
                }
                if (--nread <= 0) {
                    break;
                }
            }
        }
       
    }
}