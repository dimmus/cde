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
/* $XConsortium: find.c /main/9 1996/11/21 19:42:36 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#if defined(__linux__)
#define _GNU_SOURCE		/* strcasestr */
#endif

#include <EUSCompat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <csa.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/DialogS.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/ToggleBG.h>
#include <Dt/HourGlass.h>
#include "calendar.h"
#include "util.h"
#include "timeops.h"
#include "find.h"
#include "format.h"
#include "datefield.h"
#include "props.h"
#include "editor.h"
#include "select.h"
#include "help.h"
#include "getdate.h"

#define XOS_USE_XT_LOCKING
#define X_INCLUDE_TIME_H
#if defined(__linux__)
#undef SVR4
#endif
#include <X11/Xos_r.h>

static void layout_labels(Widget, Widget, Widget, Widget);
// misc.c
extern void _i18n_WidestWidget(int num, Widget *ret, Dimension *dim, ...);
extern void _i18n_HighestWidget(int num, Widget *ret, Dimension *dim, ...);

extern caddr_t
make_find(Calendar *c)
{
	Find 		*f;
	Widget 		 separator1, separator2, button_form;
	XmString 	 xmstr;
	OrderingType 	 ot = get_int_prop((Props *)c->properties,
							CP_DATEORDERING);
	SeparatorType 	 sep = get_int_prop((Props *)c->properties, 
							CP_DATESEPARATOR);
	Tick		 cursor;
	char		 buffer[50];
	int		 i;
	void 		 find_appts(), show_appt(), f_cancel_cb(), 
			 f_searchrange_cb(), f_searchall_cb();
	Arg		 args[3];
	Dimension	 highest;
	Widget	 	 highest_label;
	char		 *title;

	if (c->find == NULL) {
                c->find = (caddr_t)ckalloc(sizeof(Find));
                f = (Find*)c->find;
        }
        else
                f = (Find*)c->find;

	title = XtNewString(CATGETS(c->DT_catd, 1, 283, "Calendar : Find"));
	f->frame = XtVaCreatePopupShell("frame",
                xmDialogShellWidgetClass, 
		c->frame,
                XmNtitle, 		title,
                XmNallowShellResize, 	True,
		XmNmappedWhenManaged, 	False,
                NULL);
	XtFree(title);

        f->form = XtVaCreateWidget("form",
                xmFormWidgetClass, 
		f->frame,
                XmNautoUnmanage, 	False,
		XmNfractionBase, 	4,
		XmNmarginWidth, 	0,
		XmNmarginHeight, 	0,
                NULL);

	xmstr = XmStringCreateLocalized(CATGETS(c->DT_catd, 1, 284, "Find:"));
        f->apptstr_label = XtVaCreateWidget("label", 
		xmLabelWidgetClass, 
		f->form,
		XmNlabelString, 	xmstr,
                XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset,		10,
                XmNtopAttachment, 	XmATTACH_FORM,
		XmNtopOffset,		15,
                NULL);
	XmStringFree(xmstr);

        f->apptstr = XtVaCreateWidget("appt", 
		xmTextWidgetClass, 
		f->form,
                XmNtopAttachment, 	XmATTACH_FORM,
		XmNtopOffset,		10,
                XmNrightAttachment, 	XmATTACH_FORM,
		XmNrightOffset,		10,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		f->apptstr_label,
                NULL);

	f->search_rc_mgr = XtVaCreateWidget("search_rc_mgr",
                xmRowColumnWidgetClass, 
		f->form,
                XmNpacking, 		XmPACK_COLUMN,
                XmNorientation, 	XmVERTICAL,
                XmNradioBehavior, 	True,
                XmNisHomogeneous, 	True,
                XmNentryClass, 		xmToggleButtonGadgetClass,
		XmNmarginWidth,		0,
                XmNtopAttachment, 	XmATTACH_WIDGET,
                XmNtopWidget, 		f->apptstr,
                XmNleftAttachment, 	XmATTACH_OPPOSITE_WIDGET,
                XmNleftWidget, 		f->apptstr,
                XmNleftOffset, 		0,
                NULL);
 
        xmstr = XmStringCreateLocalized(
				CATGETS(c->DT_catd, 1, 973, "Search all"));
        f->search_all = XtVaCreateWidget("searchAll",
                xmToggleButtonGadgetClass, 
		f->search_rc_mgr,
		XmNlabelString, 	xmstr,
		XmNuserData, 		f,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(f->search_all, XmNvalueChangedCallback, f_searchall_cb, 
									NULL);
 
        xmstr = XmStringCreateLocalized(
				CATGETS(c->DT_catd, 1, 974, "Search from"));
        f->search_range = XtVaCreateWidget("searchrange",
                xmToggleButtonGadgetClass, 
		f->search_rc_mgr,
		XmNlabelString, 	xmstr,
		XmNuserData, 		f,
		XmNset, 		True,
                NULL);
	XmStringFree(xmstr);
	XtAddCallback(f->search_range, XmNvalueChangedCallback, 
							f_searchrange_cb, NULL);

	f->search_set = search_range;
 
	/* default beginning of range query to 6 months ago */

	cursor = now();
	for (i = 0; i < 6; i++)
		cursor = prevmonth_exactday(cursor);

	format_tick(cursor, ot, sep, buffer);

        f->search_from = XtVaCreateWidget("search_from", 
		xmTextWidgetClass, 
		f->form,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		f->search_rc_mgr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		f->apptstr,
		XmNtopOffset, 		30, 
		XmNuserData, 		c,
		XmNvalue, 		buffer,
                NULL);
	XtSetSensitive(f->search_from, True);


	/* default end of range query to 6 from now */

	cursor = now();
	for (i = 0; i < 6; i++)
		cursor = nextmonth_exactday(cursor);

	format_tick(cursor, ot, sep, buffer);

	xmstr = XmStringCreateLocalized(CATGETS(c->DT_catd, 1, 975, "to"));
        f->search_tolabel = XtVaCreateWidget("tolabel", 
		xmLabelWidgetClass, 
		f->form,
		XmNlabelString, 	xmstr,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		f->search_from,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		f->apptstr,
		XmNtopOffset, 		35, 
                NULL);
	XmStringFree(xmstr);

        f->search_to = XtVaCreateWidget("search_to", 
		xmTextWidgetClass, 
		f->form,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		f->search_tolabel,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		f->apptstr,
		XmNtopOffset, 		30, 
		XmNrightOffset,		10, 
		XmNuserData, 		c,
		XmNvalue, 		buffer,
                NULL);
	XtSetSensitive(f->search_to, True);

        separator1 = XtVaCreateWidget("separator1",
                xmSeparatorGadgetClass, 
		f->form,
                XmNleftAttachment, 	XmATTACH_FORM,
                XmNrightAttachment, 	XmATTACH_FORM,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		f->search_rc_mgr,
		XmNtopOffset, 		5,
                NULL);
 
	xmstr = XmStringCreateLocalized(
		CATGETS(c->DT_catd, 1, 848, "Date"));
        f->date_label = XtVaCreateWidget("finddatelabel", 
		xmLabelWidgetClass, 
		f->form,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		separator1,
		XmNtopOffset, 		10,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		10,
                NULL);
	XmStringFree(xmstr);
	xmstr = XmStringCreateLocalized(
		CATGETS(c->DT_catd, 1, 849, "Time"));
        f->time_label = XtVaCreateWidget("findtimelabel", 
		xmLabelWidgetClass, 
		f->form,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		separator1,
		XmNtopOffset, 		10,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		f->date_label,
                NULL);
	XmStringFree(xmstr);
	xmstr = XmStringCreateLocalized(
		CATGETS(c->DT_catd, 1, 850, "What"));
        f->what_label = XtVaCreateWidget("findwhatlabel", 
		xmLabelWidgetClass, 
		f->form,
		XmNlabelString, 	xmstr,
		XmNtopAttachment, 	XmATTACH_WIDGET,
		XmNtopWidget, 		separator1,
		XmNtopOffset, 		10,
		XmNleftAttachment, 	XmATTACH_WIDGET,
		XmNleftWidget, 		f->time_label,
                NULL);
	XmStringFree(xmstr);

	f->find_message = XtVaCreateWidget("message",
		xmLabelGadgetClass, 	f->form,
		XmNalignment, 		XmALIGNMENT_BEGINNING,
		XmNleftAttachment, 	XmATTACH_FORM,
		XmNleftOffset, 		10,
		XmNrightAttachment, 	XmATTACH_FORM,
		XmNbottomAttachment, 	XmATTACH_FORM,
		NULL);

        button_form = XtVaCreateWidget("print_button_form_mgr",
                xmFormWidgetClass,
                f->form,
                XmNautoUnmanage,        False,
                XmNfractionBase,        5,
		XmNhorizontalSpacing,	0,
                XmNleftAttachment,      XmATTACH_FORM,
                XmNleftOffset,          5,
                XmNrightAttachment,     XmATTACH_FORM,
                XmNrightOffset,         5,
                XmNbottomAttachment,    XmATTACH_WIDGET,
                XmNbottomWidget,        f->find_message,
                XmNbottomOffset,        5,
                NULL);

        separator2 = XtVaCreateWidget("separator1",
                xmSeparatorGadgetClass, 
		f->form,
                XmNleftAttachment, 	XmATTACH_FORM,
                XmNrightAttachment, 	XmATTACH_FORM,
                XmNbottomAttachment, 	XmATTACH_WIDGET,
		XmNbottomWidget, 	button_form,
		XmNbottomOffset, 	5,
                NULL);

	xmstr = XmStringCreateLocalized(CATGETS(c->DT_catd, 1, 285, "Find"));
        f->find_button = XtVaCreateWidget("findbutton", 
		xmPushButtonGadgetClass, 
		button_form,
		XmNlabelString, 	xmstr,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	0,
		XmNleftOffset, 		0,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	1,
		XmNrightOffset, 	0,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNshowAsDefault,	0,
                NULL);
	XtAddCallback(f->find_button, XmNactivateCallback, find_appts, NULL);
	XmStringFree(xmstr);
		
	xmstr = XmStringCreateLocalized(
				CATGETS(c->DT_catd, 1, 851, "Show Appointment"));
	f->show_button = XtVaCreateWidget("show", 
		xmPushButtonGadgetClass, 
		button_form,
		XmNlabelString, 	xmstr,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	1,
		XmNleftOffset, 		0,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	3,
		XmNrightOffset, 	0,
		XmNbottomAttachment,	XmATTACH_FORM,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(f->show_button, XmNactivateCallback, show_appt, NULL);

	xmstr = XmStringCreateLocalized(CATGETS(c->DT_catd, 1, 680, "Close"));
	f->cancel_button = XtVaCreateWidget("cancel", 
		xmPushButtonGadgetClass, 
		button_form,
		XmNlabelString, 	xmstr,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	3,
		XmNleftOffset, 		0,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	4,
		XmNrightOffset, 	0,
		XmNbottomAttachment,	XmATTACH_FORM,
                NULL);
	XmStringFree(xmstr);

	XtAddCallback(f->cancel_button, XmNactivateCallback, f_cancel_cb, NULL);

	xmstr = XmStringCreateLocalized(CATGETS(c->DT_catd, 1, 77, "Help"));
	f->help_button = XtVaCreateWidget("help", 
		xmPushButtonGadgetClass, 
		button_form,
		XmNlabelString, 	xmstr,
		XmNleftAttachment, 	XmATTACH_POSITION,
		XmNleftPosition, 	4,
		XmNleftOffset, 		0,
		XmNrightAttachment, 	XmATTACH_POSITION,
		XmNrightPosition, 	5,
		XmNrightOffset, 	0,
		XmNbottomAttachment,	XmATTACH_FORM,
                NULL);
        XtAddCallback(f->help_button, XmNactivateCallback, 
		(XtCallbackProc)help_cb, FIND_HELP_BUTTON);
        XtAddCallback(f->form, XmNhelpCallback, 
		(XtCallbackProc)help_cb, (XtPointer) FIND_HELP_BUTTON);
	XmStringFree(xmstr);

	XtSetArg(args[0], XmNlistSizePolicy, XmCONSTANT);
	XtSetArg(args[1], XmNvisibleItemCount, 5);
	XtSetArg(args[2], XmNscrollBarDisplayPolicy, XmSTATIC);

	f->find_list = (Widget)XmCreateScrolledList(f->form,
                "find_list", args, 3);
        f->find_list_sw = XtParent(f->find_list);

	_i18n_HighestWidget(3,&highest_label, &highest,
		f->date_label, f->time_label, f->what_label);

	XtVaSetValues(f->find_list_sw,
                XmNtopAttachment, 	XmATTACH_WIDGET,
                XmNtopWidget, 		highest_label,
                XmNleftAttachment, 	XmATTACH_FORM,
                XmNleftOffset, 		10,
                XmNrightAttachment, 	XmATTACH_FORM,
                XmNrightOffset, 	10,
                XmNbottomAttachment, 	XmATTACH_WIDGET,
		XmNbottomWidget, 	separator2,
		XmNbottomOffset, 	10,
                XmNlistSizePolicy, 	XmCONSTANT,
		NULL);

	XtManageChild(f->find_list);

	XtAddCallback(f->find_list, XmNdefaultActionCallback, show_appt, NULL);

	layout_labels(f->date_label, f->time_label, 
		      f->what_label, f->find_list);

        /* set default button */
        XtVaSetValues(button_form,
		XmNdefaultButton, 	f->find_button,
        	XmNcancelButton, 	f->cancel_button,
		NULL);
        XtVaSetValues(f->form,
		XmNdefaultButton, 	f->find_button,
        	XmNcancelButton, 	f->cancel_button,
		NULL);

/*
	XtManageChild(f->apptstr_label);
	XtManageChild(f->apptstr);
	XtManageChild(f->search_rc_mgr);
	XtManageChild(f->search_all);
	XtManageChild(f->search_range);
	XtManageChild(f->search_from);
	XtManageChild(f->search_to);
	XtManageChild(f->search_tolabel);
	XtManageChild(separator1);
	XtManageChild(f->date_label);
	XtManageChild(f->time_label);
	XtManageChild(f->what_label);
	XtManageChild(f->find_message);
	XtManageChild(button_form);
	XtManageChild(separator2);
	XtManageChild(f->find_list);
	XtManageChild(f->find_button);
	XtManageChild(f->show_button);
	XtManageChild(f->cancel_button);
	XtManageChild(f->help_button);
*/
        ManageChildren(f->search_rc_mgr);
	ManageChildren(button_form);
	ManageChildren(f->form);
	XtManageChild(f->form);
        XtVaSetValues(f->frame, XmNmappedWhenManaged, True, NULL);
	XtRealizeWidget(f->frame);

	XtPopup(f->frame, XtGrabNone);

	return(caddr_t)f;
}

static Tick
f_get_searchdate(Widget widget, Props *p)
{
	Calendar *c = calendar;
        OrderingType ot;
        SeparatorType st;
	char *buf;
	Tick new_date;

	ot = get_int_prop(p, CP_DATEORDERING);
        st = get_int_prop(p, CP_DATESEPARATOR);
	buf = get_date_from_widget(c->view->date, widget, ot, st);

	new_date = (Tick) cm_getdate(buf, NULL);

	return(new_date);
}

void
f_searchall_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
	Find *f;
	XmToggleButtonCallbackStruct *state =
		(XmToggleButtonCallbackStruct *) call_data;

	XtVaGetValues(widget, XmNuserData, &f, NULL);

	if (state->set)
		f->search_set = search_all;
	XtSetSensitive(f->search_to, False);
	XtSetSensitive(f->search_from, False);
}

void
f_searchrange_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
	Find *f;
	XmToggleButtonCallbackStruct *state =
		(XmToggleButtonCallbackStruct *) call_data;

	XtVaGetValues(widget, XmNuserData, &f, NULL);

	if (state->set)
		f->search_set = search_range;
	XtSetSensitive(f->search_to, True);
	XtSetSensitive(f->search_from, True);
}

