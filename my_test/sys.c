#include <stdio.h>
#include <stdlib.h>
int main()
{
  int ret = system("sh -x a.sh");
  printf("ret = %d\n", ret);
}
