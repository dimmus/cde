/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: the X server, through libXrandr
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

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>

#include "backend.h"

#define BIT(o) ((DrOutputMask) 1 << (o))

typedef struct {
    Display *dpy;
    int screen;
    Window root;
    int rr_event, rr_error;
    int major, minor;
    XRRScreenResources *res;
    Window lock_win;
    Atom lock_atom;
    Atom edid_atoms[3];
} XB;

static const char *const edid_names[3] = {
    "EDID", "EdidData", "XFree86_DDC_EDID1_RAWDATA"
};

static int x_errors;
static char x_error_text[256];
static int x_daemon;

static int
x_error_handler(Display *dpy, XErrorEvent *e)
{
    char text[200];

    XGetErrorText(dpy, e->error_code, text, sizeof text);
    snprintf(x_error_text, sizeof x_error_text,
             "%s (request %d.%d)", text, e->request_code, e->minor_code);
    dr_debug("X error: %s", x_error_text);
    x_errors++;
    return 0;
}

static int
x_io_error_handler(Display *dpy)
{
    (void) dpy;
    if (x_daemon) {
        dr_info("the X connection is gone, exiting");
        exit(0);
    }
    fprintf(stderr, "%s: lost the connection to the X server\n",
            dr_progname);
    exit(1);
}

/* XSync and report an X error since errors was x_errors */
static int
x_check(XB *x, int errors, const char *what)
{
    XSync(x->dpy, False);
    if (x_errors != errors)
        return dr_error("%s failed: %s", what, x_error_text);
    return 0;
}

static int
mode_index(const XB *x, RRMode id)
{
    int i;

    for (i = 0; i < x->res->nmode; i++)
        if (x->res->modes[i].id == id)
            return i;
    return -1;
}

static int
crtc_index(const XB *x, RRCrtc id)
{
    int i;

    for (i = 0; i < x->res->ncrtc; i++)
        if (x->res->crtcs[i] == id)
            return i;
    return -1;
}

static int
output_index(const XB *x, RROutput id)
{
    int i;

    for (i = 0; i < x->res->noutput; i++)
        if (x->res->outputs[i] == id)
            return i;
    return -1;
}

static char *
read_edid(XB *x, RROutput out)
{
    int i;

    for (i = 0; i < 3; i++) {
        Atom type;
        int format;
        unsigned long n, after, k;
        unsigned char *data = NULL;
        char *hex;

        if (x->edid_atoms[i] == None)
            continue;
        if (XRRGetOutputProperty(x->dpy, out, x->edid_atoms[i], 0, 1024,
                                 False, False, AnyPropertyType, &type,
                                 &format, &n, &after, &data) != Success)
            continue;
        if (type == XA_INTEGER && format == 8 && n > 0) {
            hex = xmalloc(2 * n + 1);
            for (k = 0; k < n; k++)
                snprintf(hex + 2 * k, 3, "%02x", data[k]);
            XFree(data);
            return hex;
        }
        if (data)
            XFree(data);
    }
    return NULL;
}

/* The --gamma value of a ramp, as xrandr --verbose and autorandr get it */
static void
read_gamma(XB *x, RRCrtc crtc, int size, DrCrtcState *s)
{
    XRRCrtcGamma *g;
    unsigned short *ch[3];
    int last[3], i, j, best = 0, middle;
    double i1, v1, i2, v2, bright, e;

    if (size < 2 || !(g = XRRGetCrtcGamma(x->dpy, crtc)))
        return;
    ch[0] = g->red;
    ch[1] = g->green;
    ch[2] = g->blue;
    for (j = 0; j < 3; j++) {
        for (i = size - 1; i > 0 && ch[j][i] >= 0xffff; i--)
            ;
        last[j] = i;
        if (last[j] > last[best])
            best = j;
    }
    middle = last[best] / 2;
    i1 = (double) (middle + 1) / size;
    v1 = ch[best][middle] / 65535.0;
    i2 = (double) (last[best] + 1) / size;
    v2 = ch[best][last[best]] / 65535.0;
    s->has_gamma = 1;
    for (j = 0; j < 3; j++)
        s->gamma[j] = 1.0;
    if (v2 >= 0.0001 && v1 > 0) {
        if (last[best] + 1 == size)
            bright = v2;
        else
            bright = exp((log(v2) * log(i1) - log(v1) * log(i2))
                         / log(i1 / i2));
        for (j = 0; j < 3; j++) {
            double v = ch[j][last[j] / 2] / bright / 65535.0;
            double p = (double) (last[j] / 2 + 1) / size;
            char tmp[32];

            if (v <= 0 || p >= 1)
                continue;
            e = log(v) / log(p);
            /* xrandr prints it with %#.2g, autorandr takes 1/x */
            snprintf(tmp, sizeof tmp, "%.2g", e);
            e = strtod(tmp, NULL);
            if (e > 0)
                s->gamma[j] = round(1000.0 / e) / 1000.0;
        }
    }
    XRRFreeGamma(g);
}

