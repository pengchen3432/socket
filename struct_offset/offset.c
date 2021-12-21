#include <stdio.h>
#include <string.h>
#define offset(s, n)  (size_t)&(((s*)0)->n)
typedef struct {
    char c;
    int a;
}My;
int main()
{
    printf("%ld\n", offset(My, a));
}