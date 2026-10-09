/*
 * CDE benchmarks: ttbench, micro-benchmarks of ToolTalk (libtt and
 * ttsession).
 *
 * Licensed under the LGPL 2.1 license.
 *
 *   ttsession -s -c ttbench [-r REPEAT] [-s SCALE] [-j FILE] [CASE ...]
 *
 * open-close, ttdt-open-join and the patterns cases run in a fresh
 * process each (see start_child), which reports its own times and counts.
 *
 * ttbench needs a session: run it under a private "ttsession -c" (as
 * above, and as bench.py does) so that TT_SESSION names it.  Without
 * TT_SESSION it skips every case rather than join the user's desktop
 * session.
 *
 * Cases (see -l):
 *   open-close          tt_open + tt_close
 *   ttdt-open-join      ttdt_open + ttdt_session_join + quit + close
 *   ping                request/reply to a responder process, round trip
 *   burst               notices to the responder, until it has them all
 *   patterns-1k/-10k    tt_pattern_register of 1k/10k patterns spread
 *                       over 50 procids, per pattern
 *   ping-10k-patterns   ping while those 10k patterns are registered
 *   netfile             tt_file_netfile of a local path
 *   message-file-set    tt_message_file_set on a new message
 *   file-notice         a TT_FILE_IN_SESSION notice on a local file,
 *                       sent and fenced with a ping
 *   file-notice-db      a TT_FILE notice (needs rpc.ttdbserver; skipped
 *                       unless CDEBENCH_TTDBSERVER is set)
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <X11/Intrinsic.h>
#include <Tt/tt_c.h>
#include <Tt/tttk.h>

#include "benchutil.h"

#define OP_PING   "CdeBenchPing"
#define OP_NOTICE "CdeBenchNotice"
#define OP_FILE   "CdeBenchFile"
#define OP_QUIT   "CdeBenchQuit"
#define TIMEOUT_MS 30000
#define NPROCIDS 50

static char *main_procid;
static pid_t responder;
static char testfile[] = "/tmp/ttbench.XXXXXX";
static int have_testfile;

static int ok(Tt_status st)
{
	return tt_is_err(st) == 0;
}

static void check(Tt_status st, const char *what)
{
	if (!ok(st))
		bench_die("ttbench: %s: %s", what, tt_status_message(st));
}

/* ------------------------------------------------------------------ */
/* The responder: handles pings, observes notices                      */
/* ------------------------------------------------------------------ */

static void register_pattern(Tt_category cat, Tt_scope scope, const char *op)
{
	Tt_pattern p = tt_pattern_create();

	check(tt_pattern_category_set(p, cat), "tt_pattern_category_set");
	check(tt_pattern_scope_add(p, scope), "tt_pattern_scope_add");
	check(tt_pattern_op_add(p, op), "tt_pattern_op_add");
	if (scope == TT_SESSION || scope == TT_FILE_IN_SESSION)
		check(tt_pattern_session_add(p, tt_default_session()),
		      "tt_pattern_session_add");
	check(tt_pattern_register(p), "tt_pattern_register");
}

static void run_responder(int ready_fd)
{
	char *procid = tt_open();
	struct pollfd pfd;

	if (tt_ptr_error(procid) != TT_OK)
		_exit(2);
	register_pattern(TT_HANDLE, TT_SESSION, OP_PING);
	register_pattern(TT_HANDLE, TT_SESSION, OP_QUIT);
	register_pattern(TT_OBSERVE, TT_SESSION, OP_NOTICE);
	register_pattern(TT_OBSERVE, TT_FILE_IN_SESSION, OP_FILE);
	register_pattern(TT_OBSERVE, TT_FILE, OP_FILE);
	check(tt_session_join(tt_default_session()), "tt_session_join");
	{
		struct bench_result ready = { 0, 0, 0, 0, 0 };

		if (write(ready_fd, &ready, sizeof ready) != sizeof ready)
			_exit(3);
	}
	close(ready_fd);

	pfd.fd = tt_fd();
	pfd.events = POLLIN;
	for (;;) {
		Tt_message m;
		char *op;

		if (poll(&pfd, 1, -1) < 0 && errno != EINTR)
			_exit(4);
		while ((m = tt_message_receive()) != NULL &&
		       tt_ptr_error(m) == TT_OK) {
			op = tt_message_op(m);
			if (tt_message_state(m) == TT_SENT && op &&
			    !strcmp(op, OP_PING)) {
				tt_message_reply(m);
			} else if (op && !strcmp(op, OP_QUIT)) {
				tt_message_reply(m);
				tt_message_destroy(m);
				tt_free(op);
				tt_close();
				_exit(0);
			}
			tt_free(op);
			tt_message_destroy(m);
		}
	}
}

