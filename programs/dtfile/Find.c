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
/* $TOG: Find.c /main/10 1999/12/09 13:06:33 mgreess $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           Find.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   Description:    Source file for the find file dialog.
 *
 *   FUNCTIONS: AddMatch
 *		Create
 *		Destroy
 *		EndSearchProcess
 *		EnterStopBttn
 *		ExecuteFind
 *		ExtractDirectory
 *		FinishSearch
 *		FindProcessStarted
 *		FindPutOnDesktop
 *		FreeMatchInfo
 *		FreeGrepState
 *		FreeValues
 *		GetDefaultValues
 *		GetFileName
 *		GetFindValues
 *		GetResourceValues
 *		GetValues
 *		InstallChange
 *		InstallClose
 *		InvalidFindMessage
 *		LeaveStopBttn
 *		MakeAbsolute
 *		NewView
 *		SetActiveItem
 *		SetFocus
 *		SearchInputHandler
 *		SearchLine
 *		SetValues
 *		SpawnSearchProcess
 *		StartCallback
 *		StartNextGrep
 *		StartSearch
 *		StopCallback
 *		StopSearch
 *		WriteResourceValues
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/


#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <spawn.h>


#include <stdlib.h>

#include <string.h>

#include <Xm/XmP.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/Frame.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/VendorSEP.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>

#include <Dt/TitleBox.h>

#include <X11/ShellP.h>
#include <X11/Xatom.h>

#include <Dt/Connect.h>
#include <Dt/DtNlUtils.h>
#include <Dt/HourGlass.h>
#include <Dt/Dts.h>
#include <Dt/SharedProcs.h>

#include <Tt/tttk.h>

#include <Xm/XmPrivate.h> /* _XmStringUngenerate */

#include "Encaps.h"
#include "SharedProcs.h"
#include "FileMgr.h"
#include "Desktop.h"
#include "Main.h"
#include "Common.h"
#include "Find.h"
#include "Help.h"
#include "SharedMsgs.h"


typedef struct _Dummy {
   Boolean       displayed;
   Position      x;
   Position      y;
   Dimension     width;
   Dimension     height;

   String string;
} Dummy, *DummyPtr;


/* Error message defines */

#define BAD_DIR_NAME               0
#define NO_DIR_ACCESS              1
#define NO_EXISTANCE               2
#define NO_FILE_OR_FOLDER_ARG      3


/* More string defines */
static char * FIND_FILE = "FindFile";

/* what the output of the running search process lists (searchPhase) */
#define PHASE_NAMES  0   /* search by name: the matching files */
#define PHASE_DIRS   1   /* search by contents: the folders to search */
#define PHASE_GREP   2   /* search by contents: the files that contain it */

/* bytes read from the search process per input callback */
#define SEARCH_READ_CHUNK  16384

/* at most this many files (and bytes of names) per grep process */
#define GREP_MAX_FILES     1024
#define GREP_MAX_BYTES     (64 * 1024)

extern char **environ;

#define NEW_VIEW     0
#define CURRENT_VIEW 1



/*  Resource definitions for the find file dialog  */

static DialogResource resources[] =
{
   { "folders", XmRString, sizeof(String),
     XtOffset(FindDataPtr, directories),
     (XtPointer) NULL, _DtStringToString },

   { "name", XmRString, sizeof(String),
     XtOffset(FindDataPtr, filter),
     (XtPointer) NULL, _DtStringToString },

   { "selectedItem", XmRInt, sizeof(int),
     XtOffset(FindDataPtr, selected_item),
     (XtPointer) -1, _DtIntToString },

   { "content", XmRString, sizeof(String),
     XtOffset(FindDataPtr, content),
     (XtPointer) NULL, _DtStringToString },
};


static DialogResource match_resources[] =
{
   { "matchData", XmRString, sizeof(String),
     XtOffset(DummyPtr, string),
     (XtPointer) NULL, _DtStringToString },
};

/********    Static Function Declarations    ********/

static void Create(
                        Display *display,
                        Widget parent,
                        Widget *return_widget,
                        XtPointer *dialog) ;
static void InstallChange(
                        FindRec *find_rec,
                        XtCallbackProc callback,
                        XtPointer client_data) ;
static void InstallClose(
                        FindRec *find_rec,
                        XtCallbackProc callback,
                        XtPointer client_data) ;
static void Destroy(
                        FindRec *find_rec) ;
static XtPointer GetValues(
                        FindRec *find_rec) ;
static XtPointer GetDefaultValues( void ) ;
static XtPointer GetResourceValues(
                        XrmDatabase data_base,
                        char **name_list) ;
static void SetValues(
                        FindRec *find_rec,
                        FindData *find_data) ;
static void WriteResourceValues(
                        DialogData *values,
                        int fd,
                        char **name_list) ;
static void FreeValues(
                        FindData *find_data) ;
static Boolean GetFindValues(
                        FindRec *find_rec,
                        FindData *find_data,
                        Boolean validate) ;
static void InvalidFindMessage(
                        FindRec *find_rec,
                        int messageIndex,
                        String extra_string) ;
static void FreeMatchInfo(
                        String *matches,
                        int numMatches) ;
static void LeaveStopBttn(
                        Widget w,
                        FindRec * find_rec,
                        XEvent * event) ;
static void EnterStopBttn(
                        Widget w,
                        FindRec * find_rec,
                        XEvent * event) ;
static void StopSearch(
                        Widget w,
                        FindRec *find_rec) ;
static void StartSearch(
                        Widget w,
                        FindRec *find_rec) ;
static void StartCallback(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void StopCallback(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static Boolean FindProcessStarted(
                        FindRec *find_rec,
                        FindData *find_data) ;
static Boolean ExecuteFind(
                        FindRec *find_rec,
                        FindData *find_data,
                        FileMgrData *file_mgr_data) ;
static int SpawnSearchProcess(
                        FindRec *find_rec,
                        char **argv) ;
static void StartNextGrep(
                        FindRec * find_rec) ;
static void SearchInputHandler(
                        XtPointer client_data,
                        int *source,
                        XtInputId *id) ;
static void EndSearchProcess(
                        FindRec *find_rec,
                        Boolean kill_it) ;
static void FreeGrepState(
                        FindRec *find_rec) ;
static void GetFileName(
                        Widget list,
                        int selectedItem,
                        String *host,
                        String *path,
                        FileMgrData *file_mgr_data) ;
static Boolean ExtractDirectory(
                        String host,
                        String path,
                        char **file_name) ;
static void NewView(
                        Widget widget,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void FindPutOnDesktop(
                        Widget widget,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void SetActiveItem(
                        Widget widget,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void SetFocus(
                        FindRec *find_rec,
                        FindData *find_data ) ;

/********    End Static Function Declarations    ********/


/*
 *  The Dialog Class structure.
 */

static DialogClass findClassRec =
{
   resources,
   XtNumber(resources),
   Create,
   (DialogInstallChangeProc) InstallChange,
   (DialogInstallCloseProc) InstallClose,
   (DialogDestroyProc) Destroy,
   (DialogGetValuesProc) GetValues,
   GetDefaultValues,
   GetResourceValues,
   (DialogSetValuesProc) SetValues,
   WriteResourceValues,
   (DialogFreeValuesProc) FreeValues,
   (DialogMapWindowProc) _DtGenericMapWindow,
   (DialogSetFocusProc) SetFocus,
};

DialogClass * findClass = (DialogClass *) &findClassRec;


/************************************************************************
 *
 *  Create
 *
 ************************************************************************/

static void
Create(
        Display *display,
        Widget parent,
        Widget *return_widget,
        XtPointer *dialog )
{
   FindRec * find_rec;
   Widget shell, form, form2;
   Widget newFM, outputSeparator;
   Widget headLabel, contentLabel, contentText;
   Widget filterText, filterLabel, listLabel, scrolledList, dirName, dirLabel;
#if defined(sun)
   Widget form1;
   Widget followLink, followLinkPD;
#endif
   Widget putOnDT, separator;
   Widget start, stop, close, help;
   XmString label_string;

   Arg args[12];
   int n;
   XtTranslations trans_table;

   /*  Allocate the find file dialog instance record.  */

   find_rec = (FindRec *) XtMalloc (sizeof (FindRec));

   /*  Create the shell and form used for the dialog.  */

   n = 0;
   XtSetArg (args[n], XmNmwmFunctions, MWM_FUNC_MOVE |
             MWM_FUNC_CLOSE );                                  ++n;
   XtSetArg (args[n], XmNmwmDecorations, MWM_DECOR_BORDER |
             MWM_DECOR_TITLE);                                ++n;
   XtSetArg (args[n], XmNallowShellResize, False);              ++n;
   shell = XmCreateDialogShell (parent, "find_files", args, n);

   /* Set the useAsyncGeo on the shell */
   XtSetArg (args[0], XmNuseAsyncGeometry, True);
   XtSetValues (XtParent(shell), args, 1);

   trans_table = XtParseTranslationTable(translations_space);

   n = 0;
   XtSetArg (args[n], XmNmarginWidth, 1);				n++;
   XtSetArg (args[n], XmNmarginHeight, 1);				n++;
   XtSetArg (args[n], XmNshadowThickness, 1);			n++;
   XtSetArg (args[n], XmNshadowType, XmSHADOW_OUT);			n++;
   XtSetArg (args[n], XmNautoUnmanage, False);			n++;
   form = XmCreateForm (shell, "form", args, n);
   XtAddCallback(form, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);

   label_string = XmStringCreateLocalized ((GETMESSAGE(15,47, "Fill in one or more fields to specify which items to find:")));
   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNleftOffset, 5);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);			n++;
   XtSetArg (args[n], XmNtopOffset, 14);				n++;
   XtSetArg (args[n], XmNtraversalOn, False);				n++;
   headLabel = XmCreateLabelGadget (form, "hdlb", args, n);
   XtManageChild (headLabel);
   XmStringFree (label_string);

   label_string = XmStringCreateLocalized ((GETMESSAGE(15,31, "File or Folder Name: ")));
   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNleftOffset, 5);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, headLabel);				n++;
   XtSetArg (args[n], XmNtopOffset, 15);				n++;
   XtSetArg (args[n], XmNtraversalOn, False);				n++;
   filterLabel = XmCreateLabelGadget (form, "file_name_label", args, n);
   XtManageChild (filterLabel);
   XmStringFree (label_string);
   XtAddCallback(filterLabel, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);

   n = 0;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNleftWidget, filterLabel);			n++;
   XtSetArg (args[n], XmNleftOffset, 0);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, headLabel);				n++;
   XtSetArg (args[n], XmNtopOffset, 10);				n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNrightOffset, 10);				n++;
   filterText = XmCreateTextField (form, "file_name_text", args, n);
   XtManageChild (filterText);
   XtAddCallback(filterText, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);

   /* set up translations in the filter text edit widget */
   XtOverrideTranslations(filterText, trans_table);

   label_string = XmStringCreateLocalized ((GETMESSAGE(15,32, "File Contents:")));
   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNleftOffset, 5);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, filterText);			n++;
   XtSetArg (args[n], XmNtopOffset, 15);				n++;
   XtSetArg (args[n], XmNtraversalOn, False);				n++;
   contentLabel = XmCreateLabelGadget (form, "content_label", args, n);
   XtManageChild (contentLabel);
   XmStringFree (label_string);
   XtAddCallback(contentLabel, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);

   n = 0;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNleftWidget, contentLabel);			n++;
   XtSetArg (args[n], XmNleftOffset, 0);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, filterText);			n++;
   XtSetArg (args[n], XmNtopOffset, 10);				n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNrightOffset, 10);				n++;
   contentText = XmCreateTextField (form, "content_text", args, n);
   XtManageChild (contentText);
   XtAddCallback(contentText, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);

