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
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Font.c
 **
 **   Project:     DT 3.0
 **
 **   Description: Controls the Dtstyle Font dialog
 **
 **                The dialog has two modes: Bitmap (the core/font set
 **                fonts of the Size list, as always) and Scalable (Xft),
 **                which adds Xft render tables for Motif clients on top
 **                of the bitmap fonts.  The resources themselves, the
 **                read-back and the preview fonts are built in FontXft.c.
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
/* $XConsortium: Font.c /main/7 1996/10/30 11:14:15 drk $ */

/*+++++++++++++++++++++++++++++++++++++++*/
/* include files                         */
/*+++++++++++++++++++++++++++++++++++++++*/

#include <X11/Xlib.h>
#include <Xm/MwmUtil.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/ComboBox.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/SSpinB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>
#include <Xm/VendorSEP.h>

#include <Dt/DialogBox.h>
#include <Dt/Icon.h>
#include <Dt/TitleBox.h>

#include <Dt/Message.h>
#include <Dt/SessionM.h>
#include <Dt/HourGlass.h>
#include <Dt/Wsm.h>
#include <Dt/GetDispRes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "Help.h"
#include "Main.h"
#include "Resource.h"
#include "Protocol.h"
#include "SaveRestore.h"
#include "FontXft.h"

/*+++++++++++++++++++++++++++++++++++++++*/
/* Local #defines                        */
/*+++++++++++++++++++++++++++++++++++++++*/

#define FONT_MSG   ((char *)GETMESSAGE(5, 23, "Style Manager - Font"))
#define PREVIEW    ((char *)GETMESSAGE(5, 17, "Preview"))
#define IMMEDIATE  ((char *)GETMESSAGE(5, 18, "The changes to fonts will show up in some\napplications the next time they are started.\nOther applications, such as file manager and\napplication manager, will not show the font\nchanges until you Exit the desktop and then log\nback in."))
#define LATER      ((char *)GETMESSAGE(5, 19, "The selected font will be used when\n you restart this session."))
#define INFO_MSG   ((char *)GETMESSAGE(5, 24, "The font that is currently used for your desktop is not\navailable in the Size list. If a new font is selected and\napplied, you will not be able to return to the current font\nusing the Style Manager - Font dialog."))
#define SYSTEM_MSG ((char *)GETMESSAGE(5, 20, "AaBbCcDdEeFfGg0123456789"))
#define USER_MSG   ((char *)GETMESSAGE(5, 21, "AaBbCcDdEeFfGg0123456789"))
#define BLANK_MSG  "                          "
#define SIZE       ((char *)GETMESSAGE(5, 22, "Size"))

/* Scalable (Xft) mode */
#define FONT_TYPE      ((char *)GETMESSAGE(5, 25, "Font Type"))
#define BITMAP_TYPE    ((char *)GETMESSAGE(5, 26, "Bitmap"))
#define XFT_TYPE       ((char *)GETMESSAGE(5, 27, "Scalable (Xft)"))
#define XFT_FONTS      ((char *)GETMESSAGE(5, 28, "Scalable Fonts"))
#define FAMILY_HDR     ((char *)GETMESSAGE(5, 29, "Family"))
#define STYLE_HDR      ((char *)GETMESSAGE(5, 30, "Style"))
#define SIZE_HDR       ((char *)GETMESSAGE(5, 31, "Size"))
#define SYSTEM_FONT    ((char *)GETMESSAGE(5, 32, "System Font:"))
#define USER_FONT      ((char *)GETMESSAGE(5, 33, "User Font:"))
#define MONO_ONLY      ((char *)GETMESSAGE(5, 34, "Monospace families only"))
#define RENDERING      ((char *)GETMESSAGE(5, 35, "Rendering"))
#define ANTIALIAS_LBL  ((char *)GETMESSAGE(5, 36, "Antialiasing:"))
#define HINTING_LBL    ((char *)GETMESSAGE(5, 37, "Hinting:"))
#define HINTSTYLE_LBL  ((char *)GETMESSAGE(5, 38, "Hint Style:"))
#define RGBA_LBL       ((char *)GETMESSAGE(5, 39, "Subpixel Order:"))
#define LCDFILTER_LBL  ((char *)GETMESSAGE(5, 40, "LCD Filter:"))
#define DPI_LBL        ((char *)GETMESSAGE(5, 41, "Resolution:"))
#define OPT_DEFAULT    ((char *)GETMESSAGE(5, 42, "Default"))
#define OPT_ON         ((char *)GETMESSAGE(5, 43, "On"))
#define OPT_OFF        ((char *)GETMESSAGE(5, 44, "Off"))
#define HS_NONE        ((char *)GETMESSAGE(5, 45, "None"))
#define HS_SLIGHT      ((char *)GETMESSAGE(5, 46, "Slight"))
#define HS_MEDIUM      ((char *)GETMESSAGE(5, 47, "Medium"))
#define HS_FULL        ((char *)GETMESSAGE(5, 48, "Full"))
#define RGBA_NONE      ((char *)GETMESSAGE(5, 49, "None"))
#define RGBA_RGB       ((char *)GETMESSAGE(5, 50, "RGB"))
#define RGBA_BGR       ((char *)GETMESSAGE(5, 51, "BGR"))
#define RGBA_VRGB      ((char *)GETMESSAGE(5, 52, "Vertical RGB"))
#define RGBA_VBGR      ((char *)GETMESSAGE(5, 53, "Vertical BGR"))
#define LCD_NONE       ((char *)GETMESSAGE(5, 54, "None"))
#define LCD_STANDARD   ((char *)GETMESSAGE(5, 55, "Standard"))
#define LCD_LIGHT      ((char *)GETMESSAGE(5, 56, "Light"))
#define LCD_LEGACY     ((char *)GETMESSAGE(5, 57, "Legacy"))
#define DPI_AUTO       ((char *)GETMESSAGE(5, 58, "Automatic"))
#define FC_OVERRIDE    ((char *)GETMESSAGE(5, 59, "Override the system font configuration\n(affects all applications)"))
#define BITMAP_SIZE    ((char *)GETMESSAGE(5, 60, "Bitmap Size"))
#define BITMAP_CAPTION ((char *)GETMESSAGE(5, 61, "Window manager, terminals and help:"))
#define NO_XFT_MOTIF   ((char *)GETMESSAGE(5, 62, "Scalable fonts are not available: the Motif\nlibrary was built without Xft support."))
#define NO_XFT_FONTS   ((char *)GETMESSAGE(5, 63, "No scalable fonts are installed."))
#define RENDER_NOTE    ((char *)GETMESSAGE(5, 64, "Rendering settings take effect in applications\nstarted after the change. Without Override,\nthe system font configuration may take precedence."))
#define FC_FILE_ERR    ((char *)GETMESSAGE(5, 65, "The font configuration file could not be written."))
#define NO_XFT_BUILD   ((char *)GETMESSAGE(5, 66, "Scalable fonts are not available: Style Manager\nwas built without fontconfig and Xft."))

/* which face a family/style/size control edits (callback client_data) */
#define FACE_SYSTEM 0
#define FACE_USER   1

/* which rendering option a combo box edits (callback client_data) */
#define OPT_ANTIALIAS 0
#define OPT_HINTING   1
#define OPT_HINTSTYLE 2
#define OPT_RGBA      3
#define OPT_LCDFILTER 4

#define FAMILY_COLUMNS 22
#define STYLE_COLUMNS  14
#define VISIBLE_ITEMS  12


/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Variables                    */
/*+++++++++++++++++++++++++++++++++++++++*/

typedef struct {
    Widget fontWkarea;
    Widget fontpictLabel;
    Widget previewTB;
    Widget previewForm;
    Widget systemLabel;
    Widget userText;
    Widget bitmapCaption;
    Widget bitmapSample;
    Widget sizeTB;
    Widget sizeList;
    /* Font Type */
    Widget fontTypeTB;
    Widget fontTypeRC;
    Widget bitmapToggle;
    Widget xftToggle;
    /* Scalable Fonts */
    Widget xftTB;
    Widget xftForm;
    Widget familyHeader, styleHeader, sizeHeader;
    Widget systemFontLabel, systemFamily, systemStyle, systemSize;
    Widget userFontLabel, userFamily, userStyle, userSize;
    Widget monoToggle;
    Widget xftNote;
    /* Rendering */
    Widget renderTB;
    Widget renderForm;
    Widget antialiasLabel, antialias;
    Widget hintingLabel, hinting;
    Widget hintStyleLabel, hintStyle;
    Widget rgbaLabel, rgba;
    Widget lcdFilterLabel, lcdFilter;
    Widget dpiLabel, dpiAuto, dpi;
    Widget fcOverride;
    Widget renderNote;
    int    originalFontIndex;
    int    selectedFontIndex;
    String selectedFontStr;
    Boolean userTextChanged;
    Boolean settingText;      /* userText is being set by the program */
    Boolean updating;         /* controls are being set by the program */
    FxSettings applied;       /* what the desktop has (last commit) */
    FxSettings pending;       /* what the dialog shows */
    FxFamilyList *families;   /* NULL: no scalable fonts (or no Xft) */
    Boolean xftAvailable;
    Boolean applyNoticeShown;
    int    *userFamilyMap;    /* userFamily position -> family index */
    int    numUserFamilies;
    int    autoDpi;
} FontData;
static FontData font;

