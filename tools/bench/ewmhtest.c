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

/* A 32-bit property of the root window, or NULL (*n = 0) when absent. */
static unsigned long *get_prop(Atom prop, Atom type, int *n)
{
	Atom rtype;
	int format;
	unsigned long nitems, after;
	unsigned char *data = NULL;

	*n = 0;
	if (XGetWindowProperty(dpy, rootw, prop, 0, 65536, False, type,
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
 * Handle events until a PropertyNotify of prop on the root window, or
 * until the deadline.  Returns 0 on timeout.  *counted, if counted is not
 * NULL, counts the PropertyNotify events of counted_prop.
 */
static Atom counted_prop;
static int *counted;

static int wait_notify(Atom prop, double deadline)
{
	struct pollfd pfd = { ConnectionNumber(dpy), POLLIN, 0 };
	XEvent ev;

	XFlush(dpy);
	for (;;) {
		while (XPending(dpy)) {
			XNextEvent(dpy, &ev);
			if (ev.type != PropertyNotify ||
			    ev.xproperty.window != rootw)
				continue;
			if (counted && ev.xproperty.atom == counted_prop)
				++*counted;
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
 * Wait until check() holds of the root window property prop, rechecking
 * at each of its PropertyNotify events.
 */
static void wait_for(Atom prop, int (*check)(Atom prop, const void *arg),
		     const void *arg, const char *what)
{
	double deadline = bench_now_ns() + TIMEOUT_MS * 1e6;

	XSync(dpy, False);
	while (!check(prop, arg)) {
		if (!wait_notify(prop, deadline)) {
			fprintf(stderr, "ewmhtest: timed out waiting for %s\n",
				what);
			print_list("_NET_CLIENT_LIST", a_client_list);
			print_list("_NET_CLIENT_LIST_STACKING", a_stacking);
			print_list("_NET_ACTIVE_WINDOW", a_active);
			fail("%s", what);
		}
	}
}

/* An expected list of windows: exactly these, or ending with them. */
struct expect {
	const Window *w;
	int n;
	int suffix;
};

/* The list holds our windows (others ignored) as expected. */
static int list_is(Atom prop, const void *arg)
{
	const struct expect *e = arg;
	unsigned long *v;
	int i, n, k = 0, ok;
	Window mine[MAX_OURS];

	v = get_prop(prop, XA_WINDOW, &n);
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

static int active_is(Atom prop, const void *arg)
{
	Window want = *(const Window *)arg;
	unsigned long *v;
	int n, ok;

	v = get_prop(prop, XA_WINDOW, &n);
	ok = n == 1 && v[0] == want;
	if (v)
		XFree(v);
	return ok;
}

static int active_is_not(Atom prop, const void *arg)
{
	Window w = *(const Window *)arg;
	unsigned long *v;
	int n, ok;

	v = get_prop(prop, XA_WINDOW, &n);
	ok = n == 1 && v[0] != w;
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
		wait_notify(a_client_list, bench_now_ns() + 100 * 1e6);
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
	swa.event_mask = StructureNotifyMask;
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
	if (!active_is(a_active, &b))
		fail("_NET_ACTIVE_WINDOW is no longer B");

	/* an iconified window is not active */
	XIconifyWindow(dpy, b, DefaultScreen(dpy));
	wait_for(a_active, active_is_not, &b,
		 "_NET_ACTIVE_WINDOW not to be B once iconified");

	destroy_all();
	printf("active: ok\n");
}

static const struct {
	const char *name;
	void (*run)(void);
} tests[] = {
	{ "clientlist", test_clientlist },
	{ "active", test_active },
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
