/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
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
/* $XConsortium: fontpath.c /main/4 1995/10/27 16:13:29 rswiston $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
**  fontpath.c - font path modification routines
**
**  $fontpath.c,v 1.1 93/06/24 14:52:10 bill Exp $
**
**  Copyright 1993 Hewlett-Packard Company
*/

# include "dm.h"
# include <signal.h>
# include <X11/Xatom.h>
# include <X11/Xmu/Error.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <limits.h>
# include <unistd.h>
# include "vgproto.h"

/* 
**  Prototypes
*/


int ApplyFontPathMods( struct display *d, Display *dpy );
int AppendDesktopFontPath( struct display *d, Display *dpy );
void GetSysParms( char **tzpp, char **fhpp, char **ftpp );
static int PathInPList(char *path, char **fplist, int listlen);
static int PathInZList(char *path, char *fplist, int listlen);
static int SeparateParts( char **path );

void GenerateAuthData(char *auth, int len); // genauth.c


static void
DebugFontPath(char *note, char **path, int nelems)
{
    int i;
    Debug("  %s: %d elements\n",note,nelems);
    for (i=0; i<nelems; i++)
        Debug("    %s\n",path[i]);
}

static int 
ErrorHandler(Display *dpy, XErrorEvent *event)
{
    XmuPrintDefaultErrorMessage(dpy, event, stderr);
    return 0; /* nonfatal */
}

/* ________________________________________________________________
**|                                                                |
**| ApplyFontPathMods(d) -                                         |
**|                                                                |
**|     If new font path mods are mandated, fabricate and apply    |
**|     an appropriate new complete X server font path.            |
**|								   |
**| Specify: d = pointer to display structure			   |
**|								   |
**| Returns: nothing						   |
**|________________________________________________________________|
*/

int 
ApplyFontPathMods( struct display *d, Display *dpy )
{
    char *s;
    char *fph = NULL;
    char *fpt = NULL;
    char **fontPath,**newList;
    int numHeads = 0;
    int numTails = 0;
    int i,j,numPaths;

    Debug("ApplyFontPathMods() for %s\n",d->name);

    if (d->displayType.location == Foreign) {
        Debug("  Foreign display\n");
        return 0;
    }

    if (!(fontPath=XGetFontPath(dpy, &numPaths))) {
        Debug("  Can't get font path\n");
        return 0;
    }

    /*
    **  Font path mods can come from (in priority order):
    **
    **    1. FONT_PATH_HEAD/TAIL definitions in /etc/src.sh
    **    2. fontPathHead/Tail resources
    */

    GetSysParms(0,&fph,&fpt);

    if (fph)          { s = "sys parm file"; }
    else if (fpHead)  { s = "resource"; fph = strdup(fpHead); }
    if (fph && !*fph) { free(fph); fph = NULL; }
    if (fph)          Debug("  +fp (%s) %s\n",s,fph);

    if (fpt)          { s = "sys parm file"; }
    else if (fpTail)  { s = "resource"; fpt = strdup(fpTail); }
    if (fpt && !*fpt) { free(fpt); fpt = NULL; }
    if (fpt)          Debug("  fp+ (%s) %s\n",s,fpt);

    /* 
    **  Break up fph and fpt into constituent parts and 
    **  then reconstruct the complete, modified font path.
    **  During reconstruction we also eliminate redundancies.
    */

    numHeads = SeparateParts(&fph);
    numTails = SeparateParts(&fpt);
    if (numHeads || numTails) {
        newList = (char **) malloc((numHeads+numPaths+numTails)
            * sizeof(char *));
        if (newList) {
            for (s=fph, i=j=0; j<numHeads; j++) {
                if (!PathInPList(s,newList,i))
                    newList[i++] = s;
                while (*s) s++;
                s++;
            }
            for (j=0; j<numPaths; j++) {
                if (!PathInPList(fontPath[j],newList,i) &&
                    !PathInZList(fontPath[j],fpt,numTails)) {
                    newList[i++] = fontPath[j];
                }
            }
            for (s=fpt, j=0; j<numTails; j++) {
                if (!PathInPList(s,newList,i))
                    newList[i++] = s;   
                while (*s) s++;
                s++;
            }
            /*
            **  Nothing to do if the path already is the new one (the
            **  elements are there, in that order): setting the path
            **  makes the server re-check every element, and XSync()
            **  costs a round trip.
            */
            if (i == numPaths) {
                for (j = 0; j < i; j++)
                    if (strcmp(newList[j], fontPath[j]) != 0)
                        break;
                if (j == i) {
                    Debug("  Font path unchanged\n");
                    free(newList);
                    newList = NULL;
                }
            }
        }
        if (newList) {
            if (debugLevel > 0)
                DebugFontPath("Request (XSetFontPath)",newList,i);

            /*
            **  Tell X server to set new font path now.  Log failure,
            **  but don't let it be fatal.  (Note that caller should
            **  reset error handler to elsewhere when we return.)
            */

            (void)XSetErrorHandler(ErrorHandler);
            XSetFontPath(dpy, newList, i);
            XSync(dpy, True);
            free(newList);
            if (debugLevel > 0) {
                newList = XGetFontPath(dpy, &i);
                DebugFontPath("Confirm (XGetFontPath)",newList,i);
                XFreeFontPath(newList);
            }
        }
    }

    if (fph) free(fph);
    if (fpt) free(fpt);
    XFreeFontPath(fontPath);

    return 1;
}

