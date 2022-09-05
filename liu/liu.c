#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>        
#include <sys/stat.h>        
#include <fcntl.h>
#include <string.h>
int main()
{
    int fd;
    int i;
    char value[100] = {0};
    fd = open( "chen", O_EXCL );
    if ( fd < 0 ) {
        fd = open( "chen", O_RDWR | O_CREAT, 0666 );
        if ( fd < 0 ) {
            printf("open failed\n");
        }
    }
    printf("fd = %d", fd );
    for ( i = 0; i < 1000000000000000000; i++ ) {
        sprintf(value, "%d\n", i);
        write( fd, value, strlen(value) );

    }
    close( fd );
    return 0;
}