static void
fmt_time_what(
	Dtcm_appointment 	*appt,
	char 			*buf,
	DisplayType		 display)
{
        int 			 hr, mn;
	time_t			 tick;
        Lines 			*lines;
        Boolean 		 am;
        struct tm 		*tm;
	char			 tmp[16];
	_Xltimeparams		 localtime_buf;
	(void) localtime_buf;	/* unused unless XTHREADS */
 
        if(!appt || !buf) return;
        _csa_iso8601_to_tick(appt->time->value->item.string_value, &tick);
        tm = _XLocaltime(&tick, localtime_buf);
        hr = tm->tm_hour;
        mn = tm->tm_min;
        if (showtime_set(appt) && !magic_time(tick)) {
                if (display == HOUR12) {
                        am = adjust_hour(&hr);
                        sprintf(tmp, "%2d:%02d%s  ", hr, mn, am ? "a" : "p");
                }
                else
                        sprintf(tmp, "%02d%02d    ", hr, mn);

		sprintf(buf, "%8s", tmp);
        } else
		sprintf(buf, "%8s", "");

        lines = text_to_lines(appt->what->value->item.string_value, 1);
        if (lines != NULL && lines->s != NULL) {
                (void) cm_strcat(buf, lines->s);
        }

        if (lines != NULL) {
                destroy_lines(lines);
        }
}

