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
 **   File:        FontXft.c
 **
 **   Project:     CDE
 **
 **   Description: Non-UI logic of the Style Manager Font dialog, and of
 **                its Scalable (Xft) mode in particular.
 **
 **                Resources.  Bitmap mode writes the same eleven lines as
 **                the old dialog.  Xft mode writes them too (a libXm with
 **                the loose *Font fix is required) and adds
 **
 **                    the render table lines of fxRenderLines (the one
 **                    place that says which widgets get which face)
 **                    *DtXftSystem.fontType/fontName/fontStyle/fontSize
 **                    *DtXftUser.fontType/fontName/fontStyle/fontSize
 **                    Xft.antialias/hinting/hintstyle/rgba/lcdfilter/dpi
 **
 **                _DtAddToResource only merges, so the lines a mode does
 **                not write are removed from _DT_SM_PREFERENCES and
 **                RESOURCE_MANAGER first (FxFilterOwned, FxCommit).
 **
 **                Everything that needs fontconfig or Xft is compiled
 **                only with DTSTYLE_HAVE_XFT.  Without it there is no
 **                family list and no preview, but the resource code still
 **                works, so that a non-Xft build can switch a session
 **                back to Bitmap mode.
 **
 **                This file does not use Main.h or the global `style'.
 **
 ****************************************************************************
 ************************************<+>*************************************/

/*+++++++++++++++++++++++++++++++++++++++*/
/* include files                         */
/*+++++++++++++++++++++++++++++++++++++++*/

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Xresource.h>
#include <Xm/Xm.h>
#ifdef DTSTYLE_HAVE_XFT
#include <fontconfig/fontconfig.h>
#include <X11/Xft/Xft.h>
#endif
#include <Dt/DtP.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "FontXft.h"

/*+++++++++++++++++++++++++++++++++++++++*/
/* Local #defines                        */
/*+++++++++++++++++++++++++++++++++++++++*/

#define FX_SIZE_DEFAULT  10
#define FX_PREVIEW_TAG   "DtXftPreview"
#define FX_FC_FILE       "fontconfig/conf.d/90-dtstyle.conf"
#define FX_FC_MARKER     "Written by the CDE Style Manager"
#define FX_FC_PEEK       512

/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Variables                    */
/*+++++++++++++++++++++++++++++++++++++++*/

/* A growable string */
typedef struct {
    char   *str;
    size_t  len;
    size_t  size;
} FxBuf;

/* One resource entry, as _DtGetEntries (addToRes.c) sees it */
typedef struct {
    const char *start;          /* first byte of the entry            */
    const char *next;           /* byte after it (after its newline)  */
    const char *tag;            /* NULL: comment or no colon          */
    size_t      tagLen;         /* blanks trimmed on both sides       */
    const char *value;          /* leading blanks stripped            */
    size_t      valueLen;       /* trailing blanks trimmed            */
} FxEntry;

/* The application resources with the dialog's default faces */
typedef struct {
    String  sysFamily;
    String  sysStyle;
    int     sysSize;
    String  userFamily;
    String  userStyle;
    int     userSize;
} FxAppRes;

static XtResource fxAppResources[] = {
    { "xftSystemFamily", "XftSystemFamily", XmRString, sizeof(String),
      XtOffsetOf(FxAppRes, sysFamily), XmRString, (XtPointer)"sans-serif" },
    { "xftSystemStyle", "XftSystemStyle", XmRString, sizeof(String),
      XtOffsetOf(FxAppRes, sysStyle), XmRString, (XtPointer)"Regular" },
    { "xftSystemSize", "XftSystemSize", XmRInt, sizeof(int),
      XtOffsetOf(FxAppRes, sysSize), XmRImmediate, (XtPointer)FX_SIZE_DEFAULT },
    { "xftUserFamily", "XftUserFamily", XmRString, sizeof(String),
      XtOffsetOf(FxAppRes, userFamily), XmRString, (XtPointer)"monospace" },
    { "xftUserStyle", "XftUserStyle", XmRString, sizeof(String),
      XtOffsetOf(FxAppRes, userStyle), XmRString, (XtPointer)"Regular" },
    { "xftUserSize", "XftUserSize", XmRInt, sizeof(int),
      XtOffsetOf(FxAppRes, userSize), XmRImmediate, (XtPointer)FX_SIZE_DEFAULT },
};

/* Value names, indexed by the FX_* constants (index 0 is Default).  The
 * Xft.* resources, the fontconfig file and the session file use them. */
static const char *fxBoolNames[]  = { NULL, "true", "false" };
static const char *fxHsNames[]    = { NULL, "hintnone", "hintslight",
                                      "hintmedium", "hintfull" };
static const char *fxRgbaNames[]  = { NULL, "none", "rgb", "bgr",
                                      "vrgb", "vbgr" };
static const char *fxLcdNames[]   = { NULL, "lcdnone", "lcddefault",
                                      "lcdlight", "lcdlegacy" };
static const char *fxTriState[]   = { "default", "on", "off" };

#define FX_NUM(a) ((int)(sizeof(a) / sizeof((a)[0])))

/*
 * THE place that says which widgets get which scalable font.
 *
 * Scalable (Xft) mode writes one line for each entry, "resource: face",
 * in this order, and switching back to Bitmap removes them (when their
 * value is a DtXft rendition tag).  To give more widgets the scalable
 * fonts, add a line here (and to dtstyle.man).  A line that is no longer
 * wanted stays, with face NULL: it is then removed but not written, so a
 * session saved by an older Style Manager is cleaned up too.
 *
 * A widget takes a render table line only if it has a render table
 * resource (Motif widgets do, and DtWidget's DtIcon and DtTitleBox,
 * see _DtRenderTableResource in lib/DtWidget/DtWidgetI.h) and does not
 * get its font in its creation arglist.
 */
static const struct {
    const char *resource;
    const char *face;               /* FX_TAG_SYSTEM, FX_TAG_USER or NULL */
} fxRenderLines[] = {
    /* everything: labels, buttons, menus, lists, ... */
    { "*renderTable",                             FX_TAG_SYSTEM },
    /* text that is edited */
    { "*XmText*renderTable",                      FX_TAG_USER },
    { "*XmTextField*renderTable",                 FX_TAG_USER },
    /*
     * Text fields that show a choice, like a list or an option menu:
     * these lines name the container before XmTextField, so Xrm prefers
     * them.  XmComboBox needs none: it gives its text field and list its
     * own render table.
     */
    { "*XmSimpleSpinBox*XmTextField*renderTable", FX_TAG_SYSTEM },
    { "*XmSpinBox*XmTextField*renderTable",       FX_TAG_SYSTEM },
    { "*DtComboBox*XmTextField*renderTable",      FX_TAG_SYSTEM },
    { "*DtSpinBox*XmTextField*renderTable",       FX_TAG_SYSTEM },
    /*
     * DtEditor (dtpad, dtmail) gives its text, and the buttons and labels
     * of its dialogs, its own font lists, kept in the BulletinBoard's
     * fields: Motif's render table names of those fields reach them.
     */
    { "*DtEditor*textRenderTable",                FX_TAG_USER },
    { "*DtEditor*buttonRenderTable",              FX_TAG_SYSTEM },
    { "*DtEditor*labelRenderTable",               FX_TAG_SYSTEM },
    /*
     * Programs that take the desktop fonts as their own app resources
     * systemFont and userFont (font lists, the bitmap lines) take these
     * when they are set: dtfile for its icon labels, dtcm for its
     * calendar.  One convention for every such program.
     */
    { "*systemRenderTable",                       FX_TAG_SYSTEM },
    { "*userRenderTable",                         FX_TAG_USER },
};
static const char *fxRenderPrefixes[] = {
    "*" FX_TAG_SYSTEM ".", "*" FX_TAG_USER "."
};
static const char *fxXftTags[] = {
    "Xft.antialias", "Xft.hinting", "Xft.hintstyle", "Xft.rgba",
    "Xft.lcdfilter", "Xft.dpi"
};

/* Identifiers that xrdb's cpp replaces when dt.resources is loaded at
 * login, and the prefixes of the ones xrdb defines per server/client. */
static const char *fxCppWords[] = {
    "unix", "linux", "i386", "i486", "i586", "i686", "sun", "sparc", "mips",
    "vax", "hppa", "SERVERHOST", "HOST", "CLIENTHOST", "VERSION", "REVISION",
    "VENDOR", "RELEASE", "NUM_SCREENS", "SCREEN_NUM", "BITS_PER_RGB",
    "CLASS", "COLOR", "HEIGHT", "WIDTH", "PLANES", "X_RESOLUTION",
    "Y_RESOLUTION"
};
static const char *fxCppPrefixes[] = {
    "SRVR_", "CLNT_", "VNDR_", "EXT_", "CLASS_", "DISPLAY_"
};

/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Functions                    */
/*+++++++++++++++++++++++++++++++++++++++*/

static void BufAdd(
                        FxBuf *b,
                        const char *s,
                        size_t n) ;
static void BufStr(
                        FxBuf *b,
                        const char *s) ;
static void BufLine(
                        FxBuf *b,
                        const char *tag,
                        const char *value) ;
static void BufIntLine(
                        FxBuf *b,
                        const char *tag,
                        int value) ;
static void CopyName(
                        char *dst,
                        const char *src) ;
static int ClampSize(
                        int size) ;
static int ClampDpi(
                        int dpi) ;
static int ClampIndex(
                        int value,
                        int count) ;
static void LegacyFontStrings(
                        const char *userStr,
                        char **fntstr,
                        char **fntsetstr) ;
static void AddBitmapBlock(
                        FxBuf *b,
                        const char *sysStr,
                        const char *userStr) ;
static void AddFaceBlock(
                        FxBuf *b,
                        const char *tag,
                        const FxFace *face,
                        const char *fallback) ;
static const char *LexEntry(
                        const char *p,
                        const char *end,
                        FxEntry *e) ;
static Boolean TagIs(
                        const FxEntry *e,
                        const char *tag) ;
static Boolean ValueIs(
                        const FxEntry *e,
                        const char *value) ;
static unsigned OwnedGroup(
                        const FxEntry *e) ;
static Boolean HasTag(
                        const char *db,
                        const char *tag,
                        size_t tagLen) ;
static Boolean LastValue(
                        const char *db,
                        const char *tag,
                        char *out,
                        size_t outSize) ;
static int ParseBool(
                        const char *v) ;
static int ParseNamed(
                        const char *v,
                        const char **names,
                        int count) ;
static int ParseHintStyle(
                        const char *v) ;
static int ParseRgba(
                        const char *v) ;
static int ParseLcdFilter(
                        const char *v) ;
static int ParseDpi(
                        const char *v) ;
static void ReadFace(
                        const char *rm,
                        const char *tag,
                        FxFace *face) ;
static Boolean StyleInFamily(
                        const FxFamily *fam,
                        const char *style) ;
static void NormalizeStyle(
                        const FxFamily *fam,
                        FxFace *face) ;
static void ReadOption(
                        const char *rm,
                        const char *tag,
                        int (*parse)(const char *),
                        Boolean xft,
                        int *field) ;
static void AddFcEdit(
                        FxBuf *b,
                        const char *name,
                        const char *type,
                        const char *value) ;
static Boolean IsOurs(
                        const char *path) ;
static Boolean MakeDirs(
                        char *path) ;
static Boolean WriteAll(
                        int fd,
                        const char *s,
                        size_t n) ;
static void WriteLine(
                        int fd,
                        const char *key,
                        const char *value) ;
static Boolean GetRestored(
                        XrmDatabase db,
                        const char *key,
                        char *out,
                        size_t outSize) ;
#ifdef DTSTYLE_HAVE_XFT
static const char *LangString(
                        FcPattern *p,
                        const char *object,
                        const char *langObject) ;
static int CompareFonts(
                        const void *a,
                        const void *b) ;
static void SetFcOption(
                        FcPattern *p,
                        const char *object,
                        Boolean isBool,
                        int value,
                        int builtin,
                        Boolean force) ;
#endif


/*+++++++++++++++++++++++++++++++++++++++*/
/* Small helpers                         */
/*+++++++++++++++++++++++++++++++++++++++*/

static void
BufAdd(
        FxBuf *b,
        const char *s,
        size_t n )
{
    if (b->len + n + 1 > b->size)
    {
        size_t size = b->size ? b->size : 1024;

        while (b->len + n + 1 > size)
            size *= 2;
        b->str = XtRealloc(b->str, size);
        b->size = size;
    }
    memcpy(b->str + b->len, s, n);
    b->len += n;
    b->str[b->len] = '\0';
}

static void
BufStr(
        FxBuf *b,
        const char *s )
{
    BufAdd(b, s, strlen(s));
}

/* "tag: value\n" */
static void
BufLine(
        FxBuf *b,
        const char *tag,
        const char *value )
{
    BufStr(b, tag);
    BufAdd(b, ": ", 2);
    BufStr(b, value);
    BufAdd(b, "\n", 1);
}

static void
BufIntLine(
        FxBuf *b,
        const char *tag,
        int value )
{
    char num[32];

    snprintf(num, sizeof(num), "%d", value);
    BufLine(b, tag, num);
}

/* Copy a name into an FX_NAME_MAX array, truncating it */
static void
CopyName(
        char *dst,
        const char *src )
{
    size_t n;

    if (src == NULL)
        src = "";
    n = strnlen(src, FX_NAME_MAX - 1);
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static int
ClampSize(
        int size )
{
    if (size <= 0)
        return FX_SIZE_DEFAULT;
    if (size < FX_SIZE_MIN)
        return FX_SIZE_MIN;
    if (size > FX_SIZE_MAX)
        return FX_SIZE_MAX;
    return size;
}

/* 0 (automatic) stays 0 */
static int
ClampDpi(
        int dpi )
{
    if (dpi <= 0)
        return 0;
    if (dpi < FX_DPI_MIN)
        return FX_DPI_MIN;
    if (dpi > FX_DPI_MAX)
        return FX_DPI_MAX;
    return dpi;
}

/* An option index outside 0..count-1 means Default */
static int
ClampIndex(
        int value,
        int count )
{
    return (value < 0 || value >= count) ? 0 : value;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* FxNameIsSafe                          */
/* A family or style name is written to  */
/* RESOURCE_MANAGER and to dt.resources, */
/* which xrdb runs through cpp at login. */
/* Refuse names cpp would change, and    */
/* names that would break the line.      */
/*+++++++++++++++++++++++++++++++++++++++*/
Boolean
FxNameIsSafe(
        const char *name )
{
    const unsigned char *p;
    size_t len;
    int i;

    if (name == NULL || name[0] == '\0')
        return False;
    len = strlen(name);
    if (len >= FX_NAME_MAX)
        return False;
    if (name[0] == ' ' || name[0] == '\t' ||
        name[len - 1] == ' ' || name[len - 1] == '\t' ||
        name[0] == '!' || name[0] == '#')
        return False;
    /* cpp at login: comments, and runs of blanks collapsed to one */
    if (strstr(name, "//") || strstr(name, "/*") ||
        strstr(name, "  "))
        return False;

    for (p = (const unsigned char *)name; *p; p++)
        if (*p < 0x20 || *p == 0x7f || *p == '\\')
            return False;

    /* identifier tokens */
    p = (const unsigned char *)name;
    while (*p)
    {
        const unsigned char *t;
        size_t n;

        if (!(isascii(*p) && (isalpha(*p) || *p == '_')))
        {
            /* skip a number or any other byte; a token cannot start
             * inside a run of identifier characters */
            if (isascii(*p) && isdigit(*p))
                while (*p && isascii(*p) && (isalnum(*p) || *p == '_'))
                    p++;
            else
                p++;
            continue;
        }
        t = p;
        while (*p && isascii(*p) && (isalnum(*p) || *p == '_'))
            p++;
        n = p - t;

        /* cpp predefines identifiers with a leading underscore
         * (_LP64, __GNUC__, ...) */
        if (t[0] == '_')
            return False;
        for (i = 0; i < FX_NUM(fxCppWords); i++)
            if (strlen(fxCppWords[i]) == n &&
                strncmp((const char *)t, fxCppWords[i], n) == 0)
                return False;
        for (i = 0; i < FX_NUM(fxCppPrefixes); i++)
        {
            size_t k = strlen(fxCppPrefixes[i]);

            if (n >= k && strncmp((const char *)t, fxCppPrefixes[i], k) == 0)
                return False;
        }
    }
    return True;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* FxBuildResources                      */
/*+++++++++++++++++++++++++++++++++++++++*/

/*
 * *Font and *FontSet from the user font: exactly the algorithm of the
 * old Font.c ButtonCB, so Bitmap mode writes what it always wrote.
 */
static void
LegacyFontStrings(
        const char *userStr,
        char **fntstr,
        char **fntsetstr )
{
    size_t len;
    int    n;
    char  *str, *fs, *f;

    /* for *FontSet: the first font entry, delimited by a ":" or an "=" */
    len = strcspn(userStr, ":=");
    fs = XtCalloc(1, len + 4);          /* room for the "*-*" below */
    memcpy(fs, userStr, len);

    /* a font list of several fonts becomes Xt font set syntax */
    str = strchr(fs, ';');
    while (str)
    {
        *str = ',';
        str = strchr(str, ';');
    }

    /* for *Font: the first font, delimited by a comma, a colon or an = */
    len = strcspn(fs, ",:=");
    f = XtCalloc(1, len + 1);
    memcpy(f, userStr, len);

    /* for *FontSet: a single font (not a font set) gets its charset
     * fields wildcarded */
    len = strlen(fs);
    if (userStr[len] != ':')
    {
        str = strchr(fs, '-');
        for (n = 1; n < 13 && str; n++)
            str = strchr(str + 1, '-');
        if (str)
            strcpy(str + 1, "*-*");
    }

    *fntstr = f;
    *fntsetstr = fs;
}

static void
AddBitmapBlock(
        FxBuf *b,
        const char *sysStr,
        const char *userStr )
{
    char *fntstr, *fntsetstr;

    LegacyFontStrings(userStr, &fntstr, &fntsetstr);
    BufLine(b, "*systemFont", sysStr);
    BufLine(b, "*userFont", userStr);
    BufLine(b, "*FontList", sysStr);
    BufLine(b, "*buttonFontList", sysStr);
    BufLine(b, "*labelFontList", sysStr);
    BufLine(b, "*textFontList", userStr);
    BufLine(b, "*XmText*FontList", userStr);
    BufLine(b, "*XmTextField*FontList", userStr);
    BufLine(b, "*DtEditor*textFontList", userStr);
    BufLine(b, "*Font", fntstr);
    BufLine(b, "*FontSet", fntsetstr);
    XtFree(fntstr);
    XtFree(fntsetstr);
}

/* *<tag>.fontType/fontName/fontStyle/fontSize.  A name that is not safe
 * (it can only come from a hand-edited database) is not written. */
static void
AddFaceBlock(
        FxBuf *b,
        const char *tag,
        const FxFace *face,
        const char *fallback )
{
    char res[64];

    snprintf(res, sizeof(res), "*%s.fontType", tag);
    BufLine(b, res, "FONT_IS_XFT");
    snprintf(res, sizeof(res), "*%s.fontName", tag);
    BufLine(b, res, FxNameIsSafe(face->family) ? face->family : fallback);
    if (face->style[0] && FxNameIsSafe(face->style))
    {
        snprintf(res, sizeof(res), "*%s.fontStyle", tag);
        BufLine(b, res, face->style);
    }
    snprintf(res, sizeof(res), "*%s.fontSize", tag);
    BufIntLine(b, res, ClampSize(face->size));
}

char *
FxBuildResources(
        const FxSettings *s,
        const FxBitmapChoice *b )
{
    FxBuf buf = { NULL, 0, 0 };
    int   i;

    BufAdd(&buf, "", 0);

    if (b && b->sysStr && b->userStr)
        AddBitmapBlock(&buf, b->sysStr, b->userStr);

    if (s && s->mode == FX_MODE_XFT)
    {
        for (i = 0; i < FX_NUM(fxRenderLines); i++)
            if (fxRenderLines[i].face != NULL)
                BufLine(&buf, fxRenderLines[i].resource, fxRenderLines[i].face);
        AddFaceBlock(&buf, FX_TAG_SYSTEM, &s->sys, "sans-serif");
        AddFaceBlock(&buf, FX_TAG_USER, &s->user, "monospace");

        if ((i = ClampIndex(s->antialias, FX_NUM(fxBoolNames))))
            BufLine(&buf, "Xft.antialias", fxBoolNames[i]);
        if ((i = ClampIndex(s->hinting, FX_NUM(fxBoolNames))))
            BufLine(&buf, "Xft.hinting", fxBoolNames[i]);
        if ((i = ClampIndex(s->hintStyle, FX_NUM(fxHsNames))))
            BufLine(&buf, "Xft.hintstyle", fxHsNames[i]);
        if ((i = ClampIndex(s->rgba, FX_NUM(fxRgbaNames))))
            BufLine(&buf, "Xft.rgba", fxRgbaNames[i]);
        if ((i = ClampIndex(s->lcdFilter, FX_NUM(fxLcdNames))))
            BufLine(&buf, "Xft.lcdfilter", fxLcdNames[i]);
        if ((i = ClampDpi(s->dpi)))
            BufIntLine(&buf, "Xft.dpi", i);
    }
    return buf.str;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* Resource string lexer                 */
/*+++++++++++++++++++++++++++++++++++++++*/

/*
 * Reads the entry at p and returns the start of the next one.  The rules
 * are those of _DtGetEntries (lib/DtSvc/DtUtil2/addToRes.c): an entry ends
 * at the first newline not escaped by a backslash (multibyte characters
 * are skipped whole, as _DtFindFirst does), and the tag is the text before
 * the first unescaped colon.  An entry whose first non-blank character is
 * '!' or '#' is a comment and has no tag.
 */
static const char *
LexEntry(
        const char *p,
        const char *end,
        FxEntry *e )
{
    const char *q = p, *colon = NULL, *t, *te, *v, *ve;
    int n;

    e->start = p;
    e->tag = e->value = NULL;
    e->tagLen = e->valueLen = 0;

    mblen(NULL, 0);
    while (q < end && *q != '\n')
    {
        if ((n = mblen(q, end - q)) > 1)
        {
            q += n;
            continue;
        }
        if (*q == '\\')
        {
            q += (q + 1 < end) ? 2 : 1;
            continue;
        }
        if (*q == ':' && colon == NULL)
            colon = q;
        q++;
    }
    ve = q;                             /* end of the value */
    if (q < end)
        q++;                            /* the entry includes its newline */
    e->next = q;

    for (t = p; t < ve && (*t == ' ' || *t == '\t'); t++)
        ;
    if (colon == NULL || *t == '!' || *t == '#')
        return q;

    for (te = colon; te > t && (te[-1] == ' ' || te[-1] == '\t'); te--)
        ;
    for (v = colon + 1; v < ve && (*v == ' ' || *v == '\t'); v++)
        ;
    while (ve > v && (ve[-1] == ' ' || ve[-1] == '\t'))
        ve--;

    e->tag = t;
    e->tagLen = te - t;
    e->value = v;
    e->valueLen = ve - v;
    return q;
}

static Boolean
TagIs(
        const FxEntry *e,
        const char *tag )
{
    size_t n = strlen(tag);

    return e->tag && e->tagLen == n && strncmp(e->tag, tag, n) == 0;
}

static Boolean
ValueIs(
        const FxEntry *e,
        const char *value )
{
    size_t n = strlen(value);

    return e->value && e->valueLen == n && strncmp(e->value, value, n) == 0;
}

/* The FX_OWN_* group of an entry, 0 if it is not ours */
static unsigned
OwnedGroup(
        const FxEntry *e )
{
    int i;

    if (e->tag == NULL)
        return 0;

    for (i = 0; i < FX_NUM(fxRenderLines); i++)
        if (TagIs(e, fxRenderLines[i].resource))
            return (ValueIs(e, FX_TAG_SYSTEM) || ValueIs(e, FX_TAG_USER))
                   ? FX_OWN_RENDER : 0;

    for (i = 0; i < FX_NUM(fxRenderPrefixes); i++)
    {
        size_t n = strlen(fxRenderPrefixes[i]);

        if (e->tagLen >= n && strncmp(e->tag, fxRenderPrefixes[i], n) == 0)
            return FX_OWN_RENDER;
    }

    for (i = 0; i < FX_NUM(fxXftTags); i++)
        if (TagIs(e, fxXftTags[i]))
            return FX_OWN_XFTOPT;

    return 0;
}

/* Does db have a (non-comment) entry with exactly this tag? */
static Boolean
HasTag(
        const char *db,
        const char *tag,
        size_t tagLen )
{
    const char *p = db, *end;
    FxEntry e;

    if (db == NULL)
        return False;
    end = db + strlen(db);
    while (p < end)
    {
        p = LexEntry(p, end, &e);
        if (e.tag && e.tagLen == tagLen && strncmp(e.tag, tag, tagLen) == 0)
            return True;
    }
    return False;
}

/* The value of the last entry with this tag, as _DtGetEntries keeps it.
 * False if there is none or it does not fit. */
static Boolean
LastValue(
        const char *db,
        const char *tag,
        char *out,
        size_t outSize )
{
    const char *p = db, *end;
    FxEntry e, last;
    Boolean found = False;

    if (db == NULL)
        return False;
    end = db + strlen(db);
    while (p < end)
    {
        p = LexEntry(p, end, &e);
        if (TagIs(&e, tag))
        {
            last = e;
            found = True;
        }
    }
    if (!found || last.valueLen >= outSize)
        return False;
    memcpy(out, last.value, last.valueLen);
    out[last.valueLen] = '\0';
    return True;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* FxFilterOwned                         */
/* Copies `in' without the entries of    */
/* the FX_OWN_* groups in `groups'.  An  */
/* FX_OWN_XFTOPT entry is dropped only   */
/* if onlyIfIn is NULL or has an entry   */
/* with the same tag.                    */
/*+++++++++++++++++++++++++++++++++++++++*/
char *
FxFilterOwned(
        const char *in,
        unsigned groups,
        const char *onlyIfIn,
        int *removed )
{
    const char *p, *end;
    char *out, *o;
    FxEntry e;
    unsigned g;
    int count = 0;

    if (in == NULL)
    {
        if (removed)
            *removed = 0;
        return XtNewString("");
    }

    end = in + strlen(in);
    out = o = XtMalloc(end - in + 1);
    for (p = in; p < end; )
    {
        p = LexEntry(p, end, &e);
        g = OwnedGroup(&e) & groups;
        if (g == FX_OWN_XFTOPT && onlyIfIn &&
            !HasTag(onlyIfIn, e.tag, e.tagLen))
            g = 0;
        if (g)
        {
            count++;
            continue;
        }
        memcpy(o, e.start, e.next - e.start);
        o += e.next - e.start;
    }
    *o = '\0';

    if (removed)
        *removed = count;
    return out;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* FxCommit                              */
/* Removes the lines of the other mode   */
/* and merges the new set.  *fontInfo    */
/* gets the new set for SmNewFontSettings*/
/* (XtFree it).  Returns False if the    */
/* fontconfig file could not be written. */
/*+++++++++++++++++++++++++++++++++++++++*/
Boolean
FxCommit(
        Display *dpy,
        const FxSettings *s,
        const FxBitmapChoice *b,
        Boolean immediate,
        char **fontInfo )
{
    char    *newRes, *prefs, *rm, *filtered;
    unsigned groups = FX_OWN_RENDER | FX_OWN_XFTOPT;
    Window   root = RootWindow(dpy, 0);     /* the one _DtAddToResource uses */
    int      removed;
    Boolean  ok;

    newRes = FxBuildResources(s, b);

    XGrabServer(dpy);

    /* _DT_SM_PREFERENCES is cleaned even when the resources are written
     * only at the next login: dtsession merges the new set into it and
     * writes it to dt.resources. */
    prefs = _DtGetResString(dpy, _DT_ATR_PREFS);
    filtered = FxFilterOwned(prefs, groups, NULL, &removed);
    if (removed > 0)
        XChangeProperty(dpy, root, XInternAtom(dpy, "_DT_SM_PREFERENCES", False),
                        XA_STRING, 8, PropModeReplace,
                        (unsigned char *)filtered, strlen(filtered));
    XtFree(filtered);

    if (immediate)
    {
        /* An Xft.* line is ours only if _DT_SM_PREFERENCES had it; the
         * others come from .Xdefaults and stay. */
        rm = _DtGetResString(dpy, _DT_ATR_RESMGR);
        filtered = FxFilterOwned(rm, groups, prefs ? prefs : "", &removed);
        if (removed > 0)
            XChangeProperty(dpy, root, XA_RESOURCE_MANAGER,
                            XA_STRING, 8, PropModeReplace,
                            (unsigned char *)filtered, strlen(filtered));
        XtFree(filtered);
        if (rm)
            XFree(rm);

        if (newRes[0])
            _DtAddToResource(dpy, newRes);
    }

    XUngrabServer(dpy);
    XFlush(dpy);
    if (prefs)
        XFree(prefs);

    ok = FxUpdateFcFile(s);

    if (fontInfo)
        *fontInfo = newRes;
    else
        XtFree(newRes);
    return ok;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* Read-back of the live resources       */
/*+++++++++++++++++++++++++++++++++++++++*/

/* true/false/on/off/yes/no/1/0; -1 if none of them */
static int
ParseBool(
        const char *v )
{
    if (!strcasecmp(v, "true") || !strcasecmp(v, "on") ||
        !strcasecmp(v, "yes") || !strcmp(v, "1"))
        return FX_TRI_ON;
    if (!strcasecmp(v, "false") || !strcasecmp(v, "off") ||
        !strcasecmp(v, "no") || !strcmp(v, "0"))
        return FX_TRI_OFF;
    return -1;
}

/* The index of v in names[1..count-1] (case-insensitive); 0 if none */
static int
ParseNamed(
        const char *v,
        const char **names,
        int count )
{
    int i;

    for (i = 1; i < count; i++)
        if (names[i] && !strcasecmp(v, names[i]))
            return i;
    return 0;
}

/* hintnone..hintfull or fontconfig's 0..3 */
static int
ParseHintStyle(
        const char *v )
{
    if (v[0] >= '0' && v[0] <= '3' && v[1] == '\0')
        return FX_HS_NONE + (v[0] - '0');
    return ParseNamed(v, fxHsNames, FX_NUM(fxHsNames));
}

/* none/rgb/bgr/vrgb/vbgr or fontconfig's 1..5 (0, unknown: Default) */
static int
ParseRgba(
        const char *v )
{
    static const int fromFc[] = { FX_RGBA_DEFAULT, FX_RGBA_RGB, FX_RGBA_BGR,
                                  FX_RGBA_VRGB, FX_RGBA_VBGR, FX_RGBA_NONE };

    if (v[0] >= '0' && v[0] <= '5' && v[1] == '\0')
        return fromFc[v[0] - '0'];
    return ParseNamed(v, fxRgbaNames, FX_NUM(fxRgbaNames));
}

/* lcdnone..lcdlegacy or fontconfig's 0..3 */
static int
ParseLcdFilter(
        const char *v )
{
    if (v[0] >= '0' && v[0] <= '3' && v[1] == '\0')
        return FX_LCD_NONE + (v[0] - '0');
    return ParseNamed(v, fxLcdNames, FX_NUM(fxLcdNames));
}

/* A number, rounded; 0 (automatic) if it is not one or out of range */
static int
ParseDpi(
        const char *v )
{
    char  *end;
    double d = strtod(v, &end);

    /* written so that NaN is out of range too */
    if (end == v || !(d >= FX_DPI_MIN - 0.5 && d < FX_DPI_MAX + 0.5))
        return 0;
    return (int)(d + 0.5);
}

/* *<tag>.fontName/fontStyle/fontSize into face; a missing or unusable
 * name keeps the face as it is */
static void
ReadFace(
        const char *rm,
        const char *tag,
        FxFace *face )
{
    char res[64], val[FX_NAME_MAX];
    int  size;

    snprintf(res, sizeof(res), "*%s.fontName", tag);
    if (LastValue(rm, res, val, sizeof(val)) && FxNameIsSafe(val))
    {
        CopyName(face->family, val);
        snprintf(res, sizeof(res), "*%s.fontStyle", tag);
        if (LastValue(rm, res, val, sizeof(val)) && FxNameIsSafe(val))
            CopyName(face->style, val);
        else
            face->style[0] = '\0';
    }
    snprintf(res, sizeof(res), "*%s.fontSize", tag);
    if (LastValue(rm, res, val, sizeof(val)))
    {
        size = atoi(val);
        if (size >= FX_SIZE_MIN && size <= FX_SIZE_MAX)
            face->size = size;
    }
}

/* An Xft.* option; Default (0) if its value is not understood.  Without
 * the line the option becomes Default only in Xft mode. */
static void
ReadOption(
        const char *rm,
        const char *tag,
        int (*parse)(const char *),
        Boolean xft,
        int *field )
{
    char val[FX_NAME_MAX];
    int  v;

    if (LastValue(rm, tag, val, sizeof(val)))
        *field = (v = parse(val)) < 0 ? 0 : v;
    else if (xft)
        *field = 0;
}

void
FxReadCurrent(
        Display *dpy,
        FxSettings *inout )
{
    char   *rm;
    char    val[FX_NAME_MAX];
    Boolean xft;

    rm = _DtGetResString(dpy, _DT_ATR_RESMGR);

    xft = LastValue(rm, "*renderTable", val, sizeof(val)) &&
          strcmp(val, FX_TAG_SYSTEM) == 0 &&
          HasTag(rm, "*" FX_TAG_SYSTEM ".fontType",
                 strlen("*" FX_TAG_SYSTEM ".fontType"));
    inout->mode = xft ? FX_MODE_XFT : FX_MODE_BITMAP;

    if (xft)
    {
        ReadFace(rm, FX_TAG_SYSTEM, &inout->sys);
        ReadFace(rm, FX_TAG_USER, &inout->user);
    }

    /* In Xft mode a missing Xft.* line means Default; in Bitmap mode it
     * keeps the value restored from the session file. */
    ReadOption(rm, "Xft.antialias", ParseBool, xft, &inout->antialias);
    ReadOption(rm, "Xft.hinting", ParseBool, xft, &inout->hinting);
    ReadOption(rm, "Xft.hintstyle", ParseHintStyle, xft, &inout->hintStyle);
    ReadOption(rm, "Xft.rgba", ParseRgba, xft, &inout->rgba);
    ReadOption(rm, "Xft.lcdfilter", ParseLcdFilter, xft, &inout->lcdFilter);
    ReadOption(rm, "Xft.dpi", ParseDpi, xft, &inout->dpi);

    if (xft && FxFcFileIsOurs())
        inout->fcOverride = True;

    if (rm)
        XFree(rm);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* FxGetDefaults                         */
/*+++++++++++++++++++++++++++++++++++++++*/
void
FxGetDefaults(
        Widget shell,
        FxSettings *out )
{
    FxAppRes res;

    res.sysFamily  = "sans-serif";
    res.sysStyle   = "Regular";
    res.sysSize    = FX_SIZE_DEFAULT;
    res.userFamily = "monospace";
    res.userStyle  = "Regular";
    res.userSize   = FX_SIZE_DEFAULT;
    if (shell)
        XtGetApplicationResources(shell, &res, fxAppResources,
                                  XtNumber(fxAppResources), NULL, 0);

    memset(out, 0, sizeof(*out));
    out->mode = FX_MODE_BITMAP;
    CopyName(out->sys.family, res.sysFamily);
    CopyName(out->sys.style, res.sysStyle);
    out->sys.size = ClampSize(res.sysSize);
    CopyName(out->user.family, res.userFamily);
    CopyName(out->user.style, res.userStyle);
    out->user.size = ClampSize(res.userSize);
    out->userMonoOnly = True;
    out->antialias = FX_TRI_DEFAULT;
    out->hinting = FX_TRI_DEFAULT;
    out->hintStyle = FX_HS_DEFAULT;
    out->rgba = FX_RGBA_DEFAULT;
    out->lcdFilter = FX_LCD_DEFAULT;
    out->dpi = 0;
    out->fcOverride = False;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* FxApplyRestored                       */
/*+++++++++++++++++++++++++++++++++++++++*/
void
FxApplyRestored(
        FxSettings *dst,
        const FxSettings *restored,
        unsigned mask )
{
    if (mask & FX_HAVE_FACES)
    {
        dst->sys = restored->sys;
        dst->user = restored->user;
        dst->userMonoOnly = restored->userMonoOnly;
    }
    if (mask & FX_HAVE_RENDERING)
    {
        dst->antialias = restored->antialias;
        dst->hinting = restored->hinting;
        dst->hintStyle = restored->hintStyle;
        dst->rgba = restored->rgba;
        dst->lcdFilter = restored->lcdFilter;
        dst->dpi = restored->dpi;
        dst->fcOverride = restored->fcOverride;
    }
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* Family list                           */
/*+++++++++++++++++++++++++++++++++++++++*/

#ifdef DTSTYLE_HAVE_XFT

/* One (family, style) of the font list */
typedef struct {
    const char *family;
    const char *style;
    int         weight, slant, width;
    Boolean     mono;
} FxFont;

/* The value of object at the index where langObject is "en", else the
 * first one */
static const char *
LangString(
        FcPattern *p,
        const char *object,
        const char *langObject )
{
    FcChar8 *s = NULL, *lang;
    int i;

    for (i = 0; FcPatternGetString(p, langObject, i, &lang) == FcResultMatch;
         i++)
        if (strcmp((const char *)lang, "en") == 0 &&
            FcPatternGetString(p, object, i, &s) == FcResultMatch)
            return (const char *)s;
    if (FcPatternGetString(p, object, 0, &s) == FcResultMatch)
        return (const char *)s;
    return NULL;
}

static int
CompareFonts(
        const void *a,
        const void *b )
{
    const FxFont *x = a, *y = b;
    int c;

    if ((c = strcasecmp(x->family, y->family)) != 0)
        return c;
    if ((c = strcmp(x->family, y->family)) != 0)
        return c;
    if (x->weight != y->weight)
        return x->weight - y->weight;
    if (x->slant != y->slant)
        return x->slant - y->slant;
    if (x->width != y->width)
        return x->width - y->width;
    return strcmp(x->style, y->style);
}

#endif /* DTSTYLE_HAVE_XFT */

/*
 * The scalable outline families fontconfig knows, sorted, with their
 * styles; NULL if there are none (or no fontconfig support).
 */
FxFamilyList *
FxListFamilies(void)
{
#ifdef DTSTYLE_HAVE_XFT
    FcPattern    *pat;
    FcObjectSet  *os;
    FcFontSet    *fs;
    FxFont       *fonts;
    FxFamilyList *list;
    FxFamily     *fam = NULL;
    int           i, j, n = 0, spacing;
    FcBool        b;

    if (!FcInit())
        return NULL;
    pat = FcPatternBuild(NULL, FC_OUTLINE, FcTypeBool, FcTrue,
                         FC_SCALABLE, FcTypeBool, FcTrue, (char *)NULL);
    os = FcObjectSetBuild(FC_FAMILY, FC_FAMILYLANG, FC_STYLE, FC_STYLELANG,
                          FC_SPACING, FC_WEIGHT, FC_SLANT, FC_WIDTH,
#ifdef FC_VARIABLE
                          FC_VARIABLE,
#endif
                          (char *)NULL);
    fs = (pat && os) ? FcFontList(NULL, pat, os) : NULL;
    if (pat)
        FcPatternDestroy(pat);
    if (os)
        FcObjectSetDestroy(os);
    if (fs == NULL)
        return NULL;

    fonts = (FxFont *)XtMalloc((fs->nfont + 1) * sizeof(FxFont));
    for (i = 0; i < fs->nfont; i++)
    {
        FcPattern *p = fs->fonts[i];
        FxFont *f = &fonts[n];

#ifdef FC_VARIABLE
        if (FcPatternGetBool(p, FC_VARIABLE, 0, &b) == FcResultMatch && b)
            continue;
#else
        (void)b;
#endif
        f->family = LangString(p, FC_FAMILY, FC_FAMILYLANG);
        f->style = LangString(p, FC_STYLE, FC_STYLELANG);
        if (!FxNameIsSafe(f->family) || !FxNameIsSafe(f->style))
            continue;
        if (FcPatternGetInteger(p, FC_WEIGHT, 0, &f->weight) != FcResultMatch)
            f->weight = FC_WEIGHT_REGULAR;
        if (FcPatternGetInteger(p, FC_SLANT, 0, &f->slant) != FcResultMatch)
            f->slant = FC_SLANT_ROMAN;
        if (FcPatternGetInteger(p, FC_WIDTH, 0, &f->width) != FcResultMatch)
            f->width = FC_WIDTH_NORMAL;
        f->mono = FcPatternGetInteger(p, FC_SPACING, 0, &spacing)
                  == FcResultMatch && spacing >= FC_MONO;
        n++;
    }

    if (n == 0)
    {
        XtFree((char *)fonts);
        FcFontSetDestroy(fs);
        return NULL;
    }
    qsort(fonts, n, sizeof(FxFont), CompareFonts);

    list = (FxFamilyList *)XtMalloc(sizeof(FxFamilyList));
    list->numFamilies = 0;
    list->families = (FxFamily *)XtMalloc(n * sizeof(FxFamily));
    for (i = 0; i < n; i++)
    {
        if (fam == NULL || strcmp(fam->family, fonts[i].family) != 0)
        {
            fam = &list->families[list->numFamilies++];
            fam->family = XtNewString(fonts[i].family);
            fam->mono = False;
            fam->numStyles = 0;
            fam->styles = (char **)XtMalloc(n * sizeof(char *));
        }
        if (fonts[i].mono)
            fam->mono = True;
        for (j = 0; j < fam->numStyles; j++)
            if (strcmp(fam->styles[j], fonts[i].style) == 0)
                break;
        if (j == fam->numStyles)
            fam->styles[fam->numStyles++] = XtNewString(fonts[i].style);
    }
    for (i = 0; i < list->numFamilies; i++)
        list->families[i].styles = (char **)XtRealloc(
            (char *)list->families[i].styles,
            list->families[i].numStyles * sizeof(char *));

    XtFree((char *)fonts);
    FcFontSetDestroy(fs);
    return list;
#else
    return NULL;
#endif /* DTSTYLE_HAVE_XFT */
}

void
FxFreeFamilyList(
        FxFamilyList *list )
{
    int i, j;

    if (list == NULL)
        return;
    for (i = 0; i < list->numFamilies; i++)
    {
        for (j = 0; j < list->families[i].numStyles; j++)
            XtFree(list->families[i].styles[j]);
        XtFree((char *)list->families[i].styles);
        XtFree(list->families[i].family);
    }
    XtFree((char *)list->families);
    XtFree((char *)list);
}

const FxFamily *
FxFindFamily(
        const FxFamilyList *list,
        const char *family )
{
    int i;

    if (list == NULL || family == NULL)
        return NULL;
    for (i = 0; i < list->numFamilies; i++)
        if (strcmp(list->families[i].family, family) == 0)
            return &list->families[i];
    return NULL;
}

/*
 * A listed family for name: name itself, or what fontconfig substitutes
 * for it (a generic name like "monospace", or an alias), or the first
 * (monospaced) family.  NULL only for an empty list.
 */
const char *
FxResolveFamily(
        const FxFamilyList *list,
        const char *name,
        Boolean monoOnly )
{
    int i;

    if (list == NULL || list->numFamilies == 0)
        return NULL;
    if (name == NULL)
        name = "";

    for (i = 0; i < list->numFamilies; i++)
        if (strcmp(list->families[i].family, name) == 0 &&
            (!monoOnly || list->families[i].mono))
            return list->families[i].family;
    for (i = 0; i < list->numFamilies; i++)
        if (strcasecmp(list->families[i].family, name) == 0 &&
            (!monoOnly || list->families[i].mono))
            return list->families[i].family;

#ifdef DTSTYLE_HAVE_XFT
    if (name[0] && FcInit())
    {
        FcPattern *p = FcPatternCreate();
        FcFontSet *fs = NULL;
        FcResult   res;
        const char *found = NULL;

        if (p)
        {
            FcPatternAddString(p, FC_FAMILY, (const FcChar8 *)name);
            FcPatternAddBool(p, FC_OUTLINE, FcTrue);
            FcConfigSubstitute(NULL, p, FcMatchPattern);
            FcDefaultSubstitute(p);
            fs = FcFontSort(NULL, p, FcTrue, NULL, &res);
            FcPatternDestroy(p);
        }
        for (i = 0; fs && i < fs->nfont && found == NULL; i++)
        {
            const FxFamily *fam = FxFindFamily(list,
                LangString(fs->fonts[i], FC_FAMILY, FC_FAMILYLANG));

            if (fam && (!monoOnly || fam->mono))
                found = fam->family;
        }
        if (fs)
            FcFontSetDestroy(fs);
        if (found)
            return found;
    }
#endif /* DTSTYLE_HAVE_XFT */

    if (monoOnly)
        for (i = 0; i < list->numFamilies; i++)
            if (list->families[i].mono)
                return list->families[i].family;
    return list->families[0].family;
}

const char *
FxDefaultStyle(
        const FxFamily *fam )
{
    static const char *preferred[] = {
        "Regular", "Book", "Roman", "Normal", "Medium"
    };
    int i;

    if (fam == NULL || fam->numStyles == 0)
        return "";
    for (i = 0; i < FX_NUM(preferred); i++)
        if (StyleInFamily(fam, preferred[i]))
            return preferred[i];
    return fam->styles[0];
}

static Boolean
StyleInFamily(
        const FxFamily *fam,
        const char *style )
{
    int i;

    for (i = 0; fam && i < fam->numStyles; i++)
        if (strcmp(fam->styles[i], style) == 0)
            return True;
    return False;
}

static void
NormalizeStyle(
        const FxFamily *fam,
        FxFace *face )
{
    if (fam && !StyleInFamily(fam, face->style))
        CopyName(face->style, FxDefaultStyle(fam));
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* FxNormalize                           */
/* Makes the settings fit the list: the  */
/* families and styles become listed     */
/* ones, sizes and options valid ones.   */
/*+++++++++++++++++++++++++++++++++++++++*/
void
FxNormalize(
        const FxFamilyList *list,
        FxSettings *s )
{
    const FxFamily *fam;

    s->sys.size = ClampSize(s->sys.size);
    s->user.size = ClampSize(s->user.size);
    s->dpi = ClampDpi(s->dpi);
    s->antialias = ClampIndex(s->antialias, FX_NUM(fxBoolNames));
    s->hinting = ClampIndex(s->hinting, FX_NUM(fxBoolNames));
    s->hintStyle = ClampIndex(s->hintStyle, FX_NUM(fxHsNames));
    s->rgba = ClampIndex(s->rgba, FX_NUM(fxRgbaNames));
    s->lcdFilter = ClampIndex(s->lcdFilter, FX_NUM(fxLcdNames));

    if (list == NULL || list->numFamilies == 0)
        return;

    if ((fam = FxFindFamily(list, s->sys.family)) == NULL)
    {
        CopyName(s->sys.family, FxResolveFamily(list, s->sys.family, False));
        fam = FxFindFamily(list, s->sys.family);
    }
    NormalizeStyle(fam, &s->sys);

    if ((fam = FxFindFamily(list, s->user.family)) == NULL)
    {
        CopyName(s->user.family,
                 FxResolveFamily(list, s->user.family, s->userMonoOnly));
        fam = FxFindFamily(list, s->user.family);
    }
    /* a proportional user font (chosen by hand, or no monospaced family
     * installed) turns "Monospace families only" off */
    if (fam && s->userMonoOnly && !fam->mono)
        s->userMonoOnly = False;
    NormalizeStyle(fam, &s->user);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* FxSettingsEqual                       */
/* Only what would be written counts:    */
/* in Bitmap mode nothing of the Xft     */
/* settings, and never userMonoOnly.     */
/*+++++++++++++++++++++++++++++++++++++++*/
Boolean
FxSettingsEqual(
        const FxSettings *a,
        const FxSettings *b )
{
    if (a->mode != b->mode)
        return False;
    if (a->mode == FX_MODE_BITMAP)
        return True;
    return strcmp(a->sys.family, b->sys.family) == 0 &&
           strcmp(a->sys.style, b->sys.style) == 0 &&
           a->sys.size == b->sys.size &&
           strcmp(a->user.family, b->user.family) == 0 &&
           strcmp(a->user.style, b->user.style) == 0 &&
           a->user.size == b->user.size &&
           a->antialias == b->antialias &&
           a->hinting == b->hinting &&
           a->hintStyle == b->hintStyle &&
           a->rgba == b->rgba &&
           a->lcdFilter == b->lcdFilter &&
           a->dpi == b->dpi &&
           a->fcOverride == b->fcOverride;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* FxAutoDpi                             */
/* The resolution Xft uses without an    */
/* Xft.dpi resource.                     */
/*+++++++++++++++++++++++++++++++++++++++*/
int
FxAutoDpi(
        Display *dpy )
{
    int scr = DefaultScreen(dpy);
    int mm = DisplayHeightMM(dpy, scr);

    if (mm <= 0)
        return 96;
    return (int)(DisplayHeight(dpy, scr) * 25.4 / mm + 0.5);
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* Fontconfig override file              */
/*+++++++++++++++++++++++++++++++++++++++*/

/* ${XDG_CONFIG_HOME:-$HOME/.config}/fontconfig/conf.d/90-dtstyle.conf,
 * NULL without a home */
char *
FxFcFilePath(void)
{
    const char *base = getenv("XDG_CONFIG_HOME");
    const char *dir = "";
    char *path;
    size_t len;

    if (base == NULL || base[0] != '/')
    {
        base = getenv("HOME");
        if (base == NULL || base[0] == '\0')
            return NULL;
        dir = "/.config";
    }
    len = strlen(base) + strlen(dir) + strlen("/" FX_FC_FILE) + 1;
    path = XtMalloc(len);
    snprintf(path, len, "%s%s/%s", base, dir, FX_FC_FILE);
    return path;
}

/* The file exists and carries our marker in its first bytes */
static Boolean
IsOurs(
        const char *path )
{
    char    buf[FX_FC_PEEK + 1];
    ssize_t n;
    int     fd, i;
    size_t  m = strlen(FX_FC_MARKER);

    if (path == NULL || (fd = open(path, O_RDONLY)) < 0)
        return False;
    n = read(fd, buf, FX_FC_PEEK);
    close(fd);
    for (i = 0; n > 0 && i + (ssize_t)m <= n; i++)
        if (memcmp(buf + i, FX_FC_MARKER, m) == 0)
            return True;
    return False;
}

Boolean
FxFcFileIsOurs(void)
{
    char   *path = FxFcFilePath();
    Boolean ours = IsOurs(path);

    XtFree(path);
    return ours;
}

/* mkdir -p of the directory part of path (path is restored) */
static Boolean
MakeDirs(
        char *path )
{
    char *p;
    struct stat st;
    Boolean ok = True;

    for (p = strchr(path + 1, '/'); p && ok; p = strchr(p + 1, '/'))
    {
        *p = '\0';
        if (stat(path, &st) == 0)
            ok = S_ISDIR(st.st_mode);
        else
            ok = mkdir(path, 0755) == 0 || errno == EEXIST;
        *p = '/';
    }
    return ok;
}

static Boolean
WriteAll(
        int fd,
        const char *s,
        size_t n )
{
    ssize_t w;

    while (n > 0)
    {
        if ((w = write(fd, s, n)) < 0)
        {
            if (errno == EINTR)
                continue;
            return False;
        }
        s += w;
        n -= w;
    }
    return True;
}

/* <edit name="name" mode="assign"><type>value</type></edit> */
static void
AddFcEdit(
        FxBuf *b,
        const char *name,
        const char *type,
        const char *value )
{
    BufStr(b, "    <edit name=\"");
    BufStr(b, name);
    BufStr(b, "\" mode=\"assign\"><");
    BufStr(b, type);
    BufAdd(b, ">", 1);
    BufStr(b, value);
    BufAdd(b, "</", 2);
    BufStr(b, type);
    BufStr(b, "></edit>\n");
}

/*
 * Writes the fontconfig file when the settings ask for it (Xft mode,
 * Override, at least one option not Default), and removes it otherwise.
 * A file of the same name that we did not write is never touched.
 * Returns False on an I/O error.
 */
Boolean
FxUpdateFcFile(
        const FxSettings *s )
{
    char   *path, *tmp;
    FxBuf   buf = { NULL, 0, 0 };
    Boolean want, ok = True;
    struct stat st;
    size_t  len;
    int     fd, i;

    want = s && s->mode == FX_MODE_XFT && s->fcOverride &&
           (ClampIndex(s->antialias, FX_NUM(fxBoolNames)) ||
            ClampIndex(s->hinting, FX_NUM(fxBoolNames)) ||
            ClampIndex(s->hintStyle, FX_NUM(fxHsNames)) ||
            ClampIndex(s->rgba, FX_NUM(fxRgbaNames)) ||
            ClampIndex(s->lcdFilter, FX_NUM(fxLcdNames)));
    path = FxFcFilePath();

    if (!want)
    {
        if (path && IsOurs(path) && unlink(path) != 0 && errno != ENOENT)
            ok = False;
        XtFree(path);
        return ok;
    }

    if (path == NULL)
        return False;
    if (lstat(path, &st) == 0 && !IsOurs(path))
    {
        XtFree(path);           /* someone else's file */
        return False;
    }

    BufStr(&buf, "<?xml version=\"1.0\"?>\n"
                 "<!DOCTYPE fontconfig SYSTEM \"urn:fontconfig:fonts.dtd\">\n"
                 "<!-- " FX_FC_MARKER " Font dialog (dtstyle)."
                 " It is rewritten or removed when the font settings"
                 " change. -->\n"
                 "<fontconfig>\n"
                 "  <match target=\"pattern\">\n");
    if ((i = ClampIndex(s->antialias, FX_NUM(fxBoolNames))))
        AddFcEdit(&buf, "antialias", "bool", fxBoolNames[i]);
    if ((i = ClampIndex(s->hinting, FX_NUM(fxBoolNames))))
        AddFcEdit(&buf, "hinting", "bool", fxBoolNames[i]);
    if ((i = ClampIndex(s->hintStyle, FX_NUM(fxHsNames))))
        AddFcEdit(&buf, "hintstyle", "const", fxHsNames[i]);
    if ((i = ClampIndex(s->rgba, FX_NUM(fxRgbaNames))))
        AddFcEdit(&buf, "rgba", "const", fxRgbaNames[i]);
    if ((i = ClampIndex(s->lcdFilter, FX_NUM(fxLcdNames))))
        AddFcEdit(&buf, "lcdfilter", "const", fxLcdNames[i]);
    BufStr(&buf, "  </match>\n"
                 "</fontconfig>\n");

    len = strlen(path) + 5;
    tmp = XtMalloc(len);
    snprintf(tmp, len, "%s.tmp", path);

    if (!MakeDirs(path) ||
        (fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
        ok = False;
    else
    {
        ok = WriteAll(fd, buf.str, buf.len);
        if (close(fd) != 0)
            ok = False;
        if (ok && rename(tmp, path) != 0)
            ok = False;
        if (!ok)
            unlink(tmp);
    }

    XtFree(tmp);
    XtFree(path);
    XtFree(buf.str);
    return ok;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* Preview                               */
/*+++++++++++++++++++++++++++++++++++++++*/

#ifdef DTSTYLE_HAVE_XFT
/*
 * Sets one rendering option the way a client would end up with it.  A
 * value chosen in the dialog is used if Override is on or the system
 * configuration left it unset (Xft.* is applied only then).  For Default,
 * libXft's built-in value (builtin >= 0) stands in for the Xft.* line
 * this display connection may still have cached.
 */
static void
SetFcOption(
        FcPattern *p,
        const char *object,
        Boolean isBool,
        int value,
        int builtin,
        Boolean force )
{
    FcValue v;
    Boolean have = FcPatternGet(p, object, 0, &v) == FcResultMatch;

    if (value >= 0)
    {
        if (!force && have)
            return;
        FcPatternDel(p, object);
    }
    else if (!have && builtin >= 0)
        value = builtin;
    else
        return;

    if (isBool)
        FcPatternAddBool(p, object, value ? FcTrue : FcFalse);
    else
        FcPatternAddInteger(p, object, value);
}
#endif /* DTSTYLE_HAVE_XFT */

/*
 * A one-rendition render table for face with the rendering options of s,
 * matched as a client would match it; NULL if it cannot be opened (or
 * without Xft support).  XmNfont is passed as NULL so that a *Font in
 * the resource database is not loaded into the rendition.
 */
XmRenderTable
FxPreviewRenderTable(
        Widget w,
        const FxFace *face,
        const FxSettings *s )
{
#ifdef DTSTYLE_HAVE_XFT
    static const int hsFc[] = { -1, FC_HINT_NONE, FC_HINT_SLIGHT,
                                FC_HINT_MEDIUM, FC_HINT_FULL };
    static const int rgbaFc[] = { -1, FC_RGBA_NONE, FC_RGBA_RGB, FC_RGBA_BGR,
                                  FC_RGBA_VRGB, FC_RGBA_VBGR };
    static const int lcdFc[] = { -1, FC_LCD_NONE, FC_LCD_DEFAULT,
                                 FC_LCD_LIGHT, FC_LCD_LEGACY };
    Display      *dpy;
    int           scr, i, n;
    FcPattern    *p, *m;
    FcResult      res;
    XftFont      *xf;
    Arg           args[8];
    XmRendition   r;
    XmRenderTable rt;
    Boolean       force;

    if (w == NULL || face == NULL || s == NULL || !FcInit())
        return NULL;
    dpy = XtDisplayOfObject(w);
    scr = XScreenNumberOfScreen(XtScreenOfObject(w));
    force = s->fcOverride;

    if ((p = FcPatternCreate()) == NULL)
        return NULL;
    if (face->family[0])
        FcPatternAddString(p, FC_FAMILY, (const FcChar8 *)face->family);
    if (face->style[0])
        FcPatternAddString(p, FC_STYLE, (const FcChar8 *)face->style);
    FcPatternAddDouble(p, FC_SIZE, (double)ClampSize(face->size));
    FcPatternAddDouble(p, FC_DPI,
                       (double)(ClampDpi(s->dpi) ? ClampDpi(s->dpi)
                                                 : FxAutoDpi(dpy)));
    FcConfigSubstitute(NULL, p, FcMatchPattern);

    i = ClampIndex(s->antialias, FX_NUM(fxBoolNames));
    SetFcOption(p, FC_ANTIALIAS, True, i ? i == FX_TRI_ON : -1, 1, force);
    i = ClampIndex(s->hinting, FX_NUM(fxBoolNames));
    SetFcOption(p, FC_HINTING, True, i ? i == FX_TRI_ON : -1, 1, force);
    SetFcOption(p, FC_HINT_STYLE, False,
                hsFc[ClampIndex(s->hintStyle, FX_NUM(hsFc))],
                FC_HINT_FULL, force);
    /* Default: XftDefaultSubstitute takes the subpixel order of the
     * screen (from RENDER), as libXft does for a client without Xft.rgba */
    SetFcOption(p, FC_RGBA, False,
                rgbaFc[ClampIndex(s->rgba, FX_NUM(rgbaFc))], -1, force);
    SetFcOption(p, FC_LCD_FILTER, False,
                lcdFc[ClampIndex(s->lcdFilter, FX_NUM(lcdFc))],
                FC_LCD_DEFAULT, force);

    XftDefaultSubstitute(dpy, scr, p);
    m = FcFontMatch(NULL, p, &res);
    FcPatternDestroy(p);
    if (m == NULL)
        return NULL;
    if ((xf = XftFontOpenPattern(dpy, m)) == NULL)
    {
        FcPatternDestroy(m);
        return NULL;
    }

    /* the rendition owns xf from here on */
    n = 0;
    XtSetArg(args[n], XmNfontType, XmFONT_IS_XFT); n++;
    XtSetArg(args[n], XmNxftFont, (XtArgVal)xf); n++;
    if (face->family[0])
    {
        XtSetArg(args[n], XmNfontName, (XtArgVal)face->family); n++;
    }
    XtSetArg(args[n], XmNfont, (XtArgVal)NULL); n++;
    XtSetArg(args[n], XmNloadModel, XmLOAD_IMMEDIATE); n++;
    r = XmRenditionCreate(w, FX_PREVIEW_TAG, args, n);
    rt = XmRenderTableAddRenditions(NULL, &r, 1, XmMERGE_REPLACE);
    XmRenditionFree(r);
    return rt;
#else
    (void)w;
    (void)face;
    (void)s;
    return NULL;
#endif /* DTSTYLE_HAVE_XFT */
}

/*
 * Does this libXm open Xft fonts?  An XFT rendition of a Motif built
 * without Xft has no xftFont.
 */
Boolean
FxMotifHasXft(
        Widget w )
{
    Arg         args[6];
    XmRendition r;
    XtPointer   xf = NULL;
    int         n;

    if (w == NULL)
        return False;
    n = 0;
    XtSetArg(args[n], XmNfontType, XmFONT_IS_XFT); n++;
    XtSetArg(args[n], XmNfontName, (XtArgVal)"sans-serif"); n++;
    XtSetArg(args[n], XmNfontSize, FX_SIZE_DEFAULT); n++;
    XtSetArg(args[n], XmNfont, (XtArgVal)NULL); n++;
    XtSetArg(args[n], XmNloadModel, XmLOAD_IMMEDIATE); n++;
    r = XmRenditionCreate(w, FX_PREVIEW_TAG, args, n);
    if (r == NULL)
        return False;
    n = 0;
    XtSetArg(args[n], XmNxftFont, &xf); n++;
    XmRenditionRetrieve(r, args, n);
    XmRenditionFree(r);
    return xf != NULL && xf != (XtPointer)XmAS_IS;
}


/*+++++++++++++++++++++++++++++++++++++++*/
/* Session file                          */
/*+++++++++++++++++++++++++++++++++++++++*/

static void
WriteLine(
        int fd,
        const char *key,
        const char *value )
{
    char buf[1024 + 2 * FX_NAME_MAX];

    snprintf(buf, sizeof(buf), "*Fonts.%s: %s\n", key, value);
    if (write(fd, buf, strlen(buf)) == -1)
        perror(strerror(errno));
}

/* The last Xft choices, also those made while in Bitmap mode */
void
FxSaveState(
        int fd,
        const FxSettings *s )
{
    char num[32];

    if (s->sys.family[0] && FxNameIsSafe(s->sys.family))
        WriteLine(fd, "xftSystemFamily", s->sys.family);
    if (s->sys.style[0] && FxNameIsSafe(s->sys.style))
        WriteLine(fd, "xftSystemStyle", s->sys.style);
    snprintf(num, sizeof(num), "%d", ClampSize(s->sys.size));
    WriteLine(fd, "xftSystemSize", num);
    if (s->user.family[0] && FxNameIsSafe(s->user.family))
        WriteLine(fd, "xftUserFamily", s->user.family);
    if (s->user.style[0] && FxNameIsSafe(s->user.style))
        WriteLine(fd, "xftUserStyle", s->user.style);
    snprintf(num, sizeof(num), "%d", ClampSize(s->user.size));
    WriteLine(fd, "xftUserSize", num);
    WriteLine(fd, "xftUserMonoOnly", s->userMonoOnly ? "True" : "False");

    WriteLine(fd, "xftAntialias",
              fxTriState[ClampIndex(s->antialias, FX_NUM(fxTriState))]);
    WriteLine(fd, "xftHinting",
              fxTriState[ClampIndex(s->hinting, FX_NUM(fxTriState))]);
#define FX_NAMED(names, v) \
    (ClampIndex((v), FX_NUM(names)) ? names[ClampIndex((v), FX_NUM(names))] \
                                    : "default")
    WriteLine(fd, "xftHintStyle", FX_NAMED(fxHsNames, s->hintStyle));
    WriteLine(fd, "xftRgba", FX_NAMED(fxRgbaNames, s->rgba));
    WriteLine(fd, "xftLcdFilter", FX_NAMED(fxLcdNames, s->lcdFilter));
#undef FX_NAMED
    snprintf(num, sizeof(num), "%d", ClampDpi(s->dpi));
    WriteLine(fd, "xftDpi", num);
    WriteLine(fd, "xftFcOverride", s->fcOverride ? "True" : "False");
}

/* *Fonts.<key> from the session database */
static Boolean
GetRestored(
        XrmDatabase db,
        const char *key,
        char *out,
        size_t outSize )
{
    XrmQuark          names[3];
    XrmRepresentation type;
    XrmValue          value;
    size_t            n;

    names[0] = XrmStringToQuark("Fonts");
    names[1] = XrmStringToQuark(key);
    names[2] = NULLQUARK;
    if (db == NULL ||
        !XrmQGetResource(db, names, names, &type, &value) ||
        value.addr == NULL)
        return False;
    n = strnlen((const char *)value.addr, outSize);
    if (n >= outSize)
        return False;
    memcpy(out, value.addr, n + 1);
    return True;
}

unsigned
FxRestoreState(
        XrmDatabase db,
        FxSettings *out )
{
    char     val[FX_NAME_MAX];
    unsigned mask = 0;
    int      v;

    memset(out, 0, sizeof(*out));
    out->mode = FX_MODE_BITMAP;
    out->sys.size = FX_SIZE_DEFAULT;
    out->user.size = FX_SIZE_DEFAULT;
    out->userMonoOnly = True;

    if (GetRestored(db, "xftSystemFamily", val, sizeof(val)))
    {
        mask |= FX_HAVE_FACES;
        if (FxNameIsSafe(val))
            CopyName(out->sys.family, val);
    }
    if (GetRestored(db, "xftUserFamily", val, sizeof(val)))
    {
        mask |= FX_HAVE_FACES;
        if (FxNameIsSafe(val))
            CopyName(out->user.family, val);
    }
    if (GetRestored(db, "xftSystemStyle", val, sizeof(val)) &&
        FxNameIsSafe(val))
        CopyName(out->sys.style, val);
    if (GetRestored(db, "xftUserStyle", val, sizeof(val)) &&
        FxNameIsSafe(val))
        CopyName(out->user.style, val);
    if (GetRestored(db, "xftSystemSize", val, sizeof(val)))
        out->sys.size = ClampSize(atoi(val));
    if (GetRestored(db, "xftUserSize", val, sizeof(val)))
        out->user.size = ClampSize(atoi(val));
    if (GetRestored(db, "xftUserMonoOnly", val, sizeof(val)))
        out->userMonoOnly = ParseBool(val) != FX_TRI_OFF;

    if (GetRestored(db, "xftAntialias", val, sizeof(val)))
    {
        mask |= FX_HAVE_RENDERING;
        out->antialias = (v = ParseBool(val)) < 0 ? FX_TRI_DEFAULT : v;
    }
    if (GetRestored(db, "xftHinting", val, sizeof(val)))
    {
        mask |= FX_HAVE_RENDERING;
        out->hinting = (v = ParseBool(val)) < 0 ? FX_TRI_DEFAULT : v;
    }
    if (GetRestored(db, "xftHintStyle", val, sizeof(val)))
    {
        mask |= FX_HAVE_RENDERING;
        out->hintStyle = ParseNamed(val, fxHsNames, FX_NUM(fxHsNames));
    }
    if (GetRestored(db, "xftRgba", val, sizeof(val)))
    {
        mask |= FX_HAVE_RENDERING;
        out->rgba = ParseNamed(val, fxRgbaNames, FX_NUM(fxRgbaNames));
    }
    if (GetRestored(db, "xftLcdFilter", val, sizeof(val)))
    {
        mask |= FX_HAVE_RENDERING;
        out->lcdFilter = ParseNamed(val, fxLcdNames, FX_NUM(fxLcdNames));
    }
    if (GetRestored(db, "xftDpi", val, sizeof(val)))
    {
        mask |= FX_HAVE_RENDERING;
        out->dpi = ClampDpi(atoi(val));
    }
    if (GetRestored(db, "xftFcOverride", val, sizeof(val)))
    {
        mask |= FX_HAVE_RENDERING;
        out->fcOverride = ParseBool(val) == FX_TRI_ON;
    }
    return mask;
}
