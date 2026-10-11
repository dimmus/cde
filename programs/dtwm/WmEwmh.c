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

#include <limits.h>
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
#include "WmWinConf.h"
#include "WmWinInfo.h"
#include "WmWinState.h"
#include "WmWrkspace.h"
#include "WmCDInfo.h"

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
static Atom xa__NET_WM_WINDOW_TYPE;
static Atom xa__NET_WORKAREA;
static Atom xa__NET_WM_ALLOWED_ACTIONS;
static Atom xa__NET_FRAME_EXTENTS;
static Atom xa__NET_REQUEST_FRAME_EXTENTS;
static Atom xa__NET_CLOSE_WINDOW;
static Atom xa__NET_MOVERESIZE_WINDOW;
static Atom xa__NET_RESTACK_WINDOW;
static Atom xa__NET_SHOWING_DESKTOP;

/* the _NET_WM_ALLOWED_ACTIONS atoms (shading is not supported) */
enum {
    NETACT_MOVE,
    NETACT_RESIZE,
    NETACT_MINIMIZE,
    NETACT_STICK,
    NETACT_MAXIMIZE_HORZ,
    NETACT_MAXIMIZE_VERT,
    NETACT_FULLSCREEN,
    NETACT_CHANGE_DESKTOP,
    NETACT_CLOSE,
    NETACT_ABOVE,
    NETACT_BELOW,
    NETACT_COUNT
};

static Atom netActions[NETACT_COUNT];

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
    { "_NET_WORKAREA",			&xa__NET_WORKAREA,		True },
    { "_NET_WM_STRUT",			&wmGD.xa__NET_WM_STRUT,		True },
    { "_NET_WM_STRUT_PARTIAL",		&wmGD.xa__NET_WM_STRUT_PARTIAL,	True },
    { _XA__NET_WM_STATE,		&wmGD.xa__NET_WM_STATE,		True },