static saveRestore save = {FALSE, 0, };

/* Xft choices read from the session file by restoreFonts() */
static FxSettings restored;
static unsigned restoredMask = 0;

/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Functions                    */
/*+++++++++++++++++++++++++++++++++++++++*/


static void CreateFontDlg(
                        Widget parent) ;
static void CreateXftControls( void ) ;
static void CreateRenderControls( void ) ;
static void _DtmapCB_fontBB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void ButtonCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void changeSampleFontCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void valueChangedCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void modeCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void familyCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void styleCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void sizeCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void monoCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void renderCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void dpiAutoCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void dpiCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void fcOverrideCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void SetControls( void ) ;
static void ApplyModeUI( void ) ;
static void FillFamilyCombo( int face ) ;
static void FillStyleCombo( int face ) ;
static void UpdatePreview( void ) ;
static Boolean CommitPending( void ) ;


/*+++++++++++++++++++++++++++++++++++++++*/
/* popup_fontBB                          */
/*+++++++++++++++++++++++++++++++++++++++*/
void
popup_fontBB(
        Widget shell )
{
  if (style.fontDialog == NULL) {
    _DtTurnOnHourGlass(shell);
    CreateFontDlg(shell);
    XtManageChild(style.fontDialog);
    _DtTurnOffHourGlass(shell);
  } else {
    XtManageChild(style.fontDialog);
    raiseWindow(XtWindow(XtParent(style.fontDialog)));
  }

  /* If no font is found to match current Desktop
     font, pop up informative message */
  if (font.selectedFontIndex < 0)
     InfoDialog(INFO_MSG, style.shell, False);
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* Small helpers for the Xft controls    */
/*+++++++++++++++++++++++++++++++++++++++*/

static FxFace *
PendingFace(
        int face )
{
    return face == FACE_USER ? &font.pending.user : &font.pending.sys;
}

static Widget
LabelGadget(
        Widget parent,
        char *name,
        char *text )
{
    int      n = 0;
    Arg      args[3];
    XmString string = CMPSTR(text);
    Widget   w;

    XtSetArg(args[n], XmNlabelString, string); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
    w = XmCreateLabelGadget(parent, name, args, n);
    XmStringFree(string);
    return w;
}

static Widget
ToggleGadget(
        Widget parent,
        char *name,
        char *text,
        XtCallbackProc proc,
        XtPointer client_data )
{
    int      n = 0;
    Arg      args[3];
    XmString string = CMPSTR(text);
    Widget   w;

    XtSetArg(args[n], XmNlabelString, string); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
    w = XmCreateToggleButtonGadget(parent, name, args, n);
    XmStringFree(string);
    if (proc)
        XtAddCallback(w, XmNvalueChangedCallback, proc, client_data);
    return w;
}

/* A drop-down list whose positions are 0-based (XmZERO_BASED). */
static Widget
DropDownList(
        Widget parent,
        char *name,
        int columns,
        char **items,
        int count,
        XtCallbackProc proc,
        XtPointer client_data )
{
    int       n = 0, i;
    Arg       args[8];
    XmString *strings = NULL;
    Widget    w;

    if (count > 0) {
        strings = (XmString *) XtMalloc(sizeof(XmString) * count);
        for (i = 0; i < count; i++)
            strings[i] = CMPSTR(items[i]);
        XtSetArg(args[n], XmNitems, strings); n++;
        XtSetArg(args[n], XmNitemCount, count); n++;
    }
    XtSetArg(args[n], XmNcomboBoxType, XmDROP_DOWN_LIST); n++;
    XtSetArg(args[n], XmNpositionMode, XmZERO_BASED); n++;
    XtSetArg(args[n], XmNvisibleItemCount,
             count > 0 && count < VISIBLE_ITEMS ? count : VISIBLE_ITEMS); n++;
    if (columns > 0) {
        XtSetArg(args[n], XmNcolumns, columns); n++;
    }
    w = XmCreateComboBox(parent, name, args, n);
    for (i = 0; i < count; i++)
        XmStringFree(strings[i]);
    XtFree((char *)strings);
    XtAddCallback(w, XmNselectionCallback, proc, client_data);
    return w;
}

/* Select item pos (0-based) of a combo box without calling callbacks. */
static void
SelectComboPos(
        Widget combo,
        int pos )
{
    XmStringTable items = NULL;
    int           count = 0;

    XtVaGetValues(combo, XmNitems, &items, XmNitemCount, &count, NULL);
    if (items && pos >= 0 && pos < count)
        XmComboBoxSelectItem(combo, items[pos]);
}

/* Replace the items of a combo box and select one of them. */
static void
SetComboItems(
        Widget combo,
        char **names,
        int count,
        int selected )
{
    XmString *items;
    int       i;

    items = (XmString *) XtMalloc(sizeof(XmString) * (count > 0 ? count : 1));
    for (i = 0; i < count; i++)
        items[i] = CMPSTR(names[i]);
    XtVaSetValues(combo, XmNitems, items, XmNitemCount, count, NULL);
    for (i = 0; i < count; i++)
        XmStringFree(items[i]);
    XtFree((char *)items);
    SelectComboPos(combo, selected);
}

static Widget
SpinBox(
        Widget parent,
        char *name,
        int min,
        int max,
        XtCallbackProc proc,
        XtPointer client_data )
{
    int    n = 0, columns, m;
    Arg    args[8];
    Widget w;

    /* one column more than the digits of max: the text cursor takes room */
    for (columns = 2, m = max; m >= 10; m /= 10)
        columns++;
    XtSetArg(args[n], XmNspinBoxChildType, XmNUMERIC); n++;
    XtSetArg(args[n], XmNminimumValue, min); n++;
    XtSetArg(args[n], XmNmaximumValue, max); n++;
    XtSetArg(args[n], XmNposition, min); n++;
    XtSetArg(args[n], XmNcolumns, columns); n++;
    XtSetArg(args[n], XmNeditable, False); n++;
    w = XmCreateSimpleSpinBox(parent, name, args, n);
    XtAddCallback(w, XmNvalueChangedCallback, proc, client_data);
    return w;
}

static void
SetSpinValue(
        Widget spin,
        int value,
        int min,
        int max )
{
    if (value < min)
        value = min;
    if (value > max)
        value = max;
    XtVaSetValues(spin, XmNposition, value, NULL);
}

/* Give all widgets of a column the width of the widest one; return it. */
static Dimension
EqualizeWidths(
        Widget *w,
        int count )
{
    Dimension width, max = 0;
    int       i;

    for (i = 0; i < count; i++) {
        XtVaGetValues(w[i], XmNwidth, &width, NULL);
        if (width > max)
            max = width;
    }
    for (i = 0; i < count; i++)
        XtVaSetValues(w[i], XmNwidth, max, NULL);
    return max;
}

/* Attach a row label to the vertical extent of the control it labels. */
static void
AttachRowLabel(
        Widget label,
        Widget control,
        int leftAttachment,
        Widget leftWidget,
        int leftOffset )
{
    int n = 0;
    Arg args[8];

    XtSetArg(args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, control); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, control); n++;
    XtSetArg(args[n], XmNleftAttachment, leftAttachment); n++;
    if (leftWidget) {
        XtSetArg(args[n], XmNleftWidget, leftWidget); n++;
    }
    XtSetArg(args[n], XmNleftOffset, leftOffset); n++;
    XtSetValues(label, args, n);
}

/*
 * Attach a control below topWidget (or the form), at leftOffset from the
 * form or from leftWidget.  Controls never attach to their row labels:
 * the labels attach to the controls (AttachRowLabel), and a cycle of
 * attachments makes XmForm place widgets in the wrong row.
 */
static void
AttachControl(
        Widget control,
        Widget topWidget,
        int topOffset,
        Widget leftWidget,
        int leftOffset )
{
    int n = 0;
    Arg args[8];

    if (topWidget) {
        XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
        XtSetArg(args[n], XmNtopWidget, topWidget); n++;
    } else {
        XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    }
    XtSetArg(args[n], XmNtopOffset, topOffset); n++;
    if (leftWidget) {
        XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
        XtSetArg(args[n], XmNleftWidget, leftWidget); n++;
    } else {
        XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    }
    XtSetArg(args[n], XmNleftOffset, leftOffset); n++;
    XtSetValues(control, args, n);
}

