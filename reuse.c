#include  <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
int main()
{
    int fd, ret;
    struct sockaddr_in sin;
    const int one = 1;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( fd < 0 ) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));  //绑定指点先setsockopt
    if ( ret < 0 ) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = 9999;
    sin.sin_addr.s_addr = inet_addr("192.168.2.11");
    ret = bind(fd, (struct sockaddr*)&sin, sizeof(sin));
    if ( ret < 0 ) {
        printf("%s\n", strerror(errno));
        return -1;
    }
  
    for (;;) {
        
    }
}
