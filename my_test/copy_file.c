#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <errno.h>
#include <stdbool.h>

bool copy_file(char *src, char *dest)
{
    int src_fd, dest_fd, n_char;
    char buf[1024] = {0};
    src_fd = open(src, O_RDONLY);
    if (src_fd < 0) 
    {
        syslog(LOG_ERR, "%s: %s",__func__, strerror(errno));
        return false;
    }

    dest_fd = creat(dest, 0644);
    if (dest_fd < 0)
    {
        syslog(LOG_ERR, "%s: %s",__func__, strerror(errno));
        return false;
    }

    while ( (n_char = read(src_fd, buf, sizeof(buf))) > 0 )
    {
        write(dest_fd, buf, n_char);
        memset(buf, 0x00, n_char);
    }
    if (n_char < 0)
    {
        syslog(LOG_ERR, "%s: %s",__func__, strerror(errno));
        return false;
    }

    close(src_fd);
    close(dest_fd);
    
    return true;

}