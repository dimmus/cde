/*
 * CDE benchmarks: dtsvcbench, micro-benchmarks of libDtSvc and libDtWidget.
 *
 * Licensed under the LGPL 2.1 license.
 *
 *   dtsvcbench [-r REPEAT] [-s SCALE] [-j FILE] [-l] [CASE|GROUP|all ...]
 *
 * Groups and cases (see -l):
 *   mm      _DtDtsMMInit(0) with a valid and with a stale dtdbcache file
 *   dts     DtDtsDataToDataType over a synthetic tree of 10k entries;
 *           DtDtsDataTypeToAttributeValue
 *   action  DtActionLabel, DtActionIcon, DtActionExists
 *   spc     XeSPCSpawn("/bin/true") under RLIMIT_NOFILE 1024 and under
 *           the hard limit (up to 1048576)
 *   widget  DtComboBoxAddItem, DtSpinBoxAddItem, DtEditor cases
 *
 * The action database is the installed one (/etc/dt and /usr/dt
 * appconfig/types, or CDEBENCH_TYPES_PATH, a DTDATABASESEARCHPATH value),
 * plus a one-record file in a private directory that the "stale" case
 * touches.  HOME is a private empty directory, so that the user's
 * ~/.dt/types do not count.  The dtdbcache file is private too
 * (/tmp/dtdbcache_:cdebench<pid>), and removed at exit.
 *
 * The typing cases use a tree generated in a temporary directory, or the
 * directory named by CDEBENCH_DTS_DIR (for example an sshfs mount).
 *
 * The X cases (action, spc, widget) need $DISPLAY and are skipped
 * without one (the SPC library needs the command invoker set up).  The spc
 * cases fork a child per run (the SPC library reads the fd limit once),
 * so their malloc counts are not seen.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Dt/Dt.h>
#include <Dt/Dts.h>
#include <Dt/DtsMM.h>
#include <Dt/Action.h>
#include <Dt/ComboBox.h>
#include <Dt/SpinBox.h>
#include <Dt/Editor.h>
#include <Dt/Spc.h>
#include <Dt/CmdInv.h>

#include "benchutil.h"

#define DEFAULT_TYPES_PATH \
	"/etc/dt/appconfig/types/%L,/etc/dt/appconfig/types," \
	"/usr/dt/appconfig/types/%L,/usr/dt/appconfig/types"

static char workdir[] = "/tmp/dtsvcbench.XXXXXX";
static char typesdir[sizeof workdir + 16];
static char stalefile[sizeof typesdir + 32];
static char treedir[sizeof workdir + 16];
static char cachedisplay[64];
static char *real_display;

static XtAppContext app;
static Display *dpy;
static Widget toplevel;
static int x_tried;

/* ------------------------------------------------------------------ */
/* Environment                                                         */
/* ------------------------------------------------------------------ */

static void rm_tree(const char *path)
{
	struct stat st;
	DIR *d;
	struct dirent *e;
	char sub[4096];

	if (lstat(path, &st))
		return;
	if (S_ISDIR(st.st_mode) && (d = opendir(path))) {
		while ((e = readdir(d))) {
			if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
				continue;
			snprintf(sub, sizeof sub, "%s/%s", path, e->d_name);
			rm_tree(sub);
		}
		closedir(d);
		rmdir(path);
	} else {
		unlink(path);
	}
}

static void cache_path(char *buf, size_t len)
{
	/* What _DtDtsMMCacheName(1) makes of DISPLAY. */
	snprintf(buf, len, "%s/%s%s", _DTDTSMMTEMPDIR, _DTDTSMMTEMPFILE,
		 cachedisplay);
}

static void cleanup(void)
{
	char cache[256];

	if (*cachedisplay) {
		cache_path(cache, sizeof cache);
		unlink(cache);
	}
	if (workdir[0] == '/' && strcmp(workdir, "/tmp/dtsvcbench.XXXXXX"))
		rm_tree(workdir);
}

