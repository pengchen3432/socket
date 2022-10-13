#include <stdio.h>

int main()
{
#if ! defined(GWN7062)
    printf("yes\n");
#endif
}