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
/* $XConsortium: font.c /main/4 1995/11/09 12:59:38 rswiston $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "calendar.h"

#ifndef ABS
#define ABS(x) (((x) > 0) ? (x) : (-(x)))
#endif /* ABS */

/*
 * Walk a font_list looking for a FontSet with the 
 * XmFONTLIST_DEFAULT_TAG set.  If we fail to find a FontSet with this tag,
 * return the first FontSet found.  If we fail to find a FontSet return
 * the first font found.
 *
 * This function returns either a XFontStruct or a XFontSet.  The type can
 * be determined by the value of type_return which will equal either
 * XmFONT_IS_FONTSET or XmFONT_IS_FONT.
 *
 * The XFontStruct or XFontSet that is returned is not a copy and should
 * not be freed.
 */
static XtPointer
get_font(
	XmFontList	 font_list,
	XmFontType	*type_return)
{
	XmFontContext	 fl_context;
	XmFontListEntry	 fl_entry;
	XtPointer	 fl_entry_font,
			 font_to_use = NULL;
	XmFontType	 entry_type,
			 type_to_use = XmFONT_IS_FONT;
	char		*fl_entry_font_tag;
	Boolean		 found_font_set = False,
			 found_font_struct = False;

	*type_return = XmFONT_IS_FONT;

	if (!XmFontListInitFontContext(&fl_context, font_list))
		return (XtPointer)NULL;

	do {
		fl_entry = XmFontListNextEntry(fl_context);
		if (fl_entry) {
			fl_entry_font = 
				XmFontListEntryGetFont(fl_entry, &entry_type);
			/*
			 * The font of an Xft entry is an XftFont, neither
			 * an XFontStruct nor an XFontSet: skip it.  Render
			 * tables with Xft go through rendertable_to_font().
			 */
			if (entry_type == XmFONT_IS_XFT)
				continue;
			if (entry_type == XmFONT_IS_FONTSET) {
				fl_entry_font_tag = 
						XmFontListEntryGetTag(fl_entry);
					/* 
					 * Save the first font set found in-
					 * case the default tag is not set.
					 */
				if (!found_font_set) {
					font_to_use = fl_entry_font;
					type_to_use = entry_type;
					found_font_set = True;
					found_font_struct = True;

					if (!strcmp(XmFONTLIST_DEFAULT_TAG,
						    fl_entry_font_tag))
						break;
				} else if (!strcmp(XmFONTLIST_DEFAULT_TAG, 
						    fl_entry_font_tag)) {
					/* Found right font set */
					font_to_use = fl_entry_font;
					type_to_use = entry_type;
					break;
				}
			} else if (!found_font_struct) {
				font_to_use = fl_entry_font;
				type_to_use = entry_type;
				found_font_struct = True;
			}
		}
	} while (fl_entry != NULL);

	XmFontListFreeFontContext(fl_context);

	if (!found_font_set && !found_font_struct)
		return (XtPointer)NULL;

	*type_return = type_to_use;
	return (XtPointer)font_to_use;
}

extern Boolean
fontlist_to_font(
	XmFontList	 font_list,
	Cal_Font	*cal_font)
{
	XmFontType	 type_return;
	XtPointer	 font_data;

	if (!font_list) return False;

	font_data = get_font(font_list, &type_return);

	if (!font_data) 
		return False;

	cal_font->cf_type = type_return;

	if (type_return == XmFONT_IS_FONTSET)
		cal_font->f.cf_fontset = (XFontSet)font_data;
	else
		cal_font->f.cf_font = (XFontStruct *)font_data;

	return True;
}

/*
 * Make an XmString of the first nbytes of string, tagged so that it is
 * drawn with the rendition of the Xft font.
 */
static XmString
xft_string(
	Cal_Font	*font,
	char		*string,
	int		 nbytes)
{
	char		 buf[256],
			*text = buf;
	XmString	 xms;

	if (nbytes < 0)
		nbytes = 0;
	if (nbytes >= sizeof(buf))
		text = XtMalloc(nbytes + 1);
	memcpy(text, string, nbytes);
	text[nbytes] = '\0';

	xms = XmStringCreate(text, font->f.cf_xft.tag);

	if (text != buf)
		XtFree(text);
	return xms;
}

