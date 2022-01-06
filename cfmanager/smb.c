/****************************************************************************
* *
* * FILENAME:        $RCSfile: smb.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/14
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
//=================
//  Includes
//=================
#include "smb.h"
#include "usb.h"
#include "time.h"
#include "track.h"
#include "apply.h"
#include "utils.h"
#include "cfparse.h"
#include "cfmanager.h"

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
enum {
    SMB_HOOKER_ALL,
    SMB_HOOKER_GLOBAL,
    SMB_HOOKER_SHARE,
    SMB_HOOKER_USER,

    __SMB_HOOKER_MAX
};

enum {
    SMB_VLTREE_ALL,
    SMB_VLTREE_GLOBAL,
    SMB_VLTREE_SHARE,
    SMB_VLTREE_USER,

    __SMB_VLTREE_MAX
};
//=================
//  Globals
//=================
const struct blobmsg_policy smb_global_rule_policy[__SMB_GLOBAL_RULE_MAX] = {
    [SMB_GLOBAL_RULE_ENABLE]      = { .name = "enable",
                                      .type = BLOBMSG_TYPE_BOOL },
    [SMB_GLOBAL_RULE_NAME]        = { .name = "name",
                                      .type = BLOBMSG_TYPE_STRING },
    [SMB_GLOBAL_RULE_DESCRIPTION] = { .name = "description",
                                      .type = BLOBMSG_TYPE_STRING },
    [SMB_GLOBAL_RULE_WORKGROUP]   = { .name = "workgroup",
                                      .type = BLOBMSG_TYPE_STRING },
    [SMB_GLOBAL_RULE_SECURITY]    = { .name = "security",
                                      .type = BLOBMSG_TYPE_STRING },
    [SMB_GLOBAL_RULE_INTERFACES]  = { .name = "interfaces",
                                      .type = BLOBMSG_TYPE_STRING },
    [SMB_GLOBAL_RULE_CHARSET]     = { .name = "charset",
                                      .type = BLOBMSG_TYPE_STRING },
    [SMB_GLOBAL_RULE_VALID_USERS] = { .name = "valid_users",
                                      .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy smb_share_rule_policy[__SMB_SHARE_RULE_MAX] = {
    [SMB_SHARE_RULE_NAME]       = { .name = "name",
                                    .type = BLOBMSG_TYPE_STRING },
    [SMB_SHARE_RULE_PATH]       = { .name = "path",
                                    .type = BLOBMSG_TYPE_STRING },
    [SMB_SHARE_RULE_COMMENT]    = { .name = "comment",
                                    .type = BLOBMSG_TYPE_STRING },
    [SMB_SHARE_RULE_AVAILABLE]  = { .name = "available",
                                    .type = BLOBMSG_TYPE_STRING },
    [SMB_SHARE_RULE_GUEST_OK]   = { .name = "guest_ok",
                                    .type = BLOBMSG_TYPE_STRING },
    [SMB_SHARE_RULE_GUEST_ONLY] = { .name = "guest_only",
                                    .type = BLOBMSG_TYPE_STRING },
};

const struct blobmsg_policy smb_user_rule_policy[__SMB_USER_RULE_MAX] = {
    [SMB_USER_RULE_USERNAME] = { .name = "username",
                                 .type = BLOBMSG_TYPE_STRING },
    [SMB_USER_RULE_PASSWORD] = { .name = "password",
                                 .type = BLOBMSG_TYPE_STRING },
};

struct vlist_tree smb_global_rule_vltree;
struct vlist_tree smb_share_rule_vltree;
struct vlist_tree smb_user_rule_vltree;

extern const struct blobmsg_policy usb_storage_partition_event_policy[__USB_STORAGE_PARTITION_EVENT_MAX];
//=================
//  Locals
//=================
 
/*
 * Private Functions
 */
static int
cfparse_global_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend * extend
);

static int 
cfparse_share_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

static int 
cfparse_user_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

/*
 * Private Data
 */

static const struct uci_blob_param_list global_rule_policy_list = {
    .n_params = __SMB_GLOBAL_RULE_MAX,
    .params = smb_global_rule_policy,
};

