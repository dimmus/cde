/**
 * CDE benchmarks (from Motif src/tests/bench/mwmpreload.c)
 *
 * Licensed under the LGPL 2.1 license.
 *
 * LD_PRELOAD probe for dtwmbench, loaded into the dtwm under test.
 *
 * dtwmbench passes a memfd in DTWMBENCH_SHM_FD; this library maps it and
 * keeps there (see struct dtwmbench_shm in dtwmbench.h):
 *   - the calls to the allocator and to _XReply (round trips), as the
 *     xmbench preload does;
 *   - the number of X requests dtwm had issued when it last went idle;
 *   - a sequence number bumped, with a futex wake, every time dtwm blocks
 *     in the Xt event loop with nothing left to do, so that dtwmbench can
 *     tell when dtwm has finished processing what it was sent;
 *   - the position the last XQueryPointer returned, also with a futex
 *     wake: dtwm queries the pointer once for every motion hint during an
 *     interactive move or resize, which is how dtwmbench follows a drag
 *     step by step while dtwm holds its own modal event loop;
 *   - a sequence number bumped, with a futex wake, by every successful
 *     XGrabPointer: dtwm grabs the pointer when a drag passes its move
 *     threshold, and only then reports motion as hints.
 *   - the front panel's workspace switch buttons and where they are, so
 *     that dtwmbench can click them (they are gadgets: no window of
 *     their own to look for);
 *   - and, on request, it releases button 1 from inside dtwm (through
 *     XTest on dtwm's own connection), which ends a drag while dtwm
 *     holds the server grabbed (freezeOnConfig).
 * Without DTWMBENCH_SHM_FD the counters go to a private copy, so dtwm runs
 * normally.
 *
 * With DTWMBENCH_REPLY_BACKTRACE set, every _XReply prints a backtrace to
 * stderr (dtwm's log), to find where the round trips of a case come from.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <execinfo.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/futex.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#include "dtwmbench.h"

extern void *__libc_malloc(size_t);
extern void *__libc_calloc(size_t, size_t);
extern void *__libc_realloc(void *, size_t);
extern void __libc_free(void *);

static struct dtwmbench_shm private_shm;
static struct dtwmbench_shm *shm = &private_shm;

#define MAX_DISPLAYS 4
static Display *displays[MAX_DISPLAYS];

static void on_wake(int sig);

__attribute__((constructor))
static void dtwmbench_preload_init(void)
{
	const char *s = getenv("DTWMBENCH_SHM_FD");
	void *p;
	int fd;

	if (!s || !*s)
		return;
	fd = atoi(s);
	p = mmap(NULL, sizeof *shm, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	unsetenv("DTWMBENCH_SHM_FD");
	if (p != MAP_FAILED) {
		struct sigaction sa;

		shm = p;
		memset(&sa, 0, sizeof sa);
		sa.sa_handler = on_wake;
		sigemptyset(&sa.sa_mask);
		sigaction(DTWMBENCH_WAKE_SIGNAL, &sa, NULL);
	}
}

static void wake(unsigned int *word)
{
	syscall(SYS_futex, word, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
}

void *malloc(size_t size)
{
	shm->mallocs++;
	return __libc_malloc(size);
}

void *calloc(size_t nmemb, size_t size)
{
	shm->mallocs++;
	return __libc_calloc(nmemb, size);
}

void *realloc(void *ptr, size_t size)
{
	/* Count only the reallocs that may move the block. */
	if (size)
		shm->mallocs++;
	return __libc_realloc(ptr, size);
}

void free(void *ptr)
{
	__libc_free(ptr);
}

/* Status _XReply(Display *, xReply *, int, Bool), from Xlibint.h */
typedef int (*xreply_fn)(Display *, void *, int, int);

int _XReply(Display *dpy, void *rep, int extra, int discard);

int _XReply(Display *dpy, void *rep, int extra, int discard)
{
	static xreply_fn real;
	static int trace = -1;

	if (!real)
		real = (xreply_fn)dlsym(RTLD_NEXT, "_XReply");
	if (trace < 0)
		trace = getenv("DTWMBENCH_REPLY_BACKTRACE") != NULL;
	if (trace) {
		void *bt[32];

		backtrace_symbols_fd(bt, backtrace(bt, 32), 2);
		if (write(2, "--\n", 3) < 0)
			trace = 0;
	}
	shm->replies++;
	return real(dpy, rep, extra, discard);
}

typedef Display *(*opendisplay_fn)(const char *);

Display *XOpenDisplay(const char *name)
{
	static opendisplay_fn real;
	Display *dpy;
	int i;

	if (!real)
		real = (opendisplay_fn)dlsym(RTLD_NEXT, "XOpenDisplay");
	dpy = real(name);
	for (i = 0; dpy && i < MAX_DISPLAYS; i++)
		if (!displays[i]) {
			displays[i] = dpy;
			break;
		}
	return dpy;
}