static int
xft_width(
	Cal_Font	*font,
	char		*string,
	int		 nbytes)
{
	XmString	 xms = xft_string(font, string, nbytes);
	int		 width = XmStringWidth(font->f.cf_xft.rt, xms);

	XmStringFree(xms);
	return width;
}

/*
 * Fill in cal_font from a render table that holds the one Xft rendition
 * tagged tag.  Both become the font's.  The metrics come from the
 * XmString functions: ascent and height are the font's, whatever the
 * text, and the views take the width of a ``W'' for the width of a
 * character.
 */
static Boolean
xft_font(
	XmRenderTable	 rt,
	XmStringTag	 tag,
	Cal_Font	*cal_font)
{
	XmString	 xms;
	Cal_Font	 font;

	font.cf_type = XmFONT_IS_XFT;
	font.f.cf_xft.rt = rt;
	font.f.cf_xft.tag = tag;

	xms = XmStringCreate("W", tag);
	font.f.cf_xft.ascent = XmStringBaseline(rt, xms);
	font.f.cf_xft.height = XmStringHeight(rt, xms);
	font.f.cf_xft.char_width = XmStringWidth(rt, xms);
	XmStringFree(xms);

	/* No height: the font could not be opened. */
	if (font.f.cf_xft.height <= 0) {
		XmRenderTableFree(rt);
		XtFree(tag);
		return False;
	}

	*cal_font = font;
	return True;
}

/*
 * Return a copy of the tag of the first Xft rendition of rt, or NULL if
 * it has none.
 */
static XmStringTag
xft_tag(
	XmRenderTable	 rt)
{
	XmStringTag	*tags,
			 found = NULL;
	XmRendition	 rend;
	XmFontType	 type;
	Arg		 args[1];
	int		 i, ntags;

	ntags = XmRenderTableGetTags(rt, &tags);
	for (i = 0; i < ntags; i++) {
		if (!found && (rend = XmRenderTableGetRendition(rt, tags[i]))) {
			type = XmFONT_IS_FONT;
			XtSetArg(args[0], XmNfontType, &type);
			XmRenditionRetrieve(rend, args, 1);
			XmRenditionFree(rend);
			if (type == XmFONT_IS_XFT)
				found = XtNewString(tags[i]);
		}
		XtFree(tags[i]);
	}
	if (ntags > 0)
		XtFree((char *)tags);

	return found;
}

/*
 * Like fontlist_to_font() for a render table, but a render table with an
 * Xft rendition gives an Xft font.  This is how the systemRenderTable
 * and userRenderTable resources, the Xft companions of systemFont and
 * userFont, reach the views.
 */
extern Boolean
rendertable_to_font(
	XmRenderTable	 rt,
	Cal_Font	*cal_font)
{
	XmStringTag	 tag;

	if (!rt) return False;

	if (!(tag = xft_tag(rt)))
		return fontlist_to_font(rt, cal_font);

	return xft_font(XmRenderTableCopy(rt, &tag, 1), tag, cal_font);
}

/*
 * Make the bold variant of an Xft font: the same family, size, foundry
 * and encoding with a bold style.  This fails for a font that is not an
 * Xft font or whose rendition names no family.
 */
