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
/* $TOG: tt_options.h /main/11 1998/04/03 17:09:46 mgreess $
 *
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1993, 1994, 1996 Hewlett-Packard Company.
 * (c) Copyright 1993, 1994, 1996 International Business Machines Corp.
 * (c) Copyright 1990, 1993, 1994, 1996 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994, 1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */

#ifndef _TT_OPTIONS_H
#define _TT_OPTIONS_H
/* 
 * This file controls any compilation-time options. The current set of
 * options is:
 * 
 * OPT_UNIX_SOCKET_RPC -- if defined then code to do rpc over unix
 *                        sockets is enabled (speeds up intra-system rpcs). 
 * 
 * OPT_TLI -- if defined then rpc code is compiled for the new
 *            transport-independent rpc
 * 
 * OPT_DLOPEN_X11 -- if defined then any code that would use libX11 will
 *                   attempt to dlopen it at runtime rather than require
 *                   the application to link with the library.
 *
 * OPT_DLOPEN_CE -- if defined then any code that would use libce will
 *                   attempt to dlopen it at runtime rather than require
 *                   the application to link with the library.
 *
 * OPT_ADDMSG_DIRECT -- if defined then messages are sent directly
 *			down a client's signalling fd rather than
 *			incurring an additional rpc. 
 *
 * OPT_SECURE_RPC -- if defined then code for implementing DES authorization
 *	             is included.
 *
 * OPT_CLASSING_ENGINE -- if defined, code is included to store and
 *			  retrieve type data in the OpenWindows Classing
 *	          	  Engine.  If not defined, type data is stored
 *			  only in XDR format files.
 *
 * OPT_PATCH -- if defined, build patch info into objects
 *
 * OPT_BUG_SUNOS_5 -- used to flag special hacks only needed because
 *   of bugs or quirks in SunOS 5.x.
 * 
 * OPT_BUG_RPCINTR -- used to flag that RPC_INTR is not defined
 *   in enum clnt_stat.
 * 
 * OPT_XDR_LONG_TYPE -- used for (64-bit) architectures where
 *   <rcp/xdr.h> defines x_putlong and x_getlong to not take "long*".
 *
 * OPT_POSIX_SIGNAL -- set to assume posix-style sigaction() call (default).
 * OPT_BSD_SIGNAL -- set to assume bsd-style signal() call. 
 *   If OPT_POSIX_SIGNAL is undefined and OPT_BSD_SIGNAL is not set, use
 *   the old SVR2-style sigset().
 *
 * OPT_BSD_WAIT -- set to assume bsd-style wait() calls, which expect
 *   a "union wait" instead of an int, and to not use waitpid().
 *
 * OPT_DGETTEXT -- set to use SVR4-style internationalization calls.
 *
 * OPT_CATGETS -- set to use XPG4-style internationalization calls.
 *
 * OPT_GETDTABLESIZE - use getdtablesize() instead of getrlimit to get
 *   file descriptor limit.
 *
 * OPT_GETHOSTNAME - use gethostname() instead of uname() to get host name
 *
 * OPT_SYSINFO - use sysinfo() instead of gethostid() or uname() to get host id
 *
 * OPT_CPP_PATH - string, path to cpp binary so tt_type_comp can invoke it
 *
 * OPT_CPP_OPTIONS - string, options to pass to cpp. For example, on
 *   some platforms -B is used so C++ comments are processed by cpp.
 *
 * OPT_SVR4_GETMNTENT - set to use new SVR4 flavor of getmntent.
 *
 * OPT_LOCKF_MNTTAB - set to hold a lockf lock on MNTTAB while doing
 *	getmntents.
 *
 * OPT_DEFINE_SIG_PF - define if SIG_PF (=type of pointer to signal
 *			handler function) isn't defined in system includes
 *
 * OPT_TAR_HAS_EXCLUDE_OPTION -- set if the system tar command has the X
 *   option for excluding files, which tttar uses to avoid extracting the
 *   extra file it bundles into the tar file. If this option is not available,
 *   the tttarfile file will appear after extracts, a small flaw but not a 
 *   fatal one.
 *
 * OPT_PING_TRIES - The number of times to rpc-ping ttsession to
 *   see if ttsession is alive.
 *
 * OPT_PING_SLEEP - How long to wait between each rpc-ping of ttsession.
 *
 * OPT_AUTOMOUNT_PATH_FIX - set for systems with autmounters that do
 *   not treat /tmp_mnt/nfs/... the same as /nfs/... in terms of which
 *   can tickle the automounter.  For many systems, a mount can only be
 *   tickled into existence using /nfs/...   This is bad since calls to
 *   say realpath() will hand back /tmp_mnt/nfs/... which is for all
 *   practical purposes temporary.
 *
 * OPT_SOLARIS_THREADS - Defined if the system supports SOLARIS style threads.
 *			(POSIX threads were not out at time of usage)
 *
 * OPT_XTHREADS - Defined if the system supports the Xthreads interfaces
 *			defined first in X11R6
 *
 * OPT_SOLARIS_THREADED_TRIES - How many times to try thr_create() if
 *			thr_create() returns EAGAIN.
 *
 * OPT_MAX_GET_SESSIONS - The max. number of session id's to return
 *                      in each call to _tt_get_all_sessions_1().
 *
 * OPT_HAS_CLNT_CREATE_TIMED - True if the OS has the clnt_create_timed()
 *			rpc function call.
 *
 * OPT_CLNT_CREATE_TIMEOUT - Time for clnt_create_timed() to wait
 *			before timing out.
 *
 * OPT_DO_AUTO_GARBAGE_COLLECT - Perform automatic garbage collection.
 *			Without this defined, garbage collection must
 *			be called from an rpc call. This is not should
 *			not be enabled until the rpc.ttdbserverd catches
 *			SIG{INT,TERM,...}, so that it will clean up the
 *			files prior to exiting.
 *
 * OPT_GARBAGE_IN_PARALLEL - If TRUE, enables garbage collection
 *			in a separate thread (or process). If FALSE
 *			perform garbage collection in the same
 *			thread (or process).
 */

