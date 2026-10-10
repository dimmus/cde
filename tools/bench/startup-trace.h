/*
 * CDE benchmarks: CDE_STARTUP_TRACE stamps, for C programs.
 *
 * Licensed under the LGPL 2.1 license.
 *
 * Header only: copy or include it, then call
 *
 *     cde_startup_trace("dtsession", "colors-ready");
 *
 * at each point of interest.  When CDE_STARTUP_TRACE names a file, one
 * line is appended to it:
 *
 *     <CLOCK_MONOTONIC seconds.nanoseconds> <pid> <component> <event>
 *
 * in a single write(2) on an O_APPEND descriptor, so that the lines of
 * concurrent processes (dtlogin, dtgreet, Xsession, dtsession, dtwm,
 * clients) stay whole and in order.  Without the variable it costs one
 * getenv.  CLOCK_MONOTONIC is shared by all processes of the machine, so
 * stamps of different processes compare directly.
 *
 * startup-trace.py turns such a file into the T_greeter, T_desktop and
 * T_restore numbers; README.md lists the event names it expects.
 */
#ifndef CDE_STARTUP_TRACE_H
#define CDE_STARTUP_TRACE_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* Older feature-test settings (_XOPEN_SOURCE 600) leave it out. */
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

static inline void cde_startup_trace(const char *component, const char *event)
{
	const char *path = getenv("CDE_STARTUP_TRACE");
	struct timespec ts;
	char line[256];
	int fd, len;

	if (!path || !*path)
		return;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	len = snprintf(line, sizeof line, "%lld.%09ld %d %s %s\n",
		       (long long)ts.tv_sec, ts.tv_nsec, (int)getpid(),
		       component, event);
	if (len <= 0)
		return;
	if (len >= (int)sizeof line)
		len = sizeof line - 1;
	fd = open(path, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0600);
	if (fd < 0)
		return;
	if (write(fd, line, (size_t)len) < 0) {
		/* nothing to do: tracing is best effort */
	}
	close(fd);
}

#endif /* CDE_STARTUP_TRACE_H */
