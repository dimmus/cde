/*
 * CDE - Common Desktop Environment
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these libraries and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmResCache.c
 **
 **  Description:
 **  -----------
 **  Before dtsession opens its display it loads the session resources
 **  with dtsession_res: ksh, tr, cat, xrdb and cpp, 6-8 processes and a
 **  blocking wait.  The result depends only on the resource files, a few
 **  environment variables and what xrdb's cpp symbols describe (the X
 **  server, its screens, visuals and extensions).  This file remembers
 **  the result: the RESOURCE_MANAGER and SCREEN_RESOURCES properties that
 **  dtsession_res left behind, keyed on all of those inputs.  When the
 **  key matches, the properties are set directly and dtsession_res is
 **  not run.
 **
 **  The key holds the contents of every file dtsession_res could read,
 **  and the device, inode, size, modification and change times of the
 **  dtsession_res script, xrdb and cpp.  Nothing is cached when one of
 **  the files uses #include (the included file would not be in the
 **  key), when dtsession_res read a file that is not in the key, or when
 **  DT_NO_RESOURCE_CACHE is set in the environment.
 **
 **  The cache lives in $HOME/.dt/cache/dtsession_res-<host>-<display>.
 **
 *****************************************************************************
 *************************************<+>*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include "Sm.h"
#include "SmRestore.h"

#define RES_CACHE_MAGIC		"DTSESSION_RES_CACHE 1\n"
#define RES_CACHE_DIR		"/.dt/cache"
#define RES_CACHE_PREFIX	"/dtsession_res-"
#define DTSESSION_RES		CDE_INSTALLATION_TOP "/bin/dtsession_res"
#define MAX_CANDIDATES		8

#ifndef XRDB_PROGRAM
#define XRDB_PROGRAM		"xrdb"
#endif

typedef struct {
    char	*buf;
    size_t	len;
    size_t	size;
    int		failed;
} Str;

/* the state of one root window property */
typedef struct {
    int		present;
    char	*data;
    unsigned long len;
} PropState;


static void
StrAdd(
    Str		*s,
    const char	*data,
    size_t	n)
{
    if (s->failed)
	return;
    if (s->len + n + 1 > s->size)
    {
	size_t	size = (s->size == 0) ? 4096 : s->size;
	char	*buf;

	while (s->len + n + 1 > size)
	    size *= 2;
	if ((buf = realloc(s->buf, size)) == NULL)
	{
	    s->failed = 1;
	    return;
	}
	s->buf = buf;
	s->size = size;
    }
    memcpy(s->buf + s->len, data, n);
    s->len += n;
    s->buf[s->len] = '\0';
}

static void
StrPrintf(
    Str		*s,
    const char	*fmt,
    ...)
{
    char	tmp[1024];
    va_list	ap;
    int		n;

    va_start(ap, fmt);
    n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (n < 0 || n >= (int) sizeof(tmp))
    {
	s->failed = 1;
	return;
    }
    StrAdd(s, tmp, (size_t) n);
}

static void
StrAddLine(
    Str		*s,
    const char	*tag,
    const char	*value)
{
    StrAdd(s, tag, strlen(tag));
    StrAdd(s, " ", 1);
    if (value != NULL)
	StrAdd(s, value, strlen(value));
    else
	StrAdd(s, "(unset)", 7);
    StrAdd(s, "\n", 1);
}

/*
 * KeyFile - what identifies a file's contents for the key: present or
 * not, device, inode, size, mode, modification and change time.
 */
static void
KeyFile(
    Str		*k,
    const char	*path)
{
    struct stat	st;

    StrAddLine(k, "F", path);
    if (stat(path, &st) != 0)
    {
	StrPrintf(k, "- %d\n", errno);
	return;
    }
    StrPrintf(k, "%lu %lu %lld %lo %lld.%09ld %lld.%09ld\n",
	      (unsigned long) st.st_dev, (unsigned long) st.st_ino,
	      (long long) st.st_size, (unsigned long) st.st_mode,
	      (long long) st.st_mtim.tv_sec, (long) st.st_mtim.tv_nsec,
	      (long long) st.st_ctim.tv_sec, (long) st.st_ctim.tv_nsec);
}

/*
 * KeyFileContents - a resource file goes into the key with its whole
 * contents (they are small), so that a file rewritten with the same
 * contents - dt.resources is rewritten at every session save - still
 * matches.  Readability counts too: dtsession_res skips files it cannot
 * read.
 */
#define MAX_RESOURCE_FILE	(1024 * 1024)

