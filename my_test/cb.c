#include <stdlib.h>
#include <stdio.h>
typedef int (* cb)(int a, int b);
typedef unsigned int uint32;
enum {
    ADD = 4,
    SUB,

    _MAX_SUM
};

#define BIT(x) (1ul << x)

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

uint32 option;

cb cal[_MAX_SUM] = {
    [ADD] = add,
    [SUB] = sub
}; 

int choose(int key, int a, int b)
{
    if (key < 0 || key >= _MAX_SUM)
    {
        return -1;
    }
    
    return cal[key](a, b);
}

int main()
{
    int c = choose(5, 2, 1);
    printf("c = %d\n", c);
}