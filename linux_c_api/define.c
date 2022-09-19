#include <stdio.h>

int main()
{
#if (defined(GWN7062) || defined(GWN7052))
    printf("yes\n");
#endif
}