static void
KeyFileContents(
    Str		*k,
    const char	*path)
{
    struct stat	st;
    char	*buf;
    int		fd;
    ssize_t	n;
    size_t	got = 0;

    StrAddLine(k, "C", path);
    if ((fd = open(path, O_RDONLY)) < 0)
    {
	StrPrintf(k, "- %d\n", errno);
	return;
    }
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) ||
	st.st_size > MAX_RESOURCE_FILE ||
	(buf = malloc((size_t) st.st_size + 1)) == NULL)
    {
	close(fd);
	k->failed = 1;
	return;
    }
    while (got < (size_t) st.st_size &&
	   (n = read(fd, buf + got, (size_t) st.st_size - got)) > 0)
	got += (size_t) n;
    close(fd);
    if (got != (size_t) st.st_size)
    {
	free(buf);
	k->failed = 1;
	return;
    }
    StrPrintf(k, "%lu\n", (unsigned long) got);
    StrAdd(k, buf, got);
    StrAdd(k, "\n", 1);
    free(buf);
}

/*
 * KeyServer - everything about the server that xrdb turns into cpp
 * symbols (and some more): vendor, release, protocol version, the
 * screens with their sizes, depths and visuals, and the extensions.
 */
static void
KeyServer(
    Str		*k,
    Display	*dpy)
{
    int		i, j, v, n = 0;
    char	**ext;

    StrAddLine(k, "D", DisplayString(dpy));
    StrAddLine(k, "V", ServerVendor(dpy));
    StrPrintf(k, "R %d %d %d %d\n", VendorRelease(dpy),
	      ProtocolVersion(dpy), ProtocolRevision(dpy), ScreenCount(dpy));

    for (i = 0; i < ScreenCount(dpy); i++)
    {
	Screen	*scr = ScreenOfDisplay(dpy, i);

	StrPrintf(k, "S %d %d %d %d %d %d %lu\n", i,
		  WidthOfScreen(scr), HeightOfScreen(scr),
		  WidthMMOfScreen(scr), HeightMMOfScreen(scr),
		  DefaultDepthOfScreen(scr),
		  (unsigned long) XVisualIDFromVisual(DefaultVisualOfScreen(scr)));
	for (j = 0; j < scr->ndepths; j++)
	{
	    Depth	*d = &scr->depths[j];

	    for (v = 0; v < d->nvisuals; v++)
	    {
		Visual	*vis = &d->visuals[v];

		StrPrintf(k, "v %d %lu %d %d %d %lx %lx %lx\n", d->depth,
			  (unsigned long) vis->visualid, vis->class,
			  vis->bits_per_rgb, vis->map_entries,
			  vis->red_mask, vis->green_mask, vis->blue_mask);
	    }
	}
    }

    if ((ext = XListExtensions(dpy, &n)) != NULL)
    {
	for (i = 0; i < n; i++)
	    StrAddLine(k, "X", ext[i]);
	XFreeExtensionList(ext);
    }
}

/*
 * Candidates - the files dtsession_res reads for the given options;
 * this follows dtloadresources.src.
 */
static int
Candidates(
    char	**out,
    Boolean	sys,
    Boolean	xdefaults,
    const char	*file)
{
    const char	*lang = getenv("LANG");
    const char	*home = getenv("HOME");
    const char	*ow = getenv("OPENWINHOME");
    char	buf[MAXPATHLEN + 1];
    int		n = 0;

    if (lang == NULL) lang = "";
    if (home == NULL) home = "";
    if (ow == NULL) ow = "";

#define ADD(...) \
    do { \
	if (snprintf(buf, sizeof(buf), __VA_ARGS__) >= (int) sizeof(buf)) \
	    return -1; \
	if ((out[n] = strdup(buf)) == NULL) \
	    return -1; \
	n++; \
    } while (0)

    if (sys)
    {
	ADD("%s/lib/Xdefaults", ow);
	ADD("%s/config/%s/sys.resources", CDE_INSTALLATION_TOP, lang);
	ADD("%s/config/C/sys.resources", CDE_INSTALLATION_TOP);
	ADD("%s/config/%s/sys.resources", CDE_CONFIGURATION_TOP, lang);
    }
    if (xdefaults)
    {
	ADD("%s/.Xdefaults", home);
	ADD("%s/.OWdefaults", home);
    }
    if (file != NULL)
	ADD("%s", file);
#undef ADD

    return n;
}

static void
FreeCandidates(
    char	**c,
    int		n)
{
    while (n > 0)
	free(c[--n]);
}

