#include <stdio.h>
typedef int (*calculate)(int, int);
struct Math
{
    int a;
    int b;
    calculate fun;
};

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

int callback(struct Math m)
{
    return m.fun(m.a, m.b);
}

int main()
{
    struct Math m;
    int result;
    m.a = 2;
    m.b = 1;
    m.fun = add;
    printf("add result = %d\n", callback(m));
    m.fun = sub;
    printf("sub result = %d\n", callback(m));
}