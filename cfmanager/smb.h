/****************************************************************************
* *
* * FILENAME:        $RCSfile: smb.h,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2020/12/23
* *
* * DESCRIPTION:     xxxx feature: 
* *
* *
* * Copyright (c) 2020 by Grandstream Networks, Inc.
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

#ifndef __SMB_H__
#define __SMB_H__

//=================
//  Includes
//=================
#include "global.h"
#include "config.h"

//=================
//  Defines
//=================

#define SMB_SECTION_TYPE_SIZE_MAX 20

//=================
//  Typedefs
//=================
struct smb_config_parse {
    struct vlist_node node;
    struct config_section_content cf_section;
};

enum {
    SMB_GLOBAL_RULE_ENABLE,
    SMB_GLOBAL_RULE_NAME,
    SMB_GLOBAL_RULE_DESCRIPTION,
    SMB_GLOBAL_RULE_WORKGROUP,
    SMB_GLOBAL_RULE_SECURITY,
    SMB_GLOBAL_RULE_INTERFACES,
    SMB_GLOBAL_RULE_CHARSET,
    SMB_GLOBAL_RULE_VALID_USERS,

    __SMB_GLOBAL_RULE_MAX
};

enum {
    SMB_SHARE_RULE_NAME,
    SMB_SHARE_RULE_PATH,
    SMB_SHARE_RULE_COMMENT,
    SMB_SHARE_RULE_AVAILABLE,
    SMB_SHARE_RULE_GUEST_OK,
    SMB_SHARE_RULE_GUEST_ONLY,

    __SMB_SHARE_RULE_MAX
};

enum {
    SMB_USER_RULE_USERNAME,
    SMB_USER_RULE_PASSWORD,

    __SMB_USER_RULE_MAX
};
//=================
//  Globals
//=================
extern const struct blobmsg_policy smb_global_rule_policy[__SMB_GLOBAL_RULE_MAX];
extern const struct blobmsg_policy smb_share_rule_policy[__SMB_SHARE_RULE_MAX];
extern const struct blobmsg_policy smb_user_rule_policy[__SMB_USER_RULE_MAX];

extern struct vlist_tree smb_global_rule_vltree;
extern struct vlist_tree smb_share_rule_vltree;
extern struct vlist_tree smb_user_rule_vltree;
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
cfparse_smb_init(
    void
);

void 
cfparse_smb_deinit(
    void
);

int 
cfparse_load_smb(
    void
);

int
cfparse_update_share(
    struct blob_attr * data
);

#endif //__SMB_H__