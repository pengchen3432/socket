#include <stdio.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
int main()
{
    int cli_fd;
    struct sockaddr_in in;
    int i, n, j, ret;
    pid_t pid;
    char buf[1024];

    cli_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_fd < 0)
    {
        printf("%s\n", strerror(errno));
    }

    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.2.11");

    for (i = 0; i < 3; i++)
    {
        if ((pid = fork()) == 0)
        {
            ret = connect(cli_fd, (void *)&in, sizeof(in));
            printf("ret = %d\n", ret);
            write(cli_fd, "hello\n", strlen("hello\n"));
            n = read(cli_fd, buf, sizeof(buf));
        }
    }

    while (wait(NULL) > 0)
    {
    }
}