/* Attach a control to the same row as rowWidget and the same column as colWidget. */
static void
AttachCell(
        Widget control,
        Widget rowWidget,
        Widget colWidget,
        Boolean sameWidth )
{
    int n = 0;
    Arg args[8];

    XtSetArg(args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rowWidget); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, colWidget); n++;
    if (sameWidth) {
        XtSetArg(args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
        XtSetArg(args[n], XmNrightWidget, colWidget); n++;
    }
    XtSetValues(control, args, n);
}

/* The width of the title of a TitleBox showing text (it keeps text). */
static Dimension
TitleWidth(
        Widget titleBox,
        char *text )
{
    XmString  string = CMPSTR(text);
    Dimension width = 0;

    XtVaSetValues(titleBox, XmNtitleString, string, NULL);
    XmStringFree(string);
    XtVaGetValues(_DtTitleBoxGetTitleArea(titleBox), XmNwidth, &width, NULL);
    return width;
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* CreateFontDlg                         */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
CreateFontDlg(
        Widget parent )
{

    int     n;
    int              i;
    Arg              args[MAX_ARGS];
    XmString         button_string[FONT_NUM_BUTTONS];
    XmString         string;
    int              count = 0;
    Widget           widget_list[8];
    XmString         *sizeItems;
    XmStringTable    selectedSize;
    char             sizeStr[111];
    FxSettings       cur;
    Dimension        titleWidth, width, spacing, margin, shadow;
    XtWidgetGeometry pref;
    int              column;


    font.selectedFontStr = style.xrdb.systemFontStr;

    /* Assume nothing is selected */
    font.selectedFontIndex = -1;

    /*
     * The following flag is used to determine if the user has
     * entered anything into the sample user font field.  If
     * he does, than when the font selection is changed, the
     * default message "aAbBcC..." won't be displayed overwriting
     * the user's text, only the fontlist will be changed.
     * This flag will be set in the valueChanged callback for the
     * font.userText widget.
     */
    font.userTextChanged = FALSE;

    /*
     * Look for the selectedFont in the fontChoice array and set
     * selectedFontIndex to that entry
     */
    for (i=0; font.selectedFontStr && i<style.xrdb.numFonts; i++)
        if (style.xrdb.fontChoice[i].sysStr &&
            strcmp (font.selectedFontStr,
                    style.xrdb.fontChoice[i].sysStr) == 0)
        {
            font.selectedFontIndex = i;
            if (!style.xrdb.fontChoice[i].userFont)
                GetUserFontResource(i);
            if (!style.xrdb.fontChoice[i].sysFont)
                GetSysFontResource(i);
            break;
        }

    /*
     * Save the index of the originally selected font.  If no
     * font is selected, this value will remain -1.
     */
    font.originalFontIndex = font.selectedFontIndex;

    /* Set up button labels. */
    button_string[0] = CMPSTR((String) _DtOkString);
    button_string[1] = CMPSTR((String) _DtApplyString);
    button_string[2] = CMPSTR((String) _DtCancelString);
    button_string[3] = CMPSTR((String) _DtHelpString);

    /* Create toplevel DialogBox */
    /* saveRestore
     * Note that save.poscnt has been initialized elsewhere.
     * save.posArgs may contain information from restoreFont().
     */

    XtSetArg(save.posArgs[save.poscnt], XmNallowOverlap, False); save.poscnt++;
    XtSetArg(save.posArgs[save.poscnt], XmNdefaultPosition, False);
    save.poscnt++;
    XtSetArg(save.posArgs[save.poscnt], XmNbuttonCount, FONT_NUM_BUTTONS);
    save.poscnt++;
    XtSetArg(save.posArgs[save.poscnt], XmNbuttonLabelStrings, button_string);
    save.poscnt++;
    style.fontDialog =
        __DtCreateDialogBoxDialog(parent, "Fonts", save.posArgs, save.poscnt);
    XtAddCallback(style.fontDialog, XmNcallback, ButtonCB, NULL);
    XtAddCallback(style.fontDialog, XmNmapCallback, _DtmapCB_fontBB,
                            (XtPointer)parent);
    XtAddCallback(style.fontDialog, XmNhelpCallback,
            (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_FONT_DIALOG);

    for (i = 0; i < FONT_NUM_BUTTONS; i++)
        XmStringFree(button_string[i]);

    widget_list[0] = _DtDialogBoxGetButton(style.fontDialog,
                                           FONT_CANCEL_BUTTON);
    n=0;
    XtSetArg(args[n], XmNautoUnmanage, False); n++;
    XtSetArg(args[n], XmNcancelButton, widget_list[0]); n++;
    XtSetValues (style.fontDialog, args, n);

    n=0;
    XtSetArg(args[n], XmNtitle, FONT_MSG); n++;
    XtSetArg (args[n], XmNuseAsyncGeometry, True); n++;
    XtSetArg(args[n], XmNmwmFunctions, DIALOG_MWM_FUNC); n++;
    XtSetValues (XtParent(style.fontDialog), args, n);

    n = 0;
    XtSetArg (args[n], XmNchildType, XmWORK_AREA);  n++;
    XtSetArg(args[n], XmNhorizontalSpacing, style.horizontalSpacing); n++;
    XtSetArg(args[n], XmNverticalSpacing, style.verticalSpacing); n++;
    XtSetArg(args[n], XmNallowOverlap, False); n++;
    font.fontWkarea = XmCreateForm(style.fontDialog, "fontWorkArea", args, n);

    /*
     * The settings: Xft defaults (app-defaults), the choices saved in
     * the session file, then what the desktop really uses (the live
     * RESOURCE_MANAGER), resolved against the installed fonts.
     */
    FxGetDefaults(style.shell, &cur);
    if (restoredMask)
        FxApplyRestored(&cur, &restored, restoredMask);
    FxReadCurrent(style.display, &cur);
    font.families = FxListFamilies();
    font.xftAvailable = FxMotifHasXft(font.fontWkarea) && font.families;
    FxNormalize(font.families, &cur);
    font.applied = font.pending = cur;
    /*
     * Without Xft the dialog can only offer Bitmap mode.  applied keeps
     * the real mode, so OK switches an Xft desktop back to bitmap fonts.
     */
    if (!font.xftAvailable)
        font.pending.mode = FX_MODE_BITMAP;
    font.autoDpi = FxAutoDpi(style.display);
    font.applyNoticeShown = False;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNtopOffset, style.verticalSpacing);  n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;

    XtSetArg(args[n], XmNfillMode, XmFILL_SELF); n++;
    XtSetArg(args[n], XmNbehavior, XmICON_LABEL); n++;
    XtSetArg(args[n], XmNpixmapForeground, style.secBSCol); n++;
    XtSetArg(args[n], XmNpixmapBackground, style.secTSCol); n++;
    XtSetArg(args[n], XmNstring, NULL); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNimageName, FONT_ICON); n++;
    XtSetArg(args[n], XmNtraversalOn, False); n++;
    widget_list[count++] = font.fontpictLabel =
        _DtCreateIcon(font.fontWkarea, "fontpictLabel", args, n);

    /* Font Type: Bitmap or Scalable (Xft) */

    n = 0;
    string = CMPSTR(FONT_TYPE);
    XtSetArg(args[n], XmNtitleString, string);  n++;
    XtSetArg(args[n], XmNtopAttachment,      XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing);  n++;
    XtSetArg(args[n], XmNleftAttachment,     XmATTACH_WIDGET);     n++;
    XtSetArg(args[n], XmNleftWidget,         font.fontpictLabel);  n++;
    XtSetArg(args[n], XmNleftOffset,         style.horizontalSpacing);  n++;
    XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNrightOffset,        style.horizontalSpacing);  n++;
    widget_list[count++] = font.fontTypeTB =
        _DtCreateTitleBox(font.fontWkarea, "fontTypeTB", args, n);
    XmStringFree(string);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNmarginWidth, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    font.fontTypeRC = XmCreateRadioBox(font.fontTypeTB, "fontTypeRC", args, n);
    font.bitmapToggle = ToggleGadget(font.fontTypeRC, "bitmapToggle",
                                     BITMAP_TYPE, modeCB,
                                     (XtPointer)(intptr_t)FX_MODE_BITMAP);
    font.xftToggle = ToggleGadget(font.fontTypeRC, "xftToggle",
                                  XFT_TYPE, modeCB,
                                  (XtPointer)(intptr_t)FX_MODE_XFT);
    XtManageChild(font.bitmapToggle);
    XtManageChild(font.xftToggle);
    XtManageChild(font.fontTypeRC);

    /* Create a TitleBox and Scale/List to choose the font size */

    n = 0;
    string = CMPSTR(SIZE);
    XtSetArg(args[n], XmNtitleString, string);  n++;
    XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET);     n++;
    XtSetArg(args[n], XmNtopWidget,          font.fontTypeTB);     n++;
    XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing+5);  n++;
    XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNleftOffset,         style.horizontalSpacing);  n++;
    XtSetArg(args[n], XmNbottomAttachment,   XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNbottomOffset,       style.verticalSpacing);    n++;
    widget_list[count++] = font.sizeTB =
        _DtCreateTitleBox(font.fontWkarea, "sizeTB", args, n);
    XmStringFree(string);

    /* calculate size for each of the fonts based on system font size */

    sizeItems = (XmString *) XtMalloc(sizeof(XmString) * style.xrdb.numFonts);
    for (n=0; n<style.xrdb.numFonts; n++)
      {
	sprintf(sizeStr, "%d", (int)(n+1));
	sizeItems[n] = CMPSTR(sizeStr);
	style.xrdb.fontChoice[n].pointSize = CMPSTR(sizeStr);
      }


    n=0;
    XtSetArg (args[n], XmNselectionPolicy, XmBROWSE_SELECT); n++;
    XtSetArg (args[n], XmNautomaticSelection, True); n++;
    XtSetArg (args[n], XmNvisibleItemCount, 7); n++;
    XtSetArg (args[n], XmNitemCount, style.xrdb.numFonts); n++;
    XtSetArg (args[n], XmNitems, sizeItems); n++;

    /*
     * If matching font was found for current selection,
     * display it as selected.  Otherwise, don't select
     * anything.
     */
    if (font.selectedFontIndex >=0) {
      selectedSize = &(style.xrdb.fontChoice[font.selectedFontIndex].pointSize);
      XtSetArg (args[n], XmNselectedItems, selectedSize); n++;
      XtSetArg (args[n], XmNselectedItemCount, 1); n++;
    }
    font.sizeList = XmCreateScrolledList(font.sizeTB,"sizeList",args,n);
    XtAddCallback(font.sizeList, XmNbrowseSelectionCallback,
                                         changeSampleFontCB, NULL);

    /* If a font match was found and selected, then set it's point size. */
    if (font.selectedFontIndex >=0)
      XmListSetItem(font.sizeList,
        style.xrdb.fontChoice[font.selectedFontIndex].pointSize);
    for (n=0; n<style.xrdb.numFonts; n++)
      XmStringFree(sizeItems[n]);
    XtFree((char *)sizeItems);
    XtManageChild(font.sizeList);

    /*
     * The Size box has two titles (Size, Bitmap Size).  So that switching
     * the mode does not change the layout, the boxes on the right start
     * at a fixed offset, for the wider of the two (as DtTitleBox sizes
     * itself), and the Size box is stretched up to them.
     */
    titleWidth = TitleWidth(font.sizeTB, BITMAP_SIZE);
    width = TitleWidth(font.sizeTB, SIZE);
    if (width > titleWidth)
        titleWidth = width;
    XtVaGetValues(font.sizeTB, XmNtitleSpacing, &spacing,
                  XmNmarginWidth, &margin, XmNshadowThickness, &shadow, NULL);
    XtQueryGeometry(XtParent(font.sizeList), NULL, &pref);
    width = pref.width + 2 * (margin + shadow);
    if (titleWidth + 2 * spacing > width)
        width = titleWidth + 2 * spacing;
    column = 2 * style.horizontalSpacing + width;

    /* Scalable Fonts TitleBox */
    n = 0;
    string = CMPSTR(XFT_FONTS);
    XtSetArg(args[n], XmNtitleString, string);  n++;
    XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET);     n++;
    XtSetArg(args[n], XmNtopWidget,          font.fontTypeTB);     n++;
    XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing+5);  n++;
    XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNleftOffset,         column);              n++;
    XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNrightOffset,        style.horizontalSpacing);  n++;
    widget_list[count++] = font.xftTB =
        _DtCreateTitleBox(font.fontWkarea, "xftTB", args, n);
    XmStringFree(string);
    XtVaSetValues(font.sizeTB,
                  XmNrightAttachment, XmATTACH_WIDGET,
                  XmNrightWidget, font.xftTB,
                  XmNrightOffset, style.horizontalSpacing,
                  NULL);
    CreateXftControls();

    /* Rendering TitleBox */
    n = 0;
    string = CMPSTR(RENDERING);
    XtSetArg(args[n], XmNtitleString, string);  n++;
    XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET);     n++;
    XtSetArg(args[n], XmNtopWidget,          font.xftTB);          n++;
    XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing);    n++;
    XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNleftOffset,         column);              n++;
    XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNrightOffset,        style.horizontalSpacing);  n++;
    widget_list[count++] = font.renderTB =
        _DtCreateTitleBox(font.fontWkarea, "renderTB", args, n);
    XmStringFree(string);
    CreateRenderControls();

    /* preview TitleBox */
    n = 0;
    XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET);     n++;
    XtSetArg(args[n], XmNtopWidget,          font.renderTB);       n++;
    XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing);    n++;
    XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNleftOffset,         column);              n++;
    XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNrightOffset,        style.horizontalSpacing);  n++;
    XtSetArg(args[n], XmNbottomAttachment,   XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNbottomOffset,       style.verticalSpacing);    n++;
    string = CMPSTR(PREVIEW);
    XtSetArg(args[n], XmNtitleString, string); n++;
    widget_list[count++] = font.previewTB =
        _DtCreateTitleBox(font.fontWkarea, "systemSample", args, n);
    XmStringFree(string);

    /*form to contain preview font area*/
    n = 0;
    font.previewForm =
        XmCreateForm(font.previewTB, "previewForm", args, n);

    /*
     * sample system font.  The fonts are set later with XtSetValues
     * (UpdatePreview): a *renderTable in the resource database would
     * override a font list given at creation.
     */
    n = 0;
    string = CMPSTR(BLANK_MSG);
    XtSetArg (args[n], XmNlabelString, string);  n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);  n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
    XtSetArg (args[n], XmNtopOffset, 2 * style.verticalSpacing);  n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_NONE);  n++;
    font.systemLabel =
        XmCreateLabelGadget(font.previewForm, "systemSample", args, n);
    XmStringFree(string);

    /* sample user font */
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg (args[n], XmNtopWidget, font.systemLabel);  n++;
    XtSetArg (args[n], XmNtopOffset, 2 * style.verticalSpacing);  n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET);  n++;
    XtSetArg (args[n], XmNleftWidget, font.systemLabel);  n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_NONE);  n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET);  n++;
    XtSetArg (args[n], XmNrightWidget, font.systemLabel);  n++;
    font.userText =
        XmCreateText(font.previewForm, "userText", args, n);
    /* Add callback to determine if user changes text in sample field */
    XtAddCallback(font.userText, XmNvalueChangedCallback, valueChangedCB, NULL);

    /* Xft mode: the bitmap font that dtwm, terminals and help keep using */
    font.bitmapCaption = LabelGadget(font.previewForm, "bitmapCaption",
                                     BITMAP_CAPTION);
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg (args[n], XmNtopWidget, font.userText);  n++;
    XtSetArg (args[n], XmNtopOffset, style.verticalSpacing);  n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET);  n++;
    XtSetArg (args[n], XmNleftWidget, font.systemLabel);  n++;
    XtSetValues (font.bitmapCaption, args, n);

    n = 0;
    string = CMPSTR(SYSTEM_MSG);
    XtSetArg (args[n], XmNlabelString, string);  n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);  n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg (args[n], XmNtopWidget, font.bitmapCaption);  n++;
    XtSetArg (args[n], XmNtopOffset, style.verticalSpacing);  n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET);  n++;
    XtSetArg (args[n], XmNleftWidget, font.systemLabel);  n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
    XtSetArg (args[n], XmNbottomOffset, style.verticalSpacing);  n++;
    font.bitmapSample =
        XmCreateLabelGadget(font.previewForm, "bitmapSample", args, n);
    XmStringFree(string);

    XtManageChild(font.systemLabel);
    XtManageChild(font.userText);
    XtManageChild(font.previewForm);
    XtManageChild(font.sizeList);
    XtManageChildren(widget_list,count);
    XtManageChild(font.fontWkarea);

    /* Show the settings */
    if (!font.xftAvailable) {
        XtSetSensitive(font.xftToggle, False);
        XtManageChild(font.xftNote);
    }
    SetControls();
    UpdatePreview();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* CreateXftControls                     */
