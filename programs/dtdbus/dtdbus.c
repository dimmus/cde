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
 * dtdbus - the CDE session's freedesktop.org D-Bus services
 *
 * Today's applications ask the desktop for a few things over D-Bus,
 * which CDE (ToolTalk, X11) did not answer:
 *
 *   org.freedesktop.ScreenSaver (session bus)
 *       A video player or a browser playing a video inhibits the
 *       screen saver: while any inhibition is held the X server's
 *       screen saver is suspended (XScreenSaverSuspend, which also
 *       holds off DPMS), so the screen neither blanks nor locks
 *       (dtsession locks when the screen saver comes on).  An
 *       inhibition ends with UnInhibit or when its owner leaves the
 *       bus.  Lock locks the display, SimulateUserActivity resets the
 *       idle timer.
 *
 *   org.freedesktop.Notifications (session bus)
 *       Notifications are shown as small windows at the top right of
 *       the screen, until they expire or are clicked (a click also
 *       invokes the notification's "default" action).  Only if no
 *       other notification daemon has the name.
 *
 *   org.freedesktop.login1 (system bus)
 *       "loginctl lock-session" (and anything that asks logind to
 *       lock the session) locks the display, and so does going to
 *       sleep: dtdbus holds a "delay" inhibitor lock on sleep, locks
 *       the display when logind announces the suspend, and releases
 *       the lock once dtsession has answered (or after 3 s), so that
 *       the machine wakes up locked.
 *
 * The display is locked by sending dtsession's Display_Lock request
 * over ToolTalk (what the LockDisplay action does).
 *
 * Xsession starts dtdbus in the background; it exits when the X
 * connection closes.  DTDBUS=no in ~/.dtprofile leaves it out.  With
 * -v it reports what it does on stderr, which Xsession sends to
 * ~/.dt/startlog.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/extensions/scrnsaver.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>

#include <systemd/sd-bus.h>

#include <Tt/tt_c.h>

#ifndef DTDBUS_VERSION
#define DTDBUS_VERSION "2.5"
#endif

static XtAppContext	app;
static Display		*dpy;
static Widget		toplevel;
static int		verbose;

static void
note(const char *fmt, ...)
{
    va_list ap;

    if (!verbose)
	return;
    fputs("dtdbus: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

static void
warn(const char *fmt, ...)
{
    va_list ap;

    fputs("dtdbus: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}


/*
 * sd-bus in the Xt main loop: input on the bus descriptor and sd-bus's
 * own timeouts make us process the bus; after processing, whatever is
 * queued is written out (the buses are local sockets).
 */

typedef struct {
    sd_bus		*bus;
    XtInputId		input;
    XtIntervalId	timer;
} BusWatch;

static void bus_process(BusWatch *w);

static void
bus_timeout_cb(XtPointer data, XtIntervalId *id)
{
    BusWatch *w = (BusWatch *) data;

    (void) id;
    w->timer = 0;
    bus_process(w);
}

static void
bus_input_cb(XtPointer data, int *fd, XtInputId *id)
{
    (void) fd;
    (void) id;
    bus_process((BusWatch *) data);
}

static void
bus_process(BusWatch *w)
{
    uint64_t when;
    int r;

    do {
	r = sd_bus_process(w->bus, NULL);
    } while (r > 0);
    if (r < 0) {
	warn("D-Bus connection lost: %s", strerror(-r));
	if (w->input)
	    XtRemoveInput(w->input);
	w->input = 0;
	return;
    }
    (void) sd_bus_flush(w->bus);
    if (w->timer) {
	XtRemoveTimeOut(w->timer);
	w->timer = 0;
    }
    if (sd_bus_get_timeout(w->bus, &when) >= 0 && when != UINT64_MAX) {
	struct timespec now;
	uint64_t now_us;
	unsigned long ms;

	clock_gettime(CLOCK_MONOTONIC, &now);
	now_us = (uint64_t) now.tv_sec * 1000000 + now.tv_nsec / 1000;
	ms = when > now_us ? (unsigned long) ((when - now_us + 999) / 1000) : 0;
	w->timer = XtAppAddTimeOut(app, ms, bus_timeout_cb, w);
    }
}

static void
bus_watch(BusWatch *w, sd_bus *bus)
{
    w->bus = bus;
    w->input = XtAppAddInput(app, sd_bus_get_fd(bus),
			     (XtPointer) XtInputReadMask, bus_input_cb, w);
    w->timer = 0;
    bus_process(w);
}

static BusWatch user_watch, system_watch;


/*
 * Locking the display: dtsession's Display_Lock request over ToolTalk.
 */

static int tt_up;
static void (*lock_done_cb)(void);

static void
tt_input_cb(XtPointer data, int *fd, XtInputId *id)
{
    Tt_message m;

    (void) data;
    (void) fd;
    (void) id;
    m = tt_message_receive();
    if (m != NULL && tt_ptr_error(m) == TT_OK)
	tt_message_destroy(m);
}

static Tt_callback_action
lock_reply_cb(Tt_message m, Tt_pattern p)
{
    Tt_state state = tt_message_state(m);

    (void) p;
    if (state == TT_HANDLED || state == TT_FAILED) {
	note("Display_Lock %s", state == TT_HANDLED ? "done" : "failed");
	tt_message_destroy(m);
	if (lock_done_cb) {
	    void (*cb)(void) = lock_done_cb;

	    lock_done_cb = NULL;
	    cb();
	}
	return TT_CALLBACK_PROCESSED;
    }
    return TT_CALLBACK_CONTINUE;
}

/* Returns 0 if the request could not be sent. */
static int
lock_display(void (*done)(void))
{
    Tt_message m;
    char *session;

    if (!tt_up) {
	char *procid = tt_open();

	if (tt_ptr_error(procid) != TT_OK) {
	    warn("cannot lock the display: no ToolTalk session");
	    return 0;
	}
	tt_free(procid);
	XtAppAddInput(app, tt_fd(), (XtPointer) XtInputReadMask,
		      tt_input_cb, NULL);
	tt_up = 1;
    }
    m = tt_message_create();
    session = tt_default_session();
    tt_message_class_set(m, TT_REQUEST);
    tt_message_scope_set(m, TT_SESSION);
    tt_message_address_set(m, TT_PROCEDURE);
    tt_message_session_set(m, session);
    tt_message_op_set(m, "Display_Lock");
    tt_message_callback_add(m, lock_reply_cb);
    tt_free(session);
    lock_done_cb = done;
    if (tt_message_send(m) != TT_OK) {
	warn("cannot lock the display: Display_Lock not sent");
	tt_message_destroy(m);
	lock_done_cb = NULL;
	return 0;
    }
    note("Display_Lock sent");
    return 1;
}


/*
 * org.freedesktop.ScreenSaver
 */

typedef struct Inhibitor {
    struct Inhibitor	*next;
    uint32_t		cookie;
    char		*owner;		/* unique bus name */
    char		*app;
    char		*reason;
} Inhibitor;

static Inhibitor	*inhibitors;
static uint32_t		next_cookie = 1;
static int		have_xss;

static void
saver_suspend(int on)
{
    if (have_xss) {
	XScreenSaverSuspend(dpy, on ? True : False);
	XFlush(dpy);
    }
    note("screen saver %s", on ? "suspended" : "resumed");
}

static void
inhibitor_free(Inhibitor *i)
{
    free(i->owner);
    free(i->app);
    free(i->reason);
    free(i);
}

/* Removes the inhibitors matching cookie (if not 0) or owner (if not NULL). */
static int
inhibitors_remove(uint32_t cookie, const char *owner)
{
    Inhibitor **pp = &inhibitors;
    int removed = 0;

    while (*pp) {
	Inhibitor *i = *pp;

	if ((cookie && i->cookie == cookie) ||
	    (owner && strcmp(i->owner, owner) == 0)) {
	    note("inhibition %u (%s: %s) ends", i->cookie, i->app, i->reason);
	    *pp = i->next;
	    inhibitor_free(i);
	    removed++;
	} else {
	    pp = &i->next;
	}
    }
    if (removed && inhibitors == NULL)
	saver_suspend(0);
    return removed;
}

static int
ss_inhibit(sd_bus_message *m, void *data, sd_bus_error *err)
{
    const char *app_name, *reason;
    Inhibitor *i;
    int r;

    (void) data;
    (void) err;
    r = sd_bus_message_read(m, "ss", &app_name, &reason);
    if (r < 0)
	return r;
    i = calloc(1, sizeof *i);
    if (i == NULL)
	return -ENOMEM;
    i->cookie = next_cookie++;
    if (next_cookie == 0)
	next_cookie = 1;
    i->owner = strdup(sd_bus_message_get_sender(m) ?
		      sd_bus_message_get_sender(m) : "");
    i->app = strdup(app_name);
    i->reason = strdup(reason);
    if (!i->owner || !i->app || !i->reason) {
	inhibitor_free(i);
	return -ENOMEM;
    }
    if (inhibitors == NULL)
	saver_suspend(1);
    i->next = inhibitors;
    inhibitors = i;
    note("inhibition %u by %s (%s): %s", i->cookie, i->owner, app_name,
	 reason);
    return sd_bus_reply_method_return(m, "u", i->cookie);
}

static int
ss_uninhibit(sd_bus_message *m, void *data, sd_bus_error *err)
{
    uint32_t cookie;
    int r;

    (void) data;
    (void) err;
    r = sd_bus_message_read(m, "u", &cookie);
    if (r < 0)
	return r;
    (void) inhibitors_remove(cookie, NULL);
    return sd_bus_reply_method_return(m, "");
}

static int
ss_simulate_activity(sd_bus_message *m, void *data, sd_bus_error *err)
{
    (void) data;
    (void) err;
    XResetScreenSaver(dpy);
    XFlush(dpy);
    return sd_bus_reply_method_return(m, "");
}

static int
ss_get_active(sd_bus_message *m, void *data, sd_bus_error *err)
{
    XScreenSaverInfo info;
    int active = 0;

    (void) data;
    (void) err;
    if (have_xss &&
	XScreenSaverQueryInfo(dpy, DefaultRootWindow(dpy), &info))
	active = info.state == ScreenSaverOn;
    return sd_bus_reply_method_return(m, "b", active);
}

static int
ss_set_active(sd_bus_message *m, void *data, sd_bus_error *err)
{
    int on, r;

    (void) data;
    (void) err;
    r = sd_bus_message_read(m, "b", &on);
    if (r < 0)
	return r;
    XForceScreenSaver(dpy, on ? ScreenSaverActive : ScreenSaverReset);
    XFlush(dpy);
    return sd_bus_reply_method_return(m, "b", 1);
}

static int
ss_lock(sd_bus_message *m, void *data, sd_bus_error *err)
{
    (void) data;
    (void) err;
    (void) lock_display(NULL);
    return sd_bus_reply_method_return(m, "");
}

static int
ss_idle_time(sd_bus_message *m, void *data, sd_bus_error *err)
{
    XScreenSaverInfo info;
    uint32_t idle = 0;

    (void) data;
    (void) err;
    if (have_xss &&
	XScreenSaverQueryInfo(dpy, DefaultRootWindow(dpy), &info))
	idle = (uint32_t) (info.idle / 1000);
    return sd_bus_reply_method_return(m, "u", idle);
}

static int
ss_active_time(sd_bus_message *m, void *data, sd_bus_error *err)
{
    (void) data;
    (void) err;
    return sd_bus_reply_method_return(m, "u", (uint32_t) 0);
}

static const sd_bus_vtable screensaver_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("Inhibit", "ss", "u", ss_inhibit,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("UnInhibit", "u", "", ss_uninhibit,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("SimulateUserActivity", "", "", ss_simulate_activity,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetActive", "", "b", ss_get_active,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("SetActive", "b", "b", ss_set_active,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Lock", "", "", ss_lock, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetActiveTime", "", "u", ss_active_time,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetSessionIdleTime", "", "u", ss_idle_time,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_SIGNAL("ActiveChanged", "b", 0),
    SD_BUS_VTABLE_END
};

/* An inhibitor's owner left the bus: its inhibitions end. */
static int
name_owner_changed(sd_bus_message *m, void *data, sd_bus_error *err)
{
    const char *name, *old_owner, *new_owner;

    (void) data;
    (void) err;
    if (sd_bus_message_read(m, "sss", &name, &old_owner, &new_owner) < 0)
	return 0;
    if (*old_owner && !*new_owner)
	(void) inhibitors_remove(0, old_owner);
    return 0;
}


/*
 * org.freedesktop.Notifications
 */

typedef struct Note {
    struct Note		*next;
    uint32_t		id;
    Widget		shell;
    Widget		summary;
    Widget		body;
    XtIntervalId	timer;
    int			has_default;
} Note;

static Note	*notes;
static uint32_t	next_note_id = 1;
static sd_bus	*user_bus;

#define NOTE_WIDTH_CHARS	56
#define NOTE_DEFAULT_MS		6000
#define NOTE_MARGIN		8

/* The body as plain text: markup tags removed, entities decoded,
 * lines wrapped at NOTE_WIDTH_CHARS. */
static char *
plain_text(const char *s)
{
    size_t n = strlen(s);
    char *out = malloc(n + 1), *o = out;
    int col = 0;
    char *last_space = NULL;

    if (out == NULL)
	return NULL;
    while (*s) {
	char c = *s;

	if (c == '<') {
	    const char *e = strchr(s, '>');

	    if (e) {
		if (strncmp(s, "<br", 3) == 0) {
		    *o++ = '\n';
		    col = 0;
		    last_space = NULL;
		}
		s = e + 1;
		continue;
	    }
	}
	if (c == '&') {
	    static const struct { const char *ent; char ch; } ents[] = {
		{ "&amp;", '&' }, { "&lt;", '<' }, { "&gt;", '>' },
		{ "&quot;", '"' }, { "&apos;", '\'' }, { "&#39;", '\'' },
	    };
	    size_t k;

	    for (k = 0; k < sizeof ents / sizeof ents[0]; k++) {
		size_t l = strlen(ents[k].ent);

		if (strncmp(s, ents[k].ent, l) == 0) {
		    c = ents[k].ch;
		    s += l - 1;
		    break;
		}
	    }
	}
	s++;
	if (c == '\r')
	    continue;
	if (c == '\n') {
	    *o++ = '\n';
	    col = 0;
	    last_space = NULL;
	    continue;
	}
	if (c == ' ' || c == '\t') {
	    c = ' ';
	    last_space = o;
	}
	*o++ = c;
	/* count characters, not the continuation bytes of UTF-8 */
	if (((unsigned char) c & 0xC0) != 0x80)
	    col++;
	if (col > NOTE_WIDTH_CHARS && last_space) {
	    *last_space = '\n';
	    col = 0;
	    for (char *p = last_space + 1; p < o; p++)
		if (((unsigned char) *p & 0xC0) != 0x80)
		    col++;
	    last_space = NULL;
	}
    }
    *o = '\0';
    return out;
}

static void
set_label(Widget w, const char *text)
{
    XmString xs = XmStringGenerate((XtPointer) text, NULL, XmCHARSET_TEXT,
				   NULL);

    XtVaSetValues(w, XmNlabelString, xs, NULL);
    XmStringFree(xs);
}

/* Stacks the notifications down from the top right corner. */
static void
notes_layout(void)
{
    Screen *scr = XtScreen(toplevel);
    int y = NOTE_MARGIN;
    Note *n;

    for (n = notes; n; n = n->next) {
	Dimension w = 0, h = 0;

	XtVaGetValues(n->shell, XmNwidth, &w, XmNheight, &h, NULL);
	XtVaSetValues(n->shell,
		      XmNx, (Position) (WidthOfScreen(scr) - w - NOTE_MARGIN),
		      XmNy, (Position) y, NULL);
	y += h + NOTE_MARGIN;
    }
}

static void
note_emit_closed(uint32_t id, uint32_t reason)
{
    if (user_bus) {
	(void) sd_bus_emit_signal(user_bus, "/org/freedesktop/Notifications",
				  "org.freedesktop.Notifications",
				  "NotificationClosed", "uu", id, reason);
	bus_process(&user_watch);
    }
}

/* reason: 1 expired, 2 dismissed by the user, 3 CloseNotification */
static void
note_close(Note *n, uint32_t reason)
{
    Note **pp;

    for (pp = &notes; *pp; pp = &(*pp)->next) {
	if (*pp == n) {
	    *pp = n->next;
	    break;
	}
    }
    if (n->timer)
	XtRemoveTimeOut(n->timer);
    XtDestroyWidget(n->shell);
    note("notification %u closed (%u)", n->id, reason);
    note_emit_closed(n->id, reason);
    free(n);
    notes_layout();
}

static void
note_expire_cb(XtPointer data, XtIntervalId *id)
{
    Note *n = (Note *) data;

    (void) id;
    n->timer = 0;
    note_close(n, 1);
}

static void
note_click_cb(Widget w, XtPointer data, XEvent *ev, Boolean *cont)
{
    Note *n = (Note *) data;

    (void) w;
    (void) cont;
    if (ev->type != ButtonPress)
	return;
    if (n->has_default && user_bus) {
	(void) sd_bus_emit_signal(user_bus, "/org/freedesktop/Notifications",
				  "org.freedesktop.Notifications",
				  "ActionInvoked", "us", n->id, "default");
    }
    note_close(n, 2);
}

static Note *
note_find(uint32_t id)
{
    Note *n;

    for (n = notes; n; n = n->next)
	if (n->id == id)
	    return n;
    return NULL;
}

static Note *
note_create(uint32_t id)
{
    Note *n = calloc(1, sizeof *n);
    Widget form, sep;
    Note **tail;

    if (n == NULL)
	return NULL;
    n->id = id;
    n->shell = XtVaCreatePopupShell("notification",
				    overrideShellWidgetClass, toplevel,
				    XmNallowShellResize, True, NULL);
    form = XtVaCreateManagedWidget("form", xmFormWidgetClass, n->shell,
				   XmNmarginWidth, 10, XmNmarginHeight, 8,
				   XmNshadowThickness, 2,
				   XmNshadowType, XmSHADOW_OUT, NULL);
    n->summary = XtVaCreateManagedWidget("summary", xmLabelWidgetClass, form,
					 XmNalignment, XmALIGNMENT_BEGINNING,
					 XmNtopAttachment, XmATTACH_FORM,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_FORM,
					 NULL);
    sep = XtVaCreateManagedWidget("separator", xmSeparatorWidgetClass, form,
				  XmNtopAttachment, XmATTACH_WIDGET,
				  XmNtopWidget, n->summary,
				  XmNleftAttachment, XmATTACH_FORM,
				  XmNrightAttachment, XmATTACH_FORM, NULL);
    n->body = XtVaCreateManagedWidget("body", xmLabelWidgetClass, form,
				      XmNalignment, XmALIGNMENT_BEGINNING,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, sep,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
				      NULL);
    XtAddEventHandler(form, ButtonPressMask, False, note_click_cb, n);
    XtAddEventHandler(n->summary, ButtonPressMask, False, note_click_cb, n);
    XtAddEventHandler(n->body, ButtonPressMask, False, note_click_cb, n);
    /* newest last, at the bottom of the stack */
    for (tail = &notes; *tail; tail = &(*tail)->next)
	;
    *tail = n;
    return n;
}

static int
nt_notify(sd_bus_message *m, void *data, sd_bus_error *err)
{
    const char *app_name, *icon, *summary, *body;
    uint32_t replaces;
    int32_t timeout;
    int urgency = 1, has_default = 0, r;
    char *text;
    Note *n;

    (void) data;
    (void) err;
    r = sd_bus_message_read(m, "susss", &app_name, &replaces, &icon,
			    &summary, &body);
    if (r < 0)
	return r;

    /* actions: pairs of key and label */
    r = sd_bus_message_enter_container(m, 'a', "s");
    if (r < 0)
	return r;
    for (;;) {
	const char *key, *label;

	r = sd_bus_message_read(m, "s", &key);
	if (r <= 0)
	    break;
	r = sd_bus_message_read(m, "s", &label);
	if (r <= 0)
	    break;
	if (strcmp(key, "default") == 0)
	    has_default = 1;
    }
    if (r < 0)
	return r;
    (void) sd_bus_message_exit_container(m);

    /* hints: only "urgency" (byte) */
    r = sd_bus_message_enter_container(m, 'a', "{sv}");
    if (r < 0)
	return r;
    while ((r = sd_bus_message_enter_container(m, 'e', "sv")) > 0) {
	const char *key;

	r = sd_bus_message_read(m, "s", &key);
	if (r < 0)
	    return r;
	if (strcmp(key, "urgency") == 0 &&
	    sd_bus_message_enter_container(m, 'v', "y") > 0) {
	    uint8_t u;

	    if (sd_bus_message_read(m, "y", &u) > 0)
		urgency = u;
	    (void) sd_bus_message_exit_container(m);
	} else {
	    r = sd_bus_message_skip(m, "v");
	    if (r < 0)
		return r;
	}
	(void) sd_bus_message_exit_container(m);
    }
    if (r < 0)
	return r;
    (void) sd_bus_message_exit_container(m);
    r = sd_bus_message_read(m, "i", &timeout);
    if (r < 0)
	return r;

    n = replaces ? note_find(replaces) : NULL;
    if (n == NULL) {
	uint32_t id = next_note_id++;

	if (next_note_id == 0)
	    next_note_id = 1;
	n = note_create(id);
	if (n == NULL)
	    return -ENOMEM;
    }
    n->has_default = has_default;
    text = plain_text(summary && *summary ? summary : app_name);
    set_label(n->summary, text ? text : "");
    free(text);
    text = plain_text(body);
    if (text && *text) {
	set_label(n->body, text);
	XtManageChild(n->body);
    } else {
	XtUnmanageChild(n->body);
    }
    free(text);
    XtPopup(n->shell, XtGrabNone);
    notes_layout();

    if (n->timer) {
	XtRemoveTimeOut(n->timer);
	n->timer = 0;
    }
    if (timeout < 0)
	timeout = urgency >= 2 ? 0 : NOTE_DEFAULT_MS;
    if (timeout > 0)
	n->timer = XtAppAddTimeOut(app, (unsigned long) timeout,
				   note_expire_cb, n);
    note("notification %u from %s: %s", n->id, app_name, summary);
    return sd_bus_reply_method_return(m, "u", n->id);
}

static int
nt_close(sd_bus_message *m, void *data, sd_bus_error *err)
{
    uint32_t id;
    Note *n;
    int r;

    (void) data;
    (void) err;
    r = sd_bus_message_read(m, "u", &id);
    if (r < 0)
	return r;
    if ((n = note_find(id)) != NULL)
	note_close(n, 3);
    return sd_bus_reply_method_return(m, "");
}

static int
nt_capabilities(sd_bus_message *m, void *data, sd_bus_error *err)
{
    sd_bus_message *reply = NULL;
    int r;

    (void) data;
    (void) err;
    r = sd_bus_message_new_method_return(m, &reply);
    if (r >= 0)
	r = sd_bus_message_append_strv(reply,
				       (char *[]) { "body", "actions", NULL });
    if (r >= 0)
	r = sd_bus_send(NULL, reply, NULL);
    sd_bus_message_unref(reply);
    return r;
}

static int
nt_server_info(sd_bus_message *m, void *data, sd_bus_error *err)
{
    (void) data;
    (void) err;
    return sd_bus_reply_method_return(m, "ssss", "CDE dtdbus",
				      "Common Desktop Environment",
				      DTDBUS_VERSION, "1.2");
}

static const sd_bus_vtable notifications_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("Notify", "susssasa{sv}i", "u", nt_notify,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("CloseNotification", "u", "", nt_close,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetCapabilities", "", "as", nt_capabilities,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetServerInformation", "", "ssss", nt_server_info,
		  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_SIGNAL("NotificationClosed", "uu", 0),
    SD_BUS_SIGNAL("ActionInvoked", "us", 0),
    SD_BUS_VTABLE_END
};


/*
 * logind: lock on request and before sleep
 */

static sd_bus		*system_bus;
static int		sleep_fd = -1;
static XtIntervalId	sleep_timer;

static void
sleep_inhibit(void)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    int fd, r;

    if (sleep_fd >= 0 || system_bus == NULL)
	return;
    r = sd_bus_call_method(system_bus, "org.freedesktop.login1",
			   "/org/freedesktop/login1",
			   "org.freedesktop.login1.Manager", "Inhibit",
			   &err, &reply, "ssss", "sleep", "CDE",
			   "Lock the screen before sleeping", "delay");
    if (r < 0) {
	note("no sleep inhibitor: %s", err.message ? err.message : "");
    } else if (sd_bus_message_read(reply, "h", &fd) > 0) {
	sleep_fd = fcntl(fd, F_DUPFD_CLOEXEC, 3);
	note("holding a sleep delay lock");
    }
    sd_bus_error_free(&err);
    sd_bus_message_unref(reply);
}

static void
sleep_release(void)
{
    if (sleep_timer) {
	XtRemoveTimeOut(sleep_timer);
	sleep_timer = 0;
    }
    if (sleep_fd >= 0) {
	close(sleep_fd);
	sleep_fd = -1;
	note("sleep delay lock released");
    }
}

static void
sleep_timeout_cb(XtPointer data, XtIntervalId *id)
{
    (void) data;
    (void) id;
    sleep_timer = 0;
    sleep_release();
}

static int
prepare_for_sleep(sd_bus_message *m, void *data, sd_bus_error *err)
{
    int going;

    (void) data;
    (void) err;
    if (sd_bus_message_read(m, "b", &going) < 0)
	return 0;
    if (going) {
	note("going to sleep: locking");
	if (lock_display(sleep_release)) {
	    /* do not hold the suspend up for long */
	    sleep_timer = XtAppAddTimeOut(app, 3000, sleep_timeout_cb, NULL);
	} else {
	    sleep_release();
	}
    } else {
	note("woken up");
	sleep_inhibit();
    }
    return 0;
}

static int
session_lock(sd_bus_message *m, void *data, sd_bus_error *err)
{
    (void) m;
    (void) data;
    (void) err;
    note("logind asks to lock the session");
    (void) lock_display(NULL);
    return 0;
}

static void
logind_setup(void)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    const char *id = getenv("XDG_SESSION_ID");
    const char *path = NULL;
    int r;

    r = sd_bus_open_system(&system_bus);
    if (r < 0) {
	note("no system bus: %s", strerror(-r));
	system_bus = NULL;
	return;
    }
    if (id && *id) {
	r = sd_bus_call_method(system_bus, "org.freedesktop.login1",
			       "/org/freedesktop/login1",
			       "org.freedesktop.login1.Manager", "GetSession",
			       &err, &reply, "s", id);
    } else {
	r = sd_bus_call_method(system_bus, "org.freedesktop.login1",
			       "/org/freedesktop/login1",
			       "org.freedesktop.login1.Manager",
			       "GetSessionByPID", &err, &reply, "u",
			       (uint32_t) getpid());
    }
    if (r >= 0 && sd_bus_message_read(reply, "o", &path) > 0) {
	(void) sd_bus_match_signal(system_bus, NULL, "org.freedesktop.login1",
				   path, "org.freedesktop.login1.Session",
				   "Lock", session_lock, NULL);
	note("logind session %s", path);
    } else {
	note("no logind session: %s", err.message ? err.message : "");
    }
    sd_bus_error_free(&err);
    sd_bus_message_unref(reply);
    (void) sd_bus_match_signal(system_bus, NULL, "org.freedesktop.login1",
			       "/org/freedesktop/login1",
			       "org.freedesktop.login1.Manager",
			       "PrepareForSleep", prepare_for_sleep, NULL);
    sleep_inhibit();
    bus_watch(&system_watch, system_bus);
}


/*
 * Setup
 */

static int
xio_error(Display *d)
{
    (void) d;
    /* the session's X server is gone: so is the session */
    exit(0);
    return 0;
}

static int
claim_name(sd_bus *bus, const char *name)
{
    int r = sd_bus_request_name(bus, name, 0);

    if (r < 0) {
	note("%s: %s (another service has it)", name, strerror(-r));
	return 0;
    }
    note("serving %s", name);
    return 1;
}

static void
user_bus_setup(void)
{
    int r, any = 0;

    r = sd_bus_open_user(&user_bus);
    if (r < 0) {
	warn("no session bus: %s", strerror(-r));
	user_bus = NULL;
	return;
    }
    if (sd_bus_add_object_vtable(user_bus, NULL,
				 "/org/freedesktop/ScreenSaver",
				 "org.freedesktop.ScreenSaver",
				 screensaver_vtable, NULL) >= 0 &&
	sd_bus_add_object_vtable(user_bus, NULL, "/ScreenSaver",
				 "org.freedesktop.ScreenSaver",
				 screensaver_vtable, NULL) >= 0) {
	any |= claim_name(user_bus, "org.freedesktop.ScreenSaver");
    }
    if (sd_bus_add_object_vtable(user_bus, NULL,
				 "/org/freedesktop/Notifications",
				 "org.freedesktop.Notifications",
				 notifications_vtable, NULL) >= 0) {
	any |= claim_name(user_bus, "org.freedesktop.Notifications");
    }
    (void) sd_bus_match_signal(user_bus, NULL, "org.freedesktop.DBus",
			       "/org/freedesktop/DBus",
			       "org.freedesktop.DBus", "NameOwnerChanged",
			       name_owner_changed, NULL);
    if (!any)
	note("no session bus service to provide");
    bus_watch(&user_watch, user_bus);
}

static void
usage(void)
{
    fprintf(stderr, "usage: dtdbus [-v] [-display display]\n");
    exit(2);
}

int
main(int argc, char **argv)
{
    int i, ev, er;

    XtSetLanguageProc(NULL, NULL, NULL);
    toplevel = XtVaOpenApplication(&app, "Dtdbus", NULL, 0, &argc, argv,
				   NULL, applicationShellWidgetClass,
				   XmNmappedWhenManaged, False,
				   XmNwidth, 1, XmNheight, 1, NULL);
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-v") == 0)
	    verbose = 1;
	else
	    usage();
    }
    dpy = XtDisplay(toplevel);
    XtRealizeWidget(toplevel);
    XSetIOErrorHandler(xio_error);
    have_xss = XScreenSaverQueryExtension(dpy, &ev, &er);
    if (!have_xss)
	warn("no MIT-SCREEN-SAVER extension: inhibitions are ignored");

    user_bus_setup();
    logind_setup();
    if (user_bus == NULL && system_bus == NULL) {
	warn("no D-Bus at all: nothing to do");
	return 1;
    }
    XtAppMainLoop(app);
    return 0;
}
