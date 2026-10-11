/**
 * CDE benchmarks: functional tests of dtwm's EWMH support.
 *
 * Licensed under the LGPL 2.1 license.
 *
 *   ewmhtest [-f] [-m DTWM] [-x RESOURCE]... TEST...
 *
 * ewmhtest starts the dtwm under test (-m, by default the one of this
 * build) as the window manager of $DISPLAY, which must have none and no
 * other clients, as dtwmbench does: with an empty HOME, without the front
 * panel (-f keeps it) and with the extra resources given by -x (dtwm's
 * -xrm).  It then acts as an EWMH client and checks what dtwm publishes
 * on the root window, waiting for the PropertyNotify events of its
 * writes (dtwm writes them when it goes idle).
 *
 * The tests (all: every one, in turn, in the same dtwm):
 *   clientlist  _NET_CLIENT_LIST in mapping order, transients included;
 *               _NET_CLIENT_LIST_STACKING follows XRaiseWindow and new
 *               transients; destroyed windows leave both lists; no
 *               window of dtwm's own (front panel, subpanels, icon box)
 *               is listed
 *   active      _NET_ACTIVE_WINDOW follows _NET_ACTIVE_WINDOW requests,
 *               is rewritten only when it changes, and is not an iconified
 *               window
 *   desktops    _NET_NUMBER_OF_DESKTOPS, _NET_DESKTOP_NAMES, ..._VIEWPORT,
 *               ..._GEOMETRY and _NET_CURRENT_DESKTOP follow workspace
 *               changes and the requests to switch, add and remove
 *               desktops; _NET_WM_DESKTOP of a window follows requests to
 *               move it, is honoured when set before the window is mapped
 *               (a desktop, or all), and shifts when a workspace before
 *               the window's is deleted (f.delete_workspace)
 *   xdotool     the same through xdotool's desktop commands (skipped
 *               when xdotool is not installed)
 *   states      _NET_WM_STATE: each state through messages (two unrelated
 *               ones in one message, maximizing along one axis), focused
 *               following the focus, hidden when iconified, demands
 *               attention following the urgency hint and cleared by the
 *               focus, states set before the window is mapped (full
 *               screen, maximized vertically, sticky), and xdotool
 *               windowstate
 *   layers      the stacking layers (_NET_CLIENT_LIST_STACKING and the X
 *               order of the frames agree): above and below windows stay
 *               there when others are raised (XRaiseWindow, xdotool
 *               windowraise and windowactivate) or they are lowered or
 *               raised; a dock is above them, a desktop window below all;
 *               a full screen window is above the dock only while it has
 *               the focus; transients stay above their leader and come
 *               with it; a system modal window is above everything
 *   types       _NET_WM_WINDOW_TYPE: decorations (frame and client
 *               geometry), focus when mapped, workspaces, position, and a
 *               dialog that becomes a transient of its window group
 *   struts      _NET_WM_STRUT_PARTIAL and _NET_WM_STRUT: _NET_WORKAREA of
 *               the desktops the window is on, maximized windows (through
 *               messages and xdotool) within it and refitted when it
 *               changes, full screen ignoring it, new windows placed in it
 *               (skipped with -x 'Dtwm*frontPanelStrut: True' and -f)
 *   xinerama    with two heads side by side (Xvfb +xinerama -screen 0 ...
 *               -screen 1 ...; skipped otherwise): a strut on the inner
 *               edge of the left head limits only that head
 *   panel       with -f, the front panel takes part of the work area if
 *               and only if -x 'Dtwm*frontPanelStrut: True' is given
 *
 * Exit status: 0 when every test passed, 1 on a failure, 77 when there
 * is no display.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "benchutil.h"

#ifndef EWMHTEST_WM
#define EWMHTEST_WM "dtwm"
#endif
#ifndef EWMHTEST_LIBPATH
#define EWMHTEST_LIBPATH ""
#endif

#define TIMEOUT_MS 10000
#define MAX_XRM 16

static Display *dpy;
static Window rootw;
static const char *wm_path = EWMHTEST_WM;
static pid_t wm_pid;
static char workdir[PATH_MAX];
static char logpath[PATH_MAX + 16];
static const char *xrm[MAX_XRM];
static int nxrm;
static int front_panel;

static Atom a_supported, a_check, a_client_list, a_stacking, a_active;
static Atom a_number, a_current, a_names, a_viewport, a_geometry;
static Atom a_wm_desktop, a_wm_state, a_utf8, a_dt_wm_request;
static Atom a_net_wm_state;

/* The windows the tests created and have not destroyed. */
#define MAX_OURS 32
static Window ours[MAX_OURS];
static int nours;

/* ------------------------------------------------------------------ */
/* Process and X utilities                                             */
/* ------------------------------------------------------------------ */

static void rm_tree(const char *path)
{
	struct stat st;
	struct dirent *e;
	char sub[PATH_MAX];
	DIR *d;

	if (lstat(path, &st))
		return;
	if (!S_ISDIR(st.st_mode)) {
		unlink(path);
		return;
	}
	if ((d = opendir(path))) {
		while ((e = readdir(d))) {
			if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
				continue;
			snprintf(sub, sizeof sub, "%s/%s", path, e->d_name);
			rm_tree(sub);
		}
		closedir(d);
	}
	rmdir(path);
}

/* At exit (bench_die exits too): show dtwm's log on failure, clean up. */
static int failed;

static void cleanup(void)
{
	char line[512];
	FILE *f;

	if (wm_pid > 0) {
		kill(wm_pid, SIGKILL);
		waitpid(wm_pid, NULL, 0);
		wm_pid = 0;
	}
	if (failed && *logpath && (f = fopen(logpath, "r"))) {
		while (fgets(line, sizeof line, f))
			fprintf(stderr, "dtwm: %s", line);
		fclose(f);
	}
	if (*workdir)
		rm_tree(workdir);
	*workdir = '\0';
}

static void on_signal(int sig)
{
	if (wm_pid > 0)
		kill(wm_pid, SIGKILL);
	signal(sig, SIG_DFL);
	raise(sig);
}

#define fail(...) (failed = 1, bench_die("ewmhtest: " __VA_ARGS__))

static int x_error(Display *d, XErrorEvent *ev)
{
	char text[128];

	XGetErrorText(d, ev->error_code, text, sizeof text);
	fail("X error: %s (request %d.%d)", text, ev->request_code,
	     ev->minor_code);
	return 0;
}

static void check_wm(void)
{
	int status;

	if (wm_pid > 0 && waitpid(wm_pid, &status, WNOHANG) == wm_pid) {
		wm_pid = 0;
		fail("dtwm exited (status 0x%x)", status);
	}
}

/* A 32-bit property of a window, or NULL (*n = 0) when absent. */
static unsigned long *get_prop_on(Window w, Atom prop, Atom type, int *n)
{
	Atom rtype;
	int format;
	unsigned long nitems, after;
	unsigned char *data = NULL;

	*n = 0;
	if (XGetWindowProperty(dpy, w, prop, 0, 65536, False, type,
			       &rtype, &format, &nitems, &after,
			       &data) != Success || rtype != type ||
	    format != 32) {
		if (data)
			XFree(data);
		return NULL;
	}
	*n = (int)nitems;
	return (unsigned long *)data;
}

static unsigned long *get_prop(Atom prop, Atom type, int *n)
{
	return get_prop_on(rootw, prop, type, n);
}

static int has_prop(Atom prop, Atom type)
{
	int n;
	unsigned long *v = get_prop(prop, type, &n);

	if (!v)
		return 0;
	XFree(v);
	return 1;
}

static int is_ours(Window w)
{
	int i;

	for (i = 0; i < nours; i++)
		if (ours[i] == w)
			return 1;
	return 0;
}

static void print_list(const char *name, Atom prop)
{
	int i, n;
	unsigned long *v = get_prop(prop, XA_WINDOW, &n);

	fprintf(stderr, "  %s:", name);
	for (i = 0; i < n; i++)
		fprintf(stderr, " 0x%lx", v[i]);
	fprintf(stderr, v ? "\n" : " (none)\n");
	if (v)
		XFree(v);
}

/*
 * Handle events until a PropertyNotify of prop on window w, or until the
 * deadline.  Returns 0 on timeout.  *counted, if counted is not
 * NULL, counts the PropertyNotify events of counted_prop.
 */
static Atom counted_prop;
static int *counted;

static int wait_notify(Window w, Atom prop, double deadline)
{
	struct pollfd pfd = { ConnectionNumber(dpy), POLLIN, 0 };
	XEvent ev;

	XFlush(dpy);
	for (;;) {
		while (XPending(dpy)) {
			XNextEvent(dpy, &ev);
			if (ev.type != PropertyNotify)
				continue;
			if (counted && ev.xproperty.atom == counted_prop &&
			    ev.xproperty.window == rootw)
				++*counted;
			if (ev.xproperty.window != w)
				continue;
			if (ev.xproperty.atom == prop)
				return 1;
		}
		check_wm();
		if (bench_now_ns() > deadline)
			return 0;
		poll(&pfd, 1, 50);
	}
}

/*
 * Drop the events received so far: a PropertyNotify of a write already
 * seen through the property's value may still be queued.
 */
static void drain(void)
{
	XEvent ev;

	XSync(dpy, False);
	while (XPending(dpy))
		XNextEvent(dpy, &ev);
}

/*
 * Wait until check() holds of the property prop of window w, rechecking
 * at each of its PropertyNotify events.
 */
typedef int (*checker)(Window w, Atom prop, const void *arg);

static void wait_for_on(Window w, Atom prop, checker check, const void *arg,
			const char *what)
{
	double deadline = bench_now_ns() + TIMEOUT_MS * 1e6;

	XSync(dpy, False);
	while (!check(w, prop, arg)) {
		if (!wait_notify(w, prop, deadline)) {
			fprintf(stderr, "ewmhtest: timed out waiting for %s\n",
				what);
			print_list("_NET_CLIENT_LIST", a_client_list);
			print_list("_NET_CLIENT_LIST_STACKING", a_stacking);
			print_list("_NET_ACTIVE_WINDOW", a_active);
			fail("%s", what);
		}
	}
}

