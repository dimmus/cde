/*
 * CDE benchmarks, help: DtSearch queries, collocation ones in particular.
 *
 * Licensed under the LGPL 2.1 license.
 *
 *   srbench OCF [QUERY...]
 *
 * Runs each query (default: a few collocation and boolean queries for
 * the database mksrdb.sh builds) with DtSearchQuery(), and, when
 * libDtSearch exports it, with the asynchronous DtSearchQueryStart() /
 * DtSearchQueryContinue(): time, hits, the number of calls the
 * asynchronous search took and the longest of them (what a user
 * interface waits for at most), and whether the two returned the same
 * status, stems and hits.  Then it stops an asynchronous search after a
 * few calls and checks that a query runs normally after it.  Exits 1 if
 * any result differs.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <Dt/Search.h>

typedef int (*start_fn)(void *, char *, int, char *, char *, DtSrResult **,
			long *, char *, int *, int *);
typedef int (*continue_fn)(int *);
typedef void (*stop_fn)(void);

static double now_ms(void)
{
	struct timespec t;

	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

static char *results(int st, DtSrResult *res, long cnt, char *stems, int nst)
{
	char *buf = NULL;
	size_t len;
	FILE *f = open_memstream(&buf, &len);
	DtSrResult *r;
	int i;

	fprintf(f, "status %d hits %ld stems %d:", st, cnt, nst);
	for (i = 0; i < nst; i++)
		fprintf(f, " %s", stems + i * DtSrMAXWIDTH_HWORD);
	fputc('\n', f);
	for (r = res; r; r = r->link)
		fprintf(f, "%s %d %ld\n", r->reckey, r->proximity, (long)r->dba);
	fclose(f);
	return buf;
}

int main(int argc, char **argv)
{
	static const char *defq[] = {
		"alpha @5 bravo", "window @2 menu", "text @40 editor",
		"delta & (echo @10 golf)", "(alpha @3 bravo) | (charlie @3 delta)",
		"alpha & bravo | charlie",
	};
	static char stems[DtSrMAX_STEMCOUNT * DtSrMAXWIDTH_HWORD];
	static char stems2[DtSrMAX_STEMCOUNT * DtSrMAXWIDTH_HWORD];
	start_fn qstart = (start_fn)dlsym(RTLD_DEFAULT, "DtSearchQueryStart");
	continue_fn qcont = (continue_fn)dlsym(RTLD_DEFAULT,
					       "DtSearchQueryContinue");
	stop_fn qstop = (stop_fn)dlsym(RTLD_DEFAULT, "DtSearchQueryStop");
	const char **qs = defq;
	int nq = sizeof defq / sizeof defq[0], i, st, dbc, bad = 0;
	char **dbn;

	if (argc < 2) {
		fprintf(stderr, "usage: srbench OCF [QUERY...]\n");
		return 2;
	}
	if (argc > 2) {
		qs = (const char **)argv + 2;
		nq = argc - 2;
	}
	st = DtSearchInit("srbench", NULL, 0, argv[1], NULL, &dbn, &dbc);
	if (st != DtSrOK && st != DtSrREINIT) {
		fprintf(stderr, "srbench: DtSearchInit: %d: %s\n", st,
			DtSearchGetMessages());
		return 2;
	}
	DtSearchSetMaxResults(INT_MAX);
	if (!qstart || !qcont || !qstop)
		printf("(libDtSearch does not export DtSearchQueryStart: "
		       "synchronous queries only)\n");

	for (i = 0; i < nq; i++) {
		DtSrResult *res = NULL, *res2 = NULL;
		long cnt = 0, cnt2 = 0;
		int nst = 0, nst2 = 0, done = 0, calls = 0, st2;
		double t0, t1, t2, a, longest = 0;
		char *r1, *r2;

		t0 = now_ms();
		st = DtSearchQuery((void *)qs[i], dbn[0], 'W', NULL, NULL, &res,
				   &cnt, stems, &nst);
		t1 = now_ms();
		printf("'%s': %d, %ld hits, %.0f ms", qs[i], st, cnt, t1 - t0);
		if (qstart) {
			a = now_ms();
			st2 = qstart((void *)qs[i], dbn[0], 'W', NULL, NULL,
				     &res2, &cnt2, stems2, &nst2, &done);
			longest = now_ms() - a;
			calls = 1;
			while (!done) {
				a = now_ms();
				st2 = qcont(&done);
				if (now_ms() - a > longest)
					longest = now_ms() - a;
				calls++;
			}
			t2 = now_ms();
			r1 = results(st, res, cnt, stems, nst);
			r2 = results(st2, res2, cnt2, stems2, nst2);
			if (strcmp(r1, r2))
				bad++;
			printf("; asynchronous %.0f ms in %d calls, longest "
			       "%.1f ms, %s", t2 - t1, calls, longest,
			       strcmp(r1, r2) ? "DIFFERENT RESULTS"
					      : "same results");
			free(r1);
			free(r2);
			DtSearchFreeResults(&res2);
		}
		putchar('\n');
		DtSearchFreeResults(&res);
		DtSearchFreeMessages();
	}

	if (qstart) {
		DtSrResult *res = NULL;
		long cnt = 0;
		int nst = 0, done = 0, n = 0, st2;

		st2 = qstart((void *)qs[0], dbn[0], 'W', NULL, NULL, &res, &cnt,
			     stems, &nst, &done);
		while (!done) {
			if (++n == 3)
				qstop();
			st2 = qcont(&done);
		}
		printf("stopped after %d calls: %d (%s)\n", n, st2,
		       DtSearchHasMessages() ? DtSearchGetMessages() : "-");
		DtSearchFreeMessages();
		st = DtSearchQuery((void *)qs[0], dbn[0], 'W', NULL, NULL, &res,
				   &cnt, stems, &nst);
		printf("then DtSearchQuery: %d, %ld hits\n", st, cnt);
		DtSearchFreeResults(&res);
	}
	return bad ? 1 : 0;
}