/*
 * Find searches in chunks.  It used to fetch every appointment 4 weeks
 * at a time (about 880 calls for "all") and match here.  The server now
 * does the matching (CSA_MATCH_CONTAIN on the summary), so only hits
 * come back, and a year per call keeps each reply small even for daily
 * repeating appointments and old servers that sort replies in O(n^2).
 * Without the server-side match the old 4-week chunks are kept.
 */
#define FIND_CHUNK		(52 * wksec)
#define FIND_CHUNK_UNFILTERED	(4 * wksec)

/*
 * The server matches case-insensitively in its own locale, normally C;
 * let it match only ASCII strings, so 8-bit locales keep their own
 * case folding (done here with strcasestr).
 */
static boolean_t
is_ascii(const char *s)
{
	for (; *s; s++)
		if ((unsigned char)*s >= 0x80)
			return (B_FALSE);
	return (B_TRUE);
}

/*
 * Add "summary contains str" to the range criteria set up by setup_range.
 */
static boolean_t
add_summary_match(CSA_attribute **attrs, CSA_enum **ops, int *count,
		  char *str, int version)
{
	CSA_attribute	*na;
	CSA_enum	*no;

	if ((na = (CSA_attribute *)realloc(*attrs,
	    (*count + 1) * sizeof(CSA_attribute))) == NULL)
		return (B_FALSE);
	*attrs = na;
	if ((no = (CSA_enum *)realloc(*ops,
	    (*count + 1) * sizeof(CSA_enum))) == NULL)
		return (B_FALSE);
	*ops = no;

	memset(&na[*count], 0, sizeof(CSA_attribute));
	initialize_entry_attr(CSA_ENTRY_ATTR_SUMMARY_I, &na[*count],
			      appt_write, version);
	if (na[*count].value == NULL ||
	    (na[*count].value->item.string_value = strdup(str)) == NULL) {
		free(na[*count].name);
		free(na[*count].value);
		return (B_FALSE);
	}
	no[*count] = CSA_MATCH_CONTAIN;
	(*count)++;

	return (B_TRUE);
}

