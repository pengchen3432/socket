/****************************************************************************
* *
* * FILENAME:        $RCSfile: tr069.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/10/13
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
#ifndef __TR069_H__
#define __TR069_H__
//=================
//  Includes
//=================

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
struct tr069_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
};
//=================
//  Globals
//=================
extern struct vlist_tree tr069_vltree;
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
cfparse_load_tr069(
    void
);

void
cfparse_tr069_init(
    void
);

void
cfparse_tr069_deinit(
    void
);

//=============================================================================
#endif //__TR069_H__
