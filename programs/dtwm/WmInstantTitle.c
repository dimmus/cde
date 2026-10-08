/*
 * CDE - Common Desktop Environment
 *
 * (c) Copyright 1993-2012 The Open Group
 * (c) Copyright 2012-2022 CDE Project contributors, see
 * CONTRIBUTORS for details
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

#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Dt/DialogBox.h>
#include <Dt/SharedProcs.h>

#include "WmGlobal.h"
#include "WmCDecor.h"
#include "WmError.h"
#include "WmHelp.h"
#include "WmIDecor.h"
#include "WmInstantTitle.h"
#include "WmPanelP.h"
#include "WmProperty.h"
#include "WmWinList.h"
#include "WmWrkspace.h"

extern XtPointer _XmStringUngenerate(XmString, XmStringTag,
				     XmTextType, XmTextType);

static void InstantTitleGetTitle(PtrInstantTitleData pITD) {
    String value;
    Widget textFieldW = pITD->textFieldW;
    ClientData *pCD = pITD->pCDforClient;
    XmString title = pITD->contextForClient == F_CONTEXT_ICON ?
		     ICON_DISPLAY_TITLE(pCD) : CLIENT_DISPLAY_TITLE(pCD);

    if (!title) return;

    value = _XmStringUngenerate(title, NULL, XmCHARSET_TEXT, XmCHARSET_TEXT);

    if (!value) return;

    XtVaSetValues(textFieldW, XmNvalue, value, NULL);

    XtFree(value);

    XtVaSetValues(textFieldW, XmNcursorPosition,
		  XmTextFieldGetLastPosition(textFieldW), NULL);
}

static void InstantTitleSetTitle(PtrInstantTitleData pITD) {
    char *str = XmTextFieldGetString(pITD->textFieldW);
    ClientData *pCD = pITD->pCDforClient;
    Context context = pITD->contextForClient;
    XmString *title = &pCD->instantTitle;
    Window client = pCD->client;
    Atom name = wmGD.xa__NET_WM_VISIBLE_NAME;
    Atom iconName = wmGD.xa__NET_WM_VISIBLE_ICON_NAME;

    if (*title) XmStringFree(*title);

    _DtStripSpaces(str);

    if (str[0]) {
	*title = XmStringCreateLocalized(str);
	SetUtf8String (DISPLAY, client, name, str);
	SetUtf8String (DISPLAY, client, iconName, str);
    }
    else {
	*title = NULL;
	XDeleteProperty(DISPLAY, client, name);
	XDeleteProperty(DISPLAY, client, iconName);
    }

    XtFree(str);

    if (context == F_CONTEXT_ICON || pCD->pSD->useIconBox)
	RedisplayIconTitle(pCD);

    if (context != F_CONTEXT_ICON) DrawWindowTitle(pCD, True);
}

/**
 * @brief Hides the instant title dialog.
 *
 * @param pClientData
 */
void InstantTitleHideDialog(ClientData *pClientData) {
    PtrInstantTitleData pITD = &pClientData->pSD->instantTitleData;

    if (pITD->widgetsInitialized && pITD->pCDforClient == pClientData)
	XtUnmanageChild(pITD->dialogBoxW);
}

static void InstantTitleButtonCB(Widget w, XtPointer client_data,
				 XtPointer call_data)
{
    PtrInstantTitleData pITD = client_data;
    DtDialogBoxCallbackStruct *cb = call_data;

    switch (cb->button_position) {
	case 1:
	    InstantTitleSetTitle(pITD);

	case 2:
	    InstantTitleHideDialog(pITD->pCDforClient);
	    break;

	case 3:
	    XtCallCallbacks(pITD->dialogBoxW, XmNhelpCallback, NULL);
	    break;

	default:
	    break;
    }
}

static void InstantTitleWorkspaceModifyCB(Widget w, Atom atom_name, int type,
					  XtPointer client_data)
{
    PtrInstantTitleData pITD = client_data;
    ClientData *pCD = pITD->pCDforClient;

    if (type == DtWSM_REASON_CURRENT &&
	pITD->contextForClient == F_CONTEXT_ICON &&
	XtIsManaged(pITD->dialogBoxW) && ClientInWorkspace(ACTIVE_WS, pCD))
	InstantTitleShowDialog(pCD);
}

