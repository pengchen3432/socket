#include <stdio.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
void child_main(int i, int sockfd)
{
    int cli_fd;
    int n;
    int nread;
    struct sockaddr_in clien;
    char buf[1024];
    socklen_t len = sizeof(clien);
    fd_set rset;
    FD_ZERO(&rset);
    for (;;)
    {
        FD_SET(sockfd, &rset);
        nread = select(sockfd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rset))
        {
            printf("read yes\n");
        }
        else
        {
            printf("read no\n");
        }
        cli_fd = accept(sockfd, (void *)&clien, &len);
        printf("process: %d\n", getpid());
        printf("cliaddr:%s\n", inet_ntoa(clien.sin_addr));
        printf("cliprot:%d\n", htons(clien.sin_port));
        while (1)
        {
            memset(buf, 0x00, sizeof(buf));
            n = read(cli_fd, buf, sizeof(buf));
            if (n > 0)
            {
                printf("%d: %s", getpid(), buf);
            }
            else
            {
                close(cli_fd);
                break;
            }
        }
    }
}

pid_t chile_make(int i, int sockfd)
{
    pid_t pid;
    if ((pid = fork()) > 0)
    {
        return pid;
    }
    child_main(i, sockfd);
}
int main()
{
    int ser_fd;
    struct sockaddr_in in;
    const int one = 1;
    int n, i;
    pid_t pid;
    scanf("%d", &n);
    ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_fd < 0)
    {
        printf("%s\n", strerror(errno));
    }
    setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.2.11");

    bind(ser_fd, (void *)&in, sizeof(in));
    listen(ser_fd, 64);
    for (i = 0; i < n; i++)
    {
        pid = chile_make(i, ser_fd);
        printf("%d process init\n", pid);
    }
    pause();
}
