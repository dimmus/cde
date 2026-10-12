/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: unit tests of the profile format, matching and geometry
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../backend.h"

static int failures, checks;

#define CHECK(cond) do {                                                \
        checks++;                                                       \
        if (!(cond)) {                                                  \
            fprintf(stderr, "%s:%d: failed: %s\n", __FILE__, __LINE__,  \
                    #cond);                                             \
            failures++;                                                 \
        }                                                               \
    } while (0)

#define CHECK_STR(a, b) do {                                            \
        const char *a_ = (a), *b_ = (b);                                \
        checks++;                                                       \
        if (strcmp(a_, b_)) {                                           \
            fprintf(stderr, "%s:%d: got\n%s\nexpected\n%s\n", __FILE__, \
                    __LINE__, a_, b_);                                  \
            failures++;                                                 \
        }                                                               \
    } while (0)

static DrProfile *
parse(const char *config, const char *setup)
{
    DrProfile *p = profile_new("test");
    FILE *f;

    if (config) {
        f = fmemopen((void *) config, strlen(config), "r");
        profile_parse_config(p, f, "config");
        fclose(f);
    }
    if (setup) {
        f = fmemopen((void *) setup, strlen(setup), "r");
        profile_parse_setup(p, f, "setup");
        fclose(f);
    }
    return p;
}

static char *
written(const DrProfile *p, int setup)
{
    char *s = NULL;
    size_t n = 0;
    FILE *f = open_memstream(&s, &n);

    if (setup)
        profile_write_setup(p, f);
    else
        profile_write_config(p, f);
    fclose(f);
    return s;
}

/* autorandr's format, read and written back as autorandr writes it */
static void
test_roundtrip(void)
{
    static const char in[] =
        "# a comment\n"
        "output HDMI-1\n"
        "  mode 1920x1080  \n"
        "rate 60.00\n"
        "pos 0x0\n"
        "transform 1.500000,0.000000,0.000000,0.000000,1.500000,0.000000,"
        "0.000000,0.000000,1.000000\n"
        "x-prop-broadcast_rgb Automatic\n"
        "panning 3840x2160+0+0/3840x2160+0+0/0/0/0/0\n"
        "gamma 1.0:0.909:0.833\n"
        "\n"
        "primary\n"
        "crtc 0\n"
        "rate 59.94\n"
        "output eDP-1\n"
        "scale 1.25x1.25\n"
        "mode 1280x720\n"
        "dpi 120\n"
        "filter nearest\n"
        "reflect xy\n"
        "rotate inverted\n"
        "output DP-1\n"
        "off\n";
    static const char out[] =
        "output HDMI-1\n"
        "crtc 0\n"
        "gamma 1.0:0.909:0.833\n"
        "mode 1920x1080\n"
        "panning 3840x2160+0+0/3840x2160+0+0/0/0/0/0\n"
        "pos 0x0\n"
        "primary\n"
        "rate 59.94\n"
        "transform 1.500000,0.000000,0.000000,0.000000,1.500000,0.000000,"
        "0.000000,0.000000,1.000000\n"
        "x-prop-broadcast_rgb Automatic\n"
        "output eDP-1\n"
        "dpi 120\n"
        "filter nearest\n"
        "mode 1280x720\n"
        "reflect xy\n"
        "rotate inverted\n"
        "scale 1.25x1.25\n"
        "output DP-1\n"
        "off\n";
    static const char setup_in[] =
        "# fingerprint\n"
        "eDP-1 00ffffffffffff000daeaaaa\n"
        "only-one-field\n"
        "HDMI-1 *\n";
    static const char setup_out[] =
        "HDMI-1 *\n"
        "eDP-1 00ffffffffffff000daeaaaa\n";
    DrProfile *p = parse(in, setup_in), *q;
    char *s, *t;

    CHECK(p->nout == 3);
    CHECK(p->nsetup == 2);
    CHECK_STR(opt_find(p->out[0].nopt, p->out[0].opts, "rate")->val,
              "59.94");
    CHECK(opt_find(p->out[0].nopt, p->out[0].opts, "primary")->val == NULL);
    s = written(p, 0);
    CHECK_STR(s, out);
    q = parse(s, NULL);
    t = written(q, 0);
    CHECK_STR(t, s);
    free(s);
    free(t);
    s = written(p, 1);
    CHECK_STR(s, setup_out);
    free(s);
    profile_free(p);
    profile_free(q);
}

