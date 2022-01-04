#include <stdio.h>
#include <sys/syslog.h>
#define pr(fmt, args...)  printf( fmt, ##args );

int main()
{
   int a = 1;
   int b = 2;
   pr("%d %d", a, b);
}