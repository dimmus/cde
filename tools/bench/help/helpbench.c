/*
 * CDE benchmarks, help: the help dialog of libDtHelp under load.
 *
 * Licensed under the LGPL 2.1 license.
 *
 *   helpbench [-r REPEAT] [-j FILE] [-V VOL[,VOL...]] [-w WORD] [-m PAGE]
 *             [-n MAXTOPICS] [-S DIR] [-l] [CASE|all ...]
 *
 * Drives a DtHelpDialog in-process, as a user would: every topic of the
 * volumes in turn, Backtrack, a man page paged down to the end, and the
 * Index dialog's search over all volumes.  Each case runs REPEAT times
 * (default 5) and reports the median, per operation, of
 *
 *   ns, cpu_ns          wall and CPU time of this process
 *   mallocs, requests,  from ../libcdebench_preload.so
 *   rtrips
 *   XAllocColor ...     calls counted by libhelpbench_preload.so
 *                       (colours, fonts, images: see helppreload.c)
 *
 * The program re-executes itself with both libraries preloaded.  Every
 * operation ends with the event queue drained and one XSync, so the
 * round trips per operation include that one.
 *
 * Cases (an operation in brackets):
 *   dialog      create a help dialog on the first volume's home topic,
 *               display it, destroy it [one dialog]
 *   topics      every topic of the volumes, in tree order, in one dialog,
 *               as hyperlinks go [one topic]; run 1 is the first visit,
 *               the median shows what revisiting costs
 *   requests    the same, but setting the volume with each topic, as an
 *               application's help request does [one topic]
 *   revisit     visit 4 topics, then go to them in turn again, as
 *               hyperlinks back and forth do [one topic]
 *   backtrack   visit 20 topics, then Backtrack 19 times [one Backtrack]
 *   manpage     show the man page PAGE (default ksh) [one load]
 *   pagedown    page down through it to the end [one page]
 *   selresize   all of the man page selected, the window made wider and
 *               narrower again [one width change]
 *   index_open  a new help dialog, Index...: the dialog opens on the
 *               complete index of the current volume [one dialog]
 *   index_all   a new help dialog, Index..., "Complete Index" of "All
 *               Volumes", until the search is done [one search, with the
 *               dialog's first scan of the volumes]
 *   index_again the same again in the same dialog [one search]
 *   index_word  a new help dialog, Index..., "Entries with" WORD
 *               (default "file") in all volumes [one search]
 *
 * -S DIR writes a screenshot (PPM) of the help window after each topic of
 * "topics", each page of "pagedown" and the result list of each index
 * case, so that two builds can be compared with cmp(1); one run unless
 * -r is given (runs after the first are NAME.runN.ppm).  Search results
 * are also printed (item counts) with -v.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <errno.h>
#include <execinfo.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <Xm/CutPaste.h>
#include <Xm/DrawnB.h>
#include <Xm/List.h>
#include <Xm/PushBG.h>
#include <Xm/ScrollBar.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>
#include <Dt/Help.h>
#include <Dt/HelpDialog.h>

#include "../benchutil.h"
#include "helpbench.h"

/* libDtHelp's exported topic-tree access (no public header). */
extern int _DtHelpCeOpenVolume(void *canvas, char *volFile, void **retVol);
extern int _DtHelpCeCloseVolume(void *canvas, void *vol);
extern int _DtHelpCeGetTopTopicId(void *vol, char **retId);
extern int _DtHelpCeGetTopicChildren(void *vol, char *id, char ***retIds);
extern void _DtHelpCeFreeStringArray(char **array);

#define MAX_REPEAT 64
#define NCOUNT HELPBENCH_NCOUNTS

static const char *count_names[NCOUNT] = {
	"XAllocColor", "XAllocNamedColor", "XLookupColor", "XParseColor",
	"XLoadQueryFont", "XLoadFont", "XQueryFont", "XCreateFontSet",
	"XListFonts", "XPutImage", "XCreatePixmap", "XCreateGC",
};

static XtAppContext app;
static Display *dpy;
static Widget top;
static Widget dlg;		/* the help dialog of the current case */
static int cur_vol = -1;	/* the volume it shows (index in vols) */

static char **vols;		/* volume paths (-V) */
static int nvols;
static const char *word = "file";
static const char *manpage = "ksh";
static long max_topics = 100000;
static const char *shotdir;
static int verbose;

