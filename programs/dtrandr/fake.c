/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: a fake display, read from and written to a text file, for
 * the tests (DTRANDR_FAKE=FILE)
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
 * The file describes the hardware and its configuration:
 *
 *   screen 1920x1080 mm 508x285 min 320x200 max 8192x8192
 *   primary eDP-1
 *   mode 0x41 1920x1080 1920x1080@60.01         (timings made up)
 *   mode 0x42 1280x720 74.25 1280 1390 1430 1650 720 725 730 750 +hsync
 *   crtc 0
 *     rotations normal left inverted right x y  (default: all)
 *     gammasize 256                             (default: 256)
 *     active 0x41 0 0 normal normal eDP-1       (MODE X Y ROTATE REFLECT
 *     transform 1,0,0,0,1,0,0,0,1                OUTPUT...; none: off)
 *     filter bilinear
 *     gamma 1.0:1.0:1.0
 *     panning 0x0+0+0/0x0+0+0/0/0/0/0
 *   output eDP-1
 *     connected                                 (or disconnected)
 *     edid 00ffffffffffff00...
 *     mm 344x194
 *     modes 0x41* 0x42                          (* = preferred)
 *     crtcs 0 1
 *     clones HDMI-1
 *   fail-crtc 1                                 (setting CRTC 1 fails)
 *
 * The applied configuration is written back to the file (or to
 * $DTRANDR_FAKE_OUT), in the same form, and each step of applying it is
 * appended to $DTRANDR_FAKE_LOG.  Applying checks what an X server
 * would check, so that a wrong order of steps fails here too.
 */

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "backend.h"

#define BIT(o) ((DrOutputMask) 1 << (o))

typedef struct {
    char *path;
    DrHardware hw;
    DrConfig state;                     /* while applying */
    char **clone_names;                 /* per output, until resolved */
    int *fail_crtc;
    int nfail;
    FILE *log;
} Fake;

