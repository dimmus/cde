/**
 * CDE benchmarks (from Motif src/tests/bench/mwmbench.h)
 *
 * Licensed under the LGPL 2.1 license.
 *
 * The memory shared between dtwmbench and its probe in dtwm
 * (dtwmpreload.c).  dtwm writes, dtwmbench reads; the sequence numbers are
 * futex words that dtwm wakes.
 */
#ifndef DTWMBENCH_H
#define DTWMBENCH_H

#define DTWMBENCH_MAX_SWITCH 16

/* SIGRTMIN + 6 on Linux: interrupts dtwm's poll() (release_req). */
#define DTWMBENCH_WAKE_SIGNAL 40

struct dtwmbench_shm {
	unsigned long mallocs;    /* malloc, calloc and moving realloc calls */
	unsigned long replies;    /* _XReply calls: round trips */
	unsigned long requests;   /* X requests issued when dtwm last went idle */
	unsigned int idle_seq;    /* bumped when dtwm blocks in the Xt loop */
	unsigned int idle;        /* 1 while it is blocked there */
	unsigned int query_seq;   /* bumped by every XQueryPointer */
	int query_x, query_y;     /* the root position it returned */
	unsigned int grab_seq;    /* bumped by every successful XGrabPointer */
	/*
	 * The front panel's workspace switch buttons (DtControl gadgets
	 * named "toggle" that get an XmNcallback), refreshed whenever dtwm
	 * goes idle: the window they draw in and their geometry in it.
	 */
	unsigned int fp_count;
	unsigned long fp_window[DTWMBENCH_MAX_SWITCH];
	int fp_x[DTWMBENCH_MAX_SWITCH], fp_y[DTWMBENCH_MAX_SWITCH];
	int fp_w[DTWMBENCH_MAX_SWITCH], fp_h[DTWMBENCH_MAX_SWITCH];
	/*
	 * Set by dtwmbench to have dtwm release button 1 through XTest on
	 * its own connection, the next time it polls outside the Xt loop:
	 * while dtwm holds the server grabbed (freezeOnConfig) no other
	 * client's request, XTest included, is processed.  release_seq is
	 * bumped (and woken) once it is done.
	 */
	unsigned int release_req;
	unsigned int release_seq;
	/*
	 * Set by dtwmbench before: dtwm then looks up the XTEST extension
	 * on its displays when it next goes idle, so that the release
	 * needs no round trip (one from inside libxcb's wait for an event
	 * would wait for itself).
	 */
	unsigned int release_prep;
};

#endif /* DTWMBENCH_H */
