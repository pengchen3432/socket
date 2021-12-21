#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <linux/list.h>

struct my {
    char a;
    int b;
    char c;
};
int main()
{
    struct my chen;
    struct my *p;
    printf("sizeof = %ld\n", sizeof(chen));
    printf("a = %p\n", &chen.a);
    printf("b = %p\n", &chen.b);
    printf("c = %p\n", &chen.c);
    printf("c offset = %d\n", ((int)&((struct my *)0)->c) );
    p = container_of(&chen.c, struct my, c );
}