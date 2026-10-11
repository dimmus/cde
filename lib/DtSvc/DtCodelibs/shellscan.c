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
 * $TOG: shellscan.C /main/9 1999/10/14 15:05:42 mgreess $
 *
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1993,1994,1996 Hewlett-Packard Company.
 * (c) Copyright 1993,1994,1996 International Business Machines Corp.
 * (c) Copyright 1993,1994,1996 Sun Microsystems, Inc.
 * (c) Copyright 1993,1994,1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define X_INCLUDE_PWD_H
#define XOS_USE_XT_LOCKING
#include <X11/Xos_r.h>
#include <codelibs/nl_hack.h>

# include <unistd.h>

#include "stringio.h"
#include "buf.h"
#include <codelibs/shellutils.h>

#include <codelibs/boolean.h>
#include <codelibs/stringx.h>
#include "DtSvcLock.h"

#ifdef XTHREADS
extern void XtProcessLock(void);
extern void XtProcessUnlock(void);
#endif

#define ISIDENT(CH) (isalnum(CH) || (CH) == '_')

static _SHXbuf *buf = NULL;
static const char *getvar(const char *var, char *);

/* Make this a global someday: */
static const char *(*shellvarfn)(const char *, char *) = getvar;

/*
 * Parse a sequence of the ksh meta-characters ;&|<> and whitespace
 * into a single ksh token.  Return a pointer to the token as a
 * string.  All whitespace characters are mapped to a single space
 * character.  If ch is not a meta-character, return a NULL pointer.
 */
static char *
parsemeta(int ch, _StringIO *in, char *ifs, unsigned opts, char *meta)
{
    if (ch == '\0')
	return " ";		/* whitespace */

    _DtSvcProcessLock();
    if (_SHXbuf_get_quote(buf) != NOQUOTE) {
	_DtSvcProcessUnlock();
	return NULL;		/* normal character */
    }

    if (!(opts & SHX_NOSPACE) && strchr(ifs, ch) != NULL) {
	_DtSvcProcessUnlock();
	return " ";		/* whitespace */
    }

    if (!(opts & SHX_NOMETA))
    {
	int len = 0;

	if (_SHXbuf_new_token(buf) && isascii(ch) && isdigit(ch))
	{
	    if (_StringIO_next(in) == '<' || _StringIO_next(in) == '>')
	    {
		meta[len++] = (char)ch;
		ch = _StringIO_get(in);
	    }
	}

	switch (meta[len++] = (char)ch, ch)
	{
	case ';': 
	case '&': 
	case '(': 
	case ')': 
	    meta[len++] = (_StringIO_next(in) == ch) ? _StringIO_get(in) : '\0';
	    meta[len] = '\0';
	    _DtSvcProcessUnlock();
	    return meta;
	case '|': 
	    meta[len++] = (_StringIO_next(in) == '|' ||
			   _StringIO_next(in) == '&') ?
		_StringIO_get(in) : '\0';
	    meta[len] = '\0';
	    _DtSvcProcessUnlock();
	    return meta;
	case '>': 
	case '<': 
		if (_StringIO_next(in) == ch || _StringIO_next(in) == '&')
		    meta[len++] = (char)_StringIO_get(in);
		meta[len] = '\0';
	    _DtSvcProcessUnlock();
	    return meta;
	}
    }

    _DtSvcProcessUnlock();
    return NULL; /* normal character */
}

/*
 * Takes the name of a variable, and looks up it's value.  Someday,
 * this will be replaceable by the user.
 */
static const char *
getvar(const char *name, char *buff)
{
    if (name[0] != '\0' && name[1] == '\0')
	switch (name[0])
	{
	case '$': 
	    sprintf(buff, "%d", getpid());
	    return buff;
	case '#':
	case '?':
	    return "0";
	}

    return getenv(name);
}

/*
 * Parse an environment variable name from the _StringIO stream,
 * and push its value into the _StringIO stream stack.
 */
static boolean
pushvar(_StringIO *in, char *buff)
{
    _StringIO tmp;
    privbuf_charbuf name = PRIVBUF_INIT;
    boolean ret = FALSE;
    int ch;

    _StringIO_init(&tmp);
    _StringIO_assign(&tmp, in);
    ch = _StringIO_get(&tmp);	/* get the first character after the $ */

    if (!isascii(ch))
	goto done;

    if (ch == '{')
	while ((ch = _StringIO_get(&tmp)) != '\0')
	{
	    /* ${foo!bar} form, grab everything inside {} as name */
	    if (ch == '\\')	/* Only \ does quoting inside ${} */
		    ch = _StringIO_get(&tmp);
	    else if (ch == '}')
		break;
	    *privbuf_charbuf_end(&name) = ch;
	}
    else if (ispunct(ch))
	switch (ch)		/* Special non-alnum shell variables */
	{
	case '#': 
	case '?': 
	case '$': 
	case '!': 
	case '-': 
	case '*': 
	case '@': 
	case '_':
	    *privbuf_charbuf_end(&name) = ch;
	    break;
	default: 
	    goto done;
	}
    else if (isdigit(ch))
	*privbuf_charbuf_end(&name) = ch;	/* single-digit variables */
    else if (ISIDENT(ch))
    {
	/* normal variable */
	do
	    *privbuf_charbuf_end(&name) = ch;
	while (isascii(ch = _StringIO_get(&tmp)) && ISIDENT(ch));
	_StringIO_unget(&tmp);
    }
    else
	goto done;

    *privbuf_charbuf_end(&name) = '\0';

    _StringIO_assign(in, &tmp);
    _StringIO_push(in, shellvarfn(name.arr, buff));
    ret = TRUE;

done:
    privbuf_charbuf_free(&name);
    _StringIO_free(&tmp);
    return ret;
}