#if defined(sun)
   n = 0;
   XtSetArg (args[n], XmNmarginWidth, 1);                               n++;
   XtSetArg (args[n], XmNmarginHeight, 1);                              n++;
   XtSetArg (args[n], XmNshadowThickness, 0);                           n++;
   XtSetArg (args[n], XmNautoUnmanage, False);                          n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);                n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);               n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);               n++;
   XtSetArg (args[n], XmNtopWidget, contentText);                       n++;
   XtSetArg (args[n], XmNtopOffset, 5);                                 n++;
   form1 = XmCreateForm (form, "form1", args, n);
   XtManageChild (form1);

    /* Create a Pulldown MenuPane that will contain the font sizes */
   followLinkPD = XmCreatePulldownMenu(form1, "fLinkPD", args, 0);

   label_string = XmStringCreateLocalized ((GETMESSAGE(15,33, "On")));
   XtSetArg(args[0], XmNmarginHeight, 2);
   XtSetArg(args[1], XmNmarginWidth, 12);
   XtSetArg(args[2], XmNlabelString, label_string); n++;
   find_rec->widgArry[0] =
                  XmCreatePushButtonGadget(followLinkPD, "On", args, 3);
   XmStringFree(label_string);

   label_string = XmStringCreateLocalized ((GETMESSAGE(15,34, "Off")));
   XtSetArg(args[2], XmNlabelString, label_string);
   find_rec->widgArry[1] =
                  XmCreatePushButtonGadget(followLinkPD, "Off", args, 3);
   XmStringFree(label_string);

   XtManageChildren(find_rec->widgArry, 2);

   label_string = XmStringCreateLocalized ((GETMESSAGE(15,35, "Follow Links: ")));
   /* create the Option Menu and attach it to the Pulldown MenuPane */
   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);                    n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);                n++;
   XtSetArg (args[n], XmNleftOffset, 30);                               n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);                 n++;
   XtSetArg (args[n], XmNtopOffset, 5);                                 n++;
   XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);              n++;
   XtSetArg (args[n], XmNbottomOffset, 5);                              n++;
   XtSetArg(args[n], XmNsubMenuId, followLinkPD); n++;
   XtSetArg(args[n], XmNmenuHistory, find_rec->widgArry[OFF]); n++;
   followLink = XmCreateOptionMenu(form1, "fLink", args, n);
   XtManageChild (followLink);
   XtAddCallback(followLink, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);

#endif

   n = 0;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
#if defined(sun)
   XtSetArg (args[n], XmNtopWidget, form1);		                n++;
#else
   XtSetArg (args[n], XmNtopWidget, contentText);	                n++;
#endif
   XtSetArg (args[n], XmNtopOffset, 5);					n++;
   outputSeparator = XmCreateSeparatorGadget (form, "opSeparator", args, n);
   XtManageChild (outputSeparator);

   /* Create the field for collecting the directory names to search */

   label_string = XmStringCreateLocalized (((char *)GETMESSAGE(15, 42, "Search Folder: ")));
   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNleftOffset, 5);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, outputSeparator);			n++;
   XtSetArg (args[n], XmNtopOffset, 15);				n++;
   XtSetArg (args[n], XmNtraversalOn, False);				n++;
   dirLabel = XmCreateLabelGadget (form, "folder_name_label", args, n);
   XtManageChild (dirLabel);
   XmStringFree (label_string);
   XtAddCallback(dirLabel, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);

   n = 0;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNleftWidget, dirLabel);				n++;
   XtSetArg (args[n], XmNleftOffset, 0);				n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNrightOffset, 10);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, outputSeparator);			n++;
   XtSetArg (args[n], XmNtopOffset, 10);				n++;
   dirName = XmCreateTextField (form, "folder_name_text", args, n);
   XtManageChild (dirName);
   XtAddCallback(dirName, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);

   /* set up translations in the search directory text edit widget */
   XtOverrideTranslations(dirName, trans_table);

   /* Create the widgets showing the matching files */

   n = 0;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, dirLabel);                          n++;
   XtSetArg (args[n], XmNtopOffset, 15);				n++;
   outputSeparator = XmCreateSeparatorGadget (form, "outputSeparator", args, n);
   XtManageChild (outputSeparator);

   label_string = XmStringCreateLocalized (GetSharedMessage(FILES_FOUND_LABEL));
   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNleftOffset, 5);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, outputSeparator);			n++;
   XtSetArg (args[n], XmNtopOffset, 10);				n++;
   XtSetArg (args[n], XmNtraversalOn, False);				n++;
   listLabel = XmCreateLabelGadget (form, "files_found", args, n);
   XtManageChild (listLabel);
   XmStringFree (label_string);
   XtAddCallback(listLabel, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);

   n = 0;
   XtSetArg (args[n], XmNlistSizePolicy, XmCONSTANT);			n++;
   XtSetArg (args[n], XmNscrollBarDisplayPolicy, XmSTATIC);		n++;
   XtSetArg (args[n], XmNvisibleItemCount, 5);				n++;
   scrolledList = XmCreateScrolledList (form, "file_list", args, n);
   XtManageChild (scrolledList);

   XtAddCallback (scrolledList, XmNbrowseSelectionCallback,
                                        SetActiveItem, (XtPointer) find_rec);
   XtAddCallback (scrolledList, XmNdefaultActionCallback,
                                              NewView, (XtPointer) find_rec);
   XtAddCallback(XtParent(scrolledList), XmNhelpCallback,
                 (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);


   n = 0;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, listLabel);				n++;
   XtSetArg (args[n], XmNtopOffset, 5);					n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNleftOffset, 10);				n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNrightPosition, 10);				n++;
   XtSetValues (XtParent (scrolledList), args, n);

   n = 0;
   XtSetArg (args[n], XmNmarginWidth, 1);                               n++;
   XtSetArg (args[n], XmNmarginHeight, 1);                              n++;
   XtSetArg (args[n], XmNshadowThickness, 0);                           n++;
   XtSetArg (args[n], XmNautoUnmanage, False);                          n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, scrolledList);			n++;
   XtSetArg (args[n], XmNtopOffset, 10);				n++;
   form2 = XmCreateForm (form, "form2", args, n);
   XtManageChild (form2);
