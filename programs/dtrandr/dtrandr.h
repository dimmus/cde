/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: apply saved X display setups (autorandr-compatible profiles)
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
 * The data model and the pure logic of dtrandr.  Nothing here includes
 * an X header: the hardware is described by DrHardware, which a backend
 * (the X server through libXrandr, or a text file for the tests) fills
 * in, and a configuration to apply is a DrConfig, which a backend
 * applies.  The constants have the values of the RandR protocol, so the
 * X backend passes them through unchanged.
 */

#ifndef DTRANDR_H
#define DTRANDR_H

#include <stddef.h>
#include <stdio.h>

/* Rotation and reflection bits (RR_Rotate_0 ... RR_Reflect_Y) */
#define DR_ROTATE_0     1
#define DR_ROTATE_90    2
#define DR_ROTATE_180   4
#define DR_ROTATE_270   8
#define DR_ROTATE_MASK  15
#define DR_REFLECT_X    16
#define DR_REFLECT_Y    32

/* Mode flags (RR_HSyncPositive ... RR_DoubleScan) */
#define DR_HSYNC_POS    0x01
#define DR_HSYNC_NEG    0x02
#define DR_VSYNC_POS    0x04
#define DR_VSYNC_NEG    0x08
#define DR_INTERLACE    0x10
#define DR_DOUBLESCAN   0x20

/* Outputs are kept in a bit mask per CRTC */
#define DR_MAX_OUTPUTS  64

/* The fingerprint of a connected output whose EDID cannot be read
   (autorandr writes this, followed by "-OUTPUT", to the setup file) */
#define DR_EDID_UNAVAILABLE "--CONNECTED-BUT-EDID-UNAVAILABLE-"

typedef unsigned long long DrOutputMask;

typedef struct {
    unsigned long xid;
    char *name;
    unsigned width, height;
    unsigned long dotclock;             /* Hz */
    unsigned hsync_start, hsync_end, htotal, hskew;
    unsigned vsync_start, vsync_end, vtotal;
    unsigned long flags;
} DrMode;

typedef struct {
    char *name;
    unsigned long xid;
    int connected;
    char *edid;                         /* lower-case hex, NULL if none */
    unsigned long mm_width, mm_height;
    int nmode, npreferred;
    int *modes;                         /* indices into DrHardware.modes */
    int ncrtc;
    int *crtcs;                         /* indices into DrHardware.crtcs */
    int nclone;
    int *clones;                        /* indices into DrHardware.outputs */
} DrOutput;

typedef struct {
    unsigned long xid;
    unsigned rotations;                 /* supported rotations/reflections */
    int gamma_size;
} DrCrtc;

/* The state of one CRTC */
typedef struct {
    int mode;                           /* index into modes, -1 = off */
    int x, y;
    unsigned rotation;
    DrOutputMask outputs;
    double transform[9];
    char filter[32];
    int has_gamma;
    double gamma[3];                    /* as xrandr --gamma takes it */
    int panning[12];                    /* left top width height,
                                           tracking l t w h, border l t r b */
} DrCrtcState;

/* The state of the whole screen */
typedef struct {
    int width, height, mm_width, mm_height;
    int primary;                        /* output index, -1 = none */
    int ncrtc;
    DrCrtcState *crtc;
} DrConfig;

typedef struct {
    int min_width, min_height, max_width, max_height;
    int nmode;
    DrMode *modes;
    int noutput;
    DrOutput *outputs;
    int ncrtc;
    DrCrtc *crtcs;
    DrConfig cur;                       /* current configuration */
} DrHardware;

/* A "key value" line of a profile */
typedef struct {
    char *key;
    char *val;                          /* NULL for a key without value */
} DrOpt;

typedef struct {
    char *name;
    int nopt;
    DrOpt *opts;
} DrOutputConf;

typedef struct {
    char *output;
    char *edid;
} DrSetupLine;

typedef struct {
    char *name;
    char *dir;                          /* the profile's directory */
    int source;                         /* index of the search directory */
    int nhead;
    DrOpt *head;                        /* lines before the first output */
    int nsetup;
    DrSetupLine *setup;
    int nout;
    DrOutputConf *out;
} DrProfile;

