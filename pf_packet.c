#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
void printf_ethhdr(unsigned char *mac);
int main()
{
    int fd;
    char buf[1024];
    struct sockaddr_in in;
    struct msghdr msg;
    struct iovec iov;
    struct iphdr *ip;
    struct ethhdr *eth;
    const int one = 1;
    int bytes;
    struct in_addr addr;

    iov.iov_base = buf;
    iov.iov_len = 1024;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0)
    {
        printf("%s\n", strerror(errno));
    }

    while (1)
    {
        bytes = recvmsg(fd, &msg, 0);
        if (bytes <= 0)
            continue;

        eth = (struct ethhdr *)buf;

        if (eth->h_proto == ETHERTYPE_IP)
        {
            printf("src_mac: ");
            printf_ethhdr(eth->h_source);
            printf("dst_mac: ");
            printf_ethhdr(eth->h_dest);
            printf("this is ip protocol\n");
            ip = (struct iphdr *)(buf + sizeof(eth));
            addr.s_addr = ip->saddr;
            printf("src_ip: %s\n", inet_ntoa(addr));
            addr.s_addr = ip->daddr;
            printf("dst_ip: %s\n", inet_ntoa(addr));
        }
    }
}

void printf_ethhdr(unsigned char *mac)
{
    int i;
    for (i = 0; i <= 5; i++)
    {
        printf("%02x ", mac[i]);
    }
    printf("\n");
}
