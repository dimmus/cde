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
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: server.c /main/4 1995/10/27 16:14:56 rswiston $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

# include	<sys/types.h>
# include	<sys/socket.h>
# include	<sys/signal.h>
# include	<setjmp.h>
# include       <errno.h>
# include	<pwd.h>
# include	<stddef.h>
# include	<time.h>
# include	<sys/wait.h>
#ifdef UNIXCONN
# include	<sys/un.h>
#endif
# include	"dm.h"
# include	"vgmsg.h"

static int receivedUsr1;



/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/

static const char * _SysErrorMsg( int n) ;
static SIGVAL CatchUsr1( int arg ) ;
static void   GetRemoteAddress( struct display *d, int fd) ;
static SIGVAL PingBlocked( int arg ) ;
static SIGVAL PingLost( int arg ) ;
static SIGVAL abortOpen( int arg ) ;
static int    serverPause( unsigned t, int serverPid, struct display *d,
			   int probe) ;
static int    serverListening( struct display *d ) ;
static void   msleep( unsigned ms ) ;
static SIGVAL serverPauseAbort( int arg ) ;
static SIGVAL serverPauseUsr1( int arg ) ;





/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/

static Display	*dpy;



static SIGVAL
CatchUsr1( int arg )
{
#if defined(SYSV) || defined(SVR4)
    (void) signal (SIGUSR1, CatchUsr1);
#endif
    Debug ("Display Manager caught SIGUSR1\n");
    ++receivedUsr1;
}

static const char * 
_SysErrorMsg( int n )
{

  const char *s = strerror(n);

    return (s ? s : "no such error");
}

int 
StartServerOnce( struct display *d )
{
    char	**f;
    char	**argv;
    char	arg[1024];
    int		pid;
    char	**env;
    int		probe;
    
    extern struct passwd   puser;	/* pseudo_user password entry	*/

    Debug ("Starting server for %s\n", d->name);

    /*
     * If something already accepts connections on the display's socket
     * (another X server), the socket cannot tell that ours is ready:
     * wait for SIGUSR1 or the server's death only, as before.
     */
    probe = !serverListening (d);
    if (!probe)
	Debug ("Display %s already accepts connections\n", d->name);
    receivedUsr1 = 0;
    signal (SIGUSR1, CatchUsr1);
    argv = d->argv;
    switch (pid = fork ()) {
    case 0:
	CleanUpChild ();
	if (d->authFile) {
	    sprintf (arg, "-auth %s", d->authFile);
	    argv = parseArgs (argv, arg);
	}
	if (!argv) {
	    LogError(ReadCatalog(MC_LOG_SET,MC_LOG_NO_ARGS,MC_DEF_LOG_NO_ARGS));
	    exit(1);
	}
	Debug("Server invoked as ");
	for (f = argv; *f; f++)
	    Debug ("'%s' ", *f);
	Debug ("\n");

	/*
         *  set the permissions on  console devices to  pseudo-user.
         *  run the server as a pseudo-user, not root...
         */
#ifdef sun
        if (solaris_setdevperm(d->gettyLine, puser.pw_uid, puser.pw_gid) == 0)
             Debug ("Unable to set permissions on console devices ..\n");
        else {
#endif
             if(-1 == setgid (puser.pw_gid)) {
                  Debug ("setgid() failed setting %d\n", puser.pw_gid);
             }
             if(-1 == setuid (puser.pw_uid)) {
                  Debug ("setuid() failed setting %d\n", puser.pw_uid);
             }
#ifdef sun
        }
#endif

	/*
	 *  build the server environment (if any)...
	 */
	env = 0;
	if (d->environStr && strlen(d->environStr) > 0)
	    env = parseEnv(env, d->environStr);
#ifdef sun
	if (getEnv (env, "OPENWINHOME") == NULL) 
	    env = setEnv(env, "OPENWINHOME", "/usr/openwin");
#endif
#ifdef _AIX
	if (getEnv (env, "ODMDIR") == NULL)
	    env = setEnv(env, "ODMDIR", "/etc/objrepos");
#ifdef _POWER
	env = setEnv(env, "XTOEXEC", "true");		/* flag for xserverrc */
#endif
#endif

	/*
	 * give the server SIGUSR1 ignored,
	 * it will notice that and send SIGUSR1
	 * when ready
	 */
	signal (SIGUSR1, SIG_IGN);
	(void) execve (argv[0], argv, env);
	LogError(ReadCatalog(
		MC_LOG_SET,MC_LOG_NO_EXESRV,MC_DEF_LOG_NO_EXESRV),argv[0]);
	exit(1);
    case -1:
	LogError(ReadCatalog(MC_LOG_SET,MC_LOG_FAIL_FORK,MC_DEF_LOG_FAIL_FORK));
	return 0;
    default:
	break;
    }
    Debug ("Server started. Process ID = %d\n", pid);
    d->serverPid = pid;
    if (serverPause ((unsigned) d->openDelay, pid, d, probe))
	return FALSE;
    return TRUE;
}