static char *
CachePath(void)
{
    const char	*home = getenv("HOME");
    const char	*disp = getenv("DISPLAY");
    char	host[256];
    char	*path, *p;
    size_t	len;
    struct stat	st;

    if (home == NULL || *home == '\0' || disp == NULL)
	return NULL;
    if (gethostname(host, sizeof(host)) != 0)
	return NULL;
    host[sizeof(host) - 1] = '\0';

    len = strlen(home) + strlen(RES_CACHE_DIR) + strlen(RES_CACHE_PREFIX) +
	  strlen(host) + 1 + strlen(disp) + 1;
    if ((path = malloc(len)) == NULL)
	return NULL;

    /* make $HOME/.dt and $HOME/.dt/cache when needed */
    snprintf(path, len, "%s/.dt", home);
    if (stat(path, &st) != 0 && mkdir(path, 0755) != 0 && errno != EEXIST)
    {
	free(path);
	return NULL;
    }
    snprintf(path, len, "%s%s", home, RES_CACHE_DIR);
    if (stat(path, &st) != 0 && mkdir(path, 0700) != 0 && errno != EEXIST)
    {
	free(path);
	return NULL;
    }

    snprintf(path, len, "%s%s%s", home, RES_CACHE_DIR, RES_CACHE_PREFIX);
    p = path + strlen(path);
    snprintf(p, len - (p - path), "%s-%s", host, disp);
    for (; *p != '\0'; p++)
    {
	if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
	      (*p >= '0' && *p <= '9') || *p == '.' || *p == '-'))
	    *p = '_';
    }

    return path;
}

static int
ReadExact(
    FILE	*fp,
    char	**data,
    long	len)
{
    *data = NULL;
    if (len < 0)		/* an absent property: just the empty line */
	return (getc(fp) == '\n') ? 0 : -1;
    if ((*data = malloc((size_t) len + 1)) == NULL)
	return -1;
    if (len > 0 && fread(*data, 1, (size_t) len, fp) != (size_t) len)
    {
	free(*data);
	*data = NULL;
	return -1;
    }
    (*data)[len] = '\0';
    return (getc(fp) == '\n') ? 0 : -1;
}

/*
 * LoadCache - read the cache file; True when its key is 'key' and it
 * holds a state for every screen.
 */
static Boolean
LoadCache(
    const char	*path,
    Str		*key,
    int		nscreens,
    PropState	*rm,
    PropState	*scr)
{
    FILE	*fp;
    char	line[128], *data = NULL;
    long	len;
    int		i, s;
    Boolean	ok = False;

    if ((fp = fopen(path, "r")) == NULL)
	return False;

    if (fgets(line, sizeof(line), fp) == NULL ||
	strcmp(line, RES_CACHE_MAGIC) != 0)
	goto done;

    if (fgets(line, sizeof(line), fp) == NULL ||
	sscanf(line, "K %ld", &len) != 1 || len != (long) key->len ||
	ReadExact(fp, &data, len) != 0 ||
	memcmp(data, key->buf, key->len) != 0)
	goto done;
    free(data);
    data = NULL;

    if (fgets(line, sizeof(line), fp) == NULL ||
	sscanf(line, "R %ld", &len) != 1 || ReadExact(fp, &rm->data, len) != 0)
	goto done;
    rm->present = (len >= 0);
    rm->len = (len >= 0) ? (unsigned long) len : 0;

    for (i = 0; i < nscreens; i++)
    {
	if (fgets(line, sizeof(line), fp) == NULL ||
	    sscanf(line, "S %d %ld", &s, &len) != 2 || s != i ||
	    ReadExact(fp, &scr[i].data, len) != 0)
	    goto done;
	scr[i].present = (len >= 0);
	scr[i].len = (len >= 0) ? (unsigned long) len : 0;
    }

    if (fgets(line, sizeof(line), fp) == NULL || strcmp(line, "E\n") != 0)
	goto done;

    ok = True;

done:
    free(data);
    fclose(fp);
    return ok;
}

static void
WriteProp(
    FILE	*fp,
    const char	*tag,
    PropState	*p)
{
    if (!p->present)
    {
	fprintf(fp, "%s -1\n\n", tag);
	return;
    }
    fprintf(fp, "%s %lu\n", tag, p->len);
    if (p->len > 0)
	fwrite(p->data, 1, p->len, fp);
    putc('\n', fp);
}

