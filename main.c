/****************************************************************************
*
* FILENAME:        main.c
*
* DESCRIPTION:     Description of this source file's contents
*
* Copyright (c) 2017 by Grandstream Networks, Inc.
* All rights reserved.
*
* This material is proprietary to Grandstream Networks, Inc. and,
* in addition to the above mentioned Copyright, may be
* subject to protection under other intellectual property
* regimes, including patents, trade secrets, designs and/or
* trademarks.
*
* Any use of this material for any purpose, except with an
* express license from Grandstream Networks, Inc. is strictly
* prohibited.
*
***************************************************************************/

//===========================
// Includes
//===========================
#include <stdio.h>
#include <stdlib.h>
#include <linux/filter.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/if.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "gs_utils.h"
//===========================
// Defines
//===========================

//===========================
// Typedefs
//===========================
struct ip_udp_dhcp_packet
{
    struct ether_header eth;
    struct iphdr ip;
    struct udphdr udp;
} __attribute__((packed));

//===========================
// Locals
//===========================
/* Variables */
char *app_name = "test";
/* Functions */
//=============================================================================

//==============================================================================
int main(
    int argc,
    char **argv)
//==============================================================================
{
    int ret;
    int ifindex;
    struct sockaddr_ll sock;
    int fd;
    const int const_int_1 = 1;
    int bytes;
    char buf[4096];
    struct ip_udp_dhcp_packet *packet;
    unsigned char cmsgbuf[CMSG_LEN(sizeof(struct tpacket_auxdata))];
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    char *dip = "224.0.0.251";
    struct in_addr in;

    ret = gs_get_ifindex_byname("br-lan0_zone0", &ifindex);
    if (ret < 0)
    {
        printf("Can not get interface eth0 ifindex, %s!\n", strerror(errno));
        return ret;
    }

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0)
    {
        printf("Socket creat filed, %s!\n", strerror(errno));
        return -1;
    }

    if (setsockopt(fd, SOL_PACKET, PACKET_AUXDATA, &const_int_1, sizeof(int)) < 0)
    {
        printf("Can't set PACKET_AUXDATA on raw socket, %s\n", strerror(errno));
        return -1;
    }

    sock.sll_family = AF_PACKET;
    sock.sll_protocol = htons(ETH_P_ALL);
    sock.sll_ifindex = ifindex;
    if ((ret = bind(fd, (struct sockaddr *)&sock, sizeof(sock))) < 0)
    {
        printf("Bind failed, %s!\n", strerror(errno));
        return -1;
    }

    /* used to use just safe_read(fd, &packet, sizeof(packet))
     * but we need to check for TP_STATUS_CSUMNOTREADY :(
     */
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    for (;;)
    {
        memset(buf, 0, sizeof(buf));

        bytes = recvmsg(fd, &msg, 0);
        if (bytes < 0)
        {
            if (errno == EINTR)
                continue;
            printf("Packet read error, ignoring\n");
            /* NB: possible down interface, etc. Caller should pause. */
            return bytes; /* returns -1 */
        }

        packet = (struct ip_udp_dhcp_packet *)buf;
        if (bytes < (int)(sizeof(packet->ip) + sizeof(packet->udp)))
        {
            printf("Packet is too short, ignoring\n");
            continue;
        }

        /* ignore any extra garbage bytes */
        bytes = ntohs(packet->ip.tot_len);

        /* make sure its the right packet for us, and that it passes sanity checks */
        if (packet->ip.protocol != IPPROTO_UDP || packet->ip.version != IPVERSION)
        {
            printf("Non udp!\n");
            continue;
        }

        in.s_addr = packet->ip.daddr;
        printf("dip %s\n", inet_ntoa(in));
        if (!strcmp(inet_ntoa(in), dip))
        {
            for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg))
            {
                if (cmsg->cmsg_level == SOL_PACKET && cmsg->cmsg_type == PACKET_AUXDATA)
                {
                    struct tpacket_auxdata *aux = (void *)CMSG_DATA(cmsg);
                    printf("vlan id %d\n", aux->tp_vlan_tci);
                }
            }
        }
    }

    return 0;
}

/* EOF */