static void write_file(const char *path, const void *data, size_t len)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fd < 0 || write(fd, data, len) != (ssize_t)len)
		bench_die("dtsvcbench: %s: %s", path, strerror(errno));
	close(fd);
}

static void setup_env(void)
{
	static const char record[] =
		"DATA_ATTRIBUTES CdeBenchStale\n{\n"
		"\tDESCRIPTION\tA record the dtsvcbench stale case touches.\n"
		"}\n";
	const char *types = getenv("CDEBENCH_TYPES_PATH");
	char *path, home[sizeof workdir + 16];

	if (!mkdtemp(workdir))
		bench_die("dtsvcbench: mkdtemp: %s", strerror(errno));
	atexit(cleanup);
	snprintf(typesdir, sizeof typesdir, "%s/types", workdir);
	snprintf(stalefile, sizeof stalefile, "%s/cdebench.dt", typesdir);
	snprintf(treedir, sizeof treedir, "%s/tree", workdir);
	snprintf(home, sizeof home, "%s/home", workdir);
	if (mkdir(typesdir, 0755) || mkdir(home, 0755))
		bench_die("dtsvcbench: mkdir: %s", strerror(errno));
	write_file(stalefile, record, sizeof record - 1);

	if (!types || !*types)
		types = DEFAULT_TYPES_PATH;
	path = malloc(strlen(typesdir) + strlen(types) + 2);
	sprintf(path, "%s,%s", typesdir, types);
	setenv("DTDATABASESEARCHPATH", path, 1);
	free(path);
	setenv("HOME", home, 1);

	/*
	 * The cache file is named after DISPLAY; use our own name and keep
	 * the real one for XtOpenDisplay.
	 */
	if (getenv("DISPLAY") && *getenv("DISPLAY"))
		real_display = strdup(getenv("DISPLAY"));
	snprintf(cachedisplay, sizeof cachedisplay, ":cdebench%d",
		 (int)getpid());
	setenv("DISPLAY", cachedisplay, 1);
}

/* Open the display and initialize DtSvc and the databases, once. */
static int need_x(void)
{
	static char *fake_argv[] = { (char *)"dtsvcbench", NULL };
	int fake_argc = 1;

	if (x_tried)
		return dpy ? 0 : 1;
	x_tried = 1;
	if (!real_display)
		return 1;
	XtToolkitInitialize();
	app = XtCreateApplicationContext();
	dpy = XtOpenDisplay(app, real_display, "dtsvcbench", "DtSvcBench",
			    NULL, 0, &fake_argc, fake_argv);
	if (!dpy)
		return 1;
	toplevel = XtVaAppCreateShell("dtsvcbench", "DtSvcBench",
				      applicationShellWidgetClass, dpy,
				      XmNallowShellResize, True, NULL);
	if (!DtAppInitialize(app, dpy, toplevel, "dtsvcbench", "DtSvcBench"))
		bench_die("dtsvcbench: DtAppInitialize failed");
	DtDbLoad();
	/* What the action code does before it runs a command. */
	_DtInitializeCommandInvoker(dpy, NULL, NULL, NULL, app);
	return 0;
}

/* Run the event loop until nothing is pending. */
static void drain(void)
{
	XSync(dpy, False);
	while (XtAppPending(app))
		XtAppProcessEvent(app, XtIMAll);
}

/* ------------------------------------------------------------------ */
/* mm: the dtdbcache file                                              */
/* ------------------------------------------------------------------ */

static long stale_stamp;

static int mm_valid_setup(long n)
{
	(void)n;
	/* (Re)build the shared cache for our DISPLAY, untimed. */
	if (!_DtDtsMMInit(1))
		return 1;
	return 0;
}

static long mm_valid_run(long n)
{
	long i;

	for (i = 0; i < n; i++)
		if (!_DtDtsMMInit(0))
			bench_die("dtsvcbench: _DtDtsMMInit(0) failed");
	return n;
}

