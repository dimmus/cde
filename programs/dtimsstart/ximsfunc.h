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
/* @(#)$XConsortium: ximsfunc.h /main/2 1996/05/07 14:02:55 drk $ */

#ifndef	_XIMSFUNC_H_
#define	_XIMSFUNC_H_	1


    /* External Functions */

	/* main.c */
extern	void	sigExit(int sig);
extern	void	Exit(int err_code);
extern	int	NotifyErrCode(int err_code);
extern	int	ErrFilePathNeeded(ximsError error);
extern	int	InWaitingState(void);
extern	void	ximsMain(void);
extern	void	ximsFinish(void);
extern	char	*xims_errmsg(int err_num, void *arg1, void *arg2, void *arg3);

	/* env.c */
extern	int	set_cmd_env(void);
extern	int	get_user_environ(void);
extern	int	expand_string(char *in_str, char *out_str, int out_len, ImsConf *ims);
extern	int	make_new_environ(OutEnv *oenv, UserSelection *sel);
extern	int	put_new_environ(OutEnv *oenv);
extern	int	set_remote_env(char *ptr, char *env_pass);
//extern	char	*xhp_xinput_name(/* locale */);
//extern	char	*get_real_locale(/* locale, aliases */);

	/* select.c */
extern	void	ximsSelect(void);
extern	int	update_user_selection(UserSelection *sel, ImsList *list, int idx, char *host, int host_type);
extern	int	get_ims_idx(ImsList *list, char *name);
extern	int	set_ims_status(ImsEnt *ent);
extern	int	get_ims_list(ImsList **listp, char *locale, int fill_ent);

	/* start.c */
extern	void	ximsStart(void);
extern	void	ximsWait(void);
extern	void	ximsWaitDone(void);
extern	int	is_waiting(void);
extern	void	set_sig_chld(int enable);
extern	int	im_mod_available(RunEnv *renv);
extern	int	next_wait_interval(int restart);
extern	int	mk_ims_option(char *ptr, UserSelection *sel);
extern	int	load_resources(void);
extern	int	restore_resources(void);

	/* remote.c */
extern	int	put_remote_conf(char *locale, char *ims_name);
extern	int	get_remote_conf(ImsList **listp, char *hostname, char *locale, char *ims_name);
extern	int	exec_remote_ims(UserSelection *sel);
extern	int	check_hostname(char *hostname);
extern	int	set_remote_confdata(char *confbuf, int conflen);
extern	int	read_remote_confdata(char **confbuf, int *conflen);
extern	int	get_window_status(void);
extern	int	change_window_status(int status);
extern	int	set_window_data(int ac, char **av);
extern	int	get_window_data(int *acp, char ***avp);

	/* action.c */
extern	int	invoke_action(char *action, char *host);
extern	void	send_dtims_msg(int msg,int errcode);
extern	void	dtims_msg_proc(Widget w, XtPointer cd, XEvent *event, Boolean *continue_dispatch);

	/* file.c */
extern	int	create_xims_dir(void);
extern	int	init_log_file(char *org_path, int check_size);
extern	int	set_errorlog(char *path);
extern	int	read_cmd_conf(void);
extern	int	expand_cmd_conf(void);
extern	int	read_imsconf(ImsConf *conf, char *ims_name, char *ims_fname);
extern	int	check_ims_conf(ImsConf *ims, char *ims_name);
extern	int	read_localeconf(ImsList *list, char *locale_name);
extern	int	read_user_selection(FileSel **fselp, char *locale_name);
extern	int	read_selection_file(FileSel *fsel, FILE *fp);
extern	int	save_user_selection(UserSelection *sel, char *locale_name);
extern	int	get_select_mode(void);
extern	int	set_select_mode(int cur_mode, int new_mode);
extern	int	parse_protolist(char *valp);
extern	int	default_protocol(ImsConf *conf);

	/* win.c */
extern	int	open_display(void);
extern	void	close_display(void);
extern	int	window_env_ok(void);
extern	int	init_window_env(void);
extern	void	end_window_env(void);
extern	int	clear_cmd_property(Window win);
extern	int	save_RM(void);
extern	int	merge_RM(char *res1, char *res2);
extern	int	restore_RM(void);
extern	int	start_selection_window(void);
extern	int	start_mode_window(int cur_mode);
extern	void	stop_help(void);
extern	void	ximsHelp(int help_type);
extern	int	put_msg_win(int type, char *msg);
extern	void	xevent_loop(void);
extern	void	xt_start_waiting(void);
extern	void	xt_stop_waiting(void);

	/* util.c */
		/* string */
extern	char	*strcpyx(char *dest, char *src);
extern	char	*strcpy2(char *dest, char *src1, char *src2);
extern	char	*newstradded(char *src1, char *src2, char *src3);
extern	int	str_to_int(char *ptr, int *val);
extern	bool	str_to_bool(char *ptr, bool def_val);
extern	char	*trim_line(char *ptr);
extern	char	**parse_strlist(register char *ptr, char sep_ch);
extern	int	pack_strlist(char *ptr, char **listp, char sep_ch);
extern	void	free_strlist(char **pp);
extern	bool	parse_dpy_str(char *display_str, char **host, int *dnum, int *snum, int *dnet);
extern	char	*std_dpy_str(char *display_str, int *snum);
		/* file & dir */
extern	int	make_user_dir(char *path);
extern	char	*dirname(char *path);
extern	int	is_directory(char *path, int must_writable);
extern	int	is_regularfile(char *path);
extern	int	is_emptyfile(char *path);
extern	int	is_executable(char *path);
extern	int	is_writable(char *path);
extern	int	is_readable(char *path, int allow_empty);
		/* file read */
extern	int	start_tag_line(char *fname);
extern	int	read_tag_line(FILE *fp, char **tagp, char **valp);
		/* error messges */