static void
fake_log(Fake *f, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static void
fake_log(Fake *f, const char *fmt, ...)
{
    va_list ap;

    if (!f->log)
        return;
    va_start(ap, fmt);
    vfprintf(f->log, fmt, ap);
    va_end(ap);
    fputc('\n', f->log);
    fflush(f->log);
}

static int
find_mode_id(const DrHardware *hw, unsigned long id)
{
    int i;

    for (i = 0; i < hw->nmode; i++)
        if (hw->modes[i].xid == id)
            return i;
    return -1;
}

static int
parse_rotation(const char *rot, const char *refl, unsigned *r)
{
    static const char *const names[] = { "normal", "left", "inverted",
        "right" };
    int i;

    for (i = 0; i < 4; i++)
        if (!strcmp(rot, names[i]))
            break;
    if (i == 4)
        return -1;
    *r = 1u << i;
    if (!strcmp(refl, "x"))
        *r |= DR_REFLECT_X;
    else if (!strcmp(refl, "y"))
        *r |= DR_REFLECT_Y;
    else if (!strcmp(refl, "xy"))
        *r |= DR_REFLECT_X | DR_REFLECT_Y;
    else if (strcmp(refl, "normal"))
        return -1;
    return 0;
}

static int
mode_flags(char **tok, int n, unsigned long *flags)
{
    int i;

    for (i = 0; i < n; i++) {
        if (!strcasecmp(tok[i], "+hsync"))
            *flags |= DR_HSYNC_POS;
        else if (!strcasecmp(tok[i], "-hsync"))
            *flags |= DR_HSYNC_NEG;
        else if (!strcasecmp(tok[i], "+vsync"))
            *flags |= DR_VSYNC_POS;
        else if (!strcasecmp(tok[i], "-vsync"))
            *flags |= DR_VSYNC_NEG;
        else if (!strcasecmp(tok[i], "interlace"))
            *flags |= DR_INTERLACE;
        else if (!strcasecmp(tok[i], "doublescan"))
            *flags |= DR_DOUBLESCAN;
        else
            return -1;
    }
    return 0;
}

static int
grow_crtcs(DrHardware *hw, int n)
{
    int i;

    if (n <= hw->ncrtc)
        return 0;
    hw->crtcs = xrealloc(hw->crtcs, n * sizeof *hw->crtcs);
    hw->cur.crtc = xrealloc(hw->cur.crtc, n * sizeof *hw->cur.crtc);
    for (i = hw->ncrtc; i < n; i++) {
        hw->crtcs[i].xid = 0x100 + i;
        hw->crtcs[i].rotations = DR_ROTATE_MASK | DR_REFLECT_X | DR_REFLECT_Y;
        hw->crtcs[i].gamma_size = 256;
        crtc_state_off(&hw->cur.crtc[i]);
        hw->cur.crtc[i].has_gamma = 1;
        hw->cur.crtc[i].gamma[0] = hw->cur.crtc[i].gamma[1] =
            hw->cur.crtc[i].gamma[2] = 1.0;
    }
    hw->ncrtc = hw->cur.ncrtc = n;
    return 0;
}

#define MAXTOK 64

/* Split line into white-space separated words; returns how many */
static int
tokenize(char *line, char **tok)
{
    int n = 0;
    char *s = line;

    for (;;) {
        while (isspace((unsigned char) *s))
            s++;
        if (!*s || *s == '#' || n == MAXTOK)
            break;
        tok[n++] = s;
        while (*s && !isspace((unsigned char) *s))
            s++;
        if (*s)
            *s++ = '\0';
    }
    return n;
}

typedef struct {
    int crtc;
    char *outputs;                      /* names, until resolved */
} PendingCrtc;

static int
fake_read(Fake *f)
{
    DrHardware *hw = &f->hw;
    FILE *fp = fopen(f->path, "r");
    char *line = NULL, *tok[MAXTOK], *primary = NULL;
    size_t size = 0;
    int lineno = 0, n, i, j, r = -1;
    int cur_output = -1, cur_crtc = -1;
    PendingCrtc *pend = NULL;
    int npend = 0;

    if (!fp)
        return dr_error("cannot read %s: %s", f->path, strerror(errno));
    free(f->fail_crtc);
    f->fail_crtc = NULL;
    f->nfail = 0;
    memset(hw, 0, sizeof *hw);
    hw->cur.primary = -1;
    hw->min_width = hw->min_height = 1;
    hw->max_width = hw->max_height = 32767;
    hw->cur.width = 1024;
    hw->cur.height = 768;
    hw->cur.mm_width = 271;
    hw->cur.mm_height = 203;
    while (getline(&line, &size, fp) >= 0) {
        lineno++;
        if (!(n = tokenize(line, tok)))
            continue;
#define BAD() do { dr_error("%s:%d: bad line", f->path, lineno); \
                   goto out; } while (0)
        if (!strcmp(tok[0], "screen")) {
            if (n != 8 || sscanf(tok[1], "%dx%d", &hw->cur.width,
                                 &hw->cur.height) != 2
                || strcmp(tok[2], "mm") || strcmp(tok[4], "min")
                || strcmp(tok[6], "max")
                || sscanf(tok[3], "%dx%d", &hw->cur.mm_width,
                          &hw->cur.mm_height) != 2
                || sscanf(tok[5], "%dx%d", &hw->min_width,
                          &hw->min_height) != 2
                || sscanf(tok[7], "%dx%d", &hw->max_width,
                          &hw->max_height) != 2)
                BAD();
        } else if (!strcmp(tok[0], "primary") && n == 2) {
            free(primary);
            primary = xstrdup(tok[1]);
        } else if (!strcmp(tok[0], "fail-crtc") && n == 2) {
            f->fail_crtc = xrealloc(f->fail_crtc,
                                    (f->nfail + 1) * sizeof *f->fail_crtc);
            f->fail_crtc[f->nfail++] = atoi(tok[1]);
        } else if (!strcmp(tok[0], "mode")) {
            DrMode m;
            double rate, clock;

            memset(&m, 0, sizeof m);
            if (n < 4)
                BAD();
            m.xid = strtoul(tok[1], NULL, 0);
            if (n >= 4 && sscanf(tok[3], "%ux%u@%lf", &m.width, &m.height,
                                 &rate) == 3) {
                m.hsync_start = m.width + 48;
                m.hsync_end = m.width + 80;
                m.htotal = m.width + 160;
                m.vsync_start = m.height + 3;
                m.vsync_end = m.height + 8;
                m.vtotal = m.height + 45;
                m.dotclock = (unsigned long) llround(rate * m.htotal
                                                     * m.vtotal);
                if (mode_flags(tok + 4, n - 4, &m.flags))
                    BAD();
                if (m.flags & DR_INTERLACE)
                    m.dotclock /= 2;
                if (m.flags & DR_DOUBLESCAN)
                    m.dotclock *= 2;
            } else {
                if (n < 12 || sscanf(tok[3], "%lf", &clock) != 1
                    || sscanf(tok[4], "%u", &m.width) != 1
                    || sscanf(tok[5], "%u", &m.hsync_start) != 1
                    || sscanf(tok[6], "%u", &m.hsync_end) != 1
                    || sscanf(tok[7], "%u", &m.htotal) != 1
                    || sscanf(tok[8], "%u", &m.height) != 1
                    || sscanf(tok[9], "%u", &m.vsync_start) != 1
                    || sscanf(tok[10], "%u", &m.vsync_end) != 1
                    || sscanf(tok[11], "%u", &m.vtotal) != 1
                    || mode_flags(tok + 12, n - 12, &m.flags))
                    BAD();
                m.dotclock = (unsigned long) llround(clock * 1e6);
            }
            m.name = xstrdup(tok[2]);
            hw->modes = xrealloc(hw->modes, (hw->nmode + 1)
                                 * sizeof *hw->modes);
            hw->modes[hw->nmode++] = m;
        } else if (!strcmp(tok[0], "output") && n == 2) {
            DrOutput *o;

            hw->outputs = xrealloc(hw->outputs, (hw->noutput + 1)
                                   * sizeof *hw->outputs);
            f->clone_names = xrealloc(f->clone_names, (hw->noutput + 1)
                                      * sizeof *f->clone_names);
            o = &hw->outputs[hw->noutput];
            memset(o, 0, sizeof *o);
            o->name = xstrdup(tok[1]);
            o->xid = 0x200 + hw->noutput;
            f->clone_names[hw->noutput] = NULL;
            cur_output = hw->noutput++;
            cur_crtc = -1;
        } else if (!strcmp(tok[0], "crtc") && n == 2) {
            cur_crtc = atoi(tok[1]);
            if (cur_crtc < 0 || cur_crtc > 255)
                BAD();
            grow_crtcs(hw, cur_crtc + 1);
            cur_output = -1;
        } else if (cur_output >= 0) {
            DrOutput *o = &hw->outputs[cur_output];

            if (!strcmp(tok[0], "connected") && n == 1)
                o->connected = 1;
            else if (!strcmp(tok[0], "disconnected") && n == 1)
                o->connected = 0;
            else if (!strcmp(tok[0], "edid") && n == 2) {
                free(o->edid);
                o->edid = xstrdup(tok[1]);
                for (i = 0; o->edid[i]; i++)
                    o->edid[i] = (char) tolower((unsigned char) o->edid[i]);
            } else if (!strcmp(tok[0], "mm") && n == 2) {
                if (sscanf(tok[1], "%lux%lu", &o->mm_width,
                           &o->mm_height) != 2)
                    BAD();
            } else if (!strcmp(tok[0], "modes")) {
                int pass;

                free(o->modes);
                o->modes = xcalloc(n, sizeof *o->modes);
                o->nmode = o->npreferred = 0;
                /* the preferred modes come first */
                for (pass = 0; pass < 2; pass++)
                    for (i = 1; i < n; i++) {
                        int pref = strchr(tok[i], '*') != NULL;
                        int m = find_mode_id(hw, strtoul(tok[i], NULL, 0));

                        if (m < 0)
                            BAD();
                        if (pref != (pass == 0))
                            continue;
                        o->modes[o->nmode++] = m;
                        if (pref)
                            o->npreferred++;
                    }
            } else if (!strcmp(tok[0], "crtcs")) {
                free(o->crtcs);
                o->crtcs = xcalloc(n, sizeof *o->crtcs);
                for (i = 1; i < n; i++)
                    o->crtcs[i - 1] = atoi(tok[i]);
                o->ncrtc = n - 1;
            } else if (!strcmp(tok[0], "clones")) {
                DrBuf b = { 0 };

                for (i = 1; i < n; i++)
                    buf_printf(&b, "%s ", tok[i]);
                free(f->clone_names[cur_output]);
                f->clone_names[cur_output] = buf_take(&b);
            } else
                BAD();
        } else if (cur_crtc >= 0) {
            DrCrtcState *s = &hw->cur.crtc[cur_crtc];

            if (!strcmp(tok[0], "rotations")) {
                unsigned rot;

                hw->crtcs[cur_crtc].rotations = 0;
                for (i = 1; i < n; i++) {
                    if (!strcmp(tok[i], "x") || !strcmp(tok[i], "y"))
                        parse_rotation("normal", tok[i], &rot);
                    else if (parse_rotation(tok[i], "normal", &rot))
                        BAD();
                    hw->crtcs[cur_crtc].rotations |= rot;
                }
            } else if (!strcmp(tok[0], "gammasize") && n == 2) {
                hw->crtcs[cur_crtc].gamma_size = atoi(tok[1]);
                s->has_gamma = hw->crtcs[cur_crtc].gamma_size > 0;
            } else if (!strcmp(tok[0], "active") && n >= 6) {
                DrBuf b = { 0 };

                s->mode = find_mode_id(hw, strtoul(tok[1], NULL, 0));
                s->x = atoi(tok[2]);
                s->y = atoi(tok[3]);
                if (s->mode < 0 || parse_rotation(tok[4], tok[5],
                                                  &s->rotation))
                    BAD();
                for (i = 6; i < n; i++)
                    buf_printf(&b, "%s ", tok[i]);
                pend = xrealloc(pend, (npend + 1) * sizeof *pend);
                pend[npend].crtc = cur_crtc;
                pend[npend++].outputs = buf_take(&b);
            } else if (!strcmp(tok[0], "transform") && n == 2) {
                char *e = tok[1];

                for (i = 0; i < 9; i++) {
                    s->transform[i] = strtod(e, &e);
                    if (i < 8 && *e++ != ',')
                        BAD();
                }
            } else if (!strcmp(tok[0], "filter") && n <= 2) {
                snprintf(s->filter, sizeof s->filter, "%s",
                         n == 2 ? tok[1] : "");
            } else if (!strcmp(tok[0], "gamma") && n == 2) {
                if (sscanf(tok[1], "%lf:%lf:%lf", &s->gamma[0], &s->gamma[1],
                           &s->gamma[2]) != 3)
                    BAD();
            } else if (!strcmp(tok[0], "panning") && n == 2) {
                int *p = s->panning;

                if (sscanf(tok[1], "%dx%d+%d+%d/%dx%d+%d+%d/%d/%d/%d/%d",
                           &p[2], &p[3], &p[0], &p[1], &p[6], &p[7], &p[4],
                           &p[5], &p[8], &p[9], &p[10], &p[11]) != 12)
                    BAD();
            } else
                BAD();
        } else
            BAD();
#undef BAD
    }
    /* resolve the output names */
    for (i = 0; i < npend; i++) {
        char *t[MAXTOK];
        int k, nt = tokenize(pend[i].outputs, t);

        for (k = 0; k < nt; k++) {
            int o = find_output(hw, t[k]);

            if (o < 0 || o >= DR_MAX_OUTPUTS) {
                dr_error("%s: crtc %d: no output %s", f->path, pend[i].crtc,
                         t[k]);
                goto out;
            }
            hw->cur.crtc[pend[i].crtc].outputs |= BIT(o);
        }
    }
    for (i = 0; i < hw->noutput; i++) {
        char *t[MAXTOK];
        int nt;

        if (!f->clone_names[i])
            continue;
        nt = tokenize(f->clone_names[i], t);
        hw->outputs[i].clones = xcalloc(nt, sizeof(int));
        for (j = 0; j < nt; j++) {
            int o = find_output(hw, t[j]);

            if (o < 0) {
                dr_error("%s: output %s: no clone %s", f->path,
                         hw->outputs[i].name, t[j]);
                goto out;
            }
            hw->outputs[i].clones[hw->outputs[i].nclone++] = o;
        }
    }
    if (primary && (hw->cur.primary = find_output(hw, primary)) < 0) {
        dr_error("%s: no primary output %s", f->path, primary);
        goto out;
    }
    r = 0;
  out:
    for (i = 0; i < npend; i++)
        free(pend[i].outputs);
    free(pend);
    if (f->clone_names)
        for (i = 0; i < hw->noutput; i++)
            free(f->clone_names[i]);
    free(f->clone_names);
    f->clone_names = NULL;
    free(primary);
    free(line);
    fclose(fp);
    return r;
}