extern Boolean
bold_xft_font(
	Widget		 w,
	Cal_Font	*regular,
	Cal_Font	*bold_return)
{
	XmRendition	 rend,
			 bold;
	XmRenderTable	 rt;
	String		 name = NULL,
			 style = NULL,
			 foundry = NULL,
			 encoding = NULL;
	char		 bold_style[128];
	int		 size = 0;
	Arg		 args[8];
	Cardinal	 n;

	if (regular->cf_type != XmFONT_IS_XFT)
		return False;

	if (!(rend = XmRenderTableGetRendition(regular->f.cf_xft.rt,
					       regular->f.cf_xft.tag)))
		return False;

	n = 0;
	XtSetArg(args[n], XmNfontName, &name);		n++;
	XtSetArg(args[n], XmNfontStyle, &style);	n++;
	XtSetArg(args[n], XmNfontSize, &size);		n++;
	XtSetArg(args[n], XmNfontFoundry, &foundry);	n++;
	XtSetArg(args[n], XmNfontEncoding, &encoding);	n++;
	XmRenditionRetrieve(rend, args, n);

	if (!name || name == (String)XmAS_IS || !*name) {
		XmRenditionFree(rend);
		return False;
	}

	/* Book, Regular, ... become Bold; Oblique becomes Bold Oblique. */
	if (!style || !*style || !strcasecmp(style, "Book") ||
	    !strcasecmp(style, "Regular") || !strcasecmp(style, "Normal") ||
	    !strcasecmp(style, "Medium") || !strcasecmp(style, "Roman"))
		strcpy(bold_style, "Bold");
	else if (strstr(style, "Bold"))
		snprintf(bold_style, sizeof(bold_style), "%s", style);
	else
		snprintf(bold_style, sizeof(bold_style), "Bold %s", style);

	/*
	 * A rendition keeps the strings it is given, it does not copy
	 * them all, so give it its own.  The fonts live as long as dtcm.
	 */
	n = 0;
	XtSetArg(args[n], XmNfontType, XmFONT_IS_XFT);	n++;
	XtSetArg(args[n], XmNfontName, XtNewString(name)); n++;
	XtSetArg(args[n], XmNfontStyle, XtNewString(bold_style)); n++;
	if (size > 0) {
		XtSetArg(args[n], XmNfontSize, size);	n++;
	}
	if (foundry) {
		XtSetArg(args[n], XmNfontFoundry, XtNewString(foundry)); n++;
	}
	if (encoding) {
		XtSetArg(args[n], XmNfontEncoding, XtNewString(encoding)); n++;
	}
	bold = XmRenditionCreate(w, regular->f.cf_xft.tag, args, n);
	XmRenditionFree(rend);

	rt = XmRenderTableAddRenditions(NULL, &bold, 1, XmMERGE_REPLACE);
	XmRenditionFree(bold);

	return xft_font(rt, XtNewString(regular->f.cf_xft.tag), bold_return);
}

/*
 * Look for a font that is closest to the requested point size and resolution
 * using the weight and family name specified.
 */
static Boolean
ProbeForFont(
	Calendar	*cal, 
	Font_Weight	 weight, 
	unsigned long	 target_pixel_size, 
	int		 with_foundry,
	char	        *font_name)
{
	char	       **font_names,
			 notused[48];
	int		 i, 
			 nnames,
			 size = 0,
			 closest_size_diff = 100000,
			 closest_size,
			 closest_index = -1;

	if (!strcmp(cal->app_data->app_font_family, "application")) {
		sprintf (font_name, "-%s-%s-%s-r-normal-sans-*-*-*-*-p-*",
				(with_foundry)?"dt":"*",
				cal->app_data->app_font_family,
				(weight == BOLD)?"bold":"medium");
	} else {
		sprintf (font_name, "-%s-%s-%s-r-normal--*-*-*-*-*-*",
				(with_foundry)?"dt":"*",
				cal->app_data->app_font_family,
				(weight == BOLD)?"bold":"medium");
	}

	/* See if the font exists */
	font_names = XListFonts(XtDisplay(cal->frame), font_name, 80, &nnames);
	if (!nnames) {
		if (with_foundry)
			return ProbeForFont(cal, weight, target_pixel_size, 
							     FALSE, font_name);
		else
			return FALSE;
	}

	/* For the fonts that match, get their pixel size and 
	 * look for the one with the pixel size closest to the size we are 
	 * looking for.
	 */
	for (i = 0; i < nnames; i++) {
		sscanf(font_names[i], 
		       "-%[^-]-%[^-]-%[^-]-%[^-]-%[^-]-%[^-]-%d-%s",
			notused, notused, notused, notused, notused, notused,
			&size, notused);

		if (!size)
			sscanf(font_names[i], 
			    "-%[^-]-%[^-]-%[^-]-%[^-]-%[^-]--%d-%s",
			     notused, notused, notused, notused, notused,
			     &size, notused);

		if (size) {
			if ((ABS((int)(target_pixel_size - size)) <= 
							closest_size_diff)) {
				closest_size_diff = 
					ABS((int)(target_pixel_size - size));
				closest_size = size;
				closest_index = i;
				if (closest_size_diff == 0)
					break;
			}
			size = 0;
		}
	}

	if (closest_index == -1)
		return FALSE;

	/* This one is the closest in size */
	sprintf (font_name, "-%s-%s-%s-r-normal-%s-%d-*-*-*-*-*",
		 (with_foundry)?"dt":"*",
		 cal->app_data->app_font_family,
		 (weight == BOLD)?"bold":"medium",
		 (!strcmp(cal->app_data->app_font_family, "application")) ?
								"sans": "",
		 closest_size);
		
	XFreeFontNames(font_names);

	return TRUE;
}

