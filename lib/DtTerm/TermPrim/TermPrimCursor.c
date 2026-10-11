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
 * (c) Copyright 1993, 1994, 1996 Hewlett-Packard Company               *
 * (c) Copyright 1993, 1994, 1996 International Business Machines Corp. *
 * (c) Copyright 1993, 1994, 1996 Sun Microsystems, Inc.                *
 * (c) Copyright 1993, 1994, 1996 Novell, Inc.                          *
 * (c) Copyright 1996 Digital Equipment Corporation.			*
 * (c) Copyright 1996 FUJITSU LIMITED.					*
 * (c) Copyright 1996 Hitachi.						*
 */

#include <time.h>
#include <Xm/ScrollBar.h>
#include <Xm/XmPrivate.h>

#include "TermHeader.h"
#include "TermPrimDebug.h"
#include "TermPrimI.h"
#include "TermPrimP.h"
#include "TermPrimData.h"
#include "TermPrimLineDraw.h"
#include "TermPrimBufferP.h"

static void cursorToggle(Widget w);
static void timeoutCallback(XtPointer client_data, XtIntervalId *id);

/* stop blinking (leaving the cursor drawn) after this long without
 * output, keyboard input or a focus change, so that an idle terminal
 * does not wake up blinkRate times a second forever...
 */
#define	BLINK_IDLE_STOP_MS	30000

/* set the input method spot location at most this often.  With an
 * XIM server (ibus, fcitx, ...) every update is a synchronous round
 * trip.  The first move after a quiet period is sent at once (so the
 * spot follows typing without delay); moves during a burst of output
 * are coalesced into one update at the end of the interval...
 */
#define	IM_SPOT_INTERVAL_MS	50

/* milliseconds on the monotonic clock.  The value wraps around (it is
 * unsigned, so that this is well defined even where a long is 32 bits),
 * so compare two of them by their unsigned difference...
 */
unsigned long
_DtTermPrimMonotonicMs(void)
{
    struct timespec now;

    (void) clock_gettime(CLOCK_MONOTONIC, &now);
    return((unsigned long) now.tv_sec * 1000UL +
	    (unsigned long) (now.tv_nsec / 1000000L));
}

/*
 * The blink timer.  It is armed (cursorTimeoutId != 0) only while we
 * have the focus and blinkRate > 0.  _DtTermPrimCursorOff() leaves it
 * armed, and _DtTermPrimCursorOn() arms it only if it is not, so that
 * output (which turns the cursor off and on again for every burst) does
 * not remove and re-add a timer each time.  The cursor stays drawn for
 * a full blinkRate after it is turned on, as before: a tick that comes
 * sooner than that just waits out the rest of the period...
 */
static void
addBlinkTimeout(Widget w, long ms)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;

    tw->term.tpd->cursorTimeoutId =
	    XtAppAddTimeOut(XtWidgetToApplicationContext(w),
	    (unsigned long) MAX(1, ms),
	    (XtTimerCallbackProc) timeoutCallback, (XtPointer) w);
}

/*ARGSUSED*/
static void
timeoutCallback(XtPointer client_data, XtIntervalId *id)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) client_data;
    struct termData *tpd = tw->term.tpd;
    unsigned long now;

    tpd->cursorTimeoutId = (XtIntervalId) 0;

    /* the cursor is off (output is being processed, or it has been made
     * invisible).  _DtTermPrimCursorOn() will rearm us...
     */
    if ((CURSORoff == tpd->cursorState) || !tpd->cursorVisible ||
	    !tw->term.hasFocus) {
	return;
    }

    now = _DtTermPrimMonotonicMs();
    if ((tw->term.blinkRate <= 0) ||
	    (now - tpd->cursorActiveMs >= BLINK_IDLE_STOP_MS)) {
	/* stop blinking.  Leave the cursor drawn... */
	if (CURSORon == tpd->cursorState) {
	    (void) cursorToggle((Widget) tw);
	}
	return;
    }

    if (now - tpd->cursorOnMs < (unsigned long) tw->term.blinkRate) {
	/* the cursor was turned on less than a period ago... */
	(void) addBlinkTimeout((Widget) tw,
		tw->term.blinkRate - (long) (now - tpd->cursorOnMs));
	return;
    }

    (void) cursorToggle((Widget) tw);
    (void) addBlinkTimeout((Widget) tw, tw->term.blinkRate);
}

