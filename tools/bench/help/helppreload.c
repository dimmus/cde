/*
 * CDE benchmarks, help: LD_PRELOAD counters of the Xlib calls that the
 * help system makes per topic view (colours, fonts, images).
 *
 * Licensed under the LGPL 2.1 license.
 *
 * libhelpbench_preload.so counts the calls a process makes, through the
 * dynamic linker, to
 *
 *   XAllocColor XAllocNamedColor XLookupColor XParseColor     colours
 *   XLoadQueryFont XLoadFont XQueryFont XCreateFontSet        fonts
 *   XListFonts
 *   XPutImage XCreatePixmap XCreateGC                         graphics
 *
 * It complements ../libcdebench_preload.so (mallocs, X requests, round
 * trips); preload both.  helpbench finds the counters with
 * dlsym(RTLD_DEFAULT, "helpbench_xcounts"); any other program (dthelpview,
 * dthelpprint) reports its totals at exit, one line per process,
 *
 *   helpbench pid=1234 comm=dthelpview XAllocColor=... ... ppid=...
 *
 * to the file named by CDEBENCH_REPORT (stderr for "1" or "-"), the same
 * file ../libcdebench_preload.so appends its "cdebench" line to.
 *
 * Calls that libX11 makes to itself (XLoadQueryFont from inside
 * XCreateFontSet, say) do not go through the dynamic linker when libX11
 * is linked -Bsymbolic-functions, and are then not counted: the counts
 * are of the calls made by the CDE libraries, Motif and the program.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "helpbench.h"

struct helpbench_xcounts helpbench_xcounts;

#define REAL(name, type) \
	static type real; \
	if (!real) \
		real = (type)dlsym(RTLD_NEXT, name)

static void arm_sigterm(void);
static int armed;

#define COUNT(field) do { \
	__atomic_fetch_add(&helpbench_xcounts.field, 1, __ATOMIC_RELAXED); \
	if (!armed) \
		arm_sigterm(); \
} while (0)

Status XAllocColor(Display *d, Colormap c, XColor *x)
{
	typedef Status (*fn)(Display *, Colormap, XColor *);
	REAL("XAllocColor", fn);
	COUNT(alloc_color);
	return real(d, c, x);
}

Status XAllocNamedColor(Display *d, Colormap c, const char *n, XColor *s,
			XColor *e)
{
	typedef Status (*fn)(Display *, Colormap, const char *, XColor *,
			     XColor *);
	REAL("XAllocNamedColor", fn);
	COUNT(alloc_named_color);
	return real(d, c, n, s, e);
}

Status XLookupColor(Display *d, Colormap c, const char *n, XColor *s,
		    XColor *e)
{
	typedef Status (*fn)(Display *, Colormap, const char *, XColor *,
			     XColor *);
	REAL("XLookupColor", fn);
	COUNT(lookup_color);
	return real(d, c, n, s, e);
}

Status XParseColor(Display *d, Colormap c, const char *n, XColor *x)
{
	typedef Status (*fn)(Display *, Colormap, const char *, XColor *);
	REAL("XParseColor", fn);
	COUNT(parse_color);
	return real(d, c, n, x);
}

XFontStruct *XLoadQueryFont(Display *d, const char *n)
{
	typedef XFontStruct *(*fn)(Display *, const char *);
	REAL("XLoadQueryFont", fn);
	COUNT(load_query_font);
	return real(d, n);
}

Font XLoadFont(Display *d, const char *n)
{
	typedef Font (*fn)(Display *, const char *);
	REAL("XLoadFont", fn);
	COUNT(load_font);
	return real(d, n);
}

XFontStruct *XQueryFont(Display *d, XID f)
{
	typedef XFontStruct *(*fn)(Display *, XID);
	REAL("XQueryFont", fn);
	COUNT(query_font);
	return real(d, f);
}

XFontSet XCreateFontSet(Display *d, const char *b, char ***m, int *c,
			char **s)
{
	typedef XFontSet (*fn)(Display *, const char *, char ***, int *,
			       char **);
	REAL("XCreateFontSet", fn);
	COUNT(create_font_set);
	return real(d, b, m, c, s);
}

char **XListFonts(Display *d, const char *p, int max, int *count)
{
	typedef char **(*fn)(Display *, const char *, int, int *);
	REAL("XListFonts", fn);
	COUNT(list_fonts);
	return real(d, p, max, count);
}

int XPutImage(Display *d, Drawable w, GC gc, XImage *i, int sx, int sy,
	      int dx, int dy, unsigned int wd, unsigned int ht)
{
	typedef int (*fn)(Display *, Drawable, GC, XImage *, int, int, int,
			  int, unsigned int, unsigned int);
	REAL("XPutImage", fn);
	COUNT(put_image);
	return real(d, w, gc, i, sx, sy, dx, dy, wd, ht);
}

Pixmap XCreatePixmap(Display *d, Drawable w, unsigned int wd,
		     unsigned int ht, unsigned int depth)
{
	typedef Pixmap (*fn)(Display *, Drawable, unsigned int, unsigned int,
			     unsigned int);
	REAL("XCreatePixmap", fn);
	COUNT(create_pixmap);
	return real(d, w, wd, ht, depth);
}

GC XCreateGC(Display *d, Drawable w, unsigned long mask, XGCValues *v)
{
	typedef GC (*fn)(Display *, Drawable, unsigned long, XGCValues *);
	REAL("XCreateGC", fn);
	COUNT(create_gc);
	return real(d, w, mask, v);
}

static void report(void) __attribute__((destructor));

/* Is fn in ../libcdebench_preload.so (whose handler we chain to)? */
static int dladdr_is_preload(void *fn)
{
	Dl_info info;

	return dladdr(fn, &info) && info.dli_fname &&
	       strstr(info.dli_fname, "libcdebench_preload.so") != NULL;
}