static void touch_stale(void)
{
	struct timeval tv[2];

	/* A different whole second each time: the cache keeps st_mtime. */
	if (!stale_stamp)
		stale_stamp = (long)time(NULL) - 1000000;
	tv[0].tv_sec = tv[1].tv_sec = stale_stamp++;
	tv[0].tv_usec = tv[1].tv_usec = 0;
	if (utimes(stalefile, tv))
		bench_die("dtsvcbench: utimes %s: %s", stalefile,
			  strerror(errno));
}

static int mm_stale_setup(long n)
{
	(void)n;
	touch_stale();
	return !_DtDtsMMInit(1);
}

static long mm_stale_run(long n)
{
	long i;

	/*
	 * Each time, the shared cache no longer matches the database
	 * files: _DtDtsMMInit maps it, finds it stale, and rebuilds a
	 * private one (as every client does until dtsession rebuilds the
	 * shared one).
	 */
	for (i = 0; i < n; i++) {
		touch_stale();
		if (!_DtDtsMMInit(0))
			bench_die("dtsvcbench: _DtDtsMMInit(0) failed");
	}
	return n;
}

/* ------------------------------------------------------------------ */
/* dts: data typing and attributes                                     */
/* ------------------------------------------------------------------ */

struct tree_entry {
	char *path;
	struct stat st;
	int is_link;
	char *link_target;
};

static struct tree_entry *tree;
static long ntree, tree_built_for;
static char **types_seen;
static long ntypes_seen;

static const char *const suffixes[] = {
	".c", ".h", ".txt", ".html", ".ps", ".gif", ".tar", ".Z", ".sh",
	".dt", ".xpm", ".pdf", ".sdl", ".tiff", ".man", ".o"
};
#define N_SUFFIXES (int)(sizeof suffixes / sizeof suffixes[0])

static void make_tree(long n)
{
	static const unsigned char elf[64] = {
		0x7f, 'E', 'L', 'F', 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		2, 0, 0x3e, 0, 1, 0, 0, 0
	};
	static const char text[] =
		"Plain text without a suffix, for the content rules.\n";
	char path[4096], target[4096];
	long i;

	rm_tree(treedir);
	if (mkdir(treedir, 0755))
		bench_die("dtsvcbench: mkdir %s: %s", treedir,
			  strerror(errno));
	for (i = 0; i < n; i++) {
		switch (i % 10) {
		case 0: case 1: case 2: case 3:
			snprintf(path, sizeof path, "%s/file%06ld%s", treedir,
				 i, suffixes[(i / 10) % N_SUFFIXES]);
			write_file(path, text, sizeof text - 1);
			break;
		case 4: case 5:
			snprintf(path, sizeof path, "%s/NOSUFFIX%06ld",
				 treedir, i);
			write_file(path, text, sizeof text - 1);
			break;
		case 6:
			snprintf(path, sizeof path, "%s/dir%06ld", treedir, i);
			if (mkdir(path, 0755))
				bench_die("dtsvcbench: mkdir: %s",
					  strerror(errno));
			break;
		case 7:
			snprintf(path, sizeof path, "%s/prog%06ld", treedir, i);
			write_file(path, elf, sizeof elf);
			chmod(path, 0755);
			break;
		case 8:
			snprintf(path, sizeof path, "%s/link%06ld", treedir, i);
			snprintf(target, sizeof target, "file%06ld.c",
				 i - 8);
			if (symlink(target, path))
				bench_die("dtsvcbench: symlink: %s",
					  strerror(errno));
			break;
		default:
			snprintf(path, sizeof path, "%s/dirlink%06ld",
				 treedir, i);
			snprintf(target, sizeof target, "dir%06ld", i - 3);
			if (symlink(target, path))
				bench_die("dtsvcbench: symlink: %s",
					  strerror(errno));
			break;
		}
	}
}

