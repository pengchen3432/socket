#include <stdio.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
struct udp_check_head
{
    unsigned int src_ip;
    unsigned int dst_ip;
    unsigned short protocol;
    unsigned short length;
    struct udphdr udp;
};
void copy(unsigned char *dst, unsigned char *src, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        dst[i] = src[i];
    }
}
unsigned short check_sum(unsigned short *a, int length)
{
    unsigned int sum = 0;
    while (length > 1)
    {
        sum += *a++;
        length -= 2;
    }

    if (length)
    {
        sum += *(unsigned char *)a;
    }

    while (sum >> 16)
    {
        sum = (sum >> 16) + (sum & 0xffff);
    }

    return (unsigned short)(~sum);
}
int main()
{
    unsigned char data[] = {0x47, 0x8c, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x76, 0x6f, 0x72,
                            0x74, 0x65, 0x78, 0x04, 0x64, 0x61, 0x74, 0x61, 0x09, 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f,
                            0x66, 0x74, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x1c, 0x00, 0x01};
    printf("len = %ld", sizeof(data));
    int i = 0;
    struct iphdr iph;
    struct udphdr udph;
    struct udp_check_head check;
    unsigned char *p;
    unsigned char packet[1024];
    unsigned char *buf = packet + 20;
    memset(packet, 0, sizeof(packet));
    iph.saddr = inet_addr("192.168.2.11");
    iph.daddr = inet_addr("223.5.5.5");
    udph.check = 0;
    udph.uh_dport = htons(53);
    udph.uh_sport = htons(44319);
    udph.len = htons(51);

    check.src_ip = iph.saddr;
    check.dst_ip = iph.daddr;
    check.protocol = htons(IPPROTO_UDP);
    check.length = htons(51);
    check.udp = udph;

    p = (unsigned char *)(&check);
    // printf("len = %ldn", sizeof(data));
    copy(packet, p, sizeof(check));
    copy(buf, data, sizeof(data));
    for (i = 0; i < sizeof(data) + 20; i++)
    {
        if (i == 20)
        {
            printf("\n");
        }
        printf("%02x ", packet[i]);
    }
    printf("\n");

    printf("%04x \n", htons(check_sum((unsigned short *)packet, sizeof(data) + 20)));
}