int 
StartServer( struct display *d )
{
    int	i;
    int	ret = FALSE;

    i = 0;
    while (d->serverAttempts == 0 || i < d->serverAttempts)
    {
	if ((ret = StartServerOnce (d)) == TRUE)
	    break;
	sleep (d->openDelay);
	i++;
    }
    return ret;
}


/*
 * Sleeps for ms milliseconds (less if a signal arrives).
 */
static void
msleep( unsigned ms )
{
    struct timespec	ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long) (ms % 1000) * 1000000L;
    (void) nanosleep (&ts, NULL);
}

/*
 * Returns 1 if the X server of a local display accepts connections on
 * its Unix domain socket, 0 if not or if that cannot be told (no such
 * socket for this kind of display, say).
 *
 * The server sends SIGUSR1 when it is ready, but not if a wrapper
 * script runs it as a child, and not every server does.  Without the
 * signal the wait below used to last openDelay seconds (5 by default).
 */
static int
serverListening( struct display *d )
{
#ifdef UNIXCONN
    const char		*colon;
    const char		*p;
    struct sockaddr_un	addr;
    socklen_t		len;
    int			fd;
    int			ok = 0;
    int			pass;
    long		dpy_num;
    sigset_t		block;
    sigset_t		saved;

    if (d->displayType.location != Local || !d->name ||
	!(colon = strrchr (d->name, ':')))
	return 0;

    /* Only ":N" or "unix:N" go through the Unix domain socket. */
    if (colon != d->name &&
	!(colon - d->name == 4 && strncmp (d->name, "unix", 4) == 0))
	return 0;
    p = colon + 1;
    if (*p < '0' || *p > '9')
	return 0;
    dpy_num = strtol (p, NULL, 10);
    if (dpy_num < 0 || dpy_num > 65535)
	return 0;

    /*
     * serverPause() siglongjmp()s out on SIGUSR1 and SIGALRM: keep them
     * out while a socket is open, or it would leak.
     */
    sigemptyset (&block);
    sigaddset (&block, SIGUSR1);
    sigaddset (&block, SIGALRM);
    sigprocmask (SIG_BLOCK, &block, &saved);

    /*
     * Linux servers also listen on an abstract socket of the same name;
     * try that first, then the file system one.
     */
    for (pass = 0; pass < 2 && !ok; pass++) {
	memset (&addr, 0, sizeof (addr));
	addr.sun_family = AF_UNIX;
#ifdef __linux__
	if (pass == 0) {
	    snprintf (addr.sun_path + 1, sizeof (addr.sun_path) - 1,
		      "/tmp/.X11-unix/X%ld", dpy_num);
	    len = (socklen_t) (offsetof (struct sockaddr_un, sun_path) + 1 +
			       strlen (addr.sun_path + 1));
	}
	else
#else
	if (pass == 0)
	    continue;
#endif
	{
	    snprintf (addr.sun_path, sizeof (addr.sun_path),
		      "/tmp/.X11-unix/X%ld", dpy_num);
	    len = (socklen_t) sizeof (addr);
	}
	if ((fd = socket (AF_UNIX, SOCK_STREAM, 0)) == -1)
	    break;
	/* Never block (on a full backlog, say). */
	(void) fcntl (fd, F_SETFL, O_NONBLOCK);
	if (connect (fd, (struct sockaddr *) &addr, len) == 0)
	    ok = 1;
	close (fd);
    }
    sigprocmask (SIG_SETMASK, &saved, NULL);
    return ok;
#else
    return 0;
#endif
}

/*
 * Waits up to t seconds for the server to get ready: until it sends
 * SIGUSR1, accepts connections (polled after 50 ms, then 100, 200 and
 * every 250 ms) or dies.  Returns 1 if the server is dead, 0 else (also
 * when the time runs out, as before).
 */

static sigjmp_buf	pauseAbort;
static int	serverPauseRet;

static SIGVAL
serverPauseAbort( int arg )
{
    Debug ("Display Manager pause timed out\n");
    siglongjmp (pauseAbort, 1);
}

