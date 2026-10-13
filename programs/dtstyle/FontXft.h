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
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        FontXft.h
 **
 **   Project:     CDE
 **
 **   Description: Non-UI logic of the Scalable (Xft) mode of the Style
 **                Manager Font dialog: the font resources it writes and
 **                removes, the read-back of the current settings, the
 **                fontconfig family list, the preview render table, the
 **                fontconfig override file and the session state.
 **
 **                FontXft.c does not use Main.h or the global `style'.
 **                Everything that needs fontconfig or Xft is compiled
 **                only with DTSTYLE_HAVE_XFT; without it the family list
 **                and the preview are NULL and the rest still works.
 **
 **                All returned char * are XtMalloc'd (free with XtFree).
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _fontxft_h
#define _fontxft_h
#include <X11/Intrinsic.h>
#include <X11/Xresource.h>
#include <Xm/Xm.h>

#define FX_MODE_BITMAP 0
#define FX_MODE_XFT    1
#define FX_TRI_DEFAULT 0
#define FX_TRI_ON      1
#define FX_TRI_OFF     2
#define FX_HS_DEFAULT 0
#define FX_HS_NONE    1
#define FX_HS_SLIGHT  2
#define FX_HS_MEDIUM  3
#define FX_HS_FULL    4
#define FX_RGBA_DEFAULT 0
#define FX_RGBA_NONE    1
#define FX_RGBA_RGB     2
#define FX_RGBA_BGR     3
#define FX_RGBA_VRGB    4
#define FX_RGBA_VBGR    5
#define FX_LCD_DEFAULT  0
#define FX_LCD_NONE     1
#define FX_LCD_STANDARD 2
#define FX_LCD_LIGHT    3
#define FX_LCD_LEGACY   4
#define FX_SIZE_MIN 4
#define FX_SIZE_MAX 96
#define FX_DPI_MIN 48
#define FX_DPI_MAX 480
#define FX_NAME_MAX 256
#define FX_TAG_SYSTEM "DtXftSystem"
#define FX_TAG_USER   "DtXftUser"
#define FX_OWN_RENDER 0x1
#define FX_OWN_XFTOPT 0x2
#define FX_HAVE_FACES     0x1
#define FX_HAVE_RENDERING 0x2

typedef struct { char family[FX_NAME_MAX]; char style[FX_NAME_MAX]; int size; } FxFace;
typedef struct {
    int     mode;                 /* FX_MODE_* */
    FxFace  sys, user;
    Boolean userMonoOnly;
    int     antialias, hinting;   /* FX_TRI_* */
    int     hintStyle;            /* FX_HS_* */
    int     rgba;                 /* FX_RGBA_* */
    int     lcdFilter;            /* FX_LCD_* */
    int     dpi;                  /* 0 = automatic */
    Boolean fcOverride;
} FxSettings;
typedef struct { const char *sysStr; const char *userStr; } FxBitmapChoice;
typedef struct { char *family; Boolean mono; int numStyles; char **styles; } FxFamily;
typedef struct { int numFamilies; FxFamily *families; } FxFamilyList;

extern Boolean FxMotifHasXft(Widget w);
extern FxFamilyList *FxListFamilies(void);
extern void FxFreeFamilyList(FxFamilyList *list);
extern const FxFamily *FxFindFamily(const FxFamilyList *list, const char *family);
extern const char *FxResolveFamily(const FxFamilyList *list, const char *name, Boolean monoOnly);
extern const char *FxDefaultStyle(const FxFamily *fam);
extern Boolean FxNameIsSafe(const char *name);
extern void FxGetDefaults(Widget shell, FxSettings *out);
extern void FxReadCurrent(Display *dpy, FxSettings *inout);
extern void FxApplyRestored(FxSettings *dst, const FxSettings *restored, unsigned mask);
extern void FxNormalize(const FxFamilyList *list, FxSettings *s);
extern Boolean FxSettingsEqual(const FxSettings *a, const FxSettings *b);
extern int FxAutoDpi(Display *dpy);
extern char *FxBuildResources(const FxSettings *s, const FxBitmapChoice *b);
extern char *FxFilterOwned(const char *in, unsigned groups, const char *onlyIfIn, int *removed);
extern Boolean FxCommit(Display *dpy, const FxSettings *s, const FxBitmapChoice *b,
                        Boolean immediate, char **fontInfo);
extern char *FxFcFilePath(void);
extern Boolean FxFcFileIsOurs(void);
extern Boolean FxUpdateFcFile(const FxSettings *s);
extern XmRenderTable FxPreviewRenderTable(Widget w, const FxFace *face, const FxSettings *s);
extern void FxSaveState(int fd, const FxSettings *s);
extern unsigned FxRestoreState(XrmDatabase db, FxSettings *out);
#endif
