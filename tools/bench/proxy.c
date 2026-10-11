/**
 * CDE benchmarks (copied from Motif src/tests/bench/proxy.c)
 *
 * Licensed under the LGPL 2.1 license.
 *
 * cdebench-proxy: an X11 proxy that adds latency and counts round trips.
 *
 *   cdebench-proxy [-d MS] [-s STATS] [-q] LISTEN UPSTREAM
 *
 * It listens on TCP port 6000 + LISTEN on the loopback interface, so
 * that clients reach it with DISPLAY=127.0.0.1:LISTEN, and connects each
 * client to UPSTREAM, the path of an X server's Unix socket (for example
 * /tmp/.X11-unix/X5).  Everything is forwarded unchanged, each direction
 * delayed by MS milliseconds (default 0, fractions allowed): a round
 * trip costs 2 * MS more, like "tc qdisc add dev lo root netem delay MS"
 * but without root.
 *
 * The proxy decodes the framing of the protocol (not its contents) to
 * count, per connection, the requests, replies, errors and events, and
 * the round trips: the replies or errors (and the connection setup
 * reply) the client receives after it has sent something since the
 * previous one, i.e. the times a client that waits for a reply pays the
 * latency.  When a connection closes, a JSON object with its counts and
 * the client's TCP port is appended as one line to STATS (default
 * stderr).  Once it listens, the proxy prints "ready" on stdout (unless
 * -q) and flushes it; it runs until SIGTERM or SIGINT.
 *
 * It also counts the requests per request type ("ops": core requests by
 * name, extension requests by extension name, learnt from the replies
 * to QueryExtension) and the round trips per type of the request that
 * was answered ("op_round_trips"), which is what "xtrace -c" or xscope
 * would give: say, how many GetGeometry, GetInputFocus and ClearArea a
 * client sends, and which of its requests make it wait.  SIGUSR1 makes
 * it append the counts so far of every open connection ("live": true),
 * so that a script can take the counts of one action as the difference
 * of two snapshots.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define CHUNK 65536
#define MAX_QUEUED (8 << 20)   /* stop reading a side beyond this backlog */
#define MAX_CONNS 64

/* Pending data of one direction, oldest first. */
struct chunk {
	struct chunk *next;
	int64_t due;          /* CLOCK_MONOTONIC ns when it may be sent */
	size_t len, off;
	unsigned char data[];
};

/* Incremental decoder of the message framing of one direction. */
#define CAPTURE_MAX 512       /* QueryExtension requests read whole */

struct framer {
	int phase;            /* 0: connection setup, 1: requests/messages */
	int capture;          /* reading a whole QueryExtension request */
	unsigned char hdr[CAPTURE_MAX];
	size_t have, need;
	uint64_t skip;        /* bytes of the current message still to come */
};

struct dir {
	int from, to;
	struct chunk *head, *tail;
	size_t queued;
	int eof;              /* the source has closed */
	int shut;             /* the destination's write side is shut down */
	struct framer fr;
};

struct conn {
	int id;
	int port;             /* the client's TCP port */
	struct dir c2s, s2c;
	int big_endian;
	int sent;             /* client data since the last reply or error */
	unsigned long requests, replies, errors, events, round_trips;
	unsigned long ops[256];          /* requests per major opcode */
	unsigned long op_rtrips[256];    /* round trips per answered major */
	unsigned char seqop[65536];      /* major opcode per sequence number */
	struct {
		unsigned seq;
		char name[64];
	} queries[8];                    /* QueryExtension awaiting replies */
	unsigned long long c2s_bytes, s2c_bytes;
	int64_t start;
};

static struct conn *conns[MAX_CONNS];
static int64_t delay_ns;
static FILE *stats;
static volatile sig_atomic_t quit, snapshot;

/* The extensions' names by major opcode, as the server told a client. */
static char *ext_names[256];