static int
x_query(DrBackend *b, DrHardware *hw, int probe)
{
    XB *x = b->priv;
    Window rootw;
    int gx, gy, i, j;
    unsigned gw, gh, border, depth;
    RROutput primary;

    memset(hw, 0, sizeof *hw);
    if (x->res)
        XRRFreeScreenResources(x->res);
    x->res = probe ? XRRGetScreenResources(x->dpy, x->root)
                   : XRRGetScreenResourcesCurrent(x->dpy, x->root);
    if (!x->res)
        return dr_error("cannot get the screen resources");
    XRRGetScreenSizeRange(x->dpy, x->root, &hw->min_width, &hw->min_height,
                          &hw->max_width, &hw->max_height);
    XGetGeometry(x->dpy, x->root, &rootw, &gx, &gy, &gw, &gh, &border,
                 &depth);
    for (i = 0; i < 3; i++)
        if (x->edid_atoms[i] == None)
            x->edid_atoms[i] = XInternAtom(x->dpy, edid_names[i], True);

    hw->nmode = x->res->nmode;
    hw->modes = xcalloc(hw->nmode, sizeof *hw->modes);
    for (i = 0; i < hw->nmode; i++) {
        const XRRModeInfo *mi = &x->res->modes[i];
        DrMode *m = &hw->modes[i];

        m->xid = mi->id;
        m->name = xmalloc(mi->nameLength + 1);
        memcpy(m->name, mi->name, mi->nameLength);
        m->name[mi->nameLength] = '\0';
        m->width = mi->width;
        m->height = mi->height;
        m->dotclock = mi->dotClock;
        m->hsync_start = mi->hSyncStart;
        m->hsync_end = mi->hSyncEnd;
        m->htotal = mi->hTotal;
        m->hskew = mi->hSkew;
        m->vsync_start = mi->vSyncStart;
        m->vsync_end = mi->vSyncEnd;
        m->vtotal = mi->vTotal;
        m->flags = mi->modeFlags;
    }

    if (x->res->noutput > DR_MAX_OUTPUTS)
        return dr_error("too many outputs (%d)", x->res->noutput);
    hw->noutput = x->res->noutput;
    hw->outputs = xcalloc(hw->noutput, sizeof *hw->outputs);
    for (i = 0; i < hw->noutput; i++) {
        DrOutput *o = &hw->outputs[i];
        XRROutputInfo *oi = XRRGetOutputInfo(x->dpy, x->res,
                                             x->res->outputs[i]);

        o->xid = x->res->outputs[i];
        if (!oi) {
            o->name = xasprintf("0x%lx", o->xid);
            continue;
        }
        o->name = xmalloc(oi->nameLen + 1);
        memcpy(o->name, oi->name, oi->nameLen);
        o->name[oi->nameLen] = '\0';
        o->connected = oi->connection == RR_Connected;
        o->mm_width = oi->mm_width;
        o->mm_height = oi->mm_height;
        o->modes = xcalloc(oi->nmode, sizeof(int));
        for (j = 0; j < oi->nmode; j++) {
            int m = mode_index(x, oi->modes[j]);

            if (m >= 0) {
                if (j < oi->npreferred)
                    o->npreferred++;
                o->modes[o->nmode++] = m;
            }
        }
        o->crtcs = xcalloc(oi->ncrtc, sizeof(int));
        for (j = 0; j < oi->ncrtc; j++) {
            int c = crtc_index(x, oi->crtcs[j]);

            if (c >= 0)
                o->crtcs[o->ncrtc++] = c;
        }
        o->clones = xcalloc(oi->nclone, sizeof(int));
        for (j = 0; j < oi->nclone; j++) {
            int c = output_index(x, oi->clones[j]);

            if (c >= 0)
                o->clones[o->nclone++] = c;
        }
        if (o->connected)
            o->edid = read_edid(x, o->xid);
        XRRFreeOutputInfo(oi);
    }

    hw->ncrtc = x->res->ncrtc;
    hw->crtcs = xcalloc(hw->ncrtc, sizeof *hw->crtcs);
    config_init(&hw->cur, hw->ncrtc);
    hw->cur.width = (int) gw;
    hw->cur.height = (int) gh;
    hw->cur.mm_width = DisplayWidthMM(x->dpy, x->screen);
    hw->cur.mm_height = DisplayHeightMM(x->dpy, x->screen);
    for (i = 0; i < hw->ncrtc; i++) {
        RRCrtc id = x->res->crtcs[i];
        XRRCrtcInfo *ci = XRRGetCrtcInfo(x->dpy, x->res, id);
        DrCrtcState *s = &hw->cur.crtc[i];
        int errors;

        hw->crtcs[i].xid = id;
        if (!ci)
            continue;
        hw->crtcs[i].rotations = ci->rotations;
        if (ci->mode != None) {
            s->mode = mode_index(x, ci->mode);
            s->x = ci->x;
            s->y = ci->y;
            s->rotation = ci->rotation;
            for (j = 0; j < ci->noutput; j++) {
                int o = output_index(x, ci->outputs[j]);

                if (o >= 0)
                    s->outputs |= BIT(o);
            }
        }
        XRRFreeCrtcInfo(ci);
        if (x->major > 1 || x->minor >= 3) {
            XRRCrtcTransformAttributes *ta = NULL;
            XRRPanning *pan;

            if (XRRGetCrtcTransform(x->dpy, id, &ta) && ta) {
                for (j = 0; j < 9; j++)
                    s->transform[j] = XFixedToDouble(
                        ta->currentTransform.matrix[j / 3][j % 3]);
                snprintf(s->filter, sizeof s->filter, "%s",
                         ta->currentFilter ? ta->currentFilter : "");
                XFree(ta);
            }
            if ((pan = XRRGetPanning(x->dpy, x->res, id))) {
                s->panning[0] = pan->left;
                s->panning[1] = pan->top;
                s->panning[2] = pan->width;
                s->panning[3] = pan->height;
                s->panning[4] = pan->track_left;
                s->panning[5] = pan->track_top;
                s->panning[6] = pan->track_width;
                s->panning[7] = pan->track_height;
                s->panning[8] = pan->border_left;
                s->panning[9] = pan->border_top;
                s->panning[10] = pan->border_right;
                s->panning[11] = pan->border_bottom;
                XRRFreePanning(pan);
            }
        }
        XSync(x->dpy, False);
        errors = x_errors;
        hw->crtcs[i].gamma_size = XRRGetCrtcGammaSize(x->dpy, id);
        XSync(x->dpy, False);
        if (x_errors != errors)
            hw->crtcs[i].gamma_size = 0;
        else
            read_gamma(x, id, hw->crtcs[i].gamma_size, s);
    }
    if (x->major > 1 || x->minor >= 3) {
        primary = XRRGetOutputPrimary(x->dpy, x->root);
        if (primary != None)
            hw->cur.primary = output_index(x, primary);
    }
    XSync(x->dpy, False);
    return 0;
}

