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
 * File:	buf.C $TOG: buf.C /main/5 1998/01/21 18:01:14 mgreess $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <codelibs/nl_hack.h>
#include "buf.h"

static void
start_token(_SHXbuf *b)
{
    *privbuf_strvec_at(&b->vec, b->vec.len) = _SHX_SLOT(b->buf.len);
    b->is_pattern = FALSE;
    b->_new_token = FALSE;
}

void
_SHXbuf_quote(_SHXbuf *b, Quote q)
{
    b->_quote ^= q;
    if (b->_new_token)
	start_token(b);
}

void
_SHXbuf_append_ch(_SHXbuf *b, int const ch, char flag)
{
    flag |= b->_quote;

    if (ch == '\0' && !b->_new_token)
	b->_new_token = TRUE;
    else if (b->_new_token)
	start_token(b);

    if (flag == NOQUOTE)
    {
	if (ch == '*' || ch == '?' || ch == '[')
	{
	    if (b->glob)
		b->is_pattern = TRUE;
	    else
		flag |= SINGLEQUOTE;
	}
    }

    if (ch <= 0xFF)
    {
	/* 8-bit char */
	*privbuf_charbuf_at(&b->flags, b->buf.len) = flag;
	*privbuf_charbuf_at(&b->buf, b->buf.len) = (unsigned char)ch;
    }
    else
    {
	/* multibyte char */
	long sz = b->buf.len;
	char *cp;
	int i;

	for (i=0; i<(int)MB_CUR_MAX; i++)
	    *privbuf_charbuf_at(&b->flags, sz+i) = flag;
	privbuf_charbuf_reset(&b->buf, sz + MB_CUR_MAX);
	cp = &b->buf.arr[sz];
	WCHAR(ch, cp);
    }

    /* expand token into filename(s) if appropriate */
    if (ch == '\0' && (b->is_pattern || b->completion))
	if (!(flag & EXPANDQUOTE))
	    _SHXbuf_filegen(b);
}

void
_SHXbuf_append_str(_SHXbuf *b, char const *cp, char flag)
{
    wchar_t __nlh_char[1];

    do
	_SHXbuf_append_ch(b, (int)CHARAT(cp), flag);
    while (CHARADV(cp) != '\0');
}

void
_SHXbuf_reset(_SHXbuf *b, boolean g, boolean comp)
{
    b->glob = (boolean) !!g;
    b->completion = (boolean ) !!comp;
    b->is_pattern = FALSE;
    b->_new_token = TRUE;
    b->_quote = NOQUOTE;
    privbuf_charbuf_reset(&b->buf, 0);
    privbuf_charbuf_reset(&b->flags, 0);
    privbuf_strvec_reset(&b->vec, 0);
}

/*
 * Convert the subscripts that are stored in vec[] into
 * char pointers.  The only legal operation on _SHXbuf after
 * calling vector is to call reset.
 */
char **
_SHXbuf_vector(_SHXbuf *b)
{
    long i;
    for (i = 0; i < b->vec.len; i++)
	b->vec.arr[i] = privbuf_charbuf_at(&b->buf, _SHX_OFFSET(b->vec.arr[i]));

    *privbuf_strvec_at(&b->vec, i) = NULL;

    return b->vec.arr;
}