static void
write_flags(FILE *fp, unsigned long flags)
{
    if (flags & DR_HSYNC_POS)
        fputs(" +hsync", fp);
    if (flags & DR_HSYNC_NEG)
        fputs(" -hsync", fp);
    if (flags & DR_VSYNC_POS)
        fputs(" +vsync", fp);
    if (flags & DR_VSYNC_NEG)
        fputs(" -vsync", fp);
    if (flags & DR_INTERLACE)
        fputs(" interlace", fp);
    if (flags & DR_DOUBLESCAN)
        fputs(" doublescan", fp);
}

static void
write_hw(FILE *fp, const DrHardware *hw, const DrConfig *c, const Fake *f)
{
    int i, j;

    fprintf(fp, "screen %dx%d mm %dx%d min %dx%d max %dx%d\n", c->width,
            c->height, c->mm_width, c->mm_height, hw->min_width,
            hw->min_height, hw->max_width, hw->max_height);
    if (c->primary >= 0)
        fprintf(fp, "primary %s\n", hw->outputs[c->primary].name);
    for (i = 0; i < f->nfail; i++)
        fprintf(fp, "fail-crtc %d\n", f->fail_crtc[i]);
    for (i = 0; i < hw->nmode; i++) {
        const DrMode *m = &hw->modes[i];

        fprintf(fp, "mode 0x%lx %s %.6f %u %u %u %u %u %u %u %u", m->xid,
                m->name, m->dotclock / 1e6, m->width, m->hsync_start,
                m->hsync_end, m->htotal, m->height, m->vsync_start,
                m->vsync_end, m->vtotal);
        write_flags(fp, m->flags);
        fputc('\n', fp);
    }
    for (i = 0; i < hw->ncrtc; i++) {
        const DrCrtcState *s = &c->crtc[i];
        const double *t = s->transform;
        const int *p = s->panning;
        unsigned rot = hw->crtcs[i].rotations;

        fprintf(fp, "crtc %d\n  rotations", i);
        for (j = 0; j < 4; j++)
            if (rot & (1u << j))
                fprintf(fp, " %s", rotation_name(1u << j));
        if (rot & DR_REFLECT_X)
            fputs(" x", fp);
        if (rot & DR_REFLECT_Y)
            fputs(" y", fp);
        fprintf(fp, "\n  gammasize %d\n", hw->crtcs[i].gamma_size);
        if (s->mode >= 0) {
            fprintf(fp, "  active 0x%lx %d %d %s %s", hw->modes[s->mode].xid,
                    s->x, s->y, rotation_name(s->rotation),
                    reflection_name(s->rotation));
            for (j = 0; j < hw->noutput; j++)
                if (s->outputs & BIT(j))
                    fprintf(fp, " %s", hw->outputs[j].name);
            fputc('\n', fp);
        }
        fprintf(fp, "  transform %g,%g,%g,%g,%g,%g,%g,%g,%g\n", t[0], t[1],
                t[2], t[3], t[4], t[5], t[6], t[7], t[8]);
        fprintf(fp, "  filter %s\n", s->filter);
        if (s->has_gamma)
            fprintf(fp, "  gamma %g:%g:%g\n", s->gamma[0], s->gamma[1],
                    s->gamma[2]);
        fprintf(fp, "  panning %dx%d+%d+%d/%dx%d+%d+%d/%d/%d/%d/%d\n", p[2],
                p[3], p[0], p[1], p[6], p[7], p[4], p[5], p[8], p[9], p[10],
                p[11]);
    }
    for (i = 0; i < hw->noutput; i++) {
        const DrOutput *o = &hw->outputs[i];

        fprintf(fp, "output %s\n  %s\n", o->name,
                o->connected ? "connected" : "disconnected");
        if (o->edid)
            fprintf(fp, "  edid %s\n", o->edid);
        fprintf(fp, "  mm %lux%lu\n  modes", o->mm_width, o->mm_height);
        for (j = 0; j < o->nmode; j++)
            fprintf(fp, " 0x%lx%s", hw->modes[o->modes[j]].xid,
                    j < o->npreferred ? "*" : "");
        fputs("\n  crtcs", fp);
        for (j = 0; j < o->ncrtc; j++)
            fprintf(fp, " %d", o->crtcs[j]);
        fputc('\n', fp);
        if (o->nclone) {
            fputs("  clones", fp);
            for (j = 0; j < o->nclone; j++)
                fprintf(fp, " %s", hw->outputs[o->clones[j]].name);
            fputc('\n', fp);
        }
    }
}

