#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
int main()
{
    system("cp src_file des_file");
    system("echo lala  >> des_file");
    return 0;
}