static void
SaveCache(
    const char	*path,
    Str		*key,
    int		nscreens,
    PropState	*rm,
    PropState	*scr)
{
    char	*tmp;
    size_t	len = strlen(path) + 8;
    int		fd, i;
    FILE	*fp;
    char	tag[32];

    if ((tmp = malloc(len)) == NULL)
	return;
    snprintf(tmp, len, "%sXXXXXX", path);
    if ((fd = mkstemp(tmp)) < 0)
    {
	free(tmp);
	return;
    }
    if ((fp = fdopen(fd, "w")) == NULL)
    {
	close(fd);
	unlink(tmp);
	free(tmp);
	return;
    }

    fputs(RES_CACHE_MAGIC, fp);
    fprintf(fp, "K %lu\n", (unsigned long) key->len);
    fwrite(key->buf, 1, key->len, fp);
    putc('\n', fp);
    WriteProp(fp, "R", rm);
    for (i = 0; i < nscreens; i++)
    {
	snprintf(tag, sizeof(tag), "S %d", i);
	WriteProp(fp, tag, &scr[i]);
    }
    fputs("E\n", fp);

    if (fclose(fp) != 0 || rename(tmp, path) != 0)
	unlink(tmp);
    free(tmp);
}

static void
GetProp(
    Display	*dpy,
    Window	win,
    Atom	prop,
    PropState	*p,
    Boolean	*usable)
{
    Atom		type;
    int			format;
    unsigned long	nitems, after;
    unsigned char	*data = NULL;

    p->present = 0;
    p->data = NULL;
    p->len = 0;
    if (XGetWindowProperty(dpy, win, prop, 0L, 0x7fffffffL / 4, False,
			   AnyPropertyType, &type, &format, &nitems, &after,
			   &data) != Success)
    {
	*usable = False;
	return;
    }
    if (type == None)
	return;
    if (type != XA_STRING || format != 8 || after != 0 ||
	(p->data = malloc(nitems + 1)) == NULL)
    {
	*usable = False;
    }
    else
    {
	memcpy(p->data, data, nitems);
	p->data[nitems] = '\0';
	p->present = 1;
	p->len = nitems;
    }
    if (data != NULL)
	XFree(data);
}

static void
SetProp(
    Display	*dpy,
    Window	win,
    Atom	prop,
    PropState	*p)
{
    if (p->present)
	XChangeProperty(dpy, win, prop, XA_STRING, 8, PropModeReplace,
			(unsigned char *) p->data, (int) p->len);
    else
	XDeleteProperty(dpy, win, prop);
}

/*
 * FilesUsable - the "dtsession_res*files:" entry dtsession_res puts in
 * RESOURCE_MANAGER names the files it read.  They must all be in the
 * key, and none may use #include.
 */
static Boolean
FilesUsable(
    PropState	*rm,
    char	**cand,
    int		ncand)
{
    static const char	tag[] = "dtsession_res*files:";
    char	*p, *end, *list, *tok, *save;
    Boolean	ok = True;
    int		i;

    if (!rm->present)
	return False;
    for (p = rm->data; p != NULL; p = strchr(p, '\n'))
    {
	if (*p == '\n')
	    p++;
	if (strncmp(p, tag, sizeof(tag) - 1) == 0)
	    break;
    }
    if (p == NULL)
	return False;
    p += sizeof(tag) - 1;
    end = strchr(p, '\n');
    if ((list = (end != NULL) ? strndup(p, end - p) : strdup(p)) == NULL)
	return False;

    for (tok = strtok_r(list, " \t", &save); ok && tok != NULL;
	 tok = strtok_r(NULL, " \t", &save))
    {
	FILE	*fp;
	char	line[1024];

	for (i = 0; i < ncand; i++)
	{
	    if (strcmp(tok, cand[i]) == 0)
		break;
	}
	if (i == ncand || (fp = fopen(tok, "r")) == NULL)
	{
	    ok = False;
	    break;
	}
	while (ok && fgets(line, sizeof(line), fp) != NULL)
	{
	    char	*q = line;

	    while (*q == ' ' || *q == '\t')
		q++;
	    if (*q != '#')
		continue;
	    q++;
	    while (*q == ' ' || *q == '\t')
		q++;
	    if (strncmp(q, "include", 7) == 0)
		ok = False;
	}
	fclose(fp);
    }
    free(list);

    return ok;
}

/*************************************<->*************************************
 *
 *  RestoreSessionResources (resourcePath)
 *
 *  Description:
 *  -----------
 *  The same as
 *    RestoreResources(False, "-load", "-system", "-xdefaults",
 *                     [ "-file", resourcePath, ] NULL)
 *  but uses the cached result of the last identical run when there is
 *  one.
 *
 *************************************<->***********************************/
