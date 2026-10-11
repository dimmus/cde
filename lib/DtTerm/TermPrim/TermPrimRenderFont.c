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
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include "TermHeader.h"
#include "TermPrimP.h"
#include "TermPrimDebug.h"
#include "TermPrimRenderP.h"
#include "TermPrimRenderFont.h"
#include <wchar.h>

/* is this character in the font?... */
static Boolean
fontHasChar(XFontStruct *fs, unsigned int byte1, unsigned int byte2)
{
    XCharStruct *cs;

    if ((byte1 < fs->min_byte1) || (byte1 > fs->max_byte1) ||
	    (byte2 < fs->min_char_or_byte2) ||
	    (byte2 > fs->max_char_or_byte2)) {
	return(False);
    }
    if (!fs->per_char) {
	return(True);
    }
    cs = &fs->per_char[(byte1 - fs->min_byte1) *
	    (fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1) +
	    (byte2 - fs->min_char_or_byte2)];
    return(cs->width || cs->lbearing || cs->rbearing || cs->ascent ||
	    cs->descent);
}

/*
 * In a multibyte locale the string is wchar_t's (glibc's are UCS-4).
 * A single font (rather than a font set) is indexed by code point: one
 * byte for an 8 bit font (iso8859-1), two for a matrix font
 * (iso10646-1).  Convert the run to XChar2b's.  A character the font
 * does not have is drawn as its default character (or a space), and a
 * two column character is followed by a space, so that what follows
 * stays in its cells.  Returns the number of XChar2b's...
 */
static int
wcToFontChars(XFontStruct *fs, wchar_t *wcs, int len, XChar2b *out)
{
    unsigned int defByte1 = 0;
    unsigned int defByte2 = ' ';
    unsigned int byte1;
    unsigned int byte2;
    int n = 0;
    int i;

    if (fontHasChar(fs, fs->default_char >> 8, fs->default_char & 0xff)) {
	defByte1 = fs->default_char >> 8;
	defByte2 = fs->default_char & 0xff;
    }

    for (i = 0; i < len; i++) {
	wchar_t wc = wcs[i];

	if ((wc >= 0) && (wc <= 0xffff)) {
	    byte1 = (unsigned int) wc >> 8;
	    byte2 = (unsigned int) wc & 0xff;
	} else {
	    byte1 = 0x100;			/* not in any font	*/
	    byte2 = 0;
	}
	if (!fontHasChar(fs, byte1, byte2)) {
	    byte1 = defByte1;
	    byte2 = defByte2;
	}
	out[n].byte1 = byte1;
	out[n].byte2 = byte2;
	n++;
	if (wcwidth(wc) == 2) {
	    out[n].byte1 = 0;
	    out[n].byte2 = ' ';
	    n++;
	}
    }
    return(n);
}

static void
drawFontString(Display *display, Drawable d, GC gc, int x, int y,
	XFontStruct *fs, Boolean image, Boolean twoByte, char *string,
	XChar2b *string16, int len)
{
    if (!twoByte) {
	if (image) {
	    (void) XDrawImageString(display, d, gc, x, y, string, len);
	} else {
	    (void) XDrawString(display, d, gc, x, y, string, len);
	}
    } else {
	if (image) {
	    (void) XDrawImageString16(display, d, gc, x, y, string16, len);
	} else {
	    (void) XDrawString16(display, d, gc, x, y, string16, len);
	}
    }
}