/*
   XtAddCallback(form2, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);
*/

   label_string = XmStringCreateLocalized ((GETMESSAGE(15,48, "Open Folder")));

   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION);		n++;
   XtSetArg (args[n], XmNleftPosition, 10);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);			n++;
   XtSetArg (args[n], XmNtopOffset, 5);					n++;
   XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);              n++;
   XtSetArg (args[n], XmNbottomOffset, 5);                              n++;
   XtSetArg (args[n], XmNmarginHeight, 4);				n++;
   XtSetArg (args[n], XmNmarginWidth, 10);				n++;
   newFM = XmCreatePushButtonGadget (form2, "new_view", args, n);
   XtManageChild (newFM);
   XtAddCallback (newFM, XmNactivateCallback, NewView, (XtPointer) find_rec);
   XtAddCallback(newFM, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);
   XmStringFree (label_string);

   XtSetArg (args[0], XmNdefaultButton, newFM);
   XtSetValues (form2, args, 1);

   label_string = XmStringCreateLocalized ((GETMESSAGE(15,37, "Put In Workspace")));
   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);                    n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION);            n++;
   XtSetArg (args[n], XmNleftPosition, 55);                             n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);                 n++;
   XtSetArg (args[n], XmNtopOffset, 5);                                 n++;
   XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);              n++;
   XtSetArg (args[n], XmNbottomOffset, 5);                              n++;
   XtSetArg (args[n], XmNmarginHeight, 4);                              n++;
   XtSetArg (args[n], XmNmarginHeight, 4);                              n++;
   XtSetArg (args[n], XmNmarginWidth, 10);                              n++;
   putOnDT = XmCreatePushButtonGadget (form2, "putInWorkspace", args, n);
   XtManageChild (putOnDT);
   XtAddCallback (putOnDT, XmNactivateCallback, FindPutOnDesktop,
                  (XtPointer) find_rec);
   XtAddCallback(putOnDT, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);
   XmStringFree (label_string);

   /*  Create a separator between the buttons  */

   n = 0;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, form2);				n++;
   XtSetArg (args[n], XmNtopOffset, 10);				n++;
   separator =  XmCreateSeparatorGadget (form, "separator", args, n);
   XtManageChild (separator);


   /*  Create the action buttons  */

   label_string = XmStringCreateLocalized (((char *)GETMESSAGE(15, 14, "Start")));
   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION);		n++;
   XtSetArg (args[n], XmNleftPosition, 1);				n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);		n++;
   XtSetArg (args[n], XmNrightPosition, 24);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, separator);				n++;
   XtSetArg (args[n], XmNtopOffset, 5);					n++;
   XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);		n++;
   XtSetArg (args[n], XmNbottomOffset, 5);				n++;
   XtSetArg (args[n], XmNmarginHeight, 4);				n++;
   start = XmCreatePushButtonGadget (form, "start", args, n);
   XtAddCallback (start, XmNactivateCallback, StartCallback,
							(XtPointer) find_rec);
   XtAddCallback(start, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);
   XtManageChild (start);
   XmStringFree (label_string);

   label_string = XmStringCreateLocalized (((char *)GETMESSAGE(15, 15, "Stop")));
   n = 0;
   XtSetArg (args[n], XmNlabelString, label_string);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION);		n++;
   XtSetArg (args[n], XmNleftPosition, 26);				n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);		n++;
   XtSetArg (args[n], XmNrightPosition, 49);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, separator);				n++;
   XtSetArg (args[n], XmNtopOffset, 5);					n++;
   XtSetArg (args[n], XmNmarginHeight, 4);				n++;
   stop = XmCreatePushButton (form, "stop", args, n);
   XtAddCallback (stop, XmNactivateCallback, StopCallback,
						   (XtPointer) find_rec);
   XtAddCallback(stop, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);
   XtManageChild (stop);
   XtSetSensitive (stop, False);
   XmStringFree (label_string);

   n = 0;
   XtSetArg (args[n], XmNlabelString, cancelXmString);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION);		n++;
   XtSetArg (args[n], XmNleftPosition, 51);				n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);		n++;
   XtSetArg (args[n], XmNrightPosition, 74);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, separator);				n++;
   XtSetArg (args[n], XmNtopOffset, 5);					n++;
   XtSetArg (args[n], XmNmarginHeight, 4);				n++;
   close = XmCreatePushButtonGadget (form, "close", args, n);
   XtManageChild (close);
   XtAddCallback(close, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);


   n = 0;
   XtSetArg (args[n], XmNlabelString, helpXmString);			n++;
   XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION);		n++;
   XtSetArg (args[n], XmNleftPosition, 76);				n++;
   XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);		n++;
   XtSetArg (args[n], XmNrightPosition, 99);				n++;
   XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);		n++;
   XtSetArg (args[n], XmNtopWidget, separator);				n++;
   XtSetArg (args[n], XmNtopOffset, 5);					n++;
   XtSetArg (args[n], XmNmarginHeight, 4);				n++;
   help = XmCreatePushButton (form, "help", args, n);
   XtManageChild (help);
   XtAddCallback(help, XmNactivateCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);
   XtAddCallback(help, XmNhelpCallback, (XtCallbackProc)HelpRequestCB,
                 HELP_FIND_DIALOG_STR);



   XtSetArg (args[0], XmNdefaultButton, start);
   XtSetArg (args[1], XmNcancelButton, close);
   XtSetValues (form, args, 2);

   /* Fill in our instance structure */

   find_rec->shell = shell;
   find_rec->form = form;
   find_rec->fileNameFilter = filterText;
   find_rec->content = contentText;
#if defined(sun)
   find_rec->followLink = followLink;
#else
   find_rec->followLink = NULL;
#endif
   find_rec->listLabel = listLabel;
   find_rec->matchList = scrolledList;
   find_rec->searchDirectory = dirName;
   find_rec->newFM = newFM;
   find_rec->putOnDT = putOnDT;
   find_rec->start = start;
   find_rec->stop = stop;
   find_rec->close = close;
   find_rec->help = help;

   find_rec->selectedItem = -1;
   find_rec->pipeFd = -1;
   find_rec->childpid = -1;
   find_rec->alternateInputId = 0;
   find_rec->searchInProgress = False;
   find_rec->searchPhase = PHASE_NAMES;
   find_rec->lineBuf = NULL;
   find_rec->lineLen = find_rec->lineSize = 0;
   find_rec->grepDirs = NULL;
   find_rec->grepCount = find_rec->grepSize = find_rec->grepNext = 0;
   find_rec->grepName = NULL;
   find_rec->grepContent = NULL;
   find_rec->grepGlob = NULL;
   find_rec->grepGlobNext = 0;
   find_rec->fileMgrRec = NULL;


   /*  Set the return values for the dialog widget and dialog instance.  */

   *return_widget = form;
   *dialog = (XtPointer) find_rec;
}



/************************************************************************
 *
 *  InstallChange
 *
 ************************************************************************/

static void
InstallChange(
        FindRec *find_rec,
        XtCallbackProc callback,
        XtPointer client_data )
{
   FindApply * apply_data;


   /*  Setup the callback data to be sent to the most of our */
   /*  actions callbacks.                                    */

   apply_data = (FindApply *) XtMalloc (sizeof (FindApply));
   apply_data->callback = callback;
   apply_data->client_data = client_data;
   apply_data->find_rec = (XtPointer) find_rec;
   find_rec->apply_data = apply_data;
}




/************************************************************************
 *
 *  InstallClose
 *
 ************************************************************************/

static void
InstallClose(
        FindRec *find_rec,
        XtCallbackProc callback,
        XtPointer client_data )
{
   Atom delete_window_atom;

   XtAddCallback (find_rec->close, XmNactivateCallback, callback, client_data);

   delete_window_atom = XmInternAtom (XtDisplay(find_rec->shell), "WM_DELETE_WINDOW", True);
   XmRemoveWMProtocols( find_rec->shell, &delete_window_atom, 1 );
   XmAddWMProtocolCallback( find_rec->shell, delete_window_atom, callback,
                            (XtPointer) client_data );
}




/************************************************************************
 *
 *  Destroy
 *
 ************************************************************************/

static void
Destroy(
        FindRec *find_rec )
{
   EndSearchProcess (find_rec, True);
   FreeGrepState (find_rec);
   XtFree (find_rec->lineBuf);
   XtDestroyWidget (find_rec->shell);
   XtFree ((char *) find_rec->apply_data);
   XtFree ((char *) find_rec);
}




/************************************************************************
 *
 *  GetValues
 *
 ************************************************************************/

static XtPointer
GetValues(
        FindRec *find_rec )
{
   FindData * find_data;
   Arg args[4];


   /*  Allocate and initialize the find file dialog data.  */

   find_data = (FindData *) XtMalloc (sizeof (FindData));

   find_data->displayed = True;

   XtSetArg (args[0], XmNx, &find_data->x);
   XtSetArg (args[1], XmNy, &find_data->y);
   XtSetArg (args[2], XmNwidth, &find_data->width);
   XtSetArg (args[3], XmNheight, &find_data->height);
   XtGetValues (find_rec->shell, args, 4);

   (void) GetFindValues (find_rec, find_data, False);

   return ((XtPointer) find_data);
}




/************************************************************************
 *
 *  GetDefaultValues
 *
 ************************************************************************/

static XtPointer
GetDefaultValues( void )
{
   FindData *find_data;
   char     dirbuf[MAX_DIR_PATH_LEN];

   /*  Allocate and initialize the default find file dialog data.  */

   find_data = (FindData *) XtMalloc (sizeof (FindData));
   if (!find_data)
       return NULL;

   find_data->displayed = False;
   find_data->x = 0;
   find_data->y = 0;
   find_data->height = 0;
   find_data->width = 0;

   if (!getcwd((char *)dirbuf, (unsigned int)MAX_DIR_PATH_LEN))
   {
       XtFree((char *)find_data);
       return NULL;
   }
   if(restrictMode &&
           strncmp(users_home_dir, dirbuf, strlen(users_home_dir)) != 0)
   {
      find_data->directories = (char *)XtMalloc(strlen(users_home_dir) + 1);
      strcpy(find_data->directories, users_home_dir);
   }
   else
   {
      find_data->directories = (char *)XtMalloc(strlen(dirbuf) + 1);
      strcpy(find_data->directories, dirbuf);
   }

   find_data->filter = NULL;
   find_data->content = NULL;
   find_data->matches = NULL;
   find_data->num_matches = 0;
   find_data->selected_item = -1;
#if defined(sun)
   find_data->follow_links = follow_links;
#endif

   return ((XtPointer) find_data);
}