/* ------------------------------------------------------------------ */
/* applying                                                             */

static int
op_begin(void *ctx)
{
    XB *x = ctx;

    XGrabServer(x->dpy);
    return 0;
}

static int
op_end(void *ctx)
{
    XB *x = ctx;

    XUngrabServer(x->dpy);
    XSync(x->dpy, False);
    return 0;
}

static int
op_set_size(void *ctx, int w, int h, int mmw, int mmh)
{
    XB *x = ctx;
    int errors = x_errors;

    dr_debug("XRRSetScreenSize %dx%d %dx%dmm", w, h, mmw, mmh);
    XRRSetScreenSize(x->dpy, x->root, w, h, mmw, mmh);
    return x_check(x, errors, "setting the screen size");
}

static int
op_set_crtc(void *ctx, int c, const DrCrtcState *s, const DrCrtcState *cur)
{
    XB *x = ctx;
    RRCrtc crtc = x->res->crtcs[c];
    RROutput outs[DR_MAX_OUTPUTS];
    int n = 0, o, i, errors = x_errors;
    Status st;

    if (s->mode < 0) {
        dr_debug("disabling crtc %d", c);
        st = XRRSetCrtcConfig(x->dpy, x->res, crtc, CurrentTime, 0, 0, None,
                              RR_Rotate_0, NULL, 0);
        if (x_check(x, errors, "disabling a CRTC"))
            return -1;
        if (st != RRSetConfigSuccess)
            return dr_error("disabling crtc %d failed (status %d)", c, st);
        return 0;
    }
    for (i = 0; i < 9; i++)
        if (fabs(s->transform[i] - cur->transform[i]) > 1e-4)
            break;
    /* (a server that cannot transform refuses even the identity) */
    if (i < 9 || (strcmp(s->filter, cur->filter)
                  && !transform_is_identity(s->transform))) {
        XTransform t;
        char filter[sizeof s->filter];

        if (x->major == 1 && x->minor < 3)
            return dr_error("the X server cannot transform outputs "
                            "(RandR 1.3 needed)");
        for (i = 0; i < 9; i++)
            t.matrix[i / 3][i % 3] = XDoubleToFixed(s->transform[i]);
        strcpy(filter, s->filter);
        dr_debug("crtc %d transform, filter \"%s\"", c, filter);
        XRRSetCrtcTransform(x->dpy, crtc, &t, filter, NULL, 0);
        if (x_check(x, errors, "setting a transform"))
            return -1;
    }
    for (o = 0; o < x->res->noutput && o < DR_MAX_OUTPUTS; o++)
        if (s->outputs & BIT(o))
            outs[n++] = x->res->outputs[o];
    dr_debug("crtc %d: mode 0x%lx at %d,%d rotation %u, %d outputs", c,
             (unsigned long) x->res->modes[s->mode].id, s->x, s->y,
             s->rotation, n);
    st = XRRSetCrtcConfig(x->dpy, x->res, crtc, CurrentTime, s->x, s->y,
                          x->res->modes[s->mode].id, (Rotation) s->rotation,
                          outs, n);
    if (x_check(x, errors, "setting a CRTC"))
        return -1;
    if (st != RRSetConfigSuccess)
        return dr_error("setting crtc %d failed (status %d)", c, st);
    if (memcmp(s->panning, cur->panning, sizeof s->panning)
        && (x->major > 1 || x->minor >= 3)) {
        XRRPanning p;

        memset(&p, 0, sizeof p);
        p.timestamp = CurrentTime;
        p.left = s->panning[0];
        p.top = s->panning[1];
        p.width = s->panning[2];
        p.height = s->panning[3];
        p.track_left = s->panning[4];
        p.track_top = s->panning[5];
        p.track_width = s->panning[6];
        p.track_height = s->panning[7];
        p.border_left = s->panning[8];
        p.border_top = s->panning[9];
        p.border_right = s->panning[10];
        p.border_bottom = s->panning[11];
        st = XRRSetPanning(x->dpy, x->res, crtc, &p);
        if (x_check(x, errors, "setting the panning"))
            return -1;
        if (st != RRSetConfigSuccess)
            return dr_error("setting the panning of crtc %d failed", c);
    }
    return 0;
}

