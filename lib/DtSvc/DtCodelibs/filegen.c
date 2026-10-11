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
 * File:	filegen.C $TOG: filegen.C /main/7 1999/10/14 15:05:25 mgreess $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#define X_INCLUDE_DIRENT_H
#define XOS_USE_XT_LOCKING
#include <X11/Xos_r.h>
#include <codelibs/stringx.h>
#include <codelibs/nl_hack.h>
#include "buf.h"
#include "DtSvcLock.h"

#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif

#ifdef XTHREADS
extern void XtProcessLock(void);
extern void XtProcessUnlock(void);
#endif

typedef struct _SHXcomponent
{
    boolean is_pattern;
    long offset;		/* subscript in path buffer */
    char *ptr;                  /* pointer into path buffer */
} _SHXcomponent;

PRIVBUF_DECLARE_ARRAY(_SHXcomponents, _SHXcomponent)
PRIVBUF_IMPLEMENT_ARRAY(_SHXcomponents, _SHXcomponent, 4)

/*
 * recursive routine to expand the wildcard path represented in stack into
 * all possible expansions.  The expansions are appended to b->vec.
 * filebuf is a scratch buffer passed in by the caller and is used to build
 * intermediate paths.  end is a pointer to the position in filebuf where
 * the calling routine left off.
 */
static void
expand(_SHXbuf *b, _SHXcomponents *stack,
	char *const filebuf, char *end, int compnum)
{
    _SHXcomponent *comp;
    DIR *dir;
    _Xreaddirparams dir_buf;
    struct dirent *ent;

    *end = '\0';

    if (compnum == stack->len)
	return;

    comp = _SHXcomponents_at(stack, compnum);

    /* double-slash? */
    if (comp->ptr[0] == '\0')
    {
	if (compnum + 1 == stack->len)
	    _SHXbuf_append_str(b, filebuf, EXPANDQUOTE);
	else
	{
	    *end++ = '/';
	    expand(b, stack, filebuf, end, compnum + 1);
	}
	return;
    }

    /* performance optimization:  if this path component
     * doesn't contain a wildcard, avoid doing an opendir() */
    if (!comp->is_pattern)
    {
	strcpy(end, comp->ptr);
	if (compnum + 1 == stack->len)
	{
	    /* last component, just see if the path really points to
	     * something */
	    if (access(filebuf, F_OK) != -1)
		_SHXbuf_append_str(b, filebuf, EXPANDQUOTE);
	}
	else
	{
	    /* intermediate directory just append this component and keep
	     * going */
	    char *end2 = strend(end);
	    *end2++ = '/';
	    expand(b, stack, filebuf, end2, compnum + 1);
	}
	return;
    }

    /* We have a wildcard component, open and scan its parent directory
     * and look for matches. */
    dir = opendir(filebuf[0] == '\0' ? "." : filebuf);
    if (dir == NULL)
	return;

    memset((char*) &dir_buf, 0, sizeof(_Xreaddirparams));
    while ((ent = _XReaddir(dir, dir_buf)) != NULL)
    {
	/* deleted file? */
	if (ent->d_ino == 0 || ent->d_name[0] == '\0')
	    continue;

	/* right name? */
	if (comp->is_pattern)
	{
	    wchar_t __nlh_char[1];

	    /* Must have explicit match for leading '.' */
	    if (CHARAT(ent->d_name) == '.' && CHARAT(comp->ptr) != '.')
		continue;

	    if (strwcmp(comp->ptr, ent->d_name) != 0)
		continue;
	}
	else if (strcmp(comp->ptr, ent->d_name) != 0)
	    continue;

	strcpy(end, ent->d_name);
	if (compnum + 1 == stack->len)
	    _SHXbuf_append_str(b, filebuf, EXPANDQUOTE);
	else
	{
	    char *end2 = end + strlen (ent->d_name);
	    *end2++ = '/';
	    expand(b, stack, filebuf, end2, compnum + 1);
	}
    }

    closedir(dir);
}


static char *bufptr;

static int
compar(const void *v1, const void *v2)
{
    int result;

    _DtSvcProcessLock();
    result = strcmp(&bufptr[_SHX_OFFSET(*(char *const *)v1)],
		    &bufptr[_SHX_OFFSET(*(char *const *)v2)]);
    _DtSvcProcessUnlock();
    return (result);
}

