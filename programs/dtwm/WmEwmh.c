/*
 * CDE - Common Desktop Environment
 *
 * (c) Copyright 1993-2012 The Open Group
 * (c) Copyright 2012-2022 CDE Project contributors, see
 * CONTRIBUTORS for details
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

#include <stdlib.h>
#include <string.h>

#include "WmGlobal.h"
#include "WmEvent.h"
#include "WmCEvent.h"
#include "WmEwmh.h"
#include "WmWinList.h"
#include "WmFunction.h"
#include "WmIconBox.h"
#include "WmMultiHead.h"
#include "WmProperty.h"
#include "WmWinState.h"
#include "WmWrkspace.h"

/*
 * The window manager's EWMH state is published without reading the
 * server.  Where it changes, dtwm only marks the properties concerned
 * dirty (EwmhMarkScreen, EwmhMarkClient); EwmhFlushProc, an Xt work
 * procedure, recomputes them once dtwm has handled the events at hand,
 * and writes those whose value differs from the one it last wrote (kept
 * in pSD and pCD).  A burst of changes thus costs one write per
 * property, and a change that is undone before dtwm goes idle none.
 */

static Atom xa__NET_SUPPORTED;
static Atom xa__NET_SUPPORTING_WM_CHECK;
static Atom xa__NET_CLIENT_LIST;
static Atom xa__NET_CLIENT_LIST_STACKING;
static Atom xa__NET_NUMBER_OF_DESKTOPS;
static Atom xa__NET_DESKTOP_GEOMETRY;
static Atom xa__NET_DESKTOP_VIEWPORT;
static Atom xa__NET_CURRENT_DESKTOP;
static Atom xa__NET_DESKTOP_NAMES;

/*
 * The atoms, interned together by SetupWmEwmh.  _NET_SUPPORTED lists
 * those marked supported: set the flag when dtwm implements one.  An
 * atom used outside this file is kept in wmGD (a family of them in an
 * array indexed by its enum), the others here.
 */
static const struct
{
    char	*name;
    Atom	*pAtom;			/* where the atom is kept */
    Boolean	supported;		/* listed in _NET_SUPPORTED */
} netAtoms[] =
{
    { "UTF8_STRING",			&wmGD.xa_UTF8_STRING,		False },
    { _XA__NET_SUPPORTED,		&xa__NET_SUPPORTED,		False },
    { _XA__NET_SUPPORTING_WM_CHECK,	&xa__NET_SUPPORTING_WM_CHECK,	True },
    { _XA__NET_ACTIVE_WINDOW,		&wmGD.xa__NET_ACTIVE_WINDOW,	True },
    { "_NET_CLIENT_LIST",		&xa__NET_CLIENT_LIST,		True },
    { "_NET_CLIENT_LIST_STACKING",	&xa__NET_CLIENT_LIST_STACKING,	True },
    { "_NET_NUMBER_OF_DESKTOPS",	&xa__NET_NUMBER_OF_DESKTOPS,	True },
    { "_NET_DESKTOP_GEOMETRY",		&xa__NET_DESKTOP_GEOMETRY,	True },
    { "_NET_DESKTOP_VIEWPORT",		&xa__NET_DESKTOP_VIEWPORT,	True },
    { "_NET_CURRENT_DESKTOP",		&xa__NET_CURRENT_DESKTOP,	True },
    { "_NET_DESKTOP_NAMES",		&xa__NET_DESKTOP_NAMES,		True },
    { "_NET_WM_DESKTOP",		&wmGD.xa__NET_WM_DESKTOP,	True },
    { _XA__NET_WM_NAME,			&wmGD.xa__NET_WM_NAME,		True },
    { _XA__NET_WM_ICON_NAME,		&wmGD.xa__NET_WM_ICON_NAME,	True },
    { _XA__NET_WM_VISIBLE_NAME,		&wmGD.xa__NET_WM_VISIBLE_NAME,	True },
    { _XA__NET_WM_VISIBLE_ICON_NAME,
	&wmGD.xa__NET_WM_VISIBLE_ICON_NAME,				True },
    { _XA__NET_WM_FULLSCREEN_MONITORS,
	&wmGD.xa__NET_WM_FULLSCREEN_MONITORS,				True },
    { _XA__NET_WM_STATE,		&wmGD.xa__NET_WM_STATE,		True },
    { _XA__NET_WM_STATE_MAXIMIZED_VERT,
	&wmGD.xa__NET_WM_STATE_MAXIMIZED_VERT,				True },
    { _XA__NET_WM_STATE_MAXIMIZED_HORZ,
	&wmGD.xa__NET_WM_STATE_MAXIMIZED_HORZ,				True },
    { _XA__NET_WM_STATE_FULLSCREEN,	&wmGD.xa__NET_WM_STATE_FULLSCREEN,
									True },
    { _XA__NET_WM_STATE_ABOVE,		&wmGD.xa__NET_WM_STATE_ABOVE,	True },
    { _XA__NET_WM_STATE_BELOW,		&wmGD.xa__NET_WM_STATE_BELOW,	True },
};