static void wait_for(Atom prop, checker check, const void *arg,
		     const char *what)
{
	wait_for_on(rootw, prop, check, arg, what);
}

/* An expected list of windows: exactly these, or ending with them. */
struct expect {
	const Window *w;
	int n;
	int suffix;
};

/* The list holds our windows (others ignored) as expected. */
static int list_is(Window w, Atom prop, const void *arg)
{
	const struct expect *e = arg;
	unsigned long *v;
	int i, n, k = 0, ok;
	Window mine[MAX_OURS];

	v = get_prop_on(w, prop, XA_WINDOW, &n);
	for (i = 0; i < n; i++)
		if (is_ours(v[i]) && k < MAX_OURS)
			mine[k++] = v[i];
	if (v)
		XFree(v);
	if (e->suffix)
		ok = k >= e->n &&
		     !memcmp(mine + k - e->n, e->w, e->n * sizeof(Window));
	else
		ok = k == e->n && !memcmp(mine, e->w, e->n * sizeof(Window));
	return ok;
}

static int active_is(Window w, Atom prop, const void *arg)
{
	Window want = *(const Window *)arg;
	unsigned long *v;
	int n, ok;

	v = get_prop_on(w, prop, XA_WINDOW, &n);
	ok = n == 1 && v[0] == want;
	if (v)
		XFree(v);
	return ok;
}

static int active_is_not(Window w, Atom prop, const void *arg)
{
	Window not = *(const Window *)arg;
	unsigned long *v;
	int n, ok;

	v = get_prop_on(w, prop, XA_WINDOW, &n);
	ok = n == 1 && v[0] != not;
	if (v)
		XFree(v);
	return ok;
}

static void expect_list(Atom prop, const char *name, int suffix, int n, ...)
{
	Window w[MAX_OURS];
	struct expect e = { w, n, suffix };
	char what[256];
	int i, len;
	va_list ap;

	len = snprintf(what, sizeof what, "%s %s", name,
		       suffix ? "ending with" : "=");
	va_start(ap, n);
	for (i = 0; i < n; i++) {
		w[i] = va_arg(ap, Window);
		if (len < (int)sizeof what - 16)
			len += snprintf(what + len, sizeof what - len,
					" 0x%lx", w[i]);
	}
	va_end(ap);
	wait_for(prop, list_is, &e, what);
}

/* No window but ours is listed: dtwm's own windows are not. */
static void expect_only_ours(Atom prop, const char *name)
{
	unsigned long *v;
	int i, n;

	v = get_prop(prop, XA_WINDOW, &n);
	for (i = 0; i < n; i++) {
		XClassHint ch = { NULL, NULL };

		if (is_ours(v[i]))
			continue;
		XGetClassHint(dpy, v[i], &ch);
		fail("%s lists 0x%lx (%s.%s), which is not a test window",
		     name, v[i], ch.res_name ? ch.res_name : "?",
		     ch.res_class ? ch.res_class : "?");
	}
	if (v)
		XFree(v);
}

/* ------------------------------------------------------------------ */
/* dtwm                                                                */
/* ------------------------------------------------------------------ */

static void start_wm(void)
{
	const char *argv[2 * MAX_XRM + 8];
	const char *tmp;
	int argc = 0, i, fd;
	double deadline;

	tmp = getenv("TMPDIR");
	snprintf(workdir, sizeof workdir, "%s/ewmhtest.XXXXXX",
		 tmp && *tmp ? tmp : "/tmp");
	if (!mkdtemp(workdir)) {
		*workdir = '\0';
		fail("mkdtemp: %s", strerror(errno));
	}
	snprintf(logpath, sizeof logpath, "%s/dtwm.log", workdir);

	argv[argc++] = wm_path;
	argv[argc++] = "-xrm";
	argv[argc++] = "Dtwm*useIconBox: False";
	if (!front_panel) {
		argv[argc++] = "-xrm";
		argv[argc++] = "Dtwm*useFrontPanel: False";
	}
	for (i = 0; i < nxrm; i++) {
		argv[argc++] = "-xrm";
		argv[argc++] = xrm[i];
	}
	argv[argc] = NULL;

	fflush(NULL);
	wm_pid = fork();
	if (wm_pid < 0)
		fail("fork: %s", strerror(errno));
	if (wm_pid == 0) {
		fd = open(logpath, O_WRONLY | O_CREAT | O_APPEND, 0600);
		if (fd >= 0) {
			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);
		}
		close(ConnectionNumber(dpy));
		/* An empty home: no .dtwmrc, no resources of the user. */
		setenv("HOME", workdir, 1);
		unsetenv("XENVIRONMENT");
		unsetenv("XAPPLRESDIR");
		if (*EWMHTEST_LIBPATH) {
			const char *old = getenv("LD_LIBRARY_PATH");
			char *lp = malloc(sizeof EWMHTEST_LIBPATH +
					  (old ? strlen(old) + 1 : 0));

			if (lp) {
				strcpy(lp, EWMHTEST_LIBPATH);
				if (old && *old) {
					strcat(lp, ":");
					strcat(lp, old);
				}
				setenv("LD_LIBRARY_PATH", lp, 1);
			}
		}
		execvp(wm_path, (char *const *)argv);
		fprintf(stderr, "exec %s: %s\n", wm_path, strerror(errno));
		_exit(127);
	}

	/*
	 * dtwm is ready once it has written _NET_CLIENT_LIST, which it does
	 * when it first goes idle, after adopting the existing windows.
	 */
	deadline = bench_now_ns() + 30000 * 1e6;
	while (!has_prop(a_check, XA_WINDOW) ||
	       !has_prop(a_client_list, XA_WINDOW)) {
		check_wm();
		if (bench_now_ns() > deadline)
			fail("timed out waiting for dtwm to start");
		wait_notify(rootw, a_client_list, bench_now_ns() + 100 * 1e6);
	}
}

/* ------------------------------------------------------------------ */
/* Test windows                                                        */
/* ------------------------------------------------------------------ */

static Window create_window(const char *title, Window transient_for)
{
	XSetWindowAttributes swa;
	XSizeHints *sh = XAllocSizeHints();
	XWMHints *wmh = XAllocWMHints();
	XClassHint ch = { "ewmhtest", "EwmhTest" };
	XTextProperty name;
	char *list[1];
	Window win;

	if (nours == MAX_OURS)
		fail("too many windows");
	swa.background_pixel = WhitePixel(dpy, DefaultScreen(dpy));
	swa.event_mask = StructureNotifyMask | PropertyChangeMask;
	win = XCreateWindow(dpy, rootw, 100 + 40 * nours, 100 + 30 * nours,
			    300, 200, 0, CopyFromParent, InputOutput,
			    CopyFromParent, CWBackPixel | CWEventMask, &swa);
	sh->flags = PSize;
	sh->width = 300;
	sh->height = 200;
	wmh->flags = InputHint | StateHint;
	wmh->input = True;
	wmh->initial_state = NormalState;
	list[0] = (char *)title;
	if (!XStringListToTextProperty(list, 1, &name))
		fail("out of memory");
	XSetWMProperties(dpy, win, &name, &name, NULL, 0, sh, wmh, &ch);
	if (transient_for)
		XSetTransientForHint(dpy, win, transient_for);
	XFree(name.value);
	XFree(sh);
	XFree(wmh);
	ours[nours++] = win;
	return win;
}

static void destroy_window(Window w)
{
	int i;

	XDestroyWindow(dpy, w);
	for (i = 0; i < nours; i++)
		if (ours[i] == w) {
			ours[i] = ours[--nours];
			break;
		}
}

static void destroy_all(void)
{
	while (nours)
		destroy_window(ours[0]);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 0);
}

static void send_active(Window w)
{
	XEvent ev;

	memset(&ev, 0, sizeof ev);
	ev.xclient.type = ClientMessage;
	ev.xclient.window = w;
	ev.xclient.message_type = a_active;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = 2;		/* from a pager */
	ev.xclient.data.l[1] = CurrentTime;
	XSendEvent(dpy, rootw, False,
		   SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static void test_supported(void)
{
	unsigned long *v;
	int i, n, lists = 0;

	v = get_prop(a_supported, XA_ATOM, &n);
	for (i = 0; i < n; i++)
		if (v[i] == a_client_list || v[i] == a_stacking)
			lists++;
	if (v)
		XFree(v);
	if (lists != 2)
		fail("_NET_SUPPORTED lacks the client lists");
}

static void test_clientlist(void)
{
	Window a, b, c, d;

	test_supported();
	expect_only_ours(a_client_list, "_NET_CLIENT_LIST");
	expect_only_ours(a_stacking, "_NET_CLIENT_LIST_STACKING");

	a = create_window("ewmhtest A", None);
	b = create_window("ewmhtest B", None);
	c = create_window("ewmhtest C", None);
	XMapWindow(dpy, a);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 1, a);
	XMapWindow(dpy, b);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 2, a, b);
	XMapWindow(dpy, c);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 3, a, b, c);
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 0, 3, a, b, c);

	XRaiseWindow(dpy, a);
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 0, 3, b, c, a);

	/* a transient of B comes on top, with B */
	d = create_window("ewmhtest D, transient for B", b);
	XMapWindow(dpy, d);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 4, a, b, c, d);
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 0, 4,
		    c, a, b, d);

	/* raising C (a leader without transients) */
	XRaiseWindow(dpy, c);
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 0, 4,
		    a, b, d, c);

	destroy_window(c);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 3, a, b, d);
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 0, 3, a, b, d);

	/* B goes, and dtwm withdraws its transient D with it */
	destroy_window(b);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 1, a);
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 0, 1, a);

	expect_only_ours(a_client_list, "_NET_CLIENT_LIST");
	expect_only_ours(a_stacking, "_NET_CLIENT_LIST_STACKING");
	destroy_all();
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 0, 0);
	printf("clientlist: ok\n");
}

