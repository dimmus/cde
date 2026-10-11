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
/* $TOG: Main.c /main/5 1999/09/20 14:08:17 mgreess $ */
/*
cc -g -o Main Main.c -I/usr/dt/include -I/usr/openwin/include -R/export2/build/SunOS/lib1/DtSvc:/usr/dt/lib:/usr/openwin/lib -L/export2/build/SunOS/lib1/DtSvc -L/usr/dt/lib -L/usr/openwin/lib -lDtSvc -lXt
 */
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <Tt/tt_c.h>
#include <X11/Intrinsic.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Dt/Dt.h>
#include <Dt/Dts.h>
#include <Dt/DtsMM.h>
#include <Dt/UserMsg.h>
#include <Dt/EnvControlP.h>
#include <bms/XeUserMsg.h>

static void DbCacheCreateError(Widget toplevel);
static void DbCacheCreateErrorCallback(Widget, XtPointer, XtPointer);
static void DieFromToolTalkError(char*, Tt_status);
static int InitCache(void);

#define ApplicationClass "dtdtscache"
#define RebuildMsg 	 "DtDtsCache"

int             build_db = 1;

int
main(int argc, char **argv)
{
	XtAppContext    appContext;
	Widget          toplevel;
	int		more = 1;
	Tt_message	msg;
	Tt_status	status;
	int		count = 0;
	int		slp = 0;
	int		init_flag;
	char		*locale = setlocale(LC_ALL, "");


        if(!locale)
        {
                perror("setlocale");
        }
        _DtEnvControl(DT_ENV_SET);

	if(argc > 1 && strcmp(argv[1], "-init") == 0)
	{
		init_flag = 1;
	}
	else
	{
		init_flag = 0;
	}

	if(init_flag)
	{
		/*
		 * Building the cache needs neither the X display nor the
		 * toolkit: skip XtAppInitialize() and DtAppInitialize().
		 */
		DtProgName = argv[0];
		if (!InitCache())
		{
			/*
			 * Log an error message.  Cannot post a dialog because
			 * there is a good chance system is starting up.
			 */
			_DtSimpleError(ApplicationClass, DtFatalError, NULL,
			  "Couldn't create the Desktop Action/DataTypes Database.\n\
Check disk space and/or permissions.\n");
		}
		exit(0);
	}

	if(!init_flag)
	{
		status = tt_ptr_error(tt_open());
		DieFromToolTalkError(
			"Could not connect to ToolTalk:\n%s\nExiting",
			status);
	}
	if(!init_flag)
	{
		status = tt_ptype_declare("SDTDTSCACHE");
		DieFromToolTalkError(
			"Could not declare SDTDTSCACHE ptype:\n%s\nExiting",
			status);
	}

	toplevel = XtAppInitialize(&appContext, ApplicationClass, NULL, 0,
				   &argc, argv, NULL, NULL, 0);

	if (DtAppInitialize(appContext, XtDisplay(toplevel), toplevel, argv[0],
			    ApplicationClass) == False)
	{
		_DtSimpleError(ApplicationClass,
				DtFatalError,
				NULL ,
				"%s: Couldn't initialize Dt\n",
				argv[0]);
	}

	while(!init_flag && more)
	{
		msg = tt_message_receive();
		if(msg != 0)
		{
			char	*op = tt_message_op(msg);

			if(strcmp(op, RebuildMsg) != 0)
			{
				tt_message_reject(msg);
			}
			else
			{
				if (!_DtDtsMMInit(1))
				{
				    tt_message_reply(msg);
				    DbCacheCreateError(toplevel);
				    /* NOTREACHED */
				}

				count = 0;
				slp = 0;
				tt_message_reply(msg);
/*
				m = tt_message_create();
				tt_message_scope_set(m, TT_SESSION);
				tt_message_class_set(m, TT_NOTICE);
				tt_message_op_set(m, "DtTypes_Reloaded");
				tt_message_address_set(m, TT_PROCEDURE);
				tt_message_send(m);
*/
			}
		}
		else
		{
			count++;
		}
		if(count >= 10)
		{
			if(slp)
			{
				more = 0;
				continue;
			}
			sleep(30);
			slp = 1;
		}
	}

	exit(0);
}

