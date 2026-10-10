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
 * ItemExtentI.h: running maximum of the extents of a widget's list of
 * XmStrings (DtComboBox, DtSpinBox), kept beside the widget so that
 * adding or deleting one item measures one item instead of all of
 * them.  Internal to libDtWidget; the widget instance records are
 * installed ABI and are left alone.
 */
#ifndef _DtItemExtentI_h
#define _DtItemExtentI_h

#include <Xm/Xm.h>

/* What changed since the last call for this widget. */
typedef enum {
    _DtITEMS_RESET,	/* anything: measure every item */
    _DtITEMS_ADDED,	/* one item added (item) */
    _DtITEMS_REMOVED	/* one item removed (width, height measured
			   with the same font before it went) */
} _DtItemChangeType;

typedef struct {
    _DtItemChangeType	type;
    XmString		item;
    Dimension		width, height;
} _DtItemChange;

/*
 * Largest width and height of items[0 .. count-1] in font, as the
 * loop over all items would give.  change says how the items differ
 * from the last call; it is only a hint: whenever the remembered
 * state does not match (another font, another count), every item is
 * measured.
 */
extern void _DtItemExtentsMax(Widget w, XmFontList font,
			      XmStringTable items, int count,
			      const _DtItemChange *change,
			      Dimension *longest, Dimension *highest);

/* Forget the remembered maxima (the items are not what is measured). */
extern void _DtItemExtentsReset(Widget w);

/*
 * Grow an item array that w owns to hold count items.  Room is added
 * geometrically; the array is only reallocated when it is not the one
 * last returned for (or registered with _DtItemArraySet for) w or when
 * that one is full.
 */
extern XmStringTable _DtItemArrayGrow(Widget w, XmStringTable items,
				      int count);

/*
 * Register items, an array of exactly count entries that w now owns
 * (whenever the widget allocates a new item array by other means).
 */
extern void _DtItemArraySet(Widget w, XmStringTable items, int count);

/*
 * Shrink an item array that w owns to count items: freed (NULL) at
 * zero, kept while it is the array _DtItemArrayGrow returned, and
 * reallocated to fit otherwise.
 */
extern XmStringTable _DtItemArrayShrink(Widget w, XmStringTable items,
					int count);

/* Free everything kept for w (call from the destroy method). */
extern void _DtItemExtentsDestroy(Widget w);

#endif /* _DtItemExtentI_h */