/* The clients marked by EwmhMarkClient since the last flush. */
static ClientData **dirtyClients;
static int numDirtyClients;
static int sizeDirtyClients;

static Boolean EwmhFlushProc (XtPointer clientData);


/*************************************<->*************************************
 *
 *  EwmhMarkScreen (pSD, bits)
 *
 *
 *  Description:
 *  -----------
 *  Marks root window properties of a screen for EwmhFlushProc to
 *  recompute and write if they changed.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = screen
 *
 *  bits = NET_SD_* bits of the properties
 *
 *************************************<->***********************************/

void EwmhMarkScreen (WmScreenData *pSD, unsigned int bits)
{
    pSD->netDirty |= bits;

    if (!wmGD.ewmhFlushId)
	wmGD.ewmhFlushId = XtAppAddWorkProc (wmGD.mwmAppContext,
					     EwmhFlushProc, NULL);
}


/*************************************<->*************************************
 *
 *  EwmhMarkClient (pCD, bits)
 *
 *
 *  Description:
 *  -----------
 *  Marks properties of a client window for EwmhFlushProc to recompute
 *  and write if they changed.  Only the clients in the client lists have
 *  them (EwmhAddClient marks them all first); EwmhRemoveClient forgets
 *  the marks of a client that goes away.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = client
 *
 *  bits = NET_CD_* bits of the properties
 *
 *************************************<->***********************************/

void EwmhMarkClient (ClientData *pCD, unsigned int bits)
{
    if (!bits || !(pCD->netFlags & NET_FLAG_LISTED)) return;

    if (!pCD->netDirty)
    {
	if (numDirtyClients == sizeDirtyClients)
	{
	    sizeDirtyClients = sizeDirtyClients ? 2 * sizeDirtyClients : 16;
	    dirtyClients = (ClientData **) XtRealloc ((char *)dirtyClients,
		sizeDirtyClients * sizeof (ClientData *));
	}
	dirtyClients[numDirtyClients++] = pCD;
    }
    pCD->netDirty |= bits;

    if (!wmGD.ewmhFlushId)
	wmGD.ewmhFlushId = XtAppAddWorkProc (wmGD.mwmAppContext,
					     EwmhFlushProc, NULL);
}


/*************************************<->*************************************
 *
 *  EwmhAddClient (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Adds a newly managed client to _NET_CLIENT_LIST (in the order of
 *  management) and _NET_CLIENT_LIST_STACKING.  dtwm's own icon boxes and
 *  confirmation boxes, the front panel and its subpanels, and clients
 *  embedded in the front panel are not listed.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = client, already in the client list of its screen
 *
 *************************************<->***********************************/

void EwmhAddClient (ClientData *pCD)
{
    WmScreenData *pSD = pCD->pSD;

    if ((pCD->clientFlags & (CLIENT_WM_CLIENTS | FRONT_PANEL_BOX)) ||
	(pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL) ||
	pCD->pECD || (pCD->netFlags & NET_FLAG_LISTED))
	return;

    if (pSD->numNetClients == pSD->sizeNetClients)
    {
	pSD->sizeNetClients = pSD->sizeNetClients ?
	    2 * pSD->sizeNetClients : 32;
	pSD->netClients = (Window *) XtRealloc ((char *)pSD->netClients,
	    pSD->sizeNetClients * sizeof (Window));
    }
    pSD->netClients[pSD->numNetClients++] = pCD->client;
    pCD->netFlags |= NET_FLAG_LISTED;

    EwmhMarkScreen (pSD, NET_SD_CLIENTS | NET_SD_STACKING);
    EwmhMarkClient (pCD, NET_CD_DESKTOP);
}


/*************************************<->*************************************
 *
 *  EwmhRemoveClient (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Removes a client that is being withdrawn from the client lists, and
 *  drops its pending marks: call it before its client data is freed.
 *  The properties that dtwm wrote on the window go too (they stay when
 *  dtwm exits).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = client
 *
 *************************************<->***********************************/

void EwmhRemoveClient (ClientData *pCD)
{
    WmScreenData *pSD = pCD->pSD;
    int i;

    if (pCD->netDirty)
    {
	for (i = 0; i < numDirtyClients; ++i)
	{
	    if (dirtyClients[i] == pCD)
	    {
		dirtyClients[i] = dirtyClients[--numDirtyClients];
		break;
	    }
	}
	pCD->netDirty = 0;
    }

    if (!(pCD->netFlags & NET_FLAG_LISTED)) return;
    pCD->netFlags &= ~NET_FLAG_LISTED;

    if (pCD->netDesktopWritten != NET_DESKTOP_NOT_WRITTEN &&
	!(pCD->clientFlags & CLIENT_DESTROYED))
	XDeleteProperty (DISPLAY, pCD->client, wmGD.xa__NET_WM_DESKTOP);

    for (i = 0; i < pSD->numNetClients; ++i)
    {
	if (pSD->netClients[i] == pCD->client)
	{
	    memmove (&pSD->netClients[i], &pSD->netClients[i + 1],
		     (pSD->numNetClients - i - 1) * sizeof (Window));
	    --pSD->numNetClients;

	    /* the written list no longer is a prefix of netClients */
	    if (i < pSD->numNetClientsWritten)
		pSD->numNetClientsWritten = -1;
	    break;
	}
    }

    EwmhMarkScreen (pSD, NET_SD_CLIENTS | NET_SD_STACKING);
}