static void test_active(void)
{
	Window a, b;
	int writes = 0;

	a = create_window("ewmhtest A", None);
	b = create_window("ewmhtest B", None);
	XMapWindow(dpy, a);
	XMapWindow(dpy, b);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 2, a, b);

	send_active(a);
	wait_for(a_active, active_is, &a, "_NET_ACTIVE_WINDOW = A");
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 1, 1, a);
	send_active(b);
	wait_for(a_active, active_is, &b, "_NET_ACTIVE_WINDOW = B");

	/*
	 * Activating B again changes nothing: dtwm must not rewrite
	 * _NET_ACTIVE_WINDOW.  It handles the request before the raise of
	 * A that follows, and writes the focus before the stacking order.
	 */
	drain();
	counted_prop = a_active;
	counted = &writes;
	send_active(b);
	XRaiseWindow(dpy, a);
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 0, 2, b, a);
	counted = NULL;
	if (writes)
		fail("_NET_ACTIVE_WINDOW rewritten %d times without a change",
		     writes);
	if (!active_is(rootw, a_active, &b))
		fail("_NET_ACTIVE_WINDOW is no longer B");

	/* an iconified window is not active */
	XIconifyWindow(dpy, b, DefaultScreen(dpy));
	wait_for(a_active, active_is_not, &b,
		 "_NET_ACTIVE_WINDOW not to be B once iconified");

	destroy_all();
	printf("active: ok\n");
}

/* ------------------------------------------------------------------ */
/* Desktops                                                            */
/* ------------------------------------------------------------------ */

#define ALL_DESKTOPS 0xFFFFFFFFUL

static void send_msg(Window w, Atom type, long l0)
{
	XEvent ev;

	memset(&ev, 0, sizeof ev);
	ev.xclient.type = ClientMessage;
	ev.xclient.window = w;
	ev.xclient.message_type = type;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = l0;
	ev.xclient.data.l[1] = 2;		/* from a pager */
	XSendEvent(dpy, rootw, False,
		   SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

/* The property is one CARDINAL, *arg. */
static int card_is(Window w, Atom prop, const void *arg)
{
	unsigned long want = *(const unsigned long *)arg, *v;
	int n, ok;

	v = get_prop_on(w, prop, XA_CARDINAL, &n);
	ok = n == 1 && (v[0] & 0xFFFFFFFFUL) == want;
	if (v)
		XFree(v);
	return ok;
}

static void expect_card(Window w, Atom prop, unsigned long want,
			const char *what)
{
	char text[160];

	snprintf(text, sizeof text, "%s = %ld", what, (long)want);
	wait_for_on(w, prop, card_is, &want, text);
}

static long card(Window w, Atom prop)
{
	unsigned long *v;
	long value = -1;
	int n;

	v = get_prop_on(w, prop, XA_CARDINAL, &n);
	if (n == 1)
		value = v[0] & 0xFFFFFFFFUL;
	if (v)
		XFree(v);
	return value;
}

/* The names in _NET_DESKTOP_NAMES, each terminated by a NUL. */
static int count_names(void)
{
	Atom type;
	int format, count = 0;
	unsigned long nitems, after, i;
	unsigned char *data = NULL;

	if (XGetWindowProperty(dpy, rootw, a_names, 0, 65536, False, a_utf8,
			       &type, &format, &nitems, &after,
			       &data) != Success || type != a_utf8 ||
	    format != 8) {
		if (data)
			XFree(data);
		return -1;
	}
	for (i = 0; i < nitems; i++)
		if (!data[i])
			count++;
	if (nitems && data[nitems - 1])
		count = -1;		/* the last one is not terminated */
	XFree(data);
	return count;
}

static int names_are(Window w, Atom prop, const void *arg)
{
	(void)w;
	(void)prop;
	return count_names() == *(const int *)arg;
}

/* Set the number of desktops, check the properties that go with it. */
static void expect_desktops(int num)
{
	unsigned long *v;
	int i, n;

	expect_card(rootw, a_number, num, "_NET_NUMBER_OF_DESKTOPS");
	wait_for(a_names, names_are, &num, "_NET_DESKTOP_NAMES count");
	v = get_prop(a_viewport, XA_CARDINAL, &n);
	if (n != 2 * num)
		fail("_NET_DESKTOP_VIEWPORT has %d values for %d desktops",
		     n, num);
	for (i = 0; i < n; i++)
		if (v[i])
			fail("_NET_DESKTOP_VIEWPORT is not all 0,0");
	if (v)
		XFree(v);
}

static int viewable(Window w)
{
	XWindowAttributes wa;

	XSync(dpy, False);
	return XGetWindowAttributes(dpy, w, &wa) &&
	       wa.map_state == IsViewable;
}

static void expect_viewable(Window w, int yes, const char *name)
{
	if (viewable(w) != yes)
		fail("%s is %sviewable", name, yes ? "not " : "");
}

static int has_wm_state(Window w, Atom prop, const void *arg)
{
	Atom type;
	int format;
	unsigned long nitems, after;
	unsigned char *data = NULL;
	int ok;

	(void)arg;
	ok = XGetWindowProperty(dpy, w, prop, 0, 2, False, a_wm_state, &type,
				&format, &nitems, &after, &data) == Success &&
	     type == a_wm_state && format == 32 && nitems >= 1 &&
	     ((unsigned long *)data)[0] == NormalState;
	if (data)
		XFree(data);
	return ok;
}

/* Map a window with a _NET_WM_DESKTOP set beforehand; wait until managed. */
static Window map_on_desktop(const char *title, unsigned long desktop)
{
	Window w = create_window(title, None);
	long value = (long)desktop;

	XChangeProperty(dpy, w, a_wm_desktop, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)&value, 1);
	XMapWindow(dpy, w);
	wait_for_on(w, a_wm_state, has_wm_state, NULL,
		    "WM_STATE NormalState");
	expect_list(a_client_list, "_NET_CLIENT_LIST", 1, 1, w);
	return w;
}

/* f.delete_workspace through dtwm's _DT_WM_REQUEST: the active one goes. */
static void delete_active_workspace(void)
{
	static const char req[] = "f.delete_workspace";
	unsigned long *v;
	int n;

	v = get_prop(a_check, XA_WINDOW, &n);
	if (n != 1)
		fail("no _NET_SUPPORTING_WM_CHECK window");
	XChangeProperty(dpy, v[0], a_dt_wm_request, XA_STRING, 8,
			PropModeAppend, (const unsigned char *)req,
			sizeof req);
	XFree(v);
}

static void test_desktops(void)
{
	static const char *const names[] = {
		"_NET_NUMBER_OF_DESKTOPS", "_NET_DESKTOP_GEOMETRY",
		"_NET_DESKTOP_VIEWPORT", "_NET_CURRENT_DESKTOP",
		"_NET_DESKTOP_NAMES", "_NET_WM_DESKTOP",
	};
	unsigned long *v;
	int i, j, n;
	Window w, p2, pall;

	v = get_prop(a_supported, XA_ATOM, &n);
	for (i = 0; i < (int)(sizeof names / sizeof names[0]); i++) {
		Atom a = XInternAtom(dpy, names[i], False);

		for (j = 0; j < n && v[j] != a; j++)
			;
		if (j == n)
			fail("_NET_SUPPORTED lacks %s", names[i]);
	}
	if (v)
		XFree(v);

	v = get_prop(a_geometry, XA_CARDINAL, &n);
	if (n != 2 || (int)v[0] != DisplayWidth(dpy, DefaultScreen(dpy)) ||
	    (int)v[1] != DisplayHeight(dpy, DefaultScreen(dpy)))
		fail("_NET_DESKTOP_GEOMETRY is not the screen size");
	if (v)
		XFree(v);

	/* four desktops, the first one current */
	send_msg(rootw, a_number, 4);
	expect_desktops(4);
	send_msg(rootw, a_current, 0);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");

	/* a window on the current desktop; switch away from it */
	w = create_window("ewmhtest W", None);
	XMapWindow(dpy, w);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 1, w);
	expect_card(w, a_wm_desktop, 0, "_NET_WM_DESKTOP of W");
	send_msg(rootw, a_current, 1);
	expect_card(rootw, a_current, 1, "_NET_CURRENT_DESKTOP");
	expect_viewable(w, 0, "W, on desktop 0,");
	if (card(w, a_wm_desktop) != 0)
		fail("_NET_WM_DESKTOP of W changed with the current desktop");

	/* move it to the current desktop */
	send_msg(w, a_wm_desktop, 1);
	expect_card(w, a_wm_desktop, 1, "_NET_WM_DESKTOP of W");
	expect_viewable(w, 1, "W, moved to the current desktop,");

	/* set before the window is mapped: a desktop that is not current */
	p2 = map_on_desktop("ewmhtest P2", 2);
	expect_card(p2, a_wm_desktop, 2, "_NET_WM_DESKTOP of P2");
	expect_viewable(p2, 0, "P2, on desktop 2,");
	send_msg(rootw, a_current, 2);
	expect_card(rootw, a_current, 2, "_NET_CURRENT_DESKTOP");
	expect_viewable(p2, 1, "P2, on the current desktop,");

	/* all desktops, also one added later */
	pall = map_on_desktop("ewmhtest PALL", ALL_DESKTOPS);
	expect_card(pall, a_wm_desktop, ALL_DESKTOPS, "_NET_WM_DESKTOP of PALL");
	expect_viewable(pall, 1, "PALL");
	send_msg(rootw, a_current, 0);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");
	expect_viewable(pall, 1, "PALL, after set_desktop,");
	send_msg(rootw, a_number, 5);
	expect_desktops(5);
	send_msg(rootw, a_current, 4);
	expect_card(rootw, a_current, 4, "_NET_CURRENT_DESKTOP");
	expect_viewable(pall, 1, "PALL, on a new desktop,");
	expect_viewable(w, 0, "W, on desktop 1,");
	expect_card(pall, a_wm_desktop, ALL_DESKTOPS, "_NET_WM_DESKTOP of PALL");

	/* removing the current (last) desktop: dtwm goes to the first */
	send_msg(rootw, a_number, 4);
	expect_desktops(4);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");

	/*
	 * Deleting desktop 1 (the current one, f.delete_workspace): W, only
	 * there, goes to the next one, desktop 2, which becomes 1 as P2's
	 * does.
	 */
	send_msg(rootw, a_current, 1);
	expect_card(rootw, a_current, 1, "_NET_CURRENT_DESKTOP");
	delete_active_workspace();
	expect_desktops(3);
	expect_card(rootw, a_current, 1, "_NET_CURRENT_DESKTOP");
	expect_card(p2, a_wm_desktop, 1, "_NET_WM_DESKTOP of P2");
	expect_card(w, a_wm_desktop, 1, "_NET_WM_DESKTOP of W");
	expect_card(pall, a_wm_desktop, ALL_DESKTOPS, "_NET_WM_DESKTOP of PALL");
	expect_viewable(w, 1, "W");
	expect_viewable(p2, 1, "P2");

	/* an out-of-range request changes nothing */
	send_msg(w, a_wm_desktop, 7);
	send_msg(rootw, a_current, 7);
	send_msg(w, a_wm_desktop, 0);
	expect_card(w, a_wm_desktop, 0, "_NET_WM_DESKTOP of W");
	expect_card(rootw, a_current, 1, "_NET_CURRENT_DESKTOP");

	destroy_all();
	send_msg(rootw, a_number, 4);
	send_msg(rootw, a_current, 0);
	expect_desktops(4);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");
	printf("desktops: ok\n");
}

