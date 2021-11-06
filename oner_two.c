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
    struct sockaddr_in sin;
    fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( fd < 0 ) {
        printf( "socket err %s\n", strerror( errno ) );
    }

    bzero( &sin, sizeof(sin) );
    sin.sin_family = AF_INET;
    sin.sin_port = htons( 9999 );
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");

    
    if ( connect(fd, (void *)&sin, sizeof(sin)) < 0 ) {
        printf("%s\n", strerror( errno ));
    }

    while ( ( n = read(fd, buf, PACK_SIZE) ) > 0 ) {
        buf[PACK_SIZE] = 0;
        if( fputs(buf, stdout) == EOF ) {
            printf("err\n");
        }
    }
    printf( "return\n" );
    
}