/*
 * Write a list of windows to a root window property, of which the first
 * numWritten are already there (or -1: the property is to be replaced).
 */
static void WriteNetWindowList (WmScreenData *pSD, Atom property,
    Window *windows, int num, int numWritten)
{
    if (numWritten >= 0 && num >= numWritten)
    {
	if (num > numWritten)
	    XChangeProperty (DISPLAY, pSD->rootWindow, property, XA_WINDOW, 32,
		PropModeAppend, (unsigned char *)(windows + numWritten),
		num - numWritten);
    }
    else
    {
	XChangeProperty (DISPLAY, pSD->rootWindow, property, XA_WINDOW, 32,
	    PropModeReplace, (unsigned char *)windows, num);
    }
}


/*
 * _NET_ACTIVE_WINDOW: confirmed client focus, not icons, hidden windows
 * or unlisted ones (the front panel, ...).
 */
static void FlushNetActiveWindow (WmScreenData *pSD)
{
    ClientData *pCD = wmGD.keyboardFocus;
    Window active = None;

    if (pCD && pCD->pSD == pSD && (pCD->netFlags & NET_FLAG_LISTED) &&
	(pCD->clientState == NORMAL_STATE ||
	 pCD->clientState == MAXIMIZED_STATE))
	active = pCD->client;

    if (active == pSD->netActiveWritten) return;

    XChangeProperty (DISPLAY, pSD->rootWindow, wmGD.xa__NET_ACTIVE_WINDOW,
	XA_WINDOW, 32, PropModeReplace, (unsigned char *)&active, 1);
    pSD->netActiveWritten = active;
}


/*
 * _NET_CLIENT_LIST: netClients only grows at its end, so what was
 * written is either a prefix of it, to append to, or replaced.
 */
static void FlushNetClientList (WmScreenData *pSD)
{
    if (pSD->numNetClients == pSD->numNetClientsWritten) return;

    WriteNetWindowList (pSD, xa__NET_CLIENT_LIST, pSD->netClients,
	pSD->numNetClients, pSD->numNetClientsWritten);
    pSD->numNetClientsWritten = pSD->numNetClients;
}


/*
 * _NET_CLIENT_LIST_STACKING: the listed clients from bottom to top, that
 * is the client list of the screen from its end, each entry followed by
 * the transient windows stacked with it (MakeTransientFamilyStackingList
 * gives the order of a transient tree, from the top).  New windows go on
 * top: when the old list is a prefix of the new one, append.
 */
static void FlushNetClientListStacking (WmScreenData *pSD)
{
    static Window *stack = NULL, *family = NULL;
    static int sizeStack = 0, sizeFamily = 0;
    ClientListEntry *pEntry;
    ClientData *pCD;
    int num = 0, count, i, prefix;

    if (sizeStack < pSD->numNetClients)
    {
	sizeStack = pSD->numNetClients + 32;
	stack = (Window *) XtRealloc ((char *)stack,
	    sizeStack * sizeof (Window));
    }

    for (pEntry = pSD->lastClient; pEntry; pEntry = pEntry->prevSibling)
    {
	pCD = pEntry->pCD;
	if (pEntry != &pCD->clientEntry) continue;	/* an icon */

	if (!pCD->transientChildren)
	{
	    if ((pCD->netFlags & NET_FLAG_LISTED) && num < sizeStack)
		stack[num++] = pCD->client;
	    continue;
	}

	count = CountTransientChildren (pCD);
	if (sizeFamily < count)
	{
	    sizeFamily = count + 8;
	    family = (Window *) XtRealloc ((char *)family,
		sizeFamily * sizeof (Window));
	}
	MakeTransientFamilyStackingList (family, pCD);

	for (i = count - 1; i >= 0; --i)
	{
	    ClientData *pcdFamily;

	    if (!XFindContext (DISPLAY, family[i], wmGD.windowContextType,
			       (caddr_t *)&pcdFamily) &&
		(pcdFamily->netFlags & NET_FLAG_LISTED) && num < sizeStack)
		stack[num++] = pcdFamily->client;
	}
    }

    for (prefix = 0; prefix < num && prefix < pSD->numNetStackWritten &&
		     stack[prefix] == pSD->netStackWritten[prefix]; ++prefix)
	;
    if (prefix == num && num == pSD->numNetStackWritten) return;

    WriteNetWindowList (pSD, xa__NET_CLIENT_LIST_STACKING, stack, num,
	prefix == pSD->numNetStackWritten ? prefix : -1);

    if (pSD->sizeNetStackWritten < num)
    {
	pSD->sizeNetStackWritten = sizeStack;
	pSD->netStackWritten = (Window *) XtRealloc (
	    (char *)pSD->netStackWritten, sizeStack * sizeof (Window));
    }
    memcpy (pSD->netStackWritten, stack, num * sizeof (Window));
    pSD->numNetStackWritten = num;
}


