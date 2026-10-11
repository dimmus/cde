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
 * $TOG: privbuf.h /main/5 1999/10/15 17:23:22 mgreess $
 *
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1993,1994,1996 Hewlett-Packard Company.
 * (c) Copyright 1993,1994,1996 International Business Machines Corp.
 * (c) Copyright 1993,1994,1996 Sun Microsystems, Inc.
 * (c) Copyright 1993,1994,1996 Novell, Inc.
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */
#ifndef __PRIVBUF_H_
#define __PRIVBUF_H_

typedef void (*privbuf_func)(void *v);

#ifdef __PRIVATE_
/*
 * This is only for internal library routines needing dynamic buffers.
 *
 * PRIVBUF_DECLARE_ARRAY(NAME, TYPE) declares a growable array of TYPE
 * called NAME; PRIVBUF_IMPLEMENT_ARRAY(NAME, TYPE, BUMP) defines its
 * out-of-line functions once.  They are the C form of the C++
 * declare_array/implement_array classes this code used to be built
 * with, and keep their semantics:
 *
 *   NAME##_at(a, e)	&a[e]; an index at or past the end grows the array
 *			so that e is its last element (operator[])
 *   NAME##_end(a)	appends one element and returns its address
 *   NAME##_reset(a, l)	sets the length to l, growing the storage if needed
 *   NAME##_copy(d, s)	makes d a copy of s (operator=)
 *   NAME##_free(a)	releases the storage (the destructor)
 *
 * New elements are not initialised.  A NAME initialised with
 * PRIVBUF_INIT is an empty array.
 */
#include <stddef.h>
#include <stdlib.h>

#define PRIVBUF_INIT { 0, 0, NULL }

#define PRIVBUF_DECLARE_ARRAY(NAME, TYPE)				\
typedef struct NAME { long len; long max; TYPE *arr; } NAME;		\
extern TYPE *NAME##_grow(NAME *a, long elt);				\
extern void NAME##_copy(NAME *dst, const NAME *src);			\
static inline TYPE *NAME##_at(NAME *a, long e)				\
{									\
    return e < a->len ? &a->arr[e] : NAME##_grow(a, e);		\
}									\
static inline TYPE *NAME##_end(NAME *a)				\
{									\
    return NAME##_grow(a, a->len);					\
}									\
static inline void NAME##_reset(NAME *a, long l)			\
{									\
    NAME##_grow(a, l);							\
    a->len = l;								\
}									\
static inline void NAME##_free(NAME *a)				\
{									\
    free(a->arr);							\
    a->arr = NULL;							\
    a->len = a->max = 0;						\
}

#define PRIVBUF_IMPLEMENT_ARRAY(NAME, TYPE, BUMP)			\
TYPE *NAME##_grow(NAME *a, long elt)					\
{									\
    if (elt < 0)							\
	elt = 0;							\
    if (elt >= a->max)							\
    {									\
	long omax, nmax;						\
	TYPE *narr;							\
	if (a->max <= 0)						\
	    a->max = 1;							\
	omax = a->max;							\
	nmax = elt + (omax > (BUMP) ? (BUMP) : omax);			\
	narr = (TYPE *)realloc(a->arr, (size_t)nmax * sizeof(TYPE));	\
	if (narr == NULL)						\
	    abort();							\
	a->arr = narr;							\
	a->max = nmax;							\
    }									\
    if (elt >= a->len)							\
	a->len = elt + 1;						\
    return &a->arr[elt];						\
}									\
void NAME##_copy(NAME *dst, const NAME *src)				\
{									\
    long i;								\
    if (dst == src)							\
	return;								\
    if (src->len > dst->len)						\
	NAME##_grow(dst, src->len);					\
    dst->len = src->len;						\
    for (i = 0; i < dst->len; i++)					\
	dst->arr[i] = src->arr[i];					\
}

PRIVBUF_DECLARE_ARRAY(privbuf_charbuf, char)
PRIVBUF_DECLARE_ARRAY(privbuf_strvec, char *)

#undef __PRIVATE_
#endif /* __PRIVATE_ */

#ifdef __cplusplus
extern "C" {
#endif
extern void privbuf_release(void **var);
#ifdef __cplusplus
}
#endif

#endif /* __PRIVBUF_H_ */
