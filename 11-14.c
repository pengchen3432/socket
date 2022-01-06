chen
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
void hand()
{
    int wpd;
    while ((wpd = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("%d process exit \n", wpd);
    }
}
int main()
{
    int ser_fd, cli_fd;
    struct sockaddr_in in;
    const int one = 1;
    pid_t pid;
    int n;
    char buf[1024];
    signal(SIGCHLD, hand);

    ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_fd < 0)
    {
        printf("%s\n", strerror(errno));
    }
    setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.58.130");

    bind(ser_fd, (void *)&in, sizeof(in));
    listen(ser_fd, 64);

    while (1)
    {
        cli_fd = accept(ser_fd, NULL, NULL);
        printf("cli_fd %d\n", cli_fd);
        if ((pid = fork()) == 0)
        {
            close(ser_fd);
            printf("process:%d\n", getpid());
            while (1)
            {
              
                memset(buf, 0x00, sizeof(buf));
                n = read(cli_fd, buf, sizeof(buf));
                if (n <= 0)
                {
                    break;
                }
                else 
                {
                    printf("%d:%s", getpid(), buf);
                }
            }
            close(cli_fd);
            exit(0);
        }
        close(cli_fd);
    }
}
