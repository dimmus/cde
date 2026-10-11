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
/*
 * ItemExtent.c: running maximum of the extents of the items of a
 * DtComboBox or DtSpinBox, and geometric growth of DtSpinBox's item
 * array.  Adding n items used to measure every item on each add
 * (O(n^2) XmStringExtent calls) and to grow the array by one each time.
 *
 * The state lives in an XContext keyed by the widget, so the
 * installed instance records keep their layout.
 */
#include <stdint.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include "DtWidgetI.h"
#include "ItemExtentI.h"

typedef struct {
    /* maxima of the extents of count items measured in font */
    XmFontList	font;
    int		count;		/* -1: nothing remembered */
    Dimension	longest, highest;
    int		nlongest, nhighest;	/* items that reach them */
    /* item array last grown for the widget, and its size */
    XmStringTable array;
    int		capacity;
} ItemState;

static XContext itemContext = 0;

static ItemState *
Lookup(Widget w, Boolean create)
{
    Display *dpy = XtDisplayOfObject(w);
    XPointer data;
    ItemState *s;

    _DtProcessLock();
    if (itemContext == 0)
	itemContext = XUniqueContext();
    _DtProcessUnlock();

    if (XFindContext(dpy, (XID)(uintptr_t)w, itemContext, &data) == 0)
	return (ItemState *)data;
    if (!create)
	return NULL;

    s = (ItemState *)XtMalloc(sizeof(ItemState));
    memset(s, 0, sizeof(ItemState));
    s->count = -1;
    if (XSaveContext(dpy, (XID)(uintptr_t)w, itemContext, (XPointer)s) != 0) {
	XtFree((char *)s);
	return NULL;
    }
    return s;
}

static void
MeasureAll(ItemState *s, XmFontList font, XmStringTable items, int count)
{
    Dimension width, height;
    int i;

    s->longest = s->highest = 0;
    s->nlongest = s->nhighest = 0;
    for (i = 0; i < count; i++) {
	XmStringExtent(font, items[i], &width, &height);
	if (width > s->longest) {
	    s->longest = width;
	    s->nlongest = 1;
	} else if (width == s->longest)
	    s->nlongest++;
	if (height > s->highest) {
	    s->highest = height;
	    s->nhighest = 1;
	} else if (height == s->highest)
	    s->nhighest++;
    }
    s->font = font;
    s->count = count;
}

/* Account for one item of the given size going; False to remeasure. */
static Boolean
Remove(Dimension size, Dimension *max, int *nmax)
{
    if (size > *max)
	return False;
    if (size == *max && --*nmax == 0)
	return False;
    return True;
}

void
_DtItemExtentsMax(Widget w, XmFontList font, XmStringTable items, int count,
		  const _DtItemChange *change,
		  Dimension *longest, Dimension *highest)
{
    ItemState *s = Lookup(w, True);
    ItemState tmp;
    Boolean done = False;

    if (s == NULL) {
	memset(&tmp, 0, sizeof(tmp));
	s = &tmp;
    }
    else if (change && s->count >= 0 && s->font == font) {
	Dimension width, height;

	switch (change->type) {
	case _DtITEMS_ADDED:
	    if (count != s->count + 1 || change->item == NULL)
		break;
	    XmStringExtent(font, change->item, &width, &height);
	    if (width > s->longest) {
		s->longest = width;
		s->nlongest = 1;
	    } else if (width == s->longest)
		s->nlongest++;
	    if (height > s->highest) {
		s->highest = height;
		s->nhighest = 1;
	    } else if (height == s->highest)
		s->nhighest++;
	    s->count = count;
	    done = True;
	    break;
	case _DtITEMS_REMOVED:
	    if (count != s->count - 1)
		break;
	    if (Remove(change->width, &s->longest, &s->nlongest) &&
		Remove(change->height, &s->highest, &s->nhighest)) {
		s->count = count;
		done = True;
	    }
	    break;
	default:
	    break;
	}
    }

    if (!done)
	MeasureAll(s, font, items, count);

    *longest = s->longest;
    *highest = s->highest;
}

void
_DtItemExtentsReset(Widget w)
{
    ItemState *s = Lookup(w, False);

    if (s)
	s->count = -1;
}

XmStringTable
_DtItemArrayGrow(Widget w, XmStringTable items, int count)
{
    ItemState *s = Lookup(w, True);
    int capacity;

    if (s == NULL)
	return (XmStringTable)XtRealloc((char *)items,
					sizeof(XmString) * count);

    if (items != NULL && items == s->array && count <= s->capacity)
	return items;

    capacity = (count <= 4) ? count : count + count / 2;
    if (items != NULL && items == s->array && capacity < 2 * s->capacity)
	capacity = 2 * s->capacity;

    s->array = (XmStringTable)XtRealloc((char *)items,
					sizeof(XmString) * capacity);
    s->capacity = capacity;
    return s->array;
}

void
_DtItemArraySet(Widget w, XmStringTable items, int count)
{
    ItemState *s = Lookup(w, items != NULL);

    if (s) {
	s->array = items;
	s->capacity = items ? count : 0;
    }
}

XmStringTable
_DtItemArrayShrink(Widget w, XmStringTable items, int count)
{
    ItemState *s = Lookup(w, False);

    if (count <= 0) {
	XtFree((char *)items);
	if (s && s->array == items) {
	    s->array = NULL;
	    s->capacity = 0;
	}
	return NULL;
    }
    if (s && items != NULL && items == s->array)
	return items;
    return (XmStringTable)XtRealloc((char *)items, sizeof(XmString) * count);
}

void
_DtItemExtentsDestroy(Widget w)
{
    ItemState *s = Lookup(w, False);

    if (s) {
	XDeleteContext(XtDisplayOfObject(w), (XID)(uintptr_t)w, itemContext);
	XtFree((char *)s);
    }
}