static void
test_glob(void)
{
    double c;

    CHECK(edid_glob_match("00ffAB", "00ffab", &c) == 2 && c == 1);
    CHECK(edid_glob_match("00ffab", "00ffac", &c) == 0);
    CHECK(edid_glob_match("*", "00ffab", &c) == 1 && c == 0);
    CHECK(edid_glob_match("00ff*", "00ffab", &c) == 1
          && fabs(c - 4.0 / 6) < 1e-9);
    CHECK(edid_glob_match("00ff?b", "00ffab", &c) == 1
          && fabs(c - 5.0 / 6) < 1e-9);
    CHECK(edid_glob_match("*ab", "00ffab", &c) == 1);
    CHECK(edid_glob_match("*ac", "00ffab", &c) == 0);
    CHECK(edid_glob_match("11*", "00ffab", &c) == 0);
    CHECK(edid_glob_match("00*ff*b", "00ffab", &c) == 1);
}

static void
test_numbers(void)
{
    DrMode m;
    char *s;

    memset(&m, 0, sizeof m);
    m.dotclock = 148500000;
    m.htotal = 2200;
    m.vtotal = 1125;
    CHECK(fabs(mode_refresh(&m) - 60.0) < 1e-9);
    m.flags = DR_INTERLACE;
    CHECK(fabs(mode_refresh(&m) - 120.0) < 1e-9);
    m.flags = DR_DOUBLESCAN;
    CHECK(fabs(mode_refresh(&m) - 30.0) < 1e-9);
    m.htotal = 0;
    CHECK(mode_refresh(&m) == 0);
    s = dr_format_double(1.0);
    CHECK_STR(s, "1.0");
    free(s);
    s = dr_format_double(0.833);
    CHECK_STR(s, "0.833");
    free(s);
    s = dr_format_double(2);
    CHECK_STR(s, "2.0");
    free(s);
}

static int
load_hw(DrHardware *hw)
{
    const char *srcdir = getenv("srcdir");
    char *path = xasprintf("%s/tests/fake/docked.fake",
                           srcdir ? srcdir : ".");
    DrBackend *b = fake_backend_open(path);
    int r = b->query(b, hw, 1);

    b->close(b);
    free(path);
    return r;
}

static void
test_match(const DrHardware *hw)
{
    static const char *const A = "00ffffffffffff000daeaaaa0000000001";
    static const char *const B = "00ffffffffffff0010acbbbb0000000002";
    DrProfile *p;
    char *setup;
    int exact;
    double c;

    setup = xasprintf("eDP-1 %s\nHDMI-1 %s\n", A, B);
    p = parse(NULL, setup);
    CHECK(profile_match(hw, p, &exact, &c) && exact == 2);
    profile_free(p);
    free(setup);

    setup = xasprintf("eDP-1 %s\nHDMI-1 *\n", A);
    p = parse(NULL, setup);
    CHECK(profile_match(hw, p, &exact, &c) && exact == 1);
    profile_free(p);
    free(setup);

    setup = xasprintf("eDP-1 %s\nHDMI-1 00ffffffffffff0010ac*\n", A);
    p = parse(NULL, setup);
    CHECK(profile_match(hw, p, &exact, &c) && exact == 1 && c > 1.5);
    profile_free(p);
    free(setup);

    /* a connected output missing from the setup */
    setup = xasprintf("eDP-1 %s\n", A);
    p = parse(NULL, setup);
    CHECK(!profile_match(hw, p, &exact, &c));
    profile_free(p);
    free(setup);

    /* an output in the setup that is not connected */
    setup = xasprintf("eDP-1 %s\nHDMI-1 %s\nDP-1 *\n", A, B);
    p = parse(NULL, setup);
    CHECK(!profile_match(hw, p, &exact, &c));
    profile_free(p);
    free(setup);

    /* another monitor */
    setup = xasprintf("eDP-1 %s\nHDMI-1 %s0\n", A, B);
    p = parse(NULL, setup);
    CHECK(!profile_match(hw, p, &exact, &c));
    profile_free(p);
    free(setup);

    p = parse(NULL, "");
    CHECK(!profile_match(hw, p, &exact, &c));
    profile_free(p);
}

