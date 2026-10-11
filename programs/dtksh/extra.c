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
/* $XConsortium: extra.c /main/4 1995/11/01 15:54:55 rswiston $ */

/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc. */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF    */
/*	UNIX System Laboratories, Inc.			*/
/*	The copyright notice above does not evidence any       */
/*	actual or intended publication of such source code.    */


#include	"defs.h"
#include	"shell.h"
#include	"shnodes.h"
#include	"name.h"
#include	"stdio.h"
#include	"msgs.h"


void
env_set(
        char *var )
{
	(void)nv_open(var, sh.var_tree, NV_ASSIGN);
}

void
env_set_gbl(
        char *vareqval )
{
        env_set(vareqval);
}

char *
env_get(
        char *var )
{
	Namval_t *np;
	char *val;

	np = nv_search(var,sh.var_tree,0);
	if (np && (val = nv_getval(np)))
		return(val);
	return(NULL);
}


/*
 * Compiled command strings.
 *
 * Every callback, translation, event handler, work procedure, timer and
 * input handler runs its command string through ksh_eval(), and
 * sh_eval() lexes and parses the string again each time.  A string's
 * parse tree is kept instead (on a stack of its own, as ksh keeps a
 * function's body) and executed again, but only while everything the
 * parser consults is as it was when it was parsed:
 *
 *   - the options and the shell states the lexer and parser test
 *     (posix, braceexpand, keyword, noexec, verbose, noalias, ...),
 *   - the locale (multibyte lexing),
 *   - the aliases (names and values), which are expanded while parsing,
 *   - the number of functions and built-ins and the namespace, which
 *     decide whether a command word binds to a built-in and whether it
 *     is a declaration command,
 *   - the line number the string starts on (its commands carry line
 *     numbers for error messages and LINENO).
 *
 * Strings that may define functions (whose bodies get stacks of their
 * own), that have here-documents (kept in a temporary file) or that
 * start like a compiled script are not kept, and neither are strings
 * evaluated in a subshell or whose parse defined a function after all
 * (through an alias): they run as before.  A syntax error is reported
 * exactly as before, since a string that fails to parse is not kept.
 * Execution of the tree is that of sh_eval(..., 0).
 */

#define CMD_HASH_SIZE	256	/* buckets */
#define CMD_CACHE_MAX	512	/* kept strings */

/* what a parse depends on */
typedef struct
{
	Shopt_t		options;
	unsigned int	states;
	uint32_t	locale;
	unsigned long	aliases;
	Dt_t		*fun_tree, *bltin_tree;
	ssize_t		nfun, nbltin;
	Namval_t	*namespace;
	int		line_in, firstline_in;
} parse_context_t;

typedef struct cmd_tree
{
	struct cmd_tree	*next;		/* hash chain */
	char		*cmd;		/* the string */
	unsigned int	hash;
	Shnode_t	*tree;		/* its parse tree, on stk */
	Stk_t		*stk;
	parse_context_t	ctx;		/* when it was parsed */
	int		busy;		/* executions in progress */
	int		stale;		/* free when no longer busy */
} cmd_tree_t;

static cmd_tree_t *cmd_hash[CMD_HASH_SIZE];

/* The object count of a cdt dictionary: the head of cdt's private
 * Dtdata_t (libast/cdt/cdtlib.h, built with this ksh).  dtsize() would
 * rebalance an ordered dictionary on every call. */
struct dtdata_head
{
	unsigned int	lock;
	unsigned int	type;
	ssize_t		size;
};
#define DT_COUNT(d)	((d) ? ((struct dtdata_head *)(d)->data)->size : 0)
static int cmd_count;

/* the shell states sh_lex() and sh_parse() look at */
#define PARSE_STATES	(sh_state(SH_NOALIAS) | sh_state(SH_INTERACTIVE) | \
			 sh_state(SH_HISTORY) | sh_state(SH_PROFILE) | \
			 sh_state(SH_VERBOSE) | sh_state(SH_INIT))

