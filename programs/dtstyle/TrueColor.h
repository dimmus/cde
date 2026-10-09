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
#ifndef _dtstyle_TrueColor_h
#define _dtstyle_TrueColor_h

#include <X11/Xlib.h>

/* StyleAllocColor() results */
#define STYLE_COLOR_ALLOCATED   1   /* from XAllocColor: XFreeColors it */
#define STYLE_COLOR_COMPUTED    2   /* TrueColor, computed: never free it */

/* Like XAllocColor (0 on failure), but on a TrueColor visual the pixel is
 * computed locally once that has been checked against the server. */
extern int StyleAllocColor(Display *dpy, Colormap cmap, Visual *visual,
                           XColor *color);

#endif /* _dtstyle_TrueColor_h */
