/*
 * CDE benchmarks: what the bit-rotted -DDT_PERFORMANCE code of dtfile
 * needs to compile again, force-included by dtfile-perf.sh through
 * perfbuild.py (the tree itself is not changed):
 *   - the declarations of the checkpoint functions it calls;
 *   - update_time_s and update_time_f, which ToolTalk.c uses as if they
 *     were globals while Main.c and Directory.c declare locals of those
 *     names (which hide these).
 *
 * Licensed under the LGPL 2.1 license.
 */
#ifndef CDEBENCH_DTPERF_SHIM_H
#define CDEBENCH_DTPERF_SHIM_H

#include <sys/time.h>
#include <Dt/ChkpntP.h>

static struct timeval update_time_s __attribute__((unused));
static struct timeval update_time_f __attribute__((unused));

#endif
