/*
 * CDE - Common Desktop Environment
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
 * Font lists that may hold scalable (Xft) fonts.
 *
 * With the Style Manager's Scalable (Xft) font type, the render tables
 * of Motif widgets hold Xft renditions (XmFONT_IS_XFT), whose font is an
 * XftFont, not an XFontStruct or XFontSet: code that measures or draws
 * with the core fonts of a font list must not take it for one, and
 * should measure with XmStringExtent and draw with XmStringDraw
 * instead.  These helpers, in libDtSvc, are the one place for that.
 */
#ifndef _DtFontListP_h
#define _DtFontListP_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Whether fontList has an Xft rendition (False for NULL). */
extern Boolean _DtFontListHasXft (XmFontList fontList);

#ifdef __cplusplus
}
#endif

#endif /* _DtFontListP_h */
