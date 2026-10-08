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
 *+SNOTICE
 *
 *	$TOG: EUSCompat.h /main/4 1998/04/03 17:11:57 mgreess $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef _EUSCOMPAT_H
#define _EUSCOMPAT_H

#ifdef __cplusplus
extern "C" {
#endif


/*
** System V R4 based systems define the stuff we need in
** sys/types.h. Include that and then we are done.
*/
#include <sys/types.h>

#if defined(CSRG_BASED) || defined(__linux__)
# include <sys/socket.h>
#endif

#if defined(sun) && defined(_XOPEN_SOURCE)
#ifndef B_TRUE
#define B_TRUE _B_TRUE
#endif
#ifndef B_FALSE
#define B_FALSE _B_FALSE
#endif
#endif   /* sun && _XOPEN_SOURCE */

#if defined(__linux__) || defined(CSRG_BASED)
#include <string.h>  /* memset for libcsa and others */
typedef enum {B_FALSE, B_TRUE} boolean_t;
#define MAXNAMELEN      	256
#endif


/*
** AIX defines
*/
#if defined(AIX)

#ifndef KERNEL
#define KERNEL
#endif

#ifndef _BSD_INCLUDES
#define _BSD_INCLUDES
#endif

#include <sys/types.h>

#define _SC_PAGESIZE	_SC_PAGE_SIZE

#ifndef MAXPATHLEN
#include <sys/param.h>
#endif

#define MAXNAMELEN      256

#ifndef _POWER
typedef enum {B_FALSE, B_TRUE} boolean_t;
#else /* _POWER */
#ifndef B_FALSE
#define B_FALSE 0
#endif
#ifndef B_TRUE
#define B_TRUE 1
#endif
#endif /* _POWER */

#undef BIG_ENDIAN

#endif /* AIX */

#ifdef __cplusplus
}
#endif


#endif
