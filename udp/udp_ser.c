#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
int main()
{
    int ser_fd, n;
    char buf[1024] = {0};
    struct sockaddr_in in, cli_in;
    socklen_t len;
    struct in_addr in_a;
    len = sizeof(cli_in);
    ser_fd = socket(AF_INET, SOCK_DGRAM, 0 );
    if ( ser_fd < 0 ) {
        printf("fd err\n");
        return -1;
    }
    memset( &in, 0, sizeof(in) );
    in.sin_port = htons(9999);
    in.sin_family = AF_INET;
    in.sin_addr.s_addr = INADDR_ANY;
    bind(ser_fd, (void *)&in, sizeof(in));
    while (1) {
        memset(buf, 0, sizeof(buf));
        n = recvfrom(ser_fd, buf, sizeof(buf), 0, (void *)&cli_in, &len);
        //in_a = cli_in.sin_addr.s_addr;
        printf("ip:%s\n",inet_ntoa(cli_in.sin_addr));
        printf("%d:", ntohs(cli_in.sin_port));
        printf("buf=%s\n", buf);
    }
    
}