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

#pragma once

/* image.c */
int Mirror_Image(int orientation);
int Block_Rotate(XImage *src_image, XImage *dst_image, int rtype);
void Scale_Image(void);
int Flood_Region(int flood_x, int flood_y);
void Set_FloodLimits(int x, int y);
int Flood_Fill(XImage *color_image, XImage *mono_image, int x, int y, int width, int height, unsigned long new_pixel, unsigned long new_mono);