struct topic {
	int vol;
	char *id;
};
static struct topic *topics;
static int cur_run;		/* run of the case (0: the first) */
static int repeat_set;		/* -r given */
static long ntopics;

static const struct helpbench_xcounts *xcounts(void)
{
	static const struct helpbench_xcounts *c;
	static int looked;

	if (!looked) {
		c = dlsym(RTLD_DEFAULT, "helpbench_xcounts");
		looked = 1;
	}
	return c;
}

/* Re-exec with ../libcdebench_preload.so and libhelpbench_preload.so. */
static void preload(char **argv)
{
	char exe[PATH_MAX], buf[3 * PATH_MAX], *dir;
	const char *old = getenv("LD_PRELOAD");
	ssize_t len;

	if ((bench_counters() && xcounts()) || getenv("CDEBENCH_NO_PRELOAD") ||
	    getenv("CDEBENCH_PRELOAD_TRIED"))
		return;
	len = readlink("/proc/self/exe", exe, sizeof exe - 1);
	if (len <= 0)
		return;
	exe[len] = '\0';
	dir = dirname(exe);
	snprintf(buf, sizeof buf, "%s/../libcdebench_preload.so:"
		 "%s/libhelpbench_preload.so%s%s", dir, dir,
		 old && *old ? ":" : "", old && *old ? old : "");
	setenv("LD_PRELOAD", buf, 1);
	setenv("CDEBENCH_PRELOAD_TRIED", "1", 1);
	execv("/proc/self/exe", argv);
}

/* HELPBENCH_WATCHDOG=SECONDS: print where the program is and exit if a
 * case hangs (no window manager, a lost event...). */
static void watchdog(int sig)
{
	void *bt[64];
	static const char msg[] = "helpbench: watchdog expired\n";

	(void)sig;
	if (write(2, msg, sizeof msg - 1) < 0)
		_exit(3);
	backtrace_symbols_fd(bt, backtrace(bt, 64), 2);
	_exit(3);
}

/* Drain the event queue, then make sure the server has caught up. */
static void settle(void)
{
	for (;;) {
		while (XtAppPending(app) & (XtIMXEvent | XtIMTimer))
			XtAppProcessEvent(app, XtIMXEvent | XtIMTimer);
		XSync(dpy, False);
		if (!(XtAppPending(app) & (XtIMXEvent | XtIMTimer)))
			break;
	}
}

/* Run the event loop until the work procs queued so far have finished. */
static int idle_reached;

static void wake(XtPointer data, XtIntervalId *id)
{
	(void)data;
	(void)id;
}

/*
 * XtAppProcessEvent returns only after it has dispatched an event, a
 * timer or an input; the work procs it calls while it waits do not make
 * it return.  So the sentinel also queues an immediate timer.
 */
static Boolean idle_wp(XtPointer data)
{
	(void)data;
	idle_reached = 1;
	XtAppAddTimeOut(app, 0, wake, NULL);
	return True;
}

static void arm_idle(void)
{
	idle_reached = 0;
	XtAppAddWorkProc(app, idle_wp, NULL);
}

static void wait_idle(void)
{
	while (!idle_reached)
		XtAppProcessEvent(app, XtIMAll);
	settle();
}

/*----------------------------------------------------------------------
 * Screenshots (-S)
 *----------------------------------------------------------------------*/
static void shot(Widget w, const char *name)
{
	char path[PATH_MAX];
	XWindowAttributes wa;
	XImage *img;
	FILE *f;
	int x, y;

	if (!shotdir || !w || !XtIsRealized(w))
		return;
	settle();
	if (!XGetWindowAttributes(dpy, XtWindow(w), &wa))
		return;
	img = XGetImage(dpy, XtWindow(w), 0, 0, wa.width, wa.height,
			AllPlanes, ZPixmap);
	if (!img)
		return;
	if (cur_run > 0)
		snprintf(path, sizeof path, "%s/%s.run%d.ppm", shotdir, name,
			 cur_run + 1);
	else
		snprintf(path, sizeof path, "%s/%s.ppm", shotdir, name);
	if ((f = fopen(path, "wb"))) {
		fprintf(f, "P6\n%d %d\n255\n", img->width, img->height);
		for (y = 0; y < img->height; y++)
			for (x = 0; x < img->width; x++) {
				unsigned long p = XGetPixel(img, x, y);

				putc((p >> 16) & 0xff, f);
				putc((p >> 8) & 0xff, f);
				putc(p & 0xff, f);
			}
		fclose(f);
	}
	XDestroyImage(img);
}

