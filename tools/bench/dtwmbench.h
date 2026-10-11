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

struct dtwmbench_shm {
	unsigned long mallocs;    /* malloc, calloc and moving realloc calls */
	unsigned long replies;    /* _XReply calls: round trips */
	unsigned long requests;   /* X requests issued when dtwm last went idle */
	unsigned int idle_seq;    /* bumped when dtwm blocks in the Xt loop */
	unsigned int idle;        /* 1 while it is blocked there */
	unsigned int query_seq;   /* bumped by every XQueryPointer */
	int query_x, query_y;     /* the root position it returned */
	unsigned int grab_seq;    /* bumped by every successful XGrabPointer */
};

#endif /* DTWMBENCH_H */