/* (re)start blinking if we should be and are not (it stops after
 * BLINK_IDLE_STOP_MS of inactivity)...
 */
static void
restartBlinking(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;

    if (tw->term.hasFocus && (tw->term.blinkRate > 0) &&
	    tpd->cursorVisible && (CURSORoff != tpd->cursorState) &&
	    !tpd->cursorTimeoutId) {
	(void) addBlinkTimeout(w, tw->term.blinkRate);
    }
}

/* a key was pressed: we are not idle...
 */
void
_DtTermPrimCursorKeyActivity(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;

    tpd->cursorActiveMs = _DtTermPrimMonotonicMs();
    if (!tpd->cursorTimeoutId) {
	/* restart blinking from the drawn state... */
	tpd->cursorOnMs = tpd->cursorActiveMs;
	(void) restartBlinking(w);
    }
}

/* set the input method spot location to the cursor position...
 */
static void
setIMSpot(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    XPoint point;

    tpd->IMCursorColumn = tpd->cursorColumn;
    tpd->IMCursorRow = tpd->cursorRow;
    tpd->imSpotSentMs = _DtTermPrimMonotonicMs();
    point.x = tpd->cursorColumn * tpd->cellWidth + tpd->offsetX;
    point.y = tpd->cursorRow * tpd->cellHeight + tpd->offsetY + tpd->ascent;
    DebugF('F', 1, fprintf(stderr,
	    "%s() %s calling %s\n",
	    "_DtTermPrimCursorOn",
	    "don't care",
	    "XmImVaSetValues()"));
    (void) XmImVaSetValues(w,
	    XmNspotLocation, &point,
	    NULL);
}

/*ARGSUSED*/
static void
imSpotTimeout(XtPointer client_data, XtIntervalId *id)
{
    Widget w = (Widget) client_data;
    struct termData *tpd = ((DtTermPrimitiveWidget) w)->term.tpd;

    tpd->imSpotTimerId = (XtIntervalId) 0;
    if ((tpd->IMCursorColumn != tpd->cursorColumn) ||
	    (tpd->IMCursorRow != tpd->cursorRow)) {
	(void) setIMSpot(w);
    }
}

static void
updateIMSpot(Widget w)
{
    struct termData *tpd = ((DtTermPrimitiveWidget) w)->term.tpd;
    unsigned long elapsed;

    if (((tpd->IMCursorColumn == tpd->cursorColumn) &&
	    (tpd->IMCursorRow == tpd->cursorRow)) || tpd->imSpotTimerId) {
	/* nothing to do, or already scheduled... */
	return;
    }

    elapsed = _DtTermPrimMonotonicMs() - tpd->imSpotSentMs;
    if (elapsed >= IM_SPOT_INTERVAL_MS) {
	(void) setIMSpot(w);
    } else {
	tpd->imSpotTimerId = XtAppAddTimeOut(XtWidgetToApplicationContext(w),
		IM_SPOT_INTERVAL_MS - elapsed,
		imSpotTimeout, (XtPointer) w);
    }
}

/* remove our timers (the widget is being destroyed)...
 */
void
_DtTermPrimCursorDestroy(Widget w)
{
    struct termData *tpd = ((DtTermPrimitiveWidget) w)->term.tpd;

    if (tpd->cursorTimeoutId) {
	(void) XtRemoveTimeOut(tpd->cursorTimeoutId);
	tpd->cursorTimeoutId = (XtIntervalId) 0;
    }
    if (tpd->imSpotTimerId) {
	(void) XtRemoveTimeOut(tpd->imSpotTimerId);
	tpd->imSpotTimerId = (XtIntervalId) 0;
    }
}

