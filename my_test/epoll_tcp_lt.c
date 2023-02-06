#include <stdio.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
int main()
{
    int ser_fd, nread, n, ret, i, sockfd, cli_fd;
    int epd;
    struct sockaddr_in in;
    const int one = 1;
    struct epoll_event ev;
    struct epoll_event event[1024];
    char buf[5] = {0};
    epd = epoll_create(64);
    ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( ser_fd < 0 ) {
        printf("socket err\n");
        return -1;
    }

    ret = setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if ( ret < 0 ) {
        printf("set opt err\n");
        return -1;
    }
    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.80.139");

   ret = bind(ser_fd, (void *)&in, sizeof(in));
       if ( ret < 0 ) {
        printf("bin err\n");
    }
    ret = listen(ser_fd, 64);
    if ( ret < 0 ) {
        printf("listen err\n");
    }

    ev.data.fd = ser_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epd, EPOLL_CTL_ADD, ser_fd, &ev);
    for (;;) 
    {
        nread = epoll_wait(epd, event, 1024, -1);
        for (i = 0; i < nread; i++) 
        {
            sockfd = event[i].data.fd;
            if (sockfd == ser_fd) 
            {
                cli_fd = accept(ser_fd, NULL, NULL);
                ev.data.fd = cli_fd;
                ev.events = EPOLLIN;
                epoll_ctl(epd, EPOLL_CTL_ADD, cli_fd, &ev);
            }
            else 
            {
                n = read(sockfd, buf, sizeof(buf));
                printf("n = %d\n", n);
                if (n > 0) 
                {
                    printf("buf=%s\n", buf);
                    memset(buf, 0x00, n);
                }
                else 
                {
                    ev = event[i];
                    printf("%d exit\n", sockfd);
                    epoll_ctl(epd, EPOLL_CTL_DEL, sockfd, &ev);
                    close(sockfd);
                }
            }
        }
    }
}