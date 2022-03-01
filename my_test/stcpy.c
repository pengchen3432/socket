#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main()
{
    char dst[] = "abc";
    char src[] = "abcd";
    int ret = strcmp(src, dst);
    printf("ret = %d\n", ret);
}