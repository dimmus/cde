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
 *  and write if they changed.  EwmhRemoveClient forgets the marks of a
 *  client that goes away first.
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
    if (!bits) return;

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


/* _NET_ACTIVE_WINDOW: confirmed client focus, not icons or hidden windows. */
static void FlushNetActiveWindow (WmScreenData *pSD)
{
    ClientData *pCD = wmGD.keyboardFocus;
    Window active = None;

    if (pCD && pCD->pSD == pSD &&
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


/* Client window properties: written from the stages that add them. */
static void FlushClient (ClientData *pCD)
{
    pCD->netDirty = 0;
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
    if (clientEvent->format != 32) return;

    /*
     * No root message is handled yet: the desktop requests
     * (_NET_CURRENT_DESKTOP, _NET_NUMBER_OF_DESKTOPS,
     * _NET_SHOWING_DESKTOP) are to come here.
     */
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

	EwmhMarkScreen (pSD, NET_SD_CLIENTS | NET_SD_STACKING | NET_SD_ACTIVE);
    }
}
