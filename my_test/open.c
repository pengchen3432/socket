#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
int main()
{
    int src_fd, ret, dest_fd;
    char buf[1024] = {0};
    src_fd = open("src", O_RDONLY, 0644);
    if ( src_fd < 0 ) {
        printf("src open err\n");
        return -1;
    }
    dest_fd = open("dest", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if ( dest_fd < 0 ) {
        printf("dest open err\n");
        return -1;
    }
    while ( (ret = read(src_fd, buf, sizeof(buf)) ) > 0) {
        write(dest_fd, buf, sizeof(buf));
        memset(buf, 0x00, sizeof(buf));
    }
    if (close(src_fd) == -1 || close(dest_fd) == -1 ) {
        printf("close err\n");
        return -1;
    }
    return 0;
}