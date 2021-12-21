#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
int copy_file(char *dst, char *src)
{
    int in_fd, out_fd, n_char;
    char buf[5];
    memset(buf, 0x00, sizeof(buf));
    if (src == NULL) {
        printf("src file is NULL\n");
        return -1;
    }

    in_fd = open( src, O_RDONLY );
    if ( in_fd < 0 ) {
        printf("open src  failed\n");
        return -1;
    }

    out_fd = creat( dst, 0644 );
    if ( out_fd < 0 ) {
        printf("creat dest failed\n");
        return -1;
    }

    while ((n_char = read( in_fd, buf, sizeof(buf))) > 0) {
        printf("n_char = %d\n", n_char);
        printf("buf = %s\n", buf);
        if (write( out_fd, buf, n_char ) != n_char) {
            printf("write err\n");
        }
    }
    if (n_char != 0) {
        printf("read err\n");
        return -1;
    }

    if ( close(in_fd) == -1 || close(out_fd) == -1 ) {
        printf("close err\n");
        return -1;
    }

}
int main()
{
    int ret;
    ret = copy_file("dest", "src");
    if (ret < 0) {
        printf("failed\n");
    }
    else {
        printf("success\n");
    }
}