/* ------------------------------------------------------------------ */
/* Sending                                                             */
/* ------------------------------------------------------------------ */

/* Wait until the reply to m (or its failure) comes back. */
static void wait_reply(Tt_message m)
{
	struct pollfd pfd = { tt_fd(), POLLIN, 0 };
	double deadline = bench_now_ns() + TIMEOUT_MS * 1e6;

	for (;;) {
		Tt_message r;

		while ((r = tt_message_receive()) != NULL &&
		       tt_ptr_error(r) == TT_OK) {
			Tt_state st = tt_message_state(r);

			if (r == m) {
				if (st == TT_HANDLED)
					return;
				if (st == TT_FAILED || st == TT_REJECTED)
					bench_die("ttbench: request failed "
						  "(state %d)", (int)st);
				continue;
			}
			tt_message_destroy(r);
		}
		if (bench_now_ns() > deadline)
			bench_die("ttbench: timed out waiting for a reply");
		poll(&pfd, 1, 100);
	}
}

static void ping(void)
{
	Tt_message m = tt_prequest_create(TT_SESSION, OP_PING);

	check(tt_ptr_error(m), "tt_prequest_create");
	check(tt_message_send(m), "tt_message_send");
	wait_reply(m);
	tt_message_destroy(m);
}

static int need_session(void)
{
	return main_procid == NULL;
}

static int session_setup(long n)
{
	(void)n;
	return need_session();
}

/* ------------------------------------------------------------------ */
/* Cases                                                               */
/* ------------------------------------------------------------------ */

/*
 * Cases run in a fresh process: libtt keeps one set of procids per
 * process, and tt_close of one procid of several invalidates the others
 * (after tt_open twice and tt_close of the second, tt_default_procid_set
 * of the first fails with TT_ERR_PROCID), so the main procid cannot
 * share a process with procids that come and go.  The child times its
 * own work and reports it through a pipe.
 */
static char **self_argv;
static pid_t holder;          /* a child holding its patterns */

static void child_report(int fd, double t0, double p0,
			 struct cdebench_counters *c0)
{
	const struct cdebench_counters *c = bench_counters();
	struct bench_result r;

	r.ns = bench_now_ns() - t0;
	r.cpu_ns = bench_cpu_ns() - p0;
	r.mallocs = c ? (double)(c->mallocs - c0->mallocs) : 0;
	r.requests = c ? (double)(c->requests - c0->requests) : 0;
	r.rtrips = c ? (double)(c->rtrips - c0->rtrips) : 0;
	if (write(fd, &r, sizeof r) != sizeof r)
		_exit(9);
}

static void child_open_close(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		char *p = tt_open();

		check(tt_ptr_error(p), "tt_open");
		check(tt_close(), "tt_close");
		tt_free(p);
	}
}

static void child_ttdt(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		int fd;
		char *p = ttdt_open(&fd, "ttbench", "CDE", "1.0", 1);
		Tt_pattern *pats;

		check(tt_ptr_error(p), "ttdt_open");
		pats = ttdt_session_join(NULL, NULL, NULL, NULL, 1);
		check(tt_ptr_error(pats), "ttdt_session_join");
		check(ttdt_session_quit(NULL, pats, 1), "ttdt_session_quit");
		check(ttdt_close(p, NULL, 1), "ttdt_close");
		tt_free(p);
	}
}

static char *procids[NPROCIDS];

static void child_open_procids(void)
{
	int i;

	for (i = 0; i < NPROCIDS; i++) {
		procids[i] = tt_open();
		check(tt_ptr_error(procids[i]), "tt_open");
	}
}

static void child_patterns(long n)
{
	char op[64];
	long i;

	for (i = 0; i < n; i++) {
		Tt_pattern p;

		check(tt_default_procid_set(procids[i % NPROCIDS]),
		      "tt_default_procid_set");
		p = tt_pattern_create();
		snprintf(op, sizeof op, "CdeBenchPattern%ld", i);
		tt_pattern_category_set(p, TT_OBSERVE);
		tt_pattern_scope_add(p, TT_SESSION);
		tt_pattern_op_add(p, op);
		tt_pattern_session_add(p, tt_default_session());
		check(tt_pattern_register(p), "tt_pattern_register");
	}
}

