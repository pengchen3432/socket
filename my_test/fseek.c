#include <stdio.h>
#include <string.h>
int main()
{
    FILE *fd;
    char buf[1024] = {0};
    fd = fopen("src", "r");
    while ( fgets(buf, sizeof(buf), fd) > 0)
    {
        printf("%s", buf);
        memset(buf, 0x00, sizeof(buf));
    }
    fseek(fd, 0, SEEK_SET);
    while ( fgets(buf, sizeof(buf), fd) > 0)
    {
        printf("%s", buf);
        memset(buf, 0x00, sizeof(buf));
    }
    fclose(fd);
    return 0;
}