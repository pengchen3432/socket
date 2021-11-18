/****************************************************************************
*
* FILENAME:        config_init.h
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

#ifndef CONFIG_INIT_H
#define CONFIG_INIT_H

//===========================
// Includes
//===========================

//===========================
// Defines
//===========================

//===========================
// Typedefs
//===========================
struct dvlan_service
{
    unsigned short dvlan;
    struct allow_services *services_list;
    struct dvlan_service *next;
};

struct ssid
{
    char id[8];
    char vlan[8];
    char bonjour_forward;
    char interface_2g[8];
    char interface_5g[8];
    struct dvlan_service *dvlan_service_list;
    struct ssid *next;
};

struct allow_services
{
    char services_name_id[4];
    struct allow_services *next;
};

struct socket_bind_ssid
{
    int sock;
    struct ssid *s;
    struct socket_bind_ssid *next;
};

//===========================
// Globals
//===========================
extern char *app_name;

//===========================
// Functions
//===========================
struct dvlan_service *new_dvlan_service();
struct allow_services *new_allow_services();
struct socket_bind_ssid *new_socket_bind_ssid();
struct ssid *parse_config();
void add_global_socket_bind_ssid_list(struct socket_bind_ssid **socket_bind_ssid_list_head, struct socket_bind_ssid **socket_bind_ssid_list_tail, struct socket_bind_ssid *e);

#endif
