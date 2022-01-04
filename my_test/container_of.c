#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define container_of(s, t)  (char *)(&(((struct s*)0)->t))

typedef struct Test {
    char a;
    int b;
}Test;

int main()
{
    Test p;
    Test *temp;
    printf("%p\n", &p);
    printf("%p\n", &p.b);
    char *offset = container_of(Test, b);
    printf("offset = %p\n", offset);
    temp = (Test *)((char *)&p.b - (char *)container_of(Test, b)); 
    printf("temp = %p\n", temp);
}