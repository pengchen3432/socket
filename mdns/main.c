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
#include <stdio.h>
#include <sys/epoll.h>
#include <string.h>
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
#include <syslog.h>

#include "config_init.h"
#include "forward_message.h"
#include "mdns.h"

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
char *app_name = "mdns";
static int g_bonjour_forward = 0;

int main()
{
    char packet[PACKET_SIZE + 4];
    int nready;
    int i, ret, packet_length, bytes;
    int sockfd;
    int mdns_offset;
    void *mdns_buffer;
    struct ssid *ssids;
    struct ssid *ssid;
    struct dvlan_service *dvlan_service_list;
    struct epoll_event events[64];
    struct mdns_pkt *mdns;
    struct epoll_event ev;
    struct ssid *ssids_list_head;
    struct socket_bind_ssid *socket_bind_ssid_list_head = NULL;
    struct socket_bind_ssid *socket_bind_ssid_list_tail = NULL;
    int epfd = epoll_create(64);
    char *buf = (packet + 4);

    // Initialize the bridge interface
    ret = bridge_interface_init();
    if (ret < 0)
    {
        syslog(LOG_ERR, "%s - %s:bridge init err\n", app_name, __func__);
        return -1;
    }

    // parser mdns file
    ssids_list_head = parse_config();
    if (!ssids_list_head)
    {
        syslog(LOG_ERR, "%s - %s:mdns file is NULL\n", app_name, __func__);
        return -1;
    }

    ssids = ssids_list_head;
    // Monitor the ssid interface with the bonjour switch turned on
    while (ssids)
    {
        if (ssids->bonjour_forward == '1')
        {
            g_bonjour_forward++;
            if (strlen(ssids->interface_2g))
            {
                // Apply for socket and set filter
                sockfd = socket_init(ssids->interface_2g);
                if (sockfd < 0)
                {
                    syslog(LOG_ERR, "%s - %s:socket init err\n", app_name, __func__);
                    return -1;
                }
                // Establish the mapping relationship between mapping socket file descriptor and ssid
                add_socket_bind_ssid_list(&socket_bind_ssid_list_head, &socket_bind_ssid_list_tail, sockfd, ssids);
                // Add socket to epoll tree
                ev.events = EPOLLIN;
                ev.data.fd = sockfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
            }

            if (strlen(ssids->interface_5g))
            {
                sockfd = socket_init(ssids->interface_5g);
                if (sockfd < 0)
                {
                    syslog(LOG_ERR, "%s - %s:socket init err\n", app_name, __func__);
                    return -1;
                }
                add_socket_bind_ssid_list(&socket_bind_ssid_list_head, &socket_bind_ssid_list_tail, sockfd, ssids);
                ev.events = EPOLLIN;
                ev.data.fd = sockfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
            }
        }
        ssids = ssids->next;
    }

    if (!g_bonjour_forward)
    {
        syslog(LOG_INFO, "%s - %s, There is no ssid to open bonjour\n", app_name, __func__);
        return -1;
    }

    while (1)
    {
        nready = epoll_wait(epfd, events, 64, -1);
        if (nready < 0)
        {
            syslog(LOG_ERR, "epoll err\n");
            return -1;
        }

        for (i = 0; i < nready; i++)
        {
            int fd = events[i].data.fd;
            memset(packet, 0x00, PACKET_SIZE + 4);
            // Find ssid through socket
            ssid = query_ssid_by_socket(socket_bind_ssid_list_head, fd);
            if (!ssid)
            {
                syslog(LOG_ERR, "%s - %s:no find ssid\n", app_name, __func__);
                return -1;
            }

            // recoeve packet
            bytes = recove_packet(buf, fd);
            if (bytes < 0)
                continue;

            // head offset
            memcpy(packet, buf, 12);

            // Get the service list of the vlan to be forwarded by the current ssid
            dvlan_service_list = ssid->dvlan_service_list;
            while (dvlan_service_list)
            {
                packet_length = bytes;
                // Determine whether the destination vlan exists
                if (dst_vlan_is_exist(ssids_list_head, dvlan_service_list->dvlan))
                {
                    // Allow all services to pass
                    if (allow_all_services_to_pass(dvlan_service_list->services_list))
                    {
                        // hit dst tag
                        packet_length = hit_target_vlan(packet + 12, packet_length, dvlan_service_list->dvlan);
                        // Send message to br-lan0_ex bridge
                        send_packet_to_bridge(packet, packet_length, 1);
                    }
                    else
                    {
                        // Parse mdns message
                        mdns_offset = mdns_packet_offset((struct ip_udp_mdns_packet *)buf);
                        mdns_buffer = (buf + mdns_offset);
                        mdns = mdns_parse_pkt(mdns_buffer, packet_length - mdns_offset);

                        // Determine whether the service request or answer can be passed
                        if (Allow_current_service_to_pass(mdns, dvlan_service_list->services_list))
                        {
                            packet_length = hit_target_vlan(packet + 12, packet_length, dvlan_service_list->dvlan);
                            send_packet_to_bridge(packet, packet_length, 2);
                        }

                        // Release memory
                        mdns_pkt_destroy(mdns);
                    }
                }
                dvlan_service_list = dvlan_service_list->next;
            }
        }
    }
}
