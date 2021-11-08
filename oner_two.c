#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PACK_SIZE 65535
static char addr[] = "192.168.2.11";

void str_cli(FILE *fp, int sockfd);
int main(int argc, char **argv)
{
    int fd;
    int ret;
    int n;

    struct sockaddr_in sin;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("socket err %s\n", strerror(errno));
    }

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(9999);
    sin.sin_addr.s_addr = inet_addr(addr);

    if (connect(fd, (void *)&sin, sizeof(sin)) < 0)
    {
        printf("%s\n", strerror(errno));
    }

    while (1)
    {
        printf("go\n");
        str_cli(stdin, fd);
    }
    printf("return\n");
}

void str_cli(FILE *fp, int sockfd)
{
    char recvline[PACK_SIZE];
    char sendline[PACK_SIZE];
    while (fgets(sendline, PACK_SIZE, stdin) != NULL)
    {
        write(sockfd, sendline, strlen(sendline));
        read(sockfd, recvline, PACK_SIZE);
        fputs(recvline, stdout);
    }
    return;
}
