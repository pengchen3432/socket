#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
static char addr[] = "192.168.2.11";
int recv_packet(int sockefd);
int main()
{
    int fd;
    int client_fd;
    int client_len;
    int ret, n;
    char buf[1024];
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    pid_t pid;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("%s\n", strerror(errno));
    }
    printf("serv %d\n", fd);
    int one = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (ret < 0)
    {
        printf("%s\n", strerror(errno));
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(fd, (void *)&servaddr, sizeof(servaddr));

    listen(fd, 64);

    for (;;)
    {
        client_fd = accept(fd, NULL, NULL);
        printf("client %d\n", client_fd);
        if ((pid = fork()) == 0)
        {
            close(fd);
            printf("father %d\nchild %d\n", getppid(), getpid());
            recv_packet(client_fd);
            close(client_fd);
            exit(0);
        }
        close(client_fd);
    }
    close(client_fd);
}

int recv_packet(int sockfd)
{
    int n;
    char buf[1024];
    memset(buf, 0x00, sizeof(buf));
    while (1)
    {
        n = recv(sockfd, buf, sizeof(buf), 0);
        if (n > 0)
        {
            fputs(buf, stdout);
            send(sockfd, "hello!\n", strlen("hello!\n"), 0);
        }
        else
        {
            break;
        }
    }
    return -1;
}