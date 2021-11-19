/****************************************************************************
*
* FILENAME:        forward_message.h
*
* DESCRIPTION:     Description of this header file's contents
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

#ifndef FORWARD_MESSAGE_H
#define FORWARD_MESSAGE_H

//===========================
// Includes
//===========================
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

#include "config_init.h"
#include "mdns.h"

//===========================
// Defines
//===========================
#define zero 0x00
#define ETH_HDR_LEN 14
#define UDP_HDR_LEN 8
#define PACKET_SIZE 65536

//===========================
// Typedefs
//===========================
struct vlan_hdr
{
    unsigned short h_vlan_encapsulated_proto;
    unsigned short h_vlan_TCI;
};

struct ip_udp_mdns_packet
{
    struct ether_header eth;
    struct iphdr ip;
    struct udphdr udp;
    struct mdns_pkt mdns;
} __attribute__((packed));

//===========================
// Globals
//===========================
extern char *app_name;

//===========================
// Functions
//===========================
int mdns_packet_offset(struct ip_udp_mdns_packet *packet);
int hit_target_vlan(char *buf, int packet_length, unsigned short dvlan);
int socket_init();
int bind_socket_interface(int sockfd, char *interface);
void add_socket_bind_ssid_list(struct socket_bind_ssid **socket_bind_ssid_list_head, struct socket_bind_ssid **socket_bind_ssid_list_tail, int sockfd, struct ssid *ssids);
struct ssid *dst_vlan_is_exist(struct ssid *head, int vlan);
int allow_all_services_to_pass(struct allow_services *services_list);
struct ssid *query_ssid_by_socket(struct socket_bind_ssid *socket_bind_ssid_list_head, int sockfd);
int recove_packet(char *buf, int sockfd);
int bridge_interface_init();
int Allow_current_service_to_pass(struct mdns_pkt *mdns, struct allow_services *services_list);
void send_packet_to_bridge(char *packet, int packet_length, int s);
void printf_ssids(struct ssid *ssids_list_head);

#endif