static unsigned long
alias_signature(void)
{
	unsigned long h = 5381;
	Namval_t *np;
	const char *cp;

	if (!sh.alias_tree)
		return 0;
	for (np = (Namval_t *)dtfirst(sh.alias_tree); np;
	     np = (Namval_t *)dtnext(sh.alias_tree, np))
	{
		for (cp = np->nvname; cp && *cp; cp++)
			h = h * 33 + (unsigned char)*cp;
		h = h * 33 + '=';
		h = h * 33 + (np->nvflag & 0xff);
		if ((cp = nv_getval(np)))
			for (; *cp; cp++)
				h = h * 33 + (unsigned char)*cp;
		h = h * 33 + '\n';
	}
	return h;
}

static void
parse_context(parse_context_t *pc)
{
	memset(pc, 0, sizeof(*pc));
	pc->options = sh.options;
	pc->states = sh.st.states & PARSE_STATES;
	pc->locale = ast.locale.serial;
	pc->aliases = alias_signature();
	pc->fun_tree = sh.fun_tree;
	pc->bltin_tree = sh.bltin_tree;
	pc->nfun = DT_COUNT(sh.fun_tree);
	pc->nbltin = DT_COUNT(sh.bltin_tree);
#if SHOPT_NAMESPACE
	pc->namespace = sh.namespace;
#endif
	/* sh_parse() starts counting lines here */
	pc->line_in = error_info.line + sh.st.firstline;
	if (pc->line_in == 0)
		pc->line_in = 1;
	pc->firstline_in = sh.st.firstline;
}

static int
same_context(const parse_context_t *a, const parse_context_t *b)
{
	return memcmp(&a->options, &b->options, sizeof(Shopt_t)) == 0 &&
	       a->states == b->states &&
	       a->locale == b->locale &&
	       a->aliases == b->aliases &&
	       a->fun_tree == b->fun_tree &&
	       a->bltin_tree == b->bltin_tree &&
	       a->nfun == b->nfun &&
	       a->nbltin == b->nbltin &&
	       a->namespace == b->namespace &&
	       a->line_in == b->line_in &&
	       a->firstline_in == b->firstline_in;
}

static unsigned int
cmd_hash_string(const char *cmd)
{
	unsigned int h = 2166136261u;

	for (; *cmd; cmd++)
		h = (h ^ (unsigned char)*cmd) * 16777619u;
	return h;
}

static void
free_cmd_tree(cmd_tree_t *e)
{
	stkclose(e->stk);
	free(e->cmd);
	free(e);
}

/* unlink E from the table; it is freed now or when its last run ends */
static void
drop_cmd_tree(cmd_tree_t *e)
{
	cmd_tree_t **pp;

	for (pp = &cmd_hash[e->hash % CMD_HASH_SIZE]; *pp; pp = &(*pp)->next)
		if (*pp == e)
		{
			*pp = e->next;
			cmd_count--;
			break;
		}
	e->next = NULL;
	if (e->busy)
		e->stale = 1;
	else
		free_cmd_tree(e);
}

static void
flush_cmd_trees(void)
{
	int i;

	for (i = 0; i < CMD_HASH_SIZE; i++)
		while (cmd_hash[i])
			drop_cmd_tree(cmd_hash[i]);
}

/* may CMD's parse tree be kept at all? */
static int
cmd_keepable(const char *cmd)
{
	if (!cmd || !*cmd || *cmd == '\013')	/* ^K^S^H: a compiled script */
		return 0;
	if (sh.subshell || sh.binscript ||
	    sh_isoption(SH_VERBOSE) || sh_isoption(SH_NOEXEC))
		return 0;
	/* function definitions, here-documents */
	if (strstr(cmd, "function") || strstr(cmd, "()") || strstr(cmd, "<<"))
		return 0;
	return 1;
}