extern void
load_app_font(
	Calendar	*cal, 
	Font_Weight	 weight,
	Cal_Font	*userfont,
	Cal_Font	*return_font)
{
	unsigned long	 pixel_size;
	Display		*dpy = XtDisplay(cal->frame);
	char		 font_name[128],
			*font_name_ptr = font_name,
		       **font_names;
	int		 nnames;
	XrmValue	 in_font;
	XrmValue	 out_fontlist;
	XmFontList	 font_list;
	Atom		 pixel_atom = XmInternAtom(dpy, "PIXEL_SIZE", FALSE);

	/* First get the pixel size from the User Font */
	if (userfont->cf_type == XmFONT_IS_FONT) {
		/* If we can't get the pixel size from the user font we
		 * defaults to a 12 pixel font.
		 */
		if (!XGetFontProperty(userfont->f.cf_font, pixel_atom, 
							&pixel_size))
			pixel_size = 12;
	} else if (userfont->cf_type == XmFONT_IS_FONTSET) {
		XFontStruct 	**font_struct_list;
		char		**font_name_list;
		int		  list_size;

		if (!(list_size = XFontsOfFontSet(userfont->f.cf_fontset,
					    &font_struct_list,
					    &font_name_list))) {
			pixel_size = 12;
		} else {
			if (!XGetFontProperty(font_struct_list[0],
					      pixel_atom, 
					      &pixel_size))
				pixel_size = 12;
		}
	} else
		pixel_size = 12;

	/* If the font family is not ``application'' then it is probably
	 * multibyte so we can't assume there is an add-style or a proportional
	 * version available.
	 */
	if (!strcmp(cal->app_data->app_font_family, "application")) {
		sprintf (font_name, "-dt-%s-%s-r-normal-sans-%lu-*-*-*-p-*",
				cal->app_data->app_font_family,
				(weight == BOLD)?"bold":"medium",
				pixel_size);
	} else {
		sprintf (font_name, "-dt-%s-%s-r-normal--%lu-*-*-*-*-*",
				cal->app_data->app_font_family,
				(weight == BOLD)?"bold":"medium",
				pixel_size);
	}

	/* See if the font exists */
	font_names = XListFonts(dpy, font_name, 1, &nnames);
	if (!nnames) {
		if (!ProbeForFont(cal, weight, pixel_size, TRUE, 
							font_name_ptr)) {
			/* We didn't find anything */
			*return_font = *userfont;
			return;
		}
	} else
		XFreeFontNames(font_names);

	strcat(font_name, ":");
	in_font.size = strlen(font_name);
	in_font.addr = font_name;
	out_fontlist.size = sizeof(XmFontList);
	out_fontlist.addr = (XPointer)&font_list;

	/* Convert font string to a font list. */
	if (!XtConvertAndStore(cal->frame, XtRString, &in_font, XmRFontList,
				&out_fontlist)) {
		*return_font = *userfont;
	}

	if (!fontlist_to_font(font_list, return_font)) {
		*return_font = *userfont;
	}
}

/*
 * Given a font or fontset, return the extents of the text.
 */
