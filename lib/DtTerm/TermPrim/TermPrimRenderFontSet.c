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

#include "TermHeader.h"
#include "TermPrimP.h"
#include "TermPrimDebug.h"
#include "TermPrimRenderP.h"
#include "TermPrimRenderFontSet.h"

typedef struct _TermFontSetRec {
    XFontSet fontSet;
    int ascent;
    int height;
    int width;
    /* the logical height of a run of text is the largest ascent plus the
     * largest descent of the fonts used to draw it (Xlib's generic
     * output method), so it lies between these two...
     */
    int minRunHeight;		/* smallest ascent + descent of a font	*/
    int maxRunHeight;		/* largest ascent + largest descent	*/
    int asciiRunHeight;		/* that of a run of ASCII characters	*/
} TermFontSetRec, *TermFontSet;

/* Text height may be smaller than cellHeight (happens when using font
 * sets).  In this case we need to fill the background manually and then
 * use X*DrawString instead of X*DrawImageString.  Decide from the fonts
 * in the set when we can (always the case with a single font), so that
 * we need not measure every run: 1 if every run is shorter than a cell,
 * 0 if none is, and -1 if it depends on the run...
 */
static int
runsNeedFill(TermFontSet termFontSet, int cellHeight)
{
    if (termFontSet->minRunHeight <= 0) {
	/* we don't know the fonts... */
	return(-1);
    }
    if (termFontSet->maxRunHeight < cellHeight) {
	return(1);
    }
    if (termFontSet->minRunHeight >= cellHeight) {
	return(0);
    }
    return(-1);
}

/* With fonts of different heights in the set (the usual case with a
 * UTF-8 locale's font set: the JIS X 0208 font is a pixel shorter),
 * whether a run needs the fill depends on the fonts it uses.  ASCII text
 * is all drawn with one font, whose height we measured: so we only need
 * to measure the runs that are not all ASCII...
 */
static int
runNeedsFill(TermFontSet termFontSet, int cellHeight, int mbCurMax,
	unsigned char *string, int len)
{
    int i;

    if (termFontSet->asciiRunHeight <= 0) {
	return(-1);
    }
    if (mbCurMax == 1) {
	for (i = 0; i < len; i++) {
	    if (string[i] & 0x80) {
		return(-1);
	    }
	}
    } else {
	wchar_t *wcs = (wchar_t *) string;

	for (i = 0; i < len; i++) {
	    if ((wcs[i] < 0) || (wcs[i] >= 0x80)) {
		return(-1);
	    }
	}
    }
    return(termFontSet->asciiRunHeight < cellHeight);
}

static void
FontSetRenderFunction(
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
    TermFontSet termFontSet = (TermFontSet) font->fontInfo;
    int escapement = 0;
    XRectangle extents;
    Boolean fixExtents;
    int needFill;

    /* set the renderGC... */
    valueMask = (unsigned long) 0;

    /* set background... */
    if (tpd->renderGC.background != bg) {
	tpd->renderGC.background = bg;
	values.background = bg;
	valueMask |= GCBackground;
    }

    /* since Xlib will be mucking with the GC's font under us, we need to
     * make sure we trash the cached value...
     */
    tpd->renderGC.fid = (Font) 0;

    /* Text height may be smaller than cellHeight (happens when using font sets).
       In this case we need to fill background manually and then use X*DrawString
       instead of X*DrawImageString */
    needFill = runsNeedFill(termFontSet, tpd->cellHeight);
    if (needFill < 0) {
	needFill = runNeedsFill(termFontSet, tpd->cellHeight, tpd->mbCurMax,
		string, len);
    }
    if ((needFill < 0) || isDebugFSet('t', 1)) {
	/* measure this run... */
	escapement = (tpd->mbCurMax == 1) ?
	    XmbTextExtents(termFontSet->fontSet, (char *) string, len,
			   NULL, &extents) :
	    XwcTextExtents(termFontSet->fontSet, (wchar_t*) string, len,
			   NULL, &extents);
	fixExtents = extents.height < tpd->cellHeight;
    } else {
	fixExtents = (needFill > 0);
	if (fixExtents || TermIS_UNDERLINE(flags)) {
	    /* we need the width... */
	    escapement = (tpd->mbCurMax == 1) ?
		XmbTextEscapement(termFontSet->fontSet, (char *) string,
			len) :
		XwcTextEscapement(termFontSet->fontSet, (wchar_t *) string,
			len);
	}
    }

    if (fixExtents) {
      /* set background color as foreground if needed*/
      if (tpd->renderGC.foreground != bg) {
	tpd->renderGC.foreground = bg;
	valueMask|= GCForeground ;
	values.foreground = bg;
      }
      if (valueMask) {
	(void) XChangeGC(XtDisplay(w), tpd->renderGC.gc, valueMask, &values);
	valueMask= (unsigned long) 0;
      }
      /* paint background manually */
      (void) XFillRectangle(XtDisplay(w),
			    XtWindow(w),
			    tpd->renderGC.gc,
			    x,
			    y,
			    escapement,
			    tpd->cellHeight);
    }

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
    if (valueMask) {
	(void) XChangeGC(XtDisplay(w), tpd->renderGC.gc, valueMask,
		&values);
    }

    /* draw image string a line of text... */
    if (isDebugFSet('t', 1)) {
        /* we need to clear background after the debug draw and delay,
           so this will cause  X*DrawImageString to be always used
           in debug mode */
        fixExtents = 0;
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
		escapement,
		extents.height);
	  (void) XSync(XtDisplay(w), False);
	  (void) shortSleep(100000);
    }


    if (tpd->mbCurMax == 1)
    {
        /* select right function, we do not want text background be drawn twice */
        (void) (fixExtents ? XmbDrawString: XmbDrawImageString)
                 (XtDisplay(w),                 /* Display		*/
		 XtWindow(w),			/* Drawable		*/
		 termFontSet->fontSet,		/* XFontSet		*/
		 tpd->renderGC.gc,		/* GC			*/
		 x,				/* x			*/
		 y + termFontSet->ascent,	/* y			*/
                 (char *)string,       		/* string		*/
		 len);				/* length		*/

        /* handle overstrike... */
        if (TermIS_OVERSTRIKE(flags)) {
	    (void) XmbDrawString(XtDisplay(w),	/* Display		*/
		     XtWindow(w),		/* Drawable		*/
		     termFontSet->fontSet,	/* XFontSet		*/
		     tpd->renderGC.gc,		/* GC			*/
		     x + 1,			/* x			*/
		     y + termFontSet->ascent,	/* y			*/
		     (char *)string,           	/* string		*/
		     len);			/* length		*/
	 }
    }
    else
    {
        /* select right function, we do not want text background be drawn twice */
        (void)  (fixExtents ? XwcDrawString: XwcDrawImageString)
	        (XtDisplay(w),	                /* Displa*/
	        XtWindow(w),			/* Drawable		*/
	        termFontSet->fontSet,		/* XFontSet		*/
                tpd->renderGC.gc,		/* GC			*/
	        x,				/* x			*/
	        y + termFontSet->ascent,	/* y			*/
	        (wchar_t *) string,		/* string		*/
	        len);				/* length		*/

        /* handle overstrike... */
        if (TermIS_OVERSTRIKE(flags)) {
	    (void) XwcDrawString(XtDisplay(w),	/* Display		*/
		    XtWindow(w),		/* Drawable		*/
		    termFontSet->fontSet,	/* XFontSet		*/
		    tpd->renderGC.gc,		/* GC			*/
		    x + 1,			/* x			*/
		    y + termFontSet->ascent,	/* y			*/
		    (wchar_t *) string,		/* string		*/
		    len);			/* length		*/
        }
    }
    /* handle the underline enhancement... */
    /* draw the underline... */
    if (TermIS_UNDERLINE(flags)) {
        XDrawLine(XtDisplay(w),             /* Display          */
                  XtWindow(w),              /* Window           */
                  tpd->renderGC.gc,         /* GC               */
                  x,                        /* X1               */
                  y + tpd->cellHeight - 1,  /* Y1               */
                  x - 1 + escapement,       /* X2               */
                  y + tpd->cellHeight - 1); /* Y2               */
    }
}