/*
 * sh_eval(sfopen(NULL, cmd, "s"), 0), with the parse tree taken from or
 * put into the table
 */
static int
cached_eval(char *cmd)
{
	cmd_tree_t *volatile e = NULL;
	Stk_t *volatile parse_stk = NULL;	/* tree not kept: freed after */
	Stk_t *volatile saved_stk = NULL;	/* set while parsing */
	Sfio_t *volatile iop = NULL;
	struct slnod *saveslp = sh.st.staklist;
	struct checkpt *pp = (struct checkpt *)sh.jmplist;
	struct checkpt *buffp = stkalloc(sh.stk, sizeof(struct checkpt));
	int binscript = sh.binscript;
	char comsub = sh.comsub;
	int jmpval;
	parse_context_t pc;
	unsigned int hash = cmd_hash_string(cmd);
	Shnode_t *t;
	cmd_tree_t *c;

	parse_context(&pc);
	for (c = cmd_hash[hash % CMD_HASH_SIZE]; c; c = c->next)
		if (c->hash == hash && strcmp(c->cmd, cmd) == 0)
			break;
	if (c && !same_context(&c->ctx, &pc))
	{
		drop_cmd_tree(c);
		c = NULL;
	}

	sh.binscript = 0;
	sh.comsub = 0;
	sh_pushcontext(buffp, SH_JMPEVAL);
	buffp->olist = pp->olist;
	jmpval = sigsetjmp(buffp->buff, 0);
	if (jmpval == 0)
	{
		if (c)
		{
			/* what sh_parse() would have done around the tree;
			   it leaves sh.inlineno and sh.st.firstline as it
			   found them */
			e = c;
			e->busy++;
			t = e->tree;
			sh.st.staklist = 0;
			stkseek(sh.stk, 0);
		}
		else
		{
			iop = sfopen(NULL, cmd, "s");
			parse_stk = stkopen(STK_SMALL);
			saved_stk = sh.stk;
			sh.stk = parse_stk;
			t = (Shnode_t *)sh_parse(iop, SH_NL);
			sh.stk = saved_stk;
			saved_stk = NULL;
			stkseek(sh.stk, 0);
			sfclose(iop);
			iop = NULL;
			if (t && !sh.st.staklist &&
			    (c = calloc(1, sizeof(*c))) &&
			    (c->cmd = strdup(cmd)))
			{
				if (cmd_count >= CMD_CACHE_MAX)
					flush_cmd_trees();
				c->hash = hash;
				c->tree = t;
				c->stk = parse_stk;
				parse_stk = NULL;
				c->ctx = pc;
				c->next = cmd_hash[hash % CMD_HASH_SIZE];
				cmd_hash[hash % CMD_HASH_SIZE] = c;
				cmd_count++;
				e = c;
				e->busy++;
			}
			else if (c)
			{
				free(c);
				c = NULL;
			}
		}
		if (!sh_isoption(SH_VERBOSE))
			sh_offstate(SH_VERBOSE);
		sh_exec(t, sh_isstate(SH_ERREXIT) | sh_isstate(SH_NOFORK));
	}
	if (saved_stk)
		sh.stk = saved_stk;	/* the parse failed */
	if (iop)
		sfclose(iop);
	sh_popcontext(buffp);
	sh.binscript = binscript;
	sh.comsub = comsub;
	sh_freeup();
	sh.st.staklist = saveslp;
	if (e && --e->busy == 0 && e->stale)
		free_cmd_tree(e);
	if (parse_stk)
		stkclose(parse_stk);
	if (jmpval > SH_JMPEVAL)
		siglongjmp(*sh.jmplist, jmpval);
	return sh.exitval;
}

int
ksh_eval(
        char *cmd )
{
        if (cmd_keepable(cmd))
                cached_eval(cmd);
        else
                sh_eval(sfopen(NIL(Sfio_t*),cmd,"s"),0);
        sfsync(sh.outpool);
	return(sh.exitval);
}