static void report(void)
{
	const char *dest = getenv("CDEBENCH_REPORT");
	const struct helpbench_xcounts *c = &helpbench_xcounts;
	char comm[64] = "?", line[512];
	int fd, len;
	ssize_t n;

	if (!dest || !*dest)
		return;
	if ((fd = open("/proc/self/comm", O_RDONLY | O_CLOEXEC)) >= 0) {
		n = read(fd, comm, sizeof comm - 1);
		comm[n > 0 ? n : 0] = '\0';
		comm[strcspn(comm, "\n")] = '\0';
		close(fd);
	}
	len = snprintf(line, sizeof line,
		       "helpbench pid=%d comm=%s XAllocColor=%lu "
		       "XAllocNamedColor=%lu XLookupColor=%lu XParseColor=%lu "
		       "XLoadQueryFont=%lu XLoadFont=%lu XQueryFont=%lu "
		       "XCreateFontSet=%lu XListFonts=%lu XPutImage=%lu "
		       "XCreatePixmap=%lu XCreateGC=%lu ppid=%d\n",
		       (int)getpid(), comm, c->alloc_color,
		       c->alloc_named_color, c->lookup_color, c->parse_color,
		       c->load_query_font, c->load_font, c->query_font,
		       c->create_font_set, c->list_fonts, c->put_image,
		       c->create_pixmap, c->create_gc, (int)getppid());
	if (len <= 0)
		return;
	if (len >= (int)sizeof line)
		len = sizeof line - 1;
	if (!strcmp(dest, "1") || !strcmp(dest, "-"))
		fd = 2;
	else if ((fd = open(dest, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC,
			    0644)) < 0)
		return;
	n = write(fd, line, (size_t)len);
	(void)n;
	if (fd != 2)
		close(fd);
}

/*
 * CDEBENCH_REPORT_SIGTERM: report on a default-action SIGTERM too, then
 * pass the signal on to the handler that was there (the one of
 * ../libcdebench_preload.so, which reports and dies).  Installed at the
 * first counted call, when the constructors of both libraries have run,
 * whatever their order.
 */
static struct sigaction prev_term;

static void on_sigterm(int sig)
{
	report();
	sigaction(sig, &prev_term, NULL);
	raise(sig);
}

static void arm_sigterm(void)
{
	struct sigaction cur, sa;

	armed = 1;
	if (!getenv("CDEBENCH_REPORT_SIGTERM") ||
	    sigaction(SIGTERM, NULL, &cur) != 0 ||
	    (cur.sa_flags & SA_SIGINFO) || cur.sa_handler == SIG_IGN)
		return;
	/* the program's own handler, if any, stays */
	if (cur.sa_handler != SIG_DFL &&
	    !dladdr_is_preload((void *)cur.sa_handler))
		return;
	prev_term = cur;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = on_sigterm;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
}

static void forked_child(void)
{
	memset(&helpbench_xcounts, 0, sizeof helpbench_xcounts);
}

static void preload_init(void) __attribute__((constructor));

static void preload_init(void)
{
	pthread_atfork(NULL, NULL, forked_child);
}