#define NETSTATE(s) \
    { "_NET_WM_STATE_" #s, &wmGD.xa__NET_WM_STATE_[NETSTATE_ ## s],	True }
    NETSTATE (MODAL),
    NETSTATE (STICKY),
    NETSTATE (MAXIMIZED_VERT),
    NETSTATE (MAXIMIZED_HORZ),
    NETSTATE (SKIP_TASKBAR),
    NETSTATE (SKIP_PAGER),
    NETSTATE (HIDDEN),
    NETSTATE (FULLSCREEN),
    NETSTATE (ABOVE),
    NETSTATE (BELOW),
    NETSTATE (DEMANDS_ATTENTION),
    NETSTATE (FOCUSED),
#undef NETSTATE
    { "_NET_WM_WINDOW_TYPE",		&xa__NET_WM_WINDOW_TYPE,	True },
#define NETTYPE(t) \
    { "_NET_WM_WINDOW_TYPE_" #t, \
      &wmGD.xa__NET_WM_WINDOW_TYPE_[NETTYPE_ ## t],			True }
    NETTYPE (NORMAL),
    NETTYPE (DESKTOP),
    NETTYPE (DOCK),
    NETTYPE (TOOLBAR),
    NETTYPE (MENU),
    NETTYPE (UTILITY),
    NETTYPE (SPLASH),
    NETTYPE (DIALOG),
    NETTYPE (DROPDOWN_MENU),
    NETTYPE (POPUP_MENU),
    NETTYPE (TOOLTIP),
    NETTYPE (NOTIFICATION),
    NETTYPE (COMBO),
    NETTYPE (DND),
#undef NETTYPE
    { "_NET_WM_ALLOWED_ACTIONS",	&xa__NET_WM_ALLOWED_ACTIONS,	True },
#define NETACT(a) \
    { "_NET_WM_ACTION_" #a, &netActions[NETACT_ ## a],			True }
    NETACT (MOVE),
    NETACT (RESIZE),
    NETACT (MINIMIZE),
    NETACT (STICK),
    NETACT (MAXIMIZE_HORZ),
    NETACT (MAXIMIZE_VERT),
    NETACT (FULLSCREEN),
    NETACT (CHANGE_DESKTOP),
    NETACT (CLOSE),
    NETACT (ABOVE),
    NETACT (BELOW),
#undef NETACT
    { "_NET_FRAME_EXTENTS",		&xa__NET_FRAME_EXTENTS,		True },
    { "_NET_REQUEST_FRAME_EXTENTS",	&xa__NET_REQUEST_FRAME_EXTENTS,	True },
    { "_NET_CLOSE_WINDOW",		&xa__NET_CLOSE_WINDOW,		True },
    { "_NET_MOVERESIZE_WINDOW",		&xa__NET_MOVERESIZE_WINDOW,	True },
    { "_NET_RESTACK_WINDOW",		&xa__NET_RESTACK_WINDOW,	True },
    { "_NET_WM_MOVERESIZE",		&wmGD.xa__NET_WM_MOVERESIZE,	True },
    { "_NET_SHOWING_DESKTOP",		&xa__NET_SHOWING_DESKTOP,	True },
};

/*
 * What a window type means to dtwm: the default decorations and
 * functions (TYPE_UNCHANGED: dtwm's own default, TYPE_TRANSIENT: those
 * of transient windows), which an explicit clientDecoration or
 * clientFunctions resource and then _MOTIF_WM_HINTS override; the layer;
 * and TYPE_* flags.
 */
#define TYPE_UNCHANGED		(-1)
#define TYPE_TRANSIENT		(-2)

#define TYPE_NO_FOCUS		(1 << 0)	/* not given the focus when */
						/* mapped */
#define TYPE_ALL_WORKSPACES	(1 << 1)	/* always in every workspace */
#define TYPE_STICKY		(1 << 2)	/* in every one, by default */
#define TYPE_POSITION		(1 << 3)	/* its position is honoured */
#define TYPE_CENTER		(1 << 4)	/* centred on its head */
#define TYPE_PSEUDO_TRANSIENT	(1 << 5)	/* a transient of its group */

static const struct
{
    int		decoration;
    int		functions;
    unsigned char layer;
    unsigned char flags;
} netTypes[NETTYPE_COUNT] =
{
    [NETTYPE_NORMAL] =
	{ TYPE_UNCHANGED, TYPE_UNCHANGED, LAYER_NORMAL, 0 },
    [NETTYPE_DESKTOP] =
	{ WM_DECOR_NONE, WM_FUNC_NONE, LAYER_DESKTOP,
	  TYPE_NO_FOCUS | TYPE_ALL_WORKSPACES | TYPE_POSITION },
    [NETTYPE_DOCK] =
	{ WM_DECOR_NONE, WM_FUNC_NONE, LAYER_DOCK,
	  TYPE_NO_FOCUS | TYPE_ALL_WORKSPACES | TYPE_POSITION },
    [NETTYPE_TOOLBAR] =
	{ TYPE_TRANSIENT, MWM_FUNC_MOVE | MWM_FUNC_CLOSE, LAYER_NORMAL,
	  TYPE_NO_FOCUS | TYPE_PSEUDO_TRANSIENT },
    [NETTYPE_MENU] =
	{ TYPE_TRANSIENT, MWM_FUNC_MOVE | MWM_FUNC_CLOSE, LAYER_NORMAL,
	  TYPE_NO_FOCUS | TYPE_PSEUDO_TRANSIENT },
    [NETTYPE_UTILITY] =
	{ TYPE_TRANSIENT, TYPE_TRANSIENT, LAYER_NORMAL,
	  TYPE_NO_FOCUS | TYPE_PSEUDO_TRANSIENT },
    [NETTYPE_SPLASH] =
	{ WM_DECOR_NONE, WM_FUNC_NONE, LAYER_NORMAL,
	  TYPE_NO_FOCUS | TYPE_CENTER },
    [NETTYPE_DIALOG] =
	{ TYPE_TRANSIENT, TYPE_TRANSIENT, LAYER_NORMAL,
	  TYPE_PSEUDO_TRANSIENT },
    [NETTYPE_DROPDOWN_MENU] =
	{ WM_DECOR_NONE, WM_FUNC_NONE, LAYER_ABOVE,
	  TYPE_NO_FOCUS | TYPE_POSITION },
    [NETTYPE_POPUP_MENU] =
	{ WM_DECOR_NONE, WM_FUNC_NONE, LAYER_ABOVE,
	  TYPE_NO_FOCUS | TYPE_POSITION },
    [NETTYPE_TOOLTIP] =
	{ WM_DECOR_NONE, WM_FUNC_NONE, LAYER_ABOVE,
	  TYPE_NO_FOCUS | TYPE_POSITION },
    [NETTYPE_NOTIFICATION] =
	{ WM_DECOR_NONE, MWM_FUNC_CLOSE, LAYER_ABOVE,
	  TYPE_NO_FOCUS | TYPE_STICKY | TYPE_POSITION },
    [NETTYPE_COMBO] =
	{ WM_DECOR_NONE, WM_FUNC_NONE, LAYER_ABOVE,
	  TYPE_NO_FOCUS | TYPE_POSITION },
    [NETTYPE_DND] =
	{ WM_DECOR_NONE, WM_FUNC_NONE, LAYER_ABOVE,
	  TYPE_NO_FOCUS | TYPE_POSITION },
};

#define NETSTATE_BIT(s)		(1u << NETSTATE_ ## s)
#define NETACT_BIT(a)		(1u << NETACT_ ## a)

/* Desktop windows and docks: not moved between desktops or layers. */
#define IS_DESKTOP_OR_DOCK(pCD) \
    ((pCD)->netType == NETTYPE_DESKTOP || (pCD)->netType == NETTYPE_DOCK)

/* The client whose _NET_WM_STATE_FOCUSED was last marked. */
static ClientData *netFocusClient;

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
    EwmhMarkClient (pCD, NET_CD_DESKTOP | NET_CD_STATE | NET_CD_ACTIONS |
			 NET_CD_EXTENTS);

    /* (while the initial property list is known) */
    ProcessNetWmStrut (pCD, True);
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

    if (netFocusClient == pCD)
	netFocusClient = NULL;

    /* not to be restored when the desktop is no longer shown */
    for (i = 0; i < pSD->numShowHidden; ++i)
    {
	if (pSD->pShowHidden[i] == pCD)
	{
	    memmove (&pSD->pShowHidden[i], &pSD->pShowHidden[i + 1],
		     (pSD->numShowHidden - i - 1) * sizeof (ClientData *));
	    --pSD->numShowHidden;
	    break;
	}
    }

    if (!(pCD->netFlags & NET_FLAG_LISTED)) return;
    pCD->netFlags &= ~NET_FLAG_LISTED;

    if (pCD->hasStrut)
    {
	pCD->hasStrut = False;
	--pSD->numStrutClients;
	EwmhMarkScreen (pSD, NET_SD_WORKAREA);
    }

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


/*
 * _NET_WM_STATE, as NETSTATE bits, from what dtwm knows of the client:
 * the state is dtwm's once the window is mapped, a client asks for
 * changes with messages.
 */
static unsigned int NetWmState (ClientData *pCD)
{
    unsigned int state = 0;
    int clientState = pCD->clientState & ~UNSEEN_STATE;

    if (pCD->maxConfig && pCD->fullscreen)
	state |= NETSTATE_BIT (FULLSCREEN);
    else if (pCD->maxConfig)
    {
	if (pCD->maxAxes & MAX_AXIS_VERT) state |= NETSTATE_BIT (MAXIMIZED_VERT);
	if (pCD->maxAxes & MAX_AXIS_HORZ) state |= NETSTATE_BIT (MAXIMIZED_HORZ);
    }
    if (pCD->netFlags & NET_FLAG_ABOVE) state |= NETSTATE_BIT (ABOVE);
    if (pCD->netFlags & NET_FLAG_BELOW) state |= NETSTATE_BIT (BELOW);
    if (NetWmDesktop (pCD) == NET_ALL_DESKTOPS) state |= NETSTATE_BIT (STICKY);
    if ((pCD->netFlags & NET_FLAG_MODAL) ||
	(pCD->inputMode != MWM_INPUT_MODELESS))
	state |= NETSTATE_BIT (MODAL);
    if (clientState == MINIMIZED_STATE) state |= NETSTATE_BIT (HIDDEN);
    if (pCD->netFlags & NET_FLAG_SKIP_TASKBAR)
	state |= NETSTATE_BIT (SKIP_TASKBAR);
    if (pCD->netFlags & NET_FLAG_SKIP_PAGER)
	state |= NETSTATE_BIT (SKIP_PAGER);
    if (pCD->netFlags & (NET_FLAG_URGENT_HINT | NET_FLAG_DEMANDS_ATTENTION))
	state |= NETSTATE_BIT (DEMANDS_ATTENTION);
    if (pCD == wmGD.keyboardFocus &&
	(clientState == NORMAL_STATE || clientState == MAXIMIZED_STATE))
	state |= NETSTATE_BIT (FOCUSED);

    return state;
}


/*
 * _NET_WM_ALLOWED_ACTIONS, as NETACT bits: what the client's functions
 * allow, and changing desktops and layers except for desktop windows and
 * docks (and desktops in mwm mode).  Maximizing allows full screen too.
 */
static unsigned int NetWmAllowedActions (ClientData *pCD)
{
    long functions = pCD->clientFunctions;
    unsigned int actions = 0;

    if (functions & MWM_FUNC_MOVE) actions |= NETACT_BIT (MOVE);
    if (functions & MWM_FUNC_RESIZE) actions |= NETACT_BIT (RESIZE);
    if (functions & MWM_FUNC_MINIMIZE) actions |= NETACT_BIT (MINIMIZE);
    if (functions & MWM_FUNC_MAXIMIZE)
	actions |= NETACT_BIT (MAXIMIZE_HORZ) | NETACT_BIT (MAXIMIZE_VERT) |
		   NETACT_BIT (FULLSCREEN);
    if (functions & MWM_FUNC_CLOSE) actions |= NETACT_BIT (CLOSE);

    if (IS_DESKTOP_OR_DOCK (pCD)) return actions;

    if (!wmGD.useStandardBehavior &&
	(pCD->dtwmFunctions & DtWM_FUNCTION_OCCUPY_WS))
	actions |= NETACT_BIT (CHANGE_DESKTOP) | NETACT_BIT (STICK);
    actions |= NETACT_BIT (ABOVE) | NETACT_BIT (BELOW);

    return actions;
}


/*
 * _NET_FRAME_EXTENTS (left, right, top, bottom): the offsets of the
 * client in its frame, the same on the sides and at the bottom; none for
 * a full screen window, which is shown without its frame.
 */
static void NetFrameExtents (ClientData *pCD, long extents[4])
{
    if (pCD->fullscreen && pCD->maxConfig)
    {
	extents[0] = extents[1] = extents[2] = extents[3] = 0;
    }
    else
    {
	extents[0] = extents[1] = extents[3] = pCD->clientOffset.x;
	extents[2] = pCD->clientOffset.y;
    }
}


/*
 * Client window properties: _NET_WM_DESKTOP, _NET_WM_STATE,
 * _NET_WM_ALLOWED_ACTIONS, _NET_FRAME_EXTENTS.
 */
static void FlushClient (ClientData *pCD)
{
    Atom atoms[NETSTATE_COUNT], actionAtoms[NETACT_COUNT];
    unsigned int state, actions;
    long desktop, extents[4];
    int i, num = 0;

    if (pCD->netDirty & NET_CD_DESKTOP)
    {
	/* sticky is being on all desktops */
	pCD->netDirty |= NET_CD_STATE;
	desktop = NetWmDesktop (pCD);
	if (desktop != pCD->netDesktopWritten)
	{
	    XChangeProperty (DISPLAY, pCD->client, wmGD.xa__NET_WM_DESKTOP,
		XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&desktop, 1);
	    pCD->netDesktopWritten = desktop;
	}
    }

    if (pCD->netDirty & NET_CD_STATE)
    {
	state = NetWmState (pCD);
	if (state != pCD->netStateWritten)
	{
	    for (i = 0; i < NETSTATE_COUNT; ++i)
		if (state & (1u << i))
		    atoms[num++] = wmGD.xa__NET_WM_STATE_[i];
	    XChangeProperty (DISPLAY, pCD->client, wmGD.xa__NET_WM_STATE,
		XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms, num);
	    pCD->netStateWritten = state;
	}
    }

    if (pCD->netDirty & NET_CD_ACTIONS)
    {
	actions = NetWmAllowedActions (pCD);
	if (actions != pCD->netActionsWritten)
	{
	    for (i = num = 0; i < NETACT_COUNT; ++i)
		if (actions & (1u << i))
		    actionAtoms[num++] = netActions[i];
	    XChangeProperty (DISPLAY, pCD->client, xa__NET_WM_ALLOWED_ACTIONS,
		XA_ATOM, 32, PropModeReplace, (unsigned char *)actionAtoms,
		num);
	    pCD->netActionsWritten = actions;
	}
    }

    /* (going full screen and back is a change of state) */
    if (pCD->netDirty & (NET_CD_EXTENTS | NET_CD_STATE))
    {
	NetFrameExtents (pCD, extents);
	if (memcmp (extents, pCD->netExtentsWritten, sizeof (extents)))
	{
	    XChangeProperty (DISPLAY, pCD->client, xa__NET_FRAME_EXTENTS,
		XA_CARDINAL, 32, PropModeReplace, (unsigned char *)extents, 4);
	    memcpy (pCD->netExtentsWritten, extents, sizeof (extents));
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


/*
 * Struts.  A strut is kept as _NET_WM_STRUT_PARTIAL says: the widths
 * reserved at the left, right, top and bottom edges of the root window,
 * then the ranges along those edges (inclusive) that they cover.
 */
enum {
    STRUT_LEFT, STRUT_RIGHT, STRUT_TOP, STRUT_BOTTOM,
    STRUT_LEFT_START, STRUT_LEFT_END, STRUT_RIGHT_START, STRUT_RIGHT_END,
    STRUT_TOP_START, STRUT_TOP_END, STRUT_BOTTOM_START, STRUT_BOTTOM_END
};


/*
 * The internal strut of the front panel (Dtwm*frontPanelStrut): along
 * the edge of its head that its frame touches.  Never published.
 */
static Boolean FrontPanelStrut (ClientData *pCD, long strut[12])
{
    WmScreenData *pSD = pCD->pSD;
    long rootWidth = pSD->netGeometry[0], rootHeight = pSD->netGeometry[1];
    long fx = pCD->frameInfo.x, fy = pCD->frameInfo.y;
    long fw = pCD->frameInfo.width, fh = pCD->frameInfo.height;
    long hx0 = 0, hy0 = 0, hx1 = rootWidth, hy1 = rootHeight;
    WmHeadInfo_t *pHead;
    int i;

    if (!wmGD.frontPanelStrut || !(pCD->clientFlags & FRONT_PANEL_BOX) ||
	(pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL))
	return False;

    if ((pHead = GetHeadInfo (pCD)))
    {
	hx0 = pHead->x_org;
	hy0 = pHead->y_org;
	hx1 = hx0 + pHead->width;
	hy1 = hy0 + pHead->height;
	free (pHead);
    }

    for (i = 0; i < 12; ++i) strut[i] = 0;
    if (fy + fh >= hy1)
    {
	strut[STRUT_BOTTOM] = rootHeight - fy;
	strut[STRUT_BOTTOM_START] = fx;
	strut[STRUT_BOTTOM_END] = fx + fw - 1;
    }
    else if (fy <= hy0)
    {
	strut[STRUT_TOP] = fy + fh;
	strut[STRUT_TOP_START] = fx;
	strut[STRUT_TOP_END] = fx + fw - 1;
    }
    else if (fx <= hx0)
    {
	strut[STRUT_LEFT] = fx + fw;
	strut[STRUT_LEFT_START] = fy;
	strut[STRUT_LEFT_END] = fy + fh - 1;
    }
    else if (fx + fw >= hx1)
    {
	strut[STRUT_RIGHT] = rootWidth - fx;
	strut[STRUT_RIGHT_START] = fy;
	strut[STRUT_RIGHT_END] = fy + fh - 1;
    }
    else
	return False;
    return True;
}


/* Whether [start, end] (inclusive) overlaps [from, to) */
#define OVERLAPS(start, end, from, to) \
    ((start) <= (end) && (start) < (to) && (end) >= (from))

/*
 * Shrink an area (x0, y0, x1, y1, exclusive) of a head by a strut: a side
 * moves when the strut covers part of the head along it and its inner
 * edge falls inside the head.  So a strut on the inner edge of a head
 * (between monitors) shrinks that head, not the one it extends over.
 */
static void ApplyStrut (WmScreenData *pSD, const long strut[12],
    const long head[4], long area[4])
{
    long edge;

    edge = strut[STRUT_LEFT];
    if (edge > 0 && edge > head[0] && edge < head[2] &&
	OVERLAPS (strut[STRUT_LEFT_START], strut[STRUT_LEFT_END],
		  head[1], head[3]) && edge > area[0])
	area[0] = edge;

    edge = pSD->netGeometry[0] - strut[STRUT_RIGHT];
    if (strut[STRUT_RIGHT] > 0 && edge > head[0] && edge < head[2] &&
	OVERLAPS (strut[STRUT_RIGHT_START], strut[STRUT_RIGHT_END],
		  head[1], head[3]) && edge < area[2])
	area[2] = edge;

    edge = strut[STRUT_TOP];
    if (edge > 0 && edge > head[1] && edge < head[3] &&
	OVERLAPS (strut[STRUT_TOP_START], strut[STRUT_TOP_END],
		  head[0], head[2]) && edge > area[1])
	area[1] = edge;

    edge = pSD->netGeometry[1] - strut[STRUT_BOTTOM];
    if (strut[STRUT_BOTTOM] > 0 && edge > head[1] && edge < head[3] &&
	OVERLAPS (strut[STRUT_BOTTOM_START], strut[STRUT_BOTTOM_END],
		  head[0], head[2]) && edge < area[3])
	area[3] = edge;
}


/*
 * The part of a head (x, y, width, height) that windows of a workspace
 * may use: without the struts of the clients in it that are shown
 * (normal or maximized), except pcdSelf's own.
 */
static void WorkspaceArea (WmWorkspaceData *pWS, ClientData *pcdSelf,
    long head[4], long area[4])
{
    long strut[12];
    int i, state;

    area[0] = head[0];
    area[1] = head[1];
    area[2] = head[2];
    area[3] = head[3];

    for (i = 0; i < pWS->numClients; ++i)
    {
	ClientData *pCD = pWS->ppClients[i];

	state = pCD->clientState & ~UNSEEN_STATE;
	if (pCD == pcdSelf ||
	    (state != NORMAL_STATE && state != MAXIMIZED_STATE))
	    continue;

	if (pCD->hasStrut)
	    ApplyStrut (pWS->pSD, pCD->strut, head, area);
	else if (FrontPanelStrut (pCD, strut))
	    ApplyStrut (pWS->pSD, strut, head, area);
    }
}


/*************************************<->*************************************
 *
 *  GetNetWorkArea (pCD, pX, pY, pWidth, pHeight)
 *
 *
 *  Description:
 *  -----------
 *  Reduces a head (or the screen) to the part of it that the struts leave
 *  to a client, in the active workspace if the client is in it, else in
 *  its first one: for maximizing (GetMaxInfo) and placing
 *  (FindClientPlacement) it.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = the client
 *  *pX, *pY, *pWidth, *pHeight = the head
 *
 *  Outputs:
 *  -------
 *  *pX, *pY, *pWidth, *pHeight = its work area
 *
 *************************************<->***********************************/

void GetNetWorkArea (ClientData *pCD, int *pX, int *pY, int *pWidth,
    int *pHeight)
{
    WmScreenData *pSD = pCD->pSD;
    WmWorkspaceData *pWS = pSD->pActiveWS;
    long head[4], area[4];

    if (!pSD->numStrutClients &&
	!(wmGD.frontPanelStrut && wmGD.useFrontPanel))
	return;

    if (!ClientInWorkspace (pWS, pCD) && pCD->numInhabited &&
	!(pWS = GetWorkspaceData (pSD, pCD->pWsList[0].wsID)))
	return;

    head[0] = *pX;
    head[1] = *pY;
    head[2] = *pX + *pWidth;
    head[3] = *pY + *pHeight;
    WorkspaceArea (pWS, pCD, head, area);
    *pX = area[0];
    *pY = area[1];
    *pWidth = area[2] - area[0];
    *pHeight = area[3] - area[1];
}


/*************************************<->*************************************
 *
 *  ProcessNetWmStrut (pCD, firstTime)
 *
 *
 *  Description:
 *  -----------
 *  Reads _NET_WM_STRUT_PARTIAL, or else _NET_WM_STRUT (the edges of the
 *  root window, all along them), of a listed client: when it is managed
 *  (only if it has either, HasProperty) and when either changes.
 *
 *************************************<->***********************************/

void ProcessNetWmStrut (ClientData *pCD, Boolean firstTime)
{
    WmScreenData *pSD = pCD->pSD;
    Atom actualType;
    int actualFormat, i;
    unsigned long nitems, leftover;
    long *data = NULL, strut[12];
    Boolean hasStrut = False;

    if (!(pCD->netFlags & NET_FLAG_LISTED) ||
	(firstTime && !HasProperty (pCD, wmGD.xa__NET_WM_STRUT_PARTIAL) &&
	 !HasProperty (pCD, wmGD.xa__NET_WM_STRUT)))
	return;

    if (XGetWindowProperty (DISPLAY, pCD->client,
	    wmGD.xa__NET_WM_STRUT_PARTIAL, 0L, 12L, False, XA_CARDINAL,
	    &actualType, &actualFormat, &nitems, &leftover,
	    (unsigned char **)&data) == Success &&
	actualType == XA_CARDINAL && actualFormat == 32 && nitems == 12)
    {
	for (i = 0; i < 12; ++i) strut[i] = data[i];
	hasStrut = True;
    }
    if (data) XFree (data);
    data = NULL;

    if (!hasStrut &&
	XGetWindowProperty (DISPLAY, pCD->client, wmGD.xa__NET_WM_STRUT,
	    0L, 4L, False, XA_CARDINAL, &actualType, &actualFormat, &nitems,
	    &leftover, (unsigned char **)&data) == Success &&
	actualType == XA_CARDINAL && actualFormat == 32 && nitems == 4)
    {
	for (i = 0; i < 4; ++i) strut[i] = data[i];
	for (i = 4; i < 12; i += 2)
	{
	    strut[i] = 0;
	    strut[i + 1] = LONG_MAX;
	}
	hasStrut = True;
    }
    if (data) XFree (data);

    if (hasStrut)
	for (i = 0; i < 4; ++i)
	    if (strut[i] < 0) strut[i] = 0;

    if (hasStrut == pCD->hasStrut &&
	(!hasStrut || !memcmp (strut, pCD->strut, sizeof (strut))))
	return;

    if (hasStrut != pCD->hasStrut)
	pSD->numStrutClients += hasStrut ? 1 : -1;
    pCD->hasStrut = hasStrut;
    if (hasStrut)
	memcpy (pCD->strut, strut, sizeof (strut));
    EwmhMarkScreen (pSD, NET_SD_WORKAREA);
}


/*
 * _NET_WORKAREA: for each desktop, the root window less the struts of
 * the clients in it.  A change of the work area refits the maximized
 * windows.
 */
static void FlushNetWorkArea (WmScreenData *pSD)
{
    int num = NumNetDesktops (pSD), d, j;
    long *areas, head[4];

    areas = (long *) XtMalloc (4 * num * sizeof (long));
    head[0] = head[1] = 0;
    head[2] = pSD->netGeometry[0];
    head[3] = pSD->netGeometry[1];
    for (d = 0; d < num; ++d)
    {
	long *area = &areas[4 * d];

	WorkspaceArea (&pSD->pWS[d], NULL, head, area);
	area[2] -= area[0];
	area[3] -= area[1];
    }

    /*
     * The maximized clients, to the work areas of their heads, first: a
     * client that sees the new work area sees them refitted.
     */
    for (d = 0; d < pSD->numWorkspaces; ++d)
	for (j = 0; j < pSD->pWS[d].numClients; ++j)
	    RefitMaximizedClient (pSD->pWS[d].ppClients[j]);

    if (num != pSD->numWorkAreaWritten ||
	memcmp (areas, pSD->workAreaWritten, 4 * num * sizeof (long)))
    {
	XChangeProperty (DISPLAY, pSD->rootWindow, xa__NET_WORKAREA,
	    XA_CARDINAL, 32, PropModeReplace, (unsigned char *)areas, 4 * num);
	XtFree ((char *)pSD->workAreaWritten);
	pSD->workAreaWritten = areas;
	pSD->numWorkAreaWritten = num;
    }
    else
	XtFree ((char *)areas);
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

    EwmhMarkScreen (pSD, NET_SD_DESKTOPS | NET_SD_NAMES | NET_SD_WORKAREA);
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
    EwmhMarkScreen (pSD, NET_SD_DESKTOPS | NET_SD_WORKAREA);
}


/* All the workspaces of the screen, for GetClientWorkspaceInfo. */
static Boolean AllWorkspaces (ClientData *pCD, WorkspaceID **ppIDs,
    unsigned int *pNumIDs, Boolean *pbAll)
{
    WmScreenData *pSD = pCD->pSD;
    int i;

    *ppIDs = (WorkspaceID *) XtMalloc (pSD->numWorkspaces *
				       sizeof (WorkspaceID));
    for (i = 0; i < pSD->numWorkspaces; ++i)
	(*ppIDs)[i] = pSD->pWS[i].id;
    *pNumIDs = pSD->numWorkspaces;
    *pbAll = True;
    return True;
}


/*************************************<->*************************************
 *
 *  GetNetWmTypeWorkspaces (pCD, ppIDs, pNumIDs, pbAll)
 *
 *
 *  Description:
 *  -----------
 *  Desktop windows and docks are in all workspaces, whatever else asks
 *  (GetClientWorkspaceInfo tries this first).
 *
 *
 *  Outputs:
 *  -------
 *  *ppIDs = the workspace IDs (XtMalloc'd), *pNumIDs = how many
 *  *pbAll = True
 *  RETURN = True if the window type puts the window in every workspace
 *
 *************************************<->***********************************/

Boolean GetNetWmTypeWorkspaces (ClientData *pCD, WorkspaceID **ppIDs,
    unsigned int *pNumIDs, Boolean *pbAll)
{
    if (wmGD.useStandardBehavior ||
	!(netTypes[pCD->netType].flags & TYPE_ALL_WORKSPACES))
	return False;

    return AllWorkspaces (pCD, ppIDs, pNumIDs, pbAll);
}


/*************************************<->*************************************
 *
 *  ProcessNetWmWindowType (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Reads _NET_WM_WINDOW_TYPE, once, when the window is managed (only
 *  when it has one, HasProperty): the first type that dtwm knows, else a
 *  dialog for a transient window and a normal window for the others.
 *  Sets what follows from the type before the steps of GetClientInfo
 *  that use it: the layer, focus when mapped, every workspace by
 *  default, and transient of its window group.  Decorations, functions
 *  (NetTypeDecoration, NetTypeFunctions), all workspaces
 *  (GetNetWmTypeWorkspaces) and placement (ApplyNetWmTypePlacement)
 *  come later.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = client being managed (after ProcessWmTransientFor)
 *
 *************************************<->***********************************/

/*
 * The type of a window: the first type in its _NET_WM_WINDOW_TYPE that
 * dtwm knows, else a dialog for a transient window and a normal window
 * for the others.
 */
static int ReadNetWmWindowType (Window window, Boolean transient)
{
    Atom actualType;
    int actualFormat, type = -1, t;
    unsigned long nitems, leftover, i;
    Atom *types = NULL;

    if (XGetWindowProperty (DISPLAY, window, xa__NET_WM_WINDOW_TYPE,
	    0L, 32L, False, XA_ATOM, &actualType, &actualFormat, &nitems,
	    &leftover, (unsigned char **)&types) == Success &&
	actualType == XA_ATOM && actualFormat == 32)
    {
	for (i = 0; i < nitems && type < 0; ++i)
	    for (t = 0; t < NETTYPE_COUNT; ++t)
		if (types[i] == wmGD.xa__NET_WM_WINDOW_TYPE_[t])
		{
		    type = t;
		    break;
		}
    }
    if (types) XFree (types);

    if (type < 0)
	type = transient ? NETTYPE_DIALOG : NETTYPE_NORMAL;
    return type;
}

void ProcessNetWmWindowType (ClientData *pCD)
{
    Boolean transient = (pCD->clientFlags & CLIENT_TRANSIENT) != 0;
    int type;

    if (HasProperty (pCD, xa__NET_WM_WINDOW_TYPE))
	type = ReadNetWmWindowType (pCD->client, transient);
    else
	type = transient ? NETTYPE_DIALOG : NETTYPE_NORMAL;

    pCD->netType = type;
    pCD->layer = netTypes[type].layer;
    if (netTypes[type].flags & TYPE_NO_FOCUS)
	pCD->netFlags |= NET_FLAG_NOFOCUS_MAP;
    if (netTypes[type].flags & TYPE_STICKY)
	pCD->netFlags |= NET_FLAG_INIT_STICKY;
    if ((netTypes[type].flags & TYPE_PSEUDO_TRANSIENT) &&
	!(pCD->clientFlags & CLIENT_TRANSIENT))
	pCD->bPseudoTransient = True;
}


/*
 * The default decorations and functions of the window type, for
 * ProcessMwmHints to use instead of dtwm's when no resource sets them.
 */
long NetTypeDecoration (ClientData *pCD, long decoration)
{
    int type = netTypes[pCD->netType].decoration;

    if (type == TYPE_UNCHANGED) return decoration;
    if (type == TYPE_TRANSIENT) return TRANSIENT_DECORATION (pCD);
    return type;
}

long NetTypeFunctions (ClientData *pCD, long functions)
{
    int type = netTypes[pCD->netType].functions;

    if (type == TYPE_UNCHANGED) return functions;
    if (type == TYPE_TRANSIENT) return TRANSIENT_FUNCTIONS (pCD);
    return type;
}


/*************************************<->*************************************
 *
 *  ApplyNetWmTypePlacement (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Desktop windows, docks, menus, tooltips and notifications go where
 *  they ask (as with a user specified position); a splash screen is
 *  centred on its head.  Called after WM_NORMAL_HINTS gave the geometry.
 *
 *************************************<->***********************************/

void ApplyNetWmTypePlacement (ClientData *pCD)
{
    int flags = netTypes[pCD->netType].flags;
    WmHeadInfo_t *pHead;

    if ((flags & TYPE_CENTER) && (pHead = GetHeadInfo (pCD)))
    {
	pCD->clientX = pHead->x_org + (pHead->width - pCD->clientWidth) / 2;
	pCD->clientY = pHead->y_org + (pHead->height - pCD->clientHeight) / 2;
	free (pHead);
    }
    if (flags & (TYPE_POSITION | TYPE_CENTER))
	pCD->sizeFlags |= US_POSITION;
}


/*************************************<->*************************************
 *
 *  GetNetWmDesktop (pCD, ppIDs, pNumIDs, pbAll)
 *
 *
 *  Description:
 *  -----------
 *  Reads the _NET_WM_DESKTOP that a client set before it was mapped, the
 *  workspace it asks to start in, or all of them (also when its initial
 *  _NET_WM_STATE says sticky).  It is read only when
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
    int actualFormat;
    unsigned long nitems, leftover, desktop;
    unsigned char *data = NULL;

    if (wmGD.useStandardBehavior)
	return False;

    /* _NET_WM_STATE_STICKY: all of them */
    if (pCD->netFlags & NET_FLAG_INIT_STICKY)
	desktop = NET_ALL_DESKTOPS;
    else
    {
	if (!HasProperty (pCD, wmGD.xa__NET_WM_DESKTOP) ||
	    XGetWindowProperty (DISPLAY, pCD->client, wmGD.xa__NET_WM_DESKTOP,
		0L, 1L, False, XA_CARDINAL, &actualType, &actualFormat,
		&nitems, &leftover, &data) != Success)
	    return False;

	if (actualType != XA_CARDINAL || actualFormat != 32 || nitems != 1)
	{
	    if (data) XFree (data);
	    return False;
	}
	desktop = *(unsigned long *)data & 0xFFFFFFFFUL;
	XFree (data);
    }

    if (desktop == NET_ALL_DESKTOPS)
	return AllWorkspaces (pCD, ppIDs, pNumIDs, pbAll);
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

    if (wmGD.useStandardBehavior || IS_DESKTOP_OR_DOCK (pCD) ||
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


/*
 * _NET_SHOWING_DESKTOP: dtwm shows the desktop the CDE way, by
 * iconifying the windows of the current workspace.
 */
static void FlushNetShowingDesktop (WmScreenData *pSD)
{
    long showing = pSD->showingDesktop ? 1 : 0;

    if (showing == pSD->netShowingWritten) return;

    XChangeProperty (DISPLAY, pSD->rootWindow, xa__NET_SHOWING_DESKTOP,
	XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&showing, 1);
    pSD->netShowingWritten = showing;
}


/*
 * Show the desktop: iconify the listed windows shown in the current
 * workspace that may be iconified (their transients go with them), not
 * desktop windows or docks, and note them.  Stop showing it: restore
 * those that are still iconified.
 */
static void ProcessNetShowingDesktop (WmScreenData *pSD, Boolean show)
{
    WmWorkspaceData *pWS = pSD->pActiveWS;
    ClientData **ppHidden, **ppClients, *pCD;
    int i, num;
    Time now;

    if (!show == !pSD->showingDesktop) return;
    now = GetTimestamp ();

    if (show)
    {
	/* (the workspace's list may change as windows are iconified) */
	num = pWS->numClients;
	ppClients = (ClientData **) XtMalloc ((num + 1) * sizeof (ClientData *));
	memcpy (ppClients, pWS->ppClients, num * sizeof (ClientData *));
	pSD->pShowHidden = (ClientData **) XtMalloc ((num + 1) *
						     sizeof (ClientData *));
	pSD->numShowHidden = 0;

	for (i = 0; i < num; ++i)
	{
	    pCD = ppClients[i];
	    if (pCD->transientLeader || IS_DESKTOP_OR_DOCK (pCD) ||
		!(pCD->netFlags & NET_FLAG_LISTED) ||
		!(pCD->clientFunctions & MWM_FUNC_MINIMIZE) ||
		(pCD->clientState != NORMAL_STATE &&
		 pCD->clientState != MAXIMIZED_STATE))
		continue;

	    SetClientState (pCD, MINIMIZED_STATE, now);
	    if (pCD->clientState == MINIMIZED_STATE)
		pSD->pShowHidden[pSD->numShowHidden++] = pCD;
	}
	XtFree ((char *)ppClients);
	pSD->showingDesktop = True;
    }
    else
    {
	ppHidden = pSD->pShowHidden;
	num = pSD->numShowHidden;
	pSD->pShowHidden = NULL;
	pSD->numShowHidden = 0;
	pSD->showingDesktop = False;

	for (i = 0; i < num; ++i)
	{
	    pCD = ppHidden[i];
	    if (pCD->clientState == MINIMIZED_STATE)
		SetClientState (pCD,
		    pCD->maxConfig ? MAXIMIZED_STATE : NORMAL_STATE, now);
	}
	XtFree ((char *)ppHidden);
    }

    EwmhMarkScreen (pSD, NET_SD_SHOWING);
}


/*************************************<->*************************************
 *
 *  EwmhLeaveShowingDesktop (pSD)
 *
 *
 *  Description:
 *  -----------
 *  The desktop is no longer shown once the workspace changes or a window
 *  is shown or activated: the windows iconified to show it stay so.
 *
 *************************************<->***********************************/

void EwmhLeaveShowingDesktop (WmScreenData *pSD)
{
    if (!pSD->showingDesktop) return;

    XtFree ((char *)pSD->pShowHidden);
    pSD->pShowHidden = NULL;
    pSD->numShowHidden = 0;
    pSD->showingDesktop = False;
    EwmhMarkScreen (pSD, NET_SD_SHOWING);
}


/*
 * A client is shown (SetClientState, normal or maximized): that ends
 * showing the desktop, unless it is a desktop window or a dock.
 */
void EwmhClientShown (ClientData *pCD)
{
    if (pCD->pSD->showingDesktop && (pCD->netFlags & NET_FLAG_LISTED) &&
	!IS_DESKTOP_OR_DOCK (pCD))
	EwmhLeaveShowingDesktop (pCD->pSD);
}


/*
 * Estimate the decorations of a window that is not managed yet, for
 * _NET_REQUEST_FRAME_EXTENTS: those that ProcessMwmHints would give it
 * from its type, transient or not, and _MOTIF_WM_HINTS, without the
 * client resources (their default).
 */
static void EstimateDecoration (ClientData *pCD)
{
    long functions, decoration;
    PropMwmHints *pHints;

    if (pCD->clientFlags & CLIENT_TRANSIENT)
    {
	functions = TRANSIENT_FUNCTIONS (pCD);
	decoration = TRANSIENT_DECORATION (pCD);
    }
    else
    {
	functions = WM_FUNC_ALL;
	decoration = WM_DECOR_ALL;
    }
    functions = NetTypeFunctions (pCD, functions);
    decoration = NetTypeDecoration (pCD, decoration);

    /* (the initial property list is not known: GetMwmHints reads it) */
    if ((pHints = GetMwmHints (pCD)) != NULL)
    {
	if (pHints->flags & MWM_HINTS_FUNCTIONS)
	{
	    if (pHints->functions == WM_FUNC_NONE)
		functions = WM_FUNC_NONE;
	    else if (pHints->functions == WM_FUNC_DEFAULT)
		functions = WM_FUNC_ALL;
	    else if (pHints->functions & WM_FUNC_DEFAULT)
		functions &= ~(pHints->functions);
	    else
		functions |= pHints->functions;
	}
	if (pHints->flags & MWM_HINTS_DECORATIONS)
	{
	    if (pHints->decorations == WM_DECOR_NONE)
		decoration = WM_DECOR_NONE;
	    else if (pHints->decorations == WM_DECOR_DEFAULT)
		decoration = WM_DECOR_ALL;
	    else if (pHints->decorations & WM_DECOR_DEFAULT)
		decoration &= ~(pHints->decorations);
	    else
		decoration |= pHints->decorations;

	    if (decoration &
		(MWM_DECOR_MENU | MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE))
		decoration |= MWM_DECOR_TITLE;
	    if (decoration & MWM_DECOR_RESIZEH)
		decoration |= MWM_DECOR_BORDER;
	}
	XFree ((char *)pHints);
    }

    if (!(functions & MWM_FUNC_RESIZE))
	decoration &= ~MWM_DECOR_RESIZEH;

    pCD->clientFunctions = functions;
    pCD->clientDecoration = pCD->decor = decoration;
}


/*************************************<->*************************************
 *
 *  HandleNetPrematureMessage (clientEvent)
 *
 *
 *  Description:
 *  -----------
 *  Handles an EWMH message about a window that is not managed yet, which
 *  other messages wait for (HandleEventsOnSpecialWindows keeps them):
 *  answers _NET_REQUEST_FRAME_EXTENTS with the _NET_FRAME_EXTENTS that
 *  the window would have, as far as can be told before managing it (one
 *  about a managed window is handled by HandleNetClientMessage), and
 *  drops _NET_WM_MOVERESIZE, which cannot be about a button pressed on
 *  the window.
 *
 *
 *  Inputs:
 *  ------
 *  clientEvent = a client message on a window that dtwm does not manage
 *
 *  Outputs:
 *  -------
 *  RETURN = True if it was handled, False if it is to wait
 *
 *************************************<->***********************************/

Boolean HandleNetPrematureMessage (XClientMessageEvent *clientEvent)
{
    WmScreenData *pSD;
    ClientData *pCD;
    Window root, leader;
    int x, y;
    unsigned int width, height, border, depth;
    long extents[4];

    if (clientEvent->message_type == wmGD.xa__NET_WM_MOVERESIZE)
	return True;
    if (clientEvent->message_type != xa__NET_REQUEST_FRAME_EXTENTS)
	return False;

    if (!XGetGeometry (DISPLAY, clientEvent->window, &root, &x, &y,
		       &width, &height, &border, &depth) ||
	XFindContext (DISPLAY, root, wmGD.screenContextType,
		      (caddr_t *)&pSD) ||
	!pSD->managed)
	return True;

    /* a client of which only what SetClientOffset uses is set */
    pCD = (ClientData *) XtCalloc (1, sizeof (ClientData));
    pCD->pSD = pSD;
    pCD->client = clientEvent->window;
    pCD->xBorderWidth = border;
    pCD->internalBevel = (wmGD.frameStyle == WmSLAB) ? 0 :
					    FRAME_INTERNAL_SHADOW_WIDTH;
    if (XGetTransientForHint (DISPLAY, pCD->client, &leader))
	pCD->clientFlags |= CLIENT_TRANSIENT;
    pCD->netType = ReadNetWmWindowType (pCD->client,
	(pCD->clientFlags & CLIENT_TRANSIENT) != 0);

    EstimateDecoration (pCD);
    SetClientOffset (pCD);
    NetFrameExtents (pCD, extents);
    XtFree ((char *)pCD);

    XChangeProperty (DISPLAY, clientEvent->window, xa__NET_FRAME_EXTENTS,
	XA_CARDINAL, 32, PropModeReplace, (unsigned char *)extents, 4);
    return True;
}


/* Gravity along an axis: where the reference point is. */
enum { GRAVITY_START, GRAVITY_CENTER, GRAVITY_END, GRAVITY_STATIC };

static int GravityAlongX (int gravity)
{
    switch (gravity)
    {
	case NorthGravity: case CenterGravity: case SouthGravity:
	    return GRAVITY_CENTER;
	case NorthEastGravity: case EastGravity: case SouthEastGravity:
	    return GRAVITY_END;
	case StaticGravity:
	    return GRAVITY_STATIC;
	default:
	    return GRAVITY_START;
    }
}

static int GravityAlongY (int gravity)
{
    switch (gravity)
    {
	case WestGravity: case CenterGravity: case EastGravity:
	    return GRAVITY_CENTER;
	case SouthWestGravity: case SouthGravity: case SouthEastGravity:
	    return GRAVITY_END;
	case StaticGravity:
	    return GRAVITY_STATIC;
	default:
	    return GRAVITY_START;
    }
}

/*
 * ICCCM window gravity along an axis: the frame is placed so that its
 * reference point is where that of the window (its size plus its border,
 * at pos) would be; with static gravity the client is where the inside
 * of the window would be.  The reference point of a window, that of the
 * frame of a client (at client in it, with offsets before and after),
 * and where the client goes for a reference point.
 */
static int WindowReference (int along, int pos, int size, int border)
{
    int outside = size + 2 * border;

    switch (along)
    {
	case GRAVITY_CENTER:	return pos + outside / 2;
	case GRAVITY_END:	return pos + outside;
	case GRAVITY_STATIC:	return pos + border;
	default:		return pos;
    }
}

static int FrameReference (int along, int client, int size, int before,
    int after)
{
    int start = client - before, frame = size + before + after;

    switch (along)
    {
	case GRAVITY_CENTER:	return start + frame / 2;
	case GRAVITY_END:	return start + frame;
	case GRAVITY_STATIC:	return client;
	default:		return start;
    }
}

static int ReferenceToClient (int along, int ref, int size, int before,
    int after)
{
    int frame = size + before + after;

    switch (along)
    {
	case GRAVITY_CENTER:	return ref - frame / 2 + before;
	case GRAVITY_END:	return ref - frame + before;
	case GRAVITY_STATIC:	return ref;
	default:		return ref + before;
    }
}


/*
 * _NET_MOVERESIZE_WINDOW: a ConfigureRequest of the values that the
 * flags give (bits 8 to 11: x, y, width, height).  With a gravity (bits
 * 0 to 7) the window is placed by it as ICCCM says, and a new size
 * without a position keeps the frame's reference point (a window with
 * south-east gravity grows to the left and up); without a gravity, the
 * values are taken as in a ConfigureRequest (positionIsFrame).
 */
static void ProcessNetMoveResizeWindow (ClientData *pCD, long *data)
{
    XConfigureRequestEvent request;
    long flags = data[0];
    int gravity = flags & 0xFF;
    int refX, refY, width, height, clientX, clientY, xOff, yOff;
    int left = pCD->clientOffset.x, top = pCD->clientOffset.y;
    int border = pCD->xBorderWidth;

    memset ((char *)&request, 0, sizeof (request));
    request.type = ConfigureRequest;
    request.display = DISPLAY;
    request.parent = pCD->clientBaseWin;
    request.window = pCD->client;
    if (flags & (1L << 8)) request.value_mask |= CWX;
    if (flags & (1L << 9)) request.value_mask |= CWY;
    if (flags & (1L << 10)) request.value_mask |= CWWidth;
    if (flags & (1L << 11)) request.value_mask |= CWHeight;
    request.x = data[1];
    request.y = data[2];
    request.width = data[3];
    request.height = data[4];

    if (!request.value_mask || gravity > StaticGravity) return;

    if (gravity)
    {
	int along = GravityAlongX (gravity), down = GravityAlongY (gravity);

	clientX = pCD->maxConfig ? pCD->maxX : pCD->clientX;
	clientY = pCD->maxConfig ? pCD->maxY : pCD->clientY;
	width = pCD->maxConfig ? pCD->maxWidth : pCD->clientWidth;
	height = pCD->maxConfig ? pCD->maxHeight : pCD->clientHeight;

	/* a position not given keeps the reference point of the frame */
	refX = FrameReference (along, clientX, width, left, left);
	refY = FrameReference (down, clientY, height, top, left);
	if (request.value_mask & (CWWidth | CWHeight))
	{
	    /* the size it will get */
	    unsigned int fixedWidth, fixedHeight;

	    fixedWidth = (request.value_mask & CWWidth) ? request.width : width;
	    fixedHeight = (request.value_mask & CWHeight) ?
		request.height : height;
	    FixWindowConfiguration (pCD, &fixedWidth, &fixedHeight,
		(unsigned int) pCD->widthInc, (unsigned int) pCD->heightInc);
	    width = fixedWidth;
	    height = fixedHeight;
	}
	if (request.value_mask & CWX)
	    refX = WindowReference (along, request.x, width, border);
	if (request.value_mask & CWY)
	    refY = WindowReference (down, request.y, height, border);

	clientX = ReferenceToClient (along, refX, width, left, left);
	clientY = ReferenceToClient (down, refY, height, top, left);

	/* in ConfigureRequest terms */
	xOff = wmGD.positionIsFrame ? left : 0;
	yOff = wmGD.positionIsFrame ? top : 0;
	request.x = clientX - xOff;
	request.y = clientY - yOff;
	request.value_mask |= CWX | CWY;
    }

    HandleCConfigureRequest (pCD, &request);
}


/* _NET_RESTACK_WINDOW: a ConfigureRequest to restack (in its layer). */
static void ProcessNetRestackWindow (ClientData *pCD, Window sibling,
    int detail)
{
    XConfigureRequestEvent request;

    if (detail < Above || detail > Opposite) return;

    memset ((char *)&request, 0, sizeof (request));
    request.type = ConfigureRequest;
    request.display = DISPLAY;
    request.parent = pCD->clientBaseWin;
    request.window = pCD->client;
    request.value_mask = CWStackMode | (sibling ? CWSibling : 0);
    request.above = sibling;
    request.detail = detail;

    HandleCConfigureRequest (pCD, &request);
}


/*************************************<->*************************************
 *
 *  HandleNetClientMessage (pCD, clientEvent)
 *
 *
 *  Description:
 *  -----------
 *  Handles the EWMH requests about a listed client that
 *  HandleClientMessage does not: _NET_CLOSE_WINDOW (as f.kill),
 *  _NET_MOVERESIZE_WINDOW, _NET_RESTACK_WINDOW, _NET_WM_MOVERESIZE and
 *  _NET_REQUEST_FRAME_EXTENTS.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = client
 *
 *  clientEvent = the client message
 *
 *************************************<->***********************************/

void HandleNetClientMessage (ClientData *pCD, XClientMessageEvent *clientEvent)
{
    Atom type = clientEvent->message_type;
    long *data = clientEvent->data.l;
    long extents[4];

    if (clientEvent->format != 32 || !(pCD->netFlags & NET_FLAG_LISTED))
	return;

    if (type == xa__NET_CLOSE_WINDOW)
    {
	F_Kill (NULL, pCD, NULL);
    }
    else if (type == xa__NET_MOVERESIZE_WINDOW)
    {
	ProcessNetMoveResizeWindow (pCD, data);
    }
    else if (type == xa__NET_RESTACK_WINDOW)
    {
	ProcessNetRestackWindow (pCD, (Window)data[1], (int)data[2]);
    }
    else if (type == wmGD.xa__NET_WM_MOVERESIZE)
    {
	/* (a cancel comes while the move or resize goes on) */
	if (data[2] != _NET_WM_MOVERESIZE_CANCEL)
	    HandleNetMoveResize (pCD, data[2], (int)data[0], (int)data[1],
				 (unsigned int)data[3]);
    }
    else if (type == xa__NET_REQUEST_FRAME_EXTENTS)
    {
	/* written again, for the client to see it */
	NetFrameExtents (pCD, extents);
	XChangeProperty (DISPLAY, pCD->client, xa__NET_FRAME_EXTENTS,
	    XA_CARDINAL, 32, PropModeReplace, (unsigned char *)extents, 4);
	memcpy (pCD->netExtentsWritten, extents, sizeof (extents));
    }
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
	if (bits & NET_SD_WORKAREA) FlushNetWorkArea (pSD);
	if (bits & NET_SD_SHOWING) FlushNetShowingDesktop (pSD);
    }

    return True;
}


/*
 * _NET_ACTIVE_WINDOW and _NET_WM_STATE_FOCUSED follow the keyboard
 * focus; a client that gets it no longer demands attention (unless its
 * urgency hint says so).
 */
void UpdateNetActiveWindow (void)
{
    int scr;

    for (scr = 0; scr < wmGD.numScreens; ++scr)
	if (wmGD.Screens[scr].managed)
	    EwmhMarkScreen (&wmGD.Screens[scr], NET_SD_ACTIVE);

    if (netFocusClient != wmGD.keyboardFocus)
    {
	if (netFocusClient)
	    EwmhMarkClient (netFocusClient, NET_CD_STATE);
	netFocusClient = wmGD.keyboardFocus;
	if (netFocusClient)
	{
	    netFocusClient->netFlags &= ~NET_FLAG_DEMANDS_ATTENTION;
	    EwmhMarkClient (netFocusClient, NET_CD_STATE);
	}
    }
}


/*************************************<->*************************************
 *
 *  UpdateNetUrgency (pCD, urgent)
 *
 *
 *  Description:
 *  -----------
 *  Takes the urgency hint of WM_HINTS, shown as
 *  _NET_WM_STATE_DEMANDS_ATTENTION.
 *
 *************************************<->***********************************/

void UpdateNetUrgency (ClientData *pCD, Boolean urgent)
{
    if (!urgent == !(pCD->netFlags & NET_FLAG_URGENT_HINT)) return;

    if (urgent)
	pCD->netFlags |= NET_FLAG_URGENT_HINT;
    else
	pCD->netFlags &= ~NET_FLAG_URGENT_HINT;
    EwmhMarkClient (pCD, NET_CD_STATE);
}


/*************************************<->*************************************
 *
 *  ReadInitialNetWmState (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Reads the _NET_WM_STATE that a client set before it was mapped (only
 *  when it has one, HasProperty).  dtwm keeps the flags that it only
 *  shows (skip taskbar and pager, demands attention, modal) and the
 *  layer (above, below; AddClientToList stacks it there), and notes
 *  sticky, maximized and full screen for the steps of GetClientInfo that
 *  apply them.  Hidden and focused are dtwm's, shaded is not supported.
 *
 *************************************<->***********************************/

void ReadInitialNetWmState (ClientData *pCD)
{
    static const struct
    {
	int state;
	unsigned int flag;
    } initial[] =
    {
	{ NETSTATE_SKIP_TASKBAR,	NET_FLAG_SKIP_TASKBAR },
	{ NETSTATE_SKIP_PAGER,		NET_FLAG_SKIP_PAGER },
	{ NETSTATE_DEMANDS_ATTENTION,	NET_FLAG_DEMANDS_ATTENTION },
	{ NETSTATE_MODAL,		NET_FLAG_MODAL },
	{ NETSTATE_ABOVE,		NET_FLAG_ABOVE },
	{ NETSTATE_BELOW,		NET_FLAG_BELOW },
	{ NETSTATE_STICKY,		NET_FLAG_INIT_STICKY },
	{ NETSTATE_MAXIMIZED_VERT,	NET_FLAG_INIT_MAX_VERT },
	{ NETSTATE_MAXIMIZED_HORZ,	NET_FLAG_INIT_MAX_HORZ },
	{ NETSTATE_FULLSCREEN,		NET_FLAG_INIT_FULLSCREEN },
    };
    Atom actualType;
    int actualFormat, j;
    unsigned long nitems, leftover, i;
    Atom *states = NULL;

    if (!HasProperty (pCD, wmGD.xa__NET_WM_STATE) ||
	XGetWindowProperty (DISPLAY, pCD->client, wmGD.xa__NET_WM_STATE,
	    0L, 64L, False, XA_ATOM, &actualType, &actualFormat, &nitems,
	    &leftover, (unsigned char **)&states) != Success)
	return;

    if (actualType == XA_ATOM && actualFormat == 32)
    {
	for (i = 0; i < nitems; ++i)
	    for (j = 0; j < XtNumber (initial); ++j)
		if (states[i] == wmGD.xa__NET_WM_STATE_[initial[j].state])
		    pCD->netFlags |= initial[j].flag;
    }
    if (states) XFree (states);
}


/*************************************<->*************************************
 *
 *  ApplyInitialNetWmState (pCD)
 *
 *
 *  Description:
 *  -----------
 *  A client that asked to start maximized (along one axis or both) or full
 *  screen starts so, unless it is to start iconic or its state comes from
 *  the session.  Called after WM_HINTS gave the initial state.
 *
 *************************************<->***********************************/

void ApplyInitialNetWmState (ClientData *pCD)
{
    unsigned int flags = pCD->netFlags;

    pCD->netFlags &= ~NET_FLAG_INIT;

    if (!(flags & (NET_FLAG_INIT_MAX_VERT | NET_FLAG_INIT_MAX_HORZ |
		   NET_FLAG_INIT_FULLSCREEN)) ||
	(pCD->clientFlags & SM_CLIENT_STATE) ||
	(pCD->clientState & ~UNSEEN_STATE) != NORMAL_STATE)
	return;

    if (flags & NET_FLAG_INIT_FULLSCREEN)
	pCD->fullscreen = True;
    else
	pCD->maxAxes = ((flags & NET_FLAG_INIT_MAX_VERT) ? MAX_AXIS_VERT : 0) |
		       ((flags & NET_FLAG_INIT_MAX_HORZ) ? MAX_AXIS_HORZ : 0);
    pCD->clientState = MAXIMIZED_STATE | (pCD->clientState & UNSEEN_STATE);
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

    if (clientEvent->format != 32) return;

    if (type == xa__NET_SHOWING_DESKTOP)
    {
	ProcessNetShowingDesktop (pSD, value != 0);
	return;
    }

    /* in mwm mode, there is one desktop */
    if (wmGD.useStandardBehavior) return;

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
    if (!IS_DESKTOP_OR_DOCK (pCD)) EwmhLeaveShowingDesktop (pCD->pSD);
    if (timestamp == CurrentTime) timestamp = GetTimestamp ();

    if (pLeader->clientState == MINIMIZED_STATE)
        SetClientState (pLeader,
            pLeader->maxConfig ? MAXIMIZED_STATE : NORMAL_STATE, timestamp);

    Do_Raise (pCD, NULL, STACK_NORMAL);
    Do_Focus_Key (pCD, timestamp, ALWAYS_SET_FOCUS);
}


/* Whether a state is to be set after a _NET_WM_STATE action. */
static Boolean NetStateAction (long action, Boolean set)
{
    switch (action)
    {
	case _NET_WM_STATE_REMOVE:	return False;
	case _NET_WM_STATE_ADD:		return True;
	case _NET_WM_STATE_TOGGLE:	return !set;
	default:			return set;
    }
}

/*
 * Maximize along axes (MAX_AXIS_*).  Both axes in one message are one
 * transition: toggling them unmaximizes a window maximized along either.
 * A full screen window ignores maximize requests.
 */
static void ProcessNetWmStateMaximized (ClientData *pCD, long action, int axes)
{
    int current, wanted;

    if (pCD->fullscreen) return;

    current = (pCD->clientState == MAXIMIZED_STATE) ? pCD->maxAxes : 0;
    if (axes == MAX_AXES_BOTH)
	wanted = NetStateAction (action, current != 0) ? MAX_AXES_BOTH : 0;
    else
	wanted = NetStateAction (action, (current & axes) != 0) ?
	    (current | axes) : (current & ~axes);
    if (wanted == current) return;

    /* NORMAL_STATE resets maxAxes */
    if (current)
	SetClientState (pCD, NORMAL_STATE, GetTimestamp ());
    if (wanted)
    {
	pCD->maxAxes = wanted;
	SetClientState (pCD, MAXIMIZED_STATE, GetTimestamp ());
    }
}

/* Sticky is on all desktops: the _NET_WM_DESKTOP request for all, or the
 * current one. */
static void ProcessNetWmStateSticky (ClientData *pCD, long action)
{
    WmScreenData *pSD = pCD->pSD;
    Boolean sticky = NetWmDesktop (pCD) == NET_ALL_DESKTOPS;

    if (NetStateAction (action, sticky) == sticky) return;

    ProcessNetWmDesktop (pCD, sticky ? (unsigned long)
			 (pSD->pActiveWS - pSD->pWS) : NET_ALL_DESKTOPS);
}

/* The states that dtwm only shows. */
static void ProcessNetWmStateFlag (ClientData *pCD, long action,
    unsigned int flag)
{
    unsigned int shown = flag;

    /* demanding attention shows the urgency hint too */
    if (flag == NET_FLAG_DEMANDS_ATTENTION)
	shown |= NET_FLAG_URGENT_HINT;

    if (NetStateAction (action, (pCD->netFlags & shown) != 0))
	pCD->netFlags |= flag;
    else
	pCD->netFlags &= ~flag;
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

    /* full screen with the focus is a layer of its own */
    RestackClientLayer (pCD);
}

/*
 * Above and below are layers around the normal one (ClientLayer).  The
 * window goes to the top of the layer above, the bottom of the layer
 * below, and back to the top (from above) or bottom (from below) of the
 * normal layer.
 */
static void ProcessNetWmStateAbove (ClientData *pCD, long action)
{
    Boolean above = (pCD->netFlags & NET_FLAG_ABOVE) != 0;

    if (NetStateAction (action, above) == above) return;

    if (above)
    {
	pCD->netFlags &= ~NET_FLAG_ABOVE;
	RestackClientLayer (pCD);
    }
    else
    {
	pCD->netFlags = (pCD->netFlags | NET_FLAG_ABOVE) & ~NET_FLAG_BELOW;
	Do_Raise (pCD, NULL, STACK_NORMAL);
    }
    EwmhMarkClient (pCD, NET_CD_STATE);
}

static void ProcessNetWmStateBelow (ClientData *pCD, long action)
{
    Boolean below = (pCD->netFlags & NET_FLAG_BELOW) != 0;

    if (NetStateAction (action, below) == below) return;

    if (below)
	pCD->netFlags &= ~NET_FLAG_BELOW;
    else
	pCD->netFlags = (pCD->netFlags | NET_FLAG_BELOW) & ~NET_FLAG_ABOVE;
    Do_Lower (pCD, NULL, STACK_NORMAL);
    EwmhMarkClient (pCD, NET_CD_STATE);
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

/*************************************<->*************************************
 *
 *  ProcessNetWmState (pCD, action, firstProperty, secondProperty)
 *
 *
 *  Description:
 *  -----------
 *  Handles a _NET_WM_STATE client message: each of the two states is
 *  changed on its own, except that maximizing vertically and
 *  horizontally in one message is one transition.  The states that
 *  change the window's geometry or stacking are ignored while it is not
 *  shown (in another workspace).  Hidden, focused and modal are dtwm's
 *  to say, shaded is not supported.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = client
 *
 *  action = _NET_WM_STATE_REMOVE, _ADD or _TOGGLE
 *
 *  firstProperty, secondProperty = the states (or None)
 *
 *************************************<->***********************************/

void ProcessNetWmState (ClientData *pCD, long action,
    Atom firstProperty, Atom secondProperty)
{
    Atom *states = wmGD.xa__NET_WM_STATE_;
    Atom property[2];
    Boolean unseen = (pCD->clientState & UNSEEN_STATE) != 0;
    int i, axes = 0;

    property[0] = firstProperty;
    property[1] = secondProperty;

    for (i = 0; i < 2; ++i)
    {
	Atom state = property[i];

	if (state == None || (i == 1 && state == property[0]))
	    continue;

	if (state == states[NETSTATE_MAXIMIZED_VERT])
	    axes |= MAX_AXIS_VERT;
	else if (state == states[NETSTATE_MAXIMIZED_HORZ])
	    axes |= MAX_AXIS_HORZ;
	else if (state == states[NETSTATE_FULLSCREEN])
	{
	    if (!unseen) ProcessNetWmStateFullscreen (pCD, action);
	}
	else if (state == states[NETSTATE_ABOVE])
	{
	    if (!unseen) ProcessNetWmStateAbove (pCD, action);
	}
	else if (state == states[NETSTATE_BELOW])
	{
	    if (!unseen) ProcessNetWmStateBelow (pCD, action);
	}
	else if (state == states[NETSTATE_STICKY])
	    ProcessNetWmStateSticky (pCD, action);
	else if (state == states[NETSTATE_SKIP_TASKBAR])
	    ProcessNetWmStateFlag (pCD, action, NET_FLAG_SKIP_TASKBAR);
	else if (state == states[NETSTATE_SKIP_PAGER])
	    ProcessNetWmStateFlag (pCD, action, NET_FLAG_SKIP_PAGER);
	else if (state == states[NETSTATE_DEMANDS_ATTENTION])
	    ProcessNetWmStateFlag (pCD, action, NET_FLAG_DEMANDS_ATTENTION);
    }

    if (axes && !unseen)
	ProcessNetWmStateMaximized (pCD, action, axes);

    if (!unseen && !ClientInWorkspace (ACTIVE_WS, pCD))
	SetClientState (pCD, pCD->clientState | UNSEEN_STATE, GetTimestamp ());

    EwmhMarkClient (pCD, NET_CD_STATE);
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
			     NET_SD_DESKTOPS | NET_SD_NAMES | NET_SD_WORKAREA |
			     NET_SD_SHOWING);
    }
}