void
find_appts(Widget widget, XtPointer client_data, XmPushButtonCallbackStruct *cbs)
{
	Calendar *c = calendar;
	Props *p = (Props*)c->properties;
	DisplayType dt = get_int_prop(p, CP_DEFAULTDISP);
	Find *f = (Find*)c->find;
	int i, range_count;
        char what_buf[WHAT_LEN+1], buf[WHAT_LEN+1], buf2[WHAT_LEN+1], message[40], *astr;
	XmString *items = NULL, *nitems;
	int nitems_used = 0, nitems_max = 0;
        int last_match_total = 0, match_total = 0;
	Tick end_of_time, start, stop;
	Tick_list *ptr, *next_ptr, *tail_ptr = NULL, *new_tick;
	CSA_return_code stat;
        CSA_entry_handle *entries = NULL;
	CSA_enum *ops;
        CSA_attribute *range_attrs;
	CSA_uint32 num_entries;
	Dtcm_appointment *appt;
	char *what;
	boolean_t use_filter, filtered;
	Tick	real_eot = get_eot();
	_Xltimeparams localtime_buf;
	(void) localtime_buf;	/* unused unless XTHREADS */

	astr = XmTextGetString(f->apptstr);

        if (astr == NULL || *astr == '\0') {
		sprintf(message, "%s", CATGETS(c->DT_catd, 1, 290, "Specify Appt String to Match."));
		set_message(f->find_message, message);
		XtFree(astr);
                return;
        }
	XmListDeleteAllItems(f->find_list);
	set_message(f->find_message, " ");
	ptr = f->ticks;
	while (ptr != NULL) {
		next_ptr = ptr->next;
		free(ptr);
		ptr = next_ptr;
	}
	f->ticks = NULL;

	if (f->search_set == search_all) {
		end_of_time = real_eot;
		start = get_bot();
	}
	else {

		start = f_get_searchdate(f->search_from, p);

		if (start == DATE_BBOT)
			start = get_bot();
		else if (start == DATE_AEOT) {
                        sprintf(message, "%s", CATGETS(c->DT_catd, 1, 810, "Please enter a start date after 1/1/1970"));
                        set_message(f->find_message, message);
			XtFree(astr);
                        return;
                }
		else if (start <= 0) {
                        sprintf(message, "%s", CATGETS(c->DT_catd, 1, 811, "Malformed start date"));
                        set_message(f->find_message, message);
			XtFree(astr);
                        return;
                }


		end_of_time = f_get_searchdate(f->search_to, p);

		if (end_of_time == DATE_AEOT)
			end_of_time = real_eot;
		else if (end_of_time == DATE_BBOT) {
                        sprintf(message, "%s", CATGETS(c->DT_catd, 1, 812, "Please enter an end date before 1/1/2038"));
                        set_message(f->find_message, message);
			XtFree(astr);
                        return;
                }
		else if (end_of_time <= 0) {
                        sprintf(message, "%s", CATGETS(c->DT_catd, 1, 813, "Malformed end date"));
                        set_message(f->find_message, message);
			XtFree(astr);
                        return;
                }

		if (start >= end_of_time) {
			sprintf(message, "%s", CATGETS(c->DT_catd, 1, 713, "You must choose a begin date before the end date."));
                	set_message(f->find_message, message);
			XtFree(astr);
                	return;
        	}
		
		if ((end_of_time < 0) || (end_of_time > real_eot))
			end_of_time = real_eot;
	}

	use_filter = is_ascii(astr);
	stop = start + (use_filter ? FIND_CHUNK : FIND_CHUNK_UNFILTERED);

	if ((stop > end_of_time) || (stop < 0))
		stop = end_of_time;

	appt = allocate_appt_struct(appt_read,
				    c->general->version,
				    CSA_ENTRY_ATTR_START_DATE_I,
				    CSA_ENTRY_ATTR_SUMMARY_I,
                                    CSA_X_DT_ENTRY_ATTR_SHOWTIME_I,
				    NULL);

	_DtTurnOnHourGlass(f->frame);
        for (; stop <= end_of_time;) {
		setup_range(&range_attrs, &ops, &range_count, start, stop,
			    CSA_TYPE_EVENT, 0, B_FALSE, c->general->version);
		filtered = use_filter &&
			add_summary_match(&range_attrs, &ops, &range_count,
					  astr, c->general->version);
	        stat = csa_list_entries(c->cal_handle, range_count, range_attrs, ops, &num_entries, &entries, NULL);

		if (stat != CSA_SUCCESS && filtered) {
			/* the server cannot match on the summary;
			 * fetch everything and match here
			 */
			use_filter = B_FALSE;
			free_range(&range_attrs, &ops, range_count);
			stop = start + FIND_CHUNK_UNFILTERED;
			if ((stop > end_of_time) || (stop < 0))
				stop = end_of_time;
			continue;
		}

        	if (stat != CSA_SUCCESS) {
			free_range(&range_attrs, &ops, range_count);
			break;
        	}

		for (i = 0; i < num_entries; i++) {
			stat = query_appt_struct(c->cal_handle, entries[i], appt);
                	if (stat != CSA_SUCCESS)
				break;

			/* the server matched already, but check anyway:
			 * this is cheap and the only check when the
			 * server could not filter
			 */
			what = appt->what->value->item.string_value;
			if (what == NULL || strcasestr(what, astr) == NULL)
				continue;

			if (nitems_used == nitems_max) {
				nitems_max = nitems_max ? nitems_max * 2 : 64;
				if ((nitems = (XmString *)realloc(items,
				    nitems_max * sizeof(XmString))) == NULL) {
					stat = CSA_E_INSUFFICIENT_MEMORY;
					break;
				}
				items = nitems;
			}
			new_tick = (Tick_list *) ckalloc(sizeof(Tick_list));
			if (new_tick == NULL) {
				stat = CSA_E_INSUFFICIENT_MEMORY;
				break;
			}
			new_tick->next = NULL;
			_csa_iso8601_to_tick(appt->time->value->item.date_time_value, &(new_tick->tick));
			if (f->ticks == NULL)
				f->ticks = new_tick;
			else
				tail_ptr->next = new_tick;
			tail_ptr = new_tick;
			match_total++;
			strcpy(buf, "");
			strcpy(buf2, "");
			strcpy(what_buf, "");
			strftime(buf, WHAT_LEN, "%h %e, %Y", 
			    _XLocaltime(
			      (const time_t *)&new_tick->tick,
			      localtime_buf));
			fmt_time_what(appt, what_buf, dt);
			snprintf(buf2, sizeof(buf2), "%10s  %s", buf, what_buf);
			items[nitems_used++] = XmStringCreateLocalized(buf2);
		}  /* end for i = 0 loop */

		csa_free(entries);
		free_range(&range_attrs, &ops, range_count);

		if (stat != CSA_SUCCESS)
			break;

		if (match_total != last_match_total) {
			if (match_total == 1)
				sprintf(message, CATGETS(c->DT_catd, 1, 631, "%d match found"), match_total);
			else if (match_total > 1)
				sprintf(message, CATGETS(c->DT_catd, 1, 292, "%d matches found"), match_total);

			set_message(f->find_message, message);
		}

		last_match_total = match_total;

		if (stop == real_eot)
			break;

		start = stop + 1;
	
		if (start > end_of_time)
			break;

		stop = start + (use_filter ? FIND_CHUNK : FIND_CHUNK_UNFILTERED);
		if ((stop > end_of_time) || (stop < 0))
			stop = end_of_time;
        }  /* end for range.end loop */

	/* one list update instead of one per match */
	if (nitems_used > 0) {
		XmListAddItems(f->find_list, items, nitems_used, 0);
		for (i = 0; i < nitems_used; i++)
			XmStringFree(items[i]);
	}
	free(items);

	if (stat == CSA_SUCCESS) {
		if (match_total == 0)
			sprintf(message, "%s", CATGETS(c->DT_catd, 1, 291, "Appointment Not Found."));
		else if (match_total == 1)
			sprintf(message, CATGETS(c->DT_catd, 1, 631, "%d match found"), match_total);
		else
			sprintf(message, CATGETS(c->DT_catd, 1, 292, "%d matches found"), match_total);
		set_message(f->find_message, message);
	}
	free_appt_struct(&appt);
	_DtTurnOffHourGlass(f->frame);

        XtFree(astr);
        return;
}