/*----------------------------------------------------------------------
 * The help dialog
 *----------------------------------------------------------------------*/
static void dialog_create(const char *vol, const char *loc)
{
	Arg args[8];
	int n = 0;

	XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
	XtSetArg(args[n], DtNhelpVolume, vol); n++;
	XtSetArg(args[n], DtNlocationId, loc ? loc : "_HOMETOPIC"); n++;
	XtSetArg(args[n], DtNcolumns, 80); n++;
	XtSetArg(args[n], DtNrows, 40); n++;
	dlg = DtCreateHelpDialog(top, "helpbench", args, n);
	XtManageChild(dlg);
	cur_vol = strcmp(vol, vols[0]) ? -1 : 0;
	settle();
}

static void dialog_destroy(void)
{
	if (dlg) {
		XtDestroyWidget(XtParent(dlg));
		dlg = NULL;
		cur_vol = -1;
		settle();
	}
}

/*
 * Show a topic as a hyperlink does: a new location in the volume shown,
 * the volume set only when it changes.  With 'request', set both, as an
 * application does for each of its help requests (the dialog then opens
 * the volume again).
 */
static void show_topic_how(int vol, const char *id, int request)
{
	Arg args[3];
	int n = 0;

	if (request || vol != cur_vol) {
		XtSetArg(args[n], DtNhelpVolume, vols[vol]); n++;
	}
	XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
	XtSetArg(args[n], DtNlocationId, id); n++;
	XtSetValues(dlg, args, n);
	cur_vol = vol;
	settle();
}

static void show_topic(int vol, const char *id)
{
	show_topic_how(vol, id, 0);
}

static Widget child(Widget w, const char *name)
{
	Widget c = XtNameToWidget(w, name);

	if (!c)
		bench_die("helpbench: no widget %s", name);
	return c;
}

static void activate(Widget w)
{
	XmPushButtonCallbackStruct cb;
	XButtonEvent ev;

	memset(&ev, 0, sizeof ev);
	ev.type = ButtonRelease;
	ev.display = dpy;
	ev.window = XtWindow(XtParent(w));
	memset(&cb, 0, sizeof cb);
	cb.reason = XmCR_ACTIVATE;
	cb.event = (XEvent *)&ev;
	cb.click_count = 1;
	XtCallCallbacks(w, XmNactivateCallback, &cb);
}

/* A radio button of the index dialog switched on, as by a click. */
static void toggle_on(Widget w)
{
	XmToggleButtonCallbackStruct cb;
	XButtonEvent ev;

	memset(&ev, 0, sizeof ev);
	ev.type = ButtonRelease;
	ev.display = dpy;
	ev.window = XtWindow(XtParent(w));
	XmToggleButtonGadgetSetState(w, True, False);
	memset(&cb, 0, sizeof cb);
	cb.reason = XmCR_VALUE_CHANGED;
	cb.event = (XEvent *)&ev;
	cb.set = True;
	XtCallCallbacks(w, XmNvalueChangedCallback, &cb);
}

/* The display area: the drawn button next to the vertical scroll bar. */
static Widget display_area(Widget *sb)
{
	Widget vs = child(dlg, "*DisplayDtHelpVertScrollBar");
	Widget form = XtParent(vs);
	WidgetList kids;
	Cardinal nk, i;

	XtVaGetValues(form, XmNchildren, &kids, XmNnumChildren, &nk, NULL);
	for (i = 0; i < nk; i++)
		if (XmIsDrawnButton(kids[i])) {
			if (sb)
				*sb = vs;
			return kids[i];
		}
	bench_die("helpbench: no display area");
}

/*----------------------------------------------------------------------
 * The topic list (-V)
 *----------------------------------------------------------------------*/
static void add_topics(void *vh, int vol, char *id, int depth)
{
	char **kids = NULL;
	int i, n;

	if (ntopics >= max_topics)
		return;
	topics = realloc(topics, (ntopics + 1) * sizeof *topics);
	if (!topics)
		bench_die("helpbench: out of memory");
	topics[ntopics].vol = vol;
	topics[ntopics].id = strdup(id);
	ntopics++;
	if (depth > 64)
		return;
	n = _DtHelpCeGetTopicChildren(vh, id, &kids);
	for (i = 0; i < n && kids && kids[i]; i++)
		add_topics(vh, vol, kids[i], depth + 1);
	if (kids)
		_DtHelpCeFreeStringArray(kids);
}

