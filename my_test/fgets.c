#include <stdio.h>
#include <string.h>
int main()
{
    FILE *in;
    char buf[1024] = {0};
    in = fopen("src", "r+");

    fread(buf, 1024, 1, in);
    printf("%s\n", buf);
    return 0;
}