extern	void	put_xims_msg(int msg_type, int err_num, void *arg1, void *arg2, void *arg3);
extern	void	put_xims_errmsg(int err_num, void *arg1, void *arg2, void *arg3);
extern	void	put_xims_warnmsg(int err_num, void *arg1, void *arg2, void *arg3);
extern	void	print_msg(char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5);
extern	int	put_xims_log(char *fmt, void *arg1, void *arg2, void *arg3);
		/* clear structure */
extern	void	clear_ImsConf(ImsConf *ims);
extern	void	clear_ImsEnt(ImsEnt *ent);
extern	void	clear_ImsList(ImsList *list);
extern	void	clear_FileSel(FileSel *fsel);
extern	void	clear_UserSelection(UserSelection *sel);
extern	void	clear_RunEnv(RunEnv *renv);
extern	void	clear_OutEnv(OutEnv *oenv);
extern	void	clear_UserEnv(void);
extern	void	clear_CmdConf(void);
extern	void	clear_WinEnv(void);
extern	void	clear_CmdOpt(void);
extern	void	clear_All(void);
# ifdef	DEBUG
		/* print structure */
extern	void	pr_FileSel(FileSel *fsel);
extern	void	pr_UserSelection(UserSelection *sel);
extern	void	pr_ImsConf(ImsConf *conf, char *ims_name);
extern	void	pr_ImsEnt(ImsEnt *ent, int idx);
extern	void	pr_ImsList(ImsList *list);
extern	void	pr_WinEnv(WinEnv *wenv);
extern	void	pr_RunEnv(RunEnv *renv);
extern	void	pr_UserEnv(void);
extern	void	pr_OutEnv(OutEnv *oenv);
extern	void	pr_CmdConf(void);
extern	void	pr_CmdOpt(void);
extern	void	pr_OpModeFlag(void);
extern	char	*StateName(void);
extern	char	*error_name(ximsError error);
extern	char	*proto_name(int proto_idx);
extern	char	*sig_name(int sig);
extern	void	pr_brk(char *msg);
# endif	/* DEBUG */


    /* Macros */
	/* memory */
#define	CLR(p, st)		memset((void *) p, 0, sizeof(st))
#define	COPY(d, s, n, st)	memcpy((void *)(d), (void *)(s), (n) * sizeof(st))

# ifdef	DEBUG_MEM
void	*_mp_;

#define	ALLOC(n, st)		(_mp_ = XtCalloc(n, sizeof(st)), \
    print_msg("ALLOC(%d, " #st "): _mp_=%p sz=%d\n", n, _mp_, sizeof(st)), (st *) _mp_)
#define	REALLOC(p, n, st)	( _mp = XtRealloc((char *) (p), (n) * sizeof(st)), \
    print_msg("REALLOC(%p, %d, " #st "): _mp_=%p\n", p, n, _mp_), (st *) _mp_)
#define	NEWSTR(s)		(_mp_ = (s) && *(s) ? XtNewString(s) : NULL, \
    print_msg("NEWSTR(%s): _mp_=%p\n", s, _mp_), (char *) _mp_)
#define	FREE(p)			(print_msg("FREE(%p)\n", p), XtFree((char *) p), (p) = 0)
#define	RENEWSTR(p, s)		(_mp_ = (s) && *(s) ? XtNewString(s) : NULL, \
    print_msg("RENEWSTR(" #p "): %p(%s) => %p(%s)\n", p, p, _mp_, s), \
    XtFree((char *) p), (p) = (char *) _mp_)
# else	/* DEBUG_MEM */
#define	ALLOC(n, st)		(st *) XtCalloc(n, sizeof(st))
#define	REALLOC(p, n, st)	(st *) XtRealloc((char *) (p), (n) * sizeof(st))
#define	NEWSTR(s)		((s) && *(s)) ? XtNewString(s) : NULL
#define	FREE(p)			XtFree((char *) p), (p) = 0
#define	RENEWSTR(p, s)		XtFree((char *) p), (p) = NEWSTR(s)
# endif	/* DEBUG_MEM */

#define	FREE_LIST(pp)		free_strlist(pp), (pp) = 0

	/* string */
#define	is_default_name(p)	(*(p) == NAME_DEFAULT_CHAR)
#define	to_upper_ch(c)		((c) & ~0x20)
#define	to_lower_ch(c)		((c) | 0x20)
#define	to_upper_str(p)		while (*(p)) 	*(p)++ &= ~0x20
#define	to_lower_str(p)		while (*(p)) 	*(p)++ |= 0x20
#define	is_comment_char(c)	((c) == COMMENT_CHAR || (c) == COMMENT_CHAR2)
#define	is_white(c)		((c) == ' ' || (c) == '\t' || (c) == '\n')
#define	skip_white(p)		while (is_white(*(p)))	(p)++
#define	next_field(p)	\
	for ((p)++; *(p) && !is_white(*(p)); (p)++) ; skip_white(p)
#define	cut_field(p)	\
	for ((p)++; *(p) && !is_white(*(p)); (p)++) ; \
	*(p)++ = 0; skip_white(p)

	/* misc */
#define Max(a, b)	((a) < (b) ? (b) : (a))
#define Min(a, b)	((a) > (b) ? (b) : (a))

	/* for DEBUG */
# ifdef	DEBUG
#define	DPR(msg)		if (DebugLvl >= 1)	print_msg msg
#define	DPR2(msg)		if (DebugLvl >= 2)	print_msg msg
#define	DPR3(msg)		if (DebugLvl >= 3)	print_msg msg
# else
#define	DPR(msg)
#define	DPR2(msg)
#define	DPR3(msg)
# endif	/* DEBUG */

#endif	/* _XIMSFUNC_H_ */
