/*
 * CDE - Common Desktop Environment
 *
 * xsetedid OUTPUT HEX | xsetedid -d OUTPUT
 *
 * Set (or delete) the EDID property of an output, which makes a test X
 * server look as if another monitor was plugged in: the dtrandr tests
 * use it to wake up dtrandr --daemon.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>

int
main(int argc, char **argv)
{
    int del = argc == 3 && !strcmp(argv[1], "-d");
    const char *name = argv[1 + del];
    unsigned char data[512];
    XRRScreenResources *res;
    Display *dpy;
    Atom edid;
    size_t n = 0;
    int i;

    if (argc != 3) {
        fprintf(stderr, "usage: xsetedid OUTPUT HEX | xsetedid -d OUTPUT\n");
        return 2;
    }
    if (!del)
        for (n = 0; n < sizeof data && argv[2][2 * n]; n++) {
            unsigned v;

            if (sscanf(argv[2] + 2 * n, "%2x", &v) != 1)
                return 2;
            data[n] = (unsigned char) v;
        }
    if (!(dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, "xsetedid: cannot open the display\n");
        return 1;
    }
    edid = XInternAtom(dpy, "EDID", False);
    res = XRRGetScreenResources(dpy, DefaultRootWindow(dpy));
    for (i = 0; res && i < res->noutput; i++) {
        XRROutputInfo *oi = XRRGetOutputInfo(dpy, res, res->outputs[i]);
        int match = oi && !strcmp(oi->name, name);

        if (oi)
            XRRFreeOutputInfo(oi);
        if (!match)
            continue;
        if (del)
            XRRDeleteOutputProperty(dpy, res->outputs[i], edid);
        else
            XRRChangeOutputProperty(dpy, res->outputs[i], edid, XA_INTEGER,
                                    8, PropModeReplace, data, (int) n);
        XSync(dpy, False);
        XRRFreeScreenResources(res);
        XCloseDisplay(dpy);
        return 0;
    }
    fprintf(stderr, "xsetedid: no output %s\n", name);
    return 1;
}