/* The number of desktops: one in mwm mode, else one per workspace. */
static int NumNetDesktops (WmScreenData *pSD)
{
    return wmGD.useStandardBehavior ? 1 : pSD->numWorkspaces;
}


/* The desktop index of a workspace, its index in pSD->pWS. */
static long NetDesktopIndex (WmScreenData *pSD, WorkspaceID wsID)
{
    WmWorkspaceData *pWS;

    if (wmGD.useStandardBehavior ||
	!(pWS = GetWorkspaceData (pSD, wsID)))
	return 0;
    return pWS - pSD->pWS;
}


/*
 * _NET_WM_DESKTOP can only say one desktop, or all of them: all when the
 * client is to be in every workspace (putInAll) or is in all of several,
 * else the active workspace if the client is in it, else the lowest
 * index among the workspaces it is in.
 */
static long NetWmDesktop (ClientData *pCD)
{
    WmScreenData *pSD = pCD->pSD;
    int num = NumNetDesktops (pSD);
    long desktop, lowest = num;
    int i;

    if (wmGD.useStandardBehavior)
	return 0;
    if (pCD->putInAll || (num > 1 && pCD->numInhabited >= num))
	return NET_ALL_DESKTOPS;
    if (ClientInWorkspace (pSD->pActiveWS, pCD))
	return pSD->pActiveWS - pSD->pWS;

    for (i = 0; i < pCD->numInhabited; ++i)
    {
	desktop = NetDesktopIndex (pSD, pCD->pWsList[i].wsID);
	if (desktop < lowest) lowest = desktop;
    }
    return lowest < num ? lowest : pSD->pActiveWS - pSD->pWS;
}


/* Client window properties: _NET_WM_DESKTOP (the others are to come). */
static void FlushClient (ClientData *pCD)
{
    long desktop;

    if (pCD->netDirty & NET_CD_DESKTOP)
    {
	desktop = NetWmDesktop (pCD);
	if (desktop != pCD->netDesktopWritten)
	{
	    XChangeProperty (DISPLAY, pCD->client, wmGD.xa__NET_WM_DESKTOP,
		XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&desktop, 1);
	    pCD->netDesktopWritten = desktop;
	}
    }

    pCD->netDirty = 0;
}


/*
 * _NET_NUMBER_OF_DESKTOPS, _NET_DESKTOP_VIEWPORT (no viewports: 0,0 for
 * each desktop) and _NET_DESKTOP_GEOMETRY (the screen).
 */
static void FlushNetDesktops (WmScreenData *pSD)
{
    int num = NumNetDesktops (pSD);
    long *viewport;

    if (num != pSD->netNumDesktopsWritten)
    {
	long number = num;

	XChangeProperty (DISPLAY, pSD->rootWindow, xa__NET_NUMBER_OF_DESKTOPS,
	    XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&number, 1);

	viewport = (long *) XtCalloc (2 * num, sizeof (long));
	XChangeProperty (DISPLAY, pSD->rootWindow, xa__NET_DESKTOP_VIEWPORT,
	    XA_CARDINAL, 32, PropModeReplace, (unsigned char *)viewport,
	    2 * num);
	XtFree ((char *)viewport);

	pSD->netNumDesktopsWritten = num;
    }

    if (pSD->netGeometry[0] != pSD->netGeometryWritten[0] ||
	pSD->netGeometry[1] != pSD->netGeometryWritten[1])
    {
	XChangeProperty (DISPLAY, pSD->rootWindow, xa__NET_DESKTOP_GEOMETRY,
	    XA_CARDINAL, 32, PropModeReplace,
	    (unsigned char *)pSD->netGeometry, 2);
	pSD->netGeometryWritten[0] = pSD->netGeometry[0];
	pSD->netGeometryWritten[1] = pSD->netGeometry[1];
    }
}


/* _NET_DESKTOP_NAMES: the workspace titles, in UTF-8. */
static void FlushNetDesktopNames (WmScreenData *pSD)
{
    int num = NumNetDesktops (pSD);
    char **titles;
    XTextProperty text;
    int i, len;

    titles = (char **) XtMalloc (num * sizeof (char *));
    for (i = 0; i < num; ++i)
	titles[i] = WmXmStringToString (pSD->pWS[i].title);
    for (i = 0; i < num; ++i)
	if (!titles[i]) titles[i] = XtNewString ("");

    if (XmbTextListToTextProperty (DISPLAY, titles, num, XUTF8StringStyle,
				   &text) >= Success)
    {
	/* each name is terminated, the last one too */
	len = text.nitems + 1;
	if (len != pSD->lenNetNamesWritten ||
	    memcmp (text.value, pSD->netNamesWritten, len))
	{
	    XChangeProperty (DISPLAY, pSD->rootWindow, xa__NET_DESKTOP_NAMES,
		wmGD.xa_UTF8_STRING, 8, PropModeReplace, text.value, len);
	    XtFree (pSD->netNamesWritten);
	    pSD->netNamesWritten = XtMalloc (len);
	    memcpy (pSD->netNamesWritten, text.value, len);
	    pSD->lenNetNamesWritten = len;
	}
	XFree (text.value);
    }

    for (i = 0; i < num; ++i)
	XtFree (titles[i]);
    XtFree ((char *)titles);
}


