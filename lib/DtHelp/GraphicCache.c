/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
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
/*
 * GraphicCache.c: a per-display cache of the pixmaps made from help
 * graphics files.
 *
 * Every topic view used to re-read, decode and re-colour each of its
 * graphics, and destroying the topic freed them again, so paging back and
 * forth through a volume decoded the same TIFFs over and over.  Graphics
 * are now shared while in use (reference counted) and the most recently
 * used ones are kept for a while after the last topic using them goes
 * away.  An entry is keyed on everything that determines the pixmap: the
 * display, screen, depth, visual, colormap, foreground and background,
 * media resolution, and the file's path, device, inode, size and mtime,
 * so an edited file is decoded again.
 *
 * Unused graphics are kept only for the screen's default colormap, which
 * no client can free; graphics made for any other colormap are freed when
 * the last user goes away, exactly as before.
 */
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xlibint.h>        /* XESetCloseDisplay() */
#include <X11/IntrinsicP.h>     /* XtProcessLock() */

#include "GraphicCacheI.h"
#include "Lock.h"

/* limits for graphics that are cached but not displayed */
#define GR_CACHE_MAX_UNUSED        64
#define GR_CACHE_MAX_UNUSED_BYTES  (16UL * 1024 * 1024)

typedef struct _GrCacheEntry {
    struct _GrCacheEntry *next;
    _DtHelpGrCacheKey     key;          /* key.path points at "path" */
    char                 *path;
    Pixmap                pix;
    Pixmap                mask;
    Dimension             width;
    Dimension             height;
    Pixel                *pixels;
    int                   num_pixels;
    Boolean               free_colors;
    Boolean               stale;        /* the file changed: don't reuse */
    int                   refs;
    unsigned long         last_use;
    unsigned long         bytes;        /* estimated server memory */
} GrCacheEntry;

typedef struct _GrCacheDisplay {
    struct _GrCacheDisplay *next;
    Display                *dpy;
} GrCacheDisplay;

static GrCacheEntry   *CacheList      = NULL;
static GrCacheDisplay *CacheDisplays  = NULL;
static unsigned long   CacheTick      = 0;
static int             UnusedCount    = 0;
static unsigned long   UnusedBytes    = 0;

static void
FreeEntryMemory (GrCacheEntry *e)
{
    free (e->pixels);
    free (e->path);
    free (e);
}

/*
 * Unlink "e" (unused) from the list and free it and its server resources.
 */
static void
DestroyEntry (GrCacheEntry *e)
{
    GrCacheEntry **pp;
    Display       *dpy = e->key.dpy;

    for (pp = &CacheList; *pp != NULL; pp = &(*pp)->next)
        if (*pp == e)
          {
            *pp = e->next;
            break;
          }

    if (e->refs == 0)
      {
        UnusedCount--;
        UnusedBytes -= e->bytes;
      }

    XFreePixmap (dpy, e->pix);
    if (e->mask != None)
        XFreePixmap (dpy, e->mask);
    if (e->num_pixels && e->free_colors)
        XFreeColors (dpy, e->key.colormap, e->pixels, e->num_pixels, 0);

    FreeEntryMemory (e);
}

/*
 * Free least recently used unused graphics until within the limits.
 */
static void
TrimCache (void)
{
    GrCacheEntry *e, *oldest;

    while (UnusedCount > GR_CACHE_MAX_UNUSED ||
           (UnusedCount > 0 && UnusedBytes > GR_CACHE_MAX_UNUSED_BYTES))
      {
        oldest = NULL;
        for (e = CacheList; e != NULL; e = e->next)
            if (e->refs == 0 && (oldest == NULL || e->last_use < oldest->last_use))
                oldest = e;
        if (oldest == NULL)
            break;
        DestroyEntry (oldest);
      }
}

/*
 * The display is closing and the server frees its resources; forget them.
 */
static int
CloseDisplayHook (Display *dpy, XExtCodes *codes)
{
    GrCacheEntry   **pp, *e;
    GrCacheDisplay **pd, *d;

    (void) codes;
    _DtHelpProcessLock();
    for (pp = &CacheList; (e = *pp) != NULL; )
      {
        if (e->key.dpy == dpy)
          {
            *pp = e->next;
            if (e->refs == 0)
              {
                UnusedCount--;
                UnusedBytes -= e->bytes;
              }
            FreeEntryMemory (e);
          }
        else
            pp = &e->next;
      }
    for (pd = &CacheDisplays; (d = *pd) != NULL; pd = &d->next)
        if (d->dpy == dpy)
          {
            *pd = d->next;
            free (d);
            break;
          }
    _DtHelpProcessUnlock();
    return 0;
}

static Boolean
HookDisplay (Display *dpy)
{
    GrCacheDisplay *d;
    XExtCodes      *codes;

    for (d = CacheDisplays; d != NULL; d = d->next)
        if (d->dpy == dpy)
            return True;

    d = (GrCacheDisplay *) malloc (sizeof (GrCacheDisplay));
    if (d == NULL)
        return False;
    codes = XAddExtension (dpy);
    if (codes == NULL)
      {
        free (d);
        return False;
      }
    XESetCloseDisplay (dpy, codes->extension, CloseDisplayHook);
    d->dpy  = dpy;
    d->next = CacheDisplays;
    CacheDisplays = d;
    return True;
}