/* Run xdotool with these arguments; its output as a number, or -1. */
static long xdotool(const char *fmt, ...)
{
	char cmd[256], out[64];
	long value = -1;
	va_list ap;
	FILE *f;

	strcpy(cmd, "xdotool ");
	va_start(ap, fmt);
	vsnprintf(cmd + 8, sizeof cmd - 8, fmt, ap);
	va_end(ap);
	fflush(NULL);
	if (!(f = popen(cmd, "r")))
		fail("popen: %s", strerror(errno));
	if (fgets(out, sizeof out, f))
		value = strtol(out, NULL, 0);
	if (pclose(f))
		fail("%s failed", cmd);
	return value;
}

static void test_xdotool(void)
{
	Window w;
	long v;

	if (system("xdotool version >/dev/null 2>&1")) {
		printf("xdotool: skipped (no xdotool)\n");
		return;
	}
	xdotool("set_num_desktops 4");
	expect_card(rootw, a_number, 4, "_NET_NUMBER_OF_DESKTOPS");
	if ((v = xdotool("get_num_desktops")) != 4)
		fail("xdotool get_num_desktops says %ld", v);
	xdotool("set_desktop 2");
	expect_card(rootw, a_current, 2, "_NET_CURRENT_DESKTOP");
	if ((v = xdotool("get_desktop")) != 2)
		fail("xdotool get_desktop says %ld", v);

	w = create_window("ewmhtest X", None);
	XMapWindow(dpy, w);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 1, w);
	expect_card(w, a_wm_desktop, 2, "_NET_WM_DESKTOP of X");
	xdotool("set_desktop_for_window 0x%lx 3", w);
	expect_card(w, a_wm_desktop, 3, "_NET_WM_DESKTOP of X");
	if ((v = xdotool("get_desktop_for_window 0x%lx", w)) != 3)
		fail("xdotool get_desktop_for_window says %ld", v);
	expect_viewable(w, 0, "X, moved to desktop 3,");

	xdotool("set_num_desktops 6");
	expect_desktops(6);
	if ((v = xdotool("get_num_desktops")) != 6)
		fail("xdotool get_num_desktops says %ld", v);
	xdotool("set_desktop 5");
	expect_card(rootw, a_current, 5, "_NET_CURRENT_DESKTOP");
	xdotool("set_num_desktops 4");
	expect_desktops(4);
	if ((v = xdotool("get_desktop")) != 0)
		fail("xdotool get_desktop says %ld after the current desktop "
		     "was removed", v);
	if ((v = xdotool("get_desktop_for_window 0x%lx", w)) != 3)
		fail("xdotool get_desktop_for_window says %ld", v);

	destroy_all();
	printf("xdotool: ok\n");
}

/* ------------------------------------------------------------------ */
/* States                                                              */
/* ------------------------------------------------------------------ */

static int panel_strut(void);

/*
 * The height of the work area of the first desktop (less than the
 * screen with Dtwm*frontPanelStrut), once the front panel is up.
 */
static int work_height(void)
{
	Atom workarea = XInternAtom(dpy, "_NET_WORKAREA", False);
	unsigned long *v;
	int n, h = DisplayHeight(dpy, DefaultScreen(dpy));
	double deadline = bench_now_ns() + TIMEOUT_MS * 1e6;

	for (;;) {
		v = get_prop(workarea, XA_CARDINAL, &n);
		if (n >= 4 && (long)v[3] < h)
			h = (int)v[3];
		if (v)
			XFree(v);
		if (h < DisplayHeight(dpy, DefaultScreen(dpy)) ||
		    !panel_strut())
			return h;
		if (!wait_notify(rootw, workarea, deadline))
			fail("no _NET_WORKAREA for the front panel");
	}
}

#define REMOVE 0
#define ADD 1
#define TOGGLE 2

static Atom state_atom(const char *name)
{
	char full[64];

	snprintf(full, sizeof full, "_NET_WM_STATE_%s", name);
	return XInternAtom(dpy, full, False);
}

static int has_state(Window w, Atom state)
{
	unsigned long *v;
	int i, n, found = 0;

	v = get_prop_on(w, a_net_wm_state, XA_ATOM, &n);
	for (i = 0; i < n; i++)
		if (v[i] == state)
			found = 1;
	if (v)
		XFree(v);
	return found;
}

struct state_expect {
	Atom state;
	int set;
};

static int state_is(Window w, Atom prop, const void *arg)
{
	const struct state_expect *e = arg;

	(void)prop;
	return has_state(w, e->state) == e->set;
}

static void expect_state(Window w, const char *name, int set)
{
	struct state_expect e = { state_atom(name), set };
	char what[128];

	snprintf(what, sizeof what, "_NET_WM_STATE %s %s", set ? "with" :
		 "without", name);
	wait_for_on(w, a_net_wm_state, state_is, &e, what);
}