static int
fake_write(Fake *f, const DrConfig *c)
{
    const char *out = getenv("DTRANDR_FAKE_OUT");
    char *tmp;
    FILE *fp;
    int r = 0;

    if (!out || !*out)
        out = f->path;
    tmp = xasprintf("%s.tmp", out);
    if (!(fp = fopen(tmp, "w"))) {
        r = dr_error("cannot write %s: %s", tmp, strerror(errno));
        free(tmp);
        return r;
    }
    write_hw(fp, &f->hw, c, f);
    if (ferror(fp) | fclose(fp) || rename(tmp, out))
        r = dr_error("cannot write %s", out);
    free(tmp);
    return r;
}

static void *
dup_mem(const void *src, size_t n)
{
    void *p = xmalloc(n);

    if (n)
        memcpy(p, src, n);
    return p;
}

static int
fake_query(DrBackend *b, DrHardware *hw, int probe)
{
    Fake *f = b->priv;
    DrHardware *h = &f->hw;
    int i;

    (void) probe;
    hw_free(h);
    if (fake_read(f)) {
        hw_free(h);
        return -1;
    }
    /* hand out a deep copy */
    *hw = *h;
    hw->modes = dup_mem(h->modes, h->nmode * sizeof *hw->modes);
    for (i = 0; i < h->nmode; i++)
        hw->modes[i].name = xstrdup(h->modes[i].name);
    hw->outputs = dup_mem(h->outputs, h->noutput * sizeof *hw->outputs);
    for (i = 0; i < h->noutput; i++) {
        DrOutput *o = &hw->outputs[i];
        const DrOutput *src = &h->outputs[i];

        o->name = xstrdup(src->name);
        o->edid = src->edid ? xstrdup(src->edid) : NULL;
        o->modes = dup_mem(src->modes, src->nmode * sizeof(int));
        o->crtcs = dup_mem(src->crtcs, src->ncrtc * sizeof(int));
        o->clones = dup_mem(src->clones, src->nclone * sizeof(int));
    }
    hw->crtcs = dup_mem(h->crtcs, h->ncrtc * sizeof *hw->crtcs);
    config_copy(&hw->cur, &h->cur);
    return 0;
}

