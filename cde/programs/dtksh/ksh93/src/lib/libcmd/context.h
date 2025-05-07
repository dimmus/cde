/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
*             Copyright (c) 2025 Contributors to ksh 93u+m             *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 2.0                  *
*                                                                      *
*                A copy of the License is available at                 *
*      https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html      *
*         (with md5 checksum 84283fa8859daf213bdda5a9f8d1be1d)         *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                  Martijn Dekker <martijn@inlv.org>                   *
*                                                                      *
***********************************************************************/
#ifndef _CONTEXT_H
#define _CONTEXT_H		1

#include <ast.h>

typedef struct Context_line_s
{
	char*		data;
	size_t		size;
	uintmax_t	line;
#ifdef _CONTEXT_LINE_PRIVATE_
	_CONTEXT_LINE_PRIVATE_
#endif
} Context_line_t;

typedef int (*Context_list_f)(Context_line_t*, int, int, void*);

typedef struct Context_s
{
	void*		handle;
#ifdef _CONTEXT_PRIVATE_
	_CONTEXT_PRIVATE_
#endif
} Context_t;

extern Context_t*	context_open(Sfio_t*, size_t, size_t, Context_list_f, void*);
extern Context_line_t*	context_line(Context_t*);
extern int		context_show(Context_t*);
extern int		context_close(Context_t*);

#endif