static void send_state(Window w, long action, const char *first,
		       const char *second)
{
	XEvent ev;

	memset(&ev, 0, sizeof ev);
	ev.xclient.type = ClientMessage;
	ev.xclient.window = w;
	ev.xclient.message_type = a_net_wm_state;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = action;
	ev.xclient.data.l[1] = first ? (long)state_atom(first) : 0;
	ev.xclient.data.l[2] = second ? (long)state_atom(second) : 0;
	ev.xclient.data.l[3] = 2;		/* from a pager */
	XSendEvent(dpy, rootw, False,
		   SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

/* Add, remove, toggle on and toggle off a state that dtwm only shows. */
static void cycle_state(Window w, const char *name)
{
	send_state(w, ADD, name, NULL);
	expect_state(w, name, 1);
	send_state(w, REMOVE, name, NULL);
	expect_state(w, name, 0);
	send_state(w, TOGGLE, name, NULL);
	expect_state(w, name, 1);
	send_state(w, TOGGLE, name, NULL);
	expect_state(w, name, 0);
}

/* The dtwm frame of a managed window: its ancestor that is a root child. */
static Window frame_of(Window w)
{
	Window r, parent, *kids;
	unsigned int nkids;

	for (;;) {
		if (!XQueryTree(dpy, w, &r, &parent, &kids, &nkids))
			fail("XQueryTree failed");
		if (kids)
			XFree(kids);
		if (parent == rootw)
			return w;
		w = parent;
	}
}

/* Root geometry of a window. */
static void root_geometry(Window w, int *x, int *y, int *wd, int *ht)
{
	Window r, child;
	unsigned int uw, uh, bw, depth;
	int gx, gy;

	XSync(dpy, False);
	XGetGeometry(dpy, w, &r, &gx, &gy, &uw, &uh, &bw, &depth);
	XTranslateCoordinates(dpy, w, rootw, 0, 0, x, y, &child);
	*wd = (int)uw;
	*ht = (int)uh;
}

static void set_urgency(Window w, int urgent)
{
	XWMHints *h = XGetWMHints(dpy, w);

	if (!h)
		fail("no WM_HINTS");
	if (urgent)
		h->flags |= XUrgencyHint;
	else
		h->flags &= ~XUrgencyHint;
	XSetWMHints(dpy, w, h);
	XFree(h);
}

/* Map a window that has these states set beforehand. */
static Window map_with_states(const char *title, const char *first,
			      const char *second)
{
	Window w = create_window(title, None);
	Atom states[2];
	int n = 0;

	if (first)
		states[n++] = state_atom(first);
	if (second)
		states[n++] = state_atom(second);
	XChangeProperty(dpy, w, a_net_wm_state, XA_ATOM, 32, PropModeReplace,
			(unsigned char *)states, n);
	XMapWindow(dpy, w);
	wait_for_on(w, a_wm_state, has_wm_state, NULL,
		    "WM_STATE NormalState");
	expect_list(a_client_list, "_NET_CLIENT_LIST", 1, 1, w);
	return w;
}

static void test_states(void)
{
	static const char *const supported[] = {
		"MODAL", "STICKY", "MAXIMIZED_VERT", "MAXIMIZED_HORZ",
		"SKIP_TASKBAR", "SKIP_PAGER", "HIDDEN", "FULLSCREEN", "ABOVE",
		"BELOW", "DEMANDS_ATTENTION", "FOCUSED",
	};
	int sw = DisplayWidth(dpy, DefaultScreen(dpy));
	int sh = DisplayHeight(dpy, DefaultScreen(dpy));
	int wh = work_height();		/* the front panel's strut */
	int i, j, n, x, y, wd, ht, fx, fy, fw, fh;
	unsigned long *v;
	Window a, b, f, m, s;

	v = get_prop(a_supported, XA_ATOM, &n);
	for (i = 0; i < (int)(sizeof supported / sizeof supported[0]); i++) {
		for (j = 0; j < n && v[j] != state_atom(supported[i]); j++)
			;
		if (j == n)
			fail("_NET_SUPPORTED lacks _NET_WM_STATE_%s",
			     supported[i]);
	}
	for (j = 0; j < n; j++)
		if (v[j] == state_atom("SHADED"))
			fail("_NET_SUPPORTED has _NET_WM_STATE_SHADED");
	if (v)
		XFree(v);

	send_msg(rootw, a_current, 0);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");

	a = create_window("ewmhtest A", None);
	b = create_window("ewmhtest B", None);
	XMapWindow(dpy, a);
	XMapWindow(dpy, b);
	expect_list(a_client_list, "_NET_CLIENT_LIST", 0, 2, a, b);

	/* focused follows the focus */
	send_active(a);
	expect_state(a, "FOCUSED", 1);
	expect_state(b, "FOCUSED", 0);

	/* the states that dtwm only shows, each through every action */
	cycle_state(a, "SKIP_TASKBAR");
	cycle_state(a, "SKIP_PAGER");
	cycle_state(b, "DEMANDS_ATTENTION");
	cycle_state(a, "ABOVE");
	cycle_state(a, "BELOW");

	/* two unrelated states in one message */
	send_state(a, ADD, "SKIP_TASKBAR", "SKIP_PAGER");
	expect_state(a, "SKIP_TASKBAR", 1);
	expect_state(a, "SKIP_PAGER", 1);
	send_state(a, REMOVE, "SKIP_PAGER", "SKIP_TASKBAR");
	expect_state(a, "SKIP_TASKBAR", 0);
	expect_state(a, "SKIP_PAGER", 0);

	/* requests for dtwm's own states change nothing */
	send_state(a, ADD, "MODAL", "HIDDEN");
	send_state(a, REMOVE, "FOCUSED", NULL);
	send_state(a, ADD, "SKIP_PAGER", NULL);
	expect_state(a, "SKIP_PAGER", 1);
	if (has_state(a, state_atom("MODAL")) ||
	    has_state(a, state_atom("HIDDEN")) ||
	    !has_state(a, state_atom("FOCUSED")))
		fail("a request changed MODAL, HIDDEN or FOCUSED");
	send_state(a, REMOVE, "SKIP_PAGER", NULL);
	expect_state(a, "SKIP_PAGER", 0);

	/* sticky is on all desktops */
	cycle_state(a, "STICKY");
	send_state(a, ADD, "STICKY", NULL);
	expect_card(a, a_wm_desktop, ALL_DESKTOPS, "_NET_WM_DESKTOP of A");
	send_state(a, REMOVE, "STICKY", NULL);
	expect_card(a, a_wm_desktop, 0, "_NET_WM_DESKTOP of A");

	/* demands attention: the urgency hint, cleared by the focus */
	set_urgency(b, 1);
	expect_state(b, "DEMANDS_ATTENTION", 1);
	set_urgency(b, 0);
	expect_state(b, "DEMANDS_ATTENTION", 0);
	send_state(b, ADD, "DEMANDS_ATTENTION", NULL);
	expect_state(b, "DEMANDS_ATTENTION", 1);
	send_active(b);
	expect_state(b, "FOCUSED", 1);
	expect_state(b, "DEMANDS_ATTENTION", 0);
	expect_state(a, "FOCUSED", 0);

	/* maximizing: one axis, then the other, both in one message */
	root_geometry(b, &x, &y, &wd, &ht);
	send_state(b, ADD, "MAXIMIZED_VERT", NULL);
	expect_state(b, "MAXIMIZED_VERT", 1);
	if (has_state(b, state_atom("MAXIMIZED_HORZ")))
		fail("MAXIMIZED_VERT also maximized horizontally");
	root_geometry(frame_of(b), &fx, &fy, &fw, &fh);
	if (fy != 0 || fh != wh || fw >= sw)
		fail("maximized vertically, the frame is %dx%d+%d+%d",
		     fw, fh, fx, fy);
	root_geometry(b, &i, &j, &n, &fh);
	if (n != wd || i != x)
		fail("maximizing vertically changed the width or x");
	send_state(b, ADD, "MAXIMIZED_HORZ", NULL);
	expect_state(b, "MAXIMIZED_HORZ", 1);
	expect_state(b, "MAXIMIZED_VERT", 1);
	root_geometry(frame_of(b), &fx, &fy, &fw, &fh);
	if (fx != 0 || fy != 0 || fw != sw || fh != wh)
		fail("maximized, the frame is %dx%d+%d+%d", fw, fh, fx, fy);
	send_state(b, REMOVE, "MAXIMIZED_VERT", NULL);
	expect_state(b, "MAXIMIZED_VERT", 0);
	expect_state(b, "MAXIMIZED_HORZ", 1);
	send_state(b, TOGGLE, "MAXIMIZED_VERT", "MAXIMIZED_HORZ");
	expect_state(b, "MAXIMIZED_HORZ", 0);
	send_state(b, TOGGLE, "MAXIMIZED_HORZ", "MAXIMIZED_VERT");
	expect_state(b, "MAXIMIZED_VERT", 1);
	expect_state(b, "MAXIMIZED_HORZ", 1);
	send_state(b, REMOVE, "MAXIMIZED_VERT", "MAXIMIZED_HORZ");
	expect_state(b, "MAXIMIZED_VERT", 0);
	expect_state(b, "MAXIMIZED_HORZ", 0);
	root_geometry(b, &i, &j, &n, &fh);
	if (i != x || j != y || n != wd || fh != ht)
		fail("B did not get its geometry back after maximizing");

	/* full screen */
	cycle_state(b, "FULLSCREEN");

	/* hidden when iconified, not any more once restored */
	XIconifyWindow(dpy, b, DefaultScreen(dpy));
	expect_state(b, "HIDDEN", 1);
	expect_state(b, "FOCUSED", 0);
	XMapWindow(dpy, b);
	expect_state(b, "HIDDEN", 0);

	/* states set before the window is mapped */
	f = map_with_states("ewmhtest full screen", "FULLSCREEN", NULL);
	expect_state(f, "FULLSCREEN", 1);
	root_geometry(frame_of(f), &fx, &fy, &fw, &fh);
	root_geometry(f, &x, &y, &wd, &ht);
	if (fx != 0 || fy != 0 || fw != sw || fh != sh ||
	    x != 0 || y != 0 || wd != sw || ht != sh)
		fail("full screen at map: frame %dx%d+%d+%d, client %dx%d+%d+%d",
		     fw, fh, fx, fy, wd, ht, x, y);

	m = map_with_states("ewmhtest maximized vertically",
			    "MAXIMIZED_VERT", "SKIP_TASKBAR");
	expect_state(m, "MAXIMIZED_VERT", 1);
	expect_state(m, "SKIP_TASKBAR", 1);
	if (has_state(m, state_atom("MAXIMIZED_HORZ")))
		fail("MAXIMIZED_VERT at map also maximized horizontally");
	root_geometry(frame_of(m), &fx, &fy, &fw, &fh);
	root_geometry(m, &x, &y, &wd, &ht);
	if (fy != 0 || fh != wh || wd != 300)
		fail("maximized vertically at map: frame %dx%d+%d+%d, "
		     "client %dx%d", fw, fh, fx, fy, wd, ht);

	s = map_with_states("ewmhtest sticky", "STICKY", "MODAL");
	expect_card(s, a_wm_desktop, ALL_DESKTOPS, "_NET_WM_DESKTOP of S");
	expect_state(s, "STICKY", 1);
	expect_state(s, "MODAL", 1);

	/* xdotool */
	if (!system("xdotool version >/dev/null 2>&1")) {
		xdotool("windowstate --add SKIP_PAGER 0x%lx", a);
		expect_state(a, "SKIP_PAGER", 1);
		xdotool("windowstate --remove SKIP_PAGER 0x%lx", a);
		expect_state(a, "SKIP_PAGER", 0);
		xdotool("windowstate --toggle STICKY 0x%lx", a);
		expect_state(a, "STICKY", 1);
		expect_card(a, a_wm_desktop, ALL_DESKTOPS,
			    "_NET_WM_DESKTOP of A");
		xdotool("windowstate --toggle STICKY 0x%lx", a);
		expect_state(a, "STICKY", 0);
		xdotool("windowstate --add MAXIMIZED_HORZ 0x%lx", a);
		expect_state(a, "MAXIMIZED_HORZ", 1);
		expect_state(a, "MAXIMIZED_VERT", 0);
		xdotool("windowstate --remove MAXIMIZED_HORZ 0x%lx", a);
		expect_state(a, "MAXIMIZED_HORZ", 0);
	}

	destroy_all();
	printf("states: ok\n");
}

/* ------------------------------------------------------------------ */
/* Layers and window types                                             */
/* ------------------------------------------------------------------ */

static Atom type_atom(const char *name)
{
	char full[64];

	snprintf(full, sizeof full, "_NET_WM_WINDOW_TYPE_%s", name);
	return XInternAtom(dpy, full, False);
}

/*
 * A window of a type (or NULL), with these states set, at a position
 * given as a program-specified one, in a window group (or None).
 */
static Window create_typed(const char *title, const char *type,
			   const char *state, int x, int y, int w, int h,
			   Window group)
{
	Window win = create_window(title, None);
	XSizeHints *sh = XAllocSizeHints();
	XWMHints *wmh = XGetWMHints(dpy, win);
	Atom a;

	if (type) {
		a = type_atom(type);
		XChangeProperty(dpy, win,
				XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False),
				XA_ATOM, 32, PropModeReplace,
				(unsigned char *)&a, 1);
	}
	if (state) {
		a = state_atom(state);
		XChangeProperty(dpy, win, a_net_wm_state, XA_ATOM, 32,
				PropModeReplace, (unsigned char *)&a, 1);
	}
	XMoveResizeWindow(dpy, win, x, y, w, h);
	sh->flags = PPosition | PSize;
	sh->x = x;
	sh->y = y;
	sh->width = w;
	sh->height = h;
	XSetWMNormalHints(dpy, win, sh);
	XFree(sh);
	if (group && wmh) {
		wmh->flags |= WindowGroupHint;
		wmh->window_group = group;
		XSetWMHints(dpy, win, wmh);
	}
	if (wmh)
		XFree(wmh);
	return win;
}

static Window map_managed(Window w)
{
	XMapWindow(dpy, w);
	wait_for_on(w, a_wm_state, has_wm_state, NULL,
		    "WM_STATE NormalState");
	expect_list(a_client_list, "_NET_CLIENT_LIST", 1, 1, w);
	return w;
}

/*
 * The X stacking order of our frames, bottom to top, must be the one of
 * _NET_CLIENT_LIST_STACKING (our windows in both).
 */
static void check_x_order(void)
{
	Window r, parent, *kids, frames[MAX_OURS], xorder[MAX_OURS];
	unsigned int nkids, i;
	unsigned long *v;
	int j, k, n, nx = 0, nl = 0;
	Window listed[MAX_OURS];

	XSync(dpy, False);
	for (j = 0; j < nours; j++)
		frames[j] = frame_of(ours[j]);
	if (!XQueryTree(dpy, rootw, &r, &parent, &kids, &nkids))
		fail("XQueryTree failed");
	for (i = 0; i < nkids; i++)
		for (j = 0; j < nours; j++)
			if (kids[i] == frames[j] && frames[j] != ours[j] &&
			    nx < MAX_OURS)
				xorder[nx++] = ours[j];

	v = get_prop(a_stacking, XA_WINDOW, &n);
	for (k = 0; k < n; k++)
		if (is_ours(v[k]) && nl < MAX_OURS)
			listed[nl++] = v[k];
	if (v)
		XFree(v);

	if (nx != nl || memcmp(xorder, listed, nx * sizeof(Window))) {
		fprintf(stderr, "ewmhtest: X order:");
		for (j = 0; j < nx; j++)
			fprintf(stderr, " 0x%lx", xorder[j]);
		fprintf(stderr, "\nmapped root children, bottom to top:");
		for (i = 0; i < nkids; i++) {
			XWindowAttributes wa;

			if (!XGetWindowAttributes(dpy, kids[i], &wa) ||
			    wa.map_state != IsViewable)
				continue;
			for (j = 0; j < nours && kids[i] != frames[j]; j++)
				;
			if (j < nours)
				fprintf(stderr, " [0x%lx]", ours[j]);
			else
				fprintf(stderr, " %s0x%lx",
					wa.override_redirect ? "o:" : "",
					kids[i]);
		}
		fprintf(stderr, "\n");
		print_list("_NET_CLIENT_LIST_STACKING", a_stacking);
		fail("the X stacking order is not _NET_CLIENT_LIST_STACKING");
	}
	if (kids)
		XFree(kids);
}

#define expect_stack(n, ...) do { \
	expect_list(a_stacking, "_NET_CLIENT_LIST_STACKING", 0, n, \
		    __VA_ARGS__); \
	check_x_order(); \
} while (0)