/*
 * Defaults - #undef then #define them if a platform needs them changed.
 */
#define	OPT_PING_TRIES		5
#define OPT_PING_SLEEP		1
#define OPT_MAX_GET_SESSIONS	100
#define	OPT_GARBAGE_IN_PARALLEL	0	/* used as a const */

/* Allow -DXTHREADS to be specified by the Makefile. */
#ifdef XTHREADS
# define OPT_XTHREADS		1
#endif

#undef  OPT_DO_AUTO_GARBAGE_COLLECT

/* 
 * WARNING: This file isn't universally included. Adding a new option
 * entails ensuring that this file is properly included in all the files
 * that depend on that option. This is done to minimize re-compile time
 * every time options are changed.
 */
#if defined(__GNUG__)
/* g++ often cannot see an implicit cast to char */
# define GNU_STRCAST (char *)
#else
# define GNU_STRCAST
#endif

#undef OPT_CPP_PATH
#define OPT_CPP_PATH CPP_PROGRAM

#ifdef sun
#define OPT_CPP_OPTIONS "-B"
#else
#define OPT_BUG_RPCINTR
#define OPT_CPP_OPTIONS ""
#define OPT_AUTOMOUNT_PATH_FIX
#define OPT_DEFINE_SIG_PF
#endif

#define OPT_POSIX_SIGNAL
#define OPT_CATGETS
#define OPT_TAR_HAS_EXCLUDE_OPTION

#if defined(_AIX)
# undef OPT_BUG_RPCINTR
# undef OPT_TAR_HAS_EXCLUDE_OPTION
# define OPT_BUG_AIX
# define OPT_GETDTABLESIZE
/* # define OPT_XTHREADS   	1 */	/* Defaulted by -DXTHREADS */

#elif defined(sun)
# define OPT_TLI 
# define OPT_SECURE_RPC 
# define OPT_SYSINFO
# define OPT_SVR4_GETMNTENT
# define OPT_LOCKF_MNTENT

# define OPT_SOLARIS_THREADS		1
/* #  define OPT_XTHREADS   		1 */	/* Defaulted by -DXTHREADS */
# define OPT_GARBAGE_THREADS		1
# define OPT_SOLARIS_THREADED_TRIES	10

# define OPT_HAS_CLNT_CREATE_TIMED	1
# define OPT_CLNT_CREATE_TIMEOUT	30

/* Turn ON/OFF parallel threads for garbage collection */
# if defined(OPT_DO_AUTO_GARBAGE_COLLECT) && defined(OPT_SOLARIS_THREADS)
#  define OPT_GARBAGE_IN_PARALLEL	1
#  define OPT_GARBAGE_THREADS		1
# else
#  define OPT_GARBAGE_IN_PARALLEL	0	/* used as a const */
# endif

#elif defined(__linux__)
# define OPT_CONST_CORRECT

#elif defined(__OpenBSD__)

#elif defined(__FreeBSD__)
# define OPT_CONST_CORRECT

#elif defined(__NetBSD__)
# define OPT_CONST_CORRECT
# define HAS_STATVFS

#else
/* Unknown configuration, complain */
#warning "Your system is unknown to us, things might not build properly without editing tt_options.h"
#endif

#endif				/* _TT_OPTIONS_H */