/*
 * dtdbcache -init, run at login: makes sure that the cache file shared
 * by the clients of the display is up to date.
 *
 * It used to rebuild the file every time (and the login scripts
 * removed it at every logout).  Now a cache file that is ours, of this
 * format and built for the database search path we have is kept if
 * libDtSvc finds it up to date (it compares the stamps of the database
 * directories and files it was built from), and rebuilt in place if
 * not.  Any other file (missing, of another format or search path, or
 * not ours) is replaced, as before.
 */
static int
InitCache(void)
{
	char		*display = getenv("DISPLAY");
	char		*name;
	int		fd;
	int		usable = 0;
	DtDtsMMHeader	hdr;

	if(!display || !*display || !(name = _DtDtsMMCacheName(1)))
	{
		return(_DtDtsMMInit(1));
	}

	if((fd = open(name, O_RDONLY | O_NOFOLLOW)) != -1)
	{
		if(pread(fd, &hdr, sizeof(hdr), 0) == (ssize_t)sizeof(hdr) &&
		   hdr.magic == DTDTSMM_MAGIC &&
		   hdr.version == DTDTSMM_VERSION)
		{
			/*
			 * Maps this very file (and takes over fd), after
			 * checking that it is a plain file of ours, of this
			 * format and of the size the header says, so the
			 * search path read from it is that of this file.
			 */
			if(_DtDtsMMapFd(fd))
			{
				char		*searchpath = _DtDtsMMSearchPath();
				const char	*path =
					_DtDtsMMBosonToString(hdr.searchpath);

				usable = searchpath && path &&
					 strcmp(searchpath, path) == 0;
				XtFree(searchpath);
			}
		}
		else
		{
			close(fd);
		}
	}
	free(name);

	/* Validated, and rebuilt in place if stale; or replaced. */
	return(_DtDtsMMInit(usable ? 0 : 1));
}

static void
DbCacheCreateErrorCallback(Widget w, XtPointer userData, XtPointer callData)
{
    XtUnmanageChild(w);
    XFlush(XtDisplay(w));
    XmUpdateDisplay(w);
    _DtSimpleError(ApplicationClass, DtFatalError, NULL,
		   "Couldn't recreate the Desktop Action/DataTypes Database.\n\
Check disk space and/or permissions.\n");
}

/*
 * This function posts an error dialog informing the user that creation
 * of the Actions/DataTypes Database failed.
 * Wait for the user to acknowledge the error, log a message, and then exit.
 */
static void 
DbCacheCreateError(Widget toplevel)
{
    Widget err;
    int n;
    Arg args[10];
    XmString okLabel;
    XmString message;
    char * messagein;
    char * title;
    Dimension dialogWd, dialogHt;

    title = "dtdbcache failure";
    messagein = "Couldn't recreate the Desktop Actions/DataTypes Database.\n\
Check disk space and/or permissions.";
    message = XmStringCreateLocalized(messagein);
    okLabel = XmStringCreateLocalized("OK");

    /* Post an error dialog */
    n = 0;
    XtSetArg(args[n], XmNtitle, title); n++;
    XtSetArg(args[n], XmNmessageString, message); n++;
    XtSetArg(args[n], XmNokLabelString, okLabel); n++;
    XtSetArg(args[n], XmNdefaultPosition, False);  n++;
    XtSetArg(args[n], XmNmappedWhenManaged, False);  n++;
    err = XmCreateErrorDialog(toplevel, "err", args, n);

    XmStringFree(okLabel);
    XmStringFree(message);

    XtUnmanageChild(XmMessageBoxGetChild(err, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(err, XmDIALOG_HELP_BUTTON));

    /* Center the dialog */
    XtRealizeWidget(err);
    XtVaGetValues(err, XmNwidth, &dialogWd, XmNheight, &dialogHt, NULL);
    XtSetArg(args[0], XmNx,
	     (WidthOfScreen(XtScreen(err)) - dialogWd) / 2);
    XtSetArg(args[1], XmNy,
	     (HeightOfScreen(XtScreen(err)) - dialogHt) / 2);
    XtSetValues (err, args, 2);

    XtManageChild(err);
    XtSetMappedWhenManaged(err, True);
    XtAddCallback(err, XmNokCallback, DbCacheCreateErrorCallback,
		  (XtPointer)NULL);

    XtAppMainLoop(XtWidgetToApplicationContext(err));
}

static void
DieFromToolTalkError(char *errfmt, Tt_status status)
{
    char	*statmsg;

    if (! tt_is_err(status)) return;

    statmsg = tt_status_message(status);
    _DtSimpleError(ApplicationClass, DtFatalError, NULL, errfmt, statmsg);
    exit(status);
}