/*
 * Forget a display when it is closed: dtwm (through DtSvc) opens and
 * closes extra connections, and poll() below reads every display it
 * knows.
 */
typedef int (*closedisplay_fn)(Display *);

int XCloseDisplay(Display *dpy)
{
	static closedisplay_fn real;
	int i;

	if (!real)
		real = (closedisplay_fn)dlsym(RTLD_NEXT, "XCloseDisplay");
	for (i = 0; i < MAX_DISPLAYS; i++)
		if (displays[i] == dpy) {
			for (; i + 1 < MAX_DISPLAYS; i++)
				displays[i] = displays[i + 1];
			displays[MAX_DISPLAYS - 1] = NULL;
			break;
		}
	return real(dpy);
}

typedef Bool (*querypointer_fn)(Display *, Window, Window *, Window *,
				int *, int *, int *, int *, unsigned int *);

Bool XQueryPointer(Display *dpy, Window w, Window *root, Window *child,
		   int *root_x, int *root_y, int *win_x, int *win_y,
		   unsigned int *mask)
{
	static querypointer_fn real;
	Bool ret;

	if (!real)
		real = (querypointer_fn)dlsym(RTLD_NEXT, "XQueryPointer");
	ret = real(dpy, w, root, child, root_x, root_y, win_x, win_y, mask);
	if (ret) {
		__atomic_store_n(&shm->query_x, *root_x, __ATOMIC_RELAXED);
		__atomic_store_n(&shm->query_y, *root_y, __ATOMIC_RELAXED);
	}
	__atomic_add_fetch(&shm->query_seq, 1, __ATOMIC_RELEASE);
	wake(&shm->query_seq);
	return ret;
}

/*
 * The front panel's switch buttons: dtwm creates them as DtControl
 * gadgets named "toggle" and adds SwitchButtonCB as their XmNcallback
 * (UI.c).  Their geometry is read with XtGetValues (no X request) each
 * time dtwm goes idle.
 */
static Widget switch_buttons[DTWMBENCH_MAX_SWITCH];
static int nswitch_buttons;

/*
 * The Xt functions are looked up when first needed, and the resource
 * names are spelled out (XtNx and friends are macros on Xt's data):
 * the library is also preloaded into what dtwm runs (sh, through
 * system()), which has no libXt to bind them to.
 */
typedef void (*addcallback_fn)(Widget, const char *, XtCallbackProc,
			       XtPointer);
typedef char *(*xtname_fn)(Widget);
typedef void (*getvalues_fn)(Widget, ArgList, Cardinal);
typedef Window (*windowof_fn)(Widget);

void XtAddCallback(Widget w, const char *name, XtCallbackProc proc,
		   XtPointer closure)
{
	static addcallback_fn real;
	static xtname_fn xtname;

	if (!real) {
		real = (addcallback_fn)dlsym(RTLD_NEXT, "XtAddCallback");
		xtname = (xtname_fn)dlsym(RTLD_DEFAULT, "XtName");
	}
	if (name && !strcmp(name, "callback") && xtname &&
	    !strcmp(xtname(w), "toggle") &&
	    nswitch_buttons < DTWMBENCH_MAX_SWITCH)
		switch_buttons[nswitch_buttons++] = w;
	real(w, name, proc, closure);
}

static void update_switch_buttons(void)
{
	static getvalues_fn getvalues;
	static windowof_fn windowof;
	int i;

	if (!getvalues) {
		getvalues = (getvalues_fn)dlsym(RTLD_DEFAULT, "XtGetValues");
		windowof = (windowof_fn)dlsym(RTLD_DEFAULT,
					      "XtWindowOfObject");
		if (!getvalues || !windowof)
			return;
	}
	for (i = 0; i < nswitch_buttons; i++) {
		Position x = 0, y = 0;
		Dimension w = 0, h = 0;
		Arg al[4];

		XtSetArg(al[0], "x", &x);
		XtSetArg(al[1], "y", &y);
		XtSetArg(al[2], "width", &w);
		XtSetArg(al[3], "height", &h);
		getvalues(switch_buttons[i], al, 4);
		shm->fp_window[i] = windowof(switch_buttons[i]);
		shm->fp_x[i] = x;
		shm->fp_y[i] = y;
		shm->fp_w[i] = w;
		shm->fp_h[i] = h;
	}
	shm->fp_count = nswitch_buttons;
}

/*
 * dtwmbench sends DTWMBENCH_WAKE_SIGNAL after setting release_req: with
 * freezeOnConfig dtwm waits for the end of the drag in XWindowEvent,
 * that is in a poll() of libxcb without a timeout, which the signal
 * interrupts (no SA_RESTART); libxcb polls again, through the wrapper
 * below.
 */
static void on_wake(int sig)
{
	(void)sig;
}

/*
 * Release button 1 through XTest on the display whose connection dtwm
 * is polling (libXtst is loaded on first use: dtwm does not link it).
 */
typedef int (*fakebutton_fn)(Display *, unsigned int, Bool, unsigned long);
typedef Bool (*querytest_fn)(Display *, int *, int *, int *, int *);

