#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <>
struct unp_in_pktinfo
{
    struct in_addr ipi_addr;
    int ip_ifindex;
};
ssize_t
recvfrom_flags(int fd, void *ptr, size_t nbytes, int *flagssp,
               struct sockaddr *sa, socklen_t *salenptr, struct unp_in_pktinfo *pktp)
{
    struct msghdr msg;
    struct iovec iov[1];
    ssize_t n;
    bzero(&msg, sizeof(msg));

    msg.msg_name = sa;
    msg.msg_namelen = *salenptr;
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    if ((n = recvmsg(fd, &msg, 0)) < 0)
    {
        return n;
    }
}
int main()
{
}