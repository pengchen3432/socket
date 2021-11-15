#include <stdio.h>
#include <unistd.h>
#include <signal.h>
void hand()
{
    printf("放假了\n");
}
int main()
{
    signal(SIGALRM, hand);
    while (1)
    {
        alarm(2);
        sleep(2);
    }
}