static int not_listed(Window w, Atom prop, const void *arg)
{
	Window gone = *(const Window *)arg;
	unsigned long *v;
	int i, n, found = 0;

	v = get_prop_on(w, prop, XA_WINDOW, &n);
	for (i = 0; i < n; i++)
		if (v[i] == gone)
			found = 1;
	if (v)
		XFree(v);
	return !found;
}

static void test_layers(void)
{
	Window n1, n2, a, b, d, k, f, l, t, s;
	long hints[5] = { 1L << 2, 0, 0, 2 /* system modal */, 0 };
	Atom mwm_hints = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
	int have_xdotool = !system("xdotool version >/dev/null 2>&1");

	send_msg(rootw, a_current, 0);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");

	n1 = map_managed(create_window("ewmhtest N1", None));
	n2 = map_managed(create_window("ewmhtest N2", None));
	a = map_managed(create_typed("ewmhtest A", NULL, "ABOVE",
				     100, 100, 300, 200, None));
	b = map_managed(create_typed("ewmhtest B", NULL, "BELOW",
				     150, 150, 300, 200, None));
	expect_state(a, "ABOVE", 1);
	expect_state(b, "BELOW", 1);
	expect_stack(4, b, n1, n2, a);

	/* raising normal windows leaves them between below and above */
	XRaiseWindow(dpy, n1);
	expect_stack(4, b, n2, n1, a);
	if (have_xdotool) {
		xdotool("windowraise 0x%lx", n2);
		expect_stack(4, b, n1, n2, a);
		xdotool("windowactivate --sync 0x%lx", n1);
		expect_stack(4, b, n2, n1, a);
	} else {
		XRaiseWindow(dpy, n2);
		expect_stack(4, b, n1, n2, a);
		send_active(n1);
		expect_stack(4, b, n2, n1, a);
	}

	/* raising the below one, lowering the above one: they stay */
	XRaiseWindow(dpy, b);
	XLowerWindow(dpy, a);
	XRaiseWindow(dpy, n2);			/* a fence */
	expect_stack(4, b, n1, n2, a);
	send_active(b);
	expect_state(b, "FOCUSED", 1);
	XRaiseWindow(dpy, n1);
	expect_stack(4, b, n2, n1, a);

	/* changing layers */
	send_state(n2, ADD, "ABOVE", NULL);
	expect_stack(4, b, n1, a, n2);
	send_state(n2, REMOVE, "ABOVE", NULL);
	expect_stack(4, b, n1, n2, a);
	send_state(n2, ADD, "BELOW", NULL);
	expect_stack(4, n2, b, n1, a);
	send_state(n2, REMOVE, "BELOW", NULL);
	expect_stack(4, b, n2, n1, a);

	/* a dock above all those, a desktop window below them */
	d = map_managed(create_typed("ewmhtest D", "DOCK", NULL,
				     0, 1000, 1920, 40, None));
	expect_stack(5, b, n2, n1, a, d);
	k = map_managed(create_typed("ewmhtest K", "DESKTOP", NULL,
				     0, 0, 1920, 1080, None));
	expect_stack(6, k, b, n2, n1, a, d);
	XRaiseWindow(dpy, k);
	XRaiseWindow(dpy, a);
	XLowerWindow(dpy, d);
	XRaiseWindow(dpy, n2);			/* a fence */
	expect_stack(6, k, b, n1, n2, a, d);
	if (have_xdotool) {
		xdotool("windowraise 0x%lx", k);
		xdotool("windowraise 0x%lx", n1);
		expect_stack(6, k, b, n2, n1, a, d);
	}

	/* full screen: above the dock while it has the focus */
	f = map_managed(create_window("ewmhtest F", None));
	send_state(f, ADD, "FULLSCREEN", NULL);
	expect_state(f, "FULLSCREEN", 1);
	send_active(f);
	expect_state(f, "FOCUSED", 1);
	expect_stack(7, k, b, n2, n1, a, d, f);
	send_active(n1);
	expect_state(n1, "FOCUSED", 1);
	expect_stack(7, k, b, n2, f, n1, a, d);
	send_active(f);
	expect_stack(7, k, b, n2, n1, a, d, f);
	send_state(f, REMOVE, "FULLSCREEN", NULL);
	expect_stack(7, k, b, n2, n1, f, a, d);

	/* a transient stays above its leader, and comes with it */
	l = map_managed(create_window("ewmhtest L", None));
	t = map_managed(create_window("ewmhtest T, transient for L", l));
	expect_stack(9, k, b, n2, n1, f, l, t, a, d);
	XRaiseWindow(dpy, n1);
	expect_stack(9, k, b, n2, f, l, t, n1, a, d);
	XRaiseWindow(dpy, l);
	expect_stack(9, k, b, n2, f, n1, l, t, a, d);
	XLowerWindow(dpy, l);
	expect_stack(9, k, b, l, t, n2, f, n1, a, d);
	XRaiseWindow(dpy, t);
	expect_stack(9, k, b, n2, f, n1, l, t, a, d);

	/* a system modal window is above everything */
	s = create_window("ewmhtest S, system modal", None);
	XChangeProperty(dpy, s, mwm_hints, mwm_hints, 32, PropModeReplace,
			(unsigned char *)hints, 5);
	map_managed(s);
	expect_stack(10, k, b, n2, f, n1, l, t, a, d, s);
	/* while it is up, the others are not raised */
	XRaiseWindow(dpy, d);
	XRaiseWindow(dpy, n1);
	destroy_window(s);
	wait_for(a_client_list, not_listed, &s, "S out of _NET_CLIENT_LIST");
	expect_stack(9, k, b, n2, f, n1, l, t, a, d);
	XRaiseWindow(dpy, n1);
	expect_stack(9, k, b, n2, f, l, t, n1, a, d);

	destroy_all();
	printf("layers: ok\n");
}

/* The frame has decorations: it is larger than the client. */
static int decorated(Window w)
{
	int x, y, wd, ht, fx, fy, fw, fh;

	root_geometry(w, &x, &y, &wd, &ht);
	root_geometry(frame_of(w), &fx, &fy, &fw, &fh);
	return fw > wd || fh > ht;
}

