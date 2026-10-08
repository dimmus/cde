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
/* process.c */
void Process_New(void);
void Process_Open(void);
void Process_Save(void);
void Process_SaveAs(void);
void Process_Quit(void);
void Process_Query_OK(void);
void Process_Query_Cancel(void);
void Process_Size_OK(void);
void Eval_NewSize(int width, int height);
void Process_Size_Cancel(void);
void Process_StdErr_OK(void);
void Process_Undo(void);
void Process_Cut(void);
void Process_Copy(XImage **img, XImage **img_mono);
void Process_Paste(void);
void Process_Scale(void);
void Process_Resize(void);
void Process_Clear(void);
void Process_GrabImage(void);
void Process_AddHotspot(void);
void Process_DeleteHotspot(void);
void Process_RotateLeft(void);
void Process_RotateRight(void);
void Process_FlipV(void);
void Process_FlipH(void);
void Process_GridState(void);
void Process_DropCheckOp(Widget w, XtPointer client_data, XtPointer call_data);
void Process_DropOp(Widget w, XtPointer client_data, XtPointer call_data);
void Do_Paste(int x, int y);
