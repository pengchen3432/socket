/****************************************************************************
*
* FILENAME:        $RCSfile: mdns.c,v $
*
* LAST REVISION:   $Revision:  $
* LAST MODIFIED:   $Date:  $
*
* DESCRIPTION:     Generates OpenWRT config based on grandstream configuration
*
* Copyright (c) 2015 by Grandstream Networks, Inc.
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
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include "uci.h"
#include "gsconfig.h"
#include "utility.h"

//===========================
// Defines
//===========================
#define GS "grandstream"
#define MDNS_CONFIGPATH "/etc/config/mdns"
#define MDNS "mdns"

//===========================
// Globals
//===========================

//===========================
// Locals
//===========================

//=============================================================================
int mdns_config(
    int temp
)
//=============================================================================
{
    struct uci_package *gs_package;
    struct uci_package *wl_package;
    struct uci_package *mdns_package;
    struct uci_section *ssid_section;
    struct uci_section *wirless_section;
    struct uci_element *ssid_element;
    struct uci_element *dvlan_services_element;
    struct uci_element *wifi_iface_element;
    struct uci_option *dvlan_services = NULL;
    const char *bonjour_forward = NULL;
    const char *vlan = NULL;
    const char *id = NULL;
    const char *ifname = NULL;
    const char *additional_ssid = NULL;
    const char *wifi_iface_ssid = NULL;
    const char *interface_2g = NULL;
    const char *interface_5g = NULL;
    struct uci_ptr ptr = { 0 };
    int ret;

    gs_package = config_init( temp, "grandstream" );
    if ( !gs_package ) {
        uci_perror( uci_ctx, APPNAME );
        return -1;
    }

    wl_package = config_init( temp, "wireless" );
    if ( !wl_package ) {
        uci_perror( uci_ctx, APPNAME );
        return -1;
    }

    uci_clean_package( MDNS_CONFIGPATH );
    mdns_package = config_init( temp, MDNS );
    if ( !mdns_package ) {
        uci_perror( uci_ctx, APPNAME );
        return -1;
    }

    uci_foreach_element( &gs_package->sections, ssid_element ) {
        ssid_section = uci_to_section(ssid_element);
        if ( !strcmp( ssid_section->type, "additional_ssid" ) ) {

            bonjour_forward = uci_lookup_option_string( uci_ctx, ssid_section, "bonjour_forward" );
            vlan = uci_lookup_option_string( uci_ctx, ssid_section, "vlan" );
            id = uci_lookup_option_string( uci_ctx, ssid_section, "id" );
            dvlan_services = uci_lookup_option( uci_ctx, ssid_section, "dvlan_services");
            additional_ssid = uci_lookup_option_string( uci_ctx, ssid_section, "ssid" );

            memset(&ptr, 0, sizeof(ptr));
            ptr.p = mdns_package;
            ret = uci_add_section_named( uci_ctx, mdns_package, "mdns", &ptr.s, ssid_section->e.name );
            uci_add_option( &ptr, "id", id );
            uci_add_option( &ptr, "ssid", additional_ssid );
            if ( vlan ) {
                uci_add_option(&ptr, "vlan", vlan);
            }
            else {
                uci_add_option(&ptr, "vlan", "0");
            }

            
            if ( !bonjour_forward || !strcasecmp( bonjour_forward, "0" )) {
                uci_add_option(&ptr, "bonjour_forward", "0");
            }
            else if ( !strcmp( bonjour_forward, "1" ) ) {
                uci_add_option(&ptr, "bonjour_forward", bonjour_forward);
                uci_foreach_element( &wl_package->sections, wifi_iface_element ) {
                    wirless_section = uci_to_section(wifi_iface_element);
                    if ( !strcmp( wirless_section->type, "wifi-iface" ) ) {
                        wifi_iface_ssid = uci_lookup_option_string( uci_ctx, wirless_section, "ssid" );
                        ifname = uci_lookup_option_string( uci_ctx, wirless_section, "ifname" );
                        if ( !strcmp(wifi_iface_ssid, additional_ssid) ) {
                            if ( !interface_2g ) {
                                interface_2g = ifname;
                                uci_add_option(&ptr, "interface_2g", ifname);
                            }
                            else if ( !interface_5g ) {
                                interface_5g = ifname;
                                uci_add_option(&ptr, "interface_5g", ifname);
                            }
                        }
                    }
                }

                interface_2g = NULL;
                interface_5g = NULL;

                uci_foreach_element( &dvlan_services->v.list, dvlan_services_element ) {
                    ptr.o = NULL;
                    ptr.option = "dvlan_services";
                    ptr.value = dvlan_services_element->name;
                    uci_add_list( uci_ctx, &ptr );
                }
            }
        }   
    }

    ret = uci_save( uci_ctx, mdns_package);
    if(ret != UCI_OK) {
        uci_perror( uci_ctx, APPNAME);
        return -1;
    }

    ret= uci_commit( uci_ctx, &mdns_package, false );
    if ( ret != UCI_OK    ) {
        uci_perror( uci_ctx, APPNAME );
        return -1;
    }

    return 0;
}

