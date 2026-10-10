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
/*
 * (c) Copyright 1995 Digital Equipment Corporation.
 * (c) Copyright 1995 Hewlett-Packard Company.
 * (c) Copyright 1995 International Business Machines Corp.
 * (c) Copyright 1995 Sun Microsystems, Inc.
 * (c) Copyright 1995 Novell, Inc.
 * (c) Copyright 1995 FUJITSU LIMITED.
 * (c) Copyright 1995 Hitachi.
 *
 * $TOG: SmAuth.c /main/4 1997/03/14 14:11:50 barstow $
 */
/******************************************************************************

Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <X11/Intrinsic.h>
#include <X11/SM/SMlib.h>
#include <X11/ICE/ICEutil.h>

#include "SmAuth.h"

typedef struct _IceAuthFileEntryList
{
  IceAuthFileEntry *fileEntry;
  struct _IceAuthFileEntryList *next;
} IceAuthFileEntryList;

/*
 * Private data
 */
#define MAGIC_COOKIE_LEN 16

/*
 * .ICEauthority locking.  These used to be passed to IceLockAuthFile()
 * as 10 retries x 2 s with a 600 s dead time, so a lock left behind by a
 * session that died less than ten minutes earlier stalled login for 20 s
 * (40 s when the fallback path was tried as well).  Lock holders keep
 * the lock for a few milliseconds, so poll every 100 ms and treat a lock
 * older than 10 s as stale.
 */
#define AUTH_POLL_MS 100
#define AUTH_RETRIES 120	/* x AUTH_POLL_MS = 12 s, > AUTH_DEADTIME */
#define AUTH_DEADTIME 10L
/*
 * The lock file's ctime comes from the file server's clock, which may be
 * off from ours (NFS home directories): a lock is only taken as stale
 * after AUTH_DEADTIME by its ctime AND by our own clock (we waited that
 * long for it), unless its ctime says it is older than libICE's usual
 * dead time of ten minutes.
 */
#define AUTH_OLDTIME 600L

/*
 * Private functions - forward declarations
 */

static int
writeIceauth (
	int			nEntries,
	IceAuthDataEntry	*entries,
	int			restore);

static int
addToEntryList (
	IceAuthFileEntryList	**entryListP,
	IceAuthFileEntry	*fileEntry);

static int
fileEntryInDataEntries (
	int			nEntries,
	IceAuthDataEntry	*entries,
	IceAuthFileEntry	*fileEntry);

static void
freeEntryList (
	IceAuthFileEntryList	*entryList);

/*
 * Private functions - implemenation.
 */

/*
 * lockAuthFile - take the lock on an ICE authority file.
 *
 * Same protocol as IceLockAuthFile() in libICE (create "<file>-c", then
 * hard-link it to "<file>-l"; the link is the lock and IceUnlockAuthFile()
 * removes both), so it interoperates with every other ICE client, but it
 * polls at AUTH_POLL_MS instead of whole seconds and re-checks for a stale
 * lock on every attempt.  The "-c" file is opened without O_TRUNC so that
 * waiting does not refresh the ctime that staleness is judged by.
 */
static int
lockAuthFile (
	const char	*file_name)
{
    char creat_name[MAXPATHLEN], link_name[MAXPATHLEN], dir_name[MAXPATHLEN];
    struct timespec poll = { 0, AUTH_POLL_MS * 1000000L };
    struct stat statb;
    char *slash;
    int created = 0;
    int retries = AUTH_RETRIES;
    int fd;
    time_t start = time(NULL), now, age;

    if (snprintf(creat_name, sizeof(creat_name), "%s-c", file_name)
	    >= (int) sizeof(creat_name) ||
	snprintf(link_name, sizeof(link_name), "%s-l", file_name)
	    >= (int) sizeof(link_name))
	return IceAuthLockError;

    /* link() needs a writable directory; without one, waiting is futile. */
    snprintf(dir_name, sizeof(dir_name), "%s", file_name);
    if ((slash = strrchr(dir_name, '/')) == NULL)
	strcpy(dir_name, ".");
    else if (slash == dir_name)
	dir_name[1] = '\0';
    else
	*slash = '\0';
    if (access(dir_name, W_OK) != 0)
	return IceAuthLockError;

    while (retries > 0)
    {
	now = time(NULL);
	if (stat(creat_name, &statb) == 0 &&
	    ((age = now - statb.st_ctime) > AUTH_OLDTIME ||
	     (age > AUTH_DEADTIME && now - start >= AUTH_DEADTIME)))
	{
	    unlink(creat_name);
	    unlink(link_name);
	    created = 0;
	}

	if (!created)
	{
	    fd = open(creat_name, O_WRONLY | O_CREAT, 0666);
	    if (fd == -1)
	    {
		if (errno != EACCES)
		    return IceAuthLockError;
	    }
	    else
	    {
		close(fd);
		created = 1;
	    }
	}

	if (created)
	{
	    if (link(creat_name, link_name) == 0)
		return IceAuthLockSuccess;

	    if (errno == ENOENT)
	    {
		/* The holder unlocked between our open and link. */
		created = 0;
		continue;
	    }

	    if (errno != EEXIST)
		return IceAuthLockError;
	}

	nanosleep(&poll, NULL);
	--retries;
    }

    return IceAuthLockTimeout;
}

