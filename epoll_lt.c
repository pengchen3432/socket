#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
int main()
{
    int ser_fd;
    int epoll_fd;
    const int one = 1;
    int nread, i, n, sockfd, cli_fd;
    struct sockaddr_in in;
    struct sockaddr_in client_in;
    struct epoll_event events[1024];
    struct epoll_event ev;
    char buf[5];
    socklen_t len = sizeof(client_in);
    epoll_fd = epoll_create(1024);

    ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_fd < 0)
    {
        printf("%s\n", strerror(errno));
    }
    setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.2.11");

    bind(ser_fd, (void *)&in, sizeof(in));
    listen(ser_fd, 64);

    ev.data.fd = ser_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ser_fd, &ev);

    for (;;)
    {
        nread = epoll_wait(epoll_fd, events, 1024, -1);
        if (nread < 0)
        {
            printf("%s\n", strerror(errno));
        }
        for (i = 0; i < nread; i++)
        {
            sockfd = events[i].data.fd;
            if (sockfd == ser_fd)
            {
                cli_fd = accept(sockfd, (void *)&client_in, &len);
                ev.data.fd = cli_fd;
                ev.events = EPOLLIN;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cli_fd, &ev);
                printf("客户端来连接了\n");
                printf("addr:%s\n", inet_ntoa(client_in.sin_addr));
                printf("port:%d\n", ntohs(client_in.sin_port));
                printf("文件描述符:%d\n", cli_fd);
            }
            else 
            {
                memset(buf, 0x00, sizeof(buf));
                n = read(sockfd, buf, sizeof(buf));
                if (n > 0)
                {
                    printf("cli_fd %d:%s\n", sockfd, buf);
                }
                else 
                {
                    if (n == 0)
                    {
                        printf("客户端正常关闭\n");
                    }
                    ev = events[i];
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, &ev);
                    close(sockfd);
                }
            }
        }
    }

    close(ser_fd);
    close(epoll_fd);
}