/* ------------------------------------------------------------------ */
/* applying, with the checks of an X server                            */

static int
op_begin(void *ctx)
{
    fake_log(ctx, "grab");
    return 0;
}

static int
op_end(void *ctx)
{
    fake_log(ctx, "ungrab");
    return 0;
}

static int
fits_screen(const DrHardware *hw, const DrCrtcState *s, int w, int h)
{
    int x1, y1, x2, y2;

    if (crtc_bounds(hw, s, &x1, &y1, &x2, &y2))
        return 1;
    return x2 <= w && y2 <= h;
}

static int
op_set_size(void *ctx, int w, int h, int mmw, int mmh)
{
    Fake *f = ctx;
    const DrHardware *hw = &f->hw;
    int c;

    fake_log(f, "size %dx%d %dx%dmm", w, h, mmw, mmh);
    if (w < hw->min_width || h < hw->min_height || w > hw->max_width
        || h > hw->max_height)
        return dr_error("fake: BadValue: screen size %dx%d", w, h);
    for (c = 0; c < f->state.ncrtc; c++)
        if (f->state.crtc[c].mode >= 0
            && !fits_screen(hw, &f->state.crtc[c], w, h))
            return dr_error("fake: BadMatch: crtc %d outside %dx%d", c, w, h);
    f->state.width = w;
    f->state.height = h;
    f->state.mm_width = mmw;
    f->state.mm_height = mmh;
    return 0;
}

