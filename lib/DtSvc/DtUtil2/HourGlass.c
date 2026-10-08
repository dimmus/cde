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
/* $XConsortium: HourGlass.c /main/5 1996/06/21 17:22:09 ageorge $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        HourGlass.c
 **
 **   Project:     dt Dt Utility function
 **
 **   Description: This module contains a simple function for
 **                creating an hourglass cursor.
 **
 **   (c) Copyright 1987, 1988, 1989 by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

/*****************************************************************************
 *
 * (c) Copyright 1989,1990 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY
 * ALL RIGHTS RESERVED
 *
 * 	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
 * AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
 * WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
 * ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 * AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 * SOFTWARE IS HEREBY TRANSFERRED.
 *
 * 	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 * NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
 * FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS
 *
 * 	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
 * ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
 * SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Notice:  Notwithstanding any other lease or license that may pertain to,
 * or accompany the delivery of, this computer software, the rights of the
 * Government regarding its use, reproduction and disclosure are as set
 * forth in Section 52.227-19 of the FARS Computer Software-Restricted
 * Rights clause.
 *
 * (c) Copyright 1989,1990 Open Software Foundation, Inc.  Unpublished - all
 * rights reserved under the Copyright laws of the United States.
 *
 * RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
 * Government is subject to the restrictions as set forth in subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
 * at DFARS 52.227-7013.
 *
 * Open Software Foundation, Inc.
 * 11 Cambridge Center
 * Cambridge, MA   02142
 * (617)621-8700
 *
 * RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
 * "restricted rights."  Use, duplication or disclosure is subject to the
 * restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
 * "Commercial Computer Software- Restricted Rights (April 1985)."  Open
 * Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
 * the contract contains the Clause at 18-52.227-74 "Rights in Data General"
 * then the "Alternate III" clause applies.
 *
 * (c) Copyright 1989,1990 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED
 *
 *
 * Open Software Foundation is a trademark of The Open Software  
 * Foundation, Inc.
 *
 * OSF is a trademark of Open Software Foundation, Inc.
 * OSF/Motif is a trademark of Open Software Foundation, Inc.
 * Motif is a trademark of Open Software Foundation, Inc.
 * DEC is a registered trademark of Digital Equipment Corporation
 * DIGITAL is a registered trademark of Digital Equipment Corporation
 * X Window System is a trademark of the Massachusetts Institute of Technology
 *
 *****************************************************************************
 *************************************<+>*************************************/


#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include "DtSvcLock.h"

#include <X11/bitmaps/xm_hour16>
#include <X11/bitmaps/xm_hour16m>
#include <X11/bitmaps/xm_hour32>
#include <X11/bitmaps/xm_hour32m>

/********    Public Function Declarations    ********/

extern Cursor _DtGetHourGlassCursor( 
                        Display *dpy) ;
extern void _DtTurnOnHourGlass( 
                        Widget w) ;
extern void _DtTurnOffHourGlass( 
                        Widget w) ;

/********    End Public Function Declarations    ********/

/*************************************<->*************************************
 *
 *  Cursor _DtGetHourGlassCursor ()
 *
 *
 *  Description:
 *  -----------
 *  Builds and returns the appropriate Hourglass cursor
 *
 *
 *  Inputs:
 *  ------
 *  dpy	= display
 * 
 *  Outputs:
 *  -------
 *  Return = cursor.
 *
 *  Comments:
 *  --------
 *  None. (None doesn't count as a comment)
 * 
 *************************************<->***********************************/
Cursor 
_DtGetHourGlassCursor(
        Display *dpy )
{
    char *bits;
    char *maskBits;
    unsigned int width;
    unsigned int height;
    unsigned int xHotspot;
    unsigned int yHotspot;
    Pixmap       pixmap;
    Pixmap       maskPixmap;
    XColor       xcolors[2];
    int          scr;
    unsigned int cWidth;
    unsigned int cHeight;
    int		 useLargeCursors = 0;
    static Cursor waitCursor=0;

    _DtSvcProcessLock();
    if (waitCursor != 0) {
      _DtSvcProcessUnlock();
      return(waitCursor);
    }

    if (XQueryBestCursor (dpy, DefaultRootWindow(dpy), 
	32, 32, &cWidth, &cHeight))
    {
	if ((cWidth >= 32) && (cHeight >= 32))
	{
	    useLargeCursors = 1;
	}
    }

    if (useLargeCursors)
    {
	width = hour32_width;
	height = hour32_height;
	bits = hour32_bits;
	maskBits = hour32m_bits;
	xHotspot = hour32_x_hot;
	yHotspot = hour32_y_hot;
    }
    else
    {
	width = hour16_width;
	height = hour16_height;
	bits = hour16_bits;
	maskBits = hour16m_bits;
	xHotspot = hour16_x_hot;
	yHotspot = hour16_y_hot;
    }

    pixmap = XCreateBitmapFromData (dpy, 
		     DefaultRootWindow(dpy), (char*) bits, 
		     width, height);

  
    maskPixmap = XCreateBitmapFromData (dpy, 
		     DefaultRootWindow(dpy), (char*) maskBits, 
		     width, height);

    xcolors[0].pixel = BlackPixelOfScreen(DefaultScreenOfDisplay(dpy));
    xcolors[1].pixel = WhitePixelOfScreen(DefaultScreenOfDisplay(dpy));

    XQueryColors (dpy, 
		  DefaultColormapOfScreen(DefaultScreenOfDisplay
					  (dpy)), xcolors, 2);

    waitCursor = XCreatePixmapCursor (dpy, pixmap, maskPixmap,
				      &(xcolors[0]), &(xcolors[1]),
				      xHotspot, yHotspot);
    XFreePixmap (dpy, pixmap);
    XFreePixmap (dpy, maskPixmap);

    _DtSvcProcessUnlock();
    return (waitCursor);
}

 
/*************************************<->*************************************
 *
 *  void DtSetHourGlass
 *
 *
 *  Description:
 *  -----------
 *  sets the window cursor to an hourglass
 *
 *
 *  Inputs:
 *  ------
 *  w   = Widget
 *
 *  Outputs:
 *  -------
 *  None
 *
 *  Comments:
 *  --------
 *  None. (None doesn't count as a comment)
 *
 *************************************<->***********************************/

void 
_DtTurnOnHourGlass(
        Widget w )
{
    Cursor cursor;
    
    cursor = _DtGetHourGlassCursor(XtDisplay(w));

    XDefineCursor(XtDisplay(w), XtWindow(w), cursor);
    XFlush(XtDisplay(w));
}


/*************************************<->*************************************
 *
 *  void _DtTurnOffHourGlass
 *
 *
 *  Description:
 *  -----------
 *  Removed the hourglass cursor from a window
 *
 *
 *  Inputs:
 *  ------
 *  w = Widget
 *
 *  Outputs:
 *  -------
 *  None
 *
 *  Comments:
 *  --------
 *  None. (None doesn't count as a comment)
 *
 *************************************<->***********************************/

void 
_DtTurnOffHourGlass(
        Widget w )
{
   
    XUndefineCursor(XtDisplay(w), XtWindow(w));
    XFlush(XtDisplay(w));
}