void
_DtTermPrimCursorChangeFocus(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    XPoint point;

    if (tw->term.hasFocus) {
	/* if the input method already didn't have focus or the cursor
	 * position has changed, then set the input method focus and
	 * spot location...
	 */
	if ((!tpd->IMHasFocus) ||
		(tpd->IMCursorColumn != tpd->cursorColumn) ||
		(tpd->IMCursorRow != tpd->cursorRow)) {
	    tpd->IMHasFocus = True;
	    tpd->IMCursorColumn = tpd->cursorColumn;
	    tpd->IMCursorRow = tpd->cursorRow;
	    tpd->imSpotSentMs = _DtTermPrimMonotonicMs();

	    point.x = tpd->IMCursorColumn * tpd->cellWidth + tpd->offsetX;
	    point.y = tpd->IMCursorRow * tpd->cellHeight + tpd->offsetY +
		    tpd->ascent;
	    DebugF('F', 1, fprintf(stderr,
		    "%s() %s calling %s\n",
		    "_DtTermPrimCursorChangeFocus",
		    "hasFocus",
		    "XmImVaSetFocusValues()"));
	    (void) XmImVaSetFocusValues(w,
		    XmNspotLocation, &point,
		    NULL);
	}

	/* we want to blink now... */
	tpd->cursorActiveMs = _DtTermPrimMonotonicMs();
	if (tpd->cursorVisible && (!tpd->cursorTimeoutId) &&
		(tw->term.blinkRate > 0) &&
		(tpd->cursorState != CURSORoff)) {
	    Debug('F', fprintf(stderr,
		    ">>we got focus, turning off cursor...\n"));
	    (void) cursorToggle(w);

	    /* add a timeout... */
	    Debug('F', fprintf(stderr, ">>adding a timeout...\n"));
	    tpd->cursorOnMs = 0;
	    (void) addBlinkTimeout(w, tw->term.blinkRate);
	}
    } else {
	if (tpd->IMHasFocus) {
	    tpd->IMHasFocus = False;
	    DebugF('F', 1, fprintf(stderr,
		    "%s() %s calling %s\n",
		    "_DtTermPrimCursorChangeFocus",
		    "!hasFocus",
		    "XmImUnsetFocus()"));
	    /* remove input method focus... */
	    (void) XmImUnsetFocus(w);
	}

	/* we want to stop blinking now (the cursor may be off with the
	 * timer still armed, or drawn with blinking stopped by
	 * timeoutCallback())...
	 */
	if (CURSORon == tpd->cursorState) {
	    /* we need to make the cursor visible... */
	    Debug('F', fprintf(stderr,
		    ">>turning on the cursor...\n"));
	    (void) cursorToggle(w);
	}
	if (tpd->cursorTimeoutId) {
	    /* we need to kill the timeout... */
	    Debug('F', fprintf(stderr, ">>removing the timeout...\n"));
	    (void) XtRemoveTimeOut(tpd->cursorTimeoutId);
	    tpd->cursorTimeoutId = (XtIntervalId) 0;
	} else {
	    if (tw->term.blinkRate > 0) {
		Debug('F', fprintf(stderr,
			">>we lost focus, but cursor is not on and blinking...\n"));
	    }
	}
    }
}

