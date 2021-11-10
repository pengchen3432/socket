#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
int main()
{
    int ser_fd;
    char buf[1024];
    struct sockaddr_in in;
    const int one = 1;
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    struct msghdr msg;
    struct iovec iov;
    int n;
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = &client;
    ser_fd = socket(AF_INET, SOCK_DGRAM, 0);
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

    while (1)
    {
        printf("hello\n");

        n = recvmsg(ser_fd, &msg, 0);

        // n = recvfrom(ser_fd, buf, sizeof(buf), 0, (void *)&client, &len);
        if (n > 0)
        {
            printf("buf = %s\n", buf);
            printf("clientaddr %s\n", inet_ntoa(client.sin_addr));
            printf("clientport %d\n", ntohs(client.sin_port));
        }
    }
}
