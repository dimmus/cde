/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: fingerprints, matching, and turning a profile into a
 * configuration (modes, CRTCs, screen size)
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
 * A profile (or a virtual profile, or the "auto" fallback) becomes a
 * DrPlan: for each output, on or off, and the mode, position, rotation,
 * transform... it should have.  plan_to_config() then gives the enabled
 * outputs CRTCs and computes the screen size, which makes the DrConfig
 * that a backend applies.  All of it follows what xrandr does with the
 * equivalent command line, so that a profile means the same here as it
 * does to autorandr.
 */

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "dtrandr.h"

#define BIT(o) ((DrOutputMask) 1 << (o))

/* ------------------------------------------------------------------ */
/* hardware and configurations                                          */

void
hw_free(DrHardware *hw)
{
    int i;

    for (i = 0; i < hw->nmode; i++)
        free(hw->modes[i].name);
    free(hw->modes);
    for (i = 0; i < hw->noutput; i++) {
        free(hw->outputs[i].name);
        free(hw->outputs[i].edid);
        free(hw->outputs[i].modes);
        free(hw->outputs[i].crtcs);
        free(hw->outputs[i].clones);
    }
    free(hw->outputs);
    free(hw->crtcs);
    config_free(&hw->cur);
    memset(hw, 0, sizeof *hw);
}

void
transform_identity(double t[9])
{
    int i;

    for (i = 0; i < 9; i++)
        t[i] = (i % 4 == 0) ? 1.0 : 0.0;
}

int
transform_is_identity(const double t[9])
{
    int i;

    for (i = 0; i < 9; i++)
        if (fabs(t[i] - ((i % 4 == 0) ? 1.0 : 0.0)) > 1e-4)
            return 0;
    return 1;
}

static int
transform_equal(const double a[9], const double b[9])
{
    int i;

    for (i = 0; i < 9; i++)
        if (fabs(a[i] - b[i]) > 1e-4)
            return 0;
    return 1;
}

void
crtc_state_off(DrCrtcState *s)
{
    memset(s, 0, sizeof *s);
    s->mode = -1;
    s->rotation = DR_ROTATE_0;
    transform_identity(s->transform);
}

void
config_init(DrConfig *c, int ncrtc)
{
    int i;

    memset(c, 0, sizeof *c);
    c->primary = -1;
    c->ncrtc = ncrtc;
    c->crtc = xcalloc(ncrtc, sizeof *c->crtc);
    for (i = 0; i < ncrtc; i++)
        crtc_state_off(&c->crtc[i]);
}

void
config_copy(DrConfig *dst, const DrConfig *src)
{
    *dst = *src;
    dst->crtc = xcalloc(src->ncrtc, sizeof *dst->crtc);
    if (src->ncrtc)
        memcpy(dst->crtc, src->crtc, src->ncrtc * sizeof *dst->crtc);
}

void
config_free(DrConfig *c)
{
    free(c->crtc);
    c->crtc = NULL;
    c->ncrtc = 0;
}

/* The CRTC that drives output in c, -1 if none */
int
output_crtc(const DrConfig *c, int output)
{
    int i;

    for (i = 0; i < c->ncrtc; i++)
        if (c->crtc[i].mode >= 0 && (c->crtc[i].outputs & BIT(output)))
            return i;
    return -1;
}

int
find_output(const DrHardware *hw, const char *name)
{
    int i;

    for (i = 0; i < hw->noutput; i++)
        if (!strcmp(hw->outputs[i].name, name))
            return i;
    return -1;
}

/* The refresh rate of a mode, as xrandr computes it */
double
mode_refresh(const DrMode *m)
{
    double vtotal = m->vtotal;

    if (m->flags & DR_DOUBLESCAN)
        vtotal *= 2;
    if (m->flags & DR_INTERLACE)
        vtotal /= 2;
    if (m->htotal && vtotal)
        return (double) m->dotclock / ((double) m->htotal * vtotal);
    return 0;
}

static const struct {
    const char *name;
    unsigned bit;
} rotations[] = {
    { "normal", DR_ROTATE_0 }, { "left", DR_ROTATE_90 },
    { "inverted", DR_ROTATE_180 }, { "right", DR_ROTATE_270 },
};

const char *
rotation_name(unsigned rotation)
{
    size_t i;

    for (i = 0; i < sizeof rotations / sizeof rotations[0]; i++)
        if ((rotation & DR_ROTATE_MASK) == rotations[i].bit)
            return rotations[i].name;
    return "normal";
}

const char *
reflection_name(unsigned rotation)
{
    switch (rotation & (DR_REFLECT_X | DR_REFLECT_Y)) {
    case DR_REFLECT_X:
        return "x";
    case DR_REFLECT_Y:
        return "y";
    case DR_REFLECT_X | DR_REFLECT_Y:
        return "xy";
    }
    return "normal";
}

/*
 * The area of the screen a CRTC shows: its mode, rotated, through its
 * transform (xrandr's mode_geometry), and its panning area.
 */
