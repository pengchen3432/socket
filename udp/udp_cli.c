#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
int main()
{
    int ser_fd, n;
    int opt = 1;
    char buf[1024] = {0};
    struct sockaddr_in in;
    socklen_t len = sizeof(in);
    ser_fd = socket(AF_INET, SOCK_DGRAM, 0 );
    if ( ser_fd < 0 ) {
        printf("fd err\n");
        return -1;
    }
    setsockopt( ser_fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt) );
    memset( &in, 0, sizeof(in) );
    in.sin_port = htons(9999);
    in.sin_family = AF_INET;
    in.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    while (fgets(buf, sizeof(buf), stdin) != 0) {
        sendto(ser_fd, buf, strlen(buf), 0, (void *)&in, len);
    }
    
}