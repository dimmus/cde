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
/* $XConsortium: GetMwmW.c /main/5 1996/05/20 16:07:08 drk $
 *
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1990,1993,1994,1996 Hewlett-Packard Company.
 * (c) Copyright 1993,1994,1996 International Business Machines Corp.
 * (c) Copyright 1993,1994,1996 Sun Microsystems, Inc.
 * (c) Copyright 1993,1994,1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */

/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     GetMwmW.c
 **
 **   Project:  DT Workspace Manager
 **
 **   Description: Gets the mwm window id.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlibint.h>	/* XESetCloseDisplay; defines XTHREADS */
#include <X11/IntrinsicP.h>	/* XtProcessLock, used when XTHREADS is set */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Dt/WsmP.h>
#include "DtSvcLock.h"



/********    Public Function Declarations    ********/

extern int _DtGetMwmWindow( 
                        Display *display,
                        Window root,
                        Window *pMwmWindow) ;

/********    End Public Function Declarations    ********/

/********    Static Function Declarations    ********/

static int _GetMwmWindow( 
                        Display *display,
                        Window root,
                        Window *pMwmWindow,
                        Atom property) ;

/********    End Static Function Declarations    ********/


/*************************************<->*************************************
 *
 *  int _GetMwmWindow (display, root, pMwmWindow, property)
 *
 *
 *  Description:
 *  -----------
 *  Get the Motif Window manager window
 *
 *
 *  Inputs:
 *  ------
 *  display		- display 
 *  root		- root window of screen
 *  pMwmWindow		- pointer to a window (to be returned)
 *  property		- the property atom
 *
 *  Outputs:
 *  --------
 *  *pMwmWindow		- mwm window id, if successful
 *  Return		- status from XGetWindowProperty
 *
 *  Comments:
 *  --------
 *  This can fail if mwm is not managing the screen for the root window
 *  passed in.
 * 
 *************************************<->***********************************/
static int 
_GetMwmWindow(
        Display *display,
        Window root,
        Window *pMwmWindow,
        Atom property )
{
    Atom actualType;
    int actualFormat;
    unsigned long nitems;
    unsigned long leftover;
    PropMotifWmInfo *pWmInfo = NULL;
    int rcode;
    Window wroot, wparent, *pchildren;
    unsigned int nchildren;

    *pMwmWindow = 0;
    if ((rcode=XGetWindowProperty(display,root,
			     property,0L, PROP_MWM_INFO_ELEMENTS,
			     False,property,
			     &actualType,&actualFormat,
			     &nitems,&leftover,(unsigned char **)&pWmInfo))==Success)
    {

        if (actualType != property)
	{
	    /* wrong type, force failure */
	    rcode = BadValue;
	}
	else
	{
	    rcode = BadWindow;	/* assume the worst */

	    /*
	     * The mwm window should be a direct child of root
	     */
	    if (XQueryTree (display, root, &wroot, &wparent,
			    &pchildren, &nchildren))
	    {
		int i;

		for (i = 0; (i < nchildren) && (rcode != Success); i++)
		{
		    if (pchildren[i] == pWmInfo->wmWindow)
		    {
			rcode = Success;
		    }
		}
	    }

	    if (rcode == Success)
	    {
		*pMwmWindow = pWmInfo->wmWindow;
	    }

	    if (pchildren)
	    {
		XFree ((char *)pchildren);
	    }

	}

	if (pWmInfo)
	{
	    XFree ((char *)pWmInfo);
	}
    }
	
    return(rcode);

} /* END OF FUNCTION _GetMwmWindow */


/*************************************<->*************************************
 *
 *  int _DtGetMwmWindow (display, root, pMwmWindow)
 *
 *
 *  Description:
 *  -----------
 *  Get the Motif Window manager window
 *
 *
 *  Inputs:
 *  ------
 *  display		- display 
 *  root		- root window of screen
 *  pMwmWindow		- pointer to a window (to be returned)
 *
 *  Outputs:
 *  --------
 *  *pMwmWindow		- mwm window id, if successful
 *  Return		- status from XGetWindowProperty
 *
 *  Comments:
 *  --------
 *  This can fail if mwm is not managing the screen for the root window
 *  passed in.
 * 
 *************************************<->***********************************/