void
show_appt(Widget widget, XtPointer client_data, XmPushButtonCallbackStruct *cbs)
{
	Calendar *c = calendar;

	Find *f = (Find*)c->find;
	int *pos_list;
	int pos_cnt;
	int i;
	Dimension w, h;
	Tick_list *ptr;
	char buf[BUFSIZ];
	int list_cnt;

	XtVaGetValues(f->find_list, XmNitemCount, &list_cnt, NULL);
	if (list_cnt == 0) {
		sprintf(buf, "%s", CATGETS(c->DT_catd, 1, 714, "There are no appointments in the list.  You must find one before showing an appointment."));
		set_message(f->find_message, buf);
		return;
	}
	if (!XmListGetSelectedPos(f->find_list, &pos_list, &pos_cnt)) {
		sprintf(buf, "%s", CATGETS(c->DT_catd, 1, 632, "Please select an appointment from the list to show"));
		set_message(f->find_message, buf);
		return;
	}
	set_message(f->find_message, " ");
	XtVaGetValues(c->canvas, XmNheight, &h, XmNwidth, &w, NULL);
	gr_clear_area(c->xcontext, 0, 0, w, h);

	c->view->olddate = c->view->date;
	for (ptr = (Tick_list*)f->ticks, i = 1; ptr != NULL && i < *pos_list; i++)
		ptr = ptr->next;

	c->view->date = ptr->tick;
	c->view->nwks = numwks(c->view->date);
	XtFree((XtPointer)pos_list);
	invalidate_cache(c);
	paint_canvas(c, NULL, RENDER_UNMAP);
	calendar_deselect(c);
	paint_selection(c);

}

