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
 * GraphicCacheI.h: a per-display cache of the pixmaps made from help
 * graphics files, so revisiting a topic does not re-decode its TIFFs,
 * XPMs, etc. and re-allocate their colours.
 */
#ifndef _DtHelpGraphicCacheI_h
#define _DtHelpGraphicCacheI_h

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

/*
 * Everything that determines the pixmap _DtHelpProcessGraphic makes
 * for a file, plus the identity of the file itself.
 */
typedef struct {
    Display         *dpy;
    int              screen;
    int              depth;
    Colormap         colormap;
    Visual          *visual;
    Pixel            fg;
    Pixel            bg;
    unsigned short   media_resolution;
    const char      *path;
    /* filled in by _DtHelpGrCacheLookup */
    unsigned long    dev;
    unsigned long    ino;
    long long        size;
    long long        mtime_sec;
    long             mtime_nsec;
} _DtHelpGrCacheKey;

/*
 * Look up the graphic for "key".  Returns 1 and the cached pixmap, mask
 * and size on a hit (the caller now holds a reference, released with
 * _DtHelpGrCacheRelease).  Returns 0 on a miss, with the file identity
 * filled into "key" for _DtHelpGrCacheAdd, or -1 when the file cannot be
 * cached (no such file).
 */
extern int _DtHelpGrCacheLookup (
    _DtHelpGrCacheKey *key,
    Pixmap            *ret_pix,
    Pixmap            *ret_mask,
    Dimension         *ret_width,
    Dimension         *ret_height);

/*
 * Hand a freshly made graphic to the cache.  On success the cache owns
 * the pixmap, mask and colours (and frees "pixels"), the caller holds one
 * reference, and True is returned.  On failure nothing changes and the
 * caller still owns everything.  "free_colors" is False when the colours
 * must never be passed to XFreeColors (XPM graphics).
 */
extern Boolean _DtHelpGrCacheAdd (
    _DtHelpGrCacheKey *key,
    Pixmap             pix,
    Pixmap             mask,
    Dimension          width,
    Dimension          height,
    Pixel             *pixels,
    int                num_pixels,
    Boolean            free_colors);

/*
 * Drop a reference to the cached graphic whose pixmap is "pix".  Returns
 * False when "pix" is not in the cache (the caller frees it as before).
 */
extern Boolean _DtHelpGrCacheRelease (
    Display           *dpy,
    Pixmap             pix);

#endif /* _DtHelpGraphicCacheI_h */
