/*
 * CDE benchmarks: the harness shared by dtsvcbench and ttbench.
 *
 * Licensed under the LGPL 2.1 license.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "benchutil.h"

#define MAX_REPEAT 64

double bench_scale = 1.0;

static struct bench_result override;
static int override_set;

void bench_override(const struct bench_result *totals)
{
	override = *totals;
	override_set = 1;
}

void bench_preload(char **argv)
{
	char exe[PATH_MAX], lib[PATH_MAX + 32], *dir;
	const char *old;
	ssize_t len;

	if (bench_counters() || getenv("CDEBENCH_NO_PRELOAD") ||
	    getenv("CDEBENCH_PRELOAD_TRIED"))
		return;
	len = readlink("/proc/self/exe", exe, sizeof exe - 1);
	if (len <= 0)
		return;
	exe[len] = '\0';
	dir = dirname(exe);
	snprintf(lib, sizeof lib, "%s/libcdebench_preload.so", dir);
	if (access(lib, R_OK))
		return;
	old = getenv("LD_PRELOAD");
	if (old && *old) {
		char *both = malloc(strlen(lib) + strlen(old) + 2);

		if (!both)
			return;
		sprintf(both, "%s:%s", lib, old);
		setenv("LD_PRELOAD", both, 1);
		free(both);
	} else {
		setenv("LD_PRELOAD", lib, 1);
	}
	/* Once only, should the library fail to load. */
	setenv("CDEBENCH_PRELOAD_TRIED", "1", 1);
	execv("/proc/self/exe", argv);
}

const struct cdebench_counters *bench_counters(void)
{
	static const struct cdebench_counters *c;
	static int looked;

	if (!looked) {
		c = dlsym(RTLD_DEFAULT, "cdebench_counters");
		looked = 1;
	}
	return c;
}