static void
FontRenderFunction(
    Widget		  w,
    TermFont		  font,
    Pixel		  fg,
    Pixel		  bg,
    unsigned long	  flags,
    int			  x,
    int			  y,
    unsigned char	 *string,
    int			  len
)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    XGCValues values;
    unsigned long valueMask;
    XFontStruct *fontStruct = (XFontStruct *) font->fontInfo;
    XChar2b string16Buffer[256];
    XChar2b *string16 = string16Buffer;
    char *string8 = (char *) string;
    Boolean twoByte = False;
    int columns = len;
    int i;

    if (tpd->mbCurMax > 1) {
	/* the string is wchar_t's (see wcToFontChars())... */
	if (2 * len > XtNumber(string16Buffer)) {
	    string16 = (XChar2b *) XtMalloc(2 * len * sizeof(XChar2b));
	}
	columns = wcToFontChars(fontStruct, (wchar_t *) string, len, string16);
	if ((fontStruct->min_byte1 == 0) && (fontStruct->max_byte1 == 0)) {
	    /* an 8 bit font: draw bytes, in place (we have room: an
	     * XChar2b is two bytes)...
	     */
	    string8 = (char *) string16;
	    for (i = 0; i < columns; i++) {
		string8[i] = (char) string16[i].byte2;
	    }
	} else {
	    twoByte = True;
	}
	len = columns;
    }

    /* set the renderGC... */
    valueMask = (unsigned long) 0;

    /* set the foreground... */
    if (TermIS_SECURE(flags)) {
	if (tpd->renderGC.foreground != bg) {
	    tpd->renderGC.foreground = bg;
	    values.foreground = bg;
	    valueMask |= GCForeground;
	}
    } else {
	if (tpd->renderGC.foreground != fg) {
	    tpd->renderGC.foreground = fg;
	    values.foreground = fg;
	    valueMask |= GCForeground;
	}
    }

    /* set background... */
    if (tpd->renderGC.background != bg) {
	tpd->renderGC.background = bg;
	values.background = bg;
	valueMask |= GCBackground;
    }

    /* set the font for renderGC if necessary */
    if (tpd->renderGC.fid != fontStruct->fid) {
	tpd->renderGC.fid = fontStruct->fid;
	values.font = fontStruct->fid;
	valueMask |= GCFont;
    }

    if (valueMask) {
	(void) XChangeGC(XtDisplay(w), tpd->renderGC.gc, valueMask,
		&values);
    }

    /* draw image string a line of text... */
    if (isDebugFSet('t', 1)) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	/* Fill in the text area so we can see what is going to
	 * be displayed...
	 */
	(void) XFillRectangle(XtDisplay(w),
		XtWindow(w),
		tpd->renderGC.gc,
		x,
		y,
		tpd->cellWidth * len,
		tpd->cellHeight);
	(void) XSync(XtDisplay(w), False);
	(void) shortSleep(100000);
    }
			
    (void) drawFontString(XtDisplay(w), XtWindow(w), tpd->renderGC.gc,
	    x, y + fontStruct->ascent, fontStruct, True, twoByte,
	    string8, string16, len);

    /* handle overstrike... */
    if (TermIS_OVERSTRIKE(flags)) {
	(void) drawFontString(XtDisplay(w), XtWindow(w), tpd->renderGC.gc,
		x + 1, y + fontStruct->ascent, fontStruct, False, twoByte,
		string8, string16, len);
    }

    /* handle the underline enhancement... */
    /* draw the underline... */
    if (TermIS_UNDERLINE(flags)) {
	XDrawLine(XtDisplay(w),			/* Display		*/
		XtWindow(w),			/* Window		*/
		tpd->renderGC.gc,		/* GC			*/
		x,				/* X1			*/
		y + tpd->cellHeight - 1,		/* Y1			*/
		x + columns * tpd->cellWidth - 1,	/* X2			*/
		y + tpd->cellHeight - 1);	/* Y2			*/
    }

    if (string16 != string16Buffer) {
	(void) XtFree((char *) string16);
    }
}

static void
FontDestroyFunction(
    Widget		  w,
    TermFont		  font
)
{
    (void) XtFree((char *) font);
}

static void
FontExtentsFunction(
    Widget		  w,
    TermFont		  font,
    unsigned char	 *string,
    int			  len,
    int			 *widthReturn,
    int			 *heightReturn,
    int			 *ascentReturn
)
{
    XFontStruct *fontStruct = (XFontStruct *) font->fontInfo;

    if (widthReturn) {
	*widthReturn = len * fontStruct->max_bounds.width;
    }
    if (heightReturn) {
	*heightReturn = fontStruct->ascent + fontStruct->descent;
    }
    if (ascentReturn) {
	*ascentReturn = fontStruct->ascent;
    }
    return;
}

TermFont
_DtTermPrimRenderFontCreate(
    Widget		  w,
    XFontStruct		 *fontStruct
)
{
    TermFont termFont;

    termFont = (TermFont) XtMalloc(sizeof(TermFontRec));
    termFont->renderFunction = FontRenderFunction;
    termFont->destroyFunction = FontDestroyFunction;
    termFont->extentsFunction = FontExtentsFunction;
    termFont->fontInfo = (XtPointer) fontStruct;

    return(termFont);
}
