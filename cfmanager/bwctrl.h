/****************************************************************************
* *
* * FILENAME:        $RCSfile: bwctrl.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/04/22
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
#ifndef __BWCTRL_H__
#define __BWCTRL_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================
struct bwctrl_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
    char id[BUF_LEN_16];   //Corresponding to section name in cfmanager
};
//=================
//  Typedefs
//=================

//=================
//  Globals
//=================
extern struct vlist_tree bwctrl_rule_vltree;
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
int
cfparse_load_bwctrl(
    void
);

void
cfparse_bwctrl_init(
    void
);

void
cfparse_bwctrl_deinit(
    void
);

#endif //__BWCTRL_H__