/* ________________________________________________________________
**|                                                                |
**| AppendDesktopFontPath(d, dpy) -                                |
**|                                                                |
**|     Append the desktop font alias directories (and, on Linux   |
**|     and the BSDs, the X server's own font directories) to the  |
**|     font path, as the "xset fp+" calls in Xsetup do, without   |
**|     forking xset, and skipping the directories that are on    |
**|     the path already.                                          |
**|                                                                |
**|     Done only for an Xsetup that mentions DTLOGIN_FONTPATH:    |
**|     such a script skips its own xset calls when dtlogin sets   |
**|     that variable.  Any other Xsetup (an old or a site copy)   |
**|     is left to do what it does.                                |
**|                                                                |
**| Returns: 1 if the script should skip its xset calls, else 0    |
**|________________________________________________________________|
*/

#if !defined(_AIX) && !defined(sun)

#if defined(SYS_FONTLIB)
/* given on the command line */
#elif defined(__linux__)
# define SYS_FONTLIB "/usr/share/fonts/X11"
#elif defined(__FreeBSD__)
# define SYS_FONTLIB "/usr/local/lib/X11/fonts"
#elif defined(__OpenBSD__)
# define SYS_FONTLIB "/usr/X11R6/lib/X11/fonts"
#elif defined(__NetBSD__)
# define SYS_FONTLIB "/usr/X11R7/lib/X11/fonts"
#endif

static int fpFailed;

static int
FpErrorHandler(Display *dpy, XErrorEvent *event)
{
    fpFailed = 1;
    return 0;
}

/* Does the setup script know about DTLOGIN_FONTPATH? */
static int
SetupKnowsFontPath(const char *setup)
{
    static char *cachedName;
    static time_t cachedMtime;
    static off_t cachedSize;
    static int cachedResult;
    struct stat st;
    char *buf;
    int fd, result = 0;
    ssize_t n;

    if (!setup || !*setup || stat(setup, &st) != 0 || !S_ISREG(st.st_mode))
        return 0;
    if (cachedName && strcmp(cachedName, setup) == 0 &&
        cachedMtime == st.st_mtime && cachedSize == st.st_size)
        return cachedResult;
    if (st.st_size > 1024 * 1024 || (fd = open(setup, O_RDONLY)) < 0)
        return 0;
    if ((buf = malloc(st.st_size + 1)) != NULL) {
        n = read(fd, buf, st.st_size);
        if (n > 0) {
            buf[n] = '\0';
            result = strstr(buf, "DTLOGIN_FONTPATH") != NULL;
        }
        free(buf);
    }
    close(fd);
    free(cachedName);
    cachedName = strdup(setup);
    cachedMtime = st.st_mtime;
    cachedSize = st.st_size;
    cachedResult = result;
    return result;
}

static int
HasFontsFile(const char *dir, int aliasToo)
{
    char path[PATH_MAX];

    snprintf(path, sizeof(path), "%s/fonts.dir", dir);
    if (access(path, F_OK) == 0)
        return 1;
    if (!aliasToo)
        return 0;
    snprintf(path, sizeof(path), "%s/fonts.alias", dir);
    return access(path, F_OK) == 0;
}