static const char *const core_names[128] = {
	NULL, "CreateWindow", "ChangeWindowAttributes", "GetWindowAttributes",
	"DestroyWindow", "DestroySubwindows", "ChangeSaveSet",
	"ReparentWindow", "MapWindow", "MapSubwindows", "UnmapWindow",
	"UnmapSubwindows", "ConfigureWindow", "CirculateWindow",
	"GetGeometry", "QueryTree", "InternAtom", "GetAtomName",
	"ChangeProperty", "DeleteProperty", "GetProperty", "ListProperties",
	"SetSelectionOwner", "GetSelectionOwner", "ConvertSelection",
	"SendEvent", "GrabPointer", "UngrabPointer", "GrabButton",
	"UngrabButton", "ChangeActivePointerGrab", "GrabKeyboard",
	"UngrabKeyboard", "GrabKey", "UngrabKey", "AllowEvents",
	"GrabServer", "UngrabServer", "QueryPointer", "GetMotionEvents",
	"TranslateCoordinates", "WarpPointer", "SetInputFocus",
	"GetInputFocus", "QueryKeymap", "OpenFont", "CloseFont", "QueryFont",
	"QueryTextExtents", "ListFonts", "ListFontsWithInfo", "SetFontPath",
	"GetFontPath", "CreatePixmap", "FreePixmap", "CreateGC", "ChangeGC",
	"CopyGC", "SetDashes", "SetClipRectangles", "FreeGC", "ClearArea",
	"CopyArea", "CopyPlane", "PolyPoint", "PolyLine", "PolySegment",
	"PolyRectangle", "PolyArc", "FillPoly", "PolyFillRectangle",
	"PolyFillArc", "PutImage", "GetImage", "PolyText8", "PolyText16",
	"ImageText8", "ImageText16", "CreateColormap", "FreeColormap",
	"CopyColormapAndFree", "InstallColormap", "UninstallColormap",
	"ListInstalledColormaps", "AllocColor", "AllocNamedColor",
	"AllocColorCells", "AllocColorPlanes", "FreeColors", "StoreColors",
	"StoreNamedColor", "QueryColors", "LookupColor", "CreateCursor",
	"CreateGlyphCursor", "FreeCursor", "RecolorCursor", "QueryBestSize",
	"QueryExtension", "ListExtensions", "ChangeKeyboardMapping",
	"GetKeyboardMapping", "ChangeKeyboardControl", "GetKeyboardControl",
	"Bell", "ChangePointerControl", "GetPointerControl", "SetScreenSaver",
	"GetScreenSaver", "ChangeHosts", "ListHosts", "SetAccessControl",
	"SetCloseDownMode", "KillClient", "RotateProperties",
	"ForceScreenSaver", "SetPointerMapping", "GetPointerMapping",
	"SetModifierMapping", "GetModifierMapping", NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, "NoOperation"
};

#define X_QueryExtension 98

