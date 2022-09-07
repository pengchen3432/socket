#include <stdio.h>
#include <string.h>
int fun(char *s) {
    int slen = 0;
    for(; *s; s++)
    {
        slen++;
    }
    s[slen] = '\0';
    return slen;
}
int main()
{
    int c = fun("1234");
    printf("c = %d\n", c);
    return 0;
}