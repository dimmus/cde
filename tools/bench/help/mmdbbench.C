/*
 * CDE benchmarks, help: reading a dtinfo information library (libDtMmdb).
 *
 * Licensed under the LGPL 2.1 license.
 *
 *   mmdbbench [-r REPEAT] [-l INFOLIB] [-b BASE] [-o FILE] [CASE...]
 *
 * Opens an infolib as dtinfo does (OLIAS_DB::openInfoLib) and reads,
 * through the same smart pointers dtinfo's node views use, every
 * section's data (case "nodes") or every graphic's data ("graphics") of
 * the base (default: the first of the infolib).  Each case runs REPEAT
 * times in the same process (default 3): run 1 is cold for libDtMmdb's
 * page cache, later runs show what the cache keeps.  Reported per
 * object: wall and CPU time; the total of bytes read is printed so that
 * two builds can be checked to read the same.  With -o, the data read
 * in the first run is written to FILE (cmp(1) it between builds).
 *
 * Syscalls are counted from outside: run.py runs it under strace -c.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "oliasdb/mmdb.h"
#include "oliasdb/node_hd.h"
#include "oliasdb/graphic_hd.h"
#include "oliasdb/olias_consts.h"

static double now_ns(clockid_t c)
{
	struct timespec ts;

	clock_gettime(c, &ts);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

static FILE *out;

static long read_all(info_base *base, int graphics, unsigned long *bytes)
{
	Iterator *it = graphics ? base->first(GRAPHIC_SET_NAME, GRAPHIC_CODE)
				: base->first(NODE_SET_NAME, OLIAS_NODE_CODE);
	long n = 0;

	if (!it)
		return 0;
	while (*it) {
		oid_t id = base->get_oid(*it);
		const char *data;
		int size;

		if (graphics) {
			graphic_smart_ptr g(base, id);

			data = g.data();
			size = g.data_size();
		} else {
			node_smart_ptr x(base, id);

			data = x.data();
			size = data ? (int)strlen(data) : 0;
		}
		if (data && size > 0) {
			*bytes += size;
			if (out)
				fwrite(data, 1, size, out);
		}
		n++;
		base->next(*it);
	}
	delete it;
	return n;
}

int main(int argc, char **argv)
{
	const char *lib = getenv("MMDBBENCH_INFOLIB");
	const char *bname = NULL, *ofile = NULL;
	int repeat = 3, opt, i, c;
	static const char *names[] = { "nodes", "graphics" };

	if (!lib)
		lib = "/usr/dt/infolib/C/cde.dti";
	while ((opt = getopt(argc, argv, "r:l:b:o:h")) != -1) {
		switch (opt) {
		case 'r': repeat = atoi(optarg); break;
		case 'l': lib = optarg; break;
		case 'b': bname = optarg; break;
		case 'o': ofile = optarg; break;
		default:
			fprintf(stderr, "usage: mmdbbench [-r REPEAT] "
				"[-l INFOLIB] [-b BASE] [-o FILE] "
				"[nodes|graphics ...]\n");
			return opt == 'h' ? 0 : 1;
		}
	}
	if (repeat < 1)
		repeat = 1;

	OLIAS_DB db;
	double t0 = now_ns(CLOCK_MONOTONIC);
	info_lib *infolib = db.openInfoLib(lib, 0, "mmdbbench");

	if (!infolib) {
		fprintf(stderr, "mmdbbench: cannot open %s\n", lib);
		return 1;
	}
	info_base *base = bname ? infolib->get_info_base(bname) : 0;
	if (!base) {
		long ind = infolib->first();

		if (ind)
			base = (*infolib)(ind);
	}
	if (!base) {
		fprintf(stderr, "mmdbbench: no base in %s\n", lib);
		return 1;
	}
	printf("open %.1f ms\n", (now_ns(CLOCK_MONOTONIC) - t0) / 1e6);
	if (ofile && !(out = fopen(ofile, "wb"))) {
		perror(ofile);
		return 1;
	}

	for (c = 0; c < 2; c++) {
		int sel = optind >= argc;

		for (i = optind; i < argc && !sel; i++)
			sel = !strcmp(argv[i], names[c]);
		if (!sel)
			continue;
		for (i = 0; i < repeat; i++) {
			unsigned long bytes = 0;
			double w0 = now_ns(CLOCK_MONOTONIC);
			double p0 = now_ns(CLOCK_PROCESS_CPUTIME_ID);
			long n = read_all(base, c, &bytes);
			double w1 = now_ns(CLOCK_MONOTONIC);
			double p1 = now_ns(CLOCK_PROCESS_CPUTIME_ID);

			if (out && i == 0) {
				fclose(out);
				out = NULL;
			}
			printf("%-9s run %d: %ld objects, %lu bytes, "
			       "%.0f ns/object, cpu %.0f ns/object\n",
			       names[c], i + 1, n, bytes,
			       n ? (w1 - w0) / n : 0.0,
			       n ? (p1 - p0) / n : 0.0);
		}
	}
	fflush(stdout);
	_exit(0);	/* (the infolib's destructors are not what is timed) */
}