static const struct uci_blob_param_list share_rule_policy_list = {
    .n_params = __SMB_SHARE_RULE_MAX,
    .params = smb_share_rule_policy,
};

static const struct uci_blob_param_list user_rule_policy_list = {
    .n_params = __SMB_USER_RULE_MAX,
    .params = smb_user_rule_policy,
};

static const struct cm_hooker_policy smb_cm_hp[__SMB_HOOKER_MAX] = {
    [SMB_HOOKER_GLOBAL] = { .cb = cfparse_global_rule_hooker },
    [SMB_HOOKER_SHARE]  = { .cb = cfparse_share_rule_hooker },
    [SMB_HOOKER_USER]   = { .cb = cfparse_user_rule_hooker },
};

static const struct cm_vltree_info smb_vltree_info[__SMB_VLTREE_MAX] = {
    [SMB_VLTREE_GLOBAL] = {
        .key = "global",
        .vltree = &smb_global_rule_vltree,
        .policy_list = &global_rule_policy_list,
        .hooker = SMB_HOOKER_GLOBAL
    },
    [SMB_HOOKER_SHARE] = {
        .key = "share",
        .vltree = &smb_share_rule_vltree,
        .policy_list = &share_rule_policy_list,
        .hooker = SMB_HOOKER_SHARE
    },
    [SMB_HOOKER_USER] = {
        .key = "user",
        .vltree = &smb_user_rule_vltree,
        .policy_list = &user_rule_policy_list,
        .hooker = SMB_HOOKER_USER
    }
};

static struct blob_buf b;

//=================
//  Functions
//=================

//=============================================================================
static struct vlist_tree *
cfparse_smb_find_tree(
    const char *section_type
)
//=============================================================================
{
    int i = 0;
    if( NULL == section_type ) {
        return NULL;
    }
    for( i=1; i<__SMB_VLTREE_MAX; i++ ) {
        if( !smb_vltree_info[i].key ) {
            continue;
        }
        if( 0 == strcmp(smb_vltree_info[i].key, section_type) ) {
            cfmanager_log_message( L_DEBUG,
                                   "Find '%s' section's vltree\n",
                                   section_type );
            return smb_vltree_info[i].vltree;
        }
    }
    cfmanager_log_message(L_ERR, "Not found %s section's vltree\n", section_type);
    return NULL;
}

//=============================================================================
static const struct uci_blob_param_list *
cfparse_smb_find_blob_param_list(
    const char *section_type
)
//=============================================================================
{

    int i = 0;
    if( NULL == section_type ) {
        return NULL;
    }
    for( i=1; i<__SMB_VLTREE_MAX; i++ ) {
        if( !smb_vltree_info[i].key ) {
            continue;
        }
        if( 0 == strcmp(smb_vltree_info[i].key, section_type) ) {
            cfmanager_log_message( L_DEBUG, 
                                   "Find '%s' section's policy list\n",
                                   section_type );
            return smb_vltree_info[i].policy_list;
        }
    }
    cfmanager_log_message( L_ERR, 
                          "Not found '%s' section's policy list\n",
                          section_type );
    return NULL;
}

//=============================================================================
static void
cfparse_smb_node_free(
    struct smb_config_parse *smb_cfpar
)
//=============================================================================
{
    SAFE_FREE( smb_cfpar->cf_section.config );
    SAFE_FREE( smb_cfpar );
}

//=============================================================================
static void 
cfparse_smb_tree_free(
    struct vlist_tree * vltree,
    struct smb_config_parse *smb_cfpar
)
//=============================================================================
{
    avl_delete( &vltree->avl, &smb_cfpar->node.avl );
    cfparse_smb_node_free( smb_cfpar );
}