/*  Scalable Fonts box: a grid of        */
/*  family, style and size per face.     */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
CreateXftControls( void )
{
    int    n;
    Arg    args[MAX_ARGS];
    Widget labels[2];
    Widget kids[16];
    int    count = 0;
    int    hs = style.horizontalSpacing, vs = style.verticalSpacing;
    int    col2;
    char  *note;

    n = 0;
    XtSetArg(args[n], XmNallowOverlap, False); n++;
    XtSetArg(args[n], XmNchildType, XmWORK_AREA);  n++;
    font.xftForm = XmCreateForm(font.xftTB, "xftForm", args, n);

    kids[count++] = font.familyHeader =
        LabelGadget(font.xftForm, "familyHeader", FAMILY_HDR);
    kids[count++] = font.styleHeader =
        LabelGadget(font.xftForm, "styleHeader", STYLE_HDR);
    kids[count++] = font.sizeHeader =
        LabelGadget(font.xftForm, "sizeHeader", SIZE_HDR);

    kids[count++] = font.systemFontLabel =
        LabelGadget(font.xftForm, "systemFontLabel", SYSTEM_FONT);
    kids[count++] = font.systemFamily =
        DropDownList(font.xftForm, "systemFamily", FAMILY_COLUMNS, NULL, 0,
                     familyCB, (XtPointer)(intptr_t)FACE_SYSTEM);
    kids[count++] = font.systemStyle =
        DropDownList(font.xftForm, "systemStyle", STYLE_COLUMNS, NULL, 0,
                     styleCB, (XtPointer)(intptr_t)FACE_SYSTEM);
    kids[count++] = font.systemSize =
        SpinBox(font.xftForm, "systemSize", FX_SIZE_MIN, FX_SIZE_MAX,
                sizeCB, (XtPointer)(intptr_t)FACE_SYSTEM);

    kids[count++] = font.userFontLabel =
        LabelGadget(font.xftForm, "userFontLabel", USER_FONT);
    kids[count++] = font.userFamily =
        DropDownList(font.xftForm, "userFamily", FAMILY_COLUMNS, NULL, 0,
                     familyCB, (XtPointer)(intptr_t)FACE_USER);
    kids[count++] = font.userStyle =
        DropDownList(font.xftForm, "userStyle", STYLE_COLUMNS, NULL, 0,
                     styleCB, (XtPointer)(intptr_t)FACE_USER);
    kids[count++] = font.userSize =
        SpinBox(font.xftForm, "userSize", FX_SIZE_MIN, FX_SIZE_MAX,
                sizeCB, (XtPointer)(intptr_t)FACE_USER);

    kids[count++] = font.monoToggle =
        ToggleGadget(font.xftForm, "monoToggle", MONO_ONLY, monoCB, NULL);

#ifdef DTSTYLE_HAVE_XFT
    note = !FxMotifHasXft(font.fontWkarea) ? NO_XFT_MOTIF : NO_XFT_FONTS;
#else
    note = NO_XFT_BUILD;
#endif
    /* managed only when scalable fonts are not available */
    font.xftNote = LabelGadget(font.xftForm, "xftNote", note);

    /* the row labels share one column; the controls start after it */
    labels[0] = font.systemFontLabel;
    labels[1] = font.userFontLabel;
    col2 = EqualizeWidths(labels, 2) + hs;

    /* row 0: column headers */
    XtVaSetValues(font.familyHeader,
                  XmNtopAttachment, XmATTACH_FORM,
                  XmNleftAttachment, XmATTACH_FORM,
                  XmNleftOffset, col2,
                  NULL);
    AttachCell(font.styleHeader, font.familyHeader, font.systemStyle, False);
    AttachCell(font.sizeHeader, font.familyHeader, font.systemSize, False);

    /* row 1: System Font */
    AttachControl(font.systemFamily, font.familyHeader, vs / 2, NULL, col2);
    AttachControl(font.systemStyle, font.familyHeader, vs / 2,
                  font.systemFamily, hs);
    AttachControl(font.systemSize, font.familyHeader, vs / 2,
                  font.systemStyle, hs);
    AttachRowLabel(font.systemFontLabel, font.systemFamily,
                   XmATTACH_FORM, NULL, 0);

    /* row 2: User Font, in the columns of row 1 */
    AttachControl(font.userFamily, font.systemFamily, vs / 2, NULL, col2);
    XtVaSetValues(font.userFamily,
                  XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
                  XmNrightWidget, font.systemFamily,
                  NULL);
    AttachCell(font.userStyle, font.userFamily, font.systemStyle, True);
    AttachCell(font.userSize, font.userFamily, font.systemSize, False);
    AttachRowLabel(font.userFontLabel, font.userFamily,
                   XmATTACH_FORM, NULL, 0);

    /* row 3: Monospace families only, under the family column */
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, font.userFamily); n++;
    XtSetArg(args[n], XmNtopOffset, vs / 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, font.userFamily); n++;
    XtSetValues(font.monoToggle, args, n);

    /* row 4: why scalable fonts cannot be chosen */
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, font.monoToggle); n++;
    XtSetArg(args[n], XmNtopOffset, vs); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetValues(font.xftNote, args, n);

    XtManageChildren(kids, count);
    XtManageChild(font.xftForm);
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* CreateRenderControls                  */
/*  Rendering box: two label + control   */
/*  pairs per row.                       */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
CreateRenderControls( void )
{
    int    n;
    Arg    args[MAX_ARGS];
    Widget column[3];
    Widget kids[24];
    int    count = 0;
    int    hs = style.horizontalSpacing, vs = style.verticalSpacing;
    int    col2, col4;
    char   dpiStr[256];
    char  *tri[3], *hintStyles[5], *rgbas[6], *lcdFilters[5];

    tri[FX_TRI_DEFAULT] = OPT_DEFAULT;
    tri[FX_TRI_ON] = OPT_ON;
    tri[FX_TRI_OFF] = OPT_OFF;
    hintStyles[FX_HS_DEFAULT] = OPT_DEFAULT;
    hintStyles[FX_HS_NONE] = HS_NONE;
    hintStyles[FX_HS_SLIGHT] = HS_SLIGHT;
    hintStyles[FX_HS_MEDIUM] = HS_MEDIUM;
    hintStyles[FX_HS_FULL] = HS_FULL;
    rgbas[FX_RGBA_DEFAULT] = OPT_DEFAULT;
    rgbas[FX_RGBA_NONE] = RGBA_NONE;
    rgbas[FX_RGBA_RGB] = RGBA_RGB;
    rgbas[FX_RGBA_BGR] = RGBA_BGR;
    rgbas[FX_RGBA_VRGB] = RGBA_VRGB;
    rgbas[FX_RGBA_VBGR] = RGBA_VBGR;
    lcdFilters[FX_LCD_DEFAULT] = OPT_DEFAULT;
    lcdFilters[FX_LCD_NONE] = LCD_NONE;
    lcdFilters[FX_LCD_STANDARD] = LCD_STANDARD;
    lcdFilters[FX_LCD_LIGHT] = LCD_LIGHT;
    lcdFilters[FX_LCD_LEGACY] = LCD_LEGACY;

    n = 0;
    XtSetArg(args[n], XmNallowOverlap, False); n++;
    XtSetArg(args[n], XmNchildType, XmWORK_AREA);  n++;
    font.renderForm = XmCreateForm(font.renderTB, "renderForm", args, n);

    /* The position of an item is the value of its FX_* constant. */
    kids[count++] = font.antialiasLabel =
        LabelGadget(font.renderForm, "antialiasLabel", ANTIALIAS_LBL);
    kids[count++] = font.antialias =
        DropDownList(font.renderForm, "antialias", 0, tri, 3,
                     renderCB, (XtPointer)(intptr_t)OPT_ANTIALIAS);
    kids[count++] = font.hintingLabel =
        LabelGadget(font.renderForm, "hintingLabel", HINTING_LBL);
    kids[count++] = font.hinting =
        DropDownList(font.renderForm, "hinting", 0, tri, 3,
                     renderCB, (XtPointer)(intptr_t)OPT_HINTING);
    kids[count++] = font.hintStyleLabel =
        LabelGadget(font.renderForm, "hintStyleLabel", HINTSTYLE_LBL);
    kids[count++] = font.hintStyle =
        DropDownList(font.renderForm, "hintStyle", 0, hintStyles, 5,
                     renderCB, (XtPointer)(intptr_t)OPT_HINTSTYLE);
    kids[count++] = font.rgbaLabel =
        LabelGadget(font.renderForm, "rgbaLabel", RGBA_LBL);
    kids[count++] = font.rgba =
        DropDownList(font.renderForm, "rgba", 0, rgbas, 6,
                     renderCB, (XtPointer)(intptr_t)OPT_RGBA);
    kids[count++] = font.lcdFilterLabel =
        LabelGadget(font.renderForm, "lcdFilterLabel", LCDFILTER_LBL);
    kids[count++] = font.lcdFilter =
        DropDownList(font.renderForm, "lcdFilter", 0, lcdFilters, 5,
                     renderCB, (XtPointer)(intptr_t)OPT_LCDFILTER);
    kids[count++] = font.dpiLabel =
        LabelGadget(font.renderForm, "dpiLabel", DPI_LBL);

    snprintf(dpiStr, sizeof(dpiStr), "%s (%d dpi)", DPI_AUTO, font.autoDpi);
    kids[count++] = font.dpiAuto =
        ToggleGadget(font.renderForm, "dpiAuto", dpiStr, dpiAutoCB, NULL);
    kids[count++] = font.dpi =
        SpinBox(font.renderForm, "dpi", FX_DPI_MIN, FX_DPI_MAX, dpiCB, NULL);

    kids[count++] = font.fcOverride =
        ToggleGadget(font.renderForm, "fcOverride", FC_OVERRIDE,
                     fcOverrideCB, NULL);
    kids[count++] = font.renderNote =
        LabelGadget(font.renderForm, "renderNote", RENDER_NOTE);

    /*
     * Columns 1 and 3 hold the labels, 2 and 4 the controls; each column
     * gets the width of its widest member.
     */
    column[0] = font.antialiasLabel;
    column[1] = font.hintStyleLabel;
    column[2] = font.lcdFilterLabel;
    col2 = EqualizeWidths(column, 3) + hs;
    column[0] = font.antialias;
    column[1] = font.hintStyle;
    column[2] = font.lcdFilter;
    EqualizeWidths(column, 3);
    column[0] = font.hintingLabel;
    column[1] = font.rgbaLabel;
    column[2] = font.dpiLabel;
    col4 = 3 * hs + EqualizeWidths(column, 3) + hs;
    column[0] = font.hinting;
    column[1] = font.rgba;
    EqualizeWidths(column, 2);

    /* row 1: Antialiasing, Hinting */
    AttachControl(font.antialias, NULL, 0, NULL, col2);
    AttachRowLabel(font.antialiasLabel, font.antialias, XmATTACH_FORM, NULL, 0);
    AttachControl(font.hinting, NULL, 0, font.antialias, col4);
    AttachRowLabel(font.hintingLabel, font.hinting,
                   XmATTACH_WIDGET, font.antialias, 3 * hs);

    /* row 2: Hint Style, Subpixel Order */
    AttachControl(font.hintStyle, font.antialias, vs / 2, NULL, col2);
    AttachRowLabel(font.hintStyleLabel, font.hintStyle, XmATTACH_FORM, NULL, 0);
    AttachCell(font.rgba, font.hintStyle, font.hinting, True);
    AttachRowLabel(font.rgbaLabel, font.rgba,
                   XmATTACH_WIDGET, font.antialias, 3 * hs);

    /* row 3: LCD Filter, Resolution */
    AttachControl(font.lcdFilter, font.hintStyle, vs / 2, NULL, col2);
    AttachRowLabel(font.lcdFilterLabel, font.lcdFilter,
                   XmATTACH_FORM, NULL, 0);
    AttachRowLabel(font.dpiAuto, font.lcdFilter,
                   XmATTACH_OPPOSITE_WIDGET, font.hinting, 0);
    AttachCell(font.dpi, font.lcdFilter, font.hinting, False);
    XtVaSetValues(font.dpi,
                  XmNleftAttachment, XmATTACH_WIDGET,
                  XmNleftWidget, font.dpiAuto,
                  XmNleftOffset, hs,
                  NULL);
    AttachRowLabel(font.dpiLabel, font.lcdFilter,
                   XmATTACH_WIDGET, font.antialias, 3 * hs);

    /* rows 4 and 5 span the box */
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, font.lcdFilter); n++;
    XtSetArg(args[n], XmNtopOffset, vs); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetValues(font.fcOverride, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, font.fcOverride); n++;
    XtSetArg(args[n], XmNtopOffset, vs); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetValues(font.renderNote, args, n);

    XtManageChildren(kids, count);
    XtManageChild(font.renderForm);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* Families and styles                   */
