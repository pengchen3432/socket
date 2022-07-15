#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <sys/un.h>
#include<errno.h>
#include<stddef.h>
#include<unistd.h>
#include <netinet/in.h>
#define path "./local.sock"
int main()
{
    unlink( path );
    return 0;
}