int
RestoreSessionResources(
    char	*resourcePath)
{
    char	*file = (resourcePath != NULL && *resourcePath != '\0')
			? resourcePath : NULL;
    char	*cand[MAX_CANDIDATES];
    int		ncand = 0, nscreens = 0, i, rc;
    Display	*dpy = NULL;
    char	*cachePath = NULL;
    Str		key = { NULL, 0, 0, 0 };
    PropState	rm = { 0, NULL, 0 };
    PropState	*scr = NULL;
    Atom	screenRes;
    Boolean	usable = True;
    char	host[256];

    if (getenv("DT_NO_RESOURCE_CACHE") != NULL ||
	(dpy = XOpenDisplay(NULL)) == NULL)
	goto uncached;

    nscreens = ScreenCount(dpy);
    if ((scr = calloc((size_t) nscreens, sizeof(PropState))) == NULL ||
	(cachePath = CachePath()) == NULL ||
	(ncand = Candidates(cand, True, True, file)) < 0)
    {
	ncand = 0;
	goto uncached;
    }

    /*
     * Build the key.  The files are looked at before dtsession_res runs,
     * so that a file changed meanwhile can only make the next key miss.
     */
    StrAdd(&key, RES_CACHE_MAGIC, strlen(RES_CACHE_MAGIC));
    StrAddLine(&key, "O", file != NULL ? "-load -system -xdefaults -file"
				       : "-load -system -xdefaults");
    StrAddLine(&key, "DISPLAY", getenv("DISPLAY"));
    StrAddLine(&key, "LANG", getenv("LANG"));
    StrAddLine(&key, "HOME", getenv("HOME"));
    StrAddLine(&key, "OPENWINHOME", getenv("OPENWINHOME"));
    if (gethostname(host, sizeof(host)) != 0)
	goto uncached;
    host[sizeof(host) - 1] = '\0';
    StrAddLine(&key, "HOST", host);
    KeyFile(&key, DTSESSION_RES);
    KeyFile(&key, XRDB_PROGRAM);
    KeyFile(&key, "/usr/bin/cpp");
    KeyFile(&key, "/lib/cpp");
    KeyFile(&key, "/usr/libexec/cpp");
    for (i = 0; i < ncand; i++)
	KeyFileContents(&key, cand[i]);
    KeyServer(&key, dpy);
    if (key.failed)
	goto uncached;

    screenRes = XInternAtom(dpy, "SCREEN_RESOURCES", False);

    if (LoadCache(cachePath, &key, nscreens, &rm, scr))
    {
	SetProp(dpy, RootWindow(dpy, 0), XA_RESOURCE_MANAGER, &rm);
	for (i = 0; i < nscreens; i++)
	    SetProp(dpy, RootWindow(dpy, i), screenRes, &scr[i]);
	XSync(dpy, False);
	rc = 0;
	goto done;
    }
    free(rm.data);
    rm.data = NULL;
    for (i = 0; i < nscreens; i++)
    {
	free(scr[i].data);
	scr[i].data = NULL;
    }

    /*
     * Miss: run dtsession_res and remember what it left behind.
     */
    if (file != NULL)
	rc = RestoreResources(False, "-load", "-system", "-xdefaults",
			      "-file", file, NULL);
    else
	rc = RestoreResources(False, "-load", "-system", "-xdefaults", NULL);

    if (rc == 0)
    {
	GetProp(dpy, RootWindow(dpy, 0), XA_RESOURCE_MANAGER, &rm, &usable);
	for (i = 0; i < nscreens; i++)
	    GetProp(dpy, RootWindow(dpy, i), screenRes, &scr[i], &usable);
	if (usable && FilesUsable(&rm, cand, ncand))
	    SaveCache(cachePath, &key, nscreens, &rm, scr);
	else
	    unlink(cachePath);
    }
    goto done;

uncached:
    if (file != NULL)
	rc = RestoreResources(False, "-load", "-system", "-xdefaults",
			      "-file", file, NULL);
    else
	rc = RestoreResources(False, "-load", "-system", "-xdefaults", NULL);

done:
    if (scr != NULL)
    {
	for (i = 0; i < nscreens; i++)
	    free(scr[i].data);
	free(scr);
    }
    free(rm.data);
    free(key.buf);
    free(cachePath);
    FreeCandidates(cand, ncand);
    if (dpy != NULL)
	XCloseDisplay(dpy);

    return rc;
}
