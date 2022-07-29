#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
int main()
{
    int cli_fd, n, ret;
    int opt = 1;
    char buf[1024] = {0};
    struct sockaddr_in in, cli_in;
    socklen_t len = sizeof(in);
    cli_fd = socket(AF_INET, SOCK_DGRAM, 0 );
    if ( cli_fd < 0 ) {
        printf("fd err\n");
        return -1;
    }
    setsockopt( cli_fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt) );
    memset( &in, 0, sizeof(in) );
    in.sin_port = htons(9999);
    in.sin_family = AF_INET;
    in.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    memset( &cli_in, 0, sizeof(cli_in) );
    cli_in.sin_addr.s_addr = inet_addr("192.168.80.139");
    cli_in.sin_port = htons(7777);
    cli_in.sin_family = AF_INET;
    ret = bind(cli_fd, (void *)&cli_in, len);
    if ( ret < 0 ) {
        printf("bind err\n");
    }
    while (fgets(buf, sizeof(buf), stdin) != 0) {
        sendto(cli_fd, buf, strlen(buf), 0, (void *)&in, len);
    }
    
}