static Boolean
SameRendering (const _DtHelpGrCacheKey *a, const _DtHelpGrCacheKey *b)
{
    return (a->dpy == b->dpy && a->screen == b->screen &&
            a->depth == b->depth && a->colormap == b->colormap &&
            a->visual == b->visual && a->fg == b->fg && a->bg == b->bg &&
            a->media_resolution == b->media_resolution &&
            strcmp (a->path, b->path) == 0);
}

static Boolean
SameFile (const _DtHelpGrCacheKey *a, const _DtHelpGrCacheKey *b)
{
    return (a->dev == b->dev && a->ino == b->ino && a->size == b->size &&
            a->mtime_sec == b->mtime_sec && a->mtime_nsec == b->mtime_nsec);
}

static Boolean
KeepWhenUnused (const GrCacheEntry *e)
{
    return (!e->stale &&
            e->key.colormap == DefaultColormap (e->key.dpy, e->key.screen));
}

int
_DtHelpGrCacheLookup (
    _DtHelpGrCacheKey *key,
    Pixmap            *ret_pix,
    Pixmap            *ret_mask,
    Dimension         *ret_width,
    Dimension         *ret_height)
{
    struct stat   st;
    GrCacheEntry *e, *next;
    int           result = 0;

    if (key->path == NULL || key->dpy == NULL)
        return -1;

    /* _DtGrOpenFile falls back to "<path>.Z" when <path> is missing */
    if (stat (key->path, &st) != 0)
      {
        size_t len = strlen (key->path);
        char  *zname = (char *) malloc (len + 3);
        int    rc = -1;

        if (zname != NULL)
          {
            memcpy (zname, key->path, len);
            memcpy (zname + len, ".Z", 3);
            rc = stat (zname, &st);
            free (zname);
          }
        if (rc != 0)
            return -1;
      }
    if (!S_ISREG (st.st_mode))
        return -1;

    key->dev        = (unsigned long) st.st_dev;
    key->ino        = (unsigned long) st.st_ino;
    key->size       = (long long) st.st_size;
    key->mtime_sec  = (long long) st.st_mtim.tv_sec;
    key->mtime_nsec = st.st_mtim.tv_nsec;

    _DtHelpProcessLock();
    for (e = CacheList; e != NULL; e = next)
      {
        next = e->next;
        if (e->stale || !SameRendering (&e->key, key))
            continue;

        if (!SameFile (&e->key, key))
          {
            /* the file changed since it was decoded */
            e->stale = True;
            if (e->refs == 0)
                DestroyEntry (e);
            continue;
          }

        if (e->refs == 0)
          {
            UnusedCount--;
            UnusedBytes -= e->bytes;
          }
        e->refs++;
        e->last_use = ++CacheTick;
        *ret_pix    = e->pix;
        *ret_mask   = e->mask;
        *ret_width  = e->width;
        *ret_height = e->height;
        result = 1;
        break;
      }
    _DtHelpProcessUnlock();

    return result;
}

Boolean
_DtHelpGrCacheAdd (
    _DtHelpGrCacheKey *key,
    Pixmap             pix,
    Pixmap             mask,
    Dimension          width,
    Dimension          height,
    Pixel             *pixels,
    int                num_pixels,
    Boolean            free_colors)
{
    GrCacheEntry  *e;
    unsigned long  bpp;

    if (pix == None || key->path == NULL)
        return False;

    e = (GrCacheEntry *) calloc (1, sizeof (GrCacheEntry));
    if (e == NULL)
        return False;
    e->path = strdup (key->path);
    if (e->path == NULL)
      {
        free (e);
        return False;
      }

    _DtHelpProcessLock();
    if (!HookDisplay (key->dpy))
      {
        _DtHelpProcessUnlock();
        free (e->path);
        free (e);
        return False;
      }

    e->key         = *key;
    e->key.path    = e->path;
    e->pix         = pix;
    e->mask        = mask;
    e->width       = width;
    e->height      = height;
    e->pixels      = pixels;
    e->num_pixels  = num_pixels;
    e->free_colors = free_colors;
    e->refs        = 1;
    e->last_use    = ++CacheTick;

    bpp = (key->depth > 16) ? 4 : (key->depth > 8) ? 2 : 1;
    e->bytes = (unsigned long) width * height * bpp;
    if (mask != None)
        e->bytes += ((unsigned long) width + 7) / 8 * height;

    e->next   = CacheList;
    CacheList = e;
    _DtHelpProcessUnlock();

    return True;
}

Boolean
_DtHelpGrCacheRelease (
    Display           *dpy,
    Pixmap             pix)
{
    GrCacheEntry *e;

    _DtHelpProcessLock();
    for (e = CacheList; e != NULL; e = e->next)
        if (e->pix == pix && e->key.dpy == dpy && e->refs > 0)
            break;

    if (e == NULL)
      {
        _DtHelpProcessUnlock();
        return False;
      }

    if (--e->refs == 0)
      {
        UnusedCount++;
        UnusedBytes += e->bytes;
        e->last_use = ++CacheTick;
        if (!KeepWhenUnused (e))
            DestroyEntry (e);
        TrimCache ();
      }
    _DtHelpProcessUnlock();

    return True;
}