/* ttbench --child MODE N FD */
static int child_main(char **argv)
{
	const char *mode = argv[2];
	long n = atol(argv[3]);
	int fd = atoi(argv[4]);
	struct cdebench_counters c0 = { 0, 0, 0, 0 };
	double t0, p0;

	if (!strcmp(mode, "responder"))
		run_responder(fd);
	if (!strcmp(mode, "patterns") || !strcmp(mode, "hold"))
		child_open_procids();
	if (bench_counters())
		c0 = *bench_counters();
	p0 = bench_cpu_ns();
	t0 = bench_now_ns();
	if (!strcmp(mode, "open-close"))
		child_open_close(n);
	else if (!strcmp(mode, "ttdt"))
		child_ttdt(n);
	else
		child_patterns(n);
	child_report(fd, t0, p0, &c0);
	close(fd);
	if (!strcmp(mode, "hold")) {
		/* Keep the patterns until the parent is done (or gone). */
		prctl(PR_SET_PDEATHSIG, SIGTERM);
		for (;;)
			pause();
	}
	_exit(0);
}

/* Start "ttbench --child MODE N FD"; returns its pid, fills *res. */
static pid_t start_child(const char *mode, long n, struct bench_result *res)
{
	char nbuf[32], fdbuf[16];
	int fds[2];
	pid_t pid;

	if (pipe(fds))
		bench_die("ttbench: pipe: %s", strerror(errno));
	snprintf(nbuf, sizeof nbuf, "%ld", n);
	snprintf(fdbuf, sizeof fdbuf, "%d", fds[1]);
	fflush(NULL);
	pid = fork();
	if (pid < 0)
		bench_die("ttbench: fork: %s", strerror(errno));
	if (pid == 0) {
		char *args[] = { self_argv[0], (char *)"--child", (char *)mode,
				 nbuf, fdbuf, NULL };

		close(fds[0]);
		execv("/proc/self/exe", args);
		_exit(127);
	}
	close(fds[1]);
	if (read(fds[0], res, sizeof *res) != sizeof *res)
		bench_die("ttbench: the %s child failed", mode);
	close(fds[0]);
	return pid;
}

static void run_child(const char *mode, long n)
{
	struct bench_result res;
	int status;
	pid_t pid = start_child(mode, n, &res);

	if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status) ||
	    WEXITSTATUS(status))
		bench_die("ttbench: the %s child failed (status 0x%x)", mode,
			  status);
	bench_override(&res);
}

/* The responder runs in its own process, started before our tt_open. */
static void start_responder(void)
{
	struct bench_result res;

	responder = start_child("responder", 0, &res);
}

static long open_close_run(long n)
{
	run_child("open-close", n);
	return n;
}

static long ttdt_run(long n)
{
	run_child("ttdt", n);
	return n;
}

static long ping_run(long n)
{
	long i;

	for (i = 0; i < n; i++)
		ping();
	return n;
}

static long burst_run(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		Tt_message m = tt_pnotice_create(TT_SESSION, OP_NOTICE);

		check(tt_ptr_error(m), "tt_pnotice_create");
		check(tt_message_send(m), "tt_message_send");
		tt_message_destroy(m);
	}
	/* Delivery to one procid is in order: the ping is the fence. */
	ping();
	return n;
}

static long patterns_run(long n)
{
	run_child("patterns", n);
	return n;
}

static void stop_holder(void)
{
	int status;

	if (holder > 0) {
		kill(holder, SIGTERM);
		waitpid(holder, &status, 0);
		holder = 0;
	}
}

/* Another process registers 10k patterns over 50 procids, and keeps them. */
static int ping_patterns_setup(long n)
{
	struct bench_result res;

	(void)n;
	if (need_session())
		return 1;
	stop_holder();
	holder = start_child("hold", (long)(10000 * bench_scale), &res);
	return 0;
}

static int netfile_setup(long n)
{
	int fd;

	(void)n;
	if (need_session())
		return 1;
	if (!have_testfile) {
		if ((fd = mkstemp(testfile)) < 0)
			bench_die("ttbench: mkstemp: %s", strerror(errno));
		close(fd);
		have_testfile = 1;
	}
	return 0;
}