static void test_types(void)
{
	static const struct {
		const char *type;
		int decorated, focus, all, position;
	} types[] = {
		{ NULL,            1, 1, 0, 0 },
		{ "NORMAL",        1, 1, 0, 0 },
		{ "DIALOG",        1, 1, 0, 0 },
		{ "DESKTOP",       0, 0, 1, 1 },
		{ "DOCK",          0, 0, 1, 1 },
		{ "TOOLBAR",       1, 0, 0, 0 },
		{ "MENU",          1, 0, 0, 0 },
		{ "UTILITY",       1, 0, 0, 0 },
		{ "SPLASH",        0, 0, 0, 0 },
		{ "DROPDOWN_MENU", 0, 0, 0, 1 },
		{ "POPUP_MENU",    0, 0, 0, 1 },
		{ "TOOLTIP",       0, 0, 0, 1 },
		{ "NOTIFICATION",  0, 0, 1, 1 },
		{ "COMBO",         0, 0, 0, 1 },
		{ "DND",           0, 0, 0, 1 },
	};
	int sw = DisplayWidth(dpy, DefaultScreen(dpy));
	int sh = DisplayHeight(dpy, DefaultScreen(dpy));
	int i, x, y, wd, ht;
	unsigned long *v;
	Window w, focus, g, dlg;
	char title[64];

	v = get_prop(a_supported, XA_ATOM, &i);
	for (x = 0; x < (int)(sizeof types / sizeof types[0]); x++) {
		if (!types[x].type)
			continue;
		for (y = 0; y < i && v[y] != type_atom(types[x].type); y++)
			;
		if (y == i)
			fail("_NET_SUPPORTED lacks _NET_WM_WINDOW_TYPE_%s",
			     types[x].type);
	}
	if (v)
		XFree(v);

	send_msg(rootw, a_current, 0);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");
	focus = map_managed(create_window("ewmhtest focus", None));
	send_active(focus);
	wait_for(a_active, active_is, &focus, "_NET_ACTIVE_WINDOW = focus");

	for (i = 0; i < (int)(sizeof types / sizeof types[0]); i++) {
		const char *name = types[i].type ? types[i].type : "(none)";

		snprintf(title, sizeof title, "ewmhtest type %s", name);
		w = map_managed(create_typed(title, types[i].type, NULL,
					     123, 234, 320, 160, None));
		if (types[i].focus) {
			wait_for(a_active, active_is, &w, title);
		} else {
			/* the fence: its _NET_WM_DESKTOP is written last */
			expect_card(w, a_wm_desktop,
				    types[i].all ? ALL_DESKTOPS : 0,
				    "_NET_WM_DESKTOP");
			if (active_is(rootw, a_active, &w))
				fail("%s: got the focus when mapped", name);
		}
		expect_card(w, a_wm_desktop, types[i].all ? ALL_DESKTOPS : 0,
			    "_NET_WM_DESKTOP");
		if (decorated(w) != types[i].decorated)
			fail("%s: %sdecorated", name,
			     types[i].decorated ? "not " : "");
		root_geometry(w, &x, &y, &wd, &ht);
		if (types[i].position && (x != 123 || y != 234))
			fail("%s: at %d,%d, not 123,234", name, x, y);
		if (types[i].type && !strcmp(types[i].type, "SPLASH") &&
		    (x != (sw - 320) / 2 || y != (sh - 160) / 2))
			fail("SPLASH: at %d,%d, not centred", x, y);
		destroy_window(w);
		send_active(focus);
		wait_for(a_active, active_is, &focus,
			 "_NET_ACTIVE_WINDOW = focus");
	}

	/* a dialog that is no transient becomes one of its window group */
	g = map_managed(create_window("ewmhtest G", None));
	dlg = map_managed(create_typed("ewmhtest dialog of G", "DIALOG", NULL,
				       200, 200, 200, 100, g));
	XRaiseWindow(dpy, focus);
	expect_stack(3, g, dlg, focus);
	XRaiseWindow(dpy, g);
	expect_stack(3, focus, g, dlg);

	destroy_all();
	printf("types: ok\n");
}

/* ------------------------------------------------------------------ */
/* Struts and the work area                                            */
/* ------------------------------------------------------------------ */

static int panel_strut(void)
{
	int i;

	if (!front_panel)
		return 0;
	for (i = 0; i < nxrm; i++)
		if (strstr(xrm[i], "frontPanelStrut") &&
		    (strstr(xrm[i], "True") || strstr(xrm[i], "true")))
			return 1;
	return 0;
}

/* Set a strut: 12 values as _NET_WM_STRUT_PARTIAL, 4 as _NET_WM_STRUT,
 * none to remove both. */
static void set_strut(Window w, int n, const long *strut)
{
	Atom partial = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
	Atom plain = XInternAtom(dpy, "_NET_WM_STRUT", False);

	if (n == 12)
		XChangeProperty(dpy, w, partial, XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *)strut, 12);
	else if (n == 4)
		XChangeProperty(dpy, w, plain, XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *)strut, 4);
	else {
		XDeleteProperty(dpy, w, partial);
		XDeleteProperty(dpy, w, plain);
	}
}

struct workarea {
	int desktop;
	long x, y, w, h;
};

static int workarea_is(Window w, Atom prop, const void *arg)
{
	const struct workarea *e = arg;
	unsigned long *v;
	int n, ok;

	v = get_prop_on(w, prop, XA_CARDINAL, &n);
	ok = n >= 4 * (e->desktop + 1) &&
	     (long)v[4 * e->desktop] == e->x &&
	     (long)v[4 * e->desktop + 1] == e->y &&
	     (long)v[4 * e->desktop + 2] == e->w &&
	     (long)v[4 * e->desktop + 3] == e->h;
	if (v)
		XFree(v);
	return ok;
}

static void expect_workarea(int desktop, long x, long y, long w, long h)
{
	struct workarea e = { desktop, x, y, w, h };
	char what[128];

	snprintf(what, sizeof what, "_NET_WORKAREA of desktop %d = "
		 "%ld,%ld %ldx%ld", desktop, x, y, w, h);
	wait_for(XInternAtom(dpy, "_NET_WORKAREA", False), workarea_is, &e,
		 what);
}

/* The root geometry of a client's frame. */
static void frame_geometry(Window w, int *x, int *y, int *wd, int *ht)
{
	root_geometry(frame_of(w), x, y, wd, ht);
}

/* Wait until the frame of w has this bottom edge (and is maximized). */
static void expect_frame_bottom(Window w, int bottom, const char *what)
{
	double deadline = bench_now_ns() + TIMEOUT_MS * 1e6;
	int x, y, wd, ht;

	for (;;) {
		frame_geometry(w, &x, &y, &wd, &ht);
		if (y + ht == bottom)
			return;
		if (bench_now_ns() > deadline)
			fail("%s: the frame is %dx%d+%d+%d, its bottom is not %d",
			     what, wd, ht, x, y, bottom);
		wait_notify(w, a_net_wm_state, bench_now_ns() + 100 * 1e6);
	}
}

static void test_struts(void)
{
	int sw = DisplayWidth(dpy, DefaultScreen(dpy));
	int sh = DisplayHeight(dpy, DefaultScreen(dpy));
	long bottom40[12] = { 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, sw - 1 };
	long bottom80[12] = { 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, sw - 1 };
	long sides[4] = { 30, 10, 20, 0 };
	long left50[12] = { 50, 0, 0, 0, 0, sh - 1, 0, 0, 0, 0, 0, 0 };
	Window d, m, m2, f, s, l, p;
	int i, x, y, wd, ht;

	if (panel_strut()) {
		printf("struts: skipped (the front panel has a strut)\n");
		return;
	}
	send_msg(rootw, a_number, 4);
	expect_desktops(4);
	send_msg(rootw, a_current, 0);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");
	for (i = 0; i < 4; i++)
		expect_workarea(i, 0, 0, sw, sh);

	/* a dock along the bottom: on every desktop */
	d = create_typed("ewmhtest dock", "DOCK", NULL, 0, sh - 40, sw, 40,
			 None);
	set_strut(d, 12, bottom40);
	map_managed(d);
	for (i = 0; i < 4; i++)
		expect_workarea(i, 0, 0, sw, sh - 40);

	/* maximized through a message, through xdotool */
	m = map_managed(create_window("ewmhtest M", None));
	send_state(m, ADD, "MAXIMIZED_VERT", "MAXIMIZED_HORZ");
	expect_frame_bottom(m, sh - 40, "maximized M");
	m2 = map_managed(create_window("ewmhtest M2", None));
	if (!system("xdotool version >/dev/null 2>&1")) {
		xdotool("windowstate --add MAXIMIZED_VERT 0x%lx", m2);
		expect_frame_bottom(m2, sh - 40, "M2 maximized by xdotool");
	}

	/* full screen covers the strut */
	f = map_managed(create_window("ewmhtest F", None));
	send_state(f, ADD, "FULLSCREEN", NULL);
	expect_frame_bottom(f, sh, "full screen F");
	frame_geometry(f, &x, &y, &wd, &ht);
	if (x != 0 || y != 0 || wd != sw)
		fail("full screen F is %dx%d+%d+%d", wd, ht, x, y);
	destroy_window(f);

	/* a new window is placed above the strut */
	p = create_window("ewmhtest tall", None);
	XResizeWindow(dpy, p, 300, sh - 100);
	{
		XSizeHints *hints = XAllocSizeHints();

		hints->flags = PSize;
		hints->width = 300;
		hints->height = sh - 100;
		XSetWMNormalHints(dpy, p, hints);
		XFree(hints);
	}
	map_managed(p);
	frame_geometry(p, &x, &y, &wd, &ht);
	if (y + ht > sh - 40)
		fail("a new window was placed over the strut "
		     "(frame %dx%d+%d+%d)", wd, ht, x, y);
	destroy_window(p);

	/* a larger strut, then none: the maximized windows follow */
	set_strut(d, 12, bottom80);
	expect_workarea(0, 0, 0, sw, sh - 80);
	expect_frame_bottom(m, sh - 80, "maximized M, strut 80");
	set_strut(d, 0, NULL);
	expect_workarea(0, 0, 0, sw, sh);
	expect_workarea(3, 0, 0, sw, sh);
	expect_frame_bottom(m, sh, "maximized M, no strut");
	set_strut(d, 12, bottom40);
	expect_workarea(0, 0, 0, sw, sh - 40);
	expect_frame_bottom(m, sh - 40, "maximized M, strut 40 again");

	/* left, top and right struts (_NET_WM_STRUT) */
	s = map_managed(create_window("ewmhtest sides", None));
	set_strut(s, 4, sides);
	expect_workarea(0, 30, 20, sw - 40, sh - 60);
	frame_geometry(m, &x, &y, &wd, &ht);
	if (x != 30 || y != 20 || x + wd != sw - 10 || y + ht != sh - 40)
		fail("maximized M is %dx%d+%d+%d with struts 30,10,20,40",
		     wd, ht, x, y);
	destroy_window(s);
	expect_workarea(0, 0, 0, sw, sh - 40);

	/* single axis: refitted along that axis only */
	send_state(m, REMOVE, "MAXIMIZED_HORZ", NULL);
	expect_state(m, "MAXIMIZED_HORZ", 0);
	frame_geometry(m, &x, &y, &wd, &ht);
	set_strut(d, 12, bottom80);
	expect_frame_bottom(m, sh - 80, "M maximized vertically, strut 80");
	{
		int x2, y2, wd2, ht2;

		frame_geometry(m, &x2, &y2, &wd2, &ht2);
		if (x2 != x || wd2 != wd)
			fail("refitting M changed its width or x");
	}

	/* a window with a strut on desktop 1 only */
	l = create_window("ewmhtest left on 1", None);
	{
		long one = 1;

		XChangeProperty(dpy, l, a_wm_desktop, XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *)&one, 1);
	}
	set_strut(l, 12, left50);
	map_managed(l);
	expect_workarea(1, 50, 0, sw - 50, sh - 80);
	expect_workarea(0, 0, 0, sw, sh - 80);
	expect_workarea(2, 0, 0, sw, sh - 80);
	XIconifyWindow(dpy, l, DefaultScreen(dpy));	/* not while iconic */
	expect_workarea(1, 0, 0, sw, sh - 80);

	destroy_all();
	for (i = 0; i < 4; i++)
		expect_workarea(i, 0, 0, sw, sh);
	printf("struts: ok\n");
}

