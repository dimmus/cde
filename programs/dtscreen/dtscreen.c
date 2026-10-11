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
/* $TOG: dtscreen.c /main/6 1998/04/20 12:58:46 mgreess $ */
/*
 */

/*-
 * dtscreen.c - X11 client to lock a display and show a screen saver.
 *
 * Copyright (c) 1988-91 by Patrick J. Naughton.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * Comments and additions should be sent to the author:
 *
 *                     naughton@eng.sun.com
 *
 *                     Patrick J. Naughton
 *                     MS 21-14
 *                     Sun Laboritories, Inc.
 *                     2550 Garcia Ave
 *                     Mountain View, CA  94043
 *
 * Revision History:
 * 16-May-91: added pyro and random modes.
 *            ripped big comment block out of all other files.
 * 29-Oct-90: added cast to XFree() arg.
 *            added volume arg to call to XBell().
 * 31-Aug-90: added blank mode.
 *            added swarm mode.
 *            moved usleep() and seconds() out to usleep.c.
 *            added SVR4 defines to dtscreen.h
 * 29-Jul-90: added support for multiple screens to be locked by one dtscreen.
 *            moved global defines to dtscreen.h
 *            removed use of allowsig().
 * 07-Jul-90: reworked commandline args and resources to use Xrm.
 *            moved resource processing out to resource.c
 * 02-Jul-90: reworked colors to not use dynamic colormap.
 * 23-May-90: added autoraise when obscured.
 * 18-Feb-90: added SunOS3.5 fix.
 *            changed -mono -> -color, and -saver -> -lock.
 *            allow non-locking screensavers to display on remote machine.
 *            added -echokeys to disable echoing of '?'s on input.
 *            cleaned up all of the parameters and defaults.
 * 23-Sep-89: Added fix to allow local hostname:0 as a display.
 *            Put empty case for Enter/Leave events.
 *            Moved colormap installation later in startup.
 * 19-Sep-89: Added '?'s on input.
 * 27-Mar-89: Added -qix mode.
 *            Fixed GContext->GC.
 * 08-Mar-89: Added -nice, -mode and -display, built vector for life and hop.
 * 24-Feb-89: Replaced hopalong display with life display from SunView1.
 * 22-Feb-89: Added fix for color servers with n < 8 planes.
 * 16-Feb-89: Updated calling conventions for XCreateHsbColormap();
 *            Added -count for number of iterations per color.
 *            Fixed defaulting mechanism.
 *            Ripped out VMS hacks.
 *            Sent to expo for X11R3 contrib.
 * 19-Jan-89: Fixed monochrome gc bug.
 * 19-Sep-88: Changed -color to -mono. (default is color on color displays).
 *            Added -saver option. (just do display... don't lock.)
 * 01-Apr-88: Added XGrabServer/XUngrabServer for more security.
 * 27-Mar-88: Rotate fractal by 45 degrees clockwise.
 * 24-Mar-88: Added color support. [-color]
 *            wrote the man page.
 * 23-Mar-88: Added HOPALONG routines from Scientific American Sept. 86 p. 14.
 *            added requirement for display to be "unix:0".
 * 22-Mar-88: Recieved Walter Milliken's comp.windows.x posting.
 *
 */

/*              include file for message texts          */
#include <limits.h>
#include <Dt/MsgCatP.h>
#define MF_DTSCREEN "dtscreen"

#include <Dt/MsgCatP.h>
#include <locale.h>
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <time.h>

#include <X11/Intrinsic.h> /* For Boolean */
#include <X11/Shell.h>
#include <X11/extensions/dpms.h>
#include "dtscreen.h"
#ifdef NEVER
/* We'd like to include DtP.h, but it interferes with dtscreen.h */
# include <Dt/DtP.h>
#endif /* NEVER */
#include <Dt/EnvControlP.h>
#include <Dt/Saver.h>

#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE 0
#endif

char       *ProgramName;        /* argv[0] */
Display    *dsp = NULL;         /* server display connection */
void        (*callback) () = NULL;
void        (*init) () = NULL;

static perwindow *Win; /* perwindow information */
static int  windows;            /* number of windows */
static Window *winprop = NULL;  /* dtsession cover windows */

extern void drawblank();        /* blank.c */

/* VARARGS1 */
void
error(char *s1, char *s2)
{
    fprintf(stderr, s1, ProgramName, s2);
    exit(1);
}

/*
 * Restore all grabs, reset screensaver, restore colormap, close connection.
 */
static void
finish(void)
{
    XSync(dsp, False);
    if (winprop) 
      XFree((char *)winprop);
    XFlush(dsp);
    XCloseDisplay(dsp);
}