/*+++++++++++++++++++++++++++++++++++++++*/

/* Set the family of a face; keep its style if the family has it. */
static void
SetFaceFamily(
        FxFace *f,
        const FxFamily *fam )
{
    const char *def;
    int         i;

    if (f->family != fam->family)
        snprintf(f->family, sizeof(f->family), "%s", fam->family);
    for (i = 0; i < fam->numStyles; i++)
        if (strcmp(fam->styles[i], f->style) == 0)
            return;
    def = FxDefaultStyle(fam);
    snprintf(f->style, sizeof(f->style), "%s", def ? def : "");
}

static void
FillFamilyCombo(
        int face )
{
    Widget   combo = face == FACE_USER ? font.userFamily : font.systemFamily;
    FxFace  *f = PendingFace(face);
    char   **names;
    char    *one[1];
    int      i, count = 0, selected = -1;
    Boolean  monoOnly = face == FACE_USER && font.pending.userMonoOnly;

    if (!font.families) {
        /* Nothing to choose from: just show the current family. */
        one[0] = f->family;
        SetComboItems(combo, one, f->family[0] ? 1 : 0, 0);
        return;
    }

    names = (char **) XtMalloc(sizeof(char *) * font.families->numFamilies);
    if (face == FACE_USER && !font.userFamilyMap)
        font.userFamilyMap = (int *)
            XtMalloc(sizeof(int) * font.families->numFamilies);
    for (i = 0; i < font.families->numFamilies; i++) {
        if (monoOnly && !font.families->families[i].mono)
            continue;
        if (strcmp(font.families->families[i].family, f->family) == 0)
            selected = count;
        if (face == FACE_USER)
            font.userFamilyMap[count] = i;
        names[count++] = font.families->families[i].family;
    }
    if (face == FACE_USER)
        font.numUserFamilies = count;
    SetComboItems(combo, names, count, selected);
    XtFree((char *)names);
}