static int
op_set_crtc(void *ctx, int c, const DrCrtcState *s, const DrCrtcState *cur)
{
    Fake *f = ctx;
    const DrHardware *hw = &f->hw;
    DrCrtcState *st = &f->state.crtc[c];
    int o, i, k;

    (void) cur;
    if (s->mode < 0) {
        fake_log(f, "disable crtc %d", c);
        st->mode = -1;
        st->outputs = 0;
        return 0;
    }
    {
        DrBuf b = { 0 };
        char *names;

        for (o = 0; o < hw->noutput; o++)
            if (s->outputs & BIT(o))
                buf_printf(&b, " %s", hw->outputs[o].name);
        names = buf_take(&b);
        fake_log(f, "crtc %d %s %dx%d+%d+%d %s %s%s", c,
                 hw->modes[s->mode].name, hw->modes[s->mode].width,
                 hw->modes[s->mode].height, s->x, s->y,
                 rotation_name(s->rotation), reflection_name(s->rotation),
                 names);
        free(names);
    }
    if (!transform_is_identity(s->transform) || s->filter[0])
        fake_log(f, "transform %d %g,%g,%g,%g,%g,%g,%g,%g,%g %s", c,
                 s->transform[0], s->transform[1], s->transform[2],
                 s->transform[3], s->transform[4], s->transform[5],
                 s->transform[6], s->transform[7], s->transform[8],
                 s->filter);
    for (i = 0; i < f->nfail; i++)
        if (f->fail_crtc[i] == c)
            return dr_error("fake: crtc %d failed (fail-crtc)", c);
    if (!s->outputs)
        return dr_error("fake: BadMatch: crtc %d without outputs", c);
    if (s->rotation != DR_ROTATE_0
        && (hw->crtcs[c].rotations & s->rotation) != s->rotation)
        return dr_error("fake: BadMatch: crtc %d cannot rotate", c);
    for (o = 0; o < hw->noutput; o++) {
        const DrOutput *out = &hw->outputs[o];

        if (!(s->outputs & BIT(o)))
            continue;
        for (i = 0; i < out->ncrtc; i++)
            if (out->crtcs[i] == c)
                break;
        if (i == out->ncrtc)
            return dr_error("fake: BadMatch: crtc %d cannot drive %s", c,
                            out->name);
        for (i = 0; i < out->nmode; i++)
            if (out->modes[i] == s->mode)
                break;
        if (i == out->nmode)
            return dr_error("fake: BadMatch: %s has no mode %s", out->name,
                            hw->modes[s->mode].name);
        for (k = 0; k < f->state.ncrtc; k++)
            if (k != c && f->state.crtc[k].mode >= 0
                && (f->state.crtc[k].outputs & BIT(o)))
                return dr_error("fake: BadMatch: %s is on crtc %d",
                                out->name, k);
        for (k = 0; k < hw->noutput; k++) {
            if (k == o || !(s->outputs & BIT(k)))
                continue;
            for (i = 0; i < out->nclone; i++)
                if (out->clones[i] == k)
                    break;
            if (i == out->nclone)
                return dr_error("fake: BadMatch: %s cannot clone %s",
                                out->name, hw->outputs[k].name);
        }
    }
    if (!fits_screen(hw, s, f->state.width, f->state.height))
        return dr_error("fake: BadValue: crtc %d outside the screen", c);
    i = st->has_gamma;
    {
        double g[3];

        memcpy(g, st->gamma, sizeof g);
        *st = *s;
        st->has_gamma = i;
        memcpy(st->gamma, g, sizeof g);
    }
    return 0;
}