/*
 * Append the elements of add[] that are not on the font path yet, in one
 * request.  Returns 0 if the server rejected the path (then nothing has
 * changed: the server takes a path all or nothing).
 */
static int
FontPathAppend(Display *dpy, char **add, int nadd)
{
    char **cur, **list;
    int ncur, n, i, ok = 1;

    if (!(cur = XGetFontPath(dpy, &ncur)))
        ncur = 0;
    list = malloc((ncur + nadd) * sizeof(char *));
    if (!list) {
        if (cur) XFreeFontPath(cur);
        return 1;
    }
    for (n = 0; n < ncur; n++)
        list[n] = cur[n];
    for (i = 0; i < nadd; i++)
        if (!PathInPList(add[i], list, n))
            list[n++] = add[i];
    if (n > ncur) {
        if (debugLevel > 0)
            DebugFontPath("Append (XSetFontPath)", list + ncur, n - ncur);
        fpFailed = 0;
        XSetFontPath(dpy, list, n);
        XSync(dpy, False);
        ok = !fpFailed;
        if (!ok)
            Debug("  Font path rejected\n");
    }
    free(list);
    if (cur) XFreeFontPath(cur);
    return ok;
}

int
AppendDesktopFontPath( struct display *d, Display *dpy )
{
    static const char *aliasDirs[] = { FONTALIASESCONFDIR, FONTALIASESDATADIR };
#ifdef SYS_FONTLIB
    static const char *sysDirs[] = {
        SYS_FONTLIB "/misc/", SYS_FONTLIB "/75dpi/", SYS_FONTLIB "/100dpi/",
        SYS_FONTLIB "/Speedo/", SYS_FONTLIB "/Type1/", SYS_FONTLIB "/PJE/"
    };
#endif
    char *all[8];
    int nalias = 0, nall, i;
    int (*oldHandler)(Display *, XErrorEvent *);

    if (!SetupKnowsFontPath(d->setup))
        return 0;

    Debug("AppendDesktopFontPath() for %s\n", d->name);
    oldHandler = XSetErrorHandler(FpErrorHandler);

    if (d->displayType.location == Local) {
        /*
         *  Local server: add the directories that exist, all in one
         *  request.  If the server rejects that, add the alias and the
         *  system directories separately, as Xsetup does.
         */
        for (i = 0; i < 2; i++)
            if (HasFontsFile(aliasDirs[i], 1))
                all[nalias++] = (char *) aliasDirs[i];
        nall = nalias;
#ifdef SYS_FONTLIB
        for (i = 0; i < (int) (sizeof(sysDirs) / sizeof(sysDirs[0])); i++)
            if (HasFontsFile(sysDirs[i], 0))
                all[nall++] = (char *) sysDirs[i];
#endif
        if (nall > 0 && !FontPathAppend(dpy, all, nall) &&
            nalias > 0 && nall > nalias) {
            FontPathAppend(dpy, all, nalias);
            FontPathAppend(dpy, all + nalias, nall - nalias);
        }
    } else {
        /*
         *  Remote server: we can't know which directories it has, so
         *  try them one at a time.
         */
        for (i = 0; i < 2; i++) {
            all[0] = (char *) aliasDirs[i];
            FontPathAppend(dpy, all, 1);
        }
    }

    XSetErrorHandler(oldHandler);
    return 1;
}

#else /* _AIX || sun: Xsetup does it */

int
AppendDesktopFontPath( struct display *d, Display *dpy )
{
    return 0;
}

#endif

/* ___________________________________________________________________
**|                                                                   |
**| PathInPList(path,fplist,listlen) -                                |
**|								      |
**|     Determine if a specific fontpath element is in a list,        |
**|     taking into account that identical elements may be formed     |
**|     differently (with multiple embedded and trailing slashes).    |
**|                                                                   |
**| Specify: (char *)path = the single element to be tested           |
**|          (char **)fplist = list of ptrs to asciz elements         |
**|          (int)listlen = number of pointers in the list            |
**|                                                                   |
**| Returns: TRUE if element is in the list                           |
**|___________________________________________________________________|
*/