static void load_topics(void)
{
	int v;

	for (v = 0; v < nvols; v++) {
		void *vh = NULL;
		char *home = NULL;

		if (_DtHelpCeOpenVolume(NULL, vols[v], &vh) != 0 || !vh)
			bench_die("helpbench: cannot open volume %s", vols[v]);
		if (_DtHelpCeGetTopTopicId(vh, &home) && home) {
			add_topics(vh, v, home, 0);
			free(home);
		}
		_DtHelpCeCloseVolume(NULL, vh);
	}
	if (!ntopics)
		bench_die("helpbench: no topics");
}

/*----------------------------------------------------------------------
 * Cases
 *----------------------------------------------------------------------*/
static int setup_dialog(long n)
{
	(void)n;
	return 0;
}

static long run_dialog(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		dialog_create(vols[0], NULL);
		dialog_destroy();
	}
	return n;
}

static int setup_topics(long n)
{
	(void)n;
	if (!dlg)
		dialog_create(vols[0], NULL);
	return 0;
}

static long run_topics(long n)
{
	long i;

	(void)n;
	for (i = 0; i < ntopics; i++) {
		show_topic(topics[i].vol, topics[i].id);
		if (shotdir) {
			char name[64];

			snprintf(name, sizeof name, "topic-%04ld", i);
			shot(XtParent(dlg), name);
		}
	}
	return ntopics;
}

static long run_requests(long n)
{
	long i;

	(void)n;
	for (i = 0; i < ntopics; i++)
		show_topic_how(topics[i].vol, topics[i].id, 1);
	return ntopics;
}

/* back and forth among a few topics seen a moment ago */
#define REVISIT_SET 4

static int setup_revisit(long n)
{
	long i;

	(void)n;
	dialog_destroy();
	dialog_create(vols[0], NULL);
	for (i = 0; i < REVISIT_SET && i < ntopics; i++)
		show_topic(topics[i].vol, topics[i].id);
	return ntopics < 2;
}

static long run_revisit(long n)
{
	long i, set = ntopics < REVISIT_SET ? ntopics : REVISIT_SET;

	for (i = 0; i < n; i++) {
		show_topic(topics[i % set].vol, topics[i % set].id);
		if (shotdir) {
			char name[64];

			snprintf(name, sizeof name, "revisit-%03ld", i);
			shot(XtParent(dlg), name);
		}
	}
	return n;
}

#define BACKTRACK_DEPTH 20

static int setup_backtrack(long n)
{
	long i;

	(void)n;
	dialog_destroy();
	dialog_create(vols[0], NULL);
	for (i = 0; i < BACKTRACK_DEPTH && i < ntopics; i++)
		show_topic(topics[i].vol, topics[i].id);
	return ntopics < 2;
}

static long run_backtrack(long n)
{
	Widget back = child(dlg, "*backTrack");
	long i, depth = ntopics < BACKTRACK_DEPTH ? ntopics : BACKTRACK_DEPTH;

	(void)n;
	for (i = 0; i < depth - 1; i++) {
		activate(back);
		settle();
		if (shotdir) {
			char name[64];

			snprintf(name, sizeof name, "back-%02ld", i);
			shot(XtParent(dlg), name);
		}
	}
	return depth - 1;
}

static void teardown_dialog(void)
{
	dialog_destroy();
}

static int setup_manpage(long n)
{
	(void)n;
	dialog_destroy();
	dialog_create(vols[0], NULL);
	return 0;
}

static void show_manpage(void)
{
	Arg args[2];

	XtSetArg(args[0], DtNhelpType, DtHELP_TYPE_MAN_PAGE);
	XtSetArg(args[1], DtNmanPage, manpage);
	XtSetValues(dlg, args, 2);
	settle();
}

static long run_manpage(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		/* switch away and back, so that each load formats it again */
		if (i)
			show_topic(0, topics[0].id);
		show_manpage();
	}
	return n;
}

static long pages;

static int setup_pagedown(long n)
{
	(void)n;
	dialog_destroy();
	dialog_create(vols[0], NULL);
	show_manpage();
	return 0;
}