/* Read the tree's entries and their stat, as dtfile does before typing. */
static void load_tree(const char *dir)
{
	DIR *d = opendir(dir);
	struct dirent *e;
	long cap = 1024;

	if (!d)
		bench_die("dtsvcbench: %s: %s", dir, strerror(errno));
	tree = calloc(cap, sizeof *tree);
	ntree = 0;
	while ((e = readdir(d))) {
		struct tree_entry *t;
		char path[4096];

		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		if (ntree == cap) {
			cap *= 2;
			tree = realloc(tree, cap * sizeof *tree);
		}
		t = &tree[ntree];
		memset(t, 0, sizeof *t);
		snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
		if (lstat(path, &t->st))
			continue;
		t->path = strdup(path);
		if (S_ISLNK(t->st.st_mode)) {
			char target[4096];
			ssize_t len = readlink(path, target, sizeof target - 1);

			t->is_link = 1;
			if (len > 0) {
				target[len] = '\0';
				t->link_target = strdup(target);
			}
			/* dtfile types a link by its target's stat */
			if (stat(path, &t->st))
				t->is_link = 2;   /* broken */
		}
		ntree++;
	}
	closedir(d);
}

static void free_tree(void)
{
	long i;

	for (i = 0; i < ntree; i++) {
		free(tree[i].path);
		free(tree[i].link_target);
	}
	free(tree);
	tree = NULL;
	ntree = 0;
}

static int dts_type_setup(long n)
{
	const char *dir = getenv("CDEBENCH_DTS_DIR");

	if (tree)
		return 0;
	if (dir && *dir) {
		load_tree(dir);
	} else {
		if (tree_built_for != n) {
			make_tree(n);
			tree_built_for = n;
		}
		load_tree(treedir);
	}
	/* Map the database before the clock starts. */
	_DtDtsMMInit(0);
	return 0;
}

static void add_type_seen(const char *type)
{
	long i;

	for (i = 0; i < ntypes_seen; i++)
		if (!strcmp(types_seen[i], type))
			return;
	types_seen = realloc(types_seen, (ntypes_seen + 1) *
			     sizeof *types_seen);
	types_seen[ntypes_seen++] = strdup(type);
}

static long dts_type_run(long n)
{
	long i;

	(void)n;
	for (i = 0; i < ntree; i++) {
		struct tree_entry *t = &tree[i];
		char *type;

		if (t->is_link == 2)
			continue;
		type = DtDtsDataToDataType(t->path, NULL, 0, &t->st,
					   t->is_link ? t->link_target : NULL,
					   NULL, NULL);
		if (type) {
			add_type_seen(type);
			DtDtsFreeDataType(type);
		}
	}
	return ntree;
}

static const char *const attrs[] = {
	"ICON", "DESCRIPTION", "ACTIONS", "INSTANCE_ICON", "IS_EXECUTABLE",
	"COPY_TO_ACTION", "MOVE_TO_ACTION", "LINK_TO_ACTION", "IS_TEXT",
	"LABEL"
};
#define N_ATTRS (long)(sizeof attrs / sizeof attrs[0])

static int dts_attr_setup(long n)
{
	(void)n;
	if (!ntypes_seen) {
		/* The types the typing case finds, or a few common ones. */
		static const char *const common[] = {
			"DIRECTORY", "DATA", "C_SRC", "HTML", "POSTSCRIPT",
			"EXECUTABLE", "TEXTFILE", "DTDATA"
		};
		size_t i;

		dts_type_setup((long)(10000 * bench_scale));
		dts_type_run(0);
		for (i = 0; !ntypes_seen && i < sizeof common /
		     sizeof common[0]; i++)
			add_type_seen(common[i]);
	}
	return 0;
}

static long dts_attr_run(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		char *v = DtDtsDataTypeToAttributeValue(
			types_seen[i % ntypes_seen], attrs[i % N_ATTRS], NULL);

		if (v)
			DtDtsFreeAttributeValue(v);
	}
	return n;
}

/* ------------------------------------------------------------------ */
/* action                                                              */
/* ------------------------------------------------------------------ */

