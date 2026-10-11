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
 * File:	stringio.h $XConsortium: stringio.h /main/3 1995/10/26 16:10:00 rswiston $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#ifndef _DtSvc_stringio_h
#define _DtSvc_stringio_h

#include <codelibs/boolean.h>
#define __PRIVATE_
#include <codelibs/privbuf.h>

/*
 * A stack of string pointers that are read one character at a time:
 * shellscan() pushes the value of a variable or command substitution
 * and reads it before going on with the rest of the string.
 */
typedef struct _StringIO
{
    privbuf_strvec ptr;
    int curr;
    int old_curr;
    char const *old_ccp;
} _StringIO;

extern int _StringIO_doit(_StringIO *in, int commit);

static inline void
_StringIO_init(_StringIO *in)
{
    in->ptr.len = in->ptr.max = 0;
    in->ptr.arr = NULL;
    in->curr = in->old_curr = -1;
    in->old_ccp = 0;
}

static inline void
_StringIO_push(_StringIO *in, char const *ccp)
{
    *privbuf_strvec_at(&in->ptr, ++in->curr) = (char*)ccp;
}

static inline void
_StringIO_unget(_StringIO *in)
{
    if (in->curr >= 0)
	*privbuf_strvec_at(&in->ptr, in->curr) = (char*)in->old_ccp;
    in->curr = in->old_curr;
}

/* Return the next character and move past it. */
static inline int
_StringIO_get(_StringIO *in)
{
    return _StringIO_doit(in, 1);
}

/* Return the next character without moving past it. */
static inline int
_StringIO_next(_StringIO *in)
{
    return _StringIO_doit(in, 0);
}

static inline boolean
_StringIO_in_expansion(_StringIO *in)
{
    return ((boolean)(in->curr > 0));
}

/* dst = src (a copy of the stack, as the C++ assignment made). */
static inline void
_StringIO_assign(_StringIO *dst, const _StringIO *src)
{
    privbuf_strvec_copy(&dst->ptr, &src->ptr);
    dst->curr = src->curr;
    dst->old_curr = src->old_curr;
    dst->old_ccp = src->old_ccp;
}

static inline void
_StringIO_free(_StringIO *in)
{
    privbuf_strvec_free(&in->ptr);
}

#endif /* _DtSvc_stringio_h */