static void *xtst(void)
{
	static void *h;

	if (!h)
		h = dlopen("libXtst.so.6", RTLD_NOW | RTLD_GLOBAL);
	return h;
}

/* Called from the Xt loop: look XTEST up on every display (a round trip
 * each, the first time). */
static void prepare_release(void)
{
	querytest_fn query = xtst() ? (querytest_fn)dlsym(xtst(),
		"XTestQueryExtension") : NULL;
	int i, a, b, c, d;

	__atomic_store_n(&shm->release_prep, 0, __ATOMIC_RELAXED);
	for (i = 0; query && i < MAX_DISPLAYS && displays[i]; i++)
		query(displays[i], &a, &b, &c, &d);
}

static void release_button(struct pollfd *fds, nfds_t nfds)
{
	static fakebutton_fn fake;
	Display *dpy = NULL;
	int i;

	for (i = 0; i < MAX_DISPLAYS && displays[i] && !dpy; i++)
		if (nfds > 0 && ConnectionNumber(displays[i]) == fds[0].fd)
			dpy = displays[i];
	if (!dpy)
		return;
	if (!fake) {
		if (xtst())
			fake = (fakebutton_fn)dlsym(xtst(),
						    "XTestFakeButtonEvent");
		if (!fake)
			return;
	}
	/*
	 * First, since sending polls too, through this wrapper, with the
	 * display locked: a second release from there would deadlock.
	 */
	__atomic_store_n(&shm->release_req, 0, __ATOMIC_RELAXED);
	fake(dpy, 1, False, CurrentTime);
	XFlush(dpy);
	__atomic_add_fetch(&shm->release_seq, 1, __ATOMIC_RELEASE);
	wake(&shm->release_seq);
}

/*
 * Whether a poll() call comes from libXt (its event loop) rather than
 * from xcb waiting for a reply or from dtwm itself.  The answer for each
 * call site is cached.
 */
static int from_xt(void *caller)
{
	static void *xt_base;
	static struct { void *addr; int xt; } cache[8];
	static int ncache;
	Dl_info info;
	int i, xt;

	for (i = 0; i < ncache; i++)
		if (cache[i].addr == caller)
			return cache[i].xt;
	if (!xt_base) {
		void *sym = dlsym(RTLD_DEFAULT, "XtAppNextEvent");

		if (sym && dladdr(sym, &info))
			xt_base = info.dli_fbase;
	}
	xt = xt_base && dladdr(caller, &info) && info.dli_fbase == xt_base;
	if (ncache < 8) {
		cache[ncache].addr = caller;
		cache[ncache].xt = xt;
		ncache++;
	}
	return xt;
}

typedef int (*poll_fn)(struct pollfd *, nfds_t, int);

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	static poll_fn real;
	int idle, ret, i, prepared = 0;

	if (!real)
		real = (poll_fn)dlsym(RTLD_NEXT, "poll");
	idle = timeout != 0 && from_xt(__builtin_return_address(0));
	if (idle) {
		unsigned long requests = 0;

		for (i = 0; i < MAX_DISPLAYS && displays[i]; i++)
			requests += XNextRequest(displays[i]) - 1;
		shm->requests = requests;
		if (nswitch_buttons)
			update_switch_buttons();
		/*
		 * Its round trips may have read events into Xlib's queue
		 * that the poll below would not see: poll without
		 * blocking this once, and libXt looks at the queue again.
		 */
		if (__atomic_load_n(&shm->release_prep, __ATOMIC_ACQUIRE)) {
			prepare_release();
			prepared = 1;
		}
		__atomic_store_n(&shm->idle, 1, __ATOMIC_RELAXED);
		__atomic_add_fetch(&shm->idle_seq, 1, __ATOMIC_RELEASE);
		wake(&shm->idle_seq);
	}
	else if (__atomic_load_n(&shm->release_req, __ATOMIC_ACQUIRE))
		release_button(fds, nfds);
	ret = real(fds, nfds, prepared ? 0 : timeout);
	if (idle)
		__atomic_store_n(&shm->idle, 0, __ATOMIC_RELAXED);
	return ret;
}

typedef int (*grabpointer_fn)(Display *, Window, Bool, unsigned int, int,
			      int, Window, Cursor, Time);

int XGrabPointer(Display *dpy, Window w, Bool owner_events,
		 unsigned int event_mask, int pointer_mode, int keyboard_mode,
		 Window confine_to, Cursor cursor, Time time)
{
	static grabpointer_fn real;
	int ret;

	if (!real)
		real = (grabpointer_fn)dlsym(RTLD_NEXT, "XGrabPointer");
	ret = real(dpy, w, owner_events, event_mask, pointer_mode,
		   keyboard_mode, confine_to, cursor, time);
	if (ret == GrabSuccess) {
		__atomic_add_fetch(&shm->grab_seq, 1, __ATOMIC_RELEASE);
		wake(&shm->grab_seq);
	}
	return ret;
}