static void
FontSetDestroyFunction(
    Widget		  w,
    TermFont		  font
)
{
    (void) XtFree((char *) font->fontInfo);
    (void) XtFree((char *) font);
}

static void
FontSetExtentsFunction(
    Widget		  w,
    TermFont		  font,
    unsigned char	 *string,
    int			  len,
    int			 *widthReturn,
    int			 *heightReturn,
    int			 *ascentReturn
)
{
    TermFontSet termFontSet = (TermFontSet) font->fontInfo;

    if (widthReturn) {
	*widthReturn = len * termFontSet->width;
    }
    if (heightReturn) {
	*heightReturn = termFontSet->height;
    }
    if (ascentReturn) {
	*ascentReturn = termFontSet->ascent;
    }
    return;
}

TermFont
_DtTermPrimRenderFontSetCreate(
    Widget		  w,
    XFontSet		  fontSet
)
{
    TermFont termFont;
    TermFontSet termFontSet;
    XFontSetExtents *fontSetExtents;

    termFont = (TermFont) XtMalloc(sizeof(TermFontRec));
    termFont->renderFunction = FontSetRenderFunction;
    termFont->destroyFunction = FontSetDestroyFunction;
    termFont->extentsFunction = FontSetExtentsFunction;

    termFontSet = (TermFontSet) XtMalloc(sizeof(TermFontSetRec));
    termFontSet->fontSet = fontSet;
    fontSetExtents = XExtentsOfFontSet(fontSet);
    termFontSet->width = fontSetExtents->max_logical_extent.width;
    termFontSet->height = fontSetExtents->max_logical_extent.height;
    termFontSet->ascent = -fontSetExtents->max_logical_extent.y;
    termFont->fontInfo = (XtPointer) termFontSet;

    /* the range of run heights (see runsNeedFill())... */
    {
	XFontStruct **fonts;
	char **fontNames;
	int numFonts;
	int maxAscent = 0;
	int maxDescent = 0;
	int i;

	termFontSet->minRunHeight = 0;
	numFonts = XFontsOfFontSet(fontSet, &fonts, &fontNames);
	for (i = 0; i < numFonts; i++) {
	    if (!fonts[i]) {
		/* not loaded (yet).  We will measure every run... */
		termFontSet->minRunHeight = 0;
		break;
	    }
	    if ((i == 0) || (fonts[i]->ascent + fonts[i]->descent <
		    termFontSet->minRunHeight)) {
		termFontSet->minRunHeight =
			fonts[i]->ascent + fonts[i]->descent;
	    }
	    maxAscent = MAX(maxAscent, fonts[i]->ascent);
	    maxDescent = MAX(maxDescent, fonts[i]->descent);
	}
	termFontSet->maxRunHeight = maxAscent + maxDescent;
    }

    /* the height of a run of ASCII characters (see runNeedsFill())... */
    {
	static char ascii[] = " !\"#$%&'()*+,-./0123456789:;<=>?@"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
		"abcdefghijklmnopqrstuvwxyz{|}~";
	XRectangle extents;

	extents.height = 0;
	(void) XmbTextExtents(fontSet, ascii, sizeof(ascii) - 1, NULL,
		&extents);
	termFontSet->asciiRunHeight = extents.height;
    }

    return(termFont);
}
