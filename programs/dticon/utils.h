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

/* utils.c */
void Create_Gfx_Labels(unsigned long fg, unsigned long bg);
void Init_Editor(Widget wid);
void GetMarginData(void);
void New_MagFactor(int new_value);
void New_FileFormat(int new_value);
void Icon_Coords(int normal_x, int normal_y, int *fat_x, int *fat_y);
void Tablet_Coords(int fat_x, int fat_y, int *raw_x, int *raw_y);
void Quantize(int *x, int *y, int center);
void Repaint_Exposed_Tablet(void);
void Repaint_Tablet(Window win, int x, int y, int width, int height);
void Paint_Tile(int x, int y, GC gc);
void Transfer_Back_Image(int x1, int y1, int x2, int y2, Boolean tflag);
void Init_Widget_List(void);
void Init_Pen_Colors(Widget wid);
void Init_Color_Table(void);
void Size_IconForm(Dimension width, Dimension height);
void Init_Icons(Dimension width, Dimension height, Boolean saveFlag);
void RegisterDropSites(void);
void Abort(char *str);
void stat_out(char *msg, char *arg0, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5, char *arg6);
void PixelTableClear(void);
int PixelTableLookup(Pixel pixelIn, Boolean allocNew);
void Switch_FillSolids(void);
void Select_New_Pen(int n);
void Backup_Icons(void);
void DoErrorDialog(char *str);
void DoQueryDialog(char *str);
void Do_GrabOp(void);
int LoadGrabbedImage(int x, int y, int width, int height);
void ParseAppArgs(int num, char *cmd[]);
void ProcessAppArgs(void);
void Set_Gfx_Labels(Boolean flag);
void SaveSession(void);
void GetSessionInfo(void);
void ChangeTitle(void);