static long run_pagedown(long n)
{
	Widget sb, da = display_area(&sb);
	String params[1] = { "1" };
	int value, last = -1;

	(void)n;
	pages = 0;
	for (;;) {
		XtVaGetValues(sb, XmNvalue, &value, NULL);
		if (value == last)
			break;
		last = value;
		XtCallActionProc(da, "PageUpOrDown", NULL, params, 1);
		settle();
		pages++;
		if (shotdir) {
			char name[64];

			snprintf(name, sizeof name, "page-%04ld", pages);
			shot(XtParent(dlg), name);
		}
	}
	if (verbose)
		fprintf(stderr, "helpbench: %ld pages\n", pages);
	return pages;
}

/* -S: the selected text, as Copy puts it on the clipboard, in DIR/NAME.txt */
static void save_selection(const char *name)
{
	static const char *formats[] = { "STRING", "COMPOUND_TEXT",
					 "UTF8_STRING" };
	Widget da = display_area(NULL);
	Window win = XtWindow(da);
	char path[PATH_MAX];
	unsigned long len = 0, got = 0;
	unsigned int f;
	long id;
	char *buf;
	FILE *fp;

	XtCallActionProc(da, "CopyToClipboard", NULL, NULL, 0);
	settle();
	for (f = 0; f < sizeof formats / sizeof formats[0]; f++)
		if (XmClipboardInquireLength(dpy, win, (char *)formats[f],
					     &len) == ClipboardSuccess && len)
			break;
	if (!len)
		return;
	buf = malloc(len);
	if (!buf)
		return;
	if (XmClipboardRetrieve(dpy, win, (char *)formats[f], buf, len, &got,
				&id) == ClipboardSuccess) {
		snprintf(path, sizeof path, "%s/%s.txt", shotdir, name);
		if ((fp = fopen(path, "wb"))) {
			fwrite(buf, 1, got, fp);
			fclose(fp);
		}
	}
	free(buf);
}

/*
 * The man page all selected, then the window made narrower and wider:
 * each width change lays the page out again and carries the selection
 * over (_DtCanvasResize gets the selected segments before and sets the
 * selection again after).
 */
static int setup_selresize(long n)
{
	Widget da;

	(void)n;
	dialog_destroy();
	dialog_create(vols[0], NULL);
	show_manpage();
	da = display_area(NULL);
	XtCallActionProc(da, "SelectAll", NULL, NULL, 0);
	settle();
	return 0;
}

static long run_selresize(long n)
{
	Widget shell = XtParent(dlg);
	Dimension w;
	long i;

	XtVaGetValues(shell, XmNwidth, &w, NULL);
	for (i = 0; i < n; i++) {
		XtVaSetValues(shell, XmNwidth, (Dimension)(w + (i & 1 ? 0 : 60)),
			      NULL);
		settle();
	}
	if (shotdir) {
		shot(shell, "selresize");
		save_selection("selresize");
	}
	return n;
}

/* The index dialog of the current help dialog, opened if need be. */
static Widget index_dialog(void)
{
	Widget form;

	arm_idle();
	activate(child(dlg, "*keyword"));
	wait_idle();
	form = child(dlg, "*parentForm");
	return form;
}

static int result_count(Widget form)
{
	int items = 0;

	XtVaGetValues(child(form, "*resultList"), XmNitemCount, &items, NULL);
	return items;
}

/*
 * Search all volumes: the complete index (word == NULL) or the entries
 * containing word.  Switching from "Current" to "All Volumes" starts it.
 */
static void index_search_all(Widget form, const char *what, const char *w)
{
	Widget cur = child(form, "*srchCurVolRadBtn");
	Widget all = child(form, "*srchAllVolRadBtn");

	if (w) {
		toggle_on(child(form, "*srchContainsRadBtn"));
		settle();
		XmTextFieldSetString(child(form, "*srchWord"), (char *)w);
		settle();
	}
	XmToggleButtonGadgetSetState(cur, True, False);
	XmToggleButtonGadgetSetState(all, False, False);
	arm_idle();
	toggle_on(all);
	wait_idle();
	if (verbose)
		fprintf(stderr, "helpbench: %s: %d result lines\n", what,
			result_count(form));
	if (shotdir)
		shot(XtParent(form), what);
}

static int setup_none(long n)
{
	(void)n;
	dialog_destroy();
	return 0;
}

static long run_index_all(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		dialog_destroy();
		dialog_create(vols[0], NULL);
		index_search_all(index_dialog(), "index_all", NULL);
	}
	return n;
}