double bench_now_ns(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

double bench_cpu_ns(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

static int cmp_double(const void *a, const void *b)
{
	double x = *(const double *)a, y = *(const double *)b;

	return (x > y) - (x < y);
}

double bench_median(double *v, int n)
{
	if (n <= 0)
		return 0;
	qsort(v, n, sizeof *v, cmp_double);
	return (n & 1) ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) / 2;
}

void bench_die(const char *fmt, ...)
{
	va_list ap;

	fflush(stdout);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	exit(1);
}

/* Returns 0 when the case ran, BENCH_SKIP when its setup declined. */
static int run_case(const struct bench_case *bc, long n, int repeat,
		    struct bench_result *res)
{
	double ns[MAX_REPEAT], cpu[MAX_REPEAT], ma[MAX_REPEAT];
	double rq[MAX_REPEAT], rt[MAX_REPEAT];
	const struct cdebench_counters *c = bench_counters();
	int r;

	for (r = 0; r < repeat; r++) {
		struct cdebench_counters c0 = { 0, 0, 0, 0 }, c1 = c0;
		double t0, t1, p0, p1;
		long ops;

		if (bc->setup && bc->setup(n))
			return BENCH_SKIP;
		if (c)
			c0 = *c;
		override_set = 0;
		p0 = bench_cpu_ns();
		t0 = bench_now_ns();
		ops = bc->run(n);
		t1 = bench_now_ns();
		p1 = bench_cpu_ns();
		if (c)
			c1 = *c;
		if (bc->teardown)
			bc->teardown();
		if (ops < 1)
			ops = 1;
		if (override_set) {
			ns[r] = override.ns / ops;
			cpu[r] = override.cpu_ns / ops;
			ma[r] = override.mallocs / ops;
			rq[r] = override.requests / ops;
			rt[r] = override.rtrips / ops;
			continue;
		}
		ns[r] = (t1 - t0) / ops;
		cpu[r] = (p1 - p0) / ops;
		ma[r] = (double)(c1.mallocs - c0.mallocs) / ops;
		rq[r] = (double)(c1.requests - c0.requests) / ops;
		rt[r] = (double)(c1.rtrips - c0.rtrips) / ops;
	}
	res->ns = bench_median(ns, repeat);
	res->cpu_ns = bench_median(cpu, repeat);
	res->mallocs = bench_median(ma, repeat);
	res->requests = bench_median(rq, repeat);
	res->rtrips = bench_median(rt, repeat);
	return 0;
}

static int selected(const struct bench_case *bc, int argc, char **argv,
		    int first)
{
	int i;

	if (first >= argc)
		return 1;
	for (i = first; i < argc; i++)
		if (!strcmp(argv[i], "all") || !strcmp(argv[i], bc->name) ||
		    !strcmp(argv[i], bc->group))
			return 1;
	return 0;
}

static void json_string(FILE *f, const char *s)
{
	fputc('"', f);
	for (; *s; s++) {
		if (*s == '"' || *s == '\\')
			fprintf(f, "\\%c", *s);
		else if ((unsigned char)*s < 0x20)
			fprintf(f, "\\u%04x", *s);
		else
			fputc(*s, f);
	}
	fputc('"', f);
}

static void usage(FILE *f, const char *prog)
{
	fprintf(f, "usage: %s [-r REPEAT] [-s SCALE] [-j FILE] [-l] "
		   "[CASE|GROUP|all ...]\n", prog);
}

int bench_main(const char *prog, const struct bench_case *cases, int ncases,
	       int argc, char **argv, void (*extra_json)(FILE *))
{
	int repeat = 5, opt, first, i, ran = 0;
	const char *json = NULL;
	FILE *jf = NULL;

	while ((opt = getopt(argc, argv, "r:s:j:lh")) != -1) {
		switch (opt) {
		case 'r':
			repeat = atoi(optarg);
			if (repeat < 1 || repeat > MAX_REPEAT)
				bench_die("%s: bad repeat count", prog);
			break;
		case 's':
			bench_scale = atof(optarg);
			if (!(bench_scale > 0))
				bench_die("%s: bad scale", prog);
			break;
		case 'j':
			json = optarg;
			break;
		case 'l':
			for (i = 0; i < ncases; i++)
				printf("%-24s %-8s %s\n", cases[i].name,
				       cases[i].group, cases[i].desc);
			return 0;
		case 'h':
			usage(stdout, prog);
			return 0;
		default:
			usage(stderr, prog);
			return 1;
		}
	}
	first = optind;
	for (opt = first; opt < argc; opt++) {
		int known = !strcmp(argv[opt], "all");

		for (i = 0; i < ncases && !known; i++)
			known = !strcmp(argv[opt], cases[i].name) ||
				!strcmp(argv[opt], cases[i].group);
		if (!known)
			bench_die("%s: unknown case %s", prog, argv[opt]);
	}

	if (json) {
		jf = strcmp(json, "-") ? fopen(json, "w") : stdout;
		if (!jf)
			bench_die("%s: %s", json, strerror(errno));
		fprintf(jf, "{\n  \"bench\": \"%s\",\n  \"version\": 1,\n"
			"  \"repeat\": %d,\n  \"scale\": %g,\n"
			"  \"counters\": %s", prog, repeat, bench_scale,
			bench_counters() ? "true" : "false");
		if (extra_json)
			extra_json(jf);
		fprintf(jf, ",\n  \"cases\": [");
	}
	if (!bench_counters())
		fprintf(stderr, "%s: libcdebench_preload.so not loaded; "
			"no malloc and round trip counts\n", prog);
	printf("%-24s %8s %12s %12s %9s %9s %7s\n", "case", "n", "ns/op",
	       "cpu-ns/op", "mallocs", "requests", "rtrips");
	for (i = 0; i < ncases; i++) {
		const struct bench_case *bc = &cases[i];
		struct bench_result res;
		long n;

		if (!selected(bc, argc, argv, first))
			continue;
		n = (long)(bc->n * bench_scale);
		if (n < 1)
			n = 1;
		if (run_case(bc, n, repeat, &res) == BENCH_SKIP) {
			printf("%-24s %8ld  skipped\n", bc->name, n);
			fflush(stdout);
			continue;
		}
		printf("%-24s %8ld %12.1f %12.1f %9.2f %9.3f %7.3f\n",
		       bc->name, n, res.ns, res.cpu_ns, res.mallocs,
		       res.requests, res.rtrips);
		fflush(stdout);
		if (jf) {
			fprintf(jf, "%s\n    {\"name\": ", ran ? "," : "");
			json_string(jf, bc->name);
			fprintf(jf, ", \"group\": ");
			json_string(jf, bc->group);
			fprintf(jf, ", \"n\": %ld, \"ns_per_op\": %.2f, "
				"\"cpu_ns_per_op\": %.2f, "
				"\"mallocs_per_op\": %.4f, "
				"\"requests_per_op\": %.4f, "
				"\"round_trips_per_op\": %.4f}", n, res.ns,
				res.cpu_ns, res.mallocs, res.requests,
				res.rtrips);
		}
		ran++;
	}
	if (jf) {
		fprintf(jf, "\n  ]\n}\n");
		if (jf != stdout)
			fclose(jf);
	}
	return 0;
}
