/*
 * CDE benchmarks: LD_PRELOAD counters.
 *
 * Licensed under the LGPL 2.1 license (derived from Motif's
 * src/tests/bench/preload.c).
 *
 * libcdebench_preload.so counts, for the process it is preloaded into:
 *   - mallocs  calls to malloc, calloc and moving realloc;
 *   - frees    calls to free with a non-NULL pointer;
 *   - requests X requests written to the server (the "requests"
 *              argument of xcb_writev, which every Xlib flush goes
 *              through), i.e. the sum of the XNextRequest deltas of all
 *              the process's displays;
 *   - rtrips   round trips: waits for a reply (xcb_wait_for_reply*, which
 *              _XReply and XSync use) after requests were sent since the
 *              previous wait.  Waits for replies to requests already
 *              answered in the same round trip (XInternAtoms, async
 *              handlers) do not count.  This is what counting _XReply
 *              calls gives, without interposing a libX11-internal symbol
 *              (which fails where libX11 is linked -Bsymbolic-functions).
 *
 * The benchmark programs of this directory (dtsvcbench, ttbench) find
 * the counters with dlsym(RTLD_DEFAULT, "cdebench_counters") and re-exec
 * themselves with the library preloaded when it is not.
 *
 * Any other program (dtterm, dtfile, dtwm, dtmail...) can be measured as
 * a whole:
 *
 *   CDEBENCH_REPORT=/tmp/counts LD_PRELOAD=.../libcdebench_preload.so dtterm
 *
 * appends one line per process at exit:
 *
 *   cdebench pid=1234 comm=dtterm mallocs=... frees=... requests=...
 *       rtrips=...
 *
 * to the file (or to stderr with CDEBENCH_REPORT=1 or "-").  No signal
 * handler is installed: the program's own handlers are left alone.
 * Processes that leave with _exit() or a signal do not report.
 *
 * With CDEBENCH_REPLY_BACKTRACE set, every round trip prints a backtrace
 * to stderr, to find where they come from.
 *
 * The allocator wrappers forward to the glibc __libc_* entry points rather
 * than to dlsym(RTLD_NEXT, ...), since dlsym itself allocates.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include "benchutil.h"

extern void *__libc_malloc(size_t);
extern void *__libc_calloc(size_t, size_t);
extern void *__libc_realloc(void *, size_t);
extern void __libc_free(void *);

/* The counters, found by the benchmarks with dlsym. */
struct cdebench_counters cdebench_counters;

void *malloc(size_t size)
{
	__atomic_fetch_add(&cdebench_counters.mallocs, 1, __ATOMIC_RELAXED);
	return __libc_malloc(size);
}

void *calloc(size_t nmemb, size_t size)
{
	__atomic_fetch_add(&cdebench_counters.mallocs, 1, __ATOMIC_RELAXED);
	return __libc_calloc(nmemb, size);
}

void *realloc(void *ptr, size_t size)
{
	/* Count only the reallocs that may move the block. */
	if (size)
		__atomic_fetch_add(&cdebench_counters.mallocs, 1,
				   __ATOMIC_RELAXED);
	return __libc_realloc(ptr, size);
}

void free(void *ptr)
{
	if (ptr)
		__atomic_fetch_add(&cdebench_counters.frees, 1,
				   __ATOMIC_RELAXED);
	__libc_free(ptr);
}

static void report(void) __attribute__((destructor));

static void report(void)
{
	const char *dest = getenv("CDEBENCH_REPORT");
	char comm[64] = "?", line[320];
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
		       "cdebench pid=%d comm=%s mallocs=%lu frees=%lu "
		       "requests=%lu rtrips=%lu\n", (int)getpid(), comm,
		       cdebench_counters.mallocs, cdebench_counters.frees,
		       cdebench_counters.requests, cdebench_counters.rtrips);
	if (len <= 0)
		return;
	if (len >= (int)sizeof line)
		len = sizeof line - 1;
	if (!strcmp(dest, "1") || !strcmp(dest, "-")) {
		fd = 2;
	} else if ((fd = open(dest, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC,
			      0644)) < 0) {
		return;
	}
	/* One write, so that the lines of concurrent processes stay whole. */
	n = write(fd, line, (size_t)len);
	(void)n;
	if (fd != 2)
		close(fd);
}

/* Requests were sent since the last round trip. */
static int sent;

static void count_reply(void)
{
	static int trace = -1;

	if (!sent)
		return;
	sent = 0;

	if (trace < 0)
		trace = getenv("CDEBENCH_REPLY_BACKTRACE") != NULL;
	if (trace) {
		void *bt[32];

		backtrace_symbols_fd(bt, backtrace(bt, 32), 2);
		if (write(2, "--\n", 3) < 0)
			trace = 0;
	}
	cdebench_counters.rtrips++;
}

/*
 * int xcb_writev(xcb_connection_t *, struct iovec *, int, uint64_t),
 * void *xcb_wait_for_reply(xcb_connection_t *, unsigned int,
 *                          xcb_generic_error_t **)
 * and its version with a 64-bit sequence number, from <xcb/xcbext.h>
 * and <xcb/xcb.h>.  Declared here so that the library builds without
 * the xcb headers.
 */
typedef int (*writev_fn)(void *, struct iovec *, int, unsigned long long);
typedef void *(*wait_fn)(void *, unsigned int, void **);
typedef void *(*wait64_fn)(void *, unsigned long long, void **);

int xcb_writev(void *c, struct iovec *vector, int count,
	       unsigned long long requests);
void *xcb_wait_for_reply(void *c, unsigned int request, void **e);
void *xcb_wait_for_reply64(void *c, unsigned long long request, void **e);

int xcb_writev(void *c, struct iovec *vector, int count,
	       unsigned long long requests)
{
	static writev_fn real;

	if (!real)
		real = (writev_fn)dlsym(RTLD_NEXT, "xcb_writev");
	if (requests) {
		sent = 1;
		cdebench_counters.requests += requests;
	}
	return real(c, vector, count, requests);
}

void *xcb_wait_for_reply(void *c, unsigned int request, void **e)
{
	static wait_fn real;

	if (!real)
		real = (wait_fn)dlsym(RTLD_NEXT, "xcb_wait_for_reply");
	count_reply();
	return real(c, request, e);
}

void *xcb_wait_for_reply64(void *c, unsigned long long request, void **e)
{
	static wait64_fn real;

	if (!real)
		real = (wait64_fn)dlsym(RTLD_NEXT, "xcb_wait_for_reply64");
	count_reply();
	return real(c, request, e);
}