static long netfile_run(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		char *nf = tt_file_netfile(testfile);

		check(tt_ptr_error(nf), "tt_file_netfile");
		tt_free(nf);
	}
	return n;
}

static long file_set_run(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		Tt_message m = tt_pnotice_create(TT_FILE_IN_SESSION, OP_FILE);

		check(tt_ptr_error(m), "tt_pnotice_create");
		check(tt_message_file_set(m, testfile),
		      "tt_message_file_set");
		tt_message_destroy(m);
	}
	return n;
}

static long file_notice(long n, Tt_scope scope)
{
	long i;

	for (i = 0; i < n; i++) {
		Tt_message m = tt_pnotice_create(scope, OP_FILE);

		check(tt_ptr_error(m), "tt_pnotice_create");
		check(tt_message_file_set(m, testfile),
		      "tt_message_file_set");
		check(tt_message_send(m), "tt_message_send (file)");
		tt_message_destroy(m);
	}
	ping();
	return n;
}

static long file_notice_run(long n)
{
	return file_notice(n, TT_FILE_IN_SESSION);
}

static int file_db_setup(long n)
{
	if (!getenv("CDEBENCH_TTDBSERVER"))
		return 1;
	return netfile_setup(n);
}

static long file_notice_db_run(long n)
{
	return file_notice(n, TT_FILE);
}

static const struct bench_case cases[] = {
	{ "open-close", "tt", "tt_open + tt_close",
	  200, session_setup, open_close_run, NULL },
	{ "ttdt-open-join", "tt",
	  "ttdt_open + ttdt_session_join + quit + close",
	  50, session_setup, ttdt_run, NULL },
	{ "ping", "tt", "request/reply round trip to another process",
	  10000, session_setup, ping_run, NULL },
	{ "burst", "tt", "notices to an observer, per notice",
	  1000, session_setup, burst_run, NULL },
	{ "patterns-1k", "tt",
	  "register 1k patterns over 50 procids, per pattern",
	  1000, session_setup, patterns_run, NULL },
	{ "patterns-10k", "tt",
	  "register 10k patterns over 50 procids, per pattern",
	  10000, session_setup, patterns_run, NULL },
	{ "ping-10k-patterns", "tt",
	  "request/reply round trip with 10k patterns registered",
	  1000, ping_patterns_setup, ping_run, stop_holder },
	{ "netfile", "file", "tt_file_netfile of a local file",
	  100, netfile_setup, netfile_run, NULL },
	{ "message-file-set", "file", "tt_message_file_set",
	  100, netfile_setup, file_set_run, NULL },
	{ "file-notice", "file",
	  "TT_FILE_IN_SESSION notice on a local file, per notice",
	  100, netfile_setup, file_notice_run, NULL },
	{ "file-notice-db", "file",
	  "TT_FILE notice (rpc.ttdbserver), per notice",
	  100, file_db_setup, file_notice_db_run, NULL },
};

static void stop_responder(void)
{
	if (responder > 0) {
		int status;

		if (main_procid) {
			Tt_message m = tt_prequest_create(TT_SESSION, OP_QUIT);

			if (tt_ptr_error(m) == TT_OK && ok(tt_message_send(m)))
				wait_reply(m);
		}
		kill(responder, SIGTERM);
		waitpid(responder, &status, 0);
		responder = 0;
	}
}

int main(int argc, char **argv)
{
	int status;
	const char *sess = getenv("TT_SESSION");

	bench_preload(argv);
	if (argc == 5 && !strcmp(argv[1], "--child"))
		return child_main(argv);
	self_argv = argv;
	signal(SIGPIPE, SIG_IGN);
	if (sess && *sess) {
		start_responder();
		main_procid = tt_open();
		if (tt_ptr_error(main_procid) != TT_OK) {
			fprintf(stderr, "ttbench: tt_open: %s\n",
				tt_status_message(tt_ptr_error(main_procid)));
			main_procid = NULL;
		} else {
			check(tt_session_join(tt_default_session()),
			      "tt_session_join");
		}
	} else {
		fprintf(stderr, "ttbench: TT_SESSION is not set; run me "
			"under \"ttsession -s -c\"\n");
	}
	status = bench_main("ttbench", cases,
			    (int)(sizeof cases / sizeof cases[0]), argc, argv,
			    NULL);
	stop_holder();
	stop_responder();
	if (main_procid)
		tt_close();
	if (have_testfile)
		unlink(testfile);
	return status;
}