static int
op_set_gamma(void *ctx, int c, const double g[3])
{
    Fake *f = ctx;

    fake_log(f, "gamma %d %g:%g:%g", c, g[0], g[1], g[2]);
    if (f->hw.crtcs[c].gamma_size <= 0)
        return dr_error("fake: crtc %d has no gamma", c);
    f->state.crtc[c].has_gamma = 1;
    memcpy(f->state.crtc[c].gamma, g, 3 * sizeof g[0]);
    return 0;
}

static int
op_set_primary(void *ctx, int o)
{
    Fake *f = ctx;

    fake_log(f, "primary %s", o >= 0 ? f->hw.outputs[o].name : "none");
    f->state.primary = o;
    return 0;
}

static const DrApplyOps fake_ops = {
    op_begin, op_end, op_set_size, op_set_crtc, op_set_gamma, op_set_primary
};

static int
fake_apply(DrBackend *b, const DrHardware *hw, const DrConfig *target)
{
    Fake *f = b->priv;
    int r;

    (void) hw;
    config_copy(&f->state, &f->hw.cur);
    r = apply_config(&f->hw, target, &fake_ops, f);
    if (fake_write(f, &f->state))
        r = -1;
    config_free(&f->state);
    return r;
}

static void
fake_close(DrBackend *b)
{
    Fake *f = b->priv;

    hw_free(&f->hw);
    if (f->log)
        fclose(f->log);
    free(f->fail_crtc);
    free(f->path);
    free(f);
    free(b);
}

DrBackend *
fake_backend_open(const char *path)
{
    DrBackend *b = xcalloc(1, sizeof *b);
    Fake *f = xcalloc(1, sizeof *f);
    const char *log = getenv("DTRANDR_FAKE_LOG");

    f->path = xstrdup(path);
    if (log && *log && !(f->log = fopen(log, "a")))
        dr_warn("cannot write %s: %s", log, strerror(errno));
    b->priv = f;
    b->query = fake_query;
    b->apply = fake_apply;
    b->close = fake_close;
    return b;
}