/************************************************************************
 *
 *  GetResourceValues
 *
 ************************************************************************/

static XtPointer
GetResourceValues(
        XrmDatabase data_base,
        char **name_list )
{
   FindData * find_data;
   Dummy dummy;


   /*  Allocate and get the resources for find file dialog data.  */

   find_data = (FindData *) XtCalloc (1, sizeof (FindData));

   _DtDialogGetResources (data_base, name_list, FIND_FILE, (char *) find_data,
                       resources, findClass->resource_count);

   /* Create new strings for any string value we read in from our
    * resourceDB.
    */
   find_data->directories = XtNewString(find_data->directories);
   find_data->filter = XtNewString(find_data->filter);
   find_data->content = XtNewString(find_data->content);

   /*  Do a special read to get the list of matches.                 */
   /*  The information is read in as a single string, and then must  */
   /*  be converted into the internal array format.                  */

   _DtDialogGetResources (data_base, name_list, FIND_FILE, (char *) (&dummy),
                       match_resources, XtNumber(match_resources));

   CvtStringToStringList (dummy.string, &find_data->matches,
                          &find_data->num_matches);

   return ((XtPointer) find_data);
}




/************************************************************************
 *
 *  SetValues
 *
 ************************************************************************/

static void
SetValues(
        FindRec *find_rec,
        FindData *find_data )
{
   Arg args[2];
   XmString * matches;
   int i;

   XmTextFieldSetString (find_rec->fileNameFilter, find_data->filter);
   XmTextFieldSetInsertionPosition(find_rec->fileNameFilter,
                        XmTextFieldGetLastPosition(find_rec->fileNameFilter));
   XmTextFieldSetString(find_rec->content, find_data->content);
   XmTextFieldSetInsertionPosition(find_rec->content,
                        XmTextFieldGetLastPosition(find_rec->content));

   if(find_data->directories)
      XtFree(find_data->directories);
   find_data->directories =
                 XtNewString(find_data->file_mgr_data->current_directory );

   XmTextFieldSetString(find_rec->searchDirectory, find_data->directories);
   XmTextFieldSetInsertionPosition(find_rec->searchDirectory,
                        XmTextFieldGetLastPosition(find_rec->searchDirectory));
   if( find_data->file_mgr_data->restricted_directory )
   {
      XtAddCallback (find_rec->searchDirectory, XmNmodifyVerifyCallback,
                     (XtCallbackProc)TextChange,
                     (XtPointer)find_data->file_mgr_data );
      XtAddCallback (find_rec->searchDirectory, XmNmotionVerifyCallback,
                     (XtCallbackProc)TextChange,
                     (XtPointer)find_data->file_mgr_data );
   }

   /* Update the list of matches */

   if (find_data->num_matches > 0)
   {
      XtSetArg (args[0], XmNitemCount, find_data->num_matches);
      matches =
         (XmString *) XtMalloc (sizeof (XmString) * find_data->num_matches);

      for (i = 0; i < find_data->num_matches; i++)
      {
         matches[i] = XmStringCreateLocalized (find_data->matches[i]);
      }

      XtSetArg (args[1], XmNitems, matches);
      XtSetValues (find_rec->matchList, args, 2);

      for (i = 0; i < find_data->num_matches; i++)
         XmStringFree (matches[i]);

      XtFree ((char *) matches);

      if (find_data->selected_item != -1)
      {
         XmListSelectPos (find_rec->matchList,
                          find_data->selected_item + 1,False);
      }
   }
   else
   {
      XtSetArg (args[0], XmNitemCount, 0);
      XtSetValues (find_rec->matchList, args, 1);
   }

#if defined(sun)
   /* Set up the Follow links option menu */
   if(find_data->follow_links)
      XtSetArg(args[0], XmNmenuHistory, find_rec->widgArry[ON]);
   else
      XtSetArg(args[0], XmNmenuHistory, find_rec->widgArry[OFF]);
   XtSetValues (find_rec->followLink, args, 1);
#endif

   /* Update button sensitivity, if one of the matches is selected */

   if (find_data->selected_item != -1)
   {
      XtSetSensitive (find_rec->newFM, True);
      XtSetSensitive (find_rec->putOnDT, True);
   }
   else
   {
      XtSetSensitive (find_rec->newFM, False);
      XtSetSensitive (find_rec->putOnDT, False);
   }

   find_rec->selectedItem = find_data->selected_item;

}




/************************************************************************
 *
 *  WriteResourceValues
 *
 ************************************************************************/

static void
WriteResourceValues(
        DialogData *values,
        int fd,
        char **name_list )
{
   FindData * find_data = (FindData *) values->data;
   FindRec  * find_rec;
   Dummy dummy;


   /*  If the dialog is currently displayed, update the geometry  */
   /*  fields to their current values.                            */

   if (find_data->displayed == True)
   {
      _DtGenericUpdateWindowPosition(values);
      find_rec = (FindRec *) _DtGetDialogInstance (values);
      (void) GetFindValues (find_rec, find_data, False);
   }

   _DtDialogPutResources (fd, name_list, FIND_FILE, (char *) values->data,
                       resources, findClass->resource_count);


   /*  Special case for writing the array of matching files.          */
   /*  Write all of the information out as a single string, which     */
   /*  we'll parse when we read it back in at a later point in time.  */
   /*  The format for this string is:                                 */
   /*                                                                 */
   /*     <match string1>, <match string2>, <match string3>, ...      */

   if (find_data->num_matches > 0)
   {
      dummy.displayed = False;
      dummy.string = CvtStringListToString (find_data->matches,
                                            find_data->num_matches);

      _DtDialogPutResources (fd, name_list, FIND_FILE, (char *) &dummy,
                          match_resources, XtNumber (match_resources));

      XtFree ((char *) dummy.string);
      dummy.string = NULL;
   }
}




/************************************************************************
 *
 *  FreeValues
 *
 ************************************************************************/

static void
FreeValues(
        FindData *find_data )
{
   if( find_data )
   {
      if( find_data->filter )
         XtFree ((char *) find_data->filter);
      if( find_data->directories )
         XtFree ((char *) find_data->directories);
      FreeMatchInfo (find_data->matches, find_data->num_matches);
      XtFree ((char *) find_data);
   }
}


/************************************************************************
 *
 *  GetFindValues
 *	Update the current find file values within the data structure
 *	from the current values.
 *
 ************************************************************************/

static Boolean
GetFindValues(
        FindRec *find_rec,
        FindData *find_data,
        Boolean validate )
{
   Arg args[2];
   int i;
   XmString * stringTable;


   /* Get the filename to search for (e.g. Filter) */

   find_data->displayed = True;
   find_data->content = XmTextFieldGetString (find_rec->content);
   find_data->filter = (char *)_DtStripSpaces (
                              XmTextFieldGetString (find_rec->fileNameFilter));
   if (validate)
   {
     if ((strlen (find_data->filter) == 0) &&
					 (strcmp(find_data->content, "") == 0))
      {
         XtFree ((char *) find_data->filter);
         find_data->filter = NULL;
      }
   }

   /* Get the list of directories to search */
   if(find_data->directories)
      XtFree(find_data->directories);
   find_data->directories = XmTextFieldGetString (find_rec->searchDirectory);

   if (validate)
   {
      find_data->directories = (char *)_DtStripSpaces (find_data->directories);

      if (strlen (find_data->directories) == 0)
      {
         /* A directory must be supplied */

         InvalidFindMessage (find_rec, BAD_DIR_NAME, NULL);
         XtFree ((char *) find_data->filter);
         find_data->filter = NULL;
         XtFree ((char *) find_data->directories);
         find_data->directories = NULL;
         return (False);
      }
   }

   /* Make a copy of the array of matching strings */

   find_data->selected_item = find_rec->selectedItem;
   XtSetArg (args[0], XmNitemCount, &find_data->num_matches);
   XtSetArg (args[1], XmNitems, &stringTable);
   XtGetValues (find_rec->matchList, args, 2);
   if (find_data->num_matches == 0)
   {
      find_data->matches = NULL;
   }
   else
   {
      find_data->matches =
         (String *) XtMalloc (sizeof(String) * find_data->num_matches);

      for (i = 0; i < find_data->num_matches; i++)
      {
         find_data->matches[i] = (char *) _XmStringUngenerate(stringTable[i],
                                                  XmFONTLIST_DEFAULT_TAG,
                                                  XmCHARSET_TEXT, XmCHARSET_TEXT);
      }
   }

#if defined(sun)
   {
      Widget menuHistory;

      XtSetArg (args[0], XmNmenuHistory, &menuHistory);
      XtGetValues (find_rec->followLink, args, 1);

      if(menuHistory == find_rec->widgArry[ON])
         find_data->follow_links = True;
      else
         find_data->follow_links = False;
   }
#endif

   return (True);
}




/************************************************************************
 *
 *  InvalidFindMessage
 *	Display an error message.
 *
 ************************************************************************/

