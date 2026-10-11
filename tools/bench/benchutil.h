/*
 * CDE benchmarks: the harness shared by dtsvcbench and ttbench.
 *
 * Licensed under the LGPL 2.1 license.
 *
 * A benchmark is a table of cases.  Each case runs REPEAT times (default
 * 5); every run times N operations and reports, as the median over the
 * runs and per operation:
 *   ns        wall time (CLOCK_MONOTONIC)
 *   cpu_ns    CPU time of this process (CLOCK_PROCESS_CPUTIME_ID)
 *   mallocs   malloc/calloc/moving realloc calls      } from
 *   requests  X requests (XNextRequest delta, all     } libcdebench_
 *             displays of the process)                } preload.so,
 *   rtrips    X round trips (waits for a reply)       } when loaded
 * The results print as a table, and as JSON with -j FILE.
 */
#ifndef CDEBENCH_UTIL_H
#define CDEBENCH_UTIL_H

#include <stdio.h>

/* The counters kept by libcdebench_preload.so. */
struct cdebench_counters {
	unsigned long mallocs;
	unsigned long frees;
	unsigned long requests;
	unsigned long rtrips;
};

struct bench_case {
	const char *name;
	const char *group;
	const char *desc;
	long n;                         /* operations at scale 1 */
	int (*setup)(long n);           /* untimed; nonzero: skip the case */
	long (*run)(long n);            /* timed; returns the operations */
	void (*teardown)(void);         /* untimed */
};

struct bench_result {
	double ns, cpu_ns, mallocs, requests, rtrips;
};

#define BENCH_SKIP 77

/*
 * Re-exec the program with libcdebench_preload.so (found next to it)
 * preloaded, unless it already is or CDEBENCH_NO_PRELOAD is set.  Call
 * first thing in main.
 */
void bench_preload(char **argv);

/* The counters, or NULL when the preload library is not loaded. */
const struct cdebench_counters *bench_counters(void);

double bench_now_ns(void);
double bench_cpu_ns(void);
double bench_median(double *v, int n);

/*
 * For a case whose work runs in another process: called from run(), it
 * replaces the measured wall time, CPU time and counters of that run by
 * these totals (not per operation).
 */
void bench_override(const struct bench_result *totals);

/* Print a message and exit with status 1. */
void bench_die(const char *fmt, ...)
	__attribute__((format(printf, 1, 2), noreturn));

/*
 * Parse the common options (-r REPEAT -s SCALE -j FILE -l -h), run the
 * selected cases (the remaining arguments: case names, group names or
 * "all"), print the table and the JSON.  Returns the exit status.
 * extra_json, if not NULL, is called to add top-level JSON members
 * (each written as ",\n  \"key\": value").
 */
int bench_main(const char *prog, const struct bench_case *cases, int ncases,
	       int argc, char **argv, void (*extra_json)(FILE *));

/* The scale factor of the current run (-s), for setups that size data. */
extern double bench_scale;

#endif /* CDEBENCH_UTIL_H */