//=============================================================================
static int
cfparse_global_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    switch( index ) {
        case SMB_GLOBAL_RULE_ENABLE:
            break;
        case SMB_GLOBAL_RULE_NAME:
            break;
        case SMB_GLOBAL_RULE_DESCRIPTION:
            break;
        case SMB_GLOBAL_RULE_WORKGROUP:
            break;
        case SMB_GLOBAL_RULE_SECURITY:
            break;
        case SMB_GLOBAL_RULE_INTERFACES:
            break;
        case SMB_GLOBAL_RULE_CHARSET:
            break;
        case SMB_GLOBAL_RULE_VALID_USERS:
            break;
        default:
            cfmanager_log_message(L_ERR, "%d value not found\n",index);
    }
    option |= OPTION_FLAGS_NEED_RELOAD;
    return option;
}

//=============================================================================
static int
cfparse_share_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int option = 0;
    switch ( index ) {
        case SMB_SHARE_RULE_NAME:
            break;
        case SMB_SHARE_RULE_PATH:
            break;
        case SMB_SHARE_RULE_COMMENT:
            break;
        case SMB_SHARE_RULE_AVAILABLE:
            break;
        case SMB_SHARE_RULE_GUEST_OK:
            break;
        case SMB_SHARE_RULE_GUEST_ONLY:
            break;
        default:
            cfmanager_log_message(L_ERR, "%d value not found\n",index);
    }
    option |= OPTION_FLAGS_NEED_RELOAD;
    return option;
}

//=============================================================================
static int
cfparse_user_rule_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend * extend
)
//=============================================================================
{
    int option = 0;
    switch ( index ) {
    case SMB_USER_RULE_USERNAME:
        break;
    case SMB_USER_RULE_PASSWORD:
        break;
    default:
        cfmanager_log_message(L_ERR, "%d value not found\n",index);
    }
    option |= OPTION_FLAGS_NEED_RELOAD;
    return option;

}

//=============================================================================
static void
cfparse_smb_update_cfg(
    const struct blobmsg_policy *policy,
    struct smb_config_parse *smb_cfpar_old,
    struct smb_config_parse *smb_cfpar_new,
    int policy_size,
    int hooker
)
//=============================================================================
{
    struct blob_attr *tb_old[policy_size];
    struct blob_attr *tb_new[policy_size];
    struct cm_vltree_extend extend;
    int i = 0, option = 0;

    struct blob_attr *config_old = smb_cfpar_old->cf_section.config;
    struct blob_attr *config_new = smb_cfpar_new->cf_section.config;

    blobmsg_parse( policy, 
            policy_size,
            tb_old,
            blob_data(config_old),
            blob_len(config_old) );
    
    blobmsg_parse( policy,
            policy_size,
            tb_new,
            blob_data(config_new),
            blob_len(config_new) );

    memset( &extend, 0, sizeof(extend) );

    for( i = 0; i < policy_size; i++ ) {
        if( (!blob_attr_equal(tb_new[i],tb_old[i])) ||
            (extend.need_set_option&BIT(i)) ) {
            if ( smb_cm_hp[hooker].cb ) {
                option |= smb_cm_hp[hooker].cb( tb_new, i, &extend );
            }
        }
    }

    if ( option & OPTION_FLAGS_NEED_RELOAD ) {
        apply_set_reload_flag( CONFIG_TRACK_SAMBA );
    }
}

//=============================================================================
static void
cfparse_smb_call_update_func(
    const char *section_type,
    struct smb_config_parse *smb_cfpar_old,
    struct smb_config_parse *smb_cfpar_new
)
//=============================================================================
{
    int i = 0;
    if( NULL==section_type || NULL==smb_cfpar_old || NULL==smb_cfpar_new ) {
        return;
    }

    for ( i = 1; i<__SMB_VLTREE_MAX; i++ ) {
        if( 0 == 
            strcmp(smb_vltree_info[i].key, section_type )
        ) {
            cfparse_smb_update_cfg(smb_vltree_info[i].policy_list->params,
                                    smb_cfpar_old,
                                    smb_cfpar_new,
                                    smb_vltree_info[i].policy_list->n_params,
                                    smb_vltree_info[i].hooker );
        }
    }
}