static void
freeEntryList (
	IceAuthFileEntryList	*entryList)
{
    IceAuthFileEntryList *nextEntryP;

    while (entryList != (IceAuthFileEntryList *)NULL)
    {
	nextEntryP = entryList->next;

	IceFreeAuthFileEntry(entryList->fileEntry);
	XtFree((char *)entryList);

	entryList = nextEntryP;
    }
}

static int
fileEntryInDataEntries (
	int			nEntries,
	IceAuthDataEntry	*entries,
	IceAuthFileEntry	*fileEntry)
{
    int i;

#define SAME_STR(field) \
    ((entries->field != (char *)NULL) &&\
     (fileEntry->field != (char *)NULL) &&\
     (strcmp(entries->field, fileEntry->field) == 0))

    for (i = 0; i < nEntries; i++, entries++)
    {
	if (SAME_STR(protocol_name) &&
	    SAME_STR(network_id) &&
	    SAME_STR(auth_name))
	    return 1;
    }

#undef SAME_STR

    return 0;
}

static int
addToEntryList (
	IceAuthFileEntryList	**entryListP,
	IceAuthFileEntry	*fileEntry)
{
    IceAuthFileEntryList *newItem;

    if ((newItem =
	 (IceAuthFileEntryList *)XtMalloc(sizeof(IceAuthFileEntryList)))
	== (IceAuthFileEntryList *)NULL)
	return 0;

    /* I assume it's ok to reverse the order; otherwise */
    /* we need to add the new item onto the end of the list. */
    newItem->fileEntry = fileEntry;
    newItem->next = *entryListP;
    *entryListP = newItem;

    return 1;
}