static int
PathInPList(char *path, char **fplist, int listlen)
{
    char *s,*t;
    while (listlen-- > 0) {
        for (s=path, t=fplist[listlen]; *s && (*s == *t); ) {
            t++; while (*t == '/') t++;
            s++; while (*s == '/') s++;
        }
        if (!*s && !*t) return 1;
    }
    return 0;    
}

/* ___________________________________________________________________
**|                                                                   |
**| PathInZList(path,fplist,listlen) -                                |
**|								      |
**|     Determine if a specific fontpath element is in a list,        |
**|     taking into account that identical elements may be formed     |
**|     differently (with multiple embedded and trailing slashes).    |
**|                                                                   |
**| Specify: (char *)path = the single element to be tested           |
**|          (char *)fplist = list of concatenated asciz elements     |
**|          (int)listlen = number of elements in the list            |
**|                                                                   |
**| Returns: TRUE if element is in the list                           |
**|___________________________________________________________________|
*/

static int
PathInZList(char *path, char *fplist, int listlen)
{
    char *s,*t;
    for (t=fplist; listlen > 0; listlen--) {
        for (s=path; *s && (*s == *t); ) {
            t++; while (*t == '/') t++;
            s++; while (*s == '/') s++;
        }
        if (!*s && !*t) return 1;
        while (*t) t++;
        t++;
    }
    return 0;    
}

/* ________________________________________________________________
**|                                                                |
**| SeparateParts(path)					           |
**|								   |
**|     Break a comma-delimited asciz path string into its	   |
**|     separate asciz constituent parts.			   |
**|								   |
**| Specify: path = ptr to asciz path string (e.g., "as,df,jk")	   |
**|								   |
**| Returns: number of constituent parts, with path string         |
**|          converted into as many sequential asciz strings       |
**|          (e.g., "as\0df\0jk").				   |
**|________________________________________________________________|
*/

static int
SeparateParts( char **path )
{
    char *t,*s;
    int nparts = 0;
    if (path && *path)
	for (s=*path; (t=strtok(s,",")); s=NULL, nparts++);
    return nparts;
}

/* ___________________________________________________________________
**|                                                                   |
**| GetSysParms(tzpp,fhpp,ftpp) -				      |
**|								      |
**|     Extract TZ, FONT_PATH_HEAD, and FONT_PATH_TAIL definitions    |
**|     from the sys parms file (typically /etc/src.sh).              |
**|                                                                   |
**| Specify: (char **)tzpp = where to put ptr to TZ string            |
**|          (char **)fhpp = where to put ptr to FONT_PATH_HEAD       |
**|          (char **)ftpp = where to put ptr to FONT_PATH_TAIL       |
**|                                                                   |
**|     Specify a NULL (char **) for any undesired strings.           |
**|                                                                   |
**| Returns: Appropriate pointers to malloc'ed strings (NULL pointer  |
**|          if a string is neither requested nor defined).           |
**|___________________________________________________________________|
*/

void
GetSysParms( char **tzpp, char **fhpp, char **ftpp )
{
    FILE *fin;
    char *s,*t,buf[256];

    if (tzpp) *tzpp = NULL;
    if (fhpp) *fhpp = NULL;
    if (ftpp) *ftpp = NULL;
    if ((*sysParmsFile != '/') || !(fin=fopen(sysParmsFile,"r"))) {
        Debug("(GetSysParms) Can't open sys parms file\n");
        return;
    }
    Debug("(GetSysParms) Reading sys parms file\n");
    while (fgets(buf,255,fin)) {
        for (t=buf; *t && *t<=' '; t++); /* t -> EOS or nonblank */
        if (!*t || *t=='#') continue;    /* ignore comment lines */
        while (*t && *t!='\n') t++;      /* t -> EOS or newline  */
        if (*t) *t = '\0';               /* discard newline char */
        if (tzpp && !*tzpp)
            if ((s=strstr(buf,"TZ=")) && (t=strtok(s+3,"; \t")))
                *tzpp = (char *) strdup(t);
        if (fhpp && !*fhpp)
            if ((s=strstr(buf,"FONT_PATH_HEAD=")) && (t=strtok(s+15,"; \t")))
                *fhpp = (char *) strdup(t);
        if (ftpp && !*ftpp)
            if ((s=strstr(buf,"FONT_PATH_TAIL=")) && (t=strtok(s+15,"; \t")))
                *ftpp = (char *) strdup(t);
    }
    fclose(fin);
}    

