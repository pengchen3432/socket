/****************************************************************************
* *
* * FILENAME:        $RCSfile: apply.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/26
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
#include "global.h"
#include "utils.h"
#include "track.h"
#include "time.h"
#include "apply.h"
#include "config.h"

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================

//=================
//  Globals
//=================
extern struct track_exec_param tk_ec_pm[__CONFIG_TRACK_MAX];
//=================
//  Locals
//=================

/*
 * Private Functions
 */

/*
 * Private Data
 */
static struct list_head applys = LIST_HEAD_INIT( applys );
static uint32_t reload_flag = 0;
//=================
//  Functions
//=================

//=============================================================================
void
apply_set_reload_flag(
    cfg_track config
)
//=============================================================================
{
    if( 0 > config || __CONFIG_TRACK_MAX < config ) {
        cfmanager_log_message( L_ERR, "Unknown config:%d\n", config );
        return;
    }

    reload_flag |= BIT( config );
}

//=============================================================================
int
apply_get_reload_flag(
    cfg_track config
)
//=============================================================================
{
    if( 0 > config || __CONFIG_TRACK_MAX < config ) {
        cfmanager_log_message( L_ERR, "Unknown config:%d\n", config );
        return 0;
    }

    return reload_flag & BIT( config );
}

//=============================================================================
void
apply_flush_reload_flag(
    cfg_track config
)
//=============================================================================
{
    if( 0 > config || __CONFIG_TRACK_MAX < config ) {
        cfmanager_log_message( L_ERR, "Unknown config:%d\n", config );
        return;
    }

    reload_flag &= ~BIT( config );
}

//=============================================================================
void
apply_add(
    const char *name
)
//=============================================================================
{
    struct apply_list *apply;

    if( cm_sve_init.boot_start || cm_sve_init.first_load_cfg ) {
        cfmanager_log_message( L_ERR, "Wait for configuration loading to complete\n" );
        return;
    }

    if( NULL == name ) {
        cfmanager_log_message( L_ERR, "the name is NULL\n" );
        return;
    }

    list_for_each_entry( apply, &applys, list ) {
        if ( !strcmp(apply->name, name) ) {
            cfmanager_log_message( L_WARNING, "%s has been added into apply list!\n", name );
            return;
        }
    }

    apply = calloc(1, sizeof(struct apply_list) );
    if( !apply ) {
        cfmanager_log_message( L_ERR, "failed to alloc apply_list struct\n" );
        return;
    }

    apply->name = strdup( name );

    list_add_tail( &apply->list, &applys );
}

//=============================================================================
void
apply_flush(
    void
)
//=============================================================================
{
    struct apply_list* node = NULL, *tmp = NULL;

    if ( !list_empty( &applys ) ) {
        list_for_each_entry_safe( node, tmp, &applys, list) {
            list_del( &node->list );
            free( node->name );
            free( node );
        }
    }
}

//=============================================================================
void
apply_exec(
    void
)
//=============================================================================
{
    struct track * t = NULL;
    struct apply_list* node = NULL, *tmp = NULL;

    if( APPLY_IN_PROCESS == apply_status ) {
        apply_timer_start();
        cfmanager_log_message( L_DEBUG,
            "The last command has not been execute complete\n" );

        return;
    }

    if ( !list_empty( &applys ) ) {
        list_for_each_entry_safe( node, tmp, &applys, list) {

            t = track_get( node->name );
            if ( t ) {
                t->flags |= OPTION_FLAGS_NEED_RELOAD;
            }

            list_del( &node->list );
            free( node->name );
            free( node );
        }
    }

    config_sync();
    track_apply();
}

//=============================================================================
void
apply_handle_all_reload_flag(
    void
)
//=============================================================================
{
    int i = 0;
    bool need_start_timer = false;

    for( i=0; i<__CONFIG_TRACK_MAX; i++ ) {
        if( !apply_get_reload_flag(i) ) {
            continue;
        }

        apply_add( tk_ec_pm[i].name );
        apply_flush_reload_flag(i);
        need_start_timer = true;
    }

    if( need_start_timer ) {
        apply_timer_start();
    }
}

//=============================================================================
void
apply_deinit(
    void
)
//=============================================================================
{
    apply_flush();
}

