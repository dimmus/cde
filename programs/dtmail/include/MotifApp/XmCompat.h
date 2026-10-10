/*
 * CDE - Common Desktop Environment
 *
 * This file is part of CDE.
 *
 * CDE is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * CDE is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with CDE. If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Replacements for the Motif calls that the Motif headers mark
 * deprecated.  Each one returns what the deprecated call returned.
 */

#ifndef _XMCOMPAT_H
#define _XMCOMPAT_H

#include <string.h>
#include <Xm/Xm.h>
#include <Xm/List.h>

/*
 * XmMessageBoxGetChild, XmSelectionBoxGetChild and
 * XmFileSelectionBoxGetChild for XmDIALOG_OK_BUTTON, XmDIALOG_HELP_BUTTON,
 * XmDIALOG_TEXT and XmDIALOG_SELECTION_LABEL.  The boxes create these
 * children, named "OK", "Help", "Text" and "Selection", before any child
 * the application adds, and XtNameToWidget() returns the first child
 * with the name.  (Only a template dialog may lack its buttons; dtmail
 * makes none.)
 */
inline Widget
XmCompatOkButton(Widget box)
{
    return XtNameToWidget(box, "OK");
}

inline Widget
XmCompatHelpButton(Widget box)
{
    return XtNameToWidget(box, "Help");
}

inline Widget
XmCompatSelectionText(Widget box)
{
    return XtNameToWidget(box, "Text");
}

inline Widget
XmCompatSelectionLabel(Widget box)
{
    return XtNameToWidget(box, "Selection");
}

/*
 * XmDIALOG_CANCEL_BUTTON: the cancel button is the BulletinBoard's
 * XmNcancelButton.
 */
inline Widget
XmCompatCancelButton(Widget box)
{
    Widget w = NULL;

    XtVaGetValues(box, XmNcancelButton, &w, NULL);
    return w;
}

/*
 * XmListGetSelectedPos: an XtMalloc'ed copy of XmNselectedPositions,
 * which the caller frees, and True; or NULL, 0 and False if no item is
 * selected.
 */
inline Boolean
XmCompatListGetSelectedPos(Widget list, int **pos_list, int *pos_count)
{
    int *positions = NULL;
    int count = 0;
    int items = 0;

    *pos_list = NULL;
    *pos_count = 0;
    XtVaGetValues(list,
		  XmNselectedPositions, &positions,
		  XmNselectedPositionCount, &count,
		  XmNitemCount, &items,
		  NULL);
    if (items <= 0 || positions == NULL || count <= 0)
	return False;

    *pos_list = (int *) XtMalloc(count * sizeof(int));
    memcpy(*pos_list, positions, count * sizeof(int));
    *pos_count = count;
    return True;
}

#endif /* _XMCOMPAT_H */