static int
op_set_gamma(void *ctx, int c, const double gamma[3])
{
    XB *x = ctx;
    RRCrtc crtc = x->res->crtcs[c];
    int size = XRRGetCrtcGammaSize(x->dpy, crtc), i, j;
    int errors = x_errors;
    XRRCrtcGamma *g;

    if (size < 2) {
        dr_info("crtc %d has no gamma ramp, not setting the gamma", c);
        return 0;
    }
    if (!(g = XRRAllocGamma(size)))
        return dr_error("out of memory");
    for (i = 0; i < size; i++) {
        double v = (double) i / (size - 1);
        unsigned short *ch[3] = { g->red, g->green, g->blue };

        for (j = 0; j < 3; j++) {
            double e = 1.0 / gamma[j], r = e == 1.0 ? v : pow(v, e);

            if (r > 1.0)
                r = 1.0;
            ch[j][i] = (unsigned short) (r * 65535.0);
        }
    }
    XRRSetCrtcGamma(x->dpy, crtc, g);
    XRRFreeGamma(g);
    return x_check(x, errors, "setting the gamma");
}

static int
op_set_primary(void *ctx, int o)
{
    XB *x = ctx;
    int errors = x_errors;

    if (x->major == 1 && x->minor < 3)
        return 0;
    XRRSetOutputPrimary(x->dpy, x->root,
                        o >= 0 ? x->res->outputs[o] : None);
    return x_check(x, errors, "setting the primary output");
}

static const DrApplyOps x_ops = {
    op_begin, op_end, op_set_size, op_set_crtc, op_set_gamma, op_set_primary
};

static int
x_apply(DrBackend *b, const DrHardware *hw, const DrConfig *target)
{
    XB *x = b->priv;

    if (!x->res || x->res->ncrtc != hw->ncrtc
        || x->res->noutput != hw->noutput)
        return dr_error("the screen resources changed");
    return apply_config(hw, target, &x_ops, x);
}