/*************************************<->*************************************
 *
 *  UpdateNetCurrentDesktop (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Writes _NET_CURRENT_DESKTOP if it changed, at once (not from
 *  EwmhFlushProc): SetCurrentWorkspaceProperty, which calls it, then
 *  syncs before the workspace change is announced.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = screen
 *
 *************************************<->***********************************/

void UpdateNetCurrentDesktop (WmScreenData *pSD)
{
    long current;

    current = wmGD.useStandardBehavior ? 0 : pSD->pActiveWS - pSD->pWS;
    if (current == pSD->netCurrentWritten) return;

    XChangeProperty (DISPLAY, pSD->rootWindow, xa__NET_CURRENT_DESKTOP,
	XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&current, 1);
    pSD->netCurrentWritten = current;
}


/*************************************<->*************************************
 *
 *  EwmhWorkspacesChanged (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Marks what depends on the set of workspaces of a screen after one was
 *  created or deleted: the number of desktops and their names, and the
 *  _NET_WM_DESKTOP of every client (deleting a workspace shifts the
 *  indices of those after it).
 *
 *
 *  Inputs:
 *  ------
 *  pSD = screen
 *
 *************************************<->***********************************/

void EwmhWorkspacesChanged (WmScreenData *pSD)
{
    int i, j;

    for (i = 0; i < pSD->numWorkspaces; ++i)
	for (j = 0; j < pSD->pWS[i].numClients; ++j)
	    EwmhMarkClient (pSD->pWS[i].ppClients[j], NET_CD_DESKTOP);

    EwmhMarkScreen (pSD, NET_SD_DESKTOPS | NET_SD_NAMES);
}


/*************************************<->*************************************
 *
 *  EwmhRootResized (pSD, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Takes the new size of the root window (RandR) for
 *  _NET_DESKTOP_GEOMETRY.
 *
 *************************************<->***********************************/

void EwmhRootResized (WmScreenData *pSD, int width, int height)
{
    pSD->netGeometry[0] = width;
    pSD->netGeometry[1] = height;
    EwmhMarkScreen (pSD, NET_SD_DESKTOPS);
}


/*************************************<->*************************************
 *
 *  GetNetWmDesktop (pCD, ppIDs, pNumIDs, pbAll)
 *
 *
 *  Description:
 *  -----------
 *  Reads the _NET_WM_DESKTOP that a client set before it was mapped, the
 *  workspace it asks to start in, or all of them.  It is read only when
 *  the window has it (HasProperty), and ignored in mwm mode or when the
 *  index is out of range.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = client being managed (its initial property list is known)
 *
 *
 *  Outputs:
 *  -------
 *  *ppIDs = the workspace IDs (XtMalloc'd), *pNumIDs = how many
 *  *pbAll = True for all workspaces
 *  RETURN = True if the property gave workspaces
 *
 *************************************<->***********************************/

Boolean GetNetWmDesktop (ClientData *pCD, WorkspaceID **ppIDs,
    unsigned int *pNumIDs, Boolean *pbAll)
{
    WmScreenData *pSD = pCD->pSD;
    Atom actualType;
    int actualFormat, i;
    unsigned long nitems, leftover, desktop;
    unsigned char *data = NULL;

    if (wmGD.useStandardBehavior ||
	!HasProperty (pCD, wmGD.xa__NET_WM_DESKTOP) ||
	XGetWindowProperty (DISPLAY, pCD->client, wmGD.xa__NET_WM_DESKTOP,
	    0L, 1L, False, XA_CARDINAL, &actualType, &actualFormat, &nitems,
	    &leftover, &data) != Success)
	return False;

    if (actualType != XA_CARDINAL || actualFormat != 32 || nitems != 1)
    {
	if (data) XFree (data);
	return False;
    }
    desktop = *(unsigned long *)data & 0xFFFFFFFFUL;
    XFree (data);

    if (desktop == NET_ALL_DESKTOPS)
    {
	*ppIDs = (WorkspaceID *) XtMalloc (pSD->numWorkspaces *
					   sizeof (WorkspaceID));
	for (i = 0; i < pSD->numWorkspaces; ++i)
	    (*ppIDs)[i] = pSD->pWS[i].id;
	*pNumIDs = pSD->numWorkspaces;
	*pbAll = True;
	return True;
    }
    if (desktop >= pSD->numWorkspaces)
	return False;

    *ppIDs = (WorkspaceID *) XtMalloc (sizeof (WorkspaceID));
    (*ppIDs)[0] = pSD->pWS[desktop].id;
    *pNumIDs = 1;
    *pbAll = False;
    return True;
}


/*************************************<->*************************************
 *
 *  ProcessNetWmDesktop (pCD, desktop)
 *
 *
 *  Description:
 *  -----------
 *  Handles a _NET_WM_DESKTOP client message: moves the client (with its
 *  transient tree) to one workspace, or puts it in all of them, if it
 *  may change workspaces.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = client
 *
 *  desktop = desktop index, or 0xFFFFFFFF for all
 *
 *************************************<->***********************************/