static void test_xinerama(void)
{
	int sw = DisplayWidth(dpy, DefaultScreen(dpy));
	int sh = DisplayHeight(dpy, DefaultScreen(dpy));
	int hw = sw / 2;
	long inner[12] = { 0, hw + 40, 0, 0, 0, 0, 0, sh - 1, 0, 0, 0, 0 };
	Window d, left, right;
	int x, y, wd, ht;

	if (sw != 2 * 1920 || sh != 1080) {
		printf("xinerama: skipped (not two 1920x1080 heads)\n");
		return;
	}
	send_msg(rootw, a_current, 0);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");

	/* a dock on the right edge of the left head */
	d = create_typed("ewmhtest inner dock", "DOCK", NULL, hw - 40, 0,
			 40, sh, None);
	set_strut(d, 12, inner);
	map_managed(d);

	left = map_managed(create_typed("ewmhtest left", NULL, NULL,
					100, 100, 300, 200, None));
	right = map_managed(create_typed("ewmhtest right", NULL, NULL,
					 hw + 100, 100, 300, 200, None));
	send_state(left, ADD, "MAXIMIZED_VERT", "MAXIMIZED_HORZ");
	send_state(right, ADD, "MAXIMIZED_VERT", "MAXIMIZED_HORZ");
	expect_state(left, "MAXIMIZED_HORZ", 1);
	expect_state(right, "MAXIMIZED_HORZ", 1);
	XSync(dpy, False);
	frame_geometry(left, &x, &y, &wd, &ht);
	if (x != 0 || x + wd != hw - 40 || ht != sh)
		fail("maximized on the left head: %dx%d+%d+%d", wd, ht, x, y);
	frame_geometry(right, &x, &y, &wd, &ht);
	if (x != hw || wd != hw || ht != sh)
		fail("maximized on the right head: %dx%d+%d+%d", wd, ht, x, y);

	destroy_all();
	printf("xinerama: ok\n");
}

static int workarea_short(Window w, Atom prop, const void *arg)
{
	unsigned long *v;
	int n, ok;

	v = get_prop_on(w, prop, XA_CARDINAL, &n);
	ok = n >= 4 && (long)v[3] < *(const int *)arg;
	if (v)
		XFree(v);
	return ok;
}

static void test_panel(void)
{
	int sw = DisplayWidth(dpy, DefaultScreen(dpy));
	int sh = DisplayHeight(dpy, DefaultScreen(dpy));
	unsigned long *v;
	int n, x, y, wd, ht;
	long area[4];
	Window m;

	if (!front_panel) {
		printf("panel: skipped (no front panel, -f)\n");
		return;
	}
	send_msg(rootw, a_current, 0);
	expect_card(rootw, a_current, 0, "_NET_CURRENT_DESKTOP");
	if (panel_strut())		/* once the panel is up */
		wait_for(XInternAtom(dpy, "_NET_WORKAREA", False),
			 workarea_short, &sh, "_NET_WORKAREA without the "
			 "front panel");
	v = get_prop(XInternAtom(dpy, "_NET_WORKAREA", False), XA_CARDINAL,
		     &n);
	if (n < 4)
		fail("no _NET_WORKAREA");
	for (n = 0; n < 4; n++)
		area[n] = v[n];
	XFree(v);

	if (!panel_strut()) {
		if (area[0] || area[1] || area[2] != sw || area[3] != sh)
			fail("_NET_WORKAREA is %ld,%ld %ldx%ld without "
			     "frontPanelStrut", area[0], area[1], area[2],
			     area[3]);
		printf("panel: ok (no strut)\n");
		return;
	}
	/* the panel is at the bottom of the screen */
	if (area[0] || area[1] || area[2] != sw || area[3] >= sh ||
	    area[3] < sh / 2)
		fail("_NET_WORKAREA is %ld,%ld %ldx%ld with frontPanelStrut",
		     area[0], area[1], area[2], area[3]);
	m = map_managed(create_window("ewmhtest M", None));
	send_state(m, ADD, "MAXIMIZED_VERT", "MAXIMIZED_HORZ");
	expect_frame_bottom(m, area[3], "maximized M above the panel");
	frame_geometry(m, &x, &y, &wd, &ht);
	destroy_all();
	printf("panel: ok (strut %ld)\n", sh - area[3]);
}

static const struct {
	const char *name;
	void (*run)(void);
} tests[] = {
	{ "clientlist", test_clientlist },
	{ "active", test_active },
	{ "desktops", test_desktops },
	{ "xdotool", test_xdotool },
	{ "states", test_states },
	{ "layers", test_layers },
	{ "types", test_types },
	{ "struts", test_struts },
	{ "xinerama", test_xinerama },
	{ "panel", test_panel },
};

#define NTESTS ((int)(sizeof tests / sizeof tests[0]))

static void usage(FILE *f)
{
	int i;

	fprintf(f, "usage: ewmhtest [-f] [-m DTWM] [-x RESOURCE]... "
		   "TEST...\ntests: all");
	for (i = 0; i < NTESTS; i++)
		fprintf(f, " %s", tests[i].name);
	fputc('\n', f);
}

int main(int argc, char **argv)
{
	int opt, i, j;

	while ((opt = getopt(argc, argv, "fm:x:h")) != -1) {
		switch (opt) {
		case 'f':
			front_panel = 1;
			break;
		case 'm':
			wm_path = optarg;
			break;
		case 'x':
			if (nxrm == MAX_XRM)
				fail("too many -x");
			xrm[nxrm++] = optarg;
			break;
		case 'h':
			usage(stdout);
			return 0;
		default:
			usage(stderr);
			return 2;
		}
	}
	if (optind == argc) {
		usage(stderr);
		return 2;
	}
	for (i = optind; i < argc; i++) {
		if (!strcmp(argv[i], "all"))
			continue;
		for (j = 0; j < NTESTS; j++)
			if (!strcmp(argv[i], tests[j].name))
				break;
		if (j == NTESTS) {
			usage(stderr);
			return 2;
		}
	}

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ewmhtest: cannot open the display\n");
		return BENCH_SKIP;
	}
	XSetErrorHandler(x_error);
	rootw = DefaultRootWindow(dpy);
	a_supported = XInternAtom(dpy, "_NET_SUPPORTED", False);
	a_check = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	a_client_list = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	a_stacking = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
	a_active = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	a_number = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
	a_current = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
	a_names = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
	a_viewport = XInternAtom(dpy, "_NET_DESKTOP_VIEWPORT", False);
	a_geometry = XInternAtom(dpy, "_NET_DESKTOP_GEOMETRY", False);
	a_wm_desktop = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
	a_wm_state = XInternAtom(dpy, "WM_STATE", False);
	a_utf8 = XInternAtom(dpy, "UTF8_STRING", False);
	a_dt_wm_request = XInternAtom(dpy, "_DT_WM_REQUEST", False);
	a_net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	XSelectInput(dpy, rootw, PropertyChangeMask);

	atexit(cleanup);
	signal(SIGINT, on_signal);
	signal(SIGTERM, on_signal);
	start_wm();

	for (i = optind; i < argc; i++)
		for (j = 0; j < NTESTS; j++)
			if (!strcmp(argv[i], "all") ||
			    !strcmp(argv[i], tests[j].name))
				tests[j].run();
	return 0;
}
