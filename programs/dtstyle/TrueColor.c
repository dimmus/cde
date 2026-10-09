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
 * StyleAllocColor: XAllocColor without a server round trip on TrueColor.
 *
 * A TrueColor colormap is static, so the pixel XAllocColor() returns for
 * an RGB value is a pure function of the visual.  The computation below
 * replicates the sample server (Xorg, Xvfb, Xwayland, Xephyr):
 * miResolveColor() truncates each component to bits_per_rgb bits, then
 * AllocColor() picks the nearest entry (lowest index on a tie) of that
 * channel's map, whose entries miInitializeColormap() sets to
 * ((((i * 65535) / lim) >> shift) * 65535) / rgbLim.  This is the method
 * of _ilXComputeColor() in lib/DtHelp/il/ilX.c.
 *
 * It is only used after a few real XAllocColor() calls returned exactly
 * the computed pixels and RGB values, so a server that resolves colours
 * differently keeps getting XAllocColor().
 */

#include <stdlib.h>
#include <X11/Xlib.h>
#include "TrueColor.h"

typedef struct {
    Display        *display;
    Colormap        colormap;
    Visual         *visual;
    Bool            usable;
    unsigned long   alpha;          /* bits the server ORs into pixels */
    int             offset[3];      /* R, G, B */
    unsigned long   lim[3];         /* mask >> offset */
    unsigned short *entries[3];     /* the channel maps, lim + 1 each */
    int             rgbShift;       /* 16 - bits_per_rgb */
    unsigned long   rgbLim;         /* (1 << bits_per_rgb) - 1 */
} TCInfo;

static TCInfo tc;                   /* dtstyle uses one colormap */

/* Index of the entry of 'e' (n entries, ascending) nearest to 'value';
 * the lowest such index on a tie. */
static unsigned long
nearest(const unsigned short *e, unsigned long n, unsigned long value)
{
    unsigned long lo = 0, hi = n, mid, best;

    while (lo < hi) {               /* first entry >= value */
        mid = (lo + hi) / 2;
        if (e[mid] < value)
            lo = mid + 1;
        else
            hi = mid;
    }
    if (lo == n)
        best = n - 1;
    else if (lo == 0)
        return 0;
    else if ((value - e[lo - 1]) <= (e[lo] - value))
        best = lo - 1;
    else
        return lo;                  /* lo is the first of its value */
    while (best > 0 && e[best - 1] == e[best])
        best--;
    return best;
}

static void
resolve(XColor *c)
{
    unsigned short *v[3];
    unsigned long index, value;
    int i;

    v[0] = &c->red;
    v[1] = &c->green;
    v[2] = &c->blue;
    c->pixel = tc.alpha;
    for (i = 0; i < 3; i++) {
        value = (((unsigned long)*v[i] >> tc.rgbShift) * 65535UL) / tc.rgbLim;
        index = nearest(tc.entries[i], tc.lim[i] + 1, value);
        c->pixel |= index << tc.offset[i];
        *v[i] = tc.entries[i][index];
    }
}

static int
visualDepth(Display *dpy, Visual *visual)
{
    int i, j, k;

    for (i = 0; i < ScreenCount(dpy); i++) {
        Screen *scr = ScreenOfDisplay(dpy, i);

        for (j = 0; j < scr->ndepths; j++)
            for (k = 0; k < scr->depths[j].nvisuals; k++)
                if (&scr->depths[j].visuals[k] == visual)
                    return scr->depths[j].depth;
    }
    return 0;
}

static void
setup(Display *dpy, Colormap cmap, Visual *visual)
{
    static const unsigned short samples[][3] = {
        { 0, 0, 0 }, { 65535, 65535, 65535 }, { 0x8000, 0x4000, 0xc000 },
        { 0x1234, 0xabcd, 0x7fff }, { 0x0101, 0xfeff, 0x8080 },
        { 0xffff, 0x0000, 0x00ff }, { 0x3333, 0x9999, 0xcccc },
        { 0x0420, 0x0841, 0xc400 }, { 0x7bef, 0x8410, 0x3def } };
    enum { NSAMPLES = sizeof(samples) / sizeof(samples[0]) };
    unsigned long masks[3], allocated[NSAMPLES];
    int i, j, nAllocated, depth;
    XColor real, computed;

    for (i = 0; i < 3; i++) {
        free(tc.entries[i]);
        tc.entries[i] = NULL;
    }
    tc.display = dpy;
    tc.colormap = cmap;
    tc.visual = visual;
    tc.usable = False;

    if (visual->class != TrueColor ||
        visual->bits_per_rgb < 1 || visual->bits_per_rgb > 16)
        return;
    if ((depth = visualDepth(dpy, visual)) == 0)
        return;

    masks[0] = visual->red_mask;
    masks[1] = visual->green_mask;
    masks[2] = visual->blue_mask;
    tc.rgbShift = 16 - visual->bits_per_rgb;
    tc.rgbLim = (1UL << visual->bits_per_rgb) - 1;
    for (i = 0; i < 3; i++) {
        if (!masks[i])
            return;
        tc.offset[i] = 0;
        while (!((masks[i] >> tc.offset[i]) & 1))
            tc.offset[i]++;
        tc.lim[i] = masks[i] >> tc.offset[i];
        /* need a contiguous mask of at most 16 bits */
        if ((tc.lim[i] & (tc.lim[i] + 1)) || tc.lim[i] > 65535 || tc.lim[i] < 1)
            return;
        tc.entries[i] = malloc((tc.lim[i] + 1) * sizeof(unsigned short));
        if (!tc.entries[i])
            return;
        for (j = 0; j <= (int)tc.lim[i]; j++)
            tc.entries[i][j] = (unsigned short)
                (((((unsigned long)j * 65535UL) / tc.lim[i]) >> tc.rgbShift)
                 * 65535UL / tc.rgbLim);
    }
    tc.alpha = (depth < 32) ? 0
             : (~(masks[0] | masks[1] | masks[2]) & 0xffffffffUL);

    /* check against the server */
    tc.usable = True;
    nAllocated = 0;
    for (i = 0; i < NSAMPLES; i++) {
        real.red = computed.red = samples[i][0];
        real.green = computed.green = samples[i][1];
        real.blue = computed.blue = samples[i][2];
        real.flags = computed.flags = DoRed | DoGreen | DoBlue;
        if (!XAllocColor(dpy, cmap, &real)) {
            tc.usable = False;
            break;
        }
        allocated[nAllocated++] = real.pixel;
        resolve(&computed);
        if (real.pixel != computed.pixel || real.red != computed.red ||
            real.green != computed.green || real.blue != computed.blue) {
            tc.usable = False;
            break;
        }
    }
    if (nAllocated)
        XFreeColors(dpy, cmap, allocated, nAllocated, 0);
}

int
StyleAllocColor(
        Display *dpy,
        Colormap cmap,
        Visual *visual,
        XColor *color )
{
    if (visual && visual->class == TrueColor) {
        if (tc.display != dpy || tc.colormap != cmap || tc.visual != visual)
            setup(dpy, cmap, visual);
        if (tc.usable) {
            resolve(color);
            return STYLE_COLOR_COMPUTED;
        }
    }
    return XAllocColor(dpy, cmap, color) ? STYLE_COLOR_ALLOCATED : 0;
}