static void
cursorToggle(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    int y;
    int height;
    int cursorRow;

    /* if the cursor is not visible or is off... */
    if (!tpd->cursorVisible || (CURSORoff == tpd->cursorState)) {
	/* do nothing... */
	return;
    }

    cursorRow = tpd->cursorRow;
    if (tpd->useHistoryBuffer) {
	/* since topRow is used for history scrolling, move down by the
	 * (negative) number of rows in topRow...
	 */
	cursorRow -= tpd->topRow;
    }

    if (cursorRow < tw->term.rows) {
	if (DtTERM_CHAR_CURSOR_BOX == tw->term.charCursorStyle) {
	    /* draw a box... */
	    y = cursorRow * tpd->cellHeight + tpd->offsetY;
	    height = tpd->cellHeight;
	} else {
	    /* draw a line... */
	    y = cursorRow * tpd->cellHeight + tpd->offsetY +
		    tpd->ascent + 1;
	    height = 2;
	}

	(void) XFillRectangle(XtDisplay(w),
					/* Display			*/
		XtWindow(w),		/* Window			*/
		tpd->cursorGC.gc,	/* GC				*/
		((tpd->cursorColumn >= tw->term.columns) ?
		tw->term.columns - 1 :
		tpd->cursorColumn) * tpd->cellWidth + tpd->offsetX,
					/* x				*/
		y,			/* y				*/
		tpd->cellWidth,		/* width			*/
		height);		/* height			*/
    }

    /* toggle the state flag... */
    tpd->cursorState = (tpd->cursorState == CURSORon) ? CURSORblink : CURSORon;

    return;
}

