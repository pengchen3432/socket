#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#define PACK_SIZE 65535
int main(int argc, char **argv)
{
    int fd;
    int ret;
    int n;
    char buf[PACK_SIZE];
    char str[1024];
    struct sockaddr_in sin;
    fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( fd < 0 ) {
        printf( "socket err %s\n", strerror( errno ) );
    }

    bzero( &sin, sizeof(sin) );
    sin.sin_family = AF_INET;
    sin.sin_port = htons( 9999 );
    sin.sin_addr.s_addr = inet_addr("192.168.58.99990");

    
    if ( connect(fd, (void *)&sin, sizeof(sin)) < 0 ) {
        printf("%s\n", strerror( errno ));
    }

    while ( 1 ) {
        memset(buf, 0x00, sizeof(buf));
        scanf( "%s", str );
        write( fd, str, strlen(str) );
        int n = read( fd, buf, sizeof(buf) );
        fputs( buf, stdout );
    }
    printf( "return\n" );
    
}