static long run_index_word(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		dialog_destroy();
		dialog_create(vols[0], NULL);
		index_search_all(index_dialog(), "index_word", word);
	}
	return n;
}

static Widget index_form;

static int setup_index_again(long n)
{
	(void)n;
	dialog_destroy();
	dialog_create(vols[0], NULL);
	index_form = index_dialog();
	index_search_all(index_form, "index_first", NULL);
	return 0;
}

static long run_index_again(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		/* back to the current volume (searched at once), then all */
		arm_idle();
		toggle_on(child(index_form, "*srchCurVolRadBtn"));
		wait_idle();
		index_search_all(index_form, "index_again", NULL);
	}
	return n;
}

static long run_index_open(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		Widget form;

		dialog_destroy();
		dialog_create(vols[0], NULL);
		form = index_dialog();
		if (verbose)
			fprintf(stderr, "helpbench: index_open: %d result "
				"lines\n", result_count(form));
		if (shotdir)
			shot(XtParent(form), "index_open");
	}
	return n;
}

struct hcase {
	const char *name;
	const char *desc;
	long n;
	int (*setup)(long n);
	long (*run)(long n);
	void (*teardown)(void);
};

static const struct hcase cases[] = {
	{ "dialog", "create, display and destroy a help dialog", 5,
	  setup_dialog, run_dialog, NULL },
	{ "topics", "every topic of the volumes in one dialog", 1,
	  setup_topics, run_topics, NULL },
	{ "requests", "every topic again, as help requests (volume set)", 1,
	  setup_topics, run_requests, NULL },
	{ "revisit", "hyperlinks back and forth among 4 topics", 40,
	  setup_revisit, run_revisit, teardown_dialog },
	{ "backtrack", "Backtrack over 20 visited topics", 1,
	  setup_backtrack, run_backtrack, teardown_dialog },
	{ "manpage", "load a man page", 3,
	  setup_manpage, run_manpage, teardown_dialog },
	{ "pagedown", "page down through the man page", 1,
	  setup_pagedown, run_pagedown, teardown_dialog },
	{ "selresize", "resize with the whole man page selected", 10,
	  setup_selresize, run_selresize, teardown_dialog },
	{ "index_open", "Index... in a new dialog (current volume)", 1,
	  setup_none, run_index_open, teardown_dialog },
	{ "index_all", "complete index of all volumes, new dialog", 1,
	  setup_none, run_index_all, teardown_dialog },
	{ "index_again", "the same again in the same dialog", 1,
	  setup_index_again, run_index_again, teardown_dialog },
	{ "index_word", "entries with WORD in all volumes, new dialog", 1,
	  setup_none, run_index_word, teardown_dialog },
};
#define NCASES ((int)(sizeof cases / sizeof cases[0]))

struct result {
	double ns, cpu_ns, mallocs, requests, rtrips;
	double x[NCOUNT];
};

static void snap(struct cdebench_counters *c, unsigned long *x)
{
	const struct cdebench_counters *bc = bench_counters();
	const struct helpbench_xcounts *xc = xcounts();

	if (bc)
		*c = *bc;
	else
		memset(c, 0, sizeof *c);
	if (xc)
		memcpy(x, xc, sizeof(*xc));
	else
		memset(x, 0, NCOUNT * sizeof *x);
}

static int run_case(const struct hcase *hc, int repeat, struct result *res,
		    struct result *first)
{
	static double v[5 + NCOUNT][MAX_REPEAT];
	int r, k;