static int64_t now_ns(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static void on_signal(int sig)
{
	if (sig == SIGUSR1)
		snapshot = 1;
	else
		quit = 1;
}

static unsigned card16(const struct conn *c, const unsigned char *p)
{
	return c->big_endian ? (p[0] << 8) | p[1] : p[0] | (p[1] << 8);
}

static uint32_t card32(const struct conn *c, const unsigned char *p)
{
	return c->big_endian ?
		((uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3] :
		p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}

static size_t pad4(size_t n)
{
	return (n + 3) & ~(size_t)3;
}

/*
 * A complete header of fr->need bytes is in fr->hdr.  Count it, and set
 * the body to skip; returns 1 instead when the header goes on (the
 * 32-bit length of a BIG-REQUEST).
 */
static int decode_c2s(struct conn *c, struct framer *fr)
{
	uint64_t total;
	unsigned len;

	if (fr->phase == 0) {
		/* xConnClientPrefix: byte order, name and data lengths. */
		c->big_endian = fr->hdr[0] == 'B';
		fr->skip = pad4(card16(c, fr->hdr + 6)) +
			   pad4(card16(c, fr->hdr + 8));
		fr->phase = 1;
		fr->need = 4;
		c->sent = 1;
		return 0;
	}
	len = card16(c, fr->hdr + 2);
	if (len == 0 && fr->need == 4) {
		fr->need = 8;
		return 1;
	}
	/* QueryExtension: read it whole, for the name. */
	if (fr->hdr[0] == X_QueryExtension && !fr->capture && len > 2 &&
	    (size_t)len * 4 <= CAPTURE_MAX) {
		fr->capture = 1;
		fr->need = (size_t)len * 4;
		return 1;
	}
	total = len ? (uint64_t)len * 4 : (uint64_t)card32(c, fr->hdr + 4) * 4;
	fr->skip = total > fr->need ? total - fr->need : 0;
	c->requests++;
	c->sent = 1;
	c->ops[fr->hdr[0]]++;
	c->seqop[c->requests & 0xffff] = fr->hdr[0];
	if (fr->capture) {
		unsigned n = card16(c, fr->hdr + 4);
		int i;

		if (n > 63)
			n = 63;
		if (8 + n > fr->need)
			n = fr->need > 8 ? (unsigned)fr->need - 8 : 0;
		for (i = 0; i < 8 && c->queries[i].name[0]; i++)
			;
		if (i == 8)
			i = 0;
		c->queries[i].seq = c->requests & 0xffff;
		memcpy(c->queries[i].name, fr->hdr + 8, n);
		c->queries[i].name[n] = '\0';
		fr->capture = 0;
	}
	fr->need = 4;
	return 0;
}

/* A reply or an error (to request seq) that the client had to wait for. */
static void answered(struct conn *c, int seq)
{
	if (c->sent) {
		c->round_trips++;
		if (seq >= 0)
			c->op_rtrips[c->seqop[seq]]++;
		c->sent = 0;
	}
}

/* The reply to a QueryExtension: remember the extension's opcode. */
static void query_reply(struct conn *c, const unsigned char *rep)
{
	unsigned seq = card16(c, rep + 2);
	int i;

	for (i = 0; i < 8; i++) {
		if (!c->queries[i].name[0] || c->queries[i].seq != seq)
			continue;
		if (rep[8] && rep[9] >= 128 && !ext_names[rep[9]])
			ext_names[rep[9]] = strdup(c->queries[i].name);
		c->queries[i].name[0] = '\0';
	}
}

static int decode_s2c(struct conn *c, struct framer *fr)
{
	int type;

	if (fr->phase == 0) {
		/* xConnSetupPrefix: the length of the rest in 4-byte units. */
		fr->skip = (uint64_t)card16(c, fr->hdr + 6) * 4;
		fr->phase = 1;
		fr->need = 32;
		answered(c, -1);
		return 0;
	}
	type = fr->hdr[0] & 0x7f;
	if (type == 1 || type == 35) /* X_Reply, GenericEvent */
		fr->skip = (uint64_t)card32(c, fr->hdr + 4) * 4;
	if (type == 0) {
		c->errors++;
		answered(c, (int)card16(c, fr->hdr + 2));
	} else if (type == 1) {
		c->replies++;
		answered(c, (int)card16(c, fr->hdr + 2));
		if (c->seqop[card16(c, fr->hdr + 2)] == X_QueryExtension)
			query_reply(c, fr->hdr);
	} else {
		c->events++;
	}
	return 0;
}

static void frame(struct conn *c, struct framer *fr, int c2s,
		  const unsigned char *p, size_t len)
{
	while (len) {
		size_t n;

		if (fr->skip) {
			n = fr->skip < len ? fr->skip : len;
			fr->skip -= n;
		} else {
			n = fr->need - fr->have;
			if (n > len)
				n = len;
			memcpy(fr->hdr + fr->have, p, n);
			fr->have += n;
			if (fr->have == fr->need &&
			    !(c2s ? decode_c2s(c, fr) : decode_s2c(c, fr)))
				fr->have = 0;
		}
		p += n;
		len -= n;
	}
}

static void free_queue(struct dir *d)
{
	while (d->head) {
		struct chunk *k = d->head;

		d->head = k->next;
		free(k);
	}
	d->tail = NULL;
	d->queued = 0;
}

static void op_name(int major, char *buf, size_t len)
{
	if (major < 128 && core_names[major])
		snprintf(buf, len, "%s", core_names[major]);
	else if (major >= 128 && ext_names[major])
		snprintf(buf, len, "%s", ext_names[major]);
	else
		snprintf(buf, len, "op%d", major);
}

static void print_ops(const unsigned long *count)
{
	char name[80];
	int i, first = 1;

	fputc('{', stats);
	for (i = 0; i < 256; i++) {
		if (!count[i])
			continue;
		op_name(i, name, sizeof name);
		fprintf(stats, "%s\"%s\": %lu", first ? "" : ", ", name,
			count[i]);
		first = 0;
	}
	fputc('}', stats);
}

static void print_conn(const struct conn *c, int live)
{
	fprintf(stats, "{\"conn\": %d, \"client_port\": %d, "
		"\"requests\": %lu, \"replies\": %lu, "
		"\"errors\": %lu, \"events\": %lu, \"round_trips\": %lu, "
		"\"c2s_bytes\": %llu, \"s2c_bytes\": %llu, "
		"\"seconds\": %.6f, \"delay_ms\": %.3f, \"live\": %s, "
		"\"ops\": ",
		c->id, c->port, c->requests, c->replies, c->errors, c->events,
		c->round_trips, c->c2s_bytes, c->s2c_bytes,
		(now_ns() - c->start) / 1e9, delay_ns / 1e6,
		live ? "true" : "false");
	print_ops(c->ops);
	fprintf(stats, ", \"op_round_trips\": ");
	print_ops(c->op_rtrips);
	fprintf(stats, "}\n");
	fflush(stats);
}

static void close_conn(int i)
{
	struct conn *c = conns[i];

	print_conn(c, 0);
	close(c->c2s.from);
	close(c->s2c.from);
	free_queue(&c->c2s);
	free_queue(&c->s2c);
	free(c);
	conns[i] = NULL;
}

/* Queue what is available from d->from, to be sent after the delay. */
static void pump_in(struct conn *c, struct dir *d, int c2s)
{
	static unsigned char buf[CHUNK];
	struct chunk *k;
	ssize_t n;

	n = read(d->from, buf, sizeof buf);
	if (n <= 0) {
		if (n == 0 || (errno != EAGAIN && errno != EINTR))
			d->eof = 1;
		return;
	}
	k = malloc(sizeof *k + n);
	if (!k) {
		perror("cdebench-proxy");
		exit(1);
	}
	memcpy(k->data, buf, n);
	frame(c, &d->fr, c2s, buf, n);
	if (c2s)
		c->c2s_bytes += n;
	else
		c->s2c_bytes += n;
	k->next = NULL;
	k->len = n;
	k->off = 0;
	k->due = now_ns() + delay_ns;
	if (d->tail)
		d->tail->next = k;
	else
		d->head = k;
	d->tail = k;
	d->queued += n;
}

/* Send the chunks that are due; returns -1 when the peer is gone. */
static int pump_out(struct dir *d, int64_t now)
{
	while (d->head && d->head->due <= now) {
		struct chunk *k = d->head;
		ssize_t n = send(d->to, k->data + k->off, k->len - k->off,
				 MSG_NOSIGNAL);

		if (n < 0)
			return errno == EAGAIN || errno == EINTR ? 0 : -1;
		k->off += n;
		d->queued -= n;
		if (k->off < k->len)
			return 0;
		d->head = k->next;
		if (!d->head)
			d->tail = NULL;
		free(k);
	}
	if (d->eof && !d->head && !d->shut) {
		shutdown(d->to, SHUT_WR);
		d->shut = 1;
	}
	return 0;
}

static int connect_upstream(const char *path)
{
	struct sockaddr_un sa;
	int fd;

	if (strlen(path) >= sizeof sa.sun_path) {
		errno = ENAMETOOLONG;
		return -1;
	}
	memset(&sa, 0, sizeof sa);
	sa.sun_family = AF_UNIX;
	strcpy(sa.sun_path, path);
	fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd < 0)
		return -1;
	if (connect(fd, (struct sockaddr *)&sa, sizeof sa) < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

static void accept_conn(int lfd, const char *upstream)
{
	static int next_id = 1;
	struct sockaddr_in peer;
	socklen_t len = sizeof peer;
	struct conn *c;
	int cfd, sfd, one = 1, i;

	cfd = accept4(lfd, (struct sockaddr *)&peer, &len,
		      SOCK_CLOEXEC | SOCK_NONBLOCK);
	if (cfd < 0)
		return;
	for (i = 0; i < MAX_CONNS && conns[i]; i++)
		;
	sfd = i < MAX_CONNS ? connect_upstream(upstream) : -1;
	if (sfd < 0) {
		fprintf(stderr, "cdebench-proxy: %s: %s\n", upstream,
			i < MAX_CONNS ? strerror(errno) : "too many clients");
		close(cfd);
		return;
	}
	fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK);
	setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
	c = calloc(1, sizeof *c);
	if (!c) {
		close(cfd);
		close(sfd);
		return;
	}
	c->id = next_id++;
	c->port = ntohs(peer.sin_port);
	c->c2s.from = c->s2c.to = cfd;
	c->c2s.to = c->s2c.from = sfd;
	c->c2s.fr.need = 12;   /* sizeof(xConnClientPrefix) */
	c->s2c.fr.need = 8;    /* sizeof(xConnSetupPrefix) */
	c->start = now_ns();
	conns[i] = c;
}

static int listen_tcp(int display)
{
	struct sockaddr_in sa;
	int fd, one = 1;

	fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd < 0)
		return -1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(6000 + display);
	sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(fd, (struct sockaddr *)&sa, sizeof sa) < 0 ||
	    listen(fd, 16) < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

static void usage(FILE *f)
{
	fprintf(f, "usage: cdebench-proxy [-d MS] [-s STATS] [-q] "
		   "LISTEN UPSTREAM\n");
}

int main(int argc, char **argv)
{
	struct pollfd pfd[1 + 2 * MAX_CONNS];
	struct conn *pc[1 + 2 * MAX_CONNS];
	struct sigaction sa;
	sigset_t block, unblocked;
	int opt, lfd, display, quiet = 0, i;
	char *end;

	stats = stderr;
	while ((opt = getopt(argc, argv, "d:s:qh")) != -1) {
		switch (opt) {
		case 'd':
			delay_ns = (int64_t)(strtod(optarg, &end) * 1e6);
			if (*end || delay_ns < 0) {
				fprintf(stderr, "cdebench-proxy: bad delay\n");
				return 2;
			}
			break;
		case 's':
			stats = fopen(optarg, "a");
			if (!stats) {
				fprintf(stderr, "cdebench-proxy: %s: %s\n",
					optarg, strerror(errno));
				return 1;
			}
			break;
		case 'q':
			quiet = 1;
			break;
		case 'h':
			usage(stdout);
			return 0;
		default:
			usage(stderr);
			return 2;
		}
	}
	if (argc - optind != 2) {
		usage(stderr);
		return 2;
	}
	display = (int)strtol(argv[optind], &end, 10);
	if (*end || display < 0 || display > 59535) {
		fprintf(stderr, "cdebench-proxy: bad display number %s\n",
			argv[optind]);
		return 2;
	}
	lfd = listen_tcp(display);
	if (lfd < 0) {
		fprintf(stderr, "cdebench-proxy: port %d: %s\n", 6000 + display,
			strerror(errno));
		return 1;
	}

	memset(&sa, 0, sizeof sa);
	sa.sa_handler = on_signal;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	signal(SIGPIPE, SIG_IGN);
	/*
	 * Delivered only inside ppoll(): one that came between the checks
	 * of quit and snapshot and the ppoll would otherwise wait for the
	 * next traffic (none, once the client is idle).
	 */
	sigemptyset(&block);
	sigaddset(&block, SIGTERM);
	sigaddset(&block, SIGINT);
	sigaddset(&block, SIGUSR1);
	sigprocmask(SIG_BLOCK, &block, &unblocked);
	sigdelset(&unblocked, SIGTERM);
	sigdelset(&unblocked, SIGINT);
	sigdelset(&unblocked, SIGUSR1);

	if (!quiet) {
		printf("ready\n");
		fflush(stdout);
	}

	while (!quit) {
		int64_t now = now_ns(), wake = -1;
		struct timespec ts, *tsp = NULL;
		int n = 1;

		if (snapshot) {
			snapshot = 0;
			for (i = 0; i < MAX_CONNS; i++)
				if (conns[i])
					print_conn(conns[i], 1);
		}

		pfd[0].fd = lfd;
		pfd[0].events = POLLIN;
		pc[0] = NULL;
		for (i = 0; i < MAX_CONNS; i++) {
			struct conn *c = conns[i];
			struct dir *d[2];
			int j;

			if (!c)
				continue;
			d[0] = &c->c2s;
			d[1] = &c->s2c;
			/* One pollfd per socket: reading one direction and
			 * writing the other. */
			for (j = 0; j < 2; j++) {
				struct dir *in = d[j], *out = d[!j];

				/* Nothing left to read or write here. */
				pfd[n].fd = in->eof && out->shut ? -1 : in->from;
				pfd[n].events = 0;
				if (!in->eof && in->queued < MAX_QUEUED)
					pfd[n].events |= POLLIN;
				if (out->head && out->head->due <= now)
					pfd[n].events |= POLLOUT;
				if (out->head && out->head->due > now &&
				    (wake < 0 || out->head->due < wake))
					wake = out->head->due;
				pc[n++] = c;
			}
		}
		if (wake >= 0) {
			ts.tv_sec = (wake - now) / 1000000000;
			ts.tv_nsec = (wake - now) % 1000000000;
			tsp = &ts;
		}
		if (ppoll(pfd, n, tsp, &unblocked) < 0) {
			if (errno == EINTR)
				continue;
			perror("cdebench-proxy: ppoll");
			break;
		}
		if (pfd[0].revents & POLLIN)
			accept_conn(lfd, argv[optind + 1]);
		for (i = 1; i < n; i++) {
			struct conn *c = pc[i];
			int c2s = (i & 1) != 0;  /* odd: the client socket */
			struct dir *in = c2s ? &c->c2s : &c->s2c;
			struct dir *out = c2s ? &c->s2c : &c->c2s;

			if (!in->eof &&
			    (pfd[i].revents & (POLLIN | POLLHUP | POLLERR)))
				pump_in(c, in, c2s);
			/* Gone both ways: drop what was still to be sent to
			 * it, and stop reading what would be. */
			if (in->eof && (pfd[i].revents & (POLLHUP | POLLERR))) {
				free_queue(out);
				out->eof = out->shut = 1;
			}
		}
		now = now_ns();
		for (i = 0; i < MAX_CONNS; i++) {
			struct conn *c = conns[i];

			if (!c)
				continue;
			if (pump_out(&c->c2s, now) < 0 ||
			    pump_out(&c->s2c, now) < 0 ||
			    (c->c2s.shut && c->s2c.shut))
				close_conn(i);
		}
	}
	for (i = 0; i < MAX_CONNS; i++)
		if (conns[i])
			close_conn(i);
	close(lfd);
	return 0;
}