/* What a profile asks of one output */
typedef struct {
    int on;
    int mode;
    int x, y;
    unsigned rotation;
    double transform[9];
    char filter[32];
    int crtc;                           /* requested CRTC, -1 = any */
    int has_gamma;
    double gamma[3];
    int has_panning;
    int panning[12];
} DrWant;

#define DR_PRIMARY_KEEP (-2)

/* What a profile asks of the screen */
typedef struct {
    int noutput;
    DrWant *want;
    int primary;                        /* output, -1 none, DR_PRIMARY_KEEP */
    double dpi;                         /* 0 = keep the current DPI */
} DrPlan;

/* util.c */
extern int dr_verbose;                  /* 1 = --verbose, 2 = --debug */
extern int dr_quiet;                    /* > 0: no error messages */
extern const char *dr_progname;

void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t size);
void *xrealloc(void *p, size_t n);
char *xstrdup(const char *s);
char *xasprintf(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
int dr_error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void dr_warn(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void dr_info(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void dr_debug(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

typedef struct {
    char *s;
    size_t len, size;
} DrBuf;

void buf_add(DrBuf *b, const char *s);
void buf_printf(DrBuf *b, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
char *buf_take(DrBuf *b);
char *dr_format_double(double d);

/* profile.c */
DrProfile *profile_new(const char *name);
void profile_free(DrProfile *p);
DrOutputConf *profile_add_output(DrProfile *p, const char *name);
void opt_add(int *nopt, DrOpt **opts, const char *key, const char *val);
const DrOpt *opt_find(int nopt, const DrOpt *opts, const char *key);
int profile_parse_config(DrProfile *p, FILE *f, const char *path);
int profile_parse_setup(DrProfile *p, FILE *f, const char *path);
void profile_write_config(const DrProfile *p, FILE *f);
void profile_write_setup(const DrProfile *p, FILE *f);
DrProfile *profile_load(const char *dir, const char *name, int source);
int profile_save(const DrProfile *p, const char *dir, int force);
int is_virtual_profile(const char *name);
int profile_name_ok(const char *name);

typedef struct {
    int nprofile;
    DrProfile **profiles;
} DrProfileList;

void profiles_scan(DrProfileList *l, const char *dir, int source);
DrProfile *profiles_find(const DrProfileList *l, const char *name);
void profiles_free(DrProfileList *l);

char *settings_get(const char *path, const char *key);

/* layout.c */
void hw_free(DrHardware *hw);
void config_init(DrConfig *c, int ncrtc);
void config_copy(DrConfig *dst, const DrConfig *src);
void config_free(DrConfig *c);
void crtc_state_off(DrCrtcState *s);
int output_crtc(const DrConfig *c, int output);
int find_output(const DrHardware *hw, const char *name);
double mode_refresh(const DrMode *m);
int transform_is_identity(const double t[9]);
void transform_identity(double t[9]);
int crtc_bounds(const DrHardware *hw, const DrCrtcState *s,
                int *x1, int *y1, int *x2, int *y2);
const char *rotation_name(unsigned rotation);
const char *reflection_name(unsigned rotation);

char *output_fingerprint(const DrHardware *hw, int output);
void print_fingerprint(const DrHardware *hw, FILE *f);
int profile_match(const DrHardware *hw, const DrProfile *p,
                  int *exact, double *closeness);
int edid_glob_match(const char *pattern, const char *edid,
                    double *closeness);

void plan_init(DrPlan *plan, const DrHardware *hw);
void plan_free(DrPlan *plan);
int plan_from_profile(const DrHardware *hw, const DrProfile *p,
                      DrPlan *plan);
int plan_virtual(const DrHardware *hw, const char *name, DrPlan *plan);
int plan_auto(const DrHardware *hw, DrPlan *plan);
int plan_to_config(const DrHardware *hw, const DrPlan *plan,
                   DrConfig *target);
int config_equal(const DrHardware *hw, const DrConfig *a,
                 const DrConfig *b);
DrProfile *profile_from_config(const DrHardware *hw, const DrConfig *c,
                               const char *name);
char *xrandr_command(const DrHardware *hw, const DrPlan *plan,
                     const DrConfig *target);

#endif /* DTRANDR_H */