int 
_DtGetMwmWindow(
        Display *display,
        Window root,
        Window *pMwmWindow )
{
    Atom xa_MWM_INFO;

    xa_MWM_INFO = XmInternAtom (display, _XA_MWM_INFO, False);
    return (_GetMwmWindow (display, root, pMwmWindow, xa_MWM_INFO));
}


/*************************************<->*************************************
 *
 *  Cached window manager window
 *
 *  _DtGetMwmWindow costs two round trips (the _MOTIF_WM_INFO property
 *  on the root, then XQueryTree of the root to check that the window it
 *  names still exists), and every DtWsm* query used to pay them before
 *  its own XGetWindowProperty on that window.
 *
 *  _DtGetMwmWindowProperty remembers the validated window per display
 *  and root, and reads the property straight from it, trapping
 *  BadWindow.  If the window has gone (the window manager exited or was
 *  restarted) or does not carry the property, the cache entry is dropped
 *  and the read is redone the old way, so the results are those of
 *  _DtGetMwmWindow plus XGetWindowProperty, in one round trip instead
 *  of three when the cache is good.
 *
 *  Entries are removed when their display is closed.
 *
 *************************************<->***********************************/

typedef struct _MwmWindowCache {
    Display			*display;
    Window			root;
    Window			wmWindow;
    struct _MwmWindowCache	*next;
} MwmWindowCache;

typedef struct _MwmDisplay {
    Display			*display;
    struct _MwmDisplay		*next;
} MwmDisplay;

static MwmWindowCache	*mwmCache = NULL;
static MwmDisplay	*mwmDisplays = NULL;	/* close hook installed */

static int
MwmCacheCloseDisplay(Display *display, XExtCodes *codes)
{
    MwmWindowCache **pp, *p;
    MwmDisplay **dpp, *dp;

    _DtSvcProcessLock();
    for (pp = &mwmCache; (p = *pp) != NULL; )
    {
	if (p->display == display)
	{
	    *pp = p->next;
	    free(p);
	}
	else
	    pp = &p->next;
    }
    for (dpp = &mwmDisplays; (dp = *dpp) != NULL; dpp = &dp->next)
    {
	if (dp->display == display)
	{
	    *dpp = dp->next;
	    free(dp);
	    break;
	}
    }
    _DtSvcProcessUnlock();
    return 0;
}

static Window
MwmCacheLookup(Display *display, Window root)
{
    MwmWindowCache *p;
    Window w = None;

    _DtSvcProcessLock();
    for (p = mwmCache; p; p = p->next)
    {
	if (p->display == display && p->root == root)
	{
	    w = p->wmWindow;
	    break;
	}
    }
    _DtSvcProcessUnlock();
    return w;
}

static void
MwmCacheStore(Display *display, Window root, Window wmWindow)
{
    MwmWindowCache *p;
    MwmDisplay *dp;

    _DtSvcProcessLock();
    for (dp = mwmDisplays; dp; dp = dp->next)
	if (dp->display == display)
	    break;
    if (dp == NULL)
    {
	XExtCodes *codes;

	/* Without the close hook a reused Display address could see a
	 * stale entry, so cache nothing if it cannot be installed. */
	if ((dp = malloc(sizeof(*dp))) == NULL ||
	    (codes = XAddExtension(display)) == NULL)
	{
	    free(dp);
	    _DtSvcProcessUnlock();
	    return;
	}
	XESetCloseDisplay(display, codes->extension, MwmCacheCloseDisplay);
	dp->display = display;
	dp->next = mwmDisplays;
	mwmDisplays = dp;
    }

    for (p = mwmCache; p; p = p->next)
	if (p->display == display && p->root == root)
	    break;
    if (p == NULL)
    {
	if ((p = malloc(sizeof(*p))) == NULL)
	{
	    _DtSvcProcessUnlock();
	    return;
	}
	p->display = display;
	p->root = root;
	p->next = mwmCache;
	mwmCache = p;
    }
    p->wmWindow = wmWindow;
    _DtSvcProcessUnlock();
}

