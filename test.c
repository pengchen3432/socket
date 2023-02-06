#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <syslog.h>
#include <stdlib.h>
int main(
  int argc, 
  char **argv
)
{
   int i = 0;
   for (i = 0; i < argc; i++) {
   	printf("i = %d  %s\n", i, argv[i]);
   }
}
