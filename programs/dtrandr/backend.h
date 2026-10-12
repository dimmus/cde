/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: the interface to the display hardware
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

#ifndef DTRANDR_BACKEND_H
#define DTRANDR_BACKEND_H

#include "dtrandr.h"

/*
 * The steps of applying a configuration.  apply_config() (apply.c)
 * decides which to take in which order, as xrandr does; a backend
 * carries them out.  Each returns 0, or reports the error and returns -1.
 * set_crtc() gets the state the CRTC has now (as far as dtrandr knows)
 * in cur, so that it only sends what changes; a state with mode -1
 * disables the CRTC.
 */
typedef struct {
    int (*begin)(void *ctx);
    int (*end)(void *ctx);
    int (*set_size)(void *ctx, int width, int height, int mm_width,
                    int mm_height);
    int (*set_crtc)(void *ctx, int crtc, const DrCrtcState *s,
                    const DrCrtcState *cur);
    int (*set_gamma)(void *ctx, int crtc, const double gamma[3]);
    int (*set_primary)(void *ctx, int output);
} DrApplyOps;

int apply_config(const DrHardware *hw, const DrConfig *target,
                 const DrApplyOps *ops, void *ctx);

typedef struct DrBackend DrBackend;

struct DrBackend {
    /* Read the hardware and its configuration.  probe: ask the server
       to look for monitors (XRRGetScreenResources) rather than report
       what it knows (XRRGetScreenResourcesCurrent). */
    int (*query)(DrBackend *b, DrHardware *hw, int probe);
    /* Apply target; hw is what query returned last */
    int (*apply)(DrBackend *b, const DrHardware *hw,
                 const DrConfig *target);
    void (*close)(DrBackend *b);
    void *priv;
};

/* fake.c: the hardware is a text file (DTRANDR_FAKE) */
DrBackend *fake_backend_open(const char *path);

/* xbackend.c */
DrBackend *x_backend_open(const char *display);
/* for --daemon: 0 = this is the only daemon on the display, 1 = another
   one runs, -1 = error */
int x_backend_lock(DrBackend *b);
int x_backend_watch(DrBackend *b);
int x_backend_fd(DrBackend *b);
/* Read the queued events; returns how many were RandR events */
int x_backend_events(DrBackend *b);

#endif /* DTRANDR_BACKEND_H */