static void
MwmCacheForget(Display *display, Window root)
{
    MwmWindowCache **pp, *p;

    _DtSvcProcessLock();
    for (pp = &mwmCache; (p = *pp) != NULL; pp = &p->next)
    {
	if (p->display == display && p->root == root)
	{
	    *pp = p->next;
	    free(p);
	    break;
	}
    }
    _DtSvcProcessUnlock();
}

/*
 * BadWindow trap for one request.  Any other error, and errors from
 * other requests that are processed while we wait for our reply, go to
 * the handler that was installed before.
 */
static Display		*trapDisplay;
static unsigned long	trapSerial;
static Bool		trapHit;
static XErrorHandler	trapPrevHandler;

static int
MwmTrapHandler(Display *display, XErrorEvent *event)
{
    if (display == trapDisplay && event->serial == trapSerial &&
	event->error_code == BadWindow)
    {
	trapHit = True;
	return 0;
    }
    return trapPrevHandler ? (*trapPrevHandler)(display, event) : 0;
}

/*
 * XGetWindowProperty that turns a BadWindow error into a BadWindow
 * return instead of calling the application's error handler.
 */
static int
GetPropertyTrapped(
        Display *display,
        Window window,
        Atom property,
        long length,
        Atom reqType,
        Atom *pActualType,
        int *pActualFormat,
        unsigned long *pItems,
        unsigned long *pLeftover,
        unsigned char **pData)
{
    int rcode;

    _DtSvcProcessLock();
    trapDisplay = display;
    trapHit = False;
    trapPrevHandler = XSetErrorHandler(MwmTrapHandler);
    trapSerial = NextRequest(display);
    *pData = NULL;
    rcode = XGetWindowProperty(display, window, property, 0L, length,
			       False, reqType, pActualType, pActualFormat,
			       pItems, pLeftover, pData);
    XSetErrorHandler(trapPrevHandler);
    trapDisplay = NULL;
    if (trapHit)
    {
	rcode = BadWindow;
	*pActualType = None;
	*pData = NULL;
    }
    _DtSvcProcessUnlock();
    return rcode;
}

/*************************************<->*************************************
 *
 *  int _DtGetMwmWindowProperty (display, root, property, length, reqType,
 *		pActualType, pActualFormat, pItems, pLeftover, pData)
 *
 *  Description:
 *  -----------
 *  Same as _DtGetMwmWindow followed, on success, by XGetWindowProperty
 *  (offset 0, no delete) on the window manager window.
 *
 *  Return: the status of _DtGetMwmWindow if that failed, else the status
 *  of XGetWindowProperty (BadWindow if the window went away meanwhile).
 *  *pActualType is None and *pData NULL when nothing was read.
 *
 *************************************<->***********************************/
int
_DtGetMwmWindowProperty(
        Display *display,
        Window root,
        Atom property,
        long length,
        Atom reqType,
        Atom *pActualType,
        int *pActualFormat,
        unsigned long *pItems,
        unsigned long *pLeftover,
        unsigned char **pData)
{
    Window wmWindow;
    int rcode;

    *pActualType = None;
    *pData = NULL;

    wmWindow = MwmCacheLookup(display, root);
    if (wmWindow != None)
    {
	rcode = GetPropertyTrapped(display, wmWindow, property, length,
				   reqType, pActualType, pActualFormat,
				   pItems, pLeftover, pData);
	if (rcode == Success && *pActualType != None)
	    return Success;

	/* Gone, or no such property: forget it and do it the long way. */
	if (*pData)
	    XFree(*pData);
	*pActualType = None;
	*pData = NULL;
	MwmCacheForget(display, root);
    }

    if ((rcode = _DtGetMwmWindow(display, root, &wmWindow)) != Success)
	return rcode;

    rcode = GetPropertyTrapped(display, wmWindow, property, length,
			       reqType, pActualType, pActualFormat,
			       pItems, pLeftover, pData);
    if (rcode == Success)
	MwmCacheStore(display, root, wmWindow);
    return rcode;
}
