#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
//　最大缓冲区大小
#define MAX_BUF_SIZE 5
#define MAX_EPOLL_EVENTS 20
#define EPOLL_LT 0
#define EPOLL_ET 1
#define FD_BLOCK 0
#define FD_NONBLOCK 1

int set_nonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    return flag;
}

void addfd_to_epoll(int epoll_fd, int fd, int epoll_type, int block_type)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;

    if (epoll_type == EPOLL_ET)
    {
        event.events |= EPOLLET;
    }

    if (block_type == FD_NONBLOCK)
    {
        set_nonblock(fd);
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

void epoll_lt(int sockfd)
{
    char buf[MAX_BUF_SIZE];
    int ret;

    memset(buf, 0x00, sizeof(buf));
    printf("开启recv..\n");
    ret = recv(sockfd, buf, sizeof(buf), 0);
    if (ret > 0)
    {
        printf("收到消息:%s,共%d个字节\n", buf, ret);
    }
    else 
    {
        if (ret == 0)
        {
            printf("客户端主动关闭\n");
        }
        close(sockfd);
    }
    printf("LT处理结束!!!!\n");
}

void epoll_et_loop(int sockfd)
{
    char buf[MAX_BUF_SIZE];
    int ret;

    while (1)
    {
        memset(buf, 0x00, sizeof(buf));
        ret = recv(sockfd, buf, sizeof(buf), 0);
        if (ret < 0)
        {
            if (errno == EAGAIN)
            {
                printf("数据读完了\n");
                break;
            }
        }
        else if (ret == 0)
        {
            printf("客户端主动关闭\n");
        }
        else 
        {
            printf("收到消息:%s,共%d个字节\n", buf, ret);
        }
    }
    printf("带循环ET处理结束!!!!\n");
}

void epoll_et_nonloop(int sockfd)
{
    char buf[MAX_BUF_SIZE];
    int ret;

    memset(buf, 0x00, sizeof(buf));
    ret = recv(sockfd, buf, sizeof(buf), 0);
    if (ret > 0)
    {
        printf("收到消息:%s,共%d个字节\n", buf, ret);
    }
    else
    {
        if (ret == 0)
        {
            printf("客户端正常退出\n");
        }
        close(sockfd);
    }
    printf("不带循环ET处理结束!\n");
}


int main()
{

}