/*
 * Frame pacing.  Each mode's "delay" is the period between frames; it is
 * never shorter than MIN_FRAME_US (about 60 frames per second), so "hop"
 * (delay 0) no longer spins a CPU.  Between frames we block in poll() on
 * the connection, so events are still handled promptly.
 */
#define MIN_FRAME_US	16667L
#define DPMS_CHECK_US	2000000L	/* how often to ask whether the monitor is on */
#define DPMS_OFF_US	1000000L	/* frame period while the monitor is off */

static long long
now_us(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

static void
handleEvent(XEvent *event)
{
#ifndef DEBUG
    if (event->type == VisibilityNotify)
        XRaiseWindow(dsp, event->xany.window);
#else
    (void) event;
#endif
}

/* True when DPMS has put the monitor to sleep, so drawing is wasted. */
static Bool
monitorOff(void)
{
    static int haveDPMS = -1;
    int dummy;
    CARD16 level;
    BOOL enabled;

    if (haveDPMS < 0)
        haveDPMS = DPMSQueryExtension(dsp, &dummy, &dummy) && DPMSCapable(dsp);
    if (!haveDPMS || !DPMSInfo(dsp, &level, &enabled))
        return False;
    return enabled && level != DPMSModeOn;
}

/* Handle events until 'deadline' (CLOCK_MONOTONIC, microseconds). */
static void
waitUntil(long long deadline)
{
    struct pollfd pfd;
    XEvent event;

    pfd.fd = ConnectionNumber(dsp);
    pfd.events = POLLIN;
    for (;;) {
        long long left;

        while (XPending(dsp)) {
            XNextEvent(dsp, &event);
            handleEvent(&event);
        }
        left = deadline - now_us();
        if (left <= 0)
            return;
        /* round up so we do not wake just before the deadline */
        if (poll(&pfd, 1, (int)((left + 999) / 1000)) < 0 && errno != EINTR)
            return;
    }
}

static void
justDisplay(void)
{
    XEvent      event;
    int         window;
    long long   period, next, nextDpmsCheck;
    Bool        off = False;

    for (window = 0; window < windows; window++)
        init(Win+window);

    /* "blank" draws nothing after init: just block for events. */
    if (callback == drawblank) {
        XFlush(dsp);
        for (;;) {
            XNextEvent(dsp, &event);
            handleEvent(&event);
        }
    }

    period = delay > MIN_FRAME_US ? delay : MIN_FRAME_US;
    next = now_us();
    nextDpmsCheck = next + DPMS_CHECK_US;
    for (;;) {
        long long now;

        if (!off)
            for (window = 0; window < windows; window++)
                callback(Win+window);
        /* Round trip: keeps us from queueing frames faster than the
         * server draws them. */
        XSync(dsp, False);

        now = now_us();
        if (now >= nextDpmsCheck) {
            off = monitorOff();
            nextDpmsCheck = now + DPMS_CHECK_US;
        }
        next += off ? DPMS_OFF_US : period;
        if (next < now)         /* fell behind: do not try to catch up */
            next = now;
        waitUntil(next);
    }
}


long
allocpixel(Colormap cmap, char *name, char *def)
{
    XColor      col;
    XColor      tmp;
    XParseColor(dsp, cmap, name, &col);
    if (!XAllocColor(dsp, cmap, &col)) {
#ifdef MIT_R5

        fprintf(stderr, 
             "couldn't allocate: %s, using %s instead\n",
                name, def);

#else
        fprintf(stderr, CATGETS(scmc_catd, 2, 35,
             "couldn't allocate: %s, using %s instead\n"),
                name, def);
#endif

        XAllocNamedColor(dsp, cmap, def, &col, &tmp);
    }
    return col.pixel;
}


int
main(int argc, char *argv[])
{
    XSetWindowAttributes xswa;
    XGCValues   xgcv;
    int         nitems = 0;
    int         window;
    int i;

   /*
    * So we can find the app-defaults file.
    */
    _DtEnvControl(DT_ENV_SET);

    /* set locale to what is defined by local environment */
    setlocale(LC_ALL,"");
    scmc_catd = CATOPEN(MF_DTSCREEN, NL_CAT_LOCALE);

    ProgramName = strrchr(argv[0], '/');
    if (ProgramName)
        ProgramName++;
    else
        ProgramName = argv[0];

    srandom(time((time_t *) 0));  /* random mode needs the seed set. */

    GetResources(argc, argv);

    CheckResources();

   if (!create)
   {
     /* Make sure DtSvc is initialized properly. */
     XtAppContext appContext;
     Widget shellWidget;
     int dummyArgc = 1;
     char *dummyArgv[1];

     dummyArgv[0] = ProgramName;
     XtToolkitInitialize();
     appContext = XtCreateApplicationContext();
     XtDisplayInitialize(appContext, dsp, ProgramName, ProgramName,
			 NULL, 0, &dummyArgc, dummyArgv);
     shellWidget = XtAppCreateShell(ProgramName, ProgramName,
				    applicationShellWidgetClass,
				    dsp, NULL, 0);
     DtAppInitialize(appContext, dsp, shellWidget,
		     ProgramName, ProgramName);

     /*
      * Try to get array of window ids from desktop. Also returned is
      * a property identifier whose change will tell us when to terminate.
      */
      if (!DtSaverGetWindows(dsp, &winprop, &nitems))
      {
        fprintf(stderr, CATGETS(scmc_catd, 2, 40,
             "%s: Cannot locate window in which to draw. Using the -create\n"
             "option will cause %s to create its own window.\n"),
             ProgramName, ProgramName);

        exit(1);
      }

      Win = (perwindow *)malloc(nitems * sizeof(perwindow));
      windows = nitems;
      for (window = 0; window < windows; window++)
      {
        Win[window].w = winprop[window];
        Win[window].perscreen = NULL;
        Win[window].data = NULL;
      }
    }
    else
    {
      Screen     *scr = ScreenOfDisplay(dsp, 0);

      Win = (perwindow *)malloc(sizeof(perwindow));
      windows = 1;
      xswa.override_redirect = True;
      xswa.background_pixel = BlackPixelOfScreen(scr);
      xswa.event_mask = VisibilityChangeMask;

      Win[0].w = XCreateWindow(
                      dsp,
                      RootWindowOfScreen(scr),
                      0, 0,
                      WidthOfScreen(scr) - 100, HeightOfScreen(scr) - 100,
                      0, CopyFromParent, InputOutput, CopyFromParent,
                      CWBackPixel | CWEventMask, &xswa);
      Win[0].perscreen = NULL;
      Win[0].data = NULL;
      XMapWindow(dsp, Win[0].w);
      XRaiseWindow(dsp, Win[0].w);
    }

    for (window = 0; window < windows; window++)
    {
      XWindowAttributes attr;

      XGetWindowAttributes(dsp, Win[window].w, &attr);

      for (i = 0; i < window; i++)
      {
        if (Win[i].perscreen && Win[i].perscreen->screen == attr.screen)
        {
          Win[window].perscreen = Win[i].perscreen;
          break;
        }
      }

      if (Win[window].perscreen == NULL)
      {
        Colormap cmap;
        perscreen *pscreen;

        pscreen = (perscreen *)malloc(sizeof(perscreen));
        Win[window].perscreen = pscreen;

        pscreen->screen = attr.screen;
        cmap = DefaultColormapOfScreen(pscreen->screen);
        
        pscreen->pixels[0] = allocpixel(cmap, "White", "White");
        pscreen->pixels[1] = allocpixel(cmap, "Black", "Black");
        if (mono || CellsOfScreen(pscreen->screen) == 2)
        {
          pscreen->npixels = 2;
        }
        else
        {
          int         colorcount = NUMCOLORS;
          u_char      red[NUMCOLORS];
          u_char      green[NUMCOLORS];
          u_char      blue[NUMCOLORS];
  
          hsbramp(0.0, saturation, 1.0, 1.0, saturation, 1.0, colorcount,
                      red, green, blue);
          pscreen->npixels = 0;
          for (i = 0; i < colorcount; i++)
          {
            XColor      xcolor;

            xcolor.red = red[i] << 8;
            xcolor.green = green[i] << 8;
            xcolor.blue = blue[i] << 8;
            xcolor.flags = DoRed | DoGreen | DoBlue;

            if (!XAllocColor(dsp, cmap, &xcolor))
                break;

            pscreen->pixels[i] = xcolor.pixel;
            pscreen->npixels++;
          }
        }
      }

      xgcv.foreground = WhitePixelOfScreen(Win[window].perscreen->screen);
      xgcv.background = BlackPixelOfScreen(Win[window].perscreen->screen);
      Win[window].gc = XCreateGC(dsp, Win[window].w,
                         GCForeground | GCBackground, &xgcv);
    }
    if(-1 == nice(nicelevel)) {
      fprintf(stderr, "dtscreen: failed to set nice() level '%s'\n", strerror(errno));
    }

    justDisplay();

    finish();

    return 0;
}
