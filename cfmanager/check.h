/****************************************************************************
* *
* * FILENAME:        $RCSfile: check.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/04/08
* *
* * DESCRIPTION:     xxxx feature:
* *
* *
* * Copyright (c) 2021 by Grandstream Networks, Inc.
* * All rights reserved.
* *
* * This material is proprietary to Grandstream Networks, Inc. and,
* * in addition to the above mentioned Copyright, may be
* * subject to protection under other intellectual property
* * regimes, including patents, trade secrets, designs and/or
* *
* * Any use of this material for any purpose, except with an
* * express license from Grandstream Networks, Inc. is strictly
* * prohibited.
* *
* ***************************************************************************/
#ifndef __CHECK_H__
#define __CHECK_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================
#define CF_CONFIG_NAME_CFMANAGER "cfmanager"

//=================
//  Typedefs
//=================
typedef enum {
    CHECK_WIRELESS,
    CHECK_DHCP,
    CHECK_NETWORK,
    CHECK_QOS,
    CHECK_SYSTEM,
    CHECK_EMAIL,
    CHECK_NOTIFICATION,
    CHECK_FIREWALL,
    CHECK_SCHEDULE,
    CHECK_MWAN3,
    CHECK_ECM,
    CHECK_DDNS,
    CHECK_QCACFG80211,
    CHECK_TRACKS,
    CHECK_FSTAB,
    CHECK_LBD,
    CHECK_IPSEC,
    CHECK_UPNPD,
    CHECK_SSID_STEERING,
    CHECK_DROPBEAR,
    CHECK_HYD,
    CHECK_NSS,
    CHECK_RPCD,
    CHECK_GRANDSTREAM,
    CHECK_CONTROLLER,
    CHECK_BWCTRL,
    CHECK_CFMANAGER,
    CHECK_SAMBA,
    CHECK_PORTALCFG,
    CHECK_TR069,
    CHECK_CLIENTBRIDGE,

    __CHECK_MAX
} check_config;
//=================
//  Globals
//=================

//=================
//  Locals
//=================

/*
 * Private Functions
 */

/*
 * Private Data
 */

//=================
//  Functions
//=================
void
check_set_defvalue(
    check_config ck_cfg
);

void
check_create_all_cfg(
    void
);

#endif //__CHECK_H__
