/****************************************************************************
*
* FILENAME:        config_init.c
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
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <uci.h>

#include "config_init.h"

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
static struct uci_context *uci_ctx = NULL;

//=============================================================================
struct dvlan_service *
new_dvlan_service()
//=============================================================================
{
    struct dvlan_service *node = (struct dvlan_service *)malloc(sizeof(struct dvlan_service));
    if (!node)
    {
        syslog(LOG_ERR, "%s - %s:new dvlan_service err %s\n", app_name, __func__, strerror(errno));
        return NULL;
    }
    node->dvlan = 0;
    node->services_list = NULL;
    node->next = NULL;

    return node;
}

//=============================================================================
struct allow_services *
new_allow_services()
//=============================================================================
{
    struct allow_services *node = (struct allow_services *)malloc(sizeof(struct allow_services));
    if (!node)
    {
        syslog(LOG_ERR, "%s - %s new allow_services err %s\n", app_name, __func__, strerror(errno));
        return NULL;
    }

    node->next = NULL;
    memset(node->services_name_id, 0x00, sizeof(node->services_name_id));

    return node;
}

//=============================================================================
struct ssid *
new_ssid()
//=============================================================================
{
    struct ssid *node = (struct ssid *)malloc(sizeof(struct ssid));
    if (!node)
    {
        syslog(LOG_ERR, "%s - %s:new ssid err %s\n", app_name, __func__, strerror(errno));
        return NULL;
    }

    memset(node, 0, sizeof(struct ssid));
    return node;
}

//=============================================================================
void add_global_ssids_list(
    struct ssid **ssids_list_head,
    struct ssid **ssids_list_tail,
    struct ssid *e)
//=============================================================================
{
    if (!e)
    {
        syslog(LOG_ERR, "%s - %s:add ssid is NULL\n", app_name, __func__);
        return;
    }

    if (!(*ssids_list_head))
    {
        (*ssids_list_head) = e;
        (*ssids_list_tail) = e;
    }
    else
    {
        (*ssids_list_tail)->next = e;
        (*ssids_list_tail) = e;
    }
}

//=============================================================================
void add_services_list(
    struct allow_services **head,
    struct allow_services **tail,
    char *temp)
//=============================================================================
{
    if (!temp)
    {
        syslog(LOG_ERR, "%s - %s add service is NULL\n", app_name, __func__);
        return;
    }

    if (!(*head))
    {
        (*head) = new_allow_services();
        (*tail) = (*head);
        strncpy((*head)->services_name_id, temp, sizeof((*head)->services_name_id) - 1);
    }
    else
    {
        (*tail)->next = new_allow_services();
        (*tail) = (*tail)->next;
        strncpy((*tail)->services_name_id, temp, sizeof((*tail)->services_name_id) - 1);
    }
}

//=============================================================================
void add_dvlan_service(
    struct dvlan_service **head,
    struct dvlan_service **tail,
    struct dvlan_service *temp)
//=============================================================================
{
    if (!temp)
    {
        syslog(LOG_ERR, "%s - %s:add dvlan_services is NULL\n", app_name, __func__);
        return;
    }

    if (!(*head))
    {
        (*head) = temp;
        (*tail) = (*head);
    }
    else
    {
        (*tail)->next = temp;
        (*tail) = (*tail)->next;
    }
}

//=============================================================================
struct socket_bind_ssid *
new_socket_bind_ssid()
//=============================================================================
{
    struct socket_bind_ssid *node = (struct socket_bind_ssid *)malloc(sizeof(struct socket_bind_ssid));
    if (!node)
    {
        syslog(LOG_ERR, "%s- %s:new socket_bind_ssid err\n", app_name, __func__);
        return NULL;
    }

    node->next = NULL;
    node->s = NULL;
    return node;
}

//=============================================================================
struct dvlan_service *
parse_str(
    char *str)
//=============================================================================
{
    struct dvlan_service *result = new_dvlan_service();
    struct allow_services *head = NULL;
    struct allow_services *tail = NULL;
    int str_index = 0;
    int space_num = 0;
    int temp_index = 0;
    char temp[100];
    memset(temp, 0x00, sizeof(temp));

    while (str[str_index] != '\0')
    {
        if (str[str_index] == ' ')
        {
            space_num++;

            if (space_num == 1)
            {
                result->dvlan = atoi(temp);
                temp_index = 0;
                memset(temp, 0x00, sizeof(temp));
            }
            else
            {
                add_services_list(&head, &tail, temp);
                temp_index = 0;
                memset(temp, 0x00, sizeof(temp));
            }
        }
        else
        {
            temp[temp_index++] = str[str_index];
        }

        str_index++;
    }

    if (temp_index)
    {
        add_services_list(&head, &tail, temp);
    }

    result->services_list = head;

    return result;
}

//=============================================================================
struct uci_package *
config_init(
    int temp,
    const char *package)
//=============================================================================
{
    struct uci_package *p = NULL;

    if (!uci_ctx)
    {
        uci_ctx = uci_alloc_context();

        uci_ctx->flags &= ~UCI_FLAG_STRICT;
        if (temp)
        {
            uci_set_savedir(uci_ctx, "/tmp");
        }
    }
    else
    {
        p = uci_lookup_package(uci_ctx, package);
        if (p)
        {
            uci_unload(uci_ctx, p);
        }
    }

    if (uci_load(uci_ctx, package, &p))
    {
        return NULL;
    }

    return p;
}

//=============================================================================
struct ssid *
parse_config()
//=============================================================================
{
    int temp = 0;
    struct ssid *ssids_list_head = NULL;
    struct ssid *ssids_list_tail = NULL;
    struct uci_package *mdns_package;
    struct uci_element *element;
    struct uci_element *list;
    struct uci_section *section;
    struct uci_option *dvlan_services = NULL;
    struct dvlan_service *dvlan_service_list_head;
    struct dvlan_service *dvlan_service_list_tail;
    struct ssid *ssid;
    struct dvlan_service *temp_dvlan_service;
    const char *vlan = NULL;
    const char *id = NULL;
    const char *bonjour_forward = NULL;
    const char *interface_2g = NULL;
    const char *interface_5g = NULL;

    mdns_package = config_init(temp, "mdns");
    if (!mdns_package)
    {
        syslog(LOG_ERR, "%s - %s:mdns_package config init err\n", app_name, __func__);
        return NULL;
    }

    uci_foreach_element(&mdns_package->sections, element)
    {
        section = uci_to_section(element);
        if (!strcmp(section->type, "mdns"))
        {

            ssid = new_ssid();

            bonjour_forward = uci_lookup_option_string(uci_ctx, section, "bonjour_forward");
            ssid->bonjour_forward = *bonjour_forward;

            id = uci_lookup_option_string(uci_ctx, section, "id");
            strncpy(ssid->id, id, sizeof(ssid->id) - 1);

            vlan = uci_lookup_option_string(uci_ctx, section, "vlan");
            strncpy(ssid->vlan, vlan, sizeof(ssid->vlan) - 1);

            interface_2g = uci_lookup_option_string(uci_ctx, section, "interface_2g");
            if (interface_2g)
            {
                strncpy(ssid->interface_2g, interface_2g, sizeof(ssid->interface_2g) - 1);
            }

            interface_5g = uci_lookup_option_string(uci_ctx, section, "interface_5g");
            if (interface_5g)
            {
                strncpy(ssid->interface_5g, interface_5g, sizeof(ssid->interface_5g) - 1);
            }

            // Get the current ssid destination vlan and service string list
            if (!strcmp(bonjour_forward, "1"))
            {
                dvlan_services = uci_lookup_option(uci_ctx, section, "dvlan_services");

                dvlan_service_list_head = dvlan_service_list_tail = NULL;

                uci_foreach_element(&dvlan_services->v.list, list)
                {
                    // parse string
                    temp_dvlan_service = parse_str(list->name);
                    // Add to the current ssid destination vlan and service list
                    add_dvlan_service(&dvlan_service_list_head, &dvlan_service_list_tail, temp_dvlan_service);
                }

                // The head pointer of the linked list of the destination vlan and service to the current ssid
                ssid->dvlan_service_list = dvlan_service_list_head;
            }

            // Add ssid to the total ssids list
            add_global_ssids_list(&ssids_list_head, &ssids_list_tail, ssid);
        }
    }

    return ssids_list_head;
}