static void
InvalidFindMessage(
        FindRec * find_rec,
        int messageIndex,
        String extra_string )
{
   String string = NULL;
   String new_string = NULL;
   char * title = NULL;
   static String badDirectoryNameMessage = NULL;
   static String noDirectoryAccessMessage = NULL;
   static String noSearchArgumentMessage = NULL;
   static String noExistanceMessage = NULL;
   char * tmpStr = NULL;

   if (noExistanceMessage == NULL)
   {
      tmpStr = GETMESSAGE(15,44, "Search Folder name argument is missing.\nType in the name of the folder where you want the search to begin.");
      badDirectoryNameMessage = XtNewString(tmpStr);

      tmpStr = GetSharedMessage(NO_DIR_ACCESS_ERROR);
      noDirectoryAccessMessage = XtNewString(tmpStr);

      tmpStr = GETMESSAGE(15, 50, "Search Folder name or File Content argument is missing\nType in the name of the folder where you want the search to begin.\nOr type in the string that you want to search.");
      noSearchArgumentMessage = XtNewString(tmpStr);

      tmpStr = GETMESSAGE(15,45, "The selected file no longer exists.\n\nSomeone deleted the file after the search process completed.");
      noExistanceMessage = XtNewString(tmpStr);
   }


   switch (messageIndex)
   {
      case BAD_DIR_NAME:
           string = badDirectoryNameMessage;
           break;
      case NO_DIR_ACCESS:
           string = noDirectoryAccessMessage;
           break;
      case NO_EXISTANCE:
           string = noExistanceMessage;
           break;
       case NO_FILE_OR_FOLDER_ARG:
           string = noSearchArgumentMessage;
           break;
   }

   if (extra_string && string)
   {
      new_string = XtMalloc (strlen(string) + strlen(extra_string) + 1);
      (void) sprintf(new_string, string, extra_string);
      tmpStr = GetSharedMessage(FIND_ERROR_TITLE);
      title = XtNewString(tmpStr);
      _DtMessage (find_rec->shell, title, new_string, NULL, HelpRequestCB);
      XtFree ((char *) new_string);
      XtFree(title);
   }
   else
   {
      tmpStr = GetSharedMessage(FIND_ERROR_TITLE);
      title = XtNewString(tmpStr);
      _DtMessage (find_rec->shell, title, string, NULL, HelpRequestCB);
      XtFree(title);
   }

   _DtTurnOffHourGlass (find_rec->shell);
}




/************************************************************************
 *
 *  FreeMatchInfo()
 *	Free up the space occupied by the array containing the match
 *      strings.
 *
 ************************************************************************/

static void
FreeMatchInfo(
        String *matches,
        int numMatches )
{
   int i;

   if (matches == NULL)
      return;

   for (i = 0; i < numMatches; i++)
      XtFree ((char *) matches[i]);


   XtFree ((char *) matches);
}



/************************************************************************
 *
 *  StopSearch()
 *	Abort an active search operation.
 *
 ************************************************************************/

static void
StopSearch(
        Widget w,
        FindRec *find_rec )
{
   Arg args[2];


   /* To avoid a race condition where the input processing routine   */
   /* detected the end of the find operation and cleaned things up,  */
   /* just as the user hit the 'stop' key, we need to check to see   */
   /* if the operation is still active.                              */

   if (find_rec->pipeFd >= 0)
   {
      /* Abort the search process, and remove the alternate input handler */
      EndSearchProcess (find_rec, True);
      find_rec->searchInProgress = False;
   }
   FreeGrepState (find_rec);


   /* Change button sensitivities */

   XtSetSensitive (find_rec->start, True);
   XtSetSensitive (find_rec->close, True);
   XtSetSensitive (find_rec->stop, False);

   XtSetArg (args[0], XmNdefaultButton, find_rec->start);
   XtSetValues (find_rec->form, args, 1);

   /*
    */

   XtRemoveEventHandler (find_rec->stop, LeaveWindowMask, FALSE, (XtEventHandler)LeaveStopBttn, find_rec);
   XtRemoveEventHandler (find_rec->stop, EnterWindowMask, FALSE, (XtEventHandler)EnterStopBttn, find_rec);

   _DtTurnOffHourGlass (find_rec->shell);
   XmUpdateDisplay (w);
}




/************************************************************************
 *
 *  StartSearch()
 *	Start an active search operation.
 *
 ************************************************************************/

static void
StartSearch(
        Widget w,
        FindRec *find_rec )
{
   FindData * find_data;
   Arg args[3];

   _DtTurnOnHourGlass (find_rec->shell);

   /* Extract current dialog values; continue only if data is valid */

   find_data = (FindData *) XtMalloc (sizeof (FindData));

   if (!GetFindValues (find_rec, find_data, True))
   {
      /* Dialog contained bogus values; abort request */

      XtFree ((char *) find_data);
      return;
   }


   /* Desensitize buttons we don't want working during a search */

   XtSetSensitive (find_rec->stop, True);
   XtSetSensitive (find_rec->start, False);
   XtSetSensitive (find_rec->close, False);
   XtSetSensitive (find_rec->newFM, False);
   XtSetSensitive (find_rec->putOnDT, False);

   XtSetArg (args[0], XmNdefaultButton, find_rec->stop);
   XtSetArg (args[1], XmNcancelButton, find_rec->close);
   XtSetValues (find_rec->form, args, 2);

   /* Clean out the match list */

   find_rec->selectedItem = -1;
   XtSetArg (args[0], XmNitemCount, 0);
   XtSetValues (find_rec->matchList, args, 1);

   XmUpdateDisplay (w);


   /* Start the find process; not much we can say if it fails! */

   if (!FindProcessStarted (find_rec, find_data))
   {
      XtSetSensitive (find_rec->close, True);
      XtSetSensitive (find_rec->newFM, False);
      XtSetSensitive (find_rec->putOnDT, False);
      XtSetSensitive (find_rec->start, True);
      XtSetSensitive (find_rec->stop, False);

      XtSetArg (args[0], XmNdefaultButton, find_rec->start);
      XtSetArg (args[1], XmNcancelButton, find_rec->close);
      XtSetValues (find_rec->form, args, 2);

      XmUpdateDisplay (w);
   }
   else
   {
      find_rec->searchInProgress = True;

      /* This event handlers will be called when user is moving his mouse
         over the Find dialog's stop button.
      */
      XtAddEventHandler( find_rec->stop, LeaveWindowMask, FALSE, (XtEventHandler)LeaveStopBttn, find_rec );
      XtAddEventHandler( find_rec->stop, EnterWindowMask, FALSE, (XtEventHandler)EnterStopBttn, find_rec );
   }

   /* Free up the dialog values we allocated */

   FreeValues (find_data);
}


/************************************************************************
 *
 *  EnterStopBttn()
 *
 ************************************************************************/

static void
EnterStopBttn(
              Widget w,
              FindRec * find_rec,
              XEvent * event )
{
  _DtTurnOffHourGlass (find_rec->shell);
}


/************************************************************************
 *
 *  LeaveStopBttn()
 *
 ************************************************************************/

static void
LeaveStopBttn(
              Widget w,
              FindRec * find_rec,
              XEvent * event )
{
  _DtTurnOnHourGlass (find_rec->shell);
}


/************************************************************************
 *
 *  StartCallback()
 *	Start a search operation.
 *
 ************************************************************************/

static void
StartCallback(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
   StartSearch (w, (FindRec *) client_data);
}




/************************************************************************
 *
 *  StopCallback
 *	Stop an active search operation.
 *
 ************************************************************************/

