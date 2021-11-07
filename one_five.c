#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
int main()
{
    int fd;
    int client_fd;
    int ret, n;
    char buf[1024];
    struct sockaddr_in sin;
    fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( fd < 0 ) {
        printf( "%s\n", strerror( errno ) );
    }    

    sin.sin_family = AF_INET;
    sin.sin_port = htons( 9999 );
    sin.sin_addr.s_addr = inet_addr( "192.168.58.99990" );

    ret = bind( fd, ( void* )&sin, sizeof(sin) );
    if ( ret < 0 ) {
        printf( "%s\n", strerror( errno ) );
    }
    
    ret = listen( fd, 64 );
    if ( ret < 0 ) {
        printf( "%s\n", strerror( errno ) );
    }
    client_fd = accept(fd, NULL, NULL );
    for (;;) {
        memset(buf, 0x00, sizeof(buf));
        n = read( client_fd, buf, sizeof(buf) );
        printf("n = %d\n", n);
        printf("%s\n", buf);
        write(client_fd, "hello world\n", strlen("hello world \n"));
        
    }
    close(client_fd);

}