	for (r = 0; r < repeat; r++) {
		struct cdebench_counters c0, c1;

		cur_run = r;
		unsigned long x0[NCOUNT], x1[NCOUNT];
		double t0, t1, p0, p1;
		long ops;

		if (hc->setup && hc->setup(hc->n))
			return BENCH_SKIP;
		snap(&c0, x0);
		p0 = bench_cpu_ns();
		t0 = bench_now_ns();
		ops = hc->run(hc->n);
		t1 = bench_now_ns();
		p1 = bench_cpu_ns();
		snap(&c1, x1);
		if (hc->teardown)
			hc->teardown();
		if (ops < 1)
			ops = 1;
		v[0][r] = (t1 - t0) / ops;
		v[1][r] = (p1 - p0) / ops;
		v[2][r] = (double)(c1.mallocs - c0.mallocs) / ops;
		v[3][r] = (double)(c1.requests - c0.requests) / ops;
		v[4][r] = (double)(c1.rtrips - c0.rtrips) / ops;
		for (k = 0; k < (int)NCOUNT; k++)
			v[5 + k][r] = (double)(x1[k] - x0[k]) / ops;
		if (r == 0) {
			first->ns = v[0][0];
			first->cpu_ns = v[1][0];
			first->mallocs = v[2][0];
			first->requests = v[3][0];
			first->rtrips = v[4][0];
			for (k = 0; k < (int)NCOUNT; k++)
				first->x[k] = v[5 + k][0];
		}
		if (verbose) {
			long pages_rss = 0, dummy;
			FILE *sf = fopen("/proc/self/statm", "r");

			if (sf) {
				if (fscanf(sf, "%ld %ld", &dummy,
					   &pages_rss) != 2)
					pages_rss = 0;
				fclose(sf);
			}
			fprintf(stderr, "helpbench: %s run %d: %ld ops, "
				"%.0f ns/op, rss %ld KB\n", hc->name, r, ops,
				v[0][r], pages_rss * (sysconf(_SC_PAGESIZE)
							/ 1024));
		}
	}
	res->ns = bench_median(v[0], repeat);
	res->cpu_ns = bench_median(v[1], repeat);
	res->mallocs = bench_median(v[2], repeat);
	res->requests = bench_median(v[3], repeat);
	res->rtrips = bench_median(v[4], repeat);
	for (k = 0; k < (int)NCOUNT; k++)
		res->x[k] = bench_median(v[5 + k], repeat);
	return 0;
}

/* The members of one result, as in the JSON of ../benchutil.c. */
static void json_result(FILE *f, const struct result *r)
{
	int k;

	fprintf(f, "\"ns_per_op\": %.2f, \"cpu_ns_per_op\": %.2f, "
		"\"mallocs_per_op\": %.4f, \"requests_per_op\": %.4f, "
		"\"round_trips_per_op\": %.4f", r->ns, r->cpu_ns,
		r->mallocs, r->requests, r->rtrips);
	for (k = 0; k < (int)NCOUNT; k++)
		fprintf(f, ", \"%s_per_op\": %.4f", count_names[k], r->x[k]);
}

static void usage(FILE *f)
{
	fprintf(f, "usage: helpbench [-r REPEAT] [-j FILE] [-V VOL[,VOL...]] "
		   "[-w WORD] [-m PAGE]\n"
		   "                 [-n MAXTOPICS] [-S DIR] [-v] [-l] "
		   "[CASE|all ...]\n");
}

static void add_volume(const char *spec)
{
	const char *dir = getenv("HELPBENCH_DIR");
	char path[PATH_MAX];

	if (!dir)
		dir = "/usr/dt/appconfig/help/C";
	if (strchr(spec, '/'))
		snprintf(path, sizeof path, "%s", spec);
	else
		snprintf(path, sizeof path, "%s/%s.sdl", dir, spec);
	vols = realloc(vols, (nvols + 1) * sizeof *vols);
	if (!vols)
		bench_die("helpbench: out of memory");
	vols[nvols++] = strdup(path);
}