void
CalTextExtents(
	Cal_Font	*font,
	char		*string,
	int		 nbytes,
	int		*x_return,
	int		*y_return,
	int		*width_return,
	int		*height_return)
{
	
	if (!font) return;

	if (font->cf_type == XmFONT_IS_FONT) {
		int		direction,
				ascent,
				descent;
		XCharStruct	overall;

		XTextExtents(font->f.cf_font, string, nbytes,
				&direction, &ascent, &descent, &overall);

		*x_return = overall.lbearing;
		*y_return = overall.ascent;
		*width_return = overall.width;
		*height_return = overall.ascent + overall.descent;
	} else if (font->cf_type == XmFONT_IS_XFT) {
		/* The logical extents, as for a font set. */
		*x_return = 0;
		*y_return = - font->f.cf_xft.ascent;
		*width_return = xft_width(font, string, nbytes);
		*height_return = font->f.cf_xft.height;
	} else {
		XRectangle	ink,
				logical;

		XmbTextExtents(font->f.cf_fontset, string, nbytes,
				&ink, &logical);

		*x_return = logical.x;
		*y_return = logical.y;
		*width_return = logical.width;
		*height_return = logical.height;
	}
}

/*
 * Given a font or fontset, render the text.  y is the baseline.
 *
 * clip is the clip of gc, or NULL when gc is not clipped to one
 * rectangle.  Xft text is not clipped by the GC: without it a repaint
 * of an exposed area would draw the text around that area again, over
 * itself, and the antialiased edges would grow darker.
 */
void
CalDrawString(
	Display		*dpy,
	Drawable	 draw,
	Cal_Font	*font,
	GC		 gc,
	int		 x,
	int		 y,
	char		*string,
	int		 length,
	XRectangle	*clip)
{

	if (font->cf_type == XmFONT_IS_FONT) {
		XSetFont(dpy, gc, font->f.cf_font->fid);
		XDrawString(dpy, draw, gc, x, y, string, length);
	} else if (font->cf_type == XmFONT_IS_XFT) {
		XmString	xms = xft_string(font, string, length);
		Dimension	width = XmStringWidth(font->f.cf_xft.rt, xms);
		int		top = y - font->f.cf_xft.ascent;

		/*
		 * XmStringDraw draws a line it does not find in the clip
		 * unclipped, so leave out such text here.
		 */
		if (!clip || (x < clip->x + (int)clip->width &&
			      clip->x < x + (int)width &&
			      top < clip->y + (int)clip->height &&
			      clip->y < top + font->f.cf_xft.height)) {
			XmStringDraw(dpy, draw, font->f.cf_xft.rt, xms, gc,
				     x, top, width, XmALIGNMENT_BEGINNING,
				     XmSTRING_DIRECTION_L_TO_R, clip);
			/* XmStringDraw leaves gc unclipped: put it back. */
			if (clip)
				XSetClipRectangles(dpy, gc, 0, 0, clip, 1,
						   Unsorted);
		}
		XmStringFree(xms);
	} else {
		XmbDrawString(dpy, draw, font->f.cf_fontset, gc, x, y, 
							string, length);
	}
}

void
CalFontExtents(
	Cal_Font	*font,
	XFontSetExtents *fse)
{
	if (font->cf_type == XmFONT_IS_FONT) {
		/*
		 * These computations are stolen from _XsimlCreateFontSet()
		 * in Xlib.
		 */
		fse->max_ink_extent.x = font->f.cf_font->min_bounds.lbearing;
		fse->max_ink_extent.y = - font->f.cf_font->max_bounds.ascent;
		fse->max_ink_extent.width = font->f.cf_font->min_bounds.width;
		fse->max_ink_extent.height = 
					 font->f.cf_font->max_bounds.ascent +
					 font->f.cf_font->max_bounds.descent;

		fse->max_logical_extent.x = 0;
		fse->max_logical_extent.y = - font->f.cf_font->ascent;
		fse->max_logical_extent.width = 
					 font->f.cf_font->max_bounds.width;
		fse->max_logical_extent.height = 
					 font->f.cf_font->ascent +
					 font->f.cf_font->descent;
	} else if (font->cf_type == XmFONT_IS_XFT) {
		/* The ink extents are not known: use the logical ones. */
		fse->max_logical_extent.x = 0;
		fse->max_logical_extent.y = - font->f.cf_xft.ascent;
		fse->max_logical_extent.width = font->f.cf_xft.char_width;
		fse->max_logical_extent.height = font->f.cf_xft.height;
		fse->max_ink_extent = fse->max_logical_extent;
	} else {
		*fse = *XExtentsOfFontSet(font->f.cf_fontset);
	}
}