static void
FillStyleCombo(
        int face )
{
    Widget          combo = face == FACE_USER ? font.userStyle
                                              : font.systemStyle;
    FxFace         *f = PendingFace(face);
    const FxFamily *fam = NULL;
    char           *one[1];
    int             i, selected = -1;

    if (font.families)
        fam = FxFindFamily(font.families, f->family);
    if (!fam) {
        one[0] = f->style;
        SetComboItems(combo, one, f->style[0] ? 1 : 0, 0);
        return;
    }
    SetFaceFamily(f, fam);      /* makes f->style one of the family's */
    for (i = 0; i < fam->numStyles; i++)
        if (strcmp(fam->styles[i], f->style) == 0)
            selected = i;
    SetComboItems(combo, fam->styles, fam->numStyles, selected);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* SetControls                           */
/*  Show font.pending in every control.  */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
SetControls( void )
{
    FxSettings *p = &font.pending;

    font.updating = True;
    XmToggleButtonGadgetSetState(font.bitmapToggle,
                                 p->mode != FX_MODE_XFT, False);
    XmToggleButtonGadgetSetState(font.xftToggle,
                                 p->mode == FX_MODE_XFT, False);

    FillFamilyCombo(FACE_SYSTEM);
    FillStyleCombo(FACE_SYSTEM);
    FillFamilyCombo(FACE_USER);
    FillStyleCombo(FACE_USER);
    SetSpinValue(font.systemSize, p->sys.size, FX_SIZE_MIN, FX_SIZE_MAX);
    SetSpinValue(font.userSize, p->user.size, FX_SIZE_MIN, FX_SIZE_MAX);
    XmToggleButtonGadgetSetState(font.monoToggle, p->userMonoOnly, False);

    SelectComboPos(font.antialias, p->antialias);
    SelectComboPos(font.hinting, p->hinting);
    SelectComboPos(font.hintStyle, p->hintStyle);
    SelectComboPos(font.rgba, p->rgba);
    SelectComboPos(font.lcdFilter, p->lcdFilter);
    XmToggleButtonGadgetSetState(font.dpiAuto, p->dpi == 0, False);
    SetSpinValue(font.dpi, p->dpi ? p->dpi : font.autoDpi,
                 FX_DPI_MIN, FX_DPI_MAX);
    XtSetSensitive(font.dpi, p->dpi != 0);
    XmToggleButtonGadgetSetState(font.fcOverride, p->fcOverride, False);
    font.updating = False;

    ApplyModeUI();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* ApplyModeUI                           */
/*  What depends on Bitmap / Scalable.   */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
ApplyModeUI( void )
{
    Boolean  xft = font.pending.mode == FX_MODE_XFT;
    XmString string;

    XtSetSensitive(font.xftTB, xft);
    XtSetSensitive(font.renderTB, xft);

    string = CMPSTR(xft ? BITMAP_SIZE : SIZE);
    XtVaSetValues(font.sizeTB, XmNtitleString, string, NULL);
    XmStringFree(string);

    if (xft) {
        XtManageChild(font.bitmapCaption);
        XtManageChild(font.bitmapSample);
    } else {
        XtUnmanageChild(font.bitmapCaption);
        XtUnmanageChild(font.bitmapSample);
    }
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* UpdatePreview                         */
/*  Show font.pending in the samples.    */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
UpdatePreview( void )
{
    int            n;
    Arg            args[MAX_ARGS];
    int            idx = font.selectedFontIndex;
    XmFontList     sysFont, userFont;
    XmRenderTable  sysRT = NULL, userRT = NULL;
    XmString       string;
    Boolean        xft = font.pending.mode == FX_MODE_XFT &&
                         font.xftAvailable;

    if (idx >= 0) {
        sysFont = style.xrdb.fontChoice[idx].sysFont;
        userFont = style.xrdb.fontChoice[idx].userFont;
    } else {
        sysFont = style.xrdb.systemFont;
        userFont = style.xrdb.userFont;
    }

    if (xft) {
        sysRT = FxPreviewRenderTable(font.systemLabel, &font.pending.sys,
                                     &font.pending);
        userRT = FxPreviewRenderTable(font.userText, &font.pending.user,
                                      &font.pending);

        /* the bitmap system font that stays in use */
        n = 0;
        string = CMPSTR(idx >= 0 ? SYSTEM_MSG : BLANK_MSG);
        XtSetArg(args[n], XmNfontList, sysFont); n++;
        XtSetArg(args[n], XmNlabelString, string); n++;
        XtSetValues(font.bitmapSample, args, n);
        XmStringFree(string);
    }

    /* Set the sample System Font string to the new font */
    n = 0;
    string = CMPSTR(sysRT || idx >= 0 ? SYSTEM_MSG : BLANK_MSG);
    if (sysRT) {
        XtSetArg(args[n], XmNrenderTable, sysRT); n++;
    } else {
        XtSetArg(args[n], XmNfontList, sysFont); n++;
    }
    XtSetArg(args[n], XmNlabelString, string); n++;
    XtSetValues(font.systemLabel, args, n);
    XmStringFree(string);

    /*
     * If the user didn't change the text field, output standard user
     * text message.
     */
    n = 0;
    if (!font.userTextChanged) {
        XtSetArg(args[n], XmNvalue, userRT || idx >= 0 ? USER_MSG : ""); n++;
    }
    if (userRT) {
        XtSetArg(args[n], XmNrenderTable, userRT); n++;
    } else {
        XtSetArg(args[n], XmNfontList, userFont); n++;
    }
    font.settingText = True;
    XtSetValues(font.userText, args, n);
    font.settingText = False;
    XmTextShowPosition(font.userText, 0);

    /* The widgets keep their own copies. */
    if (sysRT)
        XmRenderTableFree(sysRT);
    if (userRT)
        XmRenderTableFree(userRT);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* _DtmapCB_fontBB                          */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
_DtmapCB_fontBB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{

    DtWsmRemoveWorkspaceFunctions(style.display, XtWindow(XtParent(w)));

    if (!save.restoreFlag)
        putDialog ((Widget)client_data, w);

    XmTextShowPosition(font.userText, 0);

    XtRemoveCallback(style.fontDialog, XmNmapCallback, _DtmapCB_fontBB, NULL);

}


/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
/* CommitPending                                   */
/* Write the pending settings to the desktop.      */
/* Returns True if anything was committed.         */
/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
static Boolean
CommitPending( void )
{
    int            idx = font.selectedFontIndex;
    FxBitmapChoice b;
    char          *info = NULL;
    Boolean        ok;
    Boolean        changed;

    /*
     * In Bitmap mode the Xft choices do not reach the desktop; they are
     * only kept (applied, and the session file).
     */
    changed = font.pending.mode != font.applied.mode ||
              (idx >= 0 && idx != font.originalFontIndex) ||
              (font.pending.mode == FX_MODE_XFT &&
               !FxSettingsEqual(&font.pending, &font.applied));
    if (!changed) {
        font.applied = font.pending;
        return False;
    }

    if (idx >= 0) {
        b.sysStr = style.xrdb.fontChoice[idx].sysStr;
        b.userStr = style.xrdb.fontChoice[idx].userStr;
    } else {
        b.sysStr = style.xrdb.systemFontStr;
        b.userStr = style.xrdb.userFontStr;
    }

    _DtTurnOnHourGlass(style.fontDialog);
    ok = FxCommit(style.display, &font.pending, &b,
                  style.xrdb.writeXrdbImmediate, &info);
    if (info) {
        SmNewFontSettings(info);
        XtFree(info);
    }
    _DtTurnOffHourGlass(style.fontDialog);
    if (!ok)
        ErrDialog(FC_FILE_ERR, style.shell);

    font.applied = font.pending;
    font.originalFontIndex = idx;
    if (idx >= 0) {
        style.xrdb.systemFontStr = style.xrdb.fontChoice[idx].sysStr;
        style.xrdb.userFontStr = style.xrdb.fontChoice[idx].userStr;
        style.xrdb.systemFont = style.xrdb.fontChoice[idx].sysFont;
        style.xrdb.userFont = style.xrdb.fontChoice[idx].userFont;
    }
    return True;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
/* ButtonCB                                        */
/* Process callback from PushButtons in DialogBox. */
/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
static void
ButtonCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
  DtDialogBoxCallbackStruct *cb
           = (DtDialogBoxCallbackStruct *) call_data;

  switch (cb->button_position)
    {
        /* Set the xrdb or pass to dtsession and close the window */
      case FONT_OK_BUTTON:
         if (CommitPending())
         {
            XtUnmanageChild(style.fontDialog);

            /* Post an info dialog explaining when the new fonts will appear */
            if(style.xrdb.writeXrdbImmediate)
              InfoDialog(IMMEDIATE, style.shell, False);
            else
              InfoDialog(LATER, style.shell, False);
         }
         else
            XtUnmanageChild(style.fontDialog);
         break;

        /* Set the xrdb or pass to dtsession and keep the window */
      case FONT_APPLY_BUTTON:
         if (CommitPending() && !font.applyNoticeShown)
         {
            font.applyNoticeShown = True;
            if(style.xrdb.writeXrdbImmediate)
              InfoDialog(IMMEDIATE, style.shell, False);
            else
              InfoDialog(LATER, style.shell, False);
         }
         break;

    case FONT_CANCEL_BUTTON:

      /*
       * Reset the controls and the preview to what was last applied
       * and close the window.  This does not undo an Apply.
       */
      font.pending = font.applied;
      if (!font.xftAvailable)
          font.pending.mode = FX_MODE_BITMAP;
      SetControls();

      if (font.originalFontIndex >= 0) {
        if (font.originalFontIndex != font.selectedFontIndex)
          XmListSelectPos(font.sizeList, font.originalFontIndex+1, False);
        font.selectedFontIndex = font.originalFontIndex;
        font.selectedFontStr =
            style.xrdb.fontChoice[font.originalFontIndex].sysStr;
      }
      else {
	/*
	 * if no font was originally selected, need to undo any results
	 * from selections that were made by user before pressing Cancel.
	 */
	XmListDeselectAllItems(font.sizeList);
	font.userTextChanged = FALSE;
	font.selectedFontIndex = -1;
	font.selectedFontStr = style.xrdb.systemFontStr;
      }
      UpdatePreview();

      XtUnmanageChild(style.fontDialog);
      break;

    case FONT_HELP_BUTTON:
      XtCallCallbacks(style.fontDialog, XmNhelpCallback, (XtPointer)NULL);
      break;

    default:
      break;
    }
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* changSampleFontCB                     */
/*  Change the font in the sample areas  */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
changeSampleFontCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    int       pos;
    int       hourGlassOn;
    XmListCallbackStruct *cb = (XmListCallbackStruct *) call_data;

    pos = cb->item_position-1;
    if (pos < 0 || pos >= style.xrdb.numFonts)
        return;

    font.selectedFontIndex = pos;
    font.selectedFontStr = style.xrdb.fontChoice[pos].sysStr;

    hourGlassOn = !style.xrdb.fontChoice[pos].userFont ||
                !style.xrdb.fontChoice[pos].sysFont;

    if (hourGlassOn)
      _DtTurnOnHourGlass(style.fontDialog);

    if (!style.xrdb.fontChoice[pos].userFont)
      GetUserFontResource(pos);
    if (!style.xrdb.fontChoice[pos].sysFont)
      GetSysFontResource(pos);

    if (hourGlassOn)
      _DtTurnOffHourGlass(style.fontDialog);

    /* Bitmap mode: the samples; Scalable mode: the bitmap sample. */
    UpdatePreview();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* valueChangedCB                        */
/*  Set flag indicating that the user    */
/*  text field has been modified.        */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
valueChangedCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )

{
  if (!font.settingText)
    font.userTextChanged = TRUE;
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* modeCB                                */
/*  Bitmap / Scalable (Xft) toggles      */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
modeCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    XmToggleButtonCallbackStruct *cb =
        (XmToggleButtonCallbackStruct *) call_data;

    if (font.updating || !cb->set)
        return;
    font.pending.mode = (int)(intptr_t)client_data;
    ApplyModeUI();
    UpdatePreview();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* familyCB                              */
/*  A family was chosen: offer its       */
/*  styles.                              */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
familyCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    XmComboBoxCallbackStruct *cb = (XmComboBoxCallbackStruct *) call_data;
    int    face = (int)(intptr_t)client_data;
    int    pos = cb->item_position;     /* 0-based (XmZERO_BASED) */
    int    idx;

    if (font.updating || !font.families)
        return;
    if (face == FACE_USER) {
        if (pos < 0 || pos >= font.numUserFamilies)
            return;
        idx = font.userFamilyMap[pos];
    } else {
        if (pos < 0 || pos >= font.families->numFamilies)
            return;
        idx = pos;
    }
    SetFaceFamily(PendingFace(face), &font.families->families[idx]);
    font.updating = True;
    FillStyleCombo(face);
    font.updating = False;
    UpdatePreview();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* styleCB                               */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
styleCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    XmComboBoxCallbackStruct *cb = (XmComboBoxCallbackStruct *) call_data;
    FxFace         *f = PendingFace((int)(intptr_t)client_data);
    const FxFamily *fam;
    int             pos = cb->item_position;

    if (font.updating || !font.families)
        return;
    fam = FxFindFamily(font.families, f->family);
    if (!fam || pos < 0 || pos >= fam->numStyles)
        return;
    snprintf(f->style, sizeof(f->style), "%s", fam->styles[pos]);
    UpdatePreview();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* sizeCB                                */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
sizeCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    int value = 0;

    if (font.updating)
        return;
    XtVaGetValues(w, XmNposition, &value, NULL);
    PendingFace((int)(intptr_t)client_data)->size = value;
    UpdatePreview();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* monoCB                                */
/*  Monospace families only (User Font)  */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
monoCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    XmToggleButtonCallbackStruct *cb =
        (XmToggleButtonCallbackStruct *) call_data;
    const FxFamily *fam;
    const char     *name;

    if (font.updating)
        return;
    font.pending.userMonoOnly = cb->set;
    if (font.families && cb->set) {
        fam = FxFindFamily(font.families, font.pending.user.family);
        if (!fam || !fam->mono) {
            name = FxResolveFamily(font.families, "monospace", True);
            fam = name ? FxFindFamily(font.families, name) : NULL;
            if (fam)
                SetFaceFamily(&font.pending.user, fam);
        }
    }
    font.updating = True;
    FillFamilyCombo(FACE_USER);
    FillStyleCombo(FACE_USER);
    font.updating = False;
    UpdatePreview();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* renderCB                              */
/*  Antialiasing, Hinting, Hint Style,   */
/*  Subpixel Order, LCD Filter           */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
renderCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    XmComboBoxCallbackStruct *cb = (XmComboBoxCallbackStruct *) call_data;
    int pos = cb->item_position;        /* = the FX_* value */

    if (font.updating || pos < 0)
        return;
    switch ((int)(intptr_t)client_data) {
      case OPT_ANTIALIAS: font.pending.antialias = pos; break;
      case OPT_HINTING:   font.pending.hinting = pos; break;
      case OPT_HINTSTYLE: font.pending.hintStyle = pos; break;
      case OPT_RGBA:      font.pending.rgba = pos; break;
      case OPT_LCDFILTER: font.pending.lcdFilter = pos; break;
      default: return;
    }
    UpdatePreview();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* dpiAutoCB / dpiCB                     */
/*  Resolution: Automatic or a value     */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
dpiAutoCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    XmToggleButtonCallbackStruct *cb =
        (XmToggleButtonCallbackStruct *) call_data;
    int value = 0;

    if (font.updating)
        return;
    if (cb->set)
        font.pending.dpi = 0;
    else {
        XtVaGetValues(font.dpi, XmNposition, &value, NULL);
        font.pending.dpi = value;
    }
    XtSetSensitive(font.dpi, !cb->set);
    UpdatePreview();
}

static void
dpiCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    int value = 0;

    if (font.updating || XmToggleButtonGadgetGetState(font.dpiAuto))
        return;
    XtVaGetValues(w, XmNposition, &value, NULL);
    font.pending.dpi = value;
    UpdatePreview();
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* fcOverrideCB                          */
/*  It changes which options take effect */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
fcOverrideCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
    XmToggleButtonCallbackStruct *cb =
        (XmToggleButtonCallbackStruct *) call_data;

    if (font.updating)
        return;
    font.pending.fcOverride = cb->set;
    UpdatePreview();
}

/************************************************************************
 * restoreFonts()
 *
 * restore any state information saved with saveFonts.
 * This is called from restoreSession with the application
 * shell and the special xrm database retrieved for restore.
 ************************************************************************/
void
restoreFonts(
        Widget shell,
        XrmDatabase db )
{
    XrmName xrm_name[5];
    XrmRepresentation rep_type;
    XrmValue value;

    xrm_name [0] = XrmStringToQuark ("Fonts");
    xrm_name [2] = 0;

    /* get x position */
    xrm_name [1] = XrmStringToQuark ("x");
    if (XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value)){
      XtSetArg (save.posArgs[save.poscnt], XmNx, atoi((char *)value.addr));
      save.poscnt++;
      save.restoreFlag = True;
    }

    /* get y position */
    xrm_name [1] = XrmStringToQuark ("y");
    if (XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value)){
      XtSetArg (save.posArgs[save.poscnt], XmNy, atoi((char *)value.addr));
      save.poscnt++;
    }

    /* get width */
    xrm_name [1] = XrmStringToQuark ("width");
    if (XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value) &&
        atoi((char *)value.addr) > 0){
      XtSetArg (save.posArgs[save.poscnt], XmNwidth,
                atoi((char *)value.addr));
      save.poscnt++;
    }

    /* get height */
    xrm_name [1] = XrmStringToQuark ("height");
    if (XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value) &&
        atoi((char *)value.addr) > 0){
      XtSetArg (save.posArgs[save.poscnt], XmNheight,
                atoi((char *)value.addr));
      save.poscnt++;
    }

    /* the Xft choices (the mode always comes from the desktop) */
    restoredMask = FxRestoreState(db, &restored);

    xrm_name [1] = XrmStringToQuark ("ismapped");
    /* Are we supposed to be mapped? */
    if (XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value) &&
        value.addr && strcmp((char *)value.addr, "True") == 0)
      popup_fontBB(shell);
}