static int
writeIceauth (
	int			nEntries,
	IceAuthDataEntry	*entries,
	int			restore)
{
    FILE *fp = NULL;
    char *path;
    char *extraPath;
    int oldUmask;
    int i;
    IceAuthDataEntry *dataEntry;
    IceAuthFileEntry *fileEntry;
    IceAuthFileEntry newEntry;
    IceAuthFileEntryList *fileEntryList = (IceAuthFileEntryList *)NULL;
    IceAuthFileEntryList *fileEntryP;

    if ((path = IceAuthFileName()) == (char *)NULL)
	return 0;

    if (lockAuthFile(path) != IceAuthLockSuccess) {
	/*
	 * Let's try another PATH, in case IceLockAuthFile's call to
	 * link() fails.  This workaround code was taken from 
	 * dtlogin/auth.c.
	 */
        IceUnlockAuthFile(path);
	extraPath = CDE_CONFIGURATION_TOP ".ICEauthority";
	if (lockAuthFile(extraPath) != IceAuthLockSuccess) {
	    IceUnlockAuthFile (extraPath);
	    return 0;
	 }
	 path = extraPath;
    }

    /* If file exists, read entries into memory. */
    if (access(path, F_OK) == 0)
    {
	if ((fp = fopen(path, "rb")) == (FILE *)NULL)
	{
	    IceUnlockAuthFile(path);
	    return 0;
	}

	/* For each file entry: if matches something in entries, discard. */
	/* Otherwise, hold onto it - we'll be writing it back to file. */
	while ((fileEntry = IceReadAuthFileEntry(fp))
	       != (IceAuthFileEntry *)NULL)
	{
	    if (!fileEntryInDataEntries(nEntries, entries, fileEntry) &&
		!addToEntryList(&fileEntryList, fileEntry))
	    {
		freeEntryList(fileEntryList);
		IceUnlockAuthFile(path);
		fclose(fp);
		return 0;
	    }
	}

	fclose(fp);
    }

    /* Set umask to disallow non-owner access. */
    oldUmask = umask(0077);

    /* Write entries and fileEntryList to file. */
    if ((fp = fopen(path, "wb")) == (FILE *)NULL)
    {
	freeEntryList(fileEntryList);
	IceUnlockAuthFile(path);
	umask(oldUmask);
	return 0;
    }

    for (fileEntryP = fileEntryList;
	 fileEntryP != (IceAuthFileEntryList *)NULL;
	 fileEntryP = fileEntryP->next)
    {
	if (IceWriteAuthFileEntry(fp, fileEntryP->fileEntry) == 0)
	{
	    fclose(fp);
	    umask(oldUmask);
	    freeEntryList(fileEntryList);
	    IceUnlockAuthFile(path);
	    return 0;
	}
    }

    /* Done with fileEntryList - free it up. */
    freeEntryList(fileEntryList);

    if (!restore)
    {
	for (i = 0; i < nEntries; i++)
	{
	    dataEntry = &(entries[i]);
	    newEntry.protocol_name = dataEntry->protocol_name;
	    newEntry.protocol_data_length = 0;
	    newEntry.protocol_data = "";
	    newEntry.network_id = dataEntry->network_id;
	    newEntry.auth_name = dataEntry->auth_name;
	    newEntry.auth_data_length = dataEntry->auth_data_length;
	    newEntry.auth_data = dataEntry->auth_data;

	    if (IceWriteAuthFileEntry(fp, &newEntry) == 0)
	    {
		fclose(fp);
		umask(oldUmask);
		IceUnlockAuthFile(path);
		return 0;
	    }
	}
    }

    /* Success! */
    fclose(fp);
    umask(oldUmask);
    IceUnlockAuthFile(path);
    return 1;
}

/*
 * Host Based Authentication Callback.  This callback is invoked if
 * the connecting client can't offer any authentication methods that
 * we can accept.  We can accept/reject based on the hostname.
 */
Bool
HostBasedAuthProc (
	char			*hostname)

{
    /* 
     * For now, we don't support host based authentication 
     */
    return (0);	      
}


/*
 * Provide authentication data to clients that wish to connect
 */
Status
SetAuthentication (
	int			count,
	IceListenObj		*listenObjs,
	IceAuthDataEntry	**authDataEntries)
{
    int		i;
    int		nEntries = count * 2;

    if ((*authDataEntries = (IceAuthDataEntry *) XtMalloc (
	nEntries * sizeof (IceAuthDataEntry))) == NULL)
	return 0;

    for (i = 0; i < nEntries; i += 2)
    {
	(*authDataEntries)[i].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	(*authDataEntries)[i].protocol_name = "ICE";
	(*authDataEntries)[i].auth_name = "MIT-MAGIC-COOKIE-1";

	(*authDataEntries)[i].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*authDataEntries)[i].auth_data_length = MAGIC_COOKIE_LEN;

	(*authDataEntries)[i+1].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	(*authDataEntries)[i+1].protocol_name = "XSMP";
	(*authDataEntries)[i+1].auth_name = "MIT-MAGIC-COOKIE-1";

	(*authDataEntries)[i+1].auth_data = 
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*authDataEntries)[i+1].auth_data_length = MAGIC_COOKIE_LEN;

	IceSetHostBasedAuthProc (listenObjs[i/2], HostBasedAuthProc);
    }

    /* Merge new entries into auth file. */
    if (!writeIceauth(nEntries, *authDataEntries, 0))
	return 0;

    IceSetPaAuthData(nEntries, *authDataEntries);

    return 1;
}

/*
 * Free up authentication data.
 */
void
FreeAuthenticationData (
	int			count,
	IceAuthDataEntry 	*authDataEntries)
{
    int i;
    int nEntries = count * 2;

    /* Restore auth file to (approx) state before we ran.  We remove all */
    /* new entries... some of these entries may have existed in the auth */
    /* file before but were replaced when we came up. */
    writeIceauth(nEntries, authDataEntries, 1);

    /* Each transport has entries for ICE and XSMP */
    for (i = 0; i < nEntries; i++)
    {
	free (authDataEntries[i].network_id);
	free (authDataEntries[i].auth_data);
    }

    XtFree ((char *) authDataEntries);
}
