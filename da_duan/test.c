#include <stdio.h>
#include <arpa/inet.h>
int main()
{
    int i;
    unsigned char *b;
    unsigned int a = 1;
    printf("len = %ld\n", sizeof(unsigned int));
    b = (unsigned char *)&a;
    for (i = 0; i < 4; i++)
    {
        printf("0x%02x ", b[i]);
    }
    printf("\n");
    unsigned int p = htonl(a);
    b = (unsigned char *)&p;
    for (i = 0; i < 4; i++)
    {
        printf("0x%02x ", b[i]);
    }
    printf("\n");
}