static char *action_names[] = {
	"Open", "Print", "TextEditor", "Dtfile", "Terminal", "Dtcalc",
	"DtmailEdit", "OpenInPlace", "Dthelpview", "NoSuchActionCdeBench"
};
#define N_ACTIONS (long)(sizeof action_names / sizeof action_names[0])

static int action_setup(long n)
{
	(void)n;
	return need_x();
}

static long action_label_run(long n)
{
	long i;

	for (i = 0; i < n; i++)
		XtFree(DtActionLabel(action_names[i % N_ACTIONS]));
	return n;
}

static long action_icon_run(long n)
{
	long i;

	for (i = 0; i < n; i++)
		XtFree(DtActionIcon(action_names[i % N_ACTIONS]));
	return n;
}

static long action_exists_run(long n)
{
	long i, found = 0;

	for (i = 0; i < n; i++)
		found += DtActionExists(action_names[i % N_ACTIONS]) ? 1 : 0;
	if (!found)
		bench_die("dtsvcbench: no action exists; is the action "
			  "database installed?");
	return n;
}

/* ------------------------------------------------------------------ */
/* spc: XeSPCSpawn under different fd limits                          */
/* ------------------------------------------------------------------ */

static rlim_t spc_limit;

/*
 * Spawn as the command invoker does (CmdMain.c): a channel without I/O
 * and a synchronous terminator.  A client of the SPC library is not told
 * when a local child ends (only the dtspcd daemon installs a SIGCHLD
 * handler), so wait for it here: the time per spawn is then the time
 * until /bin/true has run, which includes the child's closing of every
 * descriptor up to the fd limit before it execs.
 */
static void spawn_child(long n)
{
	struct rlimit rl;
	char *argv[] = { (char *)"/bin/true", NULL };
	int status;
	long i;

	if (getrlimit(RLIMIT_NOFILE, &rl))
		_exit(2);
	rl.rlim_cur = spc_limit;
	if (rl.rlim_cur > rl.rlim_max)
		rl.rlim_cur = rl.rlim_max;
	if (setrlimit(RLIMIT_NOFILE, &rl))
		_exit(3);
	for (i = 0; i < n; i++) {
		SPC_Channel_Ptr ch = XeSPCOpen(NULL, SPCIO_NOIO |
					       SPCIO_SYNC_TERMINATOR);

		if (ch == SPC_ERROR)
			_exit(4);
		if (XeSPCSpawn(argv[0], NULL, argv, NULL, ch) == SPC_ERROR)
			_exit(6);
		if (waitpid(-1, &status, 0) < 0 || !WIFEXITED(status) ||
		    WEXITSTATUS(status))
			_exit(7);
		XeSPCClose(ch);
	}
	_exit(0);
}

static long spc_run(long n)
{
	int status;
	pid_t pid;

	fflush(NULL);
	pid = fork();
	if (pid < 0)
		bench_die("dtsvcbench: fork: %s", strerror(errno));
	if (pid == 0)
		spawn_child(n);
	if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status) ||
	    WEXITSTATUS(status))
		bench_die("dtsvcbench: the spawning child failed (status "
			  "0x%x)", status);
	return n;
}

static int spc_1k_setup(long n)
{
	(void)n;
	if (need_x())
		return 1;
	spc_limit = 1024;
	return 0;
}

static int spc_max_setup(long n)
{
	struct rlimit rl;

	(void)n;
	if (need_x() || getrlimit(RLIMIT_NOFILE, &rl))
		return 1;
	spc_limit = rl.rlim_max < 1048576 ? rl.rlim_max : 1048576;
	return 0;
}

/* ------------------------------------------------------------------ */
/* widget: DtComboBox, DtSpinBox, DtEditor                            */
/* ------------------------------------------------------------------ */

static Widget box;

static void destroy_box(void)
{
	if (box) {
		XtDestroyWidget(box);
		box = NULL;
		drain();
	}
}

static int combo_setup(long n)
{
	(void)n;
	if (need_x())
		return 1;
	box = DtCreateComboBox(toplevel, "combo", NULL, 0);
	XtManageChild(box);
	if (!XtIsRealized(toplevel))
		XtRealizeWidget(toplevel);
	drain();
	return 0;
}