int main(int argc, char **argv)
{
	const char *json = NULL, *vspec = "Filemgr";
	int repeat = 5, opt, i, k, ran = 0, first;
	FILE *jf = NULL;
	char *spec, *tok, *save;

	preload(argv);
	while ((opt = getopt(argc, argv, "r:j:V:w:m:n:S:vlh")) != -1) {
		switch (opt) {
		case 'r':
			repeat_set = 1;
			repeat = atoi(optarg);
			if (repeat < 1 || repeat > MAX_REPEAT)
				bench_die("helpbench: bad repeat count");
			break;
		case 'j': json = optarg; break;
		case 'V': vspec = optarg; break;
		case 'w': word = optarg; break;
		case 'm': manpage = optarg; break;
		case 'n': max_topics = atol(optarg); break;
		case 'S': shotdir = optarg; break;
		case 'v': verbose = 1; break;
		case 'l':
			for (i = 0; i < NCASES; i++)
				printf("%-12s %s\n", cases[i].name,
				       cases[i].desc);
			return 0;
		case 'h': usage(stdout); return 0;
		default: usage(stderr); return 1;
		}
	}
	first = optind;
	for (i = first; i < argc; i++) {
		int known = !strcmp(argv[i], "all");

		for (k = 0; k < NCASES && !known; k++)
			known = !strcmp(argv[i], cases[k].name);
		if (!known)
			bench_die("helpbench: unknown case %s", argv[i]);
	}
	spec = strdup(vspec);
	for (tok = strtok_r(spec, ",", &save); tok;
	     tok = strtok_r(NULL, ",", &save))
		add_volume(tok);
	free(spec);
	if (shotdir) {
		char *dir = strdup(shotdir), *p;

		for (p = dir + 1; *p; p++)
			if (*p == '/') {
				*p = '\0';
				mkdir(dir, 0755);
				*p = '/';
			}
		if (mkdir(dir, 0755) && errno != EEXIST)
			bench_die("helpbench: %s: %s", dir, strerror(errno));
		free(dir);
		if (!repeat_set)
			repeat = 1;
	}

	if (getenv("HELPBENCH_WATCHDOG")) {
		signal(SIGALRM, watchdog);
		alarm(atoi(getenv("HELPBENCH_WATCHDOG")));
	}
	XtSetLanguageProc(NULL, NULL, NULL);
	top = XtVaOpenApplication(&app, "Helpbench", NULL, 0, &argc, argv,
				  NULL, applicationShellWidgetClass,
				  XmNmappedWhenManaged, False,
				  XmNwidth, 10, XmNheight, 10, NULL);
	dpy = XtDisplay(top);
	XtRealizeWidget(top);
	load_topics();
	if (verbose)
		fprintf(stderr, "helpbench: %ld topics\n", ntopics);

	if (json) {
		jf = strcmp(json, "-") ? fopen(json, "w") : stdout;
		if (!jf)
			bench_die("helpbench: %s: %s", json, strerror(errno));
		fprintf(jf, "{\n  \"bench\": \"helpbench\",\n  \"version\": 1,"
			"\n  \"repeat\": %d,\n  \"counters\": %s,\n"
			"  \"topics\": %ld,\n  \"volumes\": [", repeat,
			bench_counters() && xcounts() ? "true" : "false",
			ntopics);
		for (i = 0; i < nvols; i++)
			fprintf(jf, "%s\"%s\"", i ? ", " : "", vols[i]);
		fprintf(jf, "],\n  \"cases\": [");
	}
	if (!bench_counters() || !xcounts())
		fprintf(stderr, "helpbench: preload libraries not loaded; "
			"no counts\n");
	printf("%-12s %12s %12s %9s %9s %7s %6s %6s %6s %6s\n", "case",
	       "ns/op", "cpu-ns/op", "mallocs", "requests", "rtrips",
	       "AllocC", "LoadQF", "FontSt", "PutImg");
	for (i = 0; i < NCASES; i++) {
		const struct hcase *hc = &cases[i];
		struct result res, r1;
		int sel = first >= argc;

		for (k = first; k < argc && !sel; k++)
			sel = !strcmp(argv[k], "all") ||
			      !strcmp(argv[k], hc->name);
		if (!sel)
			continue;
		if (run_case(hc, repeat, &res, &r1) == BENCH_SKIP) {
			printf("%-12s skipped\n", hc->name);
			continue;
		}
		printf("%-12s %12.0f %12.0f %9.1f %9.2f %7.2f %6.2f %6.2f "
		       "%6.2f %6.2f\n", hc->name, res.ns, res.cpu_ns,
		       res.mallocs, res.requests, res.rtrips, res.x[0],
		       res.x[4], res.x[7], res.x[9]);
		printf("%-12s %12.0f %12.0f %9.1f %9.2f %7.2f %6.2f %6.2f "
		       "%6.2f %6.2f\n", "  (run 1)", r1.ns, r1.cpu_ns,
		       r1.mallocs, r1.requests, r1.rtrips, r1.x[0], r1.x[4],
		       r1.x[7], r1.x[9]);
		fflush(stdout);
		if (jf) {
			fprintf(jf, "%s\n    {\"name\": \"%s\", "
				"\"group\": \"help\", \"n\": %ld, ",
				ran ? "," : "", hc->name, hc->n);
			json_result(jf, &res);
			fprintf(jf, ",\n     \"first_run\": {");
			json_result(jf, &r1);
			fprintf(jf, "}}");
		}
		ran++;
	}
	if (jf) {
		fprintf(jf, "\n  ]\n}\n");
		if (jf != stdout)
			fclose(jf);
	}
	dialog_destroy();
	return 0;
}