void
_DtTermPrimCursorOn(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    static Boolean alreadyActive = False;
    short chunkWidth;
    enhValues enhancements;
    unsigned long valueMask = 0L;
    XGCValues values;
    TermEnhInfoRec enhInfo;

    /* if we are being called cyclically (by _DtTermPrimScrollWait ->
     * _DtTermPrimExposeText -> _DtTermPrimCursorOn), just return...
     */
    _DtTermProcessLock();
    if (alreadyActive) {
	/*DKS!!! vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
	fprintf(stderr, "tell Dave _DtTermPrimCursorOn has alreadyActive == True\n");
	/*DKS!!! ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
	_DtTermProcessUnlock();
	return;
    }

    /* this is where we will actually perform a pending scroll and
     * text refresh...
     */
    if (tw->term.jumpScroll && tpd->scroll.jump.scrolled) {
	/* make sure we don't end up in an infinite loop... */
	alreadyActive = True;
	Debug('t', fprintf(stderr,
		">>_DtTermPrimCursorOn() calling _DtTermPrimScrollWait()\n"));
	(void) _DtTermPrimScrollWait(w);
	alreadyActive = False;
    }
    _DtTermProcessUnlock();

    /* paint any text we have been holding back (before we draw the
     * cursor over it)...
     */
    (void) _DtTermPrimRenderFlushDirty(w);

    /* the frame we may have been waiting to paint is painted now... */
    if (tpd->frameTimerId) {
	(void) XtRemoveTimeOut(tpd->frameTimerId);
	tpd->frameTimerId = (XtIntervalId) 0;
    }

#ifdef	DISOWN_SELECTION_ON_CURSOR_ON_OR_OFF 
    if ( _DtTermPrimSelectIsAboveSelection(w,tpd->cursorRow,
	    tpd->cursorColumn)) {
        _DtTermPrimSelectDisown(w) ;
    }
#endif	/* DISOWN_SELECTION_ON_CURSOR_ON_OR_OFF */

    /* update the input method spot location (at most every
     * IM_SPOT_INTERVAL_MS)...
     */
    (void) updateIMSpot(w);
#ifdef	NOT_NEEDED
    if (!tw->term.hasFocus) {
	(void) fprintf(stderr,
		"%s() %s calling %s\n",
		"_DtTermPrimCursorOn",
		"!hasFocus",
		"XmImUnsetFocus()");
	(void) XmImUnsetFocus(w);
    }
#endif	/* NOT_NEEDED */

    /* update the scrollbar and position indicator... */
    (void) _DtTermPrimCursorUpdate(w);

    /* if the cursor is not visible, we are done now... */
    if (!tpd->cursorVisible) {
	return;
    }

    /* set up the GC... */
    if (!tpd->cursorGC.gc) {
	tpd->cursorGC.foreground =
		tw->primitive.foreground ^ tw->core.background_pixel;
	values.foreground = tpd->cursorGC.foreground;
	values.function = GXxor;
	tpd->cursorGC.gc = XCreateGC(XtDisplay(w), XtWindow(w),
		GCForeground | GCFunction, &values);
    }

    /* update the cursor's foreground and background...
     */
    /* if we are past the lastUsedRow, or the column > width, use color
     * pair 0...
     */

    /* reasonable defaults... */
    enhInfo.fg = tw->primitive.foreground;
    enhInfo.bg = tw->core.background_pixel;
    if (!((tpd->lastUsedRow <= tpd->topRow + tpd->cursorRow) ||
	    (_DtTermPrimBufferGetLineWidth(tpd->termBuffer,
		    tpd->topRow + tpd->cursorRow) <= MIN(tpd->cursorColumn,
		    tw->term.columns - 1)))) {
	/* get the current enhancement to determine the color pair to use...
	 */
	(void) _DtTermPrimBufferGetEnhancement(tpd->termBuffer,
						/* TermBuffer		*/
		    tpd->topRow + tpd->cursorRow,
						/* row			*/
		    MIN(tpd->cursorColumn, tw->term.columns - 1),
						/* col			*/
		    &enhancements,		/* enhancements		*/
		    &chunkWidth,		/* width		*/
		    countNew);			/* countWhich		*/
	/* set our font and color from the enhancements... */
	if (ENH_PROC(tpd->termBuffer)) {
	    (void) (*(ENH_PROC(tpd->termBuffer)))(w, enhancements, &enhInfo);
	}
    }

    /* set the GC... */
    if (tpd->cursorGC.foreground != (enhInfo.fg ^ enhInfo.bg)) {
	tpd->cursorGC.foreground = enhInfo.fg ^ enhInfo.bg;
	values.foreground = enhInfo.fg ^ enhInfo.bg;
	valueMask |= GCForeground;
    }
    if (valueMask) {
	(void) XChangeGC(XtDisplay(w), tpd->cursorGC.gc, valueMask, &values);
    }

    if (tpd->cursorState != CURSORoff) {
	return;
    }

    tpd->cursorState = CURSORon;
    (void) cursorToggle(w);

    /* the cursor stays drawn for a full period from now.  Arm the blink
     * timer if it is not already...
     */
    tpd->cursorOnMs = _DtTermPrimMonotonicMs();
    tpd->cursorActiveMs = tpd->cursorOnMs;
    (void) restartBlinking(w);

}

void
_DtTermPrimCursorUpdate(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    Arg arglist[10];
    int i;
    short value;
    Boolean newMaximum = False;
    Boolean newValue = False;

    /* update the scrollbar... */
    if (tw->term.verticalScrollBar) {
	i = 0;
	if (tpd->useHistoryBuffer) {
#define	NO_SCROLL_REGION_HISTORY_SCROLL
#ifdef	NO_SCROLL_REGION_HISTORY_SCROLL 
	    value = tw->term.rows;
	    if ((tpd->scrollLockTopRow <= 0) &&
		    (tpd->scrollLockBottomRow >= (tw->term.rows - 1))) {
		value += tpd->lastUsedHistoryRow;
	    }
#else	/* NO_SCROLL_REGION_HISTORY_SCROLL */
	    value = tw->term.rows + tpd->lastUsedHistoryRow;
#endif	/* NO_SCROLL_REGION_HISTORY_SCROLL */
	} else {
	    value = tpd->lastUsedRow +
		    (tpd->useHistoryBuffer ? tpd->lastUsedHistoryRow : 0) -
		    tpd->scrollLockTopRow -
		    (tw->term.rows - 1 - tpd->scrollLockBottomRow);

	    /* add in any non-exisstent rows below the last used row...
	     */
	    if (tpd->allowScrollBelowBuffer) {
		/* add in a full screen (less one line and protected areas)
		 * below the last used row...
		 */
		value += tw->term.rows - 1 -
			tpd->scrollLockTopRow -
			(tw->term.rows - 1 - tpd->scrollLockBottomRow);
	    }
	}
		    
	if (tw->term.verticalScrollBarMaximum != value) {
	    /* need to set maximum ... */
	    tw->term.verticalScrollBarMaximum = value;
	    newMaximum = True;
	}

	if (tpd->useHistoryBuffer) {
	    value = tw->term.rows;
	} else {
	    value = tw->term.rows -
		    tpd->scrollLockTopRow -
		    (tw->term.rows - 1 - tpd->scrollLockBottomRow);
	}
	if (tw->term.verticalScrollBarSliderSize != value) {
	    /* need to set sliderSize ... */
	    tw->term.verticalScrollBarSliderSize = value;
	    (void) XtSetArg(arglist[i], XmNsliderSize,
		    tw->term.verticalScrollBarSliderSize); i++;
	    newValue = True;
	}

	/* verticalScrollBarPageIncrement = verticalScrollBarSliderSize... */
	if (tw->term.verticalScrollBarPageIncrement != value) {
	    /* need to set pageIncrement ... */
	    tw->term.verticalScrollBarPageIncrement = value;
	    (void) XtSetArg(arglist[i], XmNpageIncrement,
		    tw->term.verticalScrollBarPageIncrement); i++;
	    newValue = True;
	}

#ifdef	NO_SCROLL_REGION_HISTORY_SCROLL 
	value = tpd->topRow;
	if (tpd->useHistoryBuffer && (tpd->scrollLockTopRow <= 0) &&
		    (tpd->scrollLockBottomRow >= (tw->term.rows - 1))) {
	    value += tpd->lastUsedHistoryRow;
	}
#else	/* NO_SCROLL_REGION_HISTORY_SCROLL */
	value = tpd->topRow + tpd->lastUsedHistoryRow;
#endif	/* NO_SCROLL_REGION_HISTORY_SCROLL */

	if (tw->term.verticalScrollBarValue != value) {
	    /* need to set value... */
	    tw->term.verticalScrollBarValue = value;
	    (void) XtSetArg(arglist[i], XmNvalue,
		    tw->term.verticalScrollBarValue); i++;
	    newValue = True;
	}

	/* check and see if value > max - size.  If it is, adjust
	 * maximum...
	 */
	if (tw->term.verticalScrollBarValue >
		tw->term.verticalScrollBarMaximum -
		tw->term.verticalScrollBarSliderSize) {
	    tw->term.verticalScrollBarMaximum =
		    tw->term.verticalScrollBarValue +
		    tw->term.verticalScrollBarSliderSize;
	    newMaximum = True;
	}

	Debug('b', if (newMaximum || newValue) {
		fprintf(stderr, ">>_DtTermPrimCursorUpdate: sb size=%d  min=%d  max=%d  value=%d  pginc=%d\n", 
		tw->term.verticalScrollBarSliderSize,
		0,
		tw->term.verticalScrollBarMaximum,
		tw->term.verticalScrollBarValue,
		tw->term.verticalScrollBarPageIncrement);
	});

	if (newMaximum) {
	    (void) XtSetArg(arglist[i], XmNmaximum,
		    tw->term.verticalScrollBarMaximum); i++;
	    (void) XtSetValues(tw->term.verticalScrollBar, arglist, i);
	} else if (newValue) {
	    /* only need to worry about setting value, slidersize, and
	     * page increment...
	     */
	    (void) XmScrollBarSetValues(tw->term.verticalScrollBar,
		tw->term.verticalScrollBarValue,
					/* value			*/
		tw->term.verticalScrollBarSliderSize,
					/* slider_size			*/
		1,			/* increment			*/
		tw->term.verticalScrollBarPageIncrement,
					/* page_increment		*/
		False);			/* notify			*/
	}
    }

    /* invoke any cursor motion callbacks... */
    if (tw->term.statusChangeCallback) {
	_DtTermPrimInvokeStatusChangeCallback(w);
    }
}

void
_DtTermPrimCursorOff(Widget w)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;

    if (CURSORoff == tpd->cursorState) {
	return;
    }

    if (CURSORblink == tpd->cursorState) {
	/* we need to restore the text... */
	(void) cursorToggle(w);
    }

    /* reset the state flag.  The blink timer stays armed (see
     * timeoutCallback())...
     */
    tpd->cursorState = CURSORoff;

#ifdef	DISOWN_SELECTION_ON_CURSOR_ON_OR_OFF 
    if ( _DtTermPrimSelectIsAboveSelection(w,tpd->cursorRow,
                            tpd->cursorColumn)) {
       _DtTermPrimSelectDisown(w) ;
     }
#endif	/* DISOWN_SELECTION_ON_CURSOR_ON_OR_OFF */
}