static void
test_geometry(const DrHardware *hw)
{
    DrCrtcState s;
    DrPlan plan;
    DrConfig cfg;
    DrProfile *p;
    int x1, y1, x2, y2;

    crtc_state_off(&s);
    s.mode = 0;                         /* 1920x1080 */
    s.x = 100;
    s.rotation = DR_ROTATE_90;
    CHECK(!crtc_bounds(hw, &s, &x1, &y1, &x2, &y2));
    CHECK(x1 == 100 && y1 == 0 && x2 == 1180 && y2 == 1920);
    s.rotation = DR_ROTATE_0;
    s.transform[0] = s.transform[4] = 1.5;
    CHECK(!crtc_bounds(hw, &s, &x1, &y1, &x2, &y2));
    CHECK(x2 == 100 + 2880 && y2 == 1620);
    s.panning[2] = 4000;
    s.panning[3] = 2000;
    CHECK(!crtc_bounds(hw, &s, &x1, &y1, &x2, &y2));
    CHECK(x1 == 0 && x2 == 4000 && y2 == 2000);

    /* eDP-1 turned left, HDMI-1 right of it */
    p = parse("output eDP-1\nmode 1920x1080\nrotate left\nprimary\n"
              "output HDMI-1\nmode 2560x1440\npos 1080x0\n"
              "output DP-1\noff\n", NULL);
    CHECK(!plan_from_profile(hw, p, &plan));
    CHECK(!plan_to_config(hw, &plan, &cfg));
    CHECK(cfg.width == 1080 + 2560 && cfg.height == 1920);
    CHECK(cfg.primary == find_output(hw, "eDP-1"));
    /* the DPI stays: 1080 / 285mm */
    CHECK(abs(cfg.mm_height - (int) (1920 * 285.0 / 1080)) <= 1);
    config_free(&cfg);
    plan_free(&plan);
    profile_free(p);

    /* dpi, scale */
    p = parse("dpi 96\noutput HDMI-1\nmode 1280x720\nscale 2x2\n"
              "output eDP-1\noff\n", NULL);
    CHECK(!plan_from_profile(hw, p, &plan));
    CHECK(plan.dpi == 96);
    CHECK(!plan_to_config(hw, &plan, &cfg));
    CHECK(cfg.width == 2560 && cfg.height == 1440);
    CHECK(cfg.mm_width == (int) (2560 * 25.4 / 96));
    CHECK(cfg.primary == find_output(hw, "eDP-1"));    /* kept */
    config_free(&cfg);
    plan_free(&plan);
    profile_free(p);

    /* too large */
    p = parse("output HDMI-1\nmode 2560x1440\nscale 4x4\n", NULL);
    CHECK(!plan_from_profile(hw, p, &plan));
    dr_quiet++;
    CHECK(plan_to_config(hw, &plan, &cfg));
    dr_quiet--;
    config_free(&cfg);
    plan_free(&plan);
    profile_free(p);

    /* errors */
    dr_quiet++;
    p = parse("output DP-1\nmode 1280x1024\n", NULL);
    CHECK(plan_from_profile(hw, p, &plan));     /* not connected */
    profile_free(p);
    p = parse("output HDMI-1\nmode 3840x2160\n", NULL);
    CHECK(plan_from_profile(hw, p, &plan));     /* no such mode */
    profile_free(p);
    p = parse("output HDMI-1\nmode 1280x720\nrotate sideways\n", NULL);
    CHECK(plan_from_profile(hw, p, &plan));
    profile_free(p);
    p = parse("output HDMI-1\nmode 1280x720\ntransform 1,0,0\n", NULL);
    CHECK(plan_from_profile(hw, p, &plan));
    profile_free(p);
    p = parse("output HDMI-1\nmode 1280x720\npanning 10\n", NULL);
    CHECK(plan_from_profile(hw, p, &plan));
    profile_free(p);
    p = parse("output VGA-7\nmode 1280x720\n", NULL);
    CHECK(plan_from_profile(hw, p, &plan));
    profile_free(p);
    dr_quiet--;
    p = parse("output VGA-7\noff\n", NULL);     /* fine: it is off */
    CHECK(!plan_from_profile(hw, p, &plan));
    plan_free(&plan);
    profile_free(p);
}

/* The current configuration as a profile, and back */
static void
test_current(const DrHardware *hw)
{
    DrProfile *p = profile_from_config(hw, &hw->cur, "now");
    DrPlan plan;
    DrConfig cfg;
    char *s = written(p, 0);

    CHECK_STR(s, "output DP-1\noff\n"
              "output HDMI-1\noff\n"
              "output eDP-1\ncrtc 0\ngamma 1.0:1.0:1.0\nmode 1920x1080\n"
              "pos 0x0\nprimary\nrate 60.01\nrotate normal\n");
    free(s);
    CHECK(!plan_from_profile(hw, p, &plan));
    CHECK(!plan_to_config(hw, &plan, &cfg));
    CHECK(config_equal(hw, &hw->cur, &cfg));
    config_free(&cfg);
    plan_free(&plan);
    profile_free(p);
}

int
main(void)
{
    DrHardware hw;

    test_roundtrip();
    test_glob();
    test_numbers();
    if (load_hw(&hw)) {
        fprintf(stderr, "cannot read the fake display\n");
        return 1;
    }
    test_match(&hw);
    test_geometry(&hw);
    test_current(&hw);
    hw_free(&hw);
    printf("%d checks, %d failed\n", checks, failures);
    return failures != 0;
}