void
f_cancel_cb(
        Widget  widget,
        XtPointer client_data,
        XmPushButtonCallbackStruct *cbs)
{
        Calendar *c = calendar;
        Find      *f;

        f = (Find *)c->find;

        XmTextSetString(f->apptstr, "");
        set_message(f->find_message, " ");
	XmListDeleteAllItems(f->find_list);

        XtPopdown(f->frame);

        return;
}

static void
layout_labels(
	Widget	l1, 
	Widget  l2,
	Widget  l3,
	Widget  list)
{
	int		list_char_width;
	Dimension	label_width;
	XmString	xmstr;
	XmFontList	font_list;

	xmstr = XmStringCreateLocalized(" ");

	XtVaGetValues(list, XmNfontList, &font_list, NULL);

	list_char_width = XmStringWidth(font_list, xmstr);

	XtVaGetValues(l1, XmNwidth, &label_width, NULL);

	/*
	 * The ``Time'' label.
	 */
	XtVaSetValues(l2,
		XmNleftOffset,	(14 * list_char_width) - label_width,
		NULL);

	XtVaGetValues(l2, XmNwidth, &label_width, NULL);

	/*
	 * The ``What'' label.
	 */
	XtVaSetValues(l3,
		XmNleftOffset,	(8 * list_char_width) - label_width,
		NULL);

	XmStringFree(xmstr);
}