/* 
** _DtTermPrimCursorMove
** 
** Move the cursor to the specified location, scrolling the screen
** as necessary.
** 
** NOTES:
**    If the new cursor position is exceeds the line width, the line
**    is padded with spaces up to (but not including) the new cursor
**    position (just adjust the line width since the line is already
**    filled with spaces).
**    if (cursorColumn == width)
**        then the cursor is at the end of the line
**      
**    if (cursorColumn > width)  
**        then the cursor is past the end of the line,
**        pad the line by setting width = cursorColumn
*/
void
_DtTermPrimCursorMove
(
    Widget  w,
    int     row,
    int     col
)
{
    DtTermPrimitiveWidget          tw = (DtTermPrimitiveWidget) w;
    struct termData    *tpd = tw->term.tpd;
    
    /* 
    ** Constrain col to buffer width.
    */
    if (col < 0)
    {
        col = 0;
    }
    else if (col >= COLS(tpd->termBuffer))
    {
        col = COLS(tpd->termBuffer) - 1;
    }
    tpd->cursorColumn = col;

    /* 
    ** Constraint row to >= 0.  Height is open ended...
    */
    if (row < 0)
    {
        row = 0;
    } 


    /* 
    ** Scroll the window as necessary.
    */
    if (row < tpd->topRow) {
	/* scroll so that row is the topRow... */
        _DtTermPrimScrollTextTo(w, row);
	tpd->cursorRow = 0;
    } else if (row >= tpd->topRow + tw->term.rows) {
	/* scroll so that row is the bottomRow... */
        _DtTermPrimScrollTextTo(w, row - tw->term.rows + 1);
	tpd->cursorRow = tw->term.rows - 1;
    } else {
	/* we just need to move the cursor... */
	tpd->cursorRow = row - tpd->topRow;
    }

#ifdef    NOCODE
    /* 
    ** Make sure the line is padded properly.
    **
    ** NOTE: we no longer do this, it might be necessary for hpterm3.0
    **       but I will double check then
    */
    _DtTermPrimRenderPadLine(w);
#endif /* NOCODE */
}

void
_DtTermPrimSetCursorVisible
(
    Widget		  w,
    Boolean		  visible
)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData	 *tpd = tw->term.tpd;

    /* make sure the cursor is off... */
    _DtTermPrimCursorOff(w);
    if (visible) {
	/* turn it on...
	 */
	tpd->cursorVisible = True;
    } else {
	/* turn it off...
	 */
	tpd->cursorVisible = False;
    }
}

Boolean
_DtTermPrimGetCursorVisible
(
    Widget		  w
)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData	 *tpd = tw->term.tpd;

    return(tpd->cursorVisible);
}