//=============================================================================
static void
cfparse_smb_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct vlist_tree *vltree = NULL;
    struct smb_config_parse *smb_cfpar_old = NULL;
    struct smb_config_parse *smb_cfpar_new = NULL;

    if( node_old ) {
        smb_cfpar_old = container_of(node_old, struct smb_config_parse, node);
    }

    if( node_new ) {
        smb_cfpar_new = container_of(node_new, struct smb_config_parse, node);
    }

    if( smb_cfpar_old && smb_cfpar_new ) {
        cfmanager_log_message( L_WARNING, 
            "Update samba section: type '%s', name '%s'\n", 
            smb_cfpar_old->cf_section.type, 
            smb_cfpar_old->cf_section.name );

        if( blob_attr_equal(smb_cfpar_new->cf_section.config,
                            smb_cfpar_old->cf_section.config) ) {
            cfparse_smb_node_free( smb_cfpar_new );
            return;
        }

        cfparse_smb_call_update_func( smb_cfpar_old->cf_section.type,
                                     smb_cfpar_old,
                                     smb_cfpar_new );

        SAFE_FREE(smb_cfpar_old->cf_section.config);
        smb_cfpar_old->cf_section.config = 
            blob_memdup( smb_cfpar_new->cf_section.config );

        cfparse_smb_node_free( smb_cfpar_new );

    } else if( smb_cfpar_old ) {
        cfmanager_log_message( L_WARNING, 
            "Delete samba section type '%s', name '%s'\n",
            smb_cfpar_old->cf_section.type, 
            smb_cfpar_old->cf_section.name );

        vltree = cfparse_smb_find_tree(smb_cfpar_old->cf_section.type);
        if( NULL == vltree ) {
            return;
        }
        cfparse_smb_tree_free(vltree, smb_cfpar_old );

    } else if( smb_cfpar_new ) {
        cfmanager_log_message( L_WARNING, 
            "New samba section type '%s', name '%s'\n", 
            smb_cfpar_new->cf_section.type, 
            smb_cfpar_new->cf_section.name ); 
    }
    apply_set_reload_flag( CONFIG_TRACK_SAMBA );
}

//=============================================================================
static void 
cfparse_smb_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct smb_config_parse *smb_cfpar = NULL;
    struct vlist_tree *vltree = NULL;
    char *name;
    char *type;

    vltree = cfparse_smb_find_tree( section_type );
    if( NULL == vltree ) {
        cfmanager_log_message( L_DEBUG, 
            "No corresponding binary tree was found according to %s\n",
            section_type );
        return;
    }

    smb_cfpar = calloc_a( sizeof(*smb_cfpar), 
                          &name,
                          strlen(section_name)+1,
                          &type,
                          strlen(section_type)+1 );
    if( !smb_cfpar ) {
        cfmanager_log_message( L_ERR, 
            "failed to malloc samba_config_parse '%s'\n",
            section_name );
        return;
    }

    smb_cfpar->cf_section.name = strcpy( name, section_name );
    smb_cfpar->cf_section.type = strcpy( type, section_type );
    smb_cfpar->cf_section.config = blob_memdup( data );

    vlist_add( vltree, &smb_cfpar->node, smb_cfpar->cf_section.name );
}

//=============================================================================
static void
cfparse_smb_uci_to_blob(
    struct uci_section *s
)
//=============================================================================
{
    const struct uci_blob_param_list *uci_blob_list = NULL;

    blob_buf_init( &b, 0 );

    uci_blob_list = cfparse_smb_find_blob_param_list( s->type );
    if( NULL == uci_blob_list ) {
        cfmanager_log_message( L_DEBUG,
            "No corresponding uc_blob_param_list was found according to %s\n",
            s->type );
        return;
    }

    uci_to_blob( &b, s, uci_blob_list );

    cfparse_smb_add_blob_to_tree( b.head, s->e.name, s->type );

    blob_buf_free( &b );
}

