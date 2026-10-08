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

#include "WmGlobal.h"
#include "WmEvent.h"
#include "WmFunction.h"
#include "WmMultiHead.h"
#include "WmProperty.h"
#include "WmWinState.h"
#include "WmWrkspace.h"

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

    if (firstProperty  == wmGD.xa__NET_WM_STATE_MAXIMIZED_VERT &&
	secondProperty == wmGD.xa__NET_WM_STATE_MAXIMIZED_HORZ ||
	firstProperty  == wmGD.xa__NET_WM_STATE_MAXIMIZED_HORZ &&
	secondProperty == wmGD.xa__NET_WM_STATE_MAXIMIZED_VERT)
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

/**
* @brief Sets up the window manager handling of the EWMH.
*/
void SetupWmEwmh (void)
{
    int scr;

    enum {
	XA_UTF8_STRING,
	XA__NET_SUPPORTED,
	XA__NET_SUPPORTING_WM_CHECK,
	XA__NET_WM_NAME,
	XA__NET_WM_ICON_NAME,
	XA__NET_WM_VISIBLE_NAME,
	XA__NET_WM_VISIBLE_ICON_NAME,
	XA__NET_WM_FULLSCREEN_MONITORS,
	XA__NET_WM_STATE,
	XA__NET_WM_STATE_MAXIMIZED_VERT,
	XA__NET_WM_STATE_MAXIMIZED_HORZ,
	XA__NET_WM_STATE_FULLSCREEN,
	XA__NET_WM_STATE_ABOVE,
	XA__NET_WM_STATE_BELOW
    };

    static char *atom_names[] = {
	"UTF8_STRING",
	_XA__NET_SUPPORTED,
	_XA__NET_SUPPORTING_WM_CHECK,
	_XA__NET_WM_NAME,
	_XA__NET_WM_ICON_NAME,
	_XA__NET_WM_VISIBLE_NAME,
	_XA__NET_WM_VISIBLE_ICON_NAME,
	_XA__NET_WM_FULLSCREEN_MONITORS,
	_XA__NET_WM_STATE,
	_XA__NET_WM_STATE_MAXIMIZED_VERT,
	_XA__NET_WM_STATE_MAXIMIZED_HORZ,
	_XA__NET_WM_STATE_FULLSCREEN,
	_XA__NET_WM_STATE_ABOVE,
	_XA__NET_WM_STATE_BELOW
    };

    Atom atoms[XtNumber(atom_names) + 1];

    XInternAtoms(DISPLAY, atom_names, XtNumber(atom_names), False, atoms);

    wmGD.xa_UTF8_STRING = atoms[XA_UTF8_STRING];
    wmGD.xa__NET_WM_NAME = atoms[XA__NET_WM_NAME];
    wmGD.xa__NET_WM_ICON_NAME = atoms[XA__NET_WM_ICON_NAME];
    wmGD.xa__NET_WM_VISIBLE_NAME = atoms[XA__NET_WM_VISIBLE_NAME];
    wmGD.xa__NET_WM_VISIBLE_ICON_NAME = atoms[XA__NET_WM_VISIBLE_ICON_NAME];
    wmGD.xa__NET_WM_FULLSCREEN_MONITORS = atoms[XA__NET_WM_FULLSCREEN_MONITORS];
    wmGD.xa__NET_WM_STATE = atoms[XA__NET_WM_STATE];
    wmGD.xa__NET_WM_STATE_MAXIMIZED_VERT =
	atoms[XA__NET_WM_STATE_MAXIMIZED_VERT];
    wmGD.xa__NET_WM_STATE_MAXIMIZED_HORZ =
	atoms[XA__NET_WM_STATE_MAXIMIZED_HORZ];
    wmGD.xa__NET_WM_STATE_FULLSCREEN = atoms[XA__NET_WM_STATE_FULLSCREEN];
    wmGD.xa__NET_WM_STATE_ABOVE = atoms[XA__NET_WM_STATE_ABOVE];
    wmGD.xa__NET_WM_STATE_BELOW = atoms[XA__NET_WM_STATE_BELOW];

    for (scr = 0; scr < wmGD.numScreens; ++scr)
    {
	Window childWindow;
	WmScreenData *pSD;

	pSD = &(wmGD.Screens[scr]);

	if (!pSD->managed) continue;

	childWindow = pSD->wmWorkspaceWin;

	XChangeProperty(DISPLAY, childWindow, atoms[XA__NET_WM_NAME],
			atoms[XA_UTF8_STRING], 8, PropModeReplace,
			(unsigned char *)DT_WM_RESOURCE_NAME, 5);

	XChangeProperty(DISPLAY, childWindow,
			atoms[XA__NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&childWindow, 1);

	XChangeProperty(DISPLAY, wmGD.Screens[scr].rootWindow,
			atoms[XA__NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&childWindow, 1);

	XChangeProperty(DISPLAY, wmGD.Screens[scr].rootWindow,
			atoms[XA__NET_SUPPORTED], XA_ATOM, 32, PropModeReplace,
			(unsigned char *)&atoms[XA__NET_SUPPORTING_WM_CHECK],
			12);
    }
}