static long combo_run(long n)
{
	char label[32];
	long i;

	for (i = 0; i < n; i++) {
		XmString s;

		snprintf(label, sizeof label, "item %ld", i);
		s = XmStringCreateLocalized(label);
		DtComboBoxAddItem(box, s, 0, False);
		XmStringFree(s);
	}
	drain();
	return n;
}

static int spin_setup(long n)
{
	(void)n;
	if (need_x())
		return 1;
	box = DtCreateSpinBox(toplevel, "spin", NULL, 0);
	XtManageChild(box);
	if (!XtIsRealized(toplevel))
		XtRealizeWidget(toplevel);
	drain();
	return 0;
}

static long spin_run(long n)
{
	char label[32];
	long i;

	for (i = 0; i < n; i++) {
		XmString s;

		snprintf(label, sizeof label, "item %ld", i);
		s = XmStringCreateLocalized(label);
		DtSpinBoxAddItem(box, s, 0);
		XmStringFree(s);
	}
	drain();
	return n;
}

static char *editor_buf;
static size_t editor_len;
static Widget editors[2];

/*
 * One editor per status line setting, created once and only unmanaged
 * between runs: destroying a DtEditor made with showStatusLine False
 * crashes in libXm's drop site tree update (IntersectWithWidgetAncestors,
 * from TreeUpdateHandler) on the next pass of the event loop.
 */
static int editor_create(int status_line)
{
	if (need_x())
		return 1;
	if (!editors[status_line])
		editors[status_line] = XtVaCreateWidget("editor",
			dtEditorWidgetClass, toplevel,
			DtNshowStatusLine, status_line,
			DtNrows, 40, DtNcolumns, 80, NULL);
	box = editors[status_line];
	XtManageChild(box);
	if (!XtIsRealized(toplevel))
		XtRealizeWidget(toplevel);
	drain();
	return 0;
}

static void editor_teardown(void)
{
	if (box) {
		XtUnmanageChild(box);
		box = NULL;
		drain();
	}
}

/* 1 MB of text with runs of 1 to 3 NULs, about 100k of them. */
static int editor_nul_setup(long n)
{
	size_t i;

	(void)n;
	if (editor_create(False))
		return 1;
	editor_len = 1 << 20;
	free(editor_buf);
	editor_buf = malloc(editor_len);
	for (i = 0; i < editor_len; i++) {
		if (i % 10 < 1 + (i / 10) % 3)
			editor_buf[i] = '\0';
		else if (i % 80 == 79)
			editor_buf[i] = '\n';
		else
			editor_buf[i] = 'a' + (char)(i % 26);
	}
	return 0;
}

static long editor_nul_run(long n)
{
	DtEditorContentRec c;
	long i;

	for (i = 0; i < n; i++) {
		c.type = DtEDITOR_DATA;
		c.value.data.buf = editor_buf;
		c.value.data.length = (unsigned int)editor_len;
		DtEditorSetContents(box, &c);
	}
	drain();
	return n;
}

/* Text with 10k hits of "needle", replaced all at once. */
static int editor_replace_setup(long n)
{
	DtEditorContentRec c;
	size_t len = 0;
	long i;

	if (editor_create(False))
		return 1;
	free(editor_buf);
	editor_buf = malloc(64 * (size_t)n + 1);
	for (i = 0; i < n; i++)
		len += (size_t)sprintf(editor_buf + len,
				       "line %06ld has a needle in it\n", i);
	c.type = DtEDITOR_TEXT;
	c.value.string = editor_buf;
	DtEditorSetContents(box, &c);
	DtEditorSetInsertionPosition(box, 0);
	drain();
	return 0;
}

static long editor_replace_run(long n)
{
	DtEditorChangeValues v;

	v.find = (char *)"needle";
	v.changeTo = (char *)"thread";
	if (!DtEditorChange(box, &v, DtEDITOR_ALL_OCCURRENCES))
		bench_die("dtsvcbench: DtEditorChange found nothing");
	drain();
	return n;
}