static void
StopCallback(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
{
   StopSearch (w, (FindRec *) client_data);
}








/************************************************************************
 *
 *  FindProcessStarted()
 *      Determine whether to do a 'find' or a 'grep'.
 *
 ************************************************************************/

static Boolean
FindProcessStarted(
        FindRec *find_rec,
        FindData *find_data )
{
   FileMgrData * file_mgr_data;
   DialogData * dialog_data;

   dialog_data = _DtGetInstanceData ((XtPointer) (find_rec->fileMgrRec));
   file_mgr_data = (FileMgrData *) dialog_data->data;

   return(ExecuteFind(find_rec, find_data, file_mgr_data));

}

/************************************************************************
 *
 *  ExecuteFind()
 *	Create the command string for invoking the 'find' process,
 *      and then execute it.
 *
 ************************************************************************/

static Boolean
ExecuteFind(
   FindRec * find_rec,
   FindData * find_data,
   FileMgrData *file_mgr_data)
{
   String findptr;
   String host;
   String path;
   char *argv[10];
   int n = 0;
   int rc;
#if defined (SVR4) || defined(_AIX) || \
    !(defined(__linux__) || defined(CSRG_BASED) || defined(BLS))
   int access_priv;
#endif
   XmString label_string;
   char *tmpStr;
   Arg args[1];
#if defined (SVR4)  || defined(_AIX)
/* needed for getaccess () call */
   int save_ruid;
   int save_rgid;
#endif /* SVR4 */
   char *link_path;

   if(strcmp(find_data->content, "") == 0)
   {
      label_string = XmStringCreateLocalized (GetSharedMessage(FILES_FOUND_LABEL));
   }
   else if(strcmp(find_data->filter, "") == 0)
   {
      tmpStr = GETMESSAGE(15,38, "Files Found (by Contents):");
      label_string = XmStringCreateLocalized (tmpStr);
   }
   else
   {
      tmpStr = (GETMESSAGE(15,39, "Files Found (by Name and Contents):"));
      label_string = XmStringCreateLocalized (tmpStr);
   }
   XtSetArg (args[0], XmNlabelString, label_string);
   XtSetValues (find_rec->listLabel, args, 1);
   XmStringFree(label_string);

   if(find_data->filter == NULL )
   {
     InvalidFindMessage (find_rec, NO_FILE_OR_FOLDER_ARG, NULL);
     return( False );
   }


   /* Convert directory names from external to internal (nfs) format */

   findptr = find_data->directories;


   /*  Search for the end of the directory component  */

   _DtPathFromInput(findptr, file_mgr_data->current_directory, &host, &path);

   if (path == NULL)
   {
     InvalidFindMessage (find_rec, NO_DIR_ACCESS, findptr);
     return( False );
   }

   link_path = _DtFollowLink(path);
   XtFree(path);
   path = XtNewString(link_path);

   if(path == NULL)
      return False;
   /* Verify that the path exists and is accessible */
#if defined (SVR4)  || defined(_AIX)
/* needed for getaccess () call */
   save_ruid = getuid();
#if !defined(SVR4)
   setreuid(geteuid(),-1);
#else
   setuid(geteuid());
#endif
   save_rgid = getgid();
#if !defined(SVR4)
   setregid(getegid(),-1);
#else
   setgid(getegid());
#endif
   access_priv = access (path, R_OK);
#if !defined(SVR4)
   setreuid(save_ruid,-1);
   setregid(save_rgid,-1);
#else
   setuid(save_ruid);
   setgid(save_rgid);
#endif


   if (access_priv == -1 && geteuid() != root_user)
   {
#else
#  if defined(__linux__) || defined(CSRG_BASED)
   setreuid(geteuid(),-1);
   if (access ((char *) path, R_OK) == -1)
   {
#  else
#    ifdef BLS
   setresuid(geteuid(),-1,-1);
   if (access ((char *) path, R_OK) == -1)
   {
#    else
   if ((((access_priv = getaccess (path, UID_EUID, NGROUPS_EGID_SUPP,
                                      0, (void *) 0, (void *) 0)) == -1) ||
             !(access_priv & R_OK)) && (geteuid () != root_user))
   {
#    endif /* BLS */
#  endif /* Apollo */
#endif /* SVR4 */
      /* Post an error dialog, and then terminate the request */

      InvalidFindMessage (find_rec, NO_DIR_ACCESS, findptr);
      XtFree ((char *) path);
      return (False);
   }


   /*
    * Run find(1) directly, with the path and pattern as arguments.  This
    * used to be "ksh -c" with a command line, which split paths at
    * blanks and let the shell expand $, ` and \ in the pattern.
    */
   argv[n++] = "find";
   argv[n++] = path;

   if(strcmp(find_data->content, "") != 0)
   {
      /* search by contents: find the folders, then grep in each */
      argv[n++] = "-type";
      argv[n++] = "d";
      find_rec->searchPhase = PHASE_DIRS;
      FreeGrepState (find_rec);
      find_rec->grepName = XmTextFieldGetString (find_rec->fileNameFilter);
      if (strcmp (find_rec->grepName, "") == 0)
      {
         XtFree (find_rec->grepName);
         find_rec->grepName = XtNewString ("*");
      }
      find_rec->grepContent = XmTextFieldGetString (find_rec->content);
   }
   else
   {
      /* File name pattern */
      find_rec->searchPhase = PHASE_NAMES;
      if (find_data->filter)
      {
         argv[n++] = "-name";
         argv[n++] = find_data->filter;
      }
   }

#if defined(sun)
   {
      Widget menuHistory;

      XtSetArg (args[0], XmNmenuHistory, &menuHistory);
      XtGetValues (find_rec->followLink, args, 1);

      if(menuHistory == find_rec->widgArry[ON])
      {
         /* Add the option to follow a link */
         argv[n++] = "-follow";
      }
   }
#endif

   argv[n++] = "-print";
   argv[n] = NULL;

   /* Start the 'find' process, and read its output as it comes */

   rc = SpawnSearchProcess (find_rec, argv);
   XtFree ((char *) path);
   if (rc != 0)
   {
      FreeGrepState (find_rec);
      return (False);
   }

   return (True);
}


/************************************************************************
 *
 *  SpawnSearchProcess()
 *	Start find(1) or grep(1) with the given arguments (no shell), its
 *	output going to a pipe that SearchInputHandler reads.  Error
 *	messages are discarded, as the "2>&-" of the old shell commands
 *	did.  Returns 0, or -1 if the process could not be started.
 *
 ************************************************************************/

static int
SpawnSearchProcess(
        FindRec *find_rec,
        char **argv )
{
   posix_spawn_file_actions_t actions;
   posix_spawnattr_t attr;
   sigset_t sigdefault;
   int fds[2];
   pid_t pid;
   int rc;

   if (pipe (fds) < 0)
      return -1;

   posix_spawn_file_actions_init (&actions);
   posix_spawn_file_actions_addclose (&actions, fds[0]);
   if (fds[1] != STDOUT_FILENO)
   {
      posix_spawn_file_actions_adddup2 (&actions, fds[1], STDOUT_FILENO);
      posix_spawn_file_actions_addclose (&actions, fds[1]);
   }
   posix_spawn_file_actions_addopen (&actions, STDERR_FILENO, "/dev/null",
                                     O_WRONLY, 0);

   /* signals dtfile ignores must not stay ignored in the child: find has
    * to die of SIGPIPE when the search is stopped */
   posix_spawnattr_init (&attr);
   sigemptyset (&sigdefault);
   sigaddset (&sigdefault, SIGPIPE);
   sigaddset (&sigdefault, SIGCHLD);
   sigaddset (&sigdefault, SIGINT);
   sigaddset (&sigdefault, SIGQUIT);
   sigaddset (&sigdefault, SIGTERM);
   posix_spawnattr_setsigdefault (&attr, &sigdefault);
   posix_spawnattr_setflags (&attr, POSIX_SPAWN_SETSIGDEF);

   rc = posix_spawnp (&pid, argv[0], &actions, &attr, argv, environ);

   posix_spawnattr_destroy (&attr);
   posix_spawn_file_actions_destroy (&actions);
   close (fds[1]);
   if (rc != 0)
   {
      close (fds[0]);
      return -1;
   }

   DPRINTF(("SpawnSearchProcess: %s started, pid %d\n", argv[0], (int) pid));

   /* the input callback reads what is there, and never blocks */
   (void) fcntl (fds[0], F_SETFL, fcntl (fds[0], F_GETFL) | O_NONBLOCK);
   (void) fcntl (fds[0], F_SETFD, FD_CLOEXEC);

   find_rec->pipeFd = fds[0];
   find_rec->childpid = pid;
   find_rec->lineLen = 0;
   find_rec->alternateInputId =
      XtAppAddInput (XtWidgetToApplicationContext (find_rec->shell),
                     fds[0], (XtPointer) XtInputReadMask,
                     (XtInputCallbackProc) SearchInputHandler, find_rec);
   return 0;
}


/************************************************************************
 *
 *  EndSearchProcess()
 *	Stop reading the search process; optionally terminate it.
 *
 ************************************************************************/

static void
EndSearchProcess(
        FindRec *find_rec,
        Boolean kill_it )
{
   if (find_rec->alternateInputId)
   {
      XtRemoveInput (find_rec->alternateInputId);
      find_rec->alternateInputId = 0;
   }
   if (find_rec->pipeFd >= 0)
   {
      close (find_rec->pipeFd);
      find_rec->pipeFd = -1;
   }
   if (kill_it && find_rec->childpid > 1)  /* trying to be safe */
      kill (find_rec->childpid, SIGTERM);  /* Ignore errors */
   find_rec->childpid = -1;
   find_rec->lineLen = 0;
}


/************************************************************************
 *
 *  FreeGrepState()
 *	Forget the folders and patterns of a search by contents.
 *
 ************************************************************************/

static void
FreeGrepState(
        FindRec *find_rec )
{
   int i;

   for (i = 0; i < find_rec->grepCount; i++)
      XtFree (find_rec->grepDirs[i]);
   XtFree ((char *) find_rec->grepDirs);
   find_rec->grepDirs = NULL;
   find_rec->grepCount = find_rec->grepSize = find_rec->grepNext = 0;

   XtFree (find_rec->grepName);
   find_rec->grepName = NULL;
   XtFree (find_rec->grepContent);
   find_rec->grepContent = NULL;

   if (find_rec->grepGlob)
   {
      globfree ((glob_t *) find_rec->grepGlob);
      XtFree ((char *) find_rec->grepGlob);
      find_rec->grepGlob = NULL;
   }
   find_rec->grepGlobNext = 0;
}


/************************************************************************
 *
 *  FinishSearch()
 *	The search is complete: reset the dialog, report if nothing
 *	was found, else select the first match.
 *
 ************************************************************************/

static void
FinishSearch(
        FindRec *find_rec )
{
   Arg args[1];
   int item_count;
   char * title;
   char * msg;
   char * tmpStr;

   FreeGrepState (find_rec);
   XtRemoveEventHandler (find_rec->stop, LeaveWindowMask, FALSE, (XtEventHandler)LeaveStopBttn, find_rec);
   XtRemoveEventHandler (find_rec->stop, EnterWindowMask, FALSE, (XtEventHandler)EnterStopBttn, find_rec);

   find_rec->searchInProgress = False;

   /* Reset button sensitivity */

   XtSetSensitive (find_rec->close, True);
   XtSetSensitive (find_rec->start, True);
   XtSetSensitive (find_rec->stop, False);

   XtSetArg (args[0], XmNitemCount, &item_count);
   XtGetValues (find_rec->matchList, args, 1);

   XtSetArg (args[0], XmNdefaultButton, find_rec->start);
   XtSetValues (find_rec->form, args, 1);

   if (item_count == 0)
   {
      tmpStr = GetSharedMessage(FIND_ERROR_TITLE);
      title = XtNewString(tmpStr);
      tmpStr = GetSharedMessage(NO_FILES_FOUND_ERROR);
      msg = XtNewString(tmpStr);
      _DtMessage (find_rec->shell, title, msg, NULL, HelpRequestCB);
      XtFree(title);
      XtFree(msg);
   }
   else
   {
      XmListSelectPos(find_rec->matchList, 1, True);
      XmProcessTraversal(find_rec->matchList, XmTRAVERSE_CURRENT);
   }

   _DtTurnOffHourGlass (find_rec->shell);
}


/************************************************************************
 *
 *  StartNextGrep()
 *	Search by contents: start grep(1) on the next files that match the
 *	file name pattern in the folders find(1) listed.  This used to be
 *	"grep ... folder/pattern" through a shell, one per folder; glob()
 *	expands the pattern as the shell did (no dot files unless the
 *	pattern asks for them), and folder names are passed unchanged.
 *	When no folder is left, the search is complete.
 *
 ************************************************************************/

static void
StartNextGrep(
        FindRec * find_rec )
{
   glob_t *g;
   char *pattern;
   char **argv;
   size_t bytes;
   int n;

   for (;;)
   {
      g = (glob_t *) find_rec->grepGlob;
      if (g == NULL || find_rec->grepGlobNext >= g->gl_pathc)
      {
         /* expand the pattern in the next folder */
         if (g)
         {
            globfree (g);
            XtFree ((char *) g);
            find_rec->grepGlob = NULL;
         }
         if (find_rec->grepNext >= find_rec->grepCount)
         {
            FinishSearch (find_rec);
            return;
         }

         pattern = XtMalloc (strlen (find_rec->grepDirs[find_rec->grepNext]) +
                             strlen (find_rec->grepName) + 2);
         sprintf (pattern, "%s/%s", find_rec->grepDirs[find_rec->grepNext],
                  find_rec->grepName);
         find_rec->grepNext++;

         g = XtNew (glob_t);
         memset (g, 0, sizeof (glob_t));
         if (glob (pattern, 0, NULL, g) != 0)
         {
            /* no match (or an error): an empty list, safe to globfree */
            globfree (g);
            memset (g, 0, sizeof (glob_t));
         }
         XtFree (pattern);
         find_rec->grepGlob = (void *) g;
         find_rec->grepGlobNext = 0;
         continue;
      }

      /* grep -i -l -e TEXT -- FILES... */
      argv = (char **) XtMalloc ((6 + GREP_MAX_FILES + 1) * sizeof (char *));
      n = 0;
      argv[n++] = "grep";
      argv[n++] = "-i";
      argv[n++] = "-l";
      argv[n++] = "-e";
      argv[n++] = find_rec->grepContent;
      argv[n++] = "--";
      bytes = 0;
      while (find_rec->grepGlobNext < g->gl_pathc &&
             n < 6 + GREP_MAX_FILES && bytes < GREP_MAX_BYTES)
      {
         argv[n] = g->gl_pathv[find_rec->grepGlobNext++];
         bytes += strlen (argv[n++]) + 1;
      }
      argv[n] = NULL;

      if (SpawnSearchProcess (find_rec, argv) == 0)
      {
         find_rec->searchPhase = PHASE_GREP;
         XtFree ((char *) argv);
         return;
      }
      XtFree ((char *) argv);
      /* grep could not be started; go on with the next files */
   }
}


/************************************************************************
 *
 *  AddMatch()
 *	Add a file to the matches of this input callback.
 *
 ************************************************************************/

static void
AddMatch(
        FileMgrData *file_mgr_data,
        char *path,
        XmString **items,
        int *count,
        int *size )
{
   if (*count >= *size)
   {
      *size = *size ? 2 * *size : 64;
      *items = (XmString *) XtRealloc ((char *) *items,
                                       *size * sizeof (XmString));
   }
   if(file_mgr_data->restricted_directory != NULL)
      (*items)[(*count)++] = XmStringCreateLocalized (path +
                                 strlen(file_mgr_data->restricted_directory));
   else
      (*items)[(*count)++] = XmStringCreateLocalized (path);
}


/************************************************************************
 *
 *  SearchLine()
 *	Process one line of output of the search process.
 *
 ************************************************************************/

static void
SearchLine(
        FindRec *find_rec,
        FileMgrData *file_mgr_data,
        char *line,
        XmString **items,
        int *count,
        int *size )
{
   struct stat stat_data;
   char *path;
   char *file_type;
   TypeInfo *type_info;
   Boolean invisible;

   if (*line == '\0' || (path = (char *) DtEliminateDots (line)) == NULL)
      return;

   switch (find_rec->searchPhase)
   {
      case PHASE_NAMES:
         if (lstat (path, &stat_data) != 0)
            return;

         /* Strip out any invisible files; the type of a file that is not
          * a link follows from what lstat returned */
         file_type = (char *) DtDtsDataToDataType (path, NULL, 0,
                                    S_ISLNK (stat_data.st_mode) ? NULL : &stat_data,
                                    NULL, NULL, NULL);
         type_info = _DtGetTypeInfo (file_type);
         invisible = type_info && type_info->invisible;
         DtDtsFreeDataType (file_type);
         if (invisible)
            return;

         /*  Add to the scrolled list of matches  */
         if (strncmp (desktop_dir, path, strlen (desktop_dir)) != 0)
            AddMatch (file_mgr_data, path, items, count, size);
         break;

      case PHASE_DIRS:
         if (stat (path, &stat_data) != 0 && lstat (path, &stat_data) != 0)
            return;

         /* remember the folder, to search in it later */
         if (find_rec->grepCount >= find_rec->grepSize)
         {
            find_rec->grepSize = find_rec->grepSize ? 2 * find_rec->grepSize : 64;
            find_rec->grepDirs = (char **) XtRealloc ((char *) find_rec->grepDirs,
                                     find_rec->grepSize * sizeof (char *));
         }
         find_rec->grepDirs[find_rec->grepCount++] = XtNewString (path);
         break;

      case PHASE_GREP:
         if (stat (path, &stat_data) != 0 && lstat (path, &stat_data) != 0)
            return;

         /* Make sure its nots a directory */
         if (!S_ISDIR (stat_data.st_mode) &&
             strncmp (desktop_dir, path, strlen (desktop_dir)) != 0)
            AddMatch (file_mgr_data, path, items, count, size);
         break;
   }
}


/************************************************************************
 *
 *  SearchInputHandler()
 *	Called whenever the search process has sent output (or ended).
 *	Reads what is available, processes every complete line and adds
 *	the matches to the list in one call.  The old handlers read one
 *	byte at a time through stdio and returned after one line, so the
 *	rest waited in the stdio buffer until the process wrote again.
 *
 ************************************************************************/

static void
SearchInputHandler(
        XtPointer client_data,
        int *source,
        XtInputId *id )
{
   FindRec * find_rec = (FindRec *) client_data;
   FileMgrData *file_mgr_data;
   DialogData * dialog_data;
   XmString *items = NULL;
   int count = 0, size = 0;
   char *line, *nl, *end;
   Boolean eof = False;
   ssize_t n;
   int i;

   /* Abort if the pipe has already been closed */

   if (find_rec->pipeFd < 0)
   {
      Arg args[1];

      XtSetArg (args[0], XmNdefaultButton, find_rec->start);
      XtSetValues (find_rec->form, args, 1);
      _DtTurnOffHourGlass (find_rec->shell);
      return;
   }

   if (find_rec->lineSize - find_rec->lineLen < SEARCH_READ_CHUNK + 1)
   {
      find_rec->lineSize = find_rec->lineLen + SEARCH_READ_CHUNK + 1;
      find_rec->lineBuf = XtRealloc (find_rec->lineBuf, find_rec->lineSize);
   }

   do
      n = read (find_rec->pipeFd, find_rec->lineBuf + find_rec->lineLen,
                SEARCH_READ_CHUNK);
   while (n < 0 && errno == EINTR);
   if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
      return;
   if (n <= 0)
      eof = True;
   else
      find_rec->lineLen += n;

   dialog_data = _DtGetInstanceData ((XtPointer) find_rec->fileMgrRec);
   file_mgr_data = (FileMgrData *) dialog_data->data;

   line = find_rec->lineBuf;
   end = line + find_rec->lineLen;
   while ((nl = memchr (line, '\n', end - line)) != NULL)
   {
      *nl = '\0';
      SearchLine (find_rec, file_mgr_data, line, &items, &count, &size);
      line = nl + 1;
   }
   if (eof && line < end)
   {
      /* the last line had no NewLine */
      *end = '\0';
      SearchLine (find_rec, file_mgr_data, line, &items, &count, &size);
      line = end;
   }
   find_rec->lineLen = end - line;
   memmove (find_rec->lineBuf, line, find_rec->lineLen);

   if (count > 0)
   {
      XmListAddItemsUnselected (find_rec->matchList, items, count, 0);
      for (i = 0; i < count; i++)
         XmStringFree (items[i]);
   }
   XtFree ((char *) items);

   if (eof)
   {
      /* EOF; this process is complete */
      EndSearchProcess (find_rec, False);
      if (find_rec->searchPhase == PHASE_NAMES)
         FinishSearch (find_rec);
      else
         StartNextGrep (find_rec);
   }
}

/************************************************************************
 *
 *  GetFileName
 *	Get host/dir for selected match item
 *
 ************************************************************************/

static void
GetFileName(
        Widget list,
        int selectedItem,
        String *host,
        String *path,
        FileMgrData *file_mgr_data)
{
   Arg args[2];
   int count;
   XmString * items;
   String temp;
   char * tmpptr;

   XtSetArg (args[0], XmNitemCount, &count);
   XtSetArg (args[1], XmNitems, &items);
   XtGetValues (list, args, 2);

   temp =  (char *) _XmStringUngenerate(items[selectedItem],
                                        XmFONTLIST_DEFAULT_TAG,
                                        XmCHARSET_TEXT, XmCHARSET_TEXT);
   *host = NULL;
   *path = temp;
   if(file_mgr_data->restricted_directory != NULL)
   {
      tmpptr = (char *)XtMalloc(
                      strlen(file_mgr_data->restricted_directory) +
                                                     strlen(*path) + 1);
      strcpy(tmpptr, file_mgr_data->restricted_directory);
      strcat(tmpptr, *path);
      if(*host == NULL)
         XtFree(*path);
      *path = (String)tmpptr;
   }
   else if (*host != NULL)
   {
      *path = XtNewString(*path);
   }
}




/************************************************************************
 *
 *  ExtractDirectory
 *	Check if the path specifies a directory or a file.  If its a
 *      file, then extract out the directory portion, since thats all
 *      we want.
 *
 ************************************************************************/

static Boolean
ExtractDirectory(
        String host,
        String path,
        char **file_name )
{
   String realPath;
   String findptr;
   Tt_status tt_status;
   struct stat statData;

   realPath = (String) ResolveLocalPathName (host, path, NULL, home_host_name, &tt_status);
   if( TT_OK != tt_status )
     return( False );
   else
   {
     if( stat (realPath, &statData ) != 0 )
     {
       if( lstat (realPath, &statData) != 0 )
       {
         XtFree(realPath);
         return(False);
       }
     }
   }
   XtFree(realPath);

   /* Path already is a directory */

   if ((statData.st_mode & S_IFMT) == S_IFDIR)
   {
      *file_name = NULL;
      return (True);
   }

   if((findptr = strrchr (path, '/')))
   {
     *findptr = '\0';
     findptr++;
     *file_name = findptr;
     return (True);
   }
   else
   {
      *file_name = NULL;
      return (True);
   }
}




/************************************************************************
 *
 *  NewView
 *	Do the real work of viewing a directory.
 *
 ************************************************************************/

static void
NewView(
        Widget widget,
        XtPointer client_data,
        XtPointer call_data )
{
   FindRec       *find_rec;
   FileMgrData   *file_mgr_data;
   DialogData    *file_mgr_dialog_data;
   DialogData    *dialog_data;
   DirectorySet  *directory_data;
   FileViewData  *file_view_data;
   char          *file_name;
   String        path, host;
   int           j;

   find_rec = (FindRec *)client_data;
   file_mgr_dialog_data =
      (DialogData *) _DtGetInstanceData ((XtPointer) find_rec->fileMgrRec);
   file_mgr_data = (FileMgrData *) (file_mgr_dialog_data->data);

   GetFileName (find_rec->matchList, find_rec->selectedItem, &host, &path,
                                                               file_mgr_data);

   dialog_data = NULL;
   if (host == NULL)
   {
      /* No host specified, use the one associated with the view */

      if (ExtractDirectory (file_mgr_data->host, path, &file_name))
      {
         initiating_view = (XtPointer) file_mgr_data;
         if(file_mgr_data->restricted_directory == NULL)
            dialog_data = GetNewView (file_mgr_data->host, path, NULL, NULL, 0);
         else
         {
            special_view = True;
            special_treeType = file_mgr_data->show_type;
            special_viewType = file_mgr_data->view;
            special_orderType = file_mgr_data->order;
            special_directionType = file_mgr_data->direction;
            special_randomType = file_mgr_data->positionEnabled;
            special_restricted =
                   XtNewString(file_mgr_data->restricted_directory);
            if(file_mgr_data->title == NULL)
               special_title = NULL;
            else
               special_title = XtNewString(file_mgr_data->title);
            special_helpVol = XtNewString(file_mgr_data->helpVol);
            if(file_mgr_data->toolbox)
               dialog_data = GetNewView (file_mgr_data->host, path,
                              file_mgr_data->restricted_directory, NULL, 0);
            else
               dialog_data = GetNewView (file_mgr_data->host,
					 path, NULL, NULL, 0);
         }
      }
      else
        InvalidFindMessage (find_rec, NO_EXISTANCE, NULL);
   }
   else
   {
      if (ExtractDirectory (host, path, &file_name))
      {
         initiating_view = (XtPointer) file_mgr_data;
         if(file_mgr_data->restricted_directory == NULL)
            dialog_data = GetNewView (host, path, NULL, NULL, 0);
         else
         {
            special_view = True;
            special_treeType = file_mgr_data->show_type;
            special_viewType = file_mgr_data->view;
            special_orderType = file_mgr_data->order;
            special_directionType = file_mgr_data->direction;
            special_randomType = file_mgr_data->positionEnabled;
            special_restricted =
                  XtNewString(file_mgr_data->restricted_directory);
            if(file_mgr_data->title == NULL)
               special_title = NULL;
            else
               special_title = XtNewString(file_mgr_data->title);
            special_helpVol = XtNewString(file_mgr_data->helpVol);
            if(file_mgr_data->toolbox)
               dialog_data = GetNewView (file_mgr_data->host, path,
                               file_mgr_data->restricted_directory, NULL, 0);
            else
               dialog_data = GetNewView (file_mgr_data->host,
					 path, NULL, NULL, 0);
         }
      }
      else
        InvalidFindMessage  (find_rec, NO_EXISTANCE, NULL);
   }

   if(dialog_data != NULL)
   {
      file_mgr_data = (FileMgrData *) dialog_data->data;
      directory_data = file_mgr_data->directory_set[0];

      for (j = 0; j < directory_data->file_count; j++)
      {
         file_view_data = directory_data->file_view_data[j];

         if (file_view_data->filtered != True &&
            strcmp(file_name, file_view_data->file_data->file_name) == 0)
            {
               SelectFile (file_mgr_data, file_view_data);
               PositionFileView(file_view_data, file_mgr_data);
               break;
            }
      }
      if (!directory_data->file_count)
         file_mgr_data->desktop_file = XtNewString(file_name);
      else
         file_mgr_data->desktop_file = NULL;
   }
   if(file_mgr_data->selection_list[0] != NULL)
      ActivateSingleSelect ((FileMgrRec *) file_mgr_data->file_mgr_rec,
                    file_mgr_data->selection_list[0]->file_data->logical_type);

   XtFree ((char *) host);
   XtFree ((char *) path);
}



/************************************************************************
 *
 *  FindPutOnDesktop
 *      Put the found file/directory on the Desktop
 *
 ************************************************************************/

static void
FindPutOnDesktop(
        Widget widget,
        XtPointer client_data,
        XtPointer call_data )
{
   FindRec *find_rec = (FindRec *)client_data;
   FileMgrData   *file_mgr_data;
   DialogData    *file_mgr_dialog_data;
   char          *file_name;
   char          *fileptr;
   String        path, host;
   Boolean       result;
   int EndIndex  = desktop_data->numIconsUsed;

   file_mgr_dialog_data =
      (DialogData *) _DtGetInstanceData ((XtPointer) find_rec->fileMgrRec);
   file_mgr_data = (FileMgrData *) (file_mgr_dialog_data->data);

   GetFileName (find_rec->matchList, find_rec->selectedItem, &host, &path,
                                                               file_mgr_data);

   if (file_mgr_data->toolbox)
      path = _DtResolveAppManPath(path, file_mgr_data->restricted_directory);

   if( host == NULL)
       result =  ExtractDirectory (file_mgr_data->host, path, &file_name);
   else
       result = ExtractDirectory (host, path, &file_name);

   if(!result)
   {
      InvalidFindMessage (find_rec, NO_EXISTANCE, NULL);
      XtFree((char *)path);
      XtFree((char *)host);
      return;
   }

   if(file_name == NULL)
   {
      fileptr = strrchr (path, '/');
      *fileptr = '\0';
      file_name = fileptr + 1;
   }

   if( host == NULL)
   {
      SetupDesktopWindow (XtDisplay(widget), NULL,
                          (FileMgrRec *)file_mgr_data->file_mgr_rec,
                          file_name, file_mgr_data->host, path, -1, -1,
                          file_mgr_data->restricted_directory,EndIndex);
   }
   else
   {
      SetupDesktopWindow (XtDisplay(widget), NULL,
                          (FileMgrRec *)file_mgr_data->file_mgr_rec,
                          file_name, host, path, -1, -1,
                          file_mgr_data->restricted_directory,EndIndex);
   }
   XtFree((char *)path);
   XtFree((char *)host);
}

/************************************************************************
 *
 *  SetActiveItem()
 *	Saves the index of the selected item
 *
 ************************************************************************/

static void
SetActiveItem(
        Widget widget,
        XtPointer client_data,
        XtPointer call_data )
{
   FindRec * find_rec = (FindRec *) client_data;
   XmListCallbackStruct * cb = (XmListCallbackStruct *) call_data;

   find_rec->selectedItem = cb->item_position - 1;

   XtSetSensitive (find_rec->newFM, True);
   XtSetSensitive (find_rec->putOnDT, True);
}


static void
SetFocus(
        FindRec *find_rec,
        FindData *find_data )
{
   /* Force the focus to the text field */
   XmProcessTraversal(find_rec->fileNameFilter, XmTRAVERSE_CURRENT);
}