void
env_set_var(
        char *var,
        char *val )
{
	int len;
	char tmp[512];
	char *set = &tmp[0];

	if ((len = strlen(var) + strlen(val?val:"") + 2) > sizeof(tmp)) /* 11/06 CHANGED */
		set = malloc(len);
	strcpy(set, var);
	strcat(set, "=");
	strcat(set, val?val:""); 			/* 11/06 CHANGED */
	env_set(set);
	if (set != &tmp[0])
		free(set);
}

void
env_blank(
        char *var )
{
	env_set_var(var, "");
}

void
printerr(
        char *cmd,
        char *msg1,
        char *msg2 )
{
	if (msg1 == NULL)
		msg1 = "";
	if (msg2 == NULL)
		msg2 = "";
        if (cmd && (strlen(cmd) > 0))
	   printf( "%s: %s %s\n", cmd, msg1, msg2);
        else
	   printf( "%s %s\n", msg1, msg2);
}

void
printerrf(
        char *cmd,
        char *fmt,
        char *arg0,
        char *arg1,
        char *arg2,
        char *arg3,
        char *arg4,
        char *arg5,
        char *arg6,
        char *arg7 )
{
	char buf[2048];
	if (arg0 == NULL)
		arg0 = "";
	if (arg1 == NULL)
		arg1 = "";
	if (arg2 == NULL)
		arg2 = "";
	if (arg3 == NULL)
		arg3 = "";
	if (arg4 == NULL)
		arg4 = "";
	if (arg5 == NULL)
		arg5 = "";
	if (arg6 == NULL)
		arg6 = "";
	if (arg7 == NULL)
		arg7 = "";

	sprintf(buf, fmt, arg0, arg1, arg2, arg3,arg4, arg5, arg6, arg7);
        if (cmd && (strlen(cmd) > 0))
	   printf("%s: %s\n", cmd, buf);
        else
	   printf("%s\n", buf);
}

/****************************************************************************
 *
 * The following two functions are ugly, but necessary.  Ksh reserves file
 * descriptors 0 - 9 for use by shell scripts, and has intimate knowledge
 * of how and when they were opened.  Unfortunately, certain dtksh functions
 * (XtInitialize, catopen, ttdt_open, _DtActionInvoke, others) open file
 * descriptors which are not known to ksh.  We can't let these file
 * descriptors fall in the 0 - 9 range, because we can't afford to have
 * the shell script overriding our file descriptors.  Therefore, any of
 * our commands which open files must first lock our file descriptors 0 - 9,
 * thus forcing the command to get a file descriptor out of the shell's
 * range.  After the command has opened its file descriptor, it then needs
 * to unlock file descriptors 0 - 9, so that the shell script will have
 * access to them again.
 *
 **************************************************************************/


/*
 * Return a list of the file descriptors we had to open, to lock out file
 * descriptors 0 - 9; this list should be freed (and the file descriptors
 * closed) by calling UnlockkshFileDescriptors().
 */

int *LockKshFileDescriptors(void)
{
   int * fdList;
   int i;
   int fd, newfd;

   fdList = (int *)malloc(sizeof(int) * 10);
   for (i = 0; i < 10; i++)
      fdList[i] = -1;

   if ((fd = open("/dev/null", O_RDONLY)) >= 0)
   {
      if (fd < 10)
      {
         fdList[0] = fd;
         for (i = 1; i < 10; i++)
         {
            if ((newfd = dup(fd)) < 10)
               fdList[i] = newfd;
            else
            {
	       close(newfd);
               break;
            }
         }
      }
      else
         close(fd);
   }

   return(fdList);
}

void UnlockKshFileDescriptors(int *fdList)
{
   int i;

   for (i = 0; i < 10; i++)
   {
      if (fdList[i] != (-1))
         close(fdList[i]);
   }

   free((char *)fdList);
}
