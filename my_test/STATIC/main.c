#include <stdio.h>
#include "add.h"
void fun() {
    printf("fun main\n");
}

void lala() {
    static int s = 1;
    s--;
    printf("s = %d\n", s);
}
int s = 10;
int main()
{
    printf("s = %d\n", s);
    lala();
    printf("s = %d\n", s);
}