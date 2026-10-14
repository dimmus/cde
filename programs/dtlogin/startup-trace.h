/*
 * CDE_STARTUP_TRACE stamps (see tools/bench/startup-trace.h, of which
 * this is the copy the programs build with, and startup-trace.py, which
 * turns a trace into the T_greeter, T_desktop and T_restore numbers).
 *
 * Licensed under the LGPL 2.1 license.
 *
 *     cde_startup_trace("dtsession", "colors");
 *
 * When CDE_STARTUP_TRACE names a file, appends one line to it:
 *
 *     <CLOCK_MONOTONIC seconds.nanoseconds> <pid> <component> <event>
 *
 * in a single write(2) on an O_APPEND descriptor, so that the lines of
 * concurrent processes stay whole.  Without the variable it costs one
 * getenv().  dtlogin passes the variable on to the greeter, the Xsetup,
 * Xstartup and Xreset scripts and the session when it has it itself.
 *
 * The file is not followed through a symbolic link (dtlogin and the
 * scripts it runs as root write to it too), and it is created with mode
 * 0600 if it does not exist: for the stamps of the session, which runs
 * as the user, make it writable by the user beforehand.
 */
#ifndef CDE_STARTUP_TRACE_H
#define CDE_STARTUP_TRACE_H

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static inline void
cde_startup_trace_line(const char *line, int len)
{
    const char *path = getenv("CDE_STARTUP_TRACE");
    int fd;

    if (!path || !*path || len <= 0)
	return;
    fd = open(path, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC | O_NOFOLLOW,
	      0600);
    if (fd < 0)
	return;
    if (write(fd, line, (size_t) len) < 0) {
	/* best effort */
    }
    close(fd);
}

static inline int
cde_startup_tracing(void)
{
    const char *path = getenv("CDE_STARTUP_TRACE");

    return path && *path;
}

static inline void
cde_startup_trace(const char *component, const char *event)
{
    struct timespec ts;
    char line[256];
    int len;

    if (!cde_startup_tracing())
	return;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    len = snprintf(line, sizeof line, "%lld.%09ld %d %s %s\n",
		   (long long) ts.tv_sec, ts.tv_nsec, (int) getpid(),
		   component, event);
    if (len >= (int) sizeof line)
	len = sizeof line - 1;
    cde_startup_trace_line(line, len);
}

/*
 * The ksh stamps of the scripts are CLOCK_REALTIME (ksh93 has no other
 * clock); a "clock realtime=<s> monotonic=<s>" line lets
 * startup-trace.py put them on the monotonic clock.
 */
static inline void
cde_startup_trace_clock(void)
{
    struct timespec rt, mt;
    char line[128];
    int len;

    if (!cde_startup_tracing())
	return;
    clock_gettime(CLOCK_REALTIME, &rt);
    clock_gettime(CLOCK_MONOTONIC, &mt);
    len = snprintf(line, sizeof line,
		   "clock realtime=%lld.%09ld monotonic=%lld.%09ld\n",
		   (long long) rt.tv_sec, rt.tv_nsec,
		   (long long) mt.tv_sec, mt.tv_nsec);
    if (len >= (int) sizeof line)
	len = sizeof line - 1;
    cde_startup_trace_line(line, len);
}

/*
 * A stamp with details after the event, "key=value" words, for example
 *     cde_startup_tracef("libtt", "tt-tcp", "host=%s ms=%lld", ip, ms);
 * (startup-trace.py reads the first word after the component as the
 * event and ignores the rest).
 */
static inline void
cde_startup_tracef(const char *component, const char *event,
           const char *fmt, ...)
#if defined(__GNUC__)
    __attribute__((format(printf, 3, 4)))
#endif
;
static inline void
cde_startup_tracef(const char *component, const char *event,
           const char *fmt, ...)
{
    struct timespec ts;
    char line[512];
    va_list ap;
    int len, more;

    if (!getenv("CDE_STARTUP_TRACE") || !*getenv("CDE_STARTUP_TRACE"))
        return;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    len = snprintf(line, sizeof line, "%lld.%09ld %d %s %s ",
               (long long) ts.tv_sec, ts.tv_nsec, (int) getpid(),
               component, event);
    if (len <= 0 || len >= (int) sizeof line - 2)
        return;
    va_start(ap, fmt);
    more = vsnprintf(line + len, sizeof line - (size_t) len - 1, fmt, ap);
    va_end(ap);
    if (more < 0)
        more = 0;
    len += more;
    if (len > (int) sizeof line - 2)
        len = sizeof line - 2;
    line[len++] = '\n';
    {
        const char *path = getenv("CDE_STARTUP_TRACE");
        int fd = open(path, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC
#ifdef O_NOFOLLOW
                   | O_NOFOLLOW
#endif
                   , 0600);

        if (fd < 0)
            return;
        if (write(fd, line, (size_t) len) < 0) {
            /* best effort */
        }
        close(fd);
    }
}

#endif /* CDE_STARTUP_TRACE_H */