/************************************************************************
 * saveFonts()
 *
 * This routine will write out to the passed file descriptor any state
 * information this dialog needs.  It is called from saveSessionCB with the
 * file already opened.
 * All information is saved in xrm format.  There is no restriction
 * on what can be saved.  It doesn't have to be defined or be part of any
 * widget or Xt definition.  Just name and save it here and recover it in
 * restoreFonts.  The suggested minimum is whether you are mapped, and your
 * location.
 ************************************************************************/
void
saveFonts(
        int fd )
{
    Position x,y;
    char bufr[1024];     /* size=[1024], make bigger if needed */
    XmVendorShellExtObject  vendorExt;
    XmWidgetExtData         extData;
    FxSettings              keep;

    if (style.fontDialog != NULL) {
        if (XtIsManaged(style.fontDialog))
          snprintf(bufr, sizeof(bufr), "*Fonts.ismapped: True\n");
        else
          snprintf(bufr, sizeof(bufr), "*Fonts.ismapped: False\n");

        WRITE_STR2FD(fd, bufr);

        /* Get and write out the geometry info for our Window */

        x = XtX(XtParent(style.fontDialog));
        y = XtY(XtParent(style.fontDialog));

        /* Modify x & y to take into account window mgr frames
         * This is pretty bogus, but I don't know a better way to do it.
         */
        extData = _XmGetWidgetExtData(style.shell, XmSHELL_EXTENSION);
        vendorExt = (XmVendorShellExtObject)extData->widget;
        x -= vendorExt->vendor.xOffset;
        y -= vendorExt->vendor.yOffset;

        snprintf(bufr, sizeof(bufr), "*Fonts.x: %d\n", x);
        WRITE_STR2FD(fd, bufr);
        snprintf(bufr, sizeof(bufr), "*Fonts.y: %d\n", y);
        WRITE_STR2FD(fd, bufr);
        snprintf(bufr, sizeof(bufr), "*Fonts.width: %d\n",
                 (int)XtWidth(style.fontDialog));
        WRITE_STR2FD(fd, bufr);
        snprintf(bufr, sizeof(bufr), "*Fonts.height: %d\n",
                 (int)XtHeight(style.fontDialog));
        WRITE_STR2FD(fd, bufr);

        FxSaveState(fd, &font.pending);
    }
    else if (restoredMask) {
        /* never opened: pass the restored Xft choices on */
        FxGetDefaults(style.shell, &keep);
        FxApplyRestored(&keep, &restored, restoredMask);
        FxSaveState(fd, &keep);
    }
}
