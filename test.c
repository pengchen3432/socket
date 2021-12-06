#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
int main()
{
    int a = 1;
    int b = 0;
    if ( (b = a) && b == 1 )
    {
        printf("success\n");
    }
}