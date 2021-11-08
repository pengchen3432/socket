#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
void hand()
{
    pid_t wpd;
    printf("this is signal\n");
    while ((wpd = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("recv %d\n", wpd);
    }
}
int main()
{
    int i = 0;
    pid_t pid;
    int n;
    signal(SIGCHLD, hand);
    for (i = 0; i < 5; i++)
    {
        if ((pid = fork()) == 0)
        {
            printf("father %d\nchild %d\n", getppid(), getpid());
            exit(0);
        }
    }
}