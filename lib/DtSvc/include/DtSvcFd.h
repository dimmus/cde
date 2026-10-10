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
 * DtSvcFd.h -- file descriptor helpers private to libDtSvc.
 */
#ifndef _DtSvcFd_h
#define _DtSvcFd_h

/*
 * Close (cloexec == 0) or mark close-on-exec (cloexec != 0) every file
 * descriptor >= lowfd.  Uses close_range(2) or closefrom(3) where the
 * system has them, else loops up to the descriptor limit.  Safe to call
 * in a child between fork and exec.
 */
extern void _DtSvcCloseFrom(int lowfd, int cloexec);

#endif /* _DtSvcFd_h */