int
crtc_bounds(const DrHardware *hw, const DrCrtcState *s,
            int *x1, int *y1, int *x2, int *y2)
{
    const DrMode *m;
    double w, h, minx = 0, miny = 0, maxx = 0, maxy = 0;
    double pts[4][2];
    int i;

    if (s->mode < 0 || s->mode >= hw->nmode)
        return -1;
    m = &hw->modes[s->mode];
    if (s->rotation & (DR_ROTATE_90 | DR_ROTATE_270)) {
        w = m->height;
        h = m->width;
    } else {
        w = m->width;
        h = m->height;
    }
    pts[0][0] = 0; pts[0][1] = 0;
    pts[1][0] = w; pts[1][1] = 0;
    pts[2][0] = w; pts[2][1] = h;
    pts[3][0] = 0; pts[3][1] = h;
    for (i = 0; i < 4; i++) {
        const double *t = s->transform;
        double x = pts[i][0], y = pts[i][1];
        double tx = t[0] * x + t[1] * y + t[2];
        double ty = t[3] * x + t[4] * y + t[5];
        double tw = t[6] * x + t[7] * y + t[8];

        if (fabs(tw) < 1e-9) {
            tx = x;
            ty = y;
        } else {
            tx /= tw;
            ty /= tw;
        }
        if (i == 0 || tx < minx)
            minx = tx;
        if (i == 0 || tx > maxx)
            maxx = tx;
        if (i == 0 || ty < miny)
            miny = ty;
        if (i == 0 || ty > maxy)
            maxy = ty;
    }
    *x1 = s->x + (int) floor(minx);
    *y1 = s->y + (int) floor(miny);
    *x2 = s->x + (int) ceil(maxx);
    *y2 = s->y + (int) ceil(maxy);
    if (s->panning[2] > 0 && s->panning[3] > 0) {
        if (s->panning[0] < *x1)
            *x1 = s->panning[0];
        if (s->panning[1] < *y1)
            *y1 = s->panning[1];
        if (s->panning[0] + s->panning[2] > *x2)
            *x2 = s->panning[0] + s->panning[2];
        if (s->panning[1] + s->panning[3] > *y2)
            *y2 = s->panning[1] + s->panning[3];
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* fingerprints and matching                                            */

/* What the setup file says about an output; NULL if not connected */
char *
output_fingerprint(const DrHardware *hw, int o)
{
    const DrOutput *out = &hw->outputs[o];

    if (!out->connected)
        return NULL;
    if (out->edid && *out->edid)
        return xstrdup(out->edid);
    return xasprintf("%s-%s", DR_EDID_UNAVAILABLE, out->name);
}

static int
cmp_output_name(const void *a, const void *b, const DrHardware *hw)
{
    return strcmp(hw->outputs[*(const int *) a].name,
                  hw->outputs[*(const int *) b].name);
}

/* Sort output indices by name (a small insertion sort: no qsort_r) */
static void
sort_outputs(const DrHardware *hw, int *v, int n)
{
    int i, j;

    for (i = 1; i < n; i++) {
        int t = v[i];

        for (j = i; j > 0 && cmp_output_name(&v[j - 1], &t, hw) > 0; j--)
            v[j] = v[j - 1];
        v[j] = t;
    }
}

/* The connected outputs, sorted by name; returns how many */
static int
connected_outputs(const DrHardware *hw, int *v)
{
    int i, n = 0;

    for (i = 0; i < hw->noutput; i++)
        if (hw->outputs[i].connected)
            v[n++] = i;
    sort_outputs(hw, v, n);
    return n;
}

void
print_fingerprint(const DrHardware *hw, FILE *f)
{
    int *v = xcalloc(hw->noutput, sizeof *v);
    int i, n = connected_outputs(hw, v);

    for (i = 0; i < n; i++) {
        char *fp = output_fingerprint(hw, v[i]);

        fprintf(f, "%s %s\n", hw->outputs[v[i]].name, fp);
        free(fp);
    }
    free(v);
}

static int
glob(const char *p, const char *s)
{
    for (; *p; p++, s++) {
        if (*p == '*') {
            while (p[1] == '*')
                p++;
            if (!p[1])
                return 1;
            for (; *s; s++)
                if (glob(p + 1, s))
                    return 1;
            return 0;
        }
        if (!*s)
            return 0;
        if (*p != '?' && tolower((unsigned char) *p)
            != tolower((unsigned char) *s))
            return 0;
    }
    return !*s;
}

/*
 * Match a setup file EDID (exact, "*", or a glob with * and ?) against
 * an output's fingerprint: 2 for an exact match, 1 for a wildcard match
 * (closeness: the share of the EDID the pattern spells out, as
 * autorandr's match_asterisk), 0 for none.
 */
int
edid_glob_match(const char *pattern, const char *edid, double *closeness)
{
    size_t lit = 0;
    const char *p;

    *closeness = 0;
    if (!strpbrk(pattern, "*?")) {
        if (strcasecmp(pattern, edid))
            return 0;
        *closeness = 1;
        return 2;
    }
    if (!glob(pattern, edid))
        return 0;
    for (p = pattern; *p; p++)
        if (*p != '*' && *p != '?')
            lit++;
    if (*edid)
        *closeness = (double) lit / (double) strlen(edid);
    return 1;
}

/*
 * Does the profile's setup match the connected outputs?  The outputs it
 * names must be exactly the connected ones, each with a matching EDID.
 */
int
profile_match(const DrHardware *hw, const DrProfile *p,
              int *exact, double *closeness)
{
    int i, j;

    *exact = 0;
    *closeness = 0;
    if (!p->nsetup)
        return 0;
    for (i = 0; i < p->nsetup; i++) {
        int o = find_output(hw, p->setup[i].output);
        char *fp;
        double c;
        int m;

        if (o < 0 || !(fp = output_fingerprint(hw, o)))
            return 0;
        m = edid_glob_match(p->setup[i].edid, fp, &c);
        free(fp);
        if (!m)
            return 0;
        if (m == 2)
            (*exact)++;
        *closeness += c;
    }
    for (i = 0; i < hw->noutput; i++) {
        if (!hw->outputs[i].connected)
            continue;
        for (j = 0; j < p->nsetup; j++)
            if (!strcmp(p->setup[j].output, hw->outputs[i].name))
                break;
        if (j == p->nsetup)
            return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* plans                                                                */

void
plan_init(DrPlan *plan, const DrHardware *hw)
{
    int i;

    plan->noutput = hw->noutput;
    plan->want = xcalloc(hw->noutput, sizeof *plan->want);
    for (i = 0; i < hw->noutput; i++) {
        DrWant *w = &plan->want[i];

        w->mode = -1;
        w->rotation = DR_ROTATE_0;
        w->crtc = -1;
        transform_identity(w->transform);
    }
    plan->primary = DR_PRIMARY_KEEP;
    plan->dpi = 0;
}

void
plan_free(DrPlan *plan)
{
    free(plan->want);
    plan->want = NULL;
}

/*
 * The mode of output o called name (or else of size name, "WxH"),
 * closest to rate when has_rate, as xrandr picks it.
 */
static int
find_mode(const DrHardware *hw, int o, const char *name, double rate,
          int has_rate)
{
    const DrOutput *out = &hw->outputs[o];
    unsigned w, h;
    int pass, i, best = -1, n = 0;
    double bestdist = 0;

    for (pass = 0; pass < 2 && best < 0; pass++) {
        if (pass == 1 && sscanf(name, "%ux%u%n", &w, &h, &n) < 2)
            break;
        for (i = 0; i < out->nmode; i++) {
            const DrMode *m = &hw->modes[out->modes[i]];
            double dist;

            if (pass == 0 ? strcmp(m->name, name) != 0
                : (m->width != w || m->height != h))
                continue;
            /* stay away from doublescan modes unless a rate is given */
            if (!has_rate && (m->flags & DR_DOUBLESCAN))
                continue;
            dist = has_rate ? fabs(mode_refresh(m) - rate) : 0;
            if (best < 0 || dist < bestdist) {
                best = out->modes[i];
                bestdist = dist;
            }
        }
    }
    return best;
}

/* The mode to give an output that a profile does not choose one for:
   its preferred mode (the largest one if it has several or none) */
static int
preferred_mode(const DrHardware *hw, int o)
{
    const DrOutput *out = &hw->outputs[o];
    int i, best = -1, n;

    n = out->npreferred > 0 ? out->npreferred : out->nmode;
    for (i = 0; i < n; i++) {
        const DrMode *m = &hw->modes[out->modes[i]];
        const DrMode *b = best >= 0 ? &hw->modes[best] : NULL;

        if (m->flags & DR_DOUBLESCAN)
            continue;
        if (!b || (unsigned long) m->width * m->height
            > (unsigned long) b->width * b->height
            || ((unsigned long) m->width * m->height
                == (unsigned long) b->width * b->height
                && mode_refresh(m) > mode_refresh(b) + 0.005))
            best = out->modes[i];
    }
    return best;
}

static int
parse_panning(const char *s, int p[12])
{
    int v[12] = { 0 }, n;

    n = sscanf(s, "%dx%d+%d+%d/%dx%d+%d+%d/%d/%d/%d/%d",
               &v[2], &v[3], &v[0], &v[1], &v[6], &v[7], &v[4], &v[5],
               &v[8], &v[9], &v[10], &v[11]);
    if (n != 2 && n != 4 && n != 8 && n != 12)
        return -1;
    if (n < 4)
        v[0] = v[1] = 0;
    if (n < 8)
        v[4] = v[5] = v[6] = v[7] = 0;
    if (n < 12)
        v[8] = v[9] = v[10] = v[11] = 0;
    memcpy(p, v, sizeof v);
    return 0;
}

static int
parse_doubles(const char *s, char sep, double *v, int n)
{
    char *end;
    int i;

    for (i = 0; i < n; i++) {
        v[i] = strtod(s, &end);
        if (end == s)
            return -1;
        s = end;
        if (i < n - 1) {
            if (*s != sep)
                return -1;
            s++;
        }
    }
    return *s ? -1 : 0;
}

static const char *const known_keys[] = {
    "off", "mode", "pos", "rate", "primary", "rotate", "reflect",
    "transform", "scale", "filter", "gamma", "panning", "crtc", "dpi",
    NULL
};

static int
known_key(const char *key)
{
    int i;

    for (i = 0; known_keys[i]; i++)
        if (!strcmp(key, known_keys[i]))
            return 1;
    return 0;
}

static int
want_from_opts(const DrHardware *hw, const DrProfile *p, int o,
               const DrOutputConf *oc, DrWant *w, int *primary)
{
    const char *name = oc->name;
    const DrOpt *opt;
    double rate = 0;
    int has_rate = 0, i;

    for (i = 0; i < oc->nopt; i++)
        if (!known_key(oc->opts[i].key))
            dr_info("profile %s, output %s: ignoring \"%s\"", p->name,
                    name, oc->opts[i].key);
    if (!hw->outputs[o].connected)
        return dr_error("profile %s: output %s is not connected",
                        p->name, name);
    if (!(opt = opt_find(oc->nopt, oc->opts, "mode")) || !opt->val)
        return dr_error("profile %s: output %s has no mode", p->name, name);
    if ((opt = opt_find(oc->nopt, oc->opts, "rate")) && opt->val) {
        char *end;

        rate = strtod(opt->val, &end);
        if (end == opt->val || *end)
            return dr_error("profile %s: output %s: bad rate \"%s\"",
                            p->name, name, opt->val);
        has_rate = 1;
    }
    opt = opt_find(oc->nopt, oc->opts, "mode");
    w->mode = find_mode(hw, o, opt->val, rate, has_rate);
    if (w->mode < 0)
        return dr_error("profile %s: output %s has no mode %s", p->name,
                        name, opt->val);
    w->on = 1;
    if ((opt = opt_find(oc->nopt, oc->opts, "pos"))) {
        char c;

        if (!opt->val
            || sscanf(opt->val, "%dx%d%c", &w->x, &w->y, &c) != 2)
            return dr_error("profile %s: output %s: bad pos \"%s\"",
                            p->name, name, opt->val ? opt->val : "");
    }
    if (opt_find(oc->nopt, oc->opts, "primary"))
        *primary = o;
    if ((opt = opt_find(oc->nopt, oc->opts, "rotate"))) {
        size_t r;

        for (r = 0; r < sizeof rotations / sizeof rotations[0]; r++)
            if (opt->val && !strcmp(opt->val, rotations[r].name))
                break;
        if (r == sizeof rotations / sizeof rotations[0])
            return dr_error("profile %s: output %s: bad rotation \"%s\"",
                            p->name, name, opt->val ? opt->val : "");
        w->rotation = rotations[r].bit;
    }
    if ((opt = opt_find(oc->nopt, oc->opts, "reflect"))) {
        const char *v = opt->val ? opt->val : "";

        if (!strcmp(v, "x"))
            w->rotation |= DR_REFLECT_X;
        else if (!strcmp(v, "y"))
            w->rotation |= DR_REFLECT_Y;
        else if (!strcmp(v, "xy"))
            w->rotation |= DR_REFLECT_X | DR_REFLECT_Y;
        else if (strcmp(v, "normal"))
            return dr_error("profile %s: output %s: bad reflection \"%s\"",
                            p->name, name, v);
    }
    /* scale is a transform; a transform line wins over it (autorandr
       passes the options sorted, and xrandr keeps the last one) */
    if ((opt = opt_find(oc->nopt, oc->opts, "scale"))) {
        double s[2];

        if (!opt->val || parse_doubles(opt->val, 'x', s, 2)
            || s[0] <= 0 || s[1] <= 0)
            return dr_error("profile %s: output %s: bad scale \"%s\"",
                            p->name, name, opt->val ? opt->val : "");
        transform_identity(w->transform);
        w->transform[0] = s[0];
        w->transform[4] = s[1];
    }
    if ((opt = opt_find(oc->nopt, oc->opts, "transform"))) {
        if (!opt->val || (strcmp(opt->val, "none")
                          && parse_doubles(opt->val, ',', w->transform, 9)))
            return dr_error("profile %s: output %s: bad transform \"%s\"",
                            p->name, name, opt->val ? opt->val : "");
        if (!strcmp(opt->val, "none"))
            transform_identity(w->transform);
    }
    if (!transform_is_identity(w->transform))
        strcpy(w->filter, "bilinear");
    if ((opt = opt_find(oc->nopt, oc->opts, "filter")) && opt->val) {
        if (strlen(opt->val) >= sizeof w->filter)
            return dr_error("profile %s: output %s: bad filter", p->name,
                            name);
        strcpy(w->filter, opt->val);
    }
    if ((opt = opt_find(oc->nopt, oc->opts, "gamma"))) {
        if (!opt->val || parse_doubles(opt->val, ':', w->gamma, 3)
            || w->gamma[0] <= 0 || w->gamma[1] <= 0 || w->gamma[2] <= 0)
            return dr_error("profile %s: output %s: bad gamma \"%s\"",
                            p->name, name, opt->val ? opt->val : "");
        w->has_gamma = 1;
    }
    if ((opt = opt_find(oc->nopt, oc->opts, "panning"))) {
        if (!opt->val || parse_panning(opt->val, w->panning))
            return dr_error("profile %s: output %s: bad panning \"%s\"",
                            p->name, name, opt->val ? opt->val : "");
        w->has_panning = 1;
    }
    if ((opt = opt_find(oc->nopt, oc->opts, "crtc")) && opt->val) {
        char *end;
        long c = strtol(opt->val, &end, 10);
        const DrOutput *out = &hw->outputs[o];

        for (i = 0; i < out->ncrtc; i++)
            if (out->crtcs[i] == c)
                break;
        if (*end || end == opt->val || i == out->ncrtc)
            dr_info("profile %s: output %s cannot use crtc %s, "
                    "choosing another", p->name, name, opt->val);
        else
            w->crtc = (int) c;
    }
    return 0;
}

int
plan_from_profile(const DrHardware *hw, const DrProfile *p, DrPlan *plan)
{
    const DrOpt *opt;
    int i, primary = -1;

    plan_init(plan, hw);
    opt = opt_find(p->nhead, p->head, "dpi");
    for (i = 0; i < p->nout && !opt; i++)
        opt = opt_find(p->out[i].nopt, p->out[i].opts, "dpi");
    if (opt) {
        char *end;

        plan->dpi = opt->val ? strtod(opt->val, &end) : 0;
        if (!opt->val || *end || plan->dpi <= 0) {
            dr_error("profile %s: bad dpi", p->name);
            goto fail;
        }
    }
    for (i = 0; i < p->nhead; i++)
        if (strcmp(p->head[i].key, "dpi"))
            dr_info("profile %s: ignoring \"%s\"", p->name, p->head[i].key);
    for (i = 0; i < p->nout; i++) {
        const DrOutputConf *oc = &p->out[i];
        int o = find_output(hw, oc->name);

        if (opt_find(oc->nopt, oc->opts, "off")) {
            if (o >= 0)
                plan->want[o].on = 0;
            continue;
        }
        if (o < 0) {
            dr_error("profile %s: there is no output %s", p->name,
                     oc->name);
            goto fail;
        }
        if (want_from_opts(hw, p, o, oc, &plan->want[o], &primary))
            goto fail;
    }
    if (primary >= 0)
        plan->primary = primary;
    return 0;
  fail:
    plan_free(plan);
    return -1;
}

/* Turn on output o at its preferred mode at x, y */
static int
want_preferred(const DrHardware *hw, DrPlan *plan, int o, int x, int y)
{
    DrWant *w = &plan->want[o];

    w->mode = preferred_mode(hw, o);
    if (w->mode < 0)
        return dr_error("output %s has no modes", hw->outputs[o].name);
    w->on = 1;
    w->x = x;
    w->y = y;
    return 0;
}

int
plan_virtual(const DrHardware *hw, const char *name, DrPlan *plan)
{
    int *v = xcalloc(hw->noutput, sizeof *v);
    int n = connected_outputs(hw, v), i, j, r = -1;

    plan_init(plan, hw);
    if (!strcmp(name, "off")) {
        r = 0;
        goto out;
    }
    if (!n) {
        dr_error("no output is connected");
        goto out;
    }
    plan->primary = v[0];
    if (!strncmp(name, "horizontal", 10) || !strncmp(name, "vertical", 8)) {
        int horizontal = name[0] == 'h', shift = 0;
        int reverse = strstr(name, "-reverse") != NULL;

        for (i = 0; i < n; i++) {
            int o = v[reverse ? n - 1 - i : i];
            const DrMode *m;

            if (want_preferred(hw, plan, o, horizontal ? shift : 0,
                               horizontal ? 0 : shift))
                goto out;
            m = &hw->modes[plan->want[o].mode];
            shift += horizontal ? m->width : m->height;
        }
        if (reverse)
            plan->primary = v[n - 1];
    } else if (!strcmp(name, "common")) {
        /* the largest size that every connected output has a mode of */
        const DrOutput *first = &hw->outputs[v[0]];
        unsigned long bestarea = 0;
        unsigned bw = 0, bh = 0;

        for (i = 0; i < first->nmode; i++) {
            const DrMode *m = &hw->modes[first->modes[i]];
            unsigned long area = (unsigned long) m->width * m->height;

            for (j = 1; j < n; j++) {
                char size[32];

                snprintf(size, sizeof size, "%ux%u", m->width, m->height);
                if (find_mode(hw, v[j], size, 0, 0) < 0)
                    break;
            }
            if (j == n && area > bestarea) {
                bestarea = area;
                bw = m->width;
                bh = m->height;
            }
        }
        if (!bestarea) {
            dr_error("the connected outputs have no mode size in common");
            goto out;
        }
        for (j = 0; j < n; j++) {
            const DrOutput *out = &hw->outputs[v[j]];
            int m = -1;

            /* a preferred mode of that size, else the first */
            for (i = 0; i < out->nmode && m < 0; i++) {
                const DrMode *mm = &hw->modes[out->modes[i]];

                if (i < out->npreferred && mm->width == bw
                    && mm->height == bh)
                    m = out->modes[i];
            }
            if (m < 0) {
                char size[32];

                snprintf(size, sizeof size, "%ux%u", bw, bh);
                m = find_mode(hw, v[j], size, 0, 0);
            }
            plan->want[v[j]].on = 1;
            plan->want[v[j]].mode = m;
        }
    } else if (!strcmp(name, "clone-largest")) {
        /* the largest first mode of the connected outputs; every output
           shows it, scaled from its own preferred mode */
        unsigned bw = 0, bh = 0;

        for (j = 0; j < n; j++) {
            const DrOutput *out = &hw->outputs[v[j]];
            const DrMode *m;

            if (!out->nmode)
                continue;
            m = &hw->modes[out->modes[0]];
            if ((unsigned long) m->width * m->height
                > (unsigned long) bw * bh) {
                bw = m->width;
                bh = m->height;
            }
        }
        for (j = 0; j < n; j++) {
            DrWant *w = &plan->want[v[j]];
            const DrMode *m;
            double scale, movx, movy;

            if (want_preferred(hw, plan, v[j], 0, 0))
                goto out;
            m = &hw->modes[w->mode];
            scale = (double) bw / m->width;
            if ((double) bh / m->height > scale)
                scale = (double) bh / m->height;
            movx = (m->width * scale - bw) / -2;
            movy = (m->height * scale - bh) / -2;
            if (movx == 0)
                movx = 0;               /* not -0 */
            if (movy == 0)
                movy = 0;
            transform_identity(w->transform);
            w->transform[0] = w->transform[4] = scale;
            w->transform[2] = movx;
            w->transform[5] = movy;
            if (!transform_is_identity(w->transform))
                strcpy(w->filter, "bilinear");
        }
    } else {
        dr_error("unknown virtual profile %s", name);
        goto out;
    }
    r = 0;
  out:
    free(v);
    if (r)
        plan_free(plan);
    return r;
}

/*
 * The built-in fallback: change as little as possible.  Connected
 * outputs that are on stay as they are, outputs that are on but no
 * longer connected go off, and connected outputs that are off come on at
 * their preferred mode, right of the others.
 */
int
plan_auto(const DrHardware *hw, DrPlan *plan)
{
    int *v = xcalloc(hw->noutput, sizeof *v);
    int n = connected_outputs(hw, v), i, kept = 0, right = 0, top = 0;
    int r = -1;

    if (!n) {
        free(v);
        return plan_virtual(hw, "horizontal", plan);
    }
    plan_init(plan, hw);
    for (i = 0; i < n; i++) {
        int o = v[i], c = output_crtc(&hw->cur, o), x1, y1, x2, y2;
        const DrCrtcState *s;
        DrWant *w = &plan->want[o];

        if (c < 0)
            continue;
        s = &hw->cur.crtc[c];
        w->on = 1;
        w->mode = s->mode;
        w->x = s->x;
        w->y = s->y;
        w->rotation = s->rotation;
        memcpy(w->transform, s->transform, sizeof w->transform);
        strcpy(w->filter, s->filter);
        w->crtc = c;
        if (!crtc_bounds(hw, s, &x1, &y1, &x2, &y2)) {
            if (!kept || x2 > right)
                right = x2;
            if (!kept || y1 < top)
                top = y1;
        }
        kept++;
    }
    free(v);
    if (!kept) {
        plan_free(plan);
        return plan_virtual(hw, "horizontal", plan);
    }
    v = xcalloc(hw->noutput, sizeof *v);
    n = connected_outputs(hw, v);
    for (i = 0; i < n; i++) {
        int o = v[i];

        if (plan->want[o].on)
            continue;
        if (want_preferred(hw, plan, o, right, top))
            goto out;
        right += hw->modes[plan->want[o].mode].width;
    }
    if (hw->cur.primary >= 0 && plan->want[hw->cur.primary].on)
        plan->primary = hw->cur.primary;
    else
        for (i = 0; i < n; i++)
            if (plan->want[v[i]].on) {
                plan->primary = v[i];
                break;
            }
    r = 0;
  out:
    free(v);
    if (r)
        plan_free(plan);
    return r;
}

/* ------------------------------------------------------------------ */
/* CRTCs and the screen size                                            */

typedef struct {
    const DrHardware *hw;
    const DrPlan *plan;
    int *order;                         /* the enabled outputs */
    int n;
    int *crtc_of;                       /* per output */
    DrOutputMask *used;                 /* per CRTC */
} Assign;

static int
is_clone(const DrOutput *out, int other)
{
    int i;

    for (i = 0; i < out->nclone; i++)
        if (out->clones[i] == other)
            return 1;
    return 0;
}

static int
crtc_ok(const Assign *a, int c, int o)
{
    const DrHardware *hw = a->hw;
    const DrWant *w = &a->plan->want[o];
    unsigned rot = w->rotation;
    int p;

    if (rot != DR_ROTATE_0 && (hw->crtcs[c].rotations & rot) != rot)
        return 0;
    for (p = 0; p < hw->noutput; p++) {
        const DrWant *pw = &a->plan->want[p];

        if (!(a->used[c] & BIT(p)))
            continue;
        /* sharing a CRTC: the outputs must be clones of each other and
           want the same picture */
        if (!is_clone(&hw->outputs[o], p) || !is_clone(&hw->outputs[p], o))
            return 0;
        if (pw->mode != w->mode || pw->x != w->x || pw->y != w->y
            || pw->rotation != w->rotation
            || !transform_equal(pw->transform, w->transform)
            || strcmp(pw->filter, w->filter))
            return 0;
    }
    return 1;
}

static int
requested_later(const Assign *a, int k, int c)
{
    int i;

    for (i = k + 1; i < a->n; i++)
        if (a->plan->want[a->order[i]].crtc == c)
            return 1;
    return 0;
}

/* Give the outputs order[k..] CRTCs; depth-first, best choices first */
static int
assign(Assign *a, int k)
{
    const DrOutput *out;
    int o, i, j, ncand = 0, *cand, *key;

    if (k == a->n)
        return 1;
    o = a->order[k];
    out = &a->hw->outputs[o];
    cand = xcalloc(out->ncrtc, sizeof *cand);
    key = xcalloc(out->ncrtc, sizeof *key);
    for (i = 0; i < out->ncrtc; i++) {
        int c = out->crtcs[i], kk;

        if (c < 0 || c >= a->hw->ncrtc)
            continue;
        if (a->plan->want[o].crtc == c)
            kk = 0;                     /* the one the profile asks for */
        else if (output_crtc(&a->hw->cur, o) == c)
            kk = 1;                     /* the one it has now */
        else if (requested_later(a, k, c))
            kk = 3;                     /* one another output asks for */
        else
            kk = 2;
        if (a->used[c])
            kk += 10;                   /* a shared one, only if need be */
        for (j = ncand; j > 0 && key[j - 1] > kk; j--) {
            cand[j] = cand[j - 1];
            key[j] = key[j - 1];
        }
        cand[j] = c;
        key[j] = kk;
        ncand++;
    }
    for (i = 0; i < ncand; i++) {
        int c = cand[i];

        if (!crtc_ok(a, c, o))
            continue;
        a->used[c] |= BIT(o);
        a->crtc_of[o] = c;
        if (assign(a, k + 1)) {
            free(cand);
            free(key);
            return 1;
        }
        a->used[c] &= ~BIT(o);
        a->crtc_of[o] = -1;
    }
    free(cand);
    free(key);
    return 0;
}

int
plan_to_config(const DrHardware *hw, const DrPlan *plan, DrConfig *target)
{
    Assign a;
    int i, o, any = 0, w = 0, h = 0, r = -1;
    double dpi;

    config_init(target, hw->ncrtc);
    if (hw->noutput > DR_MAX_OUTPUTS)
        return dr_error("too many outputs (%d)", hw->noutput);
    memset(&a, 0, sizeof a);
    a.hw = hw;
    a.plan = plan;
    a.order = xcalloc(hw->noutput, sizeof *a.order);
    a.crtc_of = xcalloc(hw->noutput, sizeof *a.crtc_of);
    a.used = xcalloc(hw->ncrtc, sizeof *a.used);
    for (o = 0; o < hw->noutput; o++) {
        a.crtc_of[o] = -1;
        if (plan->want[o].on)
            a.order[a.n++] = o;
    }
    if (!assign(&a, 0)) {
        DrBuf b = { 0 };
        char *s;

        for (i = 0; i < a.n; i++)
            buf_printf(&b, "%s%s", i ? ", " : "",
                       hw->outputs[a.order[i]].name);
        s = buf_take(&b);
        dr_error("not enough CRTCs to drive %s at once", s);
        free(s);
        goto out;
    }
    for (o = 0; o < hw->noutput; o++) {
        const DrWant *wo = &plan->want[o];
        DrCrtcState *s;

        if (a.crtc_of[o] < 0)
            continue;
        s = &target->crtc[a.crtc_of[o]];
        s->outputs |= BIT(o);
        if (s->mode >= 0)
            continue;                   /* a clone of an earlier output */
        s->mode = wo->mode;
        s->x = wo->x;
        s->y = wo->y;
        s->rotation = wo->rotation;
        memcpy(s->transform, wo->transform, sizeof s->transform);
        strcpy(s->filter, wo->filter);
        s->has_gamma = wo->has_gamma;
        memcpy(s->gamma, wo->gamma, sizeof s->gamma);
        if (wo->has_panning)
            memcpy(s->panning, wo->panning, sizeof s->panning);
        else {
            /* keep the panning the output has now */
            int c = output_crtc(&hw->cur, o);

            if (c >= 0)
                memcpy(s->panning, hw->cur.crtc[c].panning,
                       sizeof s->panning);
        }
    }
    for (i = 0; i < hw->ncrtc; i++) {
        int x1, y1, x2, y2;

        if (crtc_bounds(hw, &target->crtc[i], &x1, &y1, &x2, &y2))
            continue;
        if (!any || x2 > w)
            w = x2;
        if (!any || y2 > h)
            h = y2;
        any = 1;
    }
    if (!any) {
        w = hw->cur.width;
        h = hw->cur.height;
    }
    if (w < hw->min_width)
        w = hw->min_width;
    if (h < hw->min_height)
        h = hw->min_height;
    if ((hw->max_width && w > hw->max_width)
        || (hw->max_height && h > hw->max_height)) {
        dr_error("the screen would be %dx%d, larger than the maximum, "
                 "%dx%d", w, h, hw->max_width, hw->max_height);
        goto out;
    }
    target->width = w;
    target->height = h;
    if (plan->dpi <= 0 && w == hw->cur.width && h == hw->cur.height) {
        target->mm_width = hw->cur.mm_width;
        target->mm_height = hw->cur.mm_height;
    } else {
        if (plan->dpi > 0)
            dpi = plan->dpi;
        else if (hw->cur.mm_height > 0)
            dpi = 25.4 * hw->cur.height / hw->cur.mm_height;
        else
            dpi = 96;
        target->mm_width = (int) (25.4 * w / dpi);
        target->mm_height = (int) (25.4 * h / dpi);
    }
    if (plan->primary == DR_PRIMARY_KEEP)
        target->primary = hw->cur.primary;
    else
        target->primary = plan->primary;
    r = 0;
  out:
    free(a.order);
    free(a.crtc_of);
    free(a.used);
    return r;
}

/*
 * Do two configurations show the same thing: each output on or off in
 * both, with the same mode, position, rotation and transform, and the
 * same primary output?  (Which CRTC drives an output does not matter.)
 */
int
config_equal(const DrHardware *hw, const DrConfig *a, const DrConfig *b)
{
    int o;

    for (o = 0; o < hw->noutput; o++) {
        int ca = output_crtc(a, o), cb = output_crtc(b, o);
        const DrCrtcState *sa, *sb;

        if (ca < 0 && cb < 0)
            continue;
        if (ca < 0 || cb < 0)
            return 0;
        sa = &a->crtc[ca];
        sb = &b->crtc[cb];
        if (sa->mode != sb->mode || sa->x != sb->x || sa->y != sb->y
            || sa->rotation != sb->rotation
            || !transform_equal(sa->transform, sb->transform))
            return 0;
    }
    return a->primary == b->primary;
}

/* ------------------------------------------------------------------ */
/* the current configuration as a profile                              */

typedef struct {
    int output;
    long key;
} SortKey;

/*
 * A profile of configuration c, as autorandr saves one: a setup line for
 * each connected output, and an output block for every output,
 * disconnected ones first, then those that are off, then the others by
 * position.
 */
DrProfile *
profile_from_config(const DrHardware *hw, const DrConfig *c,
                    const char *name)
{
    DrProfile *p = profile_new(name);
    SortKey *v = xcalloc(hw->noutput, sizeof *v);
    int i, j;

    for (i = 0; i < hw->noutput; i++) {
        char *fp = output_fingerprint(hw, i);
        int cr = output_crtc(c, i);

        if (fp) {
            p->setup = xrealloc(p->setup, (p->nsetup + 1) * sizeof *p->setup);
            p->setup[p->nsetup].output = xstrdup(hw->outputs[i].name);
            p->setup[p->nsetup].edid = fp;
            p->nsetup++;
        }
        v[i].output = i;
        if (!fp)
            v[i].key = -2;
        else if (cr < 0)
            v[i].key = -1;
        else
            v[i].key = c->crtc[cr].x + 10000L * c->crtc[cr].y;
    }
    for (i = 1; i < hw->noutput; i++) {
        SortKey t = v[i];

        for (j = i; j > 0 && v[j - 1].key > t.key; j--)
            v[j] = v[j - 1];
        v[j] = t;
    }
    for (i = 0; i < hw->noutput; i++) {
        int o = v[i].output, cr = output_crtc(c, o);
        DrOutputConf *oc = profile_add_output(p, hw->outputs[o].name);
        const DrCrtcState *s;
        const DrMode *m;
        char *val;

        if (cr < 0) {
            opt_add(&oc->nopt, &oc->opts, "off", NULL);
            continue;
        }
        s = &c->crtc[cr];
        m = &hw->modes[s->mode];
        val = xasprintf("%d", cr);
        opt_add(&oc->nopt, &oc->opts, "crtc", val);
        free(val);
        if (!transform_is_identity(s->transform) && s->filter[0])
            opt_add(&oc->nopt, &oc->opts, "filter", s->filter);
        if (s->has_gamma) {
            DrBuf b = { 0 };

            for (j = 0; j < 3; j++) {
                char *g = dr_format_double(round(s->gamma[j] * 1000) / 1000);

                buf_printf(&b, "%s%s", j ? ":" : "", g);
                free(g);
            }
            val = buf_take(&b);
            opt_add(&oc->nopt, &oc->opts, "gamma", val);
            free(val);
        }
        opt_add(&oc->nopt, &oc->opts, "mode", m->name);
        if (s->panning[2] > 0 && s->panning[3] > 0) {
            const int *pn = s->panning;

            val = xasprintf("%dx%d+%d+%d/%dx%d+%d+%d/%d/%d/%d/%d",
                            pn[2], pn[3], pn[0], pn[1], pn[6], pn[7],
                            pn[4], pn[5], pn[8], pn[9], pn[10], pn[11]);
            opt_add(&oc->nopt, &oc->opts, "panning", val);
            free(val);
        }
        val = xasprintf("%dx%d", s->x, s->y);
        opt_add(&oc->nopt, &oc->opts, "pos", val);
        free(val);
        if (c->primary == o)
            opt_add(&oc->nopt, &oc->opts, "primary", NULL);
        val = xasprintf("%.2f", mode_refresh(m));
        opt_add(&oc->nopt, &oc->opts, "rate", val);
        free(val);
        if (s->rotation & (DR_REFLECT_X | DR_REFLECT_Y))
            opt_add(&oc->nopt, &oc->opts, "reflect",
                    reflection_name(s->rotation));
        opt_add(&oc->nopt, &oc->opts, "rotate", rotation_name(s->rotation));
        if (!transform_is_identity(s->transform)) {
            const double *t = s->transform;

            val = xasprintf("%f,%f,%f,%f,%f,%f,%f,%f,%f", t[0], t[1], t[2],
                            t[3], t[4], t[5], t[6], t[7], t[8]);
            opt_add(&oc->nopt, &oc->opts, "transform", val);
            free(val);
        }
    }
    free(v);
    return p;
}

/* ------------------------------------------------------------------ */
/* the equivalent xrandr command line (for --dry-run)                  */

char *
xrandr_command(const DrHardware *hw, const DrPlan *plan,
               const DrConfig *target)
{
    DrBuf b = { 0 };
    int *v = xcalloc(hw->noutput, sizeof *v);
    long *key = xcalloc(hw->noutput, sizeof *key);
    int i, j, n = 0;

    buf_add(&b, "xrandr");
    if (plan->dpi > 0)
        buf_printf(&b, " --dpi %g", plan->dpi);
    buf_printf(&b, " --fb %dx%d", target->width, target->height);
    /* outputs that go off first, then the others by position */
    for (i = 0; i < hw->noutput; i++) {
        int c = output_crtc(target, i);
        long k;

        if (c < 0 && output_crtc(&hw->cur, i) < 0 && !plan->want[i].on
            && !hw->outputs[i].connected)
            continue;                   /* off and staying off */
        k = c < 0 ? -1 : target->crtc[c].x + 10000L * target->crtc[c].y;
        for (j = n; j > 0 && key[j - 1] > k; j--) {
            v[j] = v[j - 1];
            key[j] = key[j - 1];
        }
        v[j] = i;
        key[j] = k;
        n++;
    }
    for (i = 0; i < n; i++) {
        int o = v[i], c = output_crtc(target, o);
        const DrCrtcState *s;
        const DrMode *m;

        buf_printf(&b, " --output %s", hw->outputs[o].name);
        if (c < 0) {
            buf_add(&b, " --off");
            continue;
        }
        s = &target->crtc[c];
        m = &hw->modes[s->mode];
        buf_printf(&b, " --mode %s", m->name);
        if (mode_refresh(m) > 0)        /* (Xvfb's modes have no timings) */
            buf_printf(&b, " --rate %.2f", mode_refresh(m));
        buf_printf(&b, " --pos %dx%d --rotate %s --reflect %s", s->x, s->y,
                   rotation_name(s->rotation), reflection_name(s->rotation));
        if (transform_is_identity(s->transform))
            buf_add(&b, " --transform none");
        else {
            const double *t = s->transform;

            buf_printf(&b, " --transform %g,%g,%g,%g,%g,%g,%g,%g,%g",
                       t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7], t[8]);
        }
        if (s->filter[0])
            buf_printf(&b, " --filter %s", s->filter);
        if (plan->want[o].has_gamma)
            buf_printf(&b, " --gamma %g:%g:%g", s->gamma[0], s->gamma[1],
                       s->gamma[2]);
        if (plan->want[o].has_panning) {
            const int *pn = s->panning;

            buf_printf(&b, " --panning %dx%d+%d+%d/%dx%d+%d+%d/%d/%d/%d/%d",
                       pn[2], pn[3], pn[0], pn[1], pn[6], pn[7], pn[4],
                       pn[5], pn[8], pn[9], pn[10], pn[11]);
        }
        buf_printf(&b, " --crtc %d", c);
        if (target->primary == o)
            buf_add(&b, " --primary");
    }
    if (target->primary < 0 && hw->cur.primary >= 0)
        buf_add(&b, " --noprimary");
    free(v);
    free(key);
    return buf_take(&b);
}