static boolean
pushenv(_StringIO *in, char const *name)
{
    char *str = getenv(name);
    if (str == NULL || *str == '\0')
	return FALSE;
    else
    {
	_StringIO_push(in, str);
	return TRUE;
    }
}

/*
 * Parse ~, ~+, ~- or ~user and push the directory it names.  A home
 * directory from the password database is copied into dirbuf, which
 * must stay alive while the stream is read.
 */
static boolean
pushtilde(_StringIO *in, privbuf_charbuf *dirbuf)
{
    _StringIO tmp;
    int namelen = 0;
    privbuf_charbuf name = PRIVBUF_INIT;
    boolean ret = FALSE;
    char *str;
    int ch;

    _StringIO_init(&tmp);
    _StringIO_assign(&tmp, in);

    while ((ch = _StringIO_get(&tmp)) != '\0' && ch != '/')
	*privbuf_charbuf_at(&name, namelen++) = ch;
    *privbuf_charbuf_at(&name, namelen) = '\0';
    _StringIO_unget(&tmp);

    str = name.arr;
    switch (*str)
    {
    case '\0': 
	if (!pushenv(&tmp, "HOME"))
	    goto done;
	break;
    case '+': 
	if (!pushenv(&tmp, "PWD"))
	    goto done;
	break;
    case '-': 
	if (!pushenv(&tmp, "OLDPWD"))
	    goto done;
	break;
    default: 
	{
	  _Xgetpwparams pwd_buf;
	  struct passwd * pwd_ret;
	  size_t len;

	  memset((char*) &pwd_buf, 0, sizeof(_Xgetpwparams));
	  pwd_ret = _XGetpwnam(str, pwd_buf);

	  if (pwd_ret == NULL)
	    goto done;

	  /* pwd_ret may point into pwd_buf, which goes away on return */
	  len = strlen(pwd_ret->pw_dir);
	  privbuf_charbuf_reset(dirbuf, (long)len + 1);
	  memcpy(dirbuf->arr, pwd_ret->pw_dir, len + 1);
	  _StringIO_push(&tmp, dirbuf->arr);
	}
	break;
    }

    _StringIO_assign(in, &tmp);
    ret = TRUE;

done:
    privbuf_charbuf_free(&name);
    _StringIO_free(&tmp);
    return ret;
}

static void
pushgrave(_StringIO *in, const char endchar, boolean quotes,
	  privbuf_charbuf *result)
{
    int ch;
    char quote = NOQUOTE;
    privbuf_charbuf cmd = PRIVBUF_INIT;
    FILE *fp;
    long end;

    do
    {
	ch = _StringIO_get(in);

	if (quotes)
	    switch (ch)
	    {
	    case '"': 
		quote = DOUBLEQUOTE;
		continue;
	    case '\'': 
		if (quote == '"')
		    break;	/* not recognized inside of "" */
		do
		    *privbuf_charbuf_end(&cmd) = ch;
		while ((ch = _StringIO_get(in)) != '\'' && ch != '\0');
		*privbuf_charbuf_end(&cmd) = '\'';
		quote = NOQUOTE;
		continue;
	    case '\\': 
		*privbuf_charbuf_end(&cmd) = ch;
		ch = _StringIO_get(in);
		if (ch != '\0')
		    *privbuf_charbuf_end(&cmd) = ch;
		continue;
	    }

	if (ch == endchar)
	    ch = '\0';

	*privbuf_charbuf_end(&cmd) = ch;
    } while (ch != '\0');

    privbuf_charbuf_reset(result, 0);

    fp = popen(cmd.arr, "r");
    privbuf_charbuf_free(&cmd);
    if (fp == NULL)
	return;
    while ((ch = getc(fp)) != EOF)
	*privbuf_charbuf_end(result) = ch;
    pclose(fp);

    /* Remove trailing newline, if any */
    end = result->len - 1;
    if (end >= 0 && result->arr[end] == '\n')
	privbuf_charbuf_reset(result, end);

    *privbuf_charbuf_end(result) = '\0';
    _StringIO_push(in, result->arr);
}

