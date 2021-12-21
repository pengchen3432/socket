#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
int main()
{
    int ser_fd, ret, cli_fd, n;
    char buf[1024];
    const int one = 1;
    struct sockaddr_in in, client;
    socklen_t len = sizeof(client);
    ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_fd < 0) {
        printf("socket err\n");
        return -1;
    }

    ret = setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if ( ret < 0 ) {
        printf("set option err\n");
        return -1;
    }

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.2.11");

    ret = bind(ser_fd, (void *)&in, sizeof(in));
    if (ret < 0) {
        printf("bind err\n");
        return -1;
    }

    ret = listen(ser_fd, 64);
    if ( ret < 0 ) {
        printf("listen err\n");
        return -1;
    }

    cli_fd = accept(ser_fd, (void *)&client, &len);
    if ( cli_fd < 0 ) {
        printf("accept packet err\n");
        return -1;
    }
    
    struct in_addr addr;
    addr.s_addr = client.sin_addr.s_addr;
    printf("address %s\n", inet_ntoa(addr));
    printf("prot = %d\n", htons(client.sin_port));
    while (1) {
        n = read(cli_fd, buf, sizeof(buf));
        if (n > 0) {
            printf("buf = %s\n", buf);
            memset(buf, 0x00, sizeof(buf));
        }
        else if (n == 0) {
            printf("client exit\n");
            break;
        }
        else {
            printf("exit err\n");
            break;
        }
    }
    close(ser_fd);
    close(cli_fd);
    return 0;
}