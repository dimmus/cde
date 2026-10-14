/*
 * CDE - Common Desktop Environment
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/*
 * CDE_STARTUP_TRACE stamps of libtt (see programs/dtlogin/startup-trace.h):
 * how a client finds and reaches its ttsession, and the host name
 * lookups that take long, so that a slow or failed tt_open() in a login
 * shows where its time went.  The component is "libtt(<program>)".
 */
#ifndef _TT_STARTUP_TRACE_H
#define _TT_STARTUP_TRACE_H

#include <errno.h>
#include <string.h>
#include "../../../../programs/dtlogin/startup-trace.h"

static inline const char *
_tt_trace_component(void)
{
	static char	comp[64];

	if (comp[0] == '\0') {
#if defined(__GLIBC__)
		const char *prog = program_invocation_short_name;
#else
		const char *prog = getprogname();
#endif
		snprintf(comp, sizeof comp, "libtt(%s)",
			 (prog && *prog) ? prog : "?");
	}
	return comp;
}

// Milliseconds of CLOCK_MONOTONIC, for the "ms=" of the stamps.
static inline long long
_tt_trace_ms(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#define _tt_trace(event, ...) \
	do { \
		if (cde_startup_tracing()) { \
			cde_startup_tracef(_tt_trace_component(), event, \
					   __VA_ARGS__); \
		} \
	} while (0)

#endif /* _TT_STARTUP_TRACE_H */