void
_SHXbuf_filegen(_SHXbuf *b)
{
    privbuf_charbuf path = PRIVBUF_INIT;
    _SHXcomponents stack = PRIVBUF_INIT;
    long vecstart = b->vec.len - 1;
    long bufstart;
    int bufpos;
    char *pathbuf;
    long i;
    char filebuf[MAXPATHLEN];

    if (vecstart < 0)
	return;

    bufstart = _SHX_OFFSET(*privbuf_strvec_at(&b->vec, vecstart));

    /* Parse the file path, breaking it up into individual components.
     * Each component is marked as being either a wildcard component
     * or not.  The wildcard components will have a '\' placed before
     * any quoted wildcard characters.  The non-wildcard components
     * will be left unchanged. */
    bufpos = (int) bufstart;
    while (bufpos < b->buf.len)
    {
	_SHXcomponent *comp = _SHXcomponents_at(&stack, stack.len);
	int startpos;
	int ch;

	comp->is_pattern = FALSE;
	comp->offset = path.len;
	comp->ptr = NULL;
	startpos = bufpos;

	do
	{
	    ch = *privbuf_charbuf_at(&b->buf, bufpos);
	    switch (ch)
	    {
	    case '/': 
		ch = '\0';
		break;
	    case '*': 
	    case '?': 
	    case '[': 
		if (*privbuf_charbuf_at(&b->flags, bufpos) == NOQUOTE)
		    comp->is_pattern = TRUE;
		else
		    *privbuf_charbuf_at(&path, path.len) = '\\';
		break;
	    }

	    *privbuf_charbuf_at(&path, path.len) = ch;
	    bufpos++;
	} while (ch != '\0');

	/* Add a '*' to the end of the last component if needed
	 * for completion. */
	if (bufpos >= b->buf.len &&	/* last component? */
	    bufpos > bufstart + 1 &&	/* non-null string? */
	    b->completion && !b->is_pattern)
	{
	    *privbuf_charbuf_at(&path, path.len - 1) = '*';
	    *privbuf_charbuf_at(&path, path.len) = '\0';
	    comp->is_pattern = TRUE;
	    break;
	}

	/* If it wasn't a pattern, remove all of the '\' characters
	 * that were added. */
	if (!comp->is_pattern)
	{
	    int len = bufpos - startpos - 1;
	    strncpy(privbuf_charbuf_at(&path, comp->offset),
		    privbuf_charbuf_at(&b->buf, startpos), len);
	    *privbuf_charbuf_at(&path, comp->offset + len) = '\0';
	}
    }

    /* Fill in the character pointer values for all of the components.
     * We couldn't do this in the first pass because path is a
     * dynamic array. */
    pathbuf = path.arr;
    for (i = 0; i < stack.len; i++)
	stack.arr[i].ptr = &pathbuf[stack.arr[i].offset];

    /* Remove the token that we just copied from the return vector
     * so that we can replace it with its expansion. */
    privbuf_strvec_reset(&b->vec, vecstart);

    expand(b, &stack, filebuf, filebuf, 0);

    if (b->vec.len == vecstart)	/* no matches? */
    {
	/* restore orig. token */
	*privbuf_strvec_at(&b->vec, vecstart) = _SHX_SLOT(bufstart);
	goto done;
    }

    /* alphabetize the expansion to make it look pretty like ksh does. */
    _DtSvcProcessLock();
    bufptr = b->buf.arr;
    qsort(privbuf_strvec_at(&b->vec, vecstart),
	    (unsigned int)(b->vec.len - vecstart), sizeof (char *), compar);

    /* Find the longest match if we are doing completion: */
    if (b->completion)
    {
	/* compare all entries to a copy of the first entry */
	snprintf(filebuf, sizeof(filebuf), "%s",
		 &bufptr[_SHX_OFFSET(*privbuf_strvec_at(&b->vec, 0))]);

	for (i = 1; i < b->vec.len; i++)
	{
	    char *ref = filebuf;
	    char *ptr = &bufptr[_SHX_OFFSET(b->vec.arr[i])];
	    while (*ref == *ptr && *ref != '\0' && *ptr != '\0')
		ref++, ptr++;

	    *ref = '\0';	/* shorten the reference copy */
	}

	/* Now store the best match as the first token.  We will
	 * have to shift the expansion vector down by one to
	 * make room. */
	for (i = b->vec.len; i > 0; --i)
	{
	    char *val = b->vec.arr[i - 1];
	    *privbuf_strvec_at(&b->vec, i) = val;
	}
	b->vec.arr[0] = _SHX_SLOT(b->buf.len);
	_SHXbuf_append_str(b, filebuf, EXPANDQUOTE);
	privbuf_strvec_reset(&b->vec, b->vec.len - 1); /* adjust for the append */
    }
    _DtSvcProcessUnlock();

done:
    privbuf_charbuf_free(&path);
    _SHXcomponents_free(&stack);
}
