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

static const struct {
	const char *name;
	void (*run)(void);
} tests[] = {
	{ "clientlist", test_clientlist },
	{ "active", test_active },
	{ "desktops", test_desktops },
	{ "xdotool", test_xdotool },
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
