/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: the order in which a configuration is applied
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
 * As xrandr does it (with the server grabbed, by the backend's begin):
 *
 *  1. disable the CRTCs that go off, that change outputs (an output can
 *     only be on one CRTC, so one that moves leaves its old CRTC first),
 *     and those that would not fit the new screen size;
 *  2. set the screen size;
 *  3. set the CRTCs that change (transform, mode, panning) and gamma;
 *  4. set the primary output.
 *
 * If a step fails, the previous configuration is put back as far as it
 * can be.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "backend.h"

static int
same_state(const DrCrtcState *a, const DrCrtcState *b)
{
    int i;

    if (a->mode < 0 && b->mode < 0)
        return 1;
    if (a->mode != b->mode || a->x != b->x || a->y != b->y
        || a->rotation != b->rotation || a->outputs != b->outputs
        || memcmp(a->panning, b->panning, sizeof a->panning))
        return 0;
    for (i = 0; i < 9; i++)
        if (fabs(a->transform[i] - b->transform[i]) > 1e-4)
            return 0;
    /* the filter only matters with a transform */
    return !strcmp(a->filter, b->filter)
        || transform_is_identity(a->transform);
}

static int
gamma_differs(const DrCrtcState *a, const double g[3])
{
    int i;

    if (!a->has_gamma)
        return 1;
    for (i = 0; i < 3; i++)
        if (fabs(a->gamma[i] - g[i]) > 0.01)
            return 1;
    return 0;
}

static int
fits(const DrHardware *hw, const DrCrtcState *s, int width, int height)
{
    int x1, y1, x2, y2;

    if (crtc_bounds(hw, s, &x1, &y1, &x2, &y2))
        return 1;
    return x1 >= 0 && y1 >= 0 && x2 <= width && y2 <= height;
}

/* Disable CRTC c, keeping what dtrandr knows of its state in live */
static int
disable(const DrApplyOps *ops, void *ctx, DrCrtcState *live, int c)
{
    DrCrtcState off = live[c];

    off.mode = -1;
    off.outputs = 0;
    if (ops->set_crtc(ctx, c, &off, &live[c]))
        return -1;
    live[c] = off;
    return 0;
}

static int
set(const DrApplyOps *ops, void *ctx, DrCrtcState *live, int c,
    const DrCrtcState *s)
{
    if (ops->set_crtc(ctx, c, s, &live[c]))
        return -1;
    live[c] = *s;
    return 0;
}

int
apply_config(const DrHardware *hw, const DrConfig *target,
             const DrApplyOps *ops, void *ctx)
{
    const DrConfig *cur = &hw->cur;
    DrCrtcState *live = xcalloc(hw->ncrtc, sizeof *live);
    int c, r = -1, size_changed = 0, primary;

    if (hw->ncrtc)
        memcpy(live, cur->crtc, hw->ncrtc * sizeof *live);
    if (ops->begin && ops->begin(ctx)) {
        free(live);
        return -1;
    }
    for (c = 0; c < hw->ncrtc; c++) {
        const DrCrtcState *t = &target->crtc[c];

        if (live[c].mode < 0)
            continue;
        if (t->mode < 0 || t->outputs != live[c].outputs
            || !fits(hw, &live[c], target->width, target->height)) {
            dr_debug("disabling crtc %d", c);
            if (disable(ops, ctx, live, c))
                goto fail;
        }
    }
    if (target->width != cur->width || target->height != cur->height
        || target->mm_width != cur->mm_width
        || target->mm_height != cur->mm_height) {
        dr_debug("screen size %dx%d (%dx%d mm)", target->width,
                 target->height, target->mm_width, target->mm_height);
        size_changed = 1;
        if (ops->set_size(ctx, target->width, target->height,
                          target->mm_width, target->mm_height))
            goto fail;
    }
    for (c = 0; c < hw->ncrtc; c++) {
        const DrCrtcState *t = &target->crtc[c];
        int changed = 0;

        if (t->mode < 0)
            continue;
        if (!same_state(&live[c], t)) {
            DrCrtcState s = *t;

            /* the gamma is set apart from the CRTC */
            s.has_gamma = live[c].has_gamma;
            memcpy(s.gamma, live[c].gamma, sizeof s.gamma);
            dr_debug("setting crtc %d", c);
            if (set(ops, ctx, live, c, &s))
                goto fail;
            changed = 1;
        }
        if (t->has_gamma && (changed || gamma_differs(&live[c], t->gamma))) {
            if (ops->set_gamma(ctx, c, t->gamma))
                goto fail;
            live[c].has_gamma = 1;
            memcpy(live[c].gamma, t->gamma, sizeof t->gamma);
        }
    }
    if (target->primary != cur->primary
        && ops->set_primary(ctx, target->primary))
        goto fail;
    r = 0;
    goto out;

  fail:
    /* put the previous configuration back */
    dr_warn("restoring the previous configuration");
    for (c = 0; c < hw->ncrtc; c++)
        if (live[c].mode >= 0 && !same_state(&live[c], &cur->crtc[c]))
            disable(ops, ctx, live, c);
    if (size_changed)
        ops->set_size(ctx, cur->width, cur->height, cur->mm_width,
                      cur->mm_height);
    for (c = 0; c < hw->ncrtc; c++)
        if (cur->crtc[c].mode >= 0 && !same_state(&live[c], &cur->crtc[c])
            && set(ops, ctx, live, c, &cur->crtc[c]))
            dr_error("could not restore crtc %d", c);
    primary = cur->primary;
    ops->set_primary(ctx, primary);
  out:
    if (ops->end && ops->end(ctx))
        r = -1;
    free(live);
    return r;
}
