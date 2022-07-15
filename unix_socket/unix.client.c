#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <sys/un.h>
#include<errno.h>
#include<stddef.h>
#include<unistd.h>
#include <netinet/in.h>
#define path "/tmp/local.sock"
int main()
{
    int fd, clifd;
    struct sockaddr_un sun;
    char buf[1024] = {0};
    fd = socket( AF_UNIX, SOCK_STREAM, 0 );
    if ( fd < 0 ) {
        printf("creat socket failed\n");
    }
    sun.sun_family = AF_UNIX;
    strcpy( sun.sun_path, path );
    if ( connect( fd, (struct sockaddr *)&sun, sizeof(sun) ) < 0 ) {
        printf("connect failed\n");
    }

    while (1) {
        fgets( buf, sizeof(buf), stdin );
        send( fd, buf, sizeof(buf), 0 );
    }
}