void ProcessNetWmDesktop (ClientData *pCD, unsigned long desktop)
{
    WmScreenData *pSD = pCD->pSD;
    ClientData *pcdLeader = FindTransientTreeLeader (pCD);
    WorkspaceID *pOthers;
    WorkspaceID id;
    int i, numOthers = 0;

    if (wmGD.useStandardBehavior ||
	!(pCD->dtwmFunctions & DtWM_FUNCTION_OCCUPY_WS))
	return;

    desktop &= 0xFFFFFFFFUL;
    if (desktop == NET_ALL_DESKTOPS)
    {
	F_AddToAllWorkspaces (NULL, pcdLeader, NULL);
	return;
    }
    if (desktop >= pSD->numWorkspaces)
	return;

    id = pSD->pWS[desktop].id;
    AddClientToWorkspaces (pcdLeader, &id, 1);

    /* the list changes as the client leaves workspaces: copy it */
    pOthers = (WorkspaceID *) XtMalloc (pcdLeader->numInhabited *
					sizeof (WorkspaceID));
    for (i = 0; i < pcdLeader->numInhabited; ++i)
	if (pcdLeader->pWsList[i].wsID != id)
	    pOthers[numOthers++] = pcdLeader->pWsList[i].wsID;
    if (numOthers)
	RemoveClientFromWorkspaces (pcdLeader, pOthers, numOthers);
    XtFree ((char *)pOthers);

    pcdLeader->putInAll = pCD->putInAll = False;
    EwmhMarkClient (pCD, NET_CD_DESKTOP);
    EwmhMarkClient (pcdLeader, NET_CD_DESKTOP);
}


/*************************************<->*************************************
 *
 *  EwmhFlushProc (clientData)
 *
 *
 *  Description:
 *  -----------
 *  The Xt work procedure that writes the properties marked dirty, when
 *  their value changed.  It never reads the server.
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True: the work procedure is done (EwmhMark* add it again)
 *
 *************************************<->***********************************/

static Boolean EwmhFlushProc (XtPointer clientData)
{
    unsigned int bits;
    int scr;

    wmGD.ewmhFlushId = 0;

    while (numDirtyClients)
	FlushClient (dirtyClients[--numDirtyClients]);

    for (scr = 0; scr < wmGD.numScreens; ++scr)
    {
	WmScreenData *pSD = &wmGD.Screens[scr];

	if (!pSD->managed || !pSD->netDirty) continue;
	bits = pSD->netDirty;
	pSD->netDirty = 0;

	/*
	 * The focus first: a client that sees a list change also sees
	 * the focus change that came with it.
	 */
	if (bits & NET_SD_ACTIVE) FlushNetActiveWindow (pSD);
	if (bits & NET_SD_CLIENTS) FlushNetClientList (pSD);
	if (bits & NET_SD_STACKING) FlushNetClientListStacking (pSD);
	if (bits & NET_SD_DESKTOPS) FlushNetDesktops (pSD);
	if (bits & NET_SD_NAMES) FlushNetDesktopNames (pSD);
    }

    return True;
}


/* _NET_ACTIVE_WINDOW follows the keyboard focus. */
void UpdateNetActiveWindow (void)
{
    int scr;

    for (scr = 0; scr < wmGD.numScreens; ++scr)
	if (wmGD.Screens[scr].managed)
	    EwmhMarkScreen (&wmGD.Screens[scr], NET_SD_ACTIVE);
}


/*************************************<->*************************************
 *
 *  HandleNetRootMessage (pSD, clientEvent)
 *
 *
 *  Description:
 *  -----------
 *  Handles a client message sent to a root window, an EWMH request about
 *  the screen rather than one of its clients (those are handled by
 *  HandleClientMessage).
 *
 *
 *  Inputs:
 *  ------
 *  pSD = the screen of the root window
 *
 *  clientEvent = the client message
 *
 *************************************<->***********************************/

void HandleNetRootMessage (WmScreenData *pSD, XClientMessageEvent *clientEvent)
{
    Atom type = clientEvent->message_type;
    long value = clientEvent->data.l[0];
    int num;

    /* in mwm mode, there is one desktop */
    if (clientEvent->format != 32 || wmGD.useStandardBehavior) return;

    if (type == xa__NET_CURRENT_DESKTOP)
    {
	if (value >= 0 && value < pSD->numWorkspaces)
	    ChangeToWorkspace (&pSD->pWS[value]);
    }
    else if (type == xa__NET_NUMBER_OF_DESKTOPS)
    {
	/* add or delete workspaces at the end */
	if (value < 1) value = 1;
	if (value > MAX_WORKSPACE_COUNT) value = MAX_WORKSPACE_COUNT;

	while (pSD->numWorkspaces < value)
	{
	    num = pSD->numWorkspaces;
	    CreateWorkspace (pSD, NULL);
	    if (pSD->numWorkspaces == num) break;
	}
	while (pSD->numWorkspaces > value)
	{
	    num = pSD->numWorkspaces;
	    DeleteWorkspace (&pSD->pWS[num - 1]);
	    if (pSD->numWorkspaces == num) break;
	}
    }
}

