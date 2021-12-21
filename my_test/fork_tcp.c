#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <wait.h>
void hand() {
    int pid;
    while ( (pid = waitpid(-1, NULL, WNOHANG)) > 0 ) {
        printf("%d recv\n", pid);
    }
}
int main()
{
    int ser_fd, ret, cli_fd, n, pid;
    char buf[1024];
    const int one = 1;
    struct in_addr addr;
    memset(buf, 0x00, sizeof(buf));
    struct sockaddr_in in, client;
    socklen_t len = sizeof(client);
    signal(SIGCHLD, hand);
    ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_fd < 0) {
        printf("socket err\n");
        return -1;
    }

    ret = setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if ( ret < 0 ) {
        printf("set option err\n");
        return -1;
    }

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(9999);
    in.sin_addr.s_addr = inet_addr("192.168.2.11");

    ret = bind(ser_fd, (void *)&in, sizeof(in));
    if (ret < 0) {
        printf("bind err\n");
        return -1;
    }

    ret = listen(ser_fd, 64);
    if ( ret < 0 ) {
        printf("listen err\n");
        return -1;
    }


    while ( 1 ) {
        cli_fd = accept(ser_fd, (void *)&client, &len);
        if (cli_fd > 0) {
            addr.s_addr = client.sin_addr.s_addr;
            printf("cli_fd = %d\n", cli_fd);
        }
        else {
            continue;
        }
        pid = fork();
        if (pid > 0) {
            close(cli_fd);
        }
        else if (pid == 0) {
            close(ser_fd);
            while ( 1 ) {
                n = read(cli_fd, buf, sizeof(buf));
                if (n > 0) {
                    printf("address %s\n", inet_ntoa(addr));
                    printf("prot = %d\n", ntohs(client.sin_port));
                    printf("buf = %s\n", buf);
                }
                else if (n == 0) {
                    printf("address %s\n", inet_ntoa(addr));
                    printf("prot = %d\n", ntohs(client.sin_port));
                    printf("exit\n");
                    close(cli_fd);
                    exit(ntohs(client.sin_port));
                }
                else {
                    printf("exit\n");
                    close(cli_fd);
                    exit(-1);
                }
            }
        }
        else {
            printf("fork err\n");
            return -1;
        }
    }
    close(ser_fd);
}