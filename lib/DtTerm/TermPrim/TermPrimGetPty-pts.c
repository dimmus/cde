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
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include "TermPrim.h"
#include "TermPrimDebug.h"
#include "TermHeader.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

int
_DtTermPrimGetPty(char **ptySlave, char **ptyMaster)
{
    char *c;
    int ptyFd;

    *ptyMaster = NULL;

    if (isDebugFSet('p', 10)) {
#ifdef	BBA
#pragma	BBA_IGNORE
#endif	/*BBA*/
        return(-1);
    }

    if ((ptyFd = posix_openpt(O_RDWR)) >= 0) {

        /* use grantpt to prevent other processes from grabbing the tty that
         * goes with the pty master we have opened.  It is a mandatory step
         * in the SVR4 pty-tty initialization.  Note that /dev must be
         * mounted read/write...
         */
        Debug('T', timeStamp("_DtTermPrimGetPty() calling grantpt()"));
        if (grantpt(ptyFd) == -1) {
            (void) perror("grantpt");
            (void) close(ptyFd);
            return(-1);
        }

        /* Unlock the pty master/slave pair so the slave can be opened later */
        Debug('T', timeStamp("_DtTermPrimGetPty() calling unlockpt()"));
        if (unlockpt(ptyFd) == -1) {
            (void) perror("unlockpt");
            (void) close(ptyFd);
            return(-1);
        }
        Debug('T', timeStamp("_DtTermPrimGetPty() unlockpt() finished"));

        /* get the pty slave name... */
        if (c = ptsname(ptyFd)) {
            *ptySlave = malloc(strlen(c) + 1);
            (void) strcpy(*ptySlave, c);
            return(ptyFd);
        } else {
            /* ptsname on the pty master failed.  This should not happen!... */
            (void) perror("ptsname");
            (void) close(ptyFd);
        }
    } else {
        (void) perror("posix_openpt");
    }
    return(-1);
}

/* dummy functions */

void _DtTermPrimReleasePty(char *ptySlave) {}
void _DtTermPrimPtyCleanup(void) {}

int
_DtTermPrimSetupPty(char *ptySlave, int ptyFd)
{
  return(0);
}