/* Use the normal focus path so ICCCM input models and modality still apply. */
void ProcessNetActiveWindow (ClientData *pCD, Time timestamp)
{
    ClientData *pLeader;
    WmWorkspaceData *pWS;

    if ((pCD->clientState & ~UNSEEN_STATE) == WITHDRAWN_STATE) return;
    pLeader = FindTransientTreeLeader (pCD);
    if (wmGD.systemModalActive && pLeader != wmGD.systemModalClient) return;

    pWS = pCD->pSD->pActiveWS;
    if (!ClientInWorkspace (pWS, pCD))
    {
        if (!pCD->numInhabited) return;
        pWS = GetWorkspaceData (pCD->pSD, pCD->pWsList[0].wsID);
        if (!pWS) return;
    }

    SetActiveScreen (pCD->pSD);
    if (pWS != pCD->pSD->pActiveWS) ChangeToWorkspace (pWS);
    if (timestamp == CurrentTime) timestamp = GetTimestamp ();

    if (pLeader->clientState == MINIMIZED_STATE)
        SetClientState (pLeader,
            pLeader->maxConfig ? MAXIMIZED_STATE : NORMAL_STATE, timestamp);

    Do_Raise (pCD, NULL, STACK_NORMAL);
    Do_Focus_Key (pCD, timestamp, ALWAYS_SET_FOCUS);
}

static void ProcessNetWmStateAbove (ClientData *pCD, long action);
static void ProcessNetWmStateBelow (ClientData *pCD, long action);

static void ProcessNetWmStateMaximized (ClientData *pCD, long action)
{
    int newState;

    switch (action)
    {
	case _NET_WM_STATE_REMOVE:
	    if (pCD->clientState != MAXIMIZED_STATE) return;
	    newState = NORMAL_STATE;
	    break;
	case _NET_WM_STATE_ADD:
	    if (pCD->clientState == MAXIMIZED_STATE) return;
	    newState = MAXIMIZED_STATE;
	    break;
	case _NET_WM_STATE_TOGGLE:
	    newState = pCD->clientState == MAXIMIZED_STATE ?
		NORMAL_STATE : MAXIMIZED_STATE;
	    break;
	default:
	    return;
    }

    SetClientState (pCD, newState, GetTimestamp ());
}

static void ProcessNetWmStateFullscreen (ClientData *pCD, long action)
{
    Boolean fullscreen = pCD->fullscreen;

    switch (action)
    {
	case _NET_WM_STATE_REMOVE:
	    if (!fullscreen) return;
	    fullscreen = False;
	    break;
	case _NET_WM_STATE_ADD:
	    if (fullscreen) return;
	    fullscreen = True;
	    break;
	case _NET_WM_STATE_TOGGLE:
	    fullscreen = !fullscreen;
	    break;
	default:
	    return;
    }

    pCD->fullscreen = False;
    SetClientState (pCD, NORMAL_STATE, GetTimestamp ());

    if (fullscreen) {
	pCD->fullscreen = True;
	SetClientState (pCD, MAXIMIZED_STATE, GetTimestamp ());
    }
}

static void ProcessNetWmStateAbove (ClientData *pCD, long action)
{
    if (action == _NET_WM_STATE_TOGGLE)
	action = pCD == pCD->pSD->topClient ? _NET_WM_STATE_REMOVE :
					      _NET_WM_STATE_ADD;

    switch (action)
    {
	case _NET_WM_STATE_REMOVE:
	    if (pCD->pSD->topClient == pCD) pCD->pSD->topClient = NULL;
	    break;
	case _NET_WM_STATE_ADD:
	    pCD->pSD->topClient = pCD;
	    ProcessNetWmStateBelow (pCD, _NET_WM_STATE_REMOVE);
	    Do_Raise (pCD, NULL, STACK_NORMAL);
	    break;
	default:
	    return;
    }

    UpdateNetWmState (pCD->client, &wmGD.xa__NET_WM_STATE_ABOVE, 1, action);
}

static void ProcessNetWmStateBelow (ClientData *pCD, long action)
{
    if (action == _NET_WM_STATE_TOGGLE)
	action = pCD == pCD->pSD->bottomClient ? _NET_WM_STATE_REMOVE :
						 _NET_WM_STATE_ADD;

    switch (action)
    {
	case _NET_WM_STATE_REMOVE:
	    if (pCD->pSD->bottomClient == pCD) pCD->pSD->bottomClient = NULL;
	    break;
	case _NET_WM_STATE_ADD:
	    pCD->pSD->bottomClient = pCD;
	    ProcessNetWmStateAbove (pCD, _NET_WM_STATE_REMOVE);
	    Do_Lower (pCD, NULL, STACK_NORMAL);
	    break;
	default:
	    return;
    }

    UpdateNetWmState (pCD->client, &wmGD.xa__NET_WM_STATE_BELOW, 1, action);
}

/**
 * @brief Processes the _NET_WM_FULLSCREEN_MONITORS protocol.
 *
 * @param pCD
 * @param top
 * @param bottom
 * @param left
 * @param right
 */
