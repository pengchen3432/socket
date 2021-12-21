#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/ip.h>
void printf_mac(unsigned char *mac)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
int main()
{
    yuancheng
    git hub
    int fd;
    int ret, n;
    char buf[65535];
    struct ifreq ifr;
    struct sockaddr_in s;
    struct ethhdr *eth;
    struct iphdr *ip;
    struct in_addr in;
    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0)
    {
        printf("err %s\n", strerror(errno));
    }
    strncpy(ifr.ifr_name, "ens33", sizeof("ens33"));

    setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));
    if (ret < 0)
    {
        printf("err %s\n", strerror(errno));
    }

    for (;;)
    {
        memset(buf, 0x00, sizeof(buf));
        n = recv(fd, buf, sizeof(buf), 0);
        printf("=========================\n");
        eth = (struct ethhdr *)buf;
        printf("dst_mac:");
        printf_mac(eth->h_dest);
        //printf("%02x:%02x:%02x:%02x:%02x:%02x\n", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3],eth->h_dest[4], eth->h_dest[5]);
        printf("src_mac:");
        printf_mac(eth->h_source);
        //printf("%02x:%02x:%02x:%02x:%02x:%02x\n", eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3],eth->h_source[4], eth->h_source[5]);

        if (eth->h_proto == ETHERTYPE_IP)
        {
            printf("ip packet\n");
        }
        ip = (struct iphdr *)(buf + (sizeof(struct ethhdr)));
        if (ip->version == 4 && ip->ihl == 5)
        {
            in.s_addr = ip->daddr;
            printf("dst_ip:%s\n", inet_ntoa(in));
            in.s_addr = ip->saddr;
            printf("src_ip:%s\n", inet_ntoa(in));
        }
        printf("=========================\n\n");
    }
    dadadaadaada
}