//=============================================================================
int 
cfparse_load_smb(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct vlist_tree *vltree = NULL;
    struct uci_package *package = NULL;
    int i = 0;

    package = cfparse_init_package( "samba" );
    if (!package ) {
        cfmanager_log_message( L_ERR, "load samba package failed\n");
        return -1;
    }

    for( i = 1; i < __SMB_VLTREE_MAX; i++ ) {
        vltree = smb_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }
        vlist_update( vltree );
    }
    
    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );

        cfparse_smb_uci_to_blob( s );
    }

    for( i = 1; i<__SMB_VLTREE_MAX; i++ ) {
        vltree = smb_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }
        vlist_flush( vltree );
    }

    if( apply_get_reload_flag( CONFIG_TRACK_SAMBA ) ) {
        apply_add( "samba" );
        apply_flush_reload_flag( CONFIG_TRACK_SAMBA );
        apply_timer_start();
    }
    return 0;
}

//=============================================================================
int
cfparse_update_share(
    struct blob_attr * msg
)
//=============================================================================
{
    struct blob_attr *tb[__USB_STORAGE_PARTITION_EVENT_MAX];
    char path[LOOKUP_STR_SIZE] = {0};
    char action = USB_ACTION_NOACTION;
    char mount_path[BUF_LEN_64];
    char label[BUF_LEN_64];
    char uuid[BUF_LEN_64];
    char type[BUF_LEN_64];
    char section_name[BUF_LEN_64];
    char temp[BUF_LEN_128];

    blob_buf_init( &b, 0 );

    blobmsg_parse(usb_storage_partition_event_policy,
                  __USB_STORAGE_PARTITION_EVENT_MAX,
                  tb,
                  blobmsg_data(msg),
                  blobmsg_len(msg) );

    action = util_blobmsg_get_bool(tb[USB_STORAGE_PARTITION_ACTION],false);
    strncpy(mount_path,
            util_blobmsg_get_string(tb[USB_STORAGE_PARTITION_MOUNT_PATH], ""),
            sizeof(mount_path)-1);
    strncpy(uuid,
            util_blobmsg_get_string(tb[USB_STORAGE_PARTITION_UUID], ""),
            sizeof(uuid)-1);
    strncpy(section_name, uuid, sizeof(section_name));

    util_convert_specific_char(uuid,section_name, sizeof(section_name), '-', '_');

    if( action ) {
        strncpy(label,
                util_blobmsg_get_string(tb[USB_STORAGE_PARTITION_LABEL], ""),
                sizeof(label)-1);
        strncpy(type,
                util_blobmsg_get_string(tb[USB_STORAGE_PARTITION_TYPE], ""),
                sizeof(type)-1);

        config_add_named_section("samba", "share", section_name);
        snprintf(path, sizeof(path), "samba.%s.name", section_name);
        config_uci_set(path, label, 0);
        snprintf(path, sizeof(path), "samba.%s.path", section_name);
        config_uci_set(path, mount_path, 0);
        snprintf(temp, sizeof(temp), "This is a %s type usb storage.", type);
        snprintf(path, sizeof(path), "samba.%s.comment", section_name);
        config_uci_set(path, temp, 0);
        snprintf(path, sizeof(path), "samba.%s.available", section_name);
        config_uci_set(path, "yes", 0);
        snprintf(path, sizeof(path), "samba.%s.guest_ok", section_name);
        config_uci_set(path, "yes", 0);
        snprintf(path, sizeof(path), "samba.%s.writeable", section_name);
        config_uci_set(path, "yes", 0);

    } else {
        config_del_named_section("samba", "share", section_name);
    }

    config_commit("samba",false);
    cfparse_load_file("samba", 0, false);

    return 0;
}

//=============================================================================
void
cfparse_smb_init(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __SMB_VLTREE_MAX; i++ ) {

        vltree = smb_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }

        vlist_init( vltree, avl_strcmp, cfparse_smb_vltree_update);
        vltree->keep_old = true;
        vltree->no_delete = true;
    }
}
//=============================================================================

void 
cfparse_smb_deinit(
    void
)
//=============================================================================
{
    int i = 0;
    struct vlist_tree *vltree = NULL;

    for( i = 1; i < __SMB_VLTREE_MAX; i++ ) {
        vltree = smb_vltree_info[i].vltree;
        if( !vltree ) {
            continue;
        }
        vlist_flush_all(vltree);
    }
}