/* 10k cursor moves over 1000 lines with the status line shown. */
static int editor_cursor_setup(long n)
{
	DtEditorContentRec c;
	size_t len = 0;
	long i;

	(void)n;
	if (editor_create(True))
		return 1;
	free(editor_buf);
	editor_buf = malloc(64 * 1000 + 1);
	for (i = 0; i < 1000; i++)
		len += (size_t)sprintf(editor_buf + len,
				       "line %06ld of the cursor case\n", i);
	editor_len = len;
	c.type = DtEDITOR_TEXT;
	c.value.string = editor_buf;
	DtEditorSetContents(box, &c);
	drain();
	return 0;
}

static long editor_cursor_run(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		DtEditorSetInsertionPosition(box, (XmTextPosition)
					     ((i * 7919) % editor_len));
		/* The status line updates from the event loop. */
		while (XtAppPending(app))
			XtAppProcessEvent(app, XtIMAll);
	}
	drain();
	return n;
}

static const struct bench_case cases[] = {
	{ "mm-init-valid", "mm",
	  "_DtDtsMMInit(0): map and validate a valid dtdbcache",
	  20, mm_valid_setup, mm_valid_run, NULL },
	{ "mm-init-stale", "mm",
	  "_DtDtsMMInit(0): find the cache stale, rebuild a private one",
	  5, mm_stale_setup, mm_stale_run, NULL },
	{ "dts-type", "dts",
	  "DtDtsDataToDataType per entry of a 10k-entry tree",
	  10000, dts_type_setup, dts_type_run, NULL },
	{ "dts-attr", "dts",
	  "DtDtsDataTypeToAttributeValue",
	  100000, dts_attr_setup, dts_attr_run, NULL },
	{ "action-label", "action", "DtActionLabel",
	  100000, action_setup, action_label_run, NULL },
	{ "action-icon", "action", "DtActionIcon",
	  100000, action_setup, action_icon_run, NULL },
	{ "action-exists", "action", "DtActionExists",
	  100000, action_setup, action_exists_run, NULL },
	{ "spc-spawn-nofile-1k", "spc",
	  "XeSPCSpawn(/bin/true) and wait, RLIMIT_NOFILE 1024",
	  100, spc_1k_setup, spc_run, NULL },
	{ "spc-spawn-nofile-max", "spc",
	  "XeSPCSpawn(/bin/true) and wait, RLIMIT_NOFILE hard limit",
	  100, spc_max_setup, spc_run, NULL },
	{ "combobox-additem", "widget", "DtComboBoxAddItem at the end",
	  1000, combo_setup, combo_run, destroy_box },
	{ "spinbox-additem", "widget", "DtSpinBoxAddItem at the end",
	  1000, spin_setup, spin_run, destroy_box },
	{ "editor-set-nul", "widget",
	  "DtEditorSetContents of 1 MB with ~100k NUL runs",
	  1, editor_nul_setup, editor_nul_run, editor_teardown },
	{ "editor-replace-all", "widget",
	  "DtEditorChange all occurrences, per hit (10k hits)",
	  10000, editor_replace_setup, editor_replace_run, editor_teardown },
	{ "editor-cursor-status", "widget",
	  "DtEditorSetInsertionPosition with the status line shown",
	  10000, editor_cursor_setup, editor_cursor_run, editor_teardown },
};

static void extra_json(FILE *f)
{
	struct rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	fprintf(f, ",\n  \"nofile_hard\": %llu,\n  \"x\": %s",
		(unsigned long long)rl.rlim_max, real_display ? "true" :
		"false");
}

int main(int argc, char **argv)
{
	int status;

	bench_preload(argv);
	setup_env();
	status = bench_main("dtsvcbench", cases,
			    (int)(sizeof cases / sizeof cases[0]), argc, argv,
			    extra_json);
	free_tree();
	return status;
}
