#include <stdio.h>
int main()
{
    int a, b;
    char s[] = "1 1";
    sscanf(s, "%d %d", &a, &b);
    printf("a = %d\n", a);
    printf("b = %d\n", b);
    return 0;
}