char **
shellscan(char const *str, int *argc, unsigned opts)
{
    char *ifs;
    _StringIO in;
    int ch;
    char buff[10], meta_buff[4];
    privbuf_charbuf result = PRIVBUF_INIT;
    privbuf_charbuf homedir = PRIVBUF_INIT;
    char **vector;

    if (opts & SHX_COMPLETE)
	opts |= SHX_NOSPACE | SHX_NOMETA;

    ifs = getenv("IFS");

    if (ifs == NULL)
	ifs = " \t\n";

    _DtSvcProcessLock();
    if (buf == NULL)
    {
	buf = (_SHXbuf *)calloc(1, sizeof(_SHXbuf));
	if (buf == NULL)
	    abort();
    }
    _SHXbuf_reset(buf, (boolean)!(opts & SHX_NOGLOB),
		  (boolean)(opts & SHX_COMPLETE));

    _StringIO_init(&in);
    _StringIO_push(&in, str);

    do
    {
	ch = _StringIO_get(&in);

	/* Don't recognize special characters if this is a shell
	 * variable or command substitution. */
	if (!_StringIO_in_expansion(&in))
	{
	    /* Handle quoting rules, setting the flag array and
	     * quote variable appropriately. */
	    if (!(opts & SHX_NOQUOTES))
		switch (ch)
		{
		case '"': 
		    _SHXbuf_quote(buf, DOUBLEQUOTE);
		    continue;
		case '\'': 
		    if (_SHXbuf_get_quote(buf) == DOUBLEQUOTE)
			break;	/* not recognized inside of "" */
		    _SHXbuf_quote(buf, SINGLEQUOTE);
		    while ((ch = _StringIO_get(&in)) != '\'' && ch != '\0')
			_SHXbuf_append_ch(buf, ch, 0);
		    _SHXbuf_quote(buf, SINGLEQUOTE);
		    continue;
		case '\\': 
		    ch = _StringIO_get(&in);
		    if (ch == '\n') /* ignore \<newline> */
			continue;
		    if (ch == '\0')
		    {
			_SHXbuf_append_ch(buf, '\\', SINGLEQUOTE);
			break;
		    }
		    if (_SHXbuf_get_quote(buf) == NOQUOTE)
		    {
			_SHXbuf_append_ch(buf, ch, SINGLEQUOTE);
			continue;
		    }
		    else
		    {
			/* inside "", \ only quotes these 4 characters: */
			switch (ch)
			{
			case '$': 
			case '\\': 
			case '`': 
			case '"': 
			    _SHXbuf_append_ch(buf, ch, SINGLEQUOTE);
			    continue;
			default: 
			    /* treat the \ and the following char normally */
			    _SHXbuf_append_ch(buf, '\\', 0);
			    break;
			}
		    }
		    break;
		}

	    if (!(opts & SHX_NOCMD))
		switch (ch)
		{
		case '`': 
		    pushgrave(&in, '`', (boolean)!(opts & SHX_NOQUOTES),
			      &result);
		    continue;
		case '$': 
		    if (_StringIO_next(&in) != '(')
			break;
		    _StringIO_get(&in);	/* skip the '(' */
		    pushgrave(&in, ')', (boolean)!(opts & SHX_NOQUOTES),
			      &result);
		    continue;
		}

	    if (ch == '~' && _SHXbuf_new_token(buf) &&
		_SHXbuf_get_quote(buf) == NOQUOTE &&
		!(opts & SHX_NOTILDE))
	    {
		if (pushtilde(&in, &homedir))
		    continue;
		_SHXbuf_append_ch(buf, '~', 0);
		continue;
	    }

	    if (ch == '$' && !(opts & SHX_NOVARS))
	    {
		if (pushvar(&in, buff))
		    continue;
		_SHXbuf_append_ch(buf, '$', 0);
		continue;
	    }
	}

	/* If the next item is an unquoted whitespace character or
	 * metacharacter token, terminate the current token.  The NUL
	 * character is considered to be whitespace. */
	{
	    int curr_opts = opts;
	    char *meta;

	    if (_StringIO_in_expansion(&in))
		curr_opts |= SHX_NOMETA;

	    meta = parsemeta(ch, &in, ifs, curr_opts, meta_buff);

	    if (meta != NULL)	/* is it a meta-character? */
	    {
		/* Terminate current token, if any */
		if (!_SHXbuf_new_token(buf))
		    _SHXbuf_append_ch(buf, '\0', 0);
		if (*meta == ' ')   /* whitespace */
		{
		    /* ignore contiguous whitespace chars */
		    if (_SHXbuf_new_token(buf))
			continue;
		}
		else		/* append the metachar token */
		    _SHXbuf_append_str(buf, meta, 0);
		continue;
	    }
	}

	_SHXbuf_append_ch(buf, ch, 0);
    } while (ch != '\0');

    if (argc != NULL)
	*argc = _SHXbuf_ntokens(buf);

    vector = _SHXbuf_vector(buf);
    _DtSvcProcessUnlock();

    _StringIO_free(&in);
    privbuf_charbuf_free(&result);
    privbuf_charbuf_free(&homedir);
    return vector;
}