static Boolean InstantTitleMakeDialog(PtrInstantTitleData pITD) {
    Cardinal n;
    Arg args[10];
    XmString buttonStrings[3];
    Widget dialogShellW, dialogBoxW, formW, cancelW, textFieldW;
    Boolean *widgetsInitialized = &pITD->widgetsInitialized;

    if (*widgetsInitialized) return True;

    n = 0;

    XtSetArg(args[n], XmNallowShellResize, True); ++n;
    XtSetArg(args[n], XmNmwmFunctions,
	     MWM_FUNC_ALL | MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE); ++n;

    dialogShellW = pITD->dialogShellW = XtCreatePopupShell(
		   "instantTitleDialogShell", xmDialogShellWidgetClass,
		   pITD->pCDforClient->pSD->screenTopLevelW1, args, n);

    if (!dialogShellW) goto err;

    if (!DtWsmAddWorkspaceModifiedCallback(dialogShellW,
	    InstantTitleWorkspaceModifyCB, pITD)) goto err;

    buttonStrings[0] = XmStringCreateLocalized((char *)_DtOkString);
    buttonStrings[1] = XmStringCreateLocalized((char *)_DtCancelString);
    buttonStrings[2] = XmStringCreateLocalized((char *)_DtHelpString);

    n = 0;

    XtSetArg(args[n], XmNallowOverlap, False); ++n;
    XtSetArg(args[n], XmNdefaultPosition, False); ++n;
    XtSetArg(args[n], XmNbuttonCount, 3); ++n;
    XtSetArg(args[n], XmNbuttonLabelStrings, buttonStrings); ++n;

    dialogBoxW = pITD->dialogBoxW = _DtCreateDialogBox(dialogShellW,
				    "instantTitleDialogBox", args, n);

    XmStringFree(buttonStrings[0]);
    XmStringFree(buttonStrings[1]);
    XmStringFree(buttonStrings[2]);

    if (!dialogBoxW) goto err;

    XtAddCallback(dialogBoxW, XmNcallback, InstantTitleButtonCB, pITD);
    XtAddCallback(dialogBoxW, XmNhelpCallback, WmDtWmTopicHelpCB,
		  WM_DT_WINDOWRENAME_TOPIC);

    cancelW = _DtDialogBoxGetButton(dialogBoxW, 2);

    if (!cancelW) goto err;

    n = 0;

    XtSetArg(args[n], XmNautoUnmanage, False); ++n;
    XtSetArg(args[n], XmNcancelButton, cancelW); ++n;

    XtSetValues (dialogBoxW, args, n);

    formW = pITD->formW = XmCreateForm(dialogBoxW, "instantTitleForm", NULL, 0);

    if (!formW) goto err;

    n = 0;

    XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); ++n;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_NONE); ++n;
    XtSetArg(args[n], XmNmaxLength, FILENAME_MAX); ++n;

    textFieldW = pITD->textFieldW = XmCreateTextField(formW,
				    "instantTitleTextField", args, n);

    if (!textFieldW) goto err;

    XtManageChild(dialogShellW);
    XtManageChild(dialogBoxW);
    XtManageChild(formW);
    XtManageChild(cancelW);
    XtManageChild(textFieldW);

    DtWsmRemoveWorkspaceFunctions(DISPLAY1, XtWindow(dialogShellW));

    *widgetsInitialized = True;

    return True;

err:
    if (dialogShellW) {
	XtDestroyWidget(dialogShellW);
	*widgetsInitialized = False;
    }

    Warning(GETMESSAGE(88, 1, "1 Unable to create instant title dialog."));

    return False;
}

static void InstantTitleSetPosition(PtrInstantTitleData pITD) {
    int x, y;
    Dimension height;
    Cardinal n;
    Arg args[10];
    XFontStruct *font;
    Window windowGroup;
    Widget dialogShellW = pITD->dialogShellW;
    Context context = pITD->contextForClient;
    ClientData *pCD = pITD->pCDforClient;
    WmScreenData *pSD = pCD->pSD;

    XtVaGetValues(dialogShellW, XmNheight, &height, NULL);

    if (wmGD.positionIsFrame) {
	if (pSD->decoupleTitleAppearance)
	    font = pSD->clientTitleAppearance.font;
	else
	    font = pSD->clientAppearance.font;

	height += TEXT_HEIGHT(font) + (2 * pSD->frameBorderWidth);
    }

    GetSystemMenuPosition(pCD, &x, &y, height, context);

    if (context == F_CONTEXT_ICON) {
	if (pSD->useIconBox && P_ICON_BOX(pCD))
	    windowGroup = P_ICON_BOX(pCD)->pCD_iconBox->client;
	else
	    windowGroup = XtUnspecifiedWindowGroup;
    }
    else {
	windowGroup = pCD->client;
    }

    n = 0;

    XtSetArg(args[n], XmNx, x); ++n;
    XtSetArg(args[n], XmNy, y); ++n;
    XtSetArg(args[n], XmNtransientFor, NULL); ++n;
    XtSetArg(args[n], XmNwindowGroup, windowGroup); ++n;

    XtSetValues(dialogShellW, args, n);
}

/**
 * @brief Shows the instant title dialog.
 *
 * @param pClientData
 * @param context
 */
void InstantTitleShowDialog(ClientData *pClientData) {
    Widget dialogBoxW, textFieldW;
    Context context;
    ClientData *pCD = pClientData->transientLeader ?
		      FindTransientTreeLeader(pClientData) : pClientData;
    WmScreenData *pSD = pCD->pSD;
    PtrInstantTitleData pITD = &pSD->instantTitleData;

    switch (pCD->clientState) {
	case MINIMIZED_STATE:
	    context = F_CONTEXT_ICON;
	    break;
	case NORMAL_STATE:
	case MAXIMIZED_STATE:
	    context = F_CONTEXT_WINDOW;
	    break;
	default:
	    return;
    }

    pITD->pCDforClient = pCD;
    pITD->contextForClient = context;

    if (!InstantTitleMakeDialog(pITD)) return;

    InstantTitleGetTitle(pITD);
    InstantTitleSetPosition(pITD);

    dialogBoxW = pITD->dialogBoxW;
    XtUnmanageChild(dialogBoxW);
    XtManageChild(dialogBoxW);

    textFieldW = pITD->textFieldW;
    XmProcessTraversal(textFieldW, XmTRAVERSE_CURRENT);
    XmTextSetSelection(textFieldW, 0, XmTextGetLastPosition(textFieldW),
		       CurrentTime);
}
