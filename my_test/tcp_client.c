#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
int main()
{
    int ser_fd, ret, cli_fd, n;
    struct sockaddr_in in;
    char buf[1024];
    memset(buf, 0x00, sizeof(buf));
    
    cli_fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( cli_fd < 0 ) {
        printf("socket err\n");
        return -1;
    }

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.2.11");

    ret = connect(cli_fd, (void *)&in, sizeof(in));

    while (1) {
        fgets(buf, sizeof(buf), stdin);
        n = write(cli_fd, buf, sizeof(buf));
        memset(buf, 0x00, n);
        if ( n < 0 ) {
            printf("write err\n");
        }
    }
}