static SIGVAL
serverPauseUsr1( int arg )
{
    Debug ("Display Manager pause received SIGUSR1\n");
    ++receivedUsr1;
    siglongjmp (pauseAbort, 1);
}

static int 
serverPause( unsigned t, int serverPid, struct display *d, int probe )
{
    int			pid;
    unsigned		delay = 50;

    serverPauseRet = 0;
    Debug ("Display Manager pausing until SIGUSR1 from server or timeout\n");
    if (!sigsetjmp (pauseAbort, 1)) {
	signal (SIGALRM, serverPauseAbort);
	signal (SIGUSR1, serverPauseUsr1);
	if (!receivedUsr1)
	    alarm (t);
	else
	    Debug ("ServerPause(): already received USR1\n");
	for (;;) {
	    /*
	     * Only the server is waited for here: a wait() for any child
	     * used to reap (and lose) other displays' processes too.
	     */
	    pid = waitpid ((pid_t) serverPid, NULL, WNOHANG);
	    if (pid == serverPid ||
		(pid == -1 && errno == ECHILD))
	    {
		Debug ("Server dead\n");
		serverPauseRet = 1;
		break;
	    }
	    if (receivedUsr1) {
		Debug ("Server alive and kicking\n");
		break;
	    }
	    if (probe && serverListening (d)) {
		Debug ("Server accepts connections\n");
		break;
	    }
	    msleep (delay);
	    if ((delay *= 2) > 250)
		delay = 250;
	}
    }
    alarm ((unsigned) 0);
    signal (SIGALRM, SIG_DFL);
    signal (SIGUSR1, CatchUsr1);
    if (serverPauseRet) {
	Debug ("Server died\n");
	LogError(ReadCatalog(MC_LOG_SET,MC_LOG_SRV_DIED,MC_DEF_LOG_SRV_DIED));
    }
    return serverPauseRet;
}


/*
 * this code is complicated by some TCP failings.  On
 * many systems, the connect will occasionally hang forever,
 * this trouble is avoided by setting up a timeout to siglongjmp
 * out of the connect (possibly leaving piles of garbage around
 * inside Xlib) and give up, terminating the server.
 */

static sigjmp_buf	openAbort;

static SIGVAL
abortOpen( int arg )
{
	siglongjmp (openAbort, 1);
}

static void
GetRemoteAddress( struct display *d, int fd )
{
    char    buf[512];
    socklen_t len = sizeof (buf);

    if (d->peer)
	free ((char *) d->peer);
    getpeername (fd, (struct sockaddr *) buf, &len);
    d->peerlen = 0;
    if (len)
    {
	d->peer = malloc (len);
	if (d->peer)
	{
	    bcopy (buf, (char *) d->peer, len);
	    d->peerlen = len;
	}
    }
    Debug ("Got remote address %s %d\n", d->name, d->peerlen);
}



/****************************************************************************
 *
 *  LogOpenError()
 *
 *  If d->startAttempts is a large number and a connection cannot be made to
 *  the server, the error log can fill up rapidly with error messages. This
 *  could be common in X-terminals that do not support XDMCP and are turned
 *  off over a weekend. This routine attempts to reduce the number of error
 *  messages logged in this scenario.
 *  
 ****************************************************************************/

int 
LogOpenError( int count )
{

    if ( count <= 10			    ) return 1;
    if ( count <= 100 && (count %  10 == 0) ) return 1;
    if ( count <= 500 && (count %  50 == 0) ) return 1;
    if ( 		 (count % 100 == 0) ) return 1;
    
    return 0;
}


/*
 * Opens the display.  An attempt that fails is retried after 50 ms,
 * then 100, 200 and every 250 ms until openDelay seconds have passed,
 * and that openRepeat times; it used to sleep openDelay seconds after
 * each failure.
 */
int 
WaitForServer( struct display *d )
{
    int	    i;
    int	    hung = 0;

    for (i = 0; i < (d->openRepeat > 0 ? d->openRepeat : 1) && !hung; i++) {
	struct timespec	start;
	struct timespec	t;
	unsigned	delay = 50;
	long		elapsed;

	clock_gettime (CLOCK_MONOTONIC, &start);
	for (;;) {
	    (void) signal (SIGALRM, abortOpen);
	    (void) alarm ((unsigned) d->openTimeout);
	    if (!sigsetjmp (openAbort, 1)) {
		Debug ("Before XOpenDisplay(%s)\n", d->name);
		errno = 0;
		dpy = XOpenDisplay (d->name);
		(void) alarm ((unsigned) 0);
		(void) signal (SIGALRM, SIG_DFL);
		Debug ("After XOpenDisplay()\n");
		if (dpy) {
		    if (d->displayType.location == Foreign)
			GetRemoteAddress (d, ConnectionNumber (dpy));
		    RegisterCloseOnFork (ConnectionNumber (dpy));
		    (void) fcntl (ConnectionNumber (dpy), F_SETFD, 0);
		    return 1;
		} else {
		    Debug ("OpenDisplay failed %d (%s)\n",
			   errno, _SysErrorMsg (errno));
		}
	    } else {
		Debug ("Hung in open, aborting\n");
		if (LogOpenError(d->startTries))
		    LogError(ReadCatalog(
			    MC_LOG_SET,MC_LOG_HUNG_DPY,MC_DEF_LOG_HUNG_DPY),
			       d->name, d->startTries);
		(void) signal (SIGALRM, SIG_DFL);
		hung = 1;
		break;
	    }

	    clock_gettime (CLOCK_MONOTONIC, &t);
	    elapsed = (t.tv_sec - start.tv_sec) * 1000L +
		      (t.tv_nsec - start.tv_nsec) / 1000000L;
	    if (elapsed >= (long) d->openDelay * 1000L)
		break;
	    Debug ("Waiting for server to start %d\n", i);
	    if ((long) delay > (long) d->openDelay * 1000L - elapsed)
		delay = (unsigned) ((long) d->openDelay * 1000L - elapsed);
	    msleep (delay);
	    if ((delay *= 2) > 250)
		delay = 250;
	}
    }
    Debug ("Giving up on server\n");
    if (LogOpenError(d->startTries))
	LogError(ReadCatalog(
		MC_LOG_SET,MC_LOG_FAIL_SRVOPEN,MC_DEF_LOG_FAIL_SRVOPEN),
		d->startTries, d->name);
    return 0;
}

void
ResetServer( struct display *d )
{
    if (dpy && d->displayType.origin != FromXDMCP)
	pseudoReset (dpy);
}


/****************************************************************************
 *
 *  Server pinging routines...
 *
 *  These routines attempt to determine if the server is still alive.
 *  Periodically (d->pingInterval) an XSync is sent to the server. If an I/O
 *  error occurs, then the connection has been lost and the server needs to
 *  be reset. If the server is blocked for some reason (i.e. server grab) the
 *  XSync will block until a local timer expires. In this case, we just note
 *  the block and continue...
 *
 *  7/26/90 - prr
 *  The XSync was replaced by a socket-level ping. For some reason, an XSync
 *  to a grabbed server causes a subsequent pseudoReset (KillClients) to not 
 *  kill all clients. The socket ping is not affected by a grabbed server,
 *  but it cannot detect a server shutdown and restart within one ping
 *  interval. 
 *
 ****************************************************************************/

static sigjmp_buf	pingTime;
static int	serverDead = FALSE;

static SIGVAL
PingLost( int arg )
{
    serverDead = TRUE;
    siglongjmp (pingTime, 1);
}


static SIGVAL
PingBlocked( int arg )
{
    serverDead = FALSE;
    siglongjmp (pingTime, 1);
}


int 
PingServer( struct display *d, Display *alternateDpy )
{
    int	    (*oldError)();
    SIGVAL  (*oldSig)();
    int	    oldAlarm;

    if (!alternateDpy)
	alternateDpy = dpy;
    oldError = XSetIOErrorHandler ((XIOErrorHandler)PingLost);
    oldAlarm = alarm (0);
    oldSig = signal (SIGALRM, PingBlocked);
    alarm (d->pingTimeout * 60);
    if (!sigsetjmp (pingTime, 1))
    {
	Debug ("Ping server\n");
	XNoOp (alternateDpy);
	XSync (alternateDpy, 0);
	Debug ("Server alive\n");

	while (XPending(alternateDpy)) {
	    XEvent event;
	    XNextEvent(alternateDpy, &event);
	}
    }
    else
    {
	if ( serverDead ) {
	    Debug ("Server dead\n");
	    alarm (0);
	    signal (SIGALRM, SIG_DFL);
	    XSetIOErrorHandler (oldError);
	    return 0;
	}
	else
	    Debug ("Server blocked, continuing...\n");
    }
    alarm (0);
    signal (SIGALRM, oldSig);
    alarm (oldAlarm);
    XSetIOErrorHandler (oldError);
    return 1;
}