void ProcessNetWmFullscreenMonitors (ClientData *pCD,
    int top, int bottom, int left, int right)
{
    WmHeadInfo_t *pHeadInfo;

    pCD->fullscreenAuto = True;

    if (!(pHeadInfo = GetHeadInfoById (top))) return;
    pCD->fullscreenY = pHeadInfo->y_org;
    free(pHeadInfo);

    if (!(pHeadInfo = GetHeadInfoById (bottom))) return;
    pCD->fullscreenHeight = top == bottom ? pHeadInfo->height :
	    pHeadInfo->y_org + pHeadInfo->height;
    free(pHeadInfo);

    if (!(pHeadInfo = GetHeadInfoById (left))) return;
    pCD->fullscreenX = pHeadInfo->x_org;
    free(pHeadInfo);

    if (!(pHeadInfo = GetHeadInfoById (right))) return;
    pCD->fullscreenWidth = left == right ? pHeadInfo->width :
	    pHeadInfo->x_org + pHeadInfo->width;
    free(pHeadInfo);

    pCD->fullscreenAuto = False;
}

/**
 * @brief Processes the _NET_WM_STATE client message.
 *
 * @param pCD
 * @param action
 * @param firstProperty
 * @param secondProperty
 */
void ProcessNetWmState (ClientData *pCD, long action,
    Atom firstProperty, Atom secondProperty)
{
    if (pCD->clientState & UNSEEN_STATE) return;

    if ((firstProperty  == wmGD.xa__NET_WM_STATE_MAXIMIZED_VERT &&
	 secondProperty == wmGD.xa__NET_WM_STATE_MAXIMIZED_HORZ) ||
	(firstProperty  == wmGD.xa__NET_WM_STATE_MAXIMIZED_HORZ &&
	 secondProperty == wmGD.xa__NET_WM_STATE_MAXIMIZED_VERT))
	ProcessNetWmStateMaximized (pCD, action);
    else if (firstProperty  == wmGD.xa__NET_WM_STATE_FULLSCREEN ||
	     secondProperty == wmGD.xa__NET_WM_STATE_FULLSCREEN)
	ProcessNetWmStateFullscreen (pCD, action);
    else if (firstProperty  == wmGD.xa__NET_WM_STATE_ABOVE ||
	     secondProperty == wmGD.xa__NET_WM_STATE_ABOVE)
	ProcessNetWmStateAbove (pCD, action);
    else if (firstProperty  == wmGD.xa__NET_WM_STATE_BELOW ||
	     secondProperty == wmGD.xa__NET_WM_STATE_BELOW)
	ProcessNetWmStateBelow (pCD, action);

    if (!ClientInWorkspace (ACTIVE_WS, pCD))
	SetClientState (pCD, pCD->clientState | UNSEEN_STATE, GetTimestamp ());
}

/*************************************<->*************************************
 *
 *  SetupWmEwmh ()
 *
 *
 *  Description:
 *  -----------
 *  Interns the EWMH atoms, sets up the supporting window manager check
 *  and _NET_SUPPORTED, and marks the root window properties for the first
 *  flush, which replaces those left by a previous window manager once the
 *  initial clients are adopted.
 *
 *************************************<->***********************************/

void SetupWmEwmh (void)
{
    char *names[XtNumber(netAtoms)];
    Atom atoms[XtNumber(netAtoms)];
    Atom supported[XtNumber(netAtoms)];
    int i, numSupported = 0;
    int scr;

    for (i = 0; i < XtNumber(netAtoms); ++i)
	names[i] = netAtoms[i].name;
    XInternAtoms (DISPLAY, names, XtNumber(netAtoms), False, atoms);

    for (i = 0; i < XtNumber(netAtoms); ++i)
    {
	*netAtoms[i].pAtom = atoms[i];
	if (netAtoms[i].supported)
	    supported[numSupported++] = atoms[i];
    }

    for (scr = 0; scr < wmGD.numScreens; ++scr)
    {
	Window childWindow;
	WmScreenData *pSD;

	pSD = &(wmGD.Screens[scr]);

	if (!pSD->managed) continue;

	childWindow = pSD->wmWorkspaceWin;

	XChangeProperty(DISPLAY, childWindow, wmGD.xa__NET_WM_NAME,
			wmGD.xa_UTF8_STRING, 8, PropModeReplace,
			(unsigned char *)DT_WM_RESOURCE_NAME, 5);

	XChangeProperty(DISPLAY, childWindow,
			xa__NET_SUPPORTING_WM_CHECK, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&childWindow, 1);

	XChangeProperty(DISPLAY, pSD->rootWindow,
			xa__NET_SUPPORTING_WM_CHECK, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&childWindow, 1);

	XChangeProperty(DISPLAY, pSD->rootWindow,
			xa__NET_SUPPORTED, XA_ATOM, 32, PropModeReplace,
			(unsigned char *)supported, numSupported);

	EwmhMarkScreen (pSD, NET_SD_CLIENTS | NET_SD_STACKING | NET_SD_ACTIVE |
			     NET_SD_DESKTOPS | NET_SD_NAMES);
    }
}
