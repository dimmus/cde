/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these libraries and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/* 
 * (c) Copyright 1995 Digital Equipment Corporation.
 * (c) Copyright 1995 Hewlett-Packard Company.
 * (c) Copyright 1995 International Business Machines Corp.
 * (c) Copyright 1995 Sun Microsystems, Inc.
 * (c) Copyright 1995 Novell, Inc. 
 * (c) Copyright 1995 FUJITSU LIMITED.
 * (c) Copyright 1995 Hitachi.
 *
 * MsgCat.c - public interfaces for the Cached Message Catalog Service
 *
 * $TOG: MsgCat.c /main/4 1999/07/02 14:02:03 mgreess $
 *
 */

#include <stdio.h>
#include <string.h>

#if defined(NO_XLIB)
#define _DtSvcProcessLock()
#define _DtSvcProcessUnlock()
#else
#include <X11/Intrinsic.h>
#include <DtSvcLock.h>
#endif

#include <Dt/MsgCatP.h>

int _DtCatclose(nl_catd catd)
{
    return (catd == (nl_catd) -1) ? 0 : catclose(catd);
}

char *_DtCatgets(nl_catd catd, int set, int num, const char *dflt)
{
    char *msg = NULL;

    if (catd == (nl_catd) -1 || set < 0 || num < 0) {
      /* Some catgets() implementations will fault if catd is invalid. */
      msg = (char *) dflt;
    } else {
      /* Per POSIX, we cannot assume catgets() is thread-safe. */
      _DtSvcProcessLock();
      msg = catgets(catd, set, num, dflt);
      _DtSvcProcessUnlock();
    }

   return msg;
}
