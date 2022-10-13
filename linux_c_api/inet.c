#include <stdio.h>
#include <arpa/inet.h>
int main()
{
    char addr[] = "192.168.1.1"; 
    unsigned char * a;
    struct in6_addr addr6;
    struct in_addr addr4;
    if ( inet_pton(AF_INET, addr, &addr4.s_addr) ) {
        a = (unsigned char *)&addr4.s_addr;
        for (int i = 0; i < 4; i++) {
            printf("0x%02x\n", a[i]);
        }
    }
    char addrin[16];
    if ( inet_ntop(AF_INET, &addr4, addrin, 16) ) {
        printf("%s\n", addrin);
    }
}