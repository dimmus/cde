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

#ifndef WMEWMH_H
#define WMEWMH_H

/*
 * What EwmhFlushProc has to recompute and write, when it differs from
 * what it last wrote.  Root window properties, per screen (pSD->netDirty,
 * EwmhMarkScreen):
 */
#define NET_SD_CLIENTS		(1u << 0)	/* _NET_CLIENT_LIST */
#define NET_SD_STACKING		(1u << 1)	/* _NET_CLIENT_LIST_STACKING */
#define NET_SD_DESKTOPS		(1u << 2)	/* _NET_NUMBER_OF_DESKTOPS, */
						/* _NET_DESKTOP_VIEWPORT, */
						/* _NET_DESKTOP_GEOMETRY */
#define NET_SD_NAMES		(1u << 3)	/* _NET_DESKTOP_NAMES */
#define NET_SD_WORKAREA		(1u << 4)	/* _NET_WORKAREA */
#define NET_SD_ACTIVE		(1u << 5)	/* _NET_ACTIVE_WINDOW */
#define NET_SD_SHOWING		(1u << 6)	/* _NET_SHOWING_DESKTOP */

/* Client window properties, per client (pCD->netDirty, EwmhMarkClient): */
#define NET_CD_STATE		(1u << 0)	/* _NET_WM_STATE */
#define NET_CD_DESKTOP		(1u << 1)	/* _NET_WM_DESKTOP */
#define NET_CD_ACTIONS		(1u << 2)	/* _NET_WM_ALLOWED_ACTIONS */
#define NET_CD_EXTENTS		(1u << 3)	/* _NET_FRAME_EXTENTS */

/* An empty Window cache: no XID has its top three bits set. */
#define NET_NOT_WRITTEN		((Window) ~0UL)

/* _NET_WM_DESKTOP: all desktops, and an empty cache */
#define NET_ALL_DESKTOPS	0xFFFFFFFFL
#define NET_DESKTOP_NOT_WRITTEN	(-2L)

/* An empty _NET_WM_STATE cache (not a set of NETSTATE bits). */
#define NET_STATE_NOT_WRITTEN	(~0u)

/* pCD->netFlags: */
#define NET_FLAG_LISTED		(1u << 0)	/* in pSD->netClients */
#define NET_FLAG_SKIP_TASKBAR	(1u << 1)	/* _NET_WM_STATE flags kept */
#define NET_FLAG_SKIP_PAGER	(1u << 2)	/* for the client */
#define NET_FLAG_DEMANDS_ATTENTION (1u << 3)	/* requested by the client */
#define NET_FLAG_MODAL		(1u << 4)
#define NET_FLAG_URGENT_HINT	(1u << 5)	/* WM_HINTS XUrgencyHint */
#define NET_FLAG_INIT_STICKY	(1u << 6)	/* the _NET_WM_STATE the */
#define NET_FLAG_INIT_MAX_VERT	(1u << 7)	/* client set before it */
#define NET_FLAG_INIT_MAX_HORZ	(1u << 8)	/* was mapped, until */
#define NET_FLAG_INIT_FULLSCREEN (1u << 9)	/* GetClientInfo is done */
#define NET_FLAG_INIT		(NET_FLAG_INIT_STICKY | \
				 NET_FLAG_INIT_MAX_VERT | \
				 NET_FLAG_INIT_MAX_HORZ | \
				 NET_FLAG_INIT_FULLSCREEN)

void EwmhMarkScreen (WmScreenData *pSD, unsigned int bits);
void EwmhMarkClient (ClientData *pCD, unsigned int bits);
void EwmhAddClient (ClientData *pCD);
void EwmhRemoveClient (ClientData *pCD);
void HandleNetRootMessage (WmScreenData *pSD,
    XClientMessageEvent *clientEvent);
void UpdateNetCurrentDesktop (WmScreenData *pSD);
void EwmhWorkspacesChanged (WmScreenData *pSD);
void EwmhRootResized (WmScreenData *pSD, int width, int height);
Boolean GetNetWmDesktop (ClientData *pCD, WorkspaceID **ppIDs,
    unsigned int *pNumIDs, Boolean *pbAll);
void ProcessNetWmDesktop (ClientData *pCD, unsigned long desktop);
void ReadInitialNetWmState (ClientData *pCD);
void ApplyInitialNetWmState (ClientData *pCD);
void UpdateNetUrgency (ClientData *pCD, Boolean urgent);
void ProcessNetWmFullscreenMonitors (ClientData *pCD,
    int top, int bottom, int left, int right);
void ProcessNetWmState (ClientData *pCD, long action,
    Atom firstProperty, Atom secondProperty);
void ProcessNetActiveWindow (ClientData *pCD, Time timestamp);
void UpdateNetActiveWindow (void);
void SetupWmEwmh (void);

#endif
