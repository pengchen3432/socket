#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#define MAX_PAYLOAD 1024 // maximum payload size
#define NETLINK_CHEN 28 //自定义的协议
int main(
    int argc,
    char **argv
)
{
    int fd, ret;
    struct msghdr msg;
    struct iovec iov;
    struct sockaddr_nl src_addr, dst_addr;
    struct nlmsghdr *nl;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_CHEN);
    if ( fd < 0 ) {
        printf("socket failed\n");
        return -1;
    }

    bzero(&src_addr, sizeof(src_addr));
    src_addr.nl_family= AF_NETLINK;
    src_addr.nl_pid = getpid();
    ret = bind(fd, (void*)&src_addr, sizeof(src_addr));
    if (ret < 0) {
        printf("bind failed\n");
        return -1;
    }

    bzero(&dst_addr, sizeof(dst_addr));
    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pid = 0;
    nl = (struct nlmsghdr*) malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if ( !nl ) {
        printf("malloc failed\n");
        return -1;
    }
    memset(nl, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nl->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nl->nlmsg_pid = getpid();
    strcpy(NLMSG_DATA(nl), "hello world!");
    
    memset(&msg, 0, sizeof(msg));
    iov.iov_base = nl;
    iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
    msg.msg_name = &dst_addr;
    msg.msg_namelen = sizeof(dst_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    ret = sendmsg( fd, &msg, 0 );
    if ( ret < 0 ) {
        printf("sendmsg failed\n");
        return -1;
    }
    while (1) {
        ret = recvmsg( fd, &msg, 0 );
        if ( ret < 0 ) {
            printf("recv failed\n");
        }
        printf("msg:%s\n", (char *)NLMSG_DATA(nl));
    }

}