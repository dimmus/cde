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
 * File:	buf.h $XConsortium: buf.h /main/3 1995/10/26 16:07:20 rswiston $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#ifndef _DtSvc_buf_h
#define _DtSvc_buf_h

#include <stdint.h>
#include <codelibs/boolean.h>
#define __PRIVATE_
#include <codelibs/privbuf.h>

#define NOQUOTE		0
#define SINGLEQUOTE	1
#define DOUBLEQUOTE	2
#define EXPANDQUOTE	4

typedef unsigned char Quote;

/*
 * The token buffer of shellscan().  vec holds the start of each token
 * as an offset into buf until _SHXbuf_vector() turns the offsets into
 * pointers.
 */
typedef struct _SHXbuf
{
    boolean glob;
    boolean completion;
    boolean is_pattern;
    Quote _quote;
    boolean _new_token;
    privbuf_charbuf buf;
    privbuf_charbuf flags;
    privbuf_strvec vec;
} _SHXbuf;

extern void _SHXbuf_reset(_SHXbuf *b, boolean glob, boolean completion);
extern void _SHXbuf_append_ch(_SHXbuf *b, int const ch, char flag);
extern void _SHXbuf_append_str(_SHXbuf *b, char const *cp, char flag);
extern void _SHXbuf_quote(_SHXbuf *b, Quote q);
extern char **_SHXbuf_vector(_SHXbuf *b);
extern void _SHXbuf_filegen(_SHXbuf *b);	/* filegen.c */

static inline int
_SHXbuf_ntokens(_SHXbuf *b)
{
    return (int)b->vec.len;
}

static inline boolean
_SHXbuf_new_token(_SHXbuf *b)
{
    return b->_new_token;
}

static inline Quote
_SHXbuf_get_quote(_SHXbuf *b)
{
    return b->_quote;
}

/* Token start offsets are kept in the char * slots of vec. */
#define _SHX_OFFSET(p)	((long)(intptr_t)(p))
#define _SHX_SLOT(off)	((char *)(intptr_t)(off))

#ifndef NULL
#define NULL 0
#endif

#endif /* _DtSvc_buf_h */