static void
x_close(DrBackend *b)
{
    XB *x = b->priv;

    if (x->res)
        XRRFreeScreenResources(x->res);
    XCloseDisplay(x->dpy);
    free(x);
    free(b);
}

DrBackend *
x_backend_open(const char *display)
{
    DrBackend *b;
    XB *x;
    Display *dpy;
    int i;

    if (!(dpy = XOpenDisplay(display))) {
        dr_error("cannot open display \"%s\"", XDisplayName(display));
        return NULL;
    }
    XSetErrorHandler(x_error_handler);
    XSetIOErrorHandler(x_io_error_handler);
    fcntl(ConnectionNumber(dpy), F_SETFD, FD_CLOEXEC);
    x = xcalloc(1, sizeof *x);
    x->dpy = dpy;
    x->screen = DefaultScreen(dpy);
    x->root = RootWindow(dpy, x->screen);
    if (!XRRQueryExtension(dpy, &x->rr_event, &x->rr_error)
        || !XRRQueryVersion(dpy, &x->major, &x->minor)
        || (x->major == 1 && x->minor < 2)) {
        dr_error("the X server has no RandR 1.2 extension");
        XCloseDisplay(dpy);
        free(x);
        return NULL;
    }
    for (i = 0; i < 3; i++)
        x->edid_atoms[i] = XInternAtom(dpy, edid_names[i], True);
    b = xcalloc(1, sizeof *b);
    b->priv = x;
    b->query = x_query;
    b->apply = x_apply;
    b->close = x_close;
    return b;
}

/* ------------------------------------------------------------------ */
/* the daemon                                                           */

/* Own the selection _DTRANDR_DAEMON_S<screen>: one daemon per screen */
int
x_backend_lock(DrBackend *b)
{
    XB *x = b->priv;
    char name[64];

    snprintf(name, sizeof name, "_DTRANDR_DAEMON_S%d", x->screen);
    x->lock_atom = XInternAtom(x->dpy, name, False);
    if (XGetSelectionOwner(x->dpy, x->lock_atom) != None)
        return 1;
    x->lock_win = XCreateSimpleWindow(x->dpy, x->root, -1, -1, 1, 1, 0, 0,
                                      0);
    XSelectInput(x->dpy, x->lock_win, PropertyChangeMask);
    XSetSelectionOwner(x->dpy, x->lock_atom, x->lock_win, CurrentTime);
    if (XGetSelectionOwner(x->dpy, x->lock_atom) != x->lock_win)
        return 1;
    return 0;
}

int
x_backend_watch(DrBackend *b)
{
    XB *x = b->priv;

    x_daemon = 1;
    XRRSelectInput(x->dpy, x->root, RRScreenChangeNotifyMask
                   | RRCrtcChangeNotifyMask | RROutputChangeNotifyMask
                   | RROutputPropertyNotifyMask);
    XFlush(x->dpy);
    return 0;
}

int
x_backend_fd(DrBackend *b)
{
    XB *x = b->priv;

    return ConnectionNumber(x->dpy);
}

/*
 * Read the events there are.  Returns how many could mean a monitor came
 * or went (output changes, and changes of an EDID property), or -2 if
 * another daemon took the lock over.
 */
int
x_backend_events(DrBackend *b)
{
    XB *x = b->priv;
    int n = 0, i;
    XEvent ev;

    while (XPending(x->dpy)) {
        XNextEvent(x->dpy, &ev);
        if (ev.type == SelectionClear
            && ev.xselectionclear.window == x->lock_win) {
            dr_info("another dtrandr daemon took over");
            return -2;
        }
        if (ev.type == x->rr_event + RRScreenChangeNotify) {
            XRRUpdateConfiguration(&ev);
            dr_debug("event: screen change");
        } else if (ev.type == x->rr_event + RRNotify) {
            XRRNotifyEvent *ne = (XRRNotifyEvent *) &ev;

            XRRUpdateConfiguration(&ev);
            switch (ne->subtype) {
            case RRNotify_OutputChange:
                dr_debug("event: output change");
                n++;
                break;
            case RRNotify_OutputProperty: {
                XRROutputPropertyNotifyEvent *pe =
                    (XRROutputPropertyNotifyEvent *) &ev;
                char *nm = XGetAtomName(x->dpy, pe->property);

                for (i = 0; nm && i < 3; i++)
                    if (!strcmp(nm, edid_names[i])) {
                        dr_debug("event: %s change", nm);
                        n++;
                        break;
                    }
                if (nm)
                    XFree(nm);
                break;
            }
            default:
                dr_debug("event: crtc change");
                break;
            }
        }
    }
    return n;
}
