#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void str_cli(int socket);
void hand();

int main()
{
    int fd;
    int cli_fd;
    int ret;
    struct sockaddr_in servaddr, cliaddr;
    pid_t pid;

    socklen_t len = sizeof(cliaddr);
    const int one = 1;

    //signal(SIGCHLD, hand);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("%s\n", strerror(errno));
    }

    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (ret < 0)
    {
        printf("%s\n", strerror(errno));
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999);
    servaddr.sin_addr.s_addr = inet_addr("192.168.2.11");

    bind(fd, (void *)&servaddr, sizeof(servaddr));
    listen(fd, 2);

    for (;;)
    {
        cli_fd = accept(fd, (void *)&cliaddr, &len);
        printf("fd = %d\n", cli_fd);
        if ((pid = fork() == 0))
        {
            printf("father:%d\nchild:%d\n", getppid(), getpid());
            close(fd);
            str_cli(cli_fd);
            close(cli_fd);
            exit(0);
        }
        close(cli_fd);
        printf("cliaddr: %s\n", inet_ntoa(cliaddr.sin_addr));
        printf("cliport: %d\n", ntohs(cliaddr.sin_port));
    }
}

void str_cli(int socket)
{
    char buf[1024];
    int n;
    for (;;)
    {
        memset(buf, 0x00, sizeof(buf));
        n = read(socket, buf, sizeof(buf));
        if (n > 0)
        {
            printf("client:%s\n", buf);
        }
        else
        {
            break;
        }
    }
}

void hand()
{
    pid_t wpd;
    while ((wpd = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("%d child exit\n", wpd);
    }
}