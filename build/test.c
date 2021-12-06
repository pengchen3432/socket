#include <stdio.h>
#define a 1
int main()
{
    
    #if(gwn7630)
    printf("hello world\n");
    #endif

#if (a > 0)
    int b = 2;
    printf("b = %d\n", b);
#endif
    
}
