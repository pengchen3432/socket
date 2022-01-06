/****************************************************************************
* *
* * FILENAME:        $RCSfile: initd.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/01/12
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
#include "initd.h"
#include "global.h"
#include "utils.h"
#include <libgen.h>
//=================
//  Defines
//=================

//=================
//  Typedefs
//=================

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
static struct list_head initds = LIST_HEAD_INIT(initds);
//=================
//  Functions
//=================


//=============================================================================
static int
initd_parse(
    const char *file
)
//=============================================================================
{
    struct initd *i;

    i = calloc(1, sizeof(struct initd) );
    if ( !i ) {
        cfmanager_log_message( L_ERR, "failed to alloc initd struct\n" );
        return -1;
    }

    i->name = strdup( file + 3 );

    list_add_tail( &i->list, &initds );

    return 0;
}

//=============================================================================
void
initd_apply(
    char *command,
    int cmd_size
)
//=============================================================================
{
    //int  len = 0;
    struct initd *node;

    if( !command || cmd_size <= 0 ) {
        return;
    }

    if ( !list_empty( &initds ) ) {
        list_for_each_entry( node, &initds, list ) {
            if ( node->reload ) {
                snprintf( command + strlen( command ),
                    cmd_size - strlen( command ),
                    "/etc/init.d/%s reload&&", node->name );
                cfmanager_log_message( L_DEBUG, "%s\n", command );
                node->reload = 0;
            }
        }
    }

    util_del_dup_cmd( command, cmd_size );
}

//=============================================================================
struct initd *
initd_get(
    const char *name
)
//=============================================================================
{
    struct initd *node;

    if ( !list_empty( &initds ) ) {
        list_for_each_entry( node, &initds, list ) {

            if ( strcmp( name, node->name ) == 0 ) {
                return node;
            }
        }
    }
    else {
        cfmanager_log_message( L_DEBUG, "initds:is empty\n" );
    }

    return NULL;
}

//=============================================================================
void
initd_load(
    void
)
//=============================================================================
{
    int gl_flags = GLOB_NOESCAPE | GLOB_MARK;
    glob_t gl;
    int j;
    int ret;

    ret = glob("/etc/rc.d/S*", gl_flags, NULL, &gl );

    if ( ret < 0 ) {
        cfmanager_log_message( L_DEBUG, "glob error : %d\n", ret );
        return;
    }
    else if ( 0 == ret ) {
        for( j = 0; j < gl.gl_pathc; j++ ) {
            initd_parse( basename(gl.gl_pathv[j]) );
        }
    }
    else {
        cfmanager_log_message( L_DEBUG, "glob error : %d\n", ret );
    }

    globfree( &gl );
}

//=============================================================================
void
initd_unload(
    void
)
//=============================================================================
{
    struct initd* node = NULL, *tmp = NULL;
    if ( !list_empty( &initds ) ) {
        list_for_each_entry_safe( node, tmp, &initds, list) {
            list_del( &node->list );
            free( node->name );
            free( node );
        }
    }
}