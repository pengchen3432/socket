/****************************************************************************
*
* FILENAME:        for_ward_message.c
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
#include <stdio.h>
#include <linux/if_packet.h>
#include <syslog.h>
#include "forward_message.h"
#include "gs_utils.h"

//===========================
// Defines
//===========================

//===========================
// Typedefs
//===========================

//===========================
// Locals
//===========================

//===========================
// Globals
//===========================
static char *dip = "224.0.0.251";
static int dstsock;
static struct sockaddr_ll dst;
static const char *const g_services_list[] = {"all services", "_airplay", "_smb", "_ssh", "_airtunes", "_ftp", "_googlecast", "_iChat", "_http"};

// tcpdump  udp port 5353  -dd
struct sock_filter filter_instr[] = {
    {0x28, 0, 0, 0x0000000c},
    {0x15, 0, 6, 0x000086dd},
    {0x30, 0, 0, 0x00000014},
    {0x15, 0, 15, 0x00000011},
    {0x28, 0, 0, 0x00000036},
    {0x15, 12, 0, 0x000014e9},
    {0x28, 0, 0, 0x00000038},
    {0x15, 10, 11, 0x000014e9},
    {0x15, 0, 10, 0x00000800},
    {0x30, 0, 0, 0x00000017},
    {0x15, 0, 8, 0x00000011},
    {0x28, 0, 0, 0x00000014},
    {0x45, 6, 0, 0x00001fff},
    {0xb1, 0, 0, 0x0000000e},
    {0x48, 0, 0, 0x0000000e},
    {0x15, 2, 0, 0x000014e9},
    {0x48, 0, 0, 0x00000010},
    {0x15, 0, 1, 0x000014e9},
    {0x6, 0, 0, 0x00040000},
    {0x6, 0, 0, 0x00000000},
};
struct sock_fprog bpf = {
    .len = sizeof(filter_instr) / sizeof(filter_instr[0]),
    .filter = (struct sock_filter *)filter_instr,
};

//=============================================================================
int mdns_packet_offset(
    struct ip_udp_mdns_packet *packet)
//=============================================================================
{
    int iphdr_length = 0;
    iphdr_length = packet->ip.ihl * 4;
    return ETH_HDR_LEN + UDP_HDR_LEN + iphdr_length;
}
//=============================================================================

//=============================================================================
int hit_target_vlan(
    char *buf,
    int packet_length,
    unsigned short dvlan)
//=============================================================================
{
    // 打上目的tag
    struct vlan_hdr vlanheader;
    vlanheader.h_vlan_encapsulated_proto = htons(0x8100);
    vlanheader.h_vlan_TCI = htons(dvlan);
    memcpy(buf, &vlanheader, sizeof(char) * 4);

    return packet_length + 4;
}

//=============================================================================
int socket_init(
    char *interface)
//=============================================================================
{
    int sockfd;
    int ret;
    sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0)
    {
        syslog(LOG_ERR, "socket error!\n");
        return -1;
    }

    // Socket binding ssid interface
    ret = bind_socket_interface(sockfd, interface);
    if (ret < 0)
    {
        syslog(LOG_ERR, "socket bind interface err\n");
        return -1;
    }

    ret = setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf));
    if (ret < 0)
    {
        syslog(LOG_ERR, "setsockopt error!\n");
        return -1;
    }

    return sockfd;
}

//=============================================================================
int bind_socket_interface(
    int sockfd,
    char *interface)
//=============================================================================
{
    int ret;
    int ifindex;
    struct sockaddr_ll sur;

    if ((ret = gs_get_ifindex_byname(interface, &ifindex)) < 0)
    {
        syslog(LOG_ERR, "gs ifindex by name err\n");
        return -1;
    }

    bzero(&sur, sizeof(sur));
    sur.sll_family = AF_PACKET;
    sur.sll_protocol = htons(ETH_P_ALL);
    sur.sll_ifindex = ifindex;
    if ((ret = bind(sockfd, (struct sockaddr *)&sur, sizeof(sur))) < 0)
    {
        syslog(LOG_ERR, "sur Bind failed\n");
        return -1;
    }
    return ret;
}

//=============================================================================
void add_socket_bind_ssid_list(
    struct socket_bind_ssid **socket_bind_ssid_list_head,
    struct socket_bind_ssid **socket_bind_ssid_list_tail,
    int sockfd,
    struct ssid *ssids)
//=============================================================================
{
    struct socket_bind_ssid *node = new_socket_bind_ssid();
    node->sock = sockfd;
    node->s = ssids;

    if (!(*socket_bind_ssid_list_head))
    {
        (*socket_bind_ssid_list_head) = node;
        (*socket_bind_ssid_list_tail) = node;
    }
    else
    {
        (*socket_bind_ssid_list_tail)->next = node;
        (*socket_bind_ssid_list_tail) = node;
    }
}

//=============================================================================
struct ssid *
dst_vlan_is_exist(
    struct ssid *ssids_list_head,
    int vlan)
//=============================================================================
{
    struct ssid *head = ssids_list_head;

    while (head)
    {
        if (vlan == atoi(head->vlan))
        {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

//=============================================================================
int allow_all_services_to_pass(
    struct allow_services *services_list)
//=============================================================================
{
    if (!strcmp(services_list->services_name_id, "0"))
    {
        return 1;
    }
    return 0;
}

//=============================================================================
struct ssid *
query_ssid_by_socket(
    struct socket_bind_ssid *socket_bind_ssid_list_head,
    int sockfd)
//=============================================================================
{
    struct socket_bind_ssid *head = socket_bind_ssid_list_head;

    while (head)
    {
        if (head->sock == sockfd)
        {
            return head->s;
        }

        head = head->next;
    }

    return NULL;
}

//=============================================================================
int recove_packet(
    char *buf,
    int sockfd)
//=============================================================================
{
    int bytes;
    struct ip_udp_mdns_packet *packet;
    struct sockaddr_ll sll;
    struct iovec iov;
    struct msghdr msg;
    struct in_addr in;
    iov.iov_base = buf;
    iov.iov_len = PACKET_SIZE;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_name = &sll;
    msg.msg_namelen = sizeof(sll);
    msg.msg_iovlen = 1;
    bytes = recvmsg(sockfd, &msg, 0);
    if (bytes <= 0)
    {
        syslog(LOG_ERR, "%s - %s:Packet read error, ignoring %s\n", app_name, __func__, strerror(errno));
        return -1;
    }

    // Only bags that come in from the outside
    if (sll.sll_pkttype == PACKET_OUTGOING)
    {
        return -1;
    }

    packet = (struct ip_udp_mdns_packet *)buf;

    if (bytes < (int)(sizeof(packet->ip) + sizeof(packet->udp)))
    {
        syslog(LOG_WARNING, "%s - %s Packet is too short, ignoring\n", app_name, __func__);
        return -1;
    }

    if (packet->ip.protocol != IPPROTO_UDP || packet->ip.version != IPVERSION)
    {
        syslog(LOG_WARNING, "%s - %s not udp\n", app_name, __func__);
        return -1;
    }

    in.s_addr = packet->ip.daddr;
    if (strcmp(dip, inet_ntoa(in)))
    {
        syslog(LOG_WARNING, "%s - %s:no mdns \n", app_name, __func__);
        return -1;
    }

    return bytes;
}

//=============================================================================
int bridge_interface_init()
//=============================================================================
{
    int ifindex, ret;
    if ((dstsock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        syslog(LOG_ERR, "%s - %s:socket err %s\n", app_name, __func__, strerror(errno));
        return -1;
    }

    ret = gs_get_ifindex_byname("br-lan0_ex", &ifindex);
    if (ret < 0)
    {
        syslog(LOG_ERR, "%s - %s:gs ifindex by name err\n", app_name, __func__);
        return -1;
    }

    bzero(&dst, sizeof(dst));
    dst.sll_family = AF_PACKET;
    dst.sll_protocol = htons(ETH_P_ALL);
    dst.sll_ifindex = ifindex;
    if ((ret = bind(dstsock, (struct sockaddr *)&dst, sizeof(dst))) < 0)
    {
        syslog(LOG_ERR, "%s - %s:Bind failed, !\n", app_name, __func__);
        return -1;
    }

    return 0;
}

//=============================================================================
int Allow_current_service_to_pass(
    struct mdns_pkt *mdns,
    struct allow_services *services_list)
//=============================================================================
{
    const char *str;
    struct rr_list *qu_list;
    struct rr_list *an_list;
    mdns->rr_qn = mdns->num_qn == 0 ? NULL : mdns->rr_qn;
    mdns->rr_ans = mdns->num_ans_rr == 0 ? NULL : mdns->rr_ans;

    while (services_list)
    {
        qu_list = mdns->rr_qn;
        an_list = mdns->rr_ans;
        str = g_services_list[atoi(services_list->services_name_id)];
        // Services allowed in the request　
        while (qu_list)
        {
            if (!strncmp(str, (char *)(qu_list->e->name + 1), strlen(str)))
            {
                return 1;
            }
            qu_list = qu_list->next;
        }

        // There are allowed services in the reply
        while (an_list)
        {
            if (!strncmp(str, (char *)(an_list->e->name + 1), strlen(str)))
            {
                return 1;
            }

            an_list = an_list->next;
        }

        services_list = services_list->next;
    }

    return 0;
}

//=============================================================================
void send_packet_to_bridge(
    char *packet,
    int packet_length,
    int s)
//=============================================================================
{
    int ret;

    ret = send(dstsock, packet, packet_length, 0);
    if (s == 1)
    {
        if (ret < 0)
        {
            printf("allservices send err \n");
        }
        else
        {
            printf("allservices send success yes\n");
        }
    }
    else if (s == 2)
    {
        if (ret < 0)
        {
            printf("Special question send err no\n");
        }
        else
        {
            printf("Special question send success yes\n");
        }
    }
}

//=============================================================================
void printf_ssids(
    struct ssid *ssids_list_head)
//=============================================================================
{
    struct ssid *ssids_list = ssids_list_head;
    while (ssids_list)
    {
        printf("========================\n");
        printf("ssid = %s\n", ssids_list->id);
        printf("svlan = %s\n", ssids_list->vlan);
        printf("sbonjour_forward = %c\n", ssids_list->bonjour_forward);

        if (strlen(ssids_list->interface_2g))
        {
            printf("2g:%s\n", ssids_list->interface_2g);
        }

        if (strlen(ssids_list->interface_5g))
        {
            printf("5g:%s\n", ssids_list->interface_5g);
        }
        if (ssids_list->bonjour_forward == '1')
        {
            struct dvlan_service *e = ssids_list->dvlan_service_list;

            while (e)
            {
                printf("dvlan = %d\n", e->dvlan);
                struct allow_services *head = e->services_list;
                printf("services:");

                while (head)
                {
                    printf("%s ", g_services_list[atoi(head->services_name_id)]);
                    head = head->next;
                }
                printf("\n");
                e = e->next;
            }
        }
        ssids_list = ssids_list->next;
        printf("\n");
        printf("=======================\n");
    }
}
