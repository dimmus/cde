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
/* $TOG: Directory.c /main/18 1999/12/09 13:05:34 mgreess $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           Directory.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   Description:    Directory processing functions used by the File Browser.
 *
 *   FUNCTIONS: CheckDesktop
 *              CheckDesktopPipeCallback
 *              CheckDesktopProcess
 *              CheckListCmp
 *              DirectoryBeginModify
 *              DirectoryBusy
 *              DirectoryEndModify
 *              DirectoryFileModified
 *              DirectoryGone
 *              DirectoryModifyTime
 *              FileData2toFileData
 *              FileWindowMapUnmap
 *              FindDirectory
 *              FreeDirectory
 *              FreeFileData
 *              GetDirectoryLogicalType
 *              GetDirectoryPositionInfo
 *              GetLongName
 *              InitializeDirectoryRead
 *              InitializePositionFileName
 *              PipeReadFileData
 *              PipeReadPositionInfo
 *              PipeWriteFileData
 *              PipeWritePositionInfo
 *              ReadDir
 *              ReadDirectory
 *              ReadDirectoryFiles
 *              ReadDirectoryProcess
 *              ReadFileData
 *              ReadFileData2
 *              ReaddirPipeCallback
 *              RereadDirectory
 *              ScheduleActivity
 *              ScheduleDirectoryActivity
 *              SetDirectoryPositionInfo
 *              SkipRefresh
 *              SomeWindowMapped
 *              StickyProcIdle
 *              TimerEvent
 *              TimerEventBrokenLinks
 *              TimerEventProcess
 *              TimerPipeCallback
 *              UpdateAllProcess
 *              UpdateCachedDirectories
 *              UpdateDirectory
 *              UpdateDirectorySet
 *              UpdateSomeProcess
 *              WritePosInfoPipeCallback
 *              WritePosInfoProcess
 *              _ReadDir
 *              SelectDesktopFile
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/inotify.h>
#include <sys/vfs.h>
#define DT_USE_INOTIFY 1
#endif

#include <Xm/Xm.h>

#include <Dt/Connect.h>
#include <Dt/DtNlUtils.h>
#include <Dt/Dts.h>
#include <Dt/HourGlass.h>
#include <Dt/Icon.h>
#include <Tt/tttk.h>

#include "Encaps.h"
#include "FileMgr.h"
#include "Desktop.h"
#include "IconicPath.h"
#include "Main.h"
#include "SharedMsgs.h"
#include "SharedProcs.h"
#include "Prefs.h"

extern Boolean removingTrash;


/*--------------------------------------------------------------------
 * Constants and Types
 *------------------------------------------------------------------*/

/* File modes */
#define OPTION_OFF '-'
#define WRITE_PRIV 'w'
#define READ_PRIV  'r'
#define EXEC_PRIV  'x'

/* prefix for the name of the position info file */
#define POSITION_FILE_PREFIX  ".!dt"

/* kinds of messages sent through the pipe */
#define PIPEMSG_ERROR               1
#define PIPEMSG_FILEDATA            2
#define PIPEMSG_DONE                3
#define PIPEMSG_PATH_LOGICAL_TYPES  4
#define PIPEMSG_POSITION_INFO       5
#define PIPEMSG_FILEDATA2           6
#define PIPEMSG_FILEDATA3           7
#define PIPEMSG_DESKTOP_REMOVED     8
#define PIPEMSG_DESKTOP_CHANGED     9

#ifndef	FILEDATABUF
#define	FILEDATABUF 50
#endif /* FILEDATABUF */

#define	NILL '\0'

/*
 * Change detection.  On Linux, directories on local file systems are
 * watched with inotify; the others (NFS, SMB, FUSE, ...), and all of
 * them when inotify is not available, are polled every rereadTime
 * seconds as before.
 */
#define EVENT_DELAY_MS      100   /* collect events this long first */
#define EVENT_INTERVAL_MS  1000   /* at most one refresh per this, per dir */
/* (doubling up to rereadTime seconds while a directory keeps changing:
   no more often than the poll of a directory that changes all the time) */
#define EVENT_RETRY_MS      500   /* recheck a directory that is busy */

/*
 * Directories no window shows any more stay in the cache for a while,
 * so that going Back or Up does not read and type them again.
 */
#define DIR_CACHE_MAX         8   /* unviewed directories kept */
#define DIR_CACHE_MAX_FILES 20000 /* ... with at most this many entries */

/*
 * Background activities, ordered by priority:
 * (activity_idle must be the last one in the list!)
 */
typedef enum
{
  activity_writing_posinfo,  /* writing position information file */
  activity_reading,          /* reading the directory */
  activity_update_all,       /* updating the directory */
  activity_update_some,      /* updating selected files */
  activity_checking_links,   /* checking for broken links */
  activity_checking_desktop, /* checking desktop objects */
  activity_checking_dir,     /* checking if the directory has changed */
  activity_idle              /* no background activity */
} ActivityStatus;

/*  The internal directory structure and directory set list  */

typedef struct
{
   FileMgrData * file_mgr_data;
   Boolean       mapped;
} DirectoryView;

typedef struct
{
   char           * host_name;
   char           * directory_name;
   char           * path_name;
   char           * tt_path_name;
   Boolean          viewed;
   ActivityStatus   activity;
   Boolean          busy[activity_idle];
   Boolean          errmsg_needed;
   int              errnum;
   time_t           modify_time;
   struct timespec  mtim;          /* directory st_mtim at the last read */
   struct timespec  ctim;          /* directory st_ctim at the last read */
   int              last_check;
   Boolean          link_check_needed;
   Boolean          has_links;     /* some entry is a symbolic link */
   Boolean          local_fs;      /* the last read found a local file system */
   int              wd;            /* inotify watch (> 0), else polled */
   Boolean          ev_pending;    /* an entry changed since the last read began */
   Boolean          ev_self;       /* the directory itself changed */
   Boolean          update_changed;/* this read/update changed something */
   long             last_ev_refresh; /* when an event last started a refresh */
   long             ev_interval;   /* the least time to the next one */
   unsigned long    lru_stamp;     /* non-zero: unviewed, kept in the cache */
   Boolean          stale;         /* (cached, unviewed) changed since */
   Boolean          reread_needed; /* (cached, unviewed) missed a db reload */
   int              file_count;
   FileData       * file_data;
   FileData       * new_data;
   FileData      ** new_tail;      /* &last->next of new_data, if non-NULL */
   FileData       * dir_data;
   int              path_count;
   char          ** path_logical_types;
   int              path_generation; /* db_generation they were typed in */
   int              position_count;
   PositionInfo   * position_info;
   int              modify_begin;
   Boolean          was_up_to_date;
   int              modified_count;
   char          ** modified_list;
   int              modified_size; /* allocated entries of modified_list */
   struct _NameIndex *modified_index; /* hash of modified_list, or NULL */
   int              numOfViews;
   DirectoryView  * directoryView;
} Directory;


/* data for keeping track of sticky background procs */
typedef struct _spd
{
   pid_t child;
   int pipe_s2m_fd;
   int pipe_m2s_fd;
   Boolean idle;
   int generation;               /* db_generation when it was forked */
   struct _spd *next;
} StickyProcDesc;

/* data for callback routines that handle background processes */
typedef struct
{
   Directory *directory;
   pid_t child;
   StickyProcDesc *sticky_proc;
   ActivityStatus activity;
} PipeCallbackData;


/* background procedure */
typedef int (*DirBackgroundProc)(int, Directory *, ActivityStatus);

extern void _DtFlushIconFileCache(String path);


/*--------------------------------------------------------------------
 * Static Function Declarations
 *------------------------------------------------------------------*/

static void TimerEvent(
                        XtPointer client_data,
                        XtIntervalId *id);
static void ScheduleActivity(
			Directory *directory);
static int WritePosInfoProcess(
			int pipe_fd,
			Directory *directory,
			ActivityStatus activity);
static int ReadDirectoryProcess(
			int pipe_fd,
			Directory *directory,
			ActivityStatus activity);
static int UpdateAllProcess(
			int pipe_fd,
			Directory *directory,
			ActivityStatus activity);
static int UpdateSomeProcess(
			int pipe_fd,
			Directory *directory,
			ActivityStatus activity);
static int TimerEventProcess(
			int pipe_fd,
			Directory *directory,
			ActivityStatus activity);
static int CheckDesktopProcess(
			int pipe_fd,
			Directory *directory,
			ActivityStatus activity);
static void WritePosInfoPipeCallback(
			XtPointer client_data,
			int *fd,
			XtInputId *id);
static void ReaddirPipeCallback(
			XtPointer client_data,
			int *fd,
			XtInputId *id);
static void TimerPipeCallback(
			XtPointer client_data,
			int *fd,
			XtInputId *id);
static void CheckDesktopPipeCallback(
			XtPointer client_data,
			int *fd,
			XtInputId *id);
static Boolean SkipRefresh(
			Directory *directory);
static void ReadDirectoryFiles(
			Widget w,
			Directory *directory);
static void EventTimer(
			XtPointer client_data,
			XtIntervalId *id);
static Boolean SomeWindowMapped(void);
static void FreeDirectory(
			Directory *directory);
static void SelectDesktopFile(FileMgrData *fmd);



/*--------------------------------------------------------------------
 * Static Data
 *------------------------------------------------------------------*/

int maxDirectoryProcesses = 10;
int maxRereadProcesses = 5;
int maxRereadProcsPerTick = 1;

XtIntervalId checkBrokenLinkTimerId = None;

static Directory ** directory_set = NULL;
static int          directory_count = 0;
static int          directory_set_size = 0;
static char       * positionFileName = NULL;
static XtAppContext app_context = None;
static int          tickTime = 0;
static Boolean      timer_suspended = False;
static int          tick_count = 0;
static long         lastLinkCheckMs = 0;
static int          db_generation = 0;   /* bumped by UpdateDirectorySet */
static unsigned long lru_clock = 0;
static int          inotify_fd = -1;
static XtIntervalId event_timer = 0;
static long         event_timer_due = 0;
static XtIntervalId poll_timer = 0;      /* the pending TimerEvent, or 0 */
static long         poll_timer_due = 0;
static Directory  dummy_dir_struct =
{
  "dummy_host",
  "dummy_directory",
  "dummy_path",
  NULL,
  False,
  activity_idle
};
static Directory *dummy_directory = &dummy_dir_struct;

static struct
{
   DirBackgroundProc main;
   XtInputCallbackProc callback;
   Boolean sticky;
   StickyProcDesc *sticky_procs;
} ActivityTable[] =
{
  { WritePosInfoProcess,  WritePosInfoPipeCallback, False,NULL },/* writing_posinfo*/
  { ReadDirectoryProcess, ReaddirPipeCallback, False,NULL },     /* reading */
  { UpdateAllProcess,     ReaddirPipeCallback, False,NULL },     /* update_all */
  { UpdateSomeProcess,    ReaddirPipeCallback, False,NULL },     /* update_some */
  { TimerEventProcess,    TimerPipeCallback, True, NULL },       /* checking_links */
  { CheckDesktopProcess,  CheckDesktopPipeCallback, True, NULL },/* checking_desktop */
  { TimerEventProcess,    TimerPipeCallback, True, NULL },  /* checking_dir */
  { NULL,                 NULL, False, NULL }               /* idle */
};


/*====================================================================
 *
 * Helpers for the background processes
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 *  PipeBuf
 *	Buffered pipe output.  A background process assembles each
 *	message in a PipeBuf and hands it to the kernel with a single
 *	write(), instead of one write() per field (and two signal()
 *	calls per string in PipeWriteString).  The byte stream is the
 *	same as before, so the readers are unchanged.
 *------------------------------------------------------------------*/

typedef struct
{
   char   *data;
   size_t  len;
   size_t  size;
} PipeBuf;

static void
PipeBufAdd(
	PipeBuf *pb,
	const void *p,
	size_t n)
{
   if (pb->len + n > pb->size)
   {
      size_t size = pb->size ? pb->size : 1024;

      while (size < pb->len + n)
         size *= 2;
      pb->data = XtRealloc(pb->data, size);
      pb->size = size;
   }
   memcpy(pb->data + pb->len, p, n);
   pb->len += n;
}

/* same encoding as PipeWriteString: a short length, then the bytes */
static void
PipeBufAddString(
	PipeBuf *pb,
	const char *s)
{
   short len = (s == NULL) ? 0 : strlen(s);

   PipeBufAdd(pb, &len, sizeof(short));
   if (len > 0)
      PipeBufAdd(pb, s, len);
}

static void
PipeBufAddMsg(
	PipeBuf *pb,
	short msg)
{
   PipeBufAdd(pb, &msg, sizeof(short));
}

static int
WriteAll(
	int fd,
	const void *buf,
	size_t n)
{
   const char *p = buf;

   while (n > 0)
   {
      ssize_t rc = write(fd, p, n);

      if (rc > 0)
      {
         p += rc;
         n -= rc;
      }
      else if (rc < 0 && errno == EINTR)
         continue;
      else
         return -1;
   }
   return 0;
}

static int
PipeBufFlush(
	int fd,
	PipeBuf *pb)
{
   int rc = 0;

   if (pb->len > 0)
      rc = WriteAll(fd, pb->data, pb->len);
   pb->len = 0;
   return rc;
}

static void
PipeBufFree(
	PipeBuf *pb)
{
   XtFree(pb->data);
   pb->data = NULL;
   pb->len = pb->size = 0;
}


/*--------------------------------------------------------------------
 *  NameIndex
 *	A hash index over an array of file names, so that matching the
 *	entries of a directory against N known (or modified) names is
 *	O(N) instead of O(N^2).
 *	NameIndexFind returns the lowest index i whose key equals name
 *	and whose taken[i] is clear (taken may be NULL), or -1.  With
 *	linear probing, equal keys are met in the order they were added,
 *	which keeps the first-match semantics of the list scans it
 *	replaces.
 *------------------------------------------------------------------*/

typedef struct _NameIndex
{
   int          *slots;      /* index into keys, or -1 */
   unsigned int  mask;
} NameIndex;

static unsigned int
NameHash(
	const char *s)
{
   unsigned int h = 2166136261u;      /* FNV-1a */

   while (*s)
   {
      h ^= (unsigned char)*s++;
      h *= 16777619u;
   }
   return h;
}

static void
NameIndexInit(
	NameIndex *ni,
	char **keys,
	int n)
{
   unsigned int size = 16;
   unsigned int h;
   int i;

   while (size < 2 * (unsigned int)n)
      size <<= 1;
   ni->mask = size - 1;
   ni->slots = (int *) XtMalloc(size * sizeof(int));
   for (h = 0; h < size; h++)
      ni->slots[h] = -1;

   for (i = 0; i < n; i++)
   {
      h = NameHash(keys[i] ? keys[i] : "") & ni->mask;
      while (ni->slots[h] >= 0)
         h = (h + 1) & ni->mask;
      ni->slots[h] = i;
   }
}

static int
NameIndexFind(
	NameIndex *ni,
	char **keys,
	const char *name,
	const char *taken)
{
   unsigned int h = NameHash(name) & ni->mask;
   int i;

   while ((i = ni->slots[h]) >= 0)
   {
      if ((taken == NULL || !taken[i]) &&
          strcmp(keys[i] ? keys[i] : "", name) == 0)
         return i;
      h = (h + 1) & ni->mask;
   }
   return -1;
}

static void
NameIndexFree(
	NameIndex *ni)
{
   XtFree((char *)ni->slots);
   ni->slots = NULL;
}

static void NameIndexAdd(NameIndex *ni, char **keys, int i);

/* free and clear the modified_list of a directory */
static void
FreeModifiedList(
	Directory *directory)
{
   int i;

   for (i = 0; i < directory->modified_count; i++)
      XtFree(directory->modified_list[i]);
   XtFree((char *)directory->modified_list);
   directory->modified_list = NULL;
   directory->modified_count = 0;
   directory->modified_size = 0;
   if (directory->modified_index)
   {
      NameIndexFree(directory->modified_index);
      XtFree((char *)directory->modified_index);
      directory->modified_index = NULL;
   }
}

/* add keys[i], the last of i+1 keys, growing the table as needed */
static void
NameIndexAdd(
	NameIndex *ni,
	char **keys,
	int i)
{
   unsigned int h;

   if (ni->slots == NULL || 2 * (unsigned int)(i + 1) > ni->mask + 1)
   {
      NameIndexFree(ni);
      NameIndexInit(ni, keys, i + 1);
      return;
   }
   h = NameHash(keys[i] ? keys[i] : "") & ni->mask;
   while (ni->slots[h] >= 0)
      h = (h + 1) & ni->mask;
   ni->slots[h] = i;
}


/*--------------------------------------------------------------------
 *  TypeAttr cache
 *	While a background process types the entries of a directory,
 *	the results of DtActionExists() and of the LABEL attribute are
 *	remembered per data type: they depend only on the type, and a
 *	directory has far fewer types than entries.  The cache lives for
 *	one batch only (TypeAttrCacheBegin/End), so it can never hide a
 *	database reload; outside a batch every lookup goes to the
 *	database as before.
 *------------------------------------------------------------------*/

typedef struct _TypeAttr
{
   struct _TypeAttr *next;
   char             *type;
   Boolean           is_action;
   char             *label;      /* LABEL attribute, if not an action */
} TypeAttr;

#define TYPE_ATTR_BUCKETS 64

static TypeAttr *type_attr_cache[TYPE_ATTR_BUCKETS];
static Boolean   type_attr_cache_active = False;

static void
TypeAttrCacheBegin(void)
{
   type_attr_cache_active = True;
}

static void
TypeAttrCacheEnd(void)
{
   TypeAttr *ta, *next;
   int i;

   for (i = 0; i < TYPE_ATTR_BUCKETS; i++)
   {
      for (ta = type_attr_cache[i]; ta; ta = next)
      {
         next = ta->next;
         XtFree(ta->type);
         XtFree(ta->label);
         XtFree((char *)ta);
      }
      type_attr_cache[i] = NULL;
   }
   type_attr_cache_active = False;
}

static TypeAttr *
TypeAttrLookup(
	char *type)
{
   TypeAttr *ta;
   unsigned int b;

   if (!type_attr_cache_active)
      return NULL;

   b = NameHash(type) % TYPE_ATTR_BUCKETS;
   for (ta = type_attr_cache[b]; ta; ta = ta->next)
      if (strcmp(ta->type, type) == 0)
         return ta;

   ta = (TypeAttr *) XtMalloc(sizeof(TypeAttr));
   ta->type = XtNewString(type);
   ta->label = NULL;
   ta->is_action = DtActionExists(type);
   if (!ta->is_action)
   {
      char *ptr = DtDtsDataTypeToAttributeValue(type, DtDTS_DA_LABEL, NULL);

      if (ptr)
      {
         ta->label = XtNewString(ptr);
         DtDtsFreeAttributeValue(ptr);
      }
   }
   ta->next = type_attr_cache[b];
   type_attr_cache[b] = ta;
   return ta;
}


/*--------------------------------------------------------------------
 *  FileDataBatch
 *	Collects FileData2 records and sends them as PIPEMSG_FILEDATA3
 *	messages of up to FILEDATABUF records, each with one write().
 *------------------------------------------------------------------*/

#define PIPEMSG_HDR_LEN (2*sizeof(short) + sizeof(int))

typedef struct
{
   int    pipe_fd;
   short  count;
   char  *ptr;                 /* where the next record goes */
   char   buffer[PIPEMSG_HDR_LEN + FILEDATABUF * sizeof(FileData2)];
} FileDataBatch;

static FileDataBatch *
FileDataBatchCreate(
	int pipe_fd)
{
   FileDataBatch *b = (FileDataBatch *) XtMalloc(sizeof(FileDataBatch));

   b->pipe_fd = pipe_fd;
   b->count = 0;
   b->ptr = b->buffer + PIPEMSG_HDR_LEN;
   return b;
}

/* send the records collected so far; update_due asks for a status update */
static int
FileDataBatchFlush(
	FileDataBatch *b,
	Boolean update_due)
{
   short msg = PIPEMSG_FILEDATA3;
   short count = b->count;
   int len = b->ptr - (b->buffer + PIPEMSG_HDR_LEN);
   int rc;

   if (b->count == 0)
      return 0;
   if (update_due)
      count |= 0x8000;

   memcpy(b->buffer, &msg, sizeof(short));
   memcpy(b->buffer + sizeof(short), &count, sizeof(short));
   memcpy(b->buffer + 2*sizeof(short), &len, sizeof(int));
   rc = WriteAll(b->pipe_fd, b->buffer, b->ptr - b->buffer);

   b->count = 0;
   b->ptr = b->buffer + PIPEMSG_HDR_LEN;
   return rc;
}

/* room for the next record (sizeof(FileData2) is the most it can take) */
static FileData2 *
FileDataBatchNext(
	FileDataBatch *b)
{
   return (FileData2 *) b->ptr;
}

/* account for a record of length len written at FileDataBatchNext() */
static int
FileDataBatchAdd(
	FileDataBatch *b,
	int len)
{
   b->ptr += len;
   if (++b->count == FILEDATABUF)
      return FileDataBatchFlush(b, False);
   return 0;
}


/*--------------------------------------------------------------------
 *  Time stamps
 *------------------------------------------------------------------*/

static long
MonotonicMs(void)
{
   struct timespec ts;

   clock_gettime(CLOCK_MONOTONIC, &ts);
   return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static Boolean
TimespecEqual(
        const struct timespec *a,
        const struct timespec *b)
{
   return a->tv_sec == b->tv_sec && a->tv_nsec == b->tv_nsec;
}

/*
 * The time stamps of a directory, as the background processes send them
 * through the pipe.  st_mtime alone misses a second change within the
 * same second, so the nanosecond st_mtim and st_ctim are compared.
 */
typedef struct
{
   long            modify_time;  /* st_mtime, 0 if the stat failed */
   struct timespec mtim;
   struct timespec ctim;
   int             local_fs;     /* not a network file system */
} DirStamp;

static void
DirStampSet(
        DirStamp *ds,
        const struct stat *st,
        Boolean local_fs)
{
   memset(ds, 0, sizeof(*ds));
   if (st != NULL)
   {
      ds->modify_time = st->st_mtime;
      ds->mtim = st->st_mtim;
      ds->ctim = st->st_ctim;
   }
   ds->local_fs = local_fs;
}

/* remember the time stamps of the directory as it was (re)read */
static void
DirStampStore(
        Directory *directory,
        const DirStamp *ds)
{
   if (ds->modify_time != 0)
   {
      directory->modify_time = ds->modify_time;
      directory->mtim = ds->mtim;
      directory->ctim = ds->ctim;
   }
}


/*--------------------------------------------------------------------
 *  IsLocalFileSystem
 *	Is the open directory fd on a file system where inotify sees
 *	every change?  It does not see changes that other clients make
 *	to network file systems, so those are polled.
 *------------------------------------------------------------------*/

static Boolean
IsLocalFileSystem(
        int fd)
{
#ifdef DT_USE_INOTIFY
   struct statfs sfs;

   if (fd < 0 || fstatfs(fd, &sfs) != 0)
      return False;

   switch ((unsigned int) sfs.f_type)
   {
      case 0x6969u:      /* NFS */
      case 0x517Bu:      /* SMB */
      case 0xFF534D42u:  /* CIFS */
      case 0xFE534D42u:  /* SMB2 */
      case 0x65735546u:  /* FUSE (sshfs, ...) */
      case 0x5346414Fu:  /* AFS */
      case 0x6B414653u:  /* kAFS */
      case 0x73757245u:  /* Coda */
      case 0x564Cu:      /* NCP */
      case 0x00C36400u:  /* Ceph */
      case 0x01021997u:  /* 9P */
      case 0x01161970u:  /* GFS2 */
      case 0x7461636Fu:  /* OCFS2 */
      case 0x0BD00BD0u:  /* Lustre */
      case 0x47504653u:  /* GPFS */
      case 0x19830326u:  /* BeeGFS */
         return False;
      default:
         return True;
   }
#else
   return False;
#endif
}


/*--------------------------------------------------------------------
 *  RestartTimer
 *	Start the poll timer again if it was suspended (see TimerEvent).
 *------------------------------------------------------------------*/

static void
PollTimerArm(
        long ms)
{
   if (poll_timer != 0)
      XtRemoveTimeOut(poll_timer);
   poll_timer_due = MonotonicMs() + ms;
   poll_timer = XtAppAddTimeOut(app_context, ms, TimerEvent, NULL);
   timer_suspended = False;
}

/*
 * (Also when the timer only waits for the next link check, which can be
 * checkBrokenLink seconds away: a directory that needs polling now, e.g.
 * one that lost its watch or is on NFS, must not wait that long.)
 */
static void
RestartTimer(void)
{
   if (tickTime == 0 || !SomeWindowMapped())
      return;
   if (timer_suspended ||
       (poll_timer != 0 && poll_timer_due - MonotonicMs() > tickTime * 1000L))
      PollTimerArm(tickTime * 1000L);
}


/*--------------------------------------------------------------------
 *  inotify
 *	Each directory read from a local file system gets a watch.  An
 *	event marks the directory (ev_pending); EventTimer then starts an
 *	update (which re-stats the entries and sends only what changed),
 *	collecting the events of EVENT_DELAY_MS and starting at most one
 *	update per directory every EVENT_INTERVAL_MS (longer while it keeps
 *	changing, up to rereadTime).  A read or update
 *	that starts clears the mark, after taking in the queued events:
 *	it sees every change made before it started.
 *	Directories that lose their watch (deleted, renamed, unmounted)
 *	are polled until a read finds them again.
 *------------------------------------------------------------------*/

#ifdef DT_USE_INOTIFY
#define INOTIFY_MASK (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | \
                      IN_CLOSE_WRITE | IN_ATTRIB | IN_DELETE_SELF |        \
                      IN_MOVE_SELF | IN_ONLYDIR | IN_EXCL_UNLINK)
#endif

static void
EventTimerArm(
        long ms)
{
   long due = MonotonicMs() + ms;

   if (event_timer != 0)
   {
      if (due >= event_timer_due)
         return;
      XtRemoveTimeOut(event_timer);
   }
   event_timer_due = due;
   event_timer = XtAppAddTimeOut(app_context, ms, EventTimer, NULL);
}

/* the watch of all directories with watch descriptor wd is gone */
static void
DirectoryWatchLost(
        int wd)
{
   int i;

   for (i = 0; i < directory_count; i++)
   {
      if (directory_set[i]->wd == wd)
      {
         directory_set[i]->wd = 0;
         directory_set[i]->ev_pending = True;
      }
   }
   RestartTimer();
}

static void
DirectoryEventsHandle(
        char *buf,
        ssize_t len)
{
#ifdef DT_USE_INOTIFY
   char *p = buf;
   Boolean any = False;
   int i;

   while (p + sizeof(struct inotify_event) <= buf + len)
   {
      struct inotify_event *ev = (struct inotify_event *) p;
      const char *name = (ev->len > 0) ? ev->name : NULL;

      p += sizeof(struct inotify_event) + ev->len;
      any = True;

      if (ev->mask & IN_Q_OVERFLOW)
      {
         /* events were lost: everything may have changed */
         for (i = 0; i < directory_count; i++)
            if (directory_set[i]->wd > 0)
               directory_set[i]->ev_pending = True;
         continue;
      }

      /*
       * Saving icon positions rewrites the position file in the
       * directory itself; that alone does not make a refresh.
       */
      if (name != NULL && positionFileName != NULL &&
          !(ev->mask & (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO)) &&
          strcmp(name, positionFileName) == 0)
         continue;

      if (ev->mask & IN_MOVE_SELF)
      {
         /* the path now names something else, or nothing */
         inotify_rm_watch(inotify_fd, ev->wd);
         DirectoryWatchLost(ev->wd);
      }
      else if (ev->mask & IN_IGNORED)
         DirectoryWatchLost(ev->wd);
      else
      {
         for (i = 0; i < directory_count; i++)
            if (directory_set[i]->wd == ev->wd)
               directory_set[i]->ev_pending = True;
      }
   }

   if (any)
      EventTimerArm(EVENT_DELAY_MS);
#endif
}

/* take in all queued events */
static void
DirectoryEventsRead(void)
{
   long buf[1024];   /* (aligned for struct inotify_event) */
   ssize_t n;

   if (inotify_fd < 0)
      return;

   for (;;)
   {
      n = read(inotify_fd, buf, sizeof(buf));
      if (n > 0)
         DirectoryEventsHandle((char *)buf, n);
      else if (n < 0 && errno == EINTR)
         continue;
      else
         break;   /* EAGAIN: no more events */
   }
}

static void
DirectoryEventsCallback(
        XtPointer client_data,
        int *fd,
        XtInputId *id)
{
   DirectoryEventsRead();
}

/*
 * Watch a directory that was just read (if it is on a local file
 * system and isn't watched yet).  A change between the read and the
 * watch shows in the time stamps.
 */
static void
DirectoryWatch(
        Directory *directory)
{
#ifdef DT_USE_INOTIFY
   struct stat st;
   int wd;

   if (inotify_fd < 0 || directory->wd > 0 || !directory->local_fs ||
       directory->path_name == NULL || directory->path_name[0] == '\0')
      return;

   wd = inotify_add_watch(inotify_fd, directory->path_name, INOTIFY_MASK);
   if (wd <= 0)
      return;
   directory->wd = wd;
   DPRINTF(("DirectoryWatch: %s wd %d\n", directory->path_name, wd));

   if (stat(directory->path_name, &st) != 0 ||
       !TimespecEqual(&st.st_mtim, &directory->mtim) ||
       !TimespecEqual(&st.st_ctim, &directory->ctim))
   {
      directory->ev_pending = True;
      EventTimerArm(EVENT_DELAY_MS);
   }
#endif
}

static void
DirectoryUnwatch(
        Directory *directory)
{
#ifdef DT_USE_INOTIFY
   int wd = directory->wd;
   int i;

   if (wd <= 0)
      return;
   directory->wd = 0;

   /* two cached paths can name one directory, and share the watch */
   for (i = 0; i < directory_count; i++)
      if (directory_set[i] != directory && directory_set[i]->wd == wd)
         return;
   inotify_rm_watch(inotify_fd, wd);
#endif
}

static void
DirectoryEventsInit(void)
{
#ifdef DT_USE_INOTIFY
   /* rereadTime 0 means: no automatic refresh */
   if (rereadTime <= 0 || inotify_fd >= 0)
      return;

   inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
   if (inotify_fd < 0)
      return;
   XtAppAddInput(app_context, inotify_fd, (XtPointer)XtInputReadMask,
                 DirectoryEventsCallback, NULL);
#endif
}

/* does some view of this directory show in a mapped window? */
static Boolean
DirectoryMapped(
        Directory *directory)
{
   int i;

   for (i = 0; i < directory->numOfViews; i++)
      if (directory->directoryView[i].mapped)
         return True;
   return False;
}

/*
 * Start an update of every directory that changed.
 */
static void
EventTimer(
        XtPointer client_data,
        XtIntervalId *id)
{
   Directory *directory;
   long now, wait, retry = 0;
   int i;

   event_timer = 0;

   /* don't change any directories while a drag is active (see TimerEvent) */
   if (dragActive)
   {
      EventTimerArm(EVENT_RETRY_MS);
      return;
   }

   now = MonotonicMs();
   for (i = 0; i < directory_count; i++)
   {
      directory = directory_set[i];
      if (!directory->ev_pending)
         continue;

      /* a cached directory nobody looks at: update it when shown again */
      if (!directory->viewed)
      {
         directory->stale = True;
         directory->ev_pending = False;
         continue;
      }

      /*
       * A read or update that hasn't started yet will see the change;
       * one that is running re-arms this timer when it is done.
       */
      if (directory->busy[activity_reading] ||
          directory->busy[activity_update_all])
         continue;

      /* being modified, or not shown right now */
      if (SkipRefresh(directory))
      {
         /* (FileWindowMapUnmap re-arms the timer when it is mapped) */
         if (DirectoryMapped(directory))
            wait = EVENT_RETRY_MS;
         else
            continue;
      }
      else
      {
         if (directory->ev_interval < EVENT_INTERVAL_MS)
            directory->ev_interval = EVENT_INTERVAL_MS;
         wait = directory->last_ev_refresh + directory->ev_interval - now;
      }

      if (directory->last_ev_refresh != 0 && wait > 0)
      {
         if (retry == 0 || wait < retry)
            retry = wait;
         continue;
      }

      /* back off while it changes all the time, recover when it calms */
      if (directory->last_ev_refresh != 0 &&
          now - directory->last_ev_refresh < 2 * directory->ev_interval)
      {
         directory->ev_interval *= 2;
         if (directory->ev_interval > rereadTime * 1000L)
            directory->ev_interval = rereadTime * 1000L;
         if (directory->ev_interval < EVENT_INTERVAL_MS)
            directory->ev_interval = EVENT_INTERVAL_MS;
      }
      else
         directory->ev_interval = EVENT_INTERVAL_MS;

      DPRINTF(("EventTimer: %s changed\n", directory->directory_name));
      directory->last_ev_refresh = now;
      directory->busy[activity_update_all] = True;
      ScheduleActivity(directory);
   }

   if (retry > 0)
      EventTimerArm(retry);
}


/*--------------------------------------------------------------------
 *  Sticky background processes
 *	The directory check, the link check and the desktop check keep
 *	their process between runs (see ScheduleDirectoryActivity).  The
 *	link and desktop checks need data from the main process; a reused
 *	process gets it with its request: after the path name,
 *	  link check:     int n; n * (string name, int link kind)
 *	  desktop check:  int n; n * (string host, string dir, string file,
 *	                              int physical type, string logical type)
 *	A process forked for the request uses its own copy of the data.
 *------------------------------------------------------------------*/

/* link kinds, see TimerEventProcess */
#define LINK_VALID      1
#define LINK_RECURSIVE  2
#define LINK_BROKEN     3

typedef struct
{
   char *name;
   int   kind;
} LinkState;

typedef struct
{
   char *host;
   char *dir_linked_to;
   char *file_name;
   int   physical_type;
   char *logical_type;
} DesktopState;

/* in a reused (sticky) process: the data of the last request */
static LinkState    *sticky_links = NULL;
static int           sticky_link_count = -1;    /* -1: none received */
static DesktopState *sticky_desktop = NULL;
static int           sticky_desktop_count = -1;

static int
LinkKind(
        FileData *file_data)
{
   if (file_data->logical_type != NULL &&
       strcmp(file_data->logical_type, LT_BROKEN_LINK) == 0)
      return LINK_BROKEN;
   if (file_data->logical_type != NULL &&
       strcmp(file_data->logical_type, LT_RECURSIVE_LINK) == 0)
      return LINK_RECURSIVE;
   return LINK_VALID;
}

/* main process: add the data a sticky process needs to its request */
static void
StickyAddRequest(
        PipeBuf *pb,
        Directory *directory,
        ActivityStatus activity)
{
   FileData *file_data;
   int i, n;

   PipeBufAddString(pb, directory->path_name);

   if (activity == activity_checking_links)
   {
      n = 0;
      for (file_data = directory->file_data; file_data;
           file_data = file_data->next)
         if (file_data->link != NULL && file_data->file_name != NULL)
            n++;
      PipeBufAdd(pb, &n, sizeof(int));
      for (file_data = directory->file_data; file_data;
           file_data = file_data->next)
      {
         if (file_data->link != NULL && file_data->file_name != NULL)
         {
            int kind = LinkKind(file_data);

            PipeBufAddString(pb, file_data->file_name);
            PipeBufAdd(pb, &kind, sizeof(int));
         }
      }
   }
   else if (activity == activity_checking_desktop)
   {
      n = desktop_data->numIconsUsed;
      PipeBufAdd(pb, &n, sizeof(int));
      for (i = 0; i < n; i++)
      {
         DesktopRec *desktopWindow = desktop_data->desktopWindows[i];
         FileData *old_data = desktopWindow->file_view_data->file_data;
         int physical_type = old_data->physical_type;

         PipeBufAddString(pb, desktopWindow->host);
         PipeBufAddString(pb, desktopWindow->dir_linked_to);
         PipeBufAddString(pb, desktopWindow->file_name);
         PipeBufAdd(pb, &physical_type, sizeof(int));
         PipeBufAddString(pb, old_data->logical_type);
      }
   }
}

/* (an empty string comes through the pipe as NULL) */
static char *
PipeReadStringNonNull(
        int fd)
{
   char *s = PipeReadString(fd);

   return s ? s : XtNewString("");
}

/* sticky process: read the rest of a request (after the path name) */
static int
StickyReadRequest(
        int fd,
        ActivityStatus activity)
{
   int i, n;

   if (activity == activity_checking_links)
   {
      for (i = 0; i < sticky_link_count; i++)
         XtFree(sticky_links[i].name);
      XtFree((char *)sticky_links);
      sticky_links = NULL;
      sticky_link_count = 0;

      if (PipeRead(fd, &n, sizeof(int)) != sizeof(int) || n < 0)
         return -1;
      sticky_links = (LinkState *) XtMalloc((n + 1) * sizeof(LinkState));
      for (i = 0; i < n; i++)
      {
         sticky_links[i].name = PipeReadStringNonNull(fd);
         sticky_links[i].kind = 0;
         sticky_link_count = i + 1;
         if (PipeRead(fd, &sticky_links[i].kind, sizeof(int)) != sizeof(int))
            return -1;
      }
   }
   else if (activity == activity_checking_desktop)
   {
      for (i = 0; i < sticky_desktop_count; i++)
      {
         XtFree(sticky_desktop[i].host);
         XtFree(sticky_desktop[i].dir_linked_to);
         XtFree(sticky_desktop[i].file_name);
         XtFree(sticky_desktop[i].logical_type);
      }
      XtFree((char *)sticky_desktop);
      sticky_desktop = NULL;
      sticky_desktop_count = 0;

      if (PipeRead(fd, &n, sizeof(int)) != sizeof(int) || n < 0)
         return -1;
      sticky_desktop =
         (DesktopState *) XtMalloc((n + 1) * sizeof(DesktopState));
      for (i = 0; i < n; i++)
      {
         DesktopState *d = &sticky_desktop[i];

         d->host = PipeReadStringNonNull(fd);
         d->dir_linked_to = PipeReadStringNonNull(fd);
         d->file_name = PipeReadStringNonNull(fd);
         d->physical_type = 0;
         d->logical_type = NULL;
         sticky_desktop_count = i + 1;
         if (PipeRead(fd, &d->physical_type, sizeof(int)) != sizeof(int))
            return -1;
         d->logical_type = PipeReadString(fd);
      }
   }
   return 0;
}

/* end a sticky process and forget it */
static void
StickyProcRemove(
        ActivityStatus activity,
        StickyProcDesc *p,
        Boolean tell_it)
{
   StickyProcDesc **lp;

   for (lp = &ActivityTable[activity].sticky_procs; *lp; lp = &(*lp)->next)
   {
      if (*lp == p)
      {
         *lp = p->next;
         break;
      }
   }

   DPRINTF2(("StickyProcRemove: end sticky proc %ld\n", (long)p->child));
   if (tell_it)
      PipeWriteString(p->pipe_m2s_fd, NULL);
   close(p->pipe_s2m_fd);
   close(p->pipe_m2s_fd);
   XtFree((char *)p);
}

/* after a database reload: end idle sticky procs that type with the old one */
static void
StickyProcsRetire(void)
{
   StickyProcDesc *p, *next;
   int activity;

   for (activity = 0; activity < activity_idle; activity++)
   {
      for (p = ActivityTable[activity].sticky_procs; p; p = next)
      {
         next = p->next;
         if (p->idle && p->generation != db_generation)
            StickyProcRemove(activity, p, True);
      }
   }
}


/*--------------------------------------------------------------------
 *  Directory cache
 *------------------------------------------------------------------*/

/* remove directory_set[i] from the cache */
static void
DirectoryCacheRemove(
        int i)
{
   int k;

   DPRINTF(("DirectoryCacheRemove: removing %s:%s\n",
            directory_set[i]->host_name, directory_set[i]->directory_name));

   FreeDirectory(directory_set[i]);
   for (k = i; k < directory_count - 1; k++)
      directory_set[k] = directory_set[k + 1];
   directory_count--;
}

/* keep at most DIR_CACHE_MAX unviewed directories, least recently used out */
static void
DirectoryCacheTrim(void)
{
   int i, n, files, oldest;

   for (;;)
   {
      n = files = 0;
      oldest = -1;
      for (i = 0; i < directory_count; i++)
      {
         if (directory_set[i]->lru_stamp == 0)
            continue;
         n++;
         files += directory_set[i]->file_count;
         /*
          * (not one with a background process running: its callback
          * would take a new Directory at the same address for it)
          */
         if (directory_set[i]->activity != activity_idle)
            continue;
         if (oldest < 0 ||
             directory_set[i]->lru_stamp < directory_set[oldest]->lru_stamp)
            oldest = i;
      }
      if (oldest < 0 || (n <= DIR_CACHE_MAX && files <= DIR_CACHE_MAX_FILES))
         break;
      DirectoryCacheRemove(oldest);
   }
}

/*
 * A cached directory is shown again: bring it up to date.  The cached
 * entries show at once; an update re-stats them, and the views are
 * redrawn only if something changed.  A watched directory without
 * events and without links needs nothing.
 */
static void
DirectoryRevalidate(
        Directory *directory)
{
   directory->lru_stamp = 0;

   if (directory->busy[activity_reading])
      directory->reread_needed = directory->stale = False;
   else if (directory->reread_needed)
   {
      directory->reread_needed = directory->stale = False;
      ReadDirectoryFiles(NULL, directory);
   }
   else if (directory->wd <= 0 || directory->stale ||
            directory->ev_pending || directory->has_links)
   {
      directory->stale = False;
      directory->busy[activity_update_all] = True;
      ScheduleActivity(directory);
   }
}


/*====================================================================
 *
 * Initialization routines
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 *  InitializePositionFileName
 *	Initialize the name under which the position info is stored.
 *------------------------------------------------------------------*/

static void
InitializePositionFileName(void)
{
   struct passwd * pwInfo;

   /* Determine the name under which the position info is stored */
   if (positionFileName == NULL)
   {

      pwInfo = getpwuid(getuid());
      positionFileName = XtMalloc(strlen(pwInfo->pw_name) +
                                  strlen(POSITION_FILE_PREFIX) + 1);
      sprintf(positionFileName, "%s%s", POSITION_FILE_PREFIX, pwInfo->pw_name);
   }
}


/*--------------------------------------------------------------------
 *  InitializeDirectoryRead
 *	Set up a timer used to automatically check the read in
 *	directories to see if they have been modified.
 *------------------------------------------------------------------*/

void
InitializeDirectoryRead(
        Widget widget )

{
   /* remeber application context */
   app_context = XtWidgetToApplicationContext(widget);

   /* start timer to check for modified directories and broken links */
   tick_count = 0;
   lastLinkCheckMs = MonotonicMs();

   if (rereadTime != 0)
     tickTime = rereadTime;
   else if (checkBrokenLink != 0)
     tickTime = checkBrokenLink;
   else
     tickTime = 0;

   /* watch local directories instead of polling them */
   DirectoryEventsInit();

   if (tickTime != 0)
      PollTimerArm(tickTime * 1000L);

   /* start timer to check for broken desktop objects */
   if( desktop_data->numIconsUsed > 0
       && checkBrokenLink != 0
     )
   {
     checkBrokenLinkTimerId = XtAppAddTimeOut( app_context,
                                               checkBrokenLink * 1000,
                                               TimerEventBrokenLinks,
                                               NULL);
   }
   else
   {
     checkBrokenLinkTimerId = None;
   }
}


/*====================================================================
 *
 * Utiltiy functions
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 *  FindDirectory
 *    Given a host & directory name, find the directory in our cache.
 *------------------------------------------------------------------*/

static Directory *
FindDirectory(
        char *host_name,
        char *directory_name)
{
   int i;

   /* See if the directory is in the directory set.  First, compare    */
   /* the names from the directory entries ONLY.  There will be one    */
   /* directory entry for every directory of a different name.  This   */
   /* may mean that there is more than one directory entry for a       */
   /* single directory (ie. if there is a directory that is a link or  */
   /* a mount point in the system).                                    */
   /* If this doesn't succeed, we may be getting a ToolTalk resolved   */
   /* name. So run the comparison again but this time compare the      */
   /* incoming name to the tt_path_name in the directory entries.      */
   /* This algorithm has a limitation in that if ToolTalk has resolved */
   /* a path name, the match will occur for the first entry in the     */
   /* directory set whose tt_path_name matches our name, this MAY NOT  */
   /* be the directory where the activity originated.  The user should */
   /* only notice in the case where automatic refresh is turned off.   */
   for (i = 0; i < directory_count; i++)
   {
      if (strcmp (host_name, directory_set[i]->host_name) == 0 &&
          strcmp (directory_name, directory_set[i]->directory_name) == 0)
      {
         return directory_set[i];
      }
   }

   for (i = 0; i < directory_count; i++)
   {
      if (directory_set[i]->tt_path_name != NULL &&
          strcmp (host_name, home_host_name) == 0 &&
          strcmp (directory_name, directory_set[i]->tt_path_name) == 0)
      {
         return directory_set[i];
      }
   }

   /* not found */
   return NULL;
}


/*--------------------------------------------------------------------
 *  DirectoryGone
 *    Check if a directory has been removed from the cache.
 *------------------------------------------------------------------*/

static Boolean
DirectoryGone(
        Directory *directory)
{
   int i;

   for (i = 0; i < directory_count; i++)
      if (directory_set[i] == directory)
         return False;

   return True;
}


/*--------------------------------------------------------------------
 *  FreeFileData
 *	Free FileData structure.
 *------------------------------------------------------------------*/

void
FreeFileData(
        FileData *file_data,
        Boolean free_all)
{
   XtFree(file_data->file_name);
   file_data->file_name = NULL;

   XtFree(file_data->action_name);
   file_data->action_name = NULL;

   DtDtsFreeDataType(file_data->logical_type);
   file_data->logical_type = NULL;

   if ( file_data->final_link != NULL &&
        file_data->final_link != file_data->link)
   {
      XtFree(file_data->final_link);
      file_data->final_link = NULL;
   }

   if (file_data->link != NULL )
   {
     XtFree(file_data->link);
     file_data->link = NULL;
   }

   if (free_all)
      XtFree((char *)file_data);
}


/*--------------------------------------------------------------------
 *  FreeDirectory
 *	Free Directory structure.
 *------------------------------------------------------------------*/

static void
FreeDirectory(
        Directory *directory)
{
   int i;
   FileData *file_data, *next_file_data;

   if( directory == NULL )
     return;

   DirectoryUnwatch(directory);

   XtFree (directory->host_name);
   directory->host_name = NULL;

   XtFree (directory->directory_name);
   directory->directory_name = NULL;

   XtFree (directory->path_name);
   directory->path_name = NULL;

   XtFree (directory->tt_path_name);
   directory->tt_path_name = NULL;

   for (i=0; i < directory->path_count; i++)
      DtDtsFreeDataType(directory->path_logical_types[i]);
   XtFree ((char *) directory->path_logical_types);
   directory->path_logical_types = NULL;

   for (i = 0; i < directory->position_count; i++)
      XtFree(directory->position_info[i].name);
   XtFree ((char *) directory->position_info);
   directory->position_info = NULL;

   FreeModifiedList(directory);

   XtFree ((char *) directory->directoryView);
   directory->directoryView = NULL;

   if (directory->dir_data)
   {
      FreeFileData(directory->dir_data, True);
      directory->dir_data = NULL;
   }

   file_data = directory->file_data;
   while (file_data != NULL)
   {
      next_file_data = file_data->next;
      FreeFileData(file_data, True);
      file_data = next_file_data;
   }
   directory->file_data = NULL;

   /* (what a read in progress had sent so far) */
   file_data = directory->new_data;
   while (file_data != NULL)
   {
      next_file_data = file_data->next;
      FreeFileData(file_data, True);
      file_data = next_file_data;
   }
   directory->new_data = NULL;

   XtFree((char *) directory);
}


/*--------------------------------------------------------------------
 *  SomeWindowMapped
 *    Check if any cached directory is currently being viewed in
 *    a window that is mapped (not iconified).  (If there is none,
 *    we won't need to set a refresh timer).
 *------------------------------------------------------------------*/

static Boolean
SomeWindowMapped(void)

{
   int i, j;

   for (i = 0; i < directory_count; i++)
      for (j = 0; j < directory_set[i]->numOfViews; j++)
         if (directory_set[i]->directoryView[j].mapped)
            return True;

   return False;
}


/*====================================================================
 *
 * Routines for reading a directory
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 *  PipeWrite...
 *  PipeRead...
 *      Directories are read in a background process that is connected
 *      to the main dtfile process by a pipe.
 *	The routines below are used to send directory entry information
 *      through the pipe from the background to the main process.
 *------------------------------------------------------------------*/

/* write FileData to the pipe */
void
PipeWriteFileData(
	int fd,
	FileData *file_data)
{
   write(fd, file_data, sizeof(FileData));
   PipeWriteString(fd, file_data->file_name);
   PipeWriteString(fd, file_data->action_name);  /* @@@ ??? */
   PipeWriteString(fd, file_data->logical_type);
   if (file_data->link)
      PipeWriteString(fd, file_data->link);
   if (file_data->final_link)
      PipeWriteString(fd, file_data->final_link);
}


/* read FileData from the pipe */
static FileData *
PipeReadFileData(
	int fd)
{
   FileData *file_data;
   int n;

   file_data = (FileData *)XtCalloc(1,sizeof(FileData));
   n = PipeRead(fd, file_data, sizeof(FileData));
   if (n < sizeof(FileData))
   {
      fprintf(stderr, "PipeReadFileData: n = %d, expected %ld\n",
              n, (long)sizeof(FileData));
   }
   file_data->file_name = PipeReadString(fd);
   file_data->action_name = PipeReadString(fd);
   file_data->logical_type = PipeReadString(fd);
   if (file_data->link)
      file_data->link = PipeReadString(fd);
   if (file_data->final_link)
      file_data->final_link = PipeReadString(fd);

   /* return the file data */
   return file_data;
}

/* a NUL-terminated copy of the n bytes at p */
static char *
TextDup(
	const char *p,
	int n)
{
   char *s = XtMalloc(n + 1);

   memcpy(s, p, n);
   s[n] = NILL;
   return s;
}

FileData *
FileData2toFileData(
	FileData2 *file_data2,
	int *l)
{
   FileData *file_data;
   char *textptr = file_data2->text;

   file_data = (FileData *)XtCalloc(1,sizeof(FileData));

   file_data->next		= NULL;
   file_data->file_name		= TextDup(textptr, file_data2->file_name);
   textptr += file_data2->file_name;

   file_data->action_name	= file_data2->action_name
				? TextDup(textptr, file_data2->action_name)
				: NULL;
   textptr += file_data2->action_name;

   file_data->logical_type	= TextDup(textptr, file_data2->logical_type);
   textptr += file_data2->logical_type;

   file_data->link		= file_data2->link
				? TextDup(textptr, file_data2->link)
				: NULL;
   textptr += file_data2->link;

   file_data->final_link	= file_data2->final_link
				? TextDup(textptr, file_data2->final_link)
				: NULL;

   file_data->physical_type	= file_data2->physical_type;
   file_data->errnum		= file_data2->errnum;
   file_data->stat		= file_data2->stat;
   file_data->is_subdir		= file_data2->is_subdir;
   file_data->is_broken		= file_data2->is_broken;

   *l = sizeof(*file_data2) - sizeof(file_data2->text)
         + file_data2->file_name + file_data2->action_name
         + file_data2->logical_type + file_data2->link
         + file_data2->final_link;

   *l = (*l + sizeof(char *) - 1) & ~(sizeof(char *) - 1);

   /* return the file data */
   return file_data;
}

/* write PositionInfo to the pipe */
void
PipeWritePositionInfo(
	int fd,
	PositionInfo *position_info)
{
   PipeWriteString(fd, position_info->name);
   write(fd, &position_info->x, sizeof(Position));
   write(fd, &position_info->y, sizeof(Position));
   write(fd, &position_info->stacking_order, sizeof(int));
}


/* read PositionInfo from the pipe */
static void
PipeReadPositionInfo(
	int fd,
	PositionInfo *position_info)
{
   position_info->name = PipeReadString(fd);
   PipeRead(fd, &position_info->x, sizeof(Position));
   PipeRead(fd, &position_info->y, sizeof(Position));
   PipeRead(fd, &position_info->stacking_order, sizeof(int));
}

/*--------------------------------------------------------------------
 *  ReadFileData
 *    Given a path name, return FileData for a file.
 *------------------------------------------------------------------*/

FileData *
ReadFileData(
	char *full_directory_name,
	char *file_name)
{
   FileData *file_data;
   char full_file_name[MAX_PATH];
   char link_file_name[MAX_PATH];
   char link_path[MAX_PATH];
   int link_count;
   char ** link_list;
   int link_len;
   char * end;
   Boolean recursive_link_found;
   struct stat stat_buf;
   struct stat stat_buf2;
   int stat_result;
   int stat_errno;
   int i;

   /*  Allocate a new file structure.  */
   file_data = (FileData *)XtMalloc(sizeof(FileData));

   /* get the full name of the file */
   strcpy (full_file_name, full_directory_name);
   if (file_name)
   {
      /* append file name to the directory */
      if (strcmp(full_directory_name,"/") != 0)
         strcat (full_file_name, "/");
      strcat (full_file_name, file_name);
   }
   else
   {
      /* no file name passed: use last component of directory */
      file_name = strrchr(full_file_name, '/');
      if (file_name > full_file_name)
         file_name++;
      else
         file_name = NULL;
   }

   /* Follow symbolic links to their ultimate destination */
   link_count = 0;
   link_list = NULL;
   recursive_link_found = False;
   strcpy(link_file_name, full_file_name);

   stat_result = lstat (link_file_name, &stat_buf);
   if (stat_result == 0 && (stat_buf.st_mode & S_IFMT) == S_IFLNK)
   {
     while ((link_len = readlink(link_file_name, link_path, MAX_PATH - 1)) > 0)
     {
       link_path[link_len] = 0;
       link_list = (char **)XtRealloc((char *)link_list, sizeof(char *) *
                                      (link_count + 2));

       /* Force the link to be an absolute path, if necessary */
       if (link_path[0] != '/')
       {
         /* Relative paths are relative to the current directory */
         end = strrchr(link_file_name, '/') + 1;
         *end = '\0';
         strcat(link_file_name, link_path);
       }
       else
         strcpy(link_file_name, link_path);

       /* Check for a recursive loop; abort if found */
       for (i = 0; i < link_count; i++)
       {
         if (strcmp(link_file_name, link_list[i]) == 0)
         {
           /* Back up to last non-recursive portion */
           strcpy(link_file_name, link_list[link_count - 1]);
           recursive_link_found = True;
           break;
         }
       }

       if (recursive_link_found)
         break;

       link_list[link_count++] = XtNewString(link_file_name);
       link_list[link_count] = NULL;
     }

     /* try to stat the file that the link points to */
     if (stat (link_file_name, &stat_buf2) == 0)
     {
       /* replace lstat result with the stat */
       memcpy(&stat_buf, &stat_buf2, sizeof(struct stat));
     }
   }
   stat_errno = errno;

   /* fill in the FileData structure with the information we found */
   file_data->next = NULL;
   file_data->file_name = XtNewString(file_name? file_name: ".");
   file_data->logical_type = NULL;
   file_data->is_subdir = False;
   file_data->action_name = NULL;

   if (link_list)
   {
      file_data->link = XtNewString( link_list[0] );
      file_data->final_link = XtNewString( link_list[link_count - 1] );
      for (i = 0; i < link_count; i++)
         XtFree(link_list[i]);
      XtFree((char *)link_list);
   } else
      file_data->link = file_data->final_link = NULL;

   if (stat_result == 0)
   {
      file_data->errnum = 0;
      file_data->stat = stat_buf;

      /*  Find and set the physical type of the file  */

      if ((stat_buf.st_mode & S_IFMT) == S_IFDIR)
      {
         file_data->physical_type = DtDIRECTORY;
         if (file_name == NULL ||
             (strcmp(file_name, ".") != 0 && strcmp(file_name, "..") != 0))
         {
            file_data->is_subdir = True;
         }
      }
      else if ((stat_buf.st_mode & S_IFMT) == S_IFREG)
      {
         if ((stat_buf.st_mode & S_IXUSR) ||
             (stat_buf.st_mode & S_IXGRP) ||
             (stat_buf.st_mode & S_IXOTH))
            file_data->physical_type = DtEXECUTABLE;
         else
            file_data->physical_type = DtDATA;
      }
      else
         file_data->physical_type = DtDATA;

      /*  Find and set the logical type of the file  */
      if ((stat_buf.st_mode & S_IFMT) == S_IFLNK)
      {
         file_data->is_broken = True;
         if (recursive_link_found)
            file_data->logical_type = XtNewString(LT_RECURSIVE_LINK);
         else
            file_data->logical_type = XtNewString(LT_BROKEN_LINK);
      }
      else
      {
         file_data->is_broken = False;
         if (file_data->link)
            file_data->logical_type = (char *) DtDtsDataToDataType(
                                                 file_data->link,
                                                 NULL, 0, &stat_buf,
                                                 file_data->final_link, NULL,
                                                 NULL);
         else
            file_data->logical_type = (char *) DtDtsDataToDataType(
                                                 full_file_name,
                                                 NULL, 0, &stat_buf,
                                                 NULL, NULL,
                                                 NULL);
#if defined( DATATYPE_IS_FIXED )
#else
         /* The problem here is there isn't a way for user to mask
            only the OWNER READ bit in the MODE field of dtfile.dt file.
            If the MODE field set to d&!r Then all READ permission
            (S_IRUSR, S_IRGRP and S_IROTH)
            bits has to be off in order for the above data typing to work.
            Also data typing is unable to detect when the directory is not
            the owners and only has execute permission by that owner.
            The work around is manually checking it ourselves.
            When the data typing code is fixed, please remove this check.
         */
         if( S_ISDIR( stat_buf.st_mode ) &&
	    (strcmp (file_data->logical_type, LT_DIRECTORY) == 0))
         {
           if( strcmp( file_name, ".." ) != 0
               && strcmp( file_name, "." ) != 0 )
           {
             char * fullPathName;

             if( file_data->link )
               fullPathName = file_data->link;
             else
               fullPathName = full_file_name;

             if( access( fullPathName, R_OK ) != 0 )
             {
               XtFree( file_data->logical_type );
               file_data->logical_type = XtNewString( LT_FOLDER_LOCK );
             }
             else if( access( fullPathName, W_OK ) != 0 )
             {
               XtFree( file_data->logical_type );
               file_data->logical_type = XtNewString( LT_NON_WRITABLE_FOLDER );
             }
           }
         }
#endif
      }

      if(DtActionExists(file_data->logical_type))
      {
         file_data->action_name = (char *)DtActionLabel(file_data->file_name);
      }
      else
      {
         char *ptr = DtDtsDataTypeToAttributeValue(file_data->logical_type,
                                                   DtDTS_DA_LABEL,
                                                   NULL);
         if (ptr)
         {
            file_data->action_name = XtNewString(ptr);
            DtDtsFreeAttributeValue(ptr);
         }
      }
   }
   else
   {
      /* couldn't stat the file */
      file_data->errnum = stat_errno;
      memset(&file_data->stat, 0, sizeof(file_data->stat));
      file_data->physical_type = DtUNKNOWN;
      file_data->is_broken = True;
      file_data->logical_type = XtNewString(DtDEFAULT_DATA_FT_NAME);
   }

   return file_data;
}


/*
 * access() for a directory entry: relative to dir_fd when one is given
 * (saves resolving the directory path again), else by path.
 */
static int
EntryAccess(
	int dir_fd,
	const char *name,
	const char *path,
	int mode)
{
   if (dir_fd >= 0)
      return faccessat(dir_fd, name, mode, 0);
   return access(path, mode);
}

/*
 * ReadFileData2At
 *    The work of ReadFileData2.  If dir_fd is an open descriptor of
 *    full_directory_name and file_name is given, the entry is looked up
 *    relative to it.
 */
static int
ReadFileData2At(
	FileData2 *file_data2,
	int dir_fd,
	char *full_directory_name,
	char *file_name,
        Boolean IsToolBox)
{
   char full_file_name[MAX_PATH];
   char link_file_name[MAX_PATH];
   char link_path[MAX_PATH];
   char file_name_buf[MAXPATHLEN];
   char action_name_buf[MAXPATHLEN];
   char logical_type_buf[MAXPATHLEN];
   char link_buf[MAXPATHLEN];
   char final_link_buf[MAXPATHLEN];
   int link_count;
   char ** link_list;
   int link_len;
   char * end;
   Boolean recursive_link_found;
   struct stat stat_buf;
   int stat_result;
   int stat_errno;
   int i;
   TypeAttr *type_attr;
   Boolean is_action;

   /* only a named entry can be looked up relative to the directory */
   if (file_name == NULL)
      dir_fd = -1;

   /* get the full name of the file */
   strcpy (full_file_name, full_directory_name);

   if (file_name)
   {
      /* append file name to the directory */
      if (strcmp(full_directory_name,"/") != 0)
         strcat (full_file_name, "/");
      strcat (full_file_name, file_name);
   }
   else
   {
      /* no file name passed: use last component of directory */
      file_name = strrchr(full_file_name, '/');
      if (file_name > full_file_name)
         file_name++;
      else
         file_name = NULL;
   }

   /* Follow symbolic links to their ultimate destination */
   link_count = 0;
   link_list = NULL;
   recursive_link_found = False;
   strcpy(link_file_name, full_file_name);

   if (dir_fd >= 0)
      stat_result = fstatat(dir_fd, file_name, &stat_buf, AT_SYMLINK_NOFOLLOW);
   else
      stat_result = lstat (link_file_name, &stat_buf);
   if (stat_result == 0 && (stat_buf.st_mode & S_IFMT) == S_IFLNK)
   {
     while ((link_len = readlink(link_file_name, link_path, MAX_PATH - 1)) > 0)
     {
       link_path[link_len] = 0;
       link_list = (char **)XtRealloc((char *)link_list, sizeof(char *) *
                                      (link_count + 2));

       /* Force the link to be an absolute path, if necessary */
       if (link_path[0] != '/')
       {
         /* Relative paths are relative to the current directory */
         end = strrchr(link_file_name, '/') + 1;
         *end = '\0';
         strcat(link_file_name, link_path);
       }
       else
         strcpy(link_file_name, link_path);

       /* Check for a recursive loop; abort if found */
       for (i = 0; i < link_count; i++)
       {
         if (strcmp(link_file_name, link_list[i]) == 0)
         {
           /* Back up to last non-recursive portion */
           strcpy(link_file_name, link_list[link_count - 1]);
           recursive_link_found = True;
           break;
         }
       }

       if (recursive_link_found)
         break;

       link_list[link_count++] = XtNewString(link_file_name);
       link_list[link_count] = NULL;
     }

     if ((stat_result = stat (link_file_name, &stat_buf)) != 0)
     {
      /* probably a broken link; try lstat */
       stat_result = lstat (full_file_name, &stat_buf);
       strcpy(link_file_name, full_file_name);
     }
   }
   stat_errno = errno;

   /* fill in the FileData2 structure with the information we found */
   file_data2->next = NULL;
   strcpy(file_name_buf, (file_name ? file_name : "."));
   logical_type_buf[0] = NILL;
   file_data2->is_subdir = False;
   action_name_buf[0] = NILL;

   if (link_list)
   {
      strcpy(link_buf, link_list[0]);
      strcpy(final_link_buf, link_list[link_count - 1]);
      for (i = 0; i < link_count; i++) {
         XtFree(link_list[i]);
      }
      XtFree((char *)link_list);
   } else {
      final_link_buf[0] = NILL;
      link_buf[0] = NILL;
   }

   if (stat_result == 0)
   {
      file_data2->errnum = 0;
      file_data2->stat = stat_buf;

      /*  Find and set the physical type of the file  */

      if ((stat_buf.st_mode & S_IFMT) == S_IFDIR)
      {
         file_data2->physical_type = DtDIRECTORY;
         if (file_name == NULL ||
             (strcmp(file_name, ".") != 0 && strcmp(file_name, "..") != 0))
         {
            file_data2->is_subdir = True;
         }
      }
      else if ((stat_buf.st_mode & S_IFMT) == S_IFREG)
      {
         if ((stat_buf.st_mode & S_IXUSR) ||
             (stat_buf.st_mode & S_IXGRP) ||
             (stat_buf.st_mode & S_IXOTH))
            file_data2->physical_type = DtEXECUTABLE;
         else
            file_data2->physical_type = DtDATA;
      }
      else
         file_data2->physical_type = DtDATA;

      /*  Find and set the logical type of the file  */
      if ((stat_buf.st_mode & S_IFMT) == S_IFLNK)
      {
         file_data2->is_broken = True;
         if (recursive_link_found)
            strcpy(logical_type_buf, LT_RECURSIVE_LINK);
         else
            strcpy(logical_type_buf, LT_BROKEN_LINK);
      }
      else
      {
         char *ptr;

         file_data2->is_broken = False;
         if (link_buf[0] == NILL)
            ptr = (char *) DtDtsDataToDataType(  full_file_name,
                                                 NULL, 0, &stat_buf,
                                                 NULL, NULL,
                                                 NULL);
         else
            ptr = (char *) DtDtsDataToDataType(  link_buf,
                                                 NULL, 0, &stat_buf,
                                                 final_link_buf, NULL,
                                                 NULL);

#if defined( DATATYPE_IS_FIXED )
         strcpy(logical_type_buf, ptr);
         free(ptr);
#else
         /* The problem here is there isn't a way for user to mask
            only the OWNER READ bit in the MODE field of dtfile.dt file.
            If the MODE field set to d&!r Then all READ permission
            (S_IRUSR, S_IRGRP and S_IROTH)
            bits has to be off in order for the above data typing to work.
            Also data typing is unable to detect when the directory is not
            the owners and only has execute permission by that owner.
            The work around is manually checking it ourselves.
            When the data typing code is fixed, please remove this check.
         */
         if( !IsToolBox && S_ISDIR( stat_buf.st_mode ) &&
	    (strcmp (ptr, LT_DIRECTORY) == 0))
         {
           if( file_name == NULL ||
               (strcmp( file_name, ".." ) != 0
                && strcmp( file_name, "." ) != 0) )
           {
             char * fullPathName;
             int fd = dir_fd;

             if( link_buf[0] == NILL )
               fullPathName = full_file_name;
             else
             {
               fullPathName = link_buf;
               fd = -1;
             }

             if( EntryAccess( fd, file_name, fullPathName, R_OK ) != 0 )
             {
               free( ptr ); /* Don't use XtFree. This pointer is being kept by tooltalk */
               strcpy( logical_type_buf, LT_FOLDER_LOCK );
             }
             else if( EntryAccess( fd, file_name, fullPathName, W_OK ) != 0 )
             {
               free( ptr ); /* Don't use XtFree. This pointer is being kept by tooltalk */
               strcpy( logical_type_buf, LT_NON_WRITABLE_FOLDER );
             }
             else
             {
               strcpy( logical_type_buf, ptr );
               free( ptr ); /* Don't use XtFree. This pointer is being kept by tooltalk */
             }
           }
           else
           {
             strcpy( logical_type_buf, ptr );
             free( ptr ); /* Don't use XtFree. This pointer is being kept by tooltalk */
           }
         }
         else
         {
           strcpy( logical_type_buf, ptr );
           free( ptr ); /* Don't use XtFree. This pointer is being kept by tooltalk */
         }
#endif
      }

      type_attr = TypeAttrLookup(logical_type_buf);
      if (type_attr)
        is_action = type_attr->is_action;
      else
        is_action = DtActionExists(logical_type_buf);

      if( is_action )
      {
        char *ptr = (char *)DtActionLabel(file_name_buf);
        if (ptr)
        {
          strcpy(action_name_buf, ptr);
          free(ptr);
        }
      }
      else if (type_attr)
      {
        if (type_attr->label)
          strcpy(action_name_buf, type_attr->label);
      }
      else
      {
        char *ptr = DtDtsDataTypeToAttributeValue(logical_type_buf,
                                                  DtDTS_DA_LABEL,
                                                  NULL);
        if (ptr)
        {
          strcpy(action_name_buf, ptr);
          DtDtsFreeAttributeValue(ptr);
        }
      }
   }
   else
   {
      /* couldn't stat the file */
      file_data2->errnum = stat_errno;
      memset(&file_data2->stat, 0, sizeof(file_data2->stat));
      file_data2->physical_type = DtUNKNOWN;
      file_data2->is_broken = True;
      strcpy(logical_type_buf, DtDEFAULT_DATA_FT_NAME);
   }

	strcpy(file_data2->text, file_name_buf);
	file_data2->file_name = strlen(file_name_buf);

	strcat(file_data2->text, action_name_buf);
	file_data2->action_name = strlen(action_name_buf);

	strcat(file_data2->text, logical_type_buf);
	file_data2->logical_type = strlen(logical_type_buf);

	strcat(file_data2->text, link_buf);
	file_data2->link = strlen(link_buf);

	strcat(file_data2->text, final_link_buf);
	file_data2->final_link = strlen(final_link_buf);

	i = sizeof(*file_data2) - sizeof(file_data2->text)
		+ file_data2->file_name + file_data2->action_name
		+ file_data2->logical_type + file_data2->link
		+ file_data2->final_link;

	i = (i + sizeof(char *) - 1) & ~(sizeof(char *) - 1);

	/*
	 * This data marshalling operation relies on char[BUFSIZ]
	 * being large enough for all the text pieces.  However,
	 * BUFSIZ has nothing to do with the above operations so
	 * we'll do this assert for now.
	 */
	assert( (i <= sizeof(FileData2)) );

	return i;
}


/*--------------------------------------------------------------------
 *  ReadFileData2
 *    Given a path name, return FileData for a file.
 *------------------------------------------------------------------*/

int
ReadFileData2(
	FileData2 *file_data2,
	char *full_directory_name,
	char *file_name,
        Boolean IsToolBox)
{
   return ReadFileData2At(file_data2, -1, full_directory_name, file_name,
                          IsToolBox);
}

/*--------------------------------------------------------------------
 *  GetTTPath
 *      Resolves the links in the path.
 *------------------------------------------------------------------*/

char *
GetTTPath(char *path)
{
   Tt_message dummy_msg;
   char *tmp, *tt_path;

   dummy_msg = tt_message_create();
   tt_message_file_set(dummy_msg, path);
   tmp = tt_message_file(dummy_msg);

   tt_path = XtNewString(tmp);

   tt_free(tmp);
   tt_message_destroy(dummy_msg);

   return tt_path;
}


/*--------------------------------------------------------------------
 *  AddPathLogicalTypes
 *    Append a PIPEMSG_PATH_LOGICAL_TYPES message to pb: the logical
 *    data type of every component of the directory's path, followed
 *    by the ToolTalk name of full_directory_name.
 *    We need only the last path component for (1) the tree root icon,
 *    and (2) the current directory icon; the other path components
 *    are needed for the iconic path icons.
 *------------------------------------------------------------------*/

static void
AddPathLogicalTypes(
        PipeBuf *pb,
        char *host_name,
        char *directory_name,
        char *full_directory_name,
        char *known_tt_path)
{
   struct stat stat_buf;
   int path_count;
   char **path_logical_types;
   char *component_name;
   char *namePtr;
   char *ptr;
   char *ptrOrig;
   char *tt_path;
   int i;

   path_count = 0;
   path_logical_types = NULL;

   /* Don't muck with original string */
   ptrOrig = ptr = XtNewString(directory_name);

   for (;;)
   {
      Tt_status tt_status;

      if (ptr != NULL)
         *ptr = '\0';

      if (ptrOrig[0] == '\0')
         namePtr = "/";
      else
         namePtr = ptrOrig;

      /* get logical type of next path component */
      component_name = ResolveLocalPathName( host_name,
                                             namePtr,
                                             NULL,
                                             home_host_name,
                                             &tt_status );
      if( TT_OK != tt_status )
        break;

      DtEliminateDots (component_name);
      path_logical_types = (char **) XtRealloc((char *)path_logical_types,
                                               (path_count + 1)*sizeof(char *));
      path_logical_types[path_count] =
         (char *) DtDtsDataToDataType(component_name, NULL, 0, NULL, NULL,
                                      NULL, NULL);
#if defined( DATATYPE_IS_FIXED )
#else
      {
        if( stat( component_name, &stat_buf ) == 0 )
        {
          if( S_ISDIR( stat_buf.st_mode ) &&
	    (strcmp (path_logical_types[path_count], LT_DIRECTORY) == 0))
          {
            if( access( component_name, R_OK ) != 0 )
            {
              XtFree( path_logical_types[path_count] );
              path_logical_types[path_count] = XtNewString( LT_FOLDER_LOCK );
            }
            else if( access( component_name, W_OK ) != 0 )
            {
              XtFree( path_logical_types[path_count] );
              path_logical_types[path_count] = XtNewString( LT_NON_WRITABLE_FOLDER );
            }
          }
        }
      }
#endif
      DPRINTF2(("AddPathLogicalTypes: path '%s', fullname '%s', type %s\n",
                namePtr, component_name, path_logical_types[path_count]));

      XtFree( component_name );
      path_count++;

      if (ptr == NULL)
        break;

      /* restore '/' */
      *ptr = '/';

      /* find next component */
      if (strcmp(ptr, "/") == 0)
         break;
      ptr = DtStrchr(ptr + 1, '/');
   }
   XtFree(ptrOrig);

   /* the path_logical_types */
   DPRINTF(("AddPathLogicalTypes: sending %d path_logical_types\n",
            path_count));
   PipeBufAddMsg(pb, PIPEMSG_PATH_LOGICAL_TYPES);
   PipeBufAdd(pb, &path_count, sizeof(int));
   for(i = 0; i < path_count; i++)
   {
     PipeBufAddString(pb, path_logical_types[i]);
     XtFree((char *) path_logical_types[i]);
   }
   XtFree((char *) path_logical_types);

   /* the tt_path (it depends on the path only: an update of a directory
      that was read before can send what the read found) */
   if (known_tt_path != NULL)
      PipeBufAddString(pb, known_tt_path);
   else
   {
      tt_path = GetTTPath(full_directory_name);
      PipeBufAddString(pb, tt_path);
      XtFree(tt_path);
   }
}


/* send a PIPEMSG_ERROR message (ds: the directory's time stamps, or NULL) */
static void
PipeWriteError(
        int pipe_fd,
        PipeBuf *pb,
        int rc,
        const DirStamp *ds)
{
   DirStamp none;

   if (ds == NULL)
   {
      DirStampSet(&none, NULL, False);
      ds = &none;
   }
   PipeBufAddMsg(pb, PIPEMSG_ERROR);
   PipeBufAdd(pb, &rc, sizeof(int));
   PipeBufAdd(pb, ds, sizeof(DirStamp));
   PipeBufFlush(pipe_fd, pb);
}


/* send a PIPEMSG_DONE message */
static void
PipeWriteDone(
        int pipe_fd,
        PipeBuf *pb,
        const DirStamp *ds)
{
   PipeBufAddMsg(pb, PIPEMSG_DONE);
   PipeBufAdd(pb, ds, sizeof(DirStamp));
   PipeBufFlush(pipe_fd, pb);
}


/* is the toolbox flag set for the (first) view of this directory? */
static Boolean
DirectoryIsToolBox(
        Directory *directory)
{
   if (directory->directoryView && directory->directoryView->file_mgr_data)
      return directory->directoryView->file_mgr_data->toolbox;
   return False;
}


static int
ReadDirectoryProcess(
        int pipe_fd,
        Directory *directory,
	ActivityStatus activity)
{
#ifdef DT_PERFORMANCE
   struct timeval update_time_s;
   struct timeval update_time_f;
#endif
   char *host_name = directory->host_name;
   char *directory_name = directory->directory_name;
   struct stat stat_buf;
   DirStamp stamp;
   char *full_directory_name;
   DIR *dirp;
   struct dirent * dp;
   Boolean inDtDir;
   Boolean IsToolBox;
   Boolean done;
   Boolean update_due;
   int i;
   char * ptr;
   char file_name[MAX_PATH];
   int position_count;
   FILE * fptr;
   int x, y, stacking_order;
   int rc;
   int dir_fd;
   PipeBuf pb = { NULL, 0, 0 };
   FileDataBatch *batch;
   struct timeval time1, time2;
   long diff;

   DPRINTF(("ReadDirectoryProcess(%d, \"%s\", \"%s\")\n",
            pipe_fd, host_name, directory_name));

   /* get the full name of the current directory */
   {
     Tt_status tt_status;
     full_directory_name = ResolveLocalPathName( host_name,
                                                 directory_name,
                                                 NULL,
                                                 home_host_name,
                                                 &tt_status );
     /* It's ok not to check for tt_status.
        The code below will handle it properly.
     */
   }

   /*
    * Send the logical data type of all components of the path, and
    * the tt_path, back through the pipe.
    */
   AddPathLogicalTypes(&pb, host_name, directory_name, full_directory_name,
                       NULL);
   PipeBufFlush(pipe_fd, &pb);

   /*
    * Stat the directory to get its timestamp.
    * Also check if we have read and execute/search permisssion.
    */
   if (CheckAccess(full_directory_name, R_OK | X_OK) != 0 ||
       stat(full_directory_name, &stat_buf) != 0)
   {
      /* send an error code back through the pipe */
      rc = errno;
      DPRINTF(("ReadDirectoryProcess: sending errno %d (stat failed)\n", rc));
      PipeWriteError(pipe_fd, &pb, rc, NULL);
      PipeBufFree(&pb);
      XtFree(full_directory_name);
      return 1;
   }

   DirStampSet(&stamp, &stat_buf, False);

   /*
    * We never want to display the '~/.dt/Desktop' directory, so when we
    * are working with the .dt directory, add a special check for a
    * directory named 'Desktop'.
    */
   if ((ptr = strrchr(full_directory_name, '/')) &&
       (strcmp(ptr, "/.dt") == 0))
      inDtDir = True;
   else
      inDtDir = False;

   /* try to open the directory */
   dirp = opendir (full_directory_name);
   if (dirp == NULL)
   {
      /* send an error code back through the pipe */
      rc = errno;
      DPRINTF(("ReadDirectoryProcess: sending errno %d (opendir failed)\n",
               rc));
      PipeWriteError(pipe_fd, &pb, rc, &stamp);
      PipeBufFree(&pb);
      XtFree( full_directory_name );
      return 1;
   }
   dir_fd = dirfd(dirp);
   stamp.local_fs = IsLocalFileSystem(dir_fd);
   IsToolBox = DirectoryIsToolBox(directory);

   /*  Loop through the directory entries and update the file list  */

#ifdef DT_PERFORMANCE
   printf("  begin reading directory: %s\n", full_directory_name);
   gettimeofday(&update_time_s, NULL);
#endif

   /*
    * FILEDATA3 sends the FileData2 structures in batches of up to
    * FILEDATABUF.  FILEDATABUF appears to work the best when set to 50.
    *
    * We send data to the parent at least every half seconds, even if
    * less than FILEDATABUF worth of FileData2 structs have been read.
    * This is to ensure that the file count in the status line gets
    * updated every half seconds, even if the file system is slow.
    */
   batch = FileDataBatchCreate(pipe_fd);
   TypeAttrCacheBegin();

   /* get current time */
   gettimeofday(&time1, NULL);

   done = False;
   do
   {
     if ((dp = readdir (dirp)) != NULL)
     {
       /* if Desktop skip */
       if (inDtDir && (strcmp(dp->d_name, "Desktop") == 0))
         continue;

       /* get the info */
       batch->ptr += ReadFileData2At(FileDataBatchNext(batch), dir_fd,
                                     full_directory_name, dp->d_name,
                                     IsToolBox);
       batch->count++;
     }
     else
       done = True;

     /* check if 0.4 seconds have passed since the last status line update */
     gettimeofday(&time2, NULL);
     diff = 1024*(time2.tv_sec - time1.tv_sec);
     diff += time2.tv_usec/1024;
     diff -= time1.tv_usec/1024;
     update_due = (diff >= 400);

     /* check if we need to send the buffered data now */
     if (batch->count == FILEDATABUF ||
         (batch->count > 0 && (done || update_due)))
     {
       FileDataBatchFlush(batch, update_due);
       if (update_due)
         time1 = time2;
     }
   } while (!done);

   TypeAttrCacheEnd();
   XtFree((char *)batch);

#ifdef DT_PERFORMANCE
   gettimeofday(&update_time_f, NULL);
   if (update_time_s.tv_usec > update_time_f.tv_usec) {
      update_time_f.tv_usec += 1000000;
      update_time_f.tv_sec--;
   }
   printf("    finished reading: %s, time: %ld.%ld\n\n", full_directory_name, update_time_f.tv_sec - update_time_s.tv_sec, update_time_f.tv_usec - update_time_s.tv_usec);
#endif


   /* load position info, if available */

   /* construct full name of the position info file */
   if (strcmp(full_directory_name,"/") != 0)
     sprintf( file_name, "%s/%s", full_directory_name, positionFileName );
   else
     sprintf( file_name, "%s%s", full_directory_name, positionFileName );

   /* read the count from the position info file */
   position_count = 0;
   if ((fptr = fopen(file_name, "r")) != NULL)
   {
     PositionInfo * position_info = NULL;
     fscanf(fptr, "%d\n", &position_count);

     if (position_count > 0)
     {
       /* allocate position info array */
       position_info = (PositionInfo *)
         XtMalloc(position_count * sizeof(PositionInfo));

       /* read the position info from the file */
       i = 0;
       while (i < position_count)
       {
         if( fgets( file_name, MAX_PATH, fptr ) != NULL &&
             fscanf(fptr, "%d %d %d\n", &x, &y, &stacking_order ) == 3  )
         {
           int len = strlen(file_name);
           file_name[len-1] = 0x0;
           position_info[i].name = XtNewString(file_name);
           position_info[i].x = x,
           position_info[i].y = y,
           position_info[i].stacking_order = stacking_order;
           i++;
         }
         else
           break;
       }
       position_count = i;
     }
     else
       position_count = 0;

     fclose(fptr);

     /* send the position info back through the pipe */
     DPRINTF(("ReadDirectoryProcess: sending %d position_info\n",
              position_count));
     PipeBufAddMsg(&pb, PIPEMSG_POSITION_INFO);
     PipeBufAdd(&pb, &position_count, sizeof(int));
     for (i = 0; i < position_count; i++)
     {
       PipeBufAddString(&pb, position_info[i].name);
       XtFree( position_info[i].name );
       PipeBufAdd(&pb, &(position_info[i].x), sizeof(Position));
       PipeBufAdd(&pb, &(position_info[i].y), sizeof(Position));
       PipeBufAdd(&pb, &(position_info[i].stacking_order), sizeof(int));
     }
     XtFree( (char *)position_info );
   }

   XtFree(full_directory_name);
   closedir (dirp);

   /* send a 'done' msg through the pipe (with the position info) */
   DPRINTF(("ReadDirectoryProcess: sending DONE\n"));
   PipeWriteDone(pipe_fd, &pb, &stamp);
   PipeBufFree(&pb);
   return 0;
}


/*--------------------------------------------------------------------
 *  EntryChanged
 *    Has a known directory entry changed since its FileData was read?
 *    old->stat holds what ReadFileData2 stored: for a symbolic link
 *    that resolves, the stat of its final target; otherwise the lstat
 *    of the entry itself.  (Comparing the lstat of a link with the
 *    stat of its target made every link look modified, so all links
 *    were retyped on every refresh.)
 *    The change time is compared too, so that a chown or another
 *    attribute change shows.
 *    dir_mtim is the directory's timestamp at the last read: a link
 *    whose own timestamp is not older was (re)created since then.
 *    A link that did not resolve (broken or recursive) has changed if
 *    it resolves now, or fails differently.
 *------------------------------------------------------------------*/

static Boolean
SameFileStat(
        const struct stat *a,
        const struct stat *b)
{
   return a->st_mtim.tv_sec == b->st_mtim.tv_sec &&
          a->st_mtim.tv_nsec == b->st_mtim.tv_nsec &&
          a->st_ctim.tv_sec == b->st_ctim.tv_sec &&
          a->st_ctim.tv_nsec == b->st_ctim.tv_nsec &&
          a->st_ino == b->st_ino &&
          a->st_dev == b->st_dev &&
          a->st_mode == b->st_mode &&
          a->st_size == b->st_size;
}

static Boolean
EntryChanged(
        int dir_fd,
        const char *name,
        FileData *old,
        const struct timespec *dir_mtim)
{
   struct stat sbuf;
   Boolean was_recursive;

   if (fstatat(dir_fd, name, &sbuf, AT_SYMLINK_NOFOLLOW) != 0)
      return False;          /* gone meanwhile: keep the old data, as before */

   if (!S_ISLNK(sbuf.st_mode))
      return old->link != NULL || !SameFileStat(&sbuf, &old->stat);

   /* a symbolic link */
   if (old->link == NULL)
      return True;           /* new link */
   if (sbuf.st_mtim.tv_sec > dir_mtim->tv_sec ||
       (sbuf.st_mtim.tv_sec == dir_mtim->tv_sec &&
        sbuf.st_mtim.tv_nsec >= dir_mtim->tv_nsec))
      return True;           /* the link itself was (re)created */

   /*
    * (old->is_broken can't tell: the main process clears it when it
    * makes the icon; the logical type tells.)
    */
   was_recursive = old->logical_type != NULL &&
                   strcmp(old->logical_type, LT_RECURSIVE_LINK) == 0;
   if (was_recursive || (old->logical_type != NULL &&
                         strcmp(old->logical_type, LT_BROKEN_LINK) == 0))
   {
      /* old->stat is the link's own; it did not resolve */
      if (!SameFileStat(&sbuf, &old->stat))
         return True;
      if (fstatat(dir_fd, name, &sbuf, 0) == 0)
         return True;        /* resolves now */
      return (errno == ELOOP) != was_recursive;
   }

   if (fstatat(dir_fd, name, &sbuf, 0) != 0)
      return True;           /* no longer resolves */
   return !SameFileStat(&sbuf, &old->stat);
}


/* a FileData2 record telling the main process that a file is gone */
static int
MakeGoneRecord(
        FileData2 *file_data2,
        char *file_name)
{
   int len = strlen(file_name);
   int i;

   memset(file_data2, 0, sizeof(*file_data2) - sizeof(file_data2->text));
   file_data2->errnum = ENOENT;
   file_data2->physical_type = DtUNKNOWN;
   file_data2->is_broken = True;
   memcpy(file_data2->text, file_name, len);
   file_data2->file_name = len;

   i = sizeof(*file_data2) - sizeof(file_data2->text) + len;
   return (i + sizeof(char *) - 1) & ~(sizeof(char *) - 1);
}


/*--------------------------------------------------------------------
 *  UpdateAllProcess
 *    Main routine of the background process that checks the directory
 *    for new files or files that have disapeared.
 *------------------------------------------------------------------*/

static int
UpdateAllProcess(
                 int pipe_fd,
                 Directory *directory,
                 ActivityStatus activity)
{
   char *host_name = directory->host_name;
   char *directory_name = directory->directory_name;
   char *full_directory_name;
   struct stat stat_buf;
   DirStamp stamp;
   DIR *dirp;
   struct dirent * dp;
   Boolean inDtDir;
   Boolean IsToolBox;
   FileData *old_data;
   FileData **olds;
   char **old_names;
   char *taken;
   int n_old;
   NameIndex old_index;
   NameIndex modified_index;
   FileDataBatch *batch;
   PipeBuf pb = { NULL, 0, 0 };
   char *ptr;
   int i, rc=0;
   int dir_fd;
   Tt_status tt_status;

   DPRINTF(("UpdateAllProcess(%d, \"%s\", \"%s\")\n",
            pipe_fd, host_name, directory_name));

   /* if modified list contains "." or ".." arrange for the path_logical_types
      to get updated */
   if(directory->modified_count == 0 &&
      FindDirectory(directory->host_name, directory_name))
   {
       rc = 1;
   }
   else
   {
       for (i = 0; i < directory->modified_count; i++)
       {
           if (strcmp(directory->modified_list[i],".") == 0 ||
               strcmp(directory->modified_list[i],"..") == 0 )
           {
               rc = 1;
               break;
           }
       }
   }

   /* get the full name of the current directory */
   full_directory_name = ResolveLocalPathName( host_name,
                                               directory_name,
                                               NULL,
                                               home_host_name,
                                               &tt_status );
   /* It's ok not to check for tt_status yet. */

   if(rc)
   {
      /* send the path_logical_types and the tt_path through the pipe */
      AddPathLogicalTypes(&pb, host_name, directory_name, full_directory_name,
                          directory->tt_path_name);
      PipeBufFlush(pipe_fd, &pb);
   }

   if( TT_OK != tt_status )
   {
      rc = -1;
      DPRINTF(("UpdateAllProcess: sending errno %d (tooltalk failed)\n", rc));
      PipeWriteError(pipe_fd, &pb, rc, NULL);
      PipeBufFree(&pb);
      return 1;
   }
   (void) DtEliminateDots (full_directory_name);

   /*
    * Stat the directory to get its timestamp.
    * Also check if we still have read and execute/search permisssion.
    */
   if (CheckAccess(full_directory_name, R_OK | X_OK) != 0 ||
       stat(full_directory_name, &stat_buf) != 0)
   {
      /* send an error code back through the pipe */
      rc = errno;
      DPRINTF(("UpdateAllProcess: sending errno %d (stat failed)\n", rc));
      PipeWriteError(pipe_fd, &pb, rc, NULL);
      PipeBufFree(&pb);
      XtFree( full_directory_name );
      return 1;
   }
   DirStampSet(&stamp, &stat_buf, False);

   /* check if we are in the .dt directory */
   if ((ptr = strrchr(full_directory_name, '/')) &&
       (strcmp(ptr, "/.dt") == 0))
   {
      inDtDir = True;
   }
   else
      inDtDir = False;

   /* try to open the directory */
   dirp = opendir (full_directory_name);
   if (dirp == NULL)
   {
      /* send an error code back through the pipe */
      rc = errno;
      DPRINTF(("UpdateAllProcess: sending errno %d (opendir failed)\n", rc));
      PipeWriteError(pipe_fd, &pb, rc, &stamp);
      PipeBufFree(&pb);
      XtFree( full_directory_name );
      return 1;
   }
   dir_fd = dirfd(dirp);
   stamp.local_fs = IsLocalFileSystem(dir_fd);
   IsToolBox = DirectoryIsToolBox(directory);

   /* index the files we knew about, and the list of modified files */
   n_old = 0;
   for (old_data = directory->file_data; old_data; old_data = old_data->next)
      n_old++;
   olds = (FileData **) XtMalloc((n_old + 1) * sizeof(FileData *));
   old_names = (char **) XtMalloc((n_old + 1) * sizeof(char *));
   taken = XtCalloc(n_old + 1, sizeof(char));
   for (i = 0, old_data = directory->file_data;
        old_data;
        i++, old_data = old_data->next)
   {
      olds[i] = old_data;
      old_names[i] = old_data->file_name;
   }
   NameIndexInit(&old_index, old_names, n_old);
   NameIndexInit(&modified_index, directory->modified_list,
                 directory->modified_count);

   batch = FileDataBatchCreate(pipe_fd);
   TypeAttrCacheBegin();

   /*  Loop through the directory entries and update the file list  */
   while ((dp = readdir (dirp)))
   {
      /* if Desktop skip */
      if (inDtDir && (strcmp(dp->d_name, "Desktop") == 0))
         continue;

      /* check if we already know this file */
      i = NameIndexFind(&old_index, old_names, dp->d_name, taken);
      if (i >= 0)
      {
         /* it still exists */
         taken[i] = True;

         /*
          * If it hasn't changed and isn't on the modified list,
          * there is nothing to report.
          */
         if (!EntryChanged(dir_fd, dp->d_name, olds[i], &directory->mtim) &&
             NameIndexFind(&modified_index, directory->modified_list,
                           dp->d_name, NULL) < 0)
            continue;
      }

      /* this is a new or modified file */
      DPRINTF(("UpdateAllProcess: new or modified file \"%s\"\n",
               dp->d_name));
      FileDataBatchAdd(batch,
                       ReadFileData2At(FileDataBatchNext(batch), dir_fd,
                                       full_directory_name, dp->d_name,
                                       IsToolBox));
   }

   /* all files we didn't see no longer exist */
   for (i = 0; i < n_old; i++)
   {
      if (taken[i] || olds[i]->file_name == NULL)
         continue;
      DPRINTF(("UpdateAllProcess: file gone \"%s\"\n", olds[i]->file_name));
      FileDataBatchAdd(batch, MakeGoneRecord(FileDataBatchNext(batch),
                                             olds[i]->file_name));
   }
   FileDataBatchFlush(batch, False);

   TypeAttrCacheEnd();
   XtFree((char *)batch);
   NameIndexFree(&old_index);
   NameIndexFree(&modified_index);
   XtFree((char *)olds);
   XtFree((char *)old_names);
   XtFree(taken);

   /* free storage */
   XtFree(full_directory_name);
   closedir(dirp);

   /* send a 'done' msg through the pipe */
   DPRINTF(("UpdateAllProcess: sending DONE\n"));
   PipeWriteDone(pipe_fd, &pb, &stamp);
   PipeBufFree(&pb);
   return 0;
}


/*--------------------------------------------------------------------
 *  UpdateSomeProcess
 *    Main routine of the background process that updates a selected
 *    list of directory entries.
 *------------------------------------------------------------------*/

static int
UpdateSomeProcess(
        int pipe_fd,
        Directory *directory,
	ActivityStatus activity)
{
   char *host_name = directory->host_name;
   char *directory_name = directory->directory_name;
   char *full_directory_name;
   struct stat stat_buf;
   DirStamp stamp;
   FileDataBatch *batch;
   PipeBuf pb = { NULL, 0, 0 };
   int i;
   int rc;
   int dir_fd;
   Boolean IsToolBox;

   DPRINTF(("UpdateSomeProcess(%d, \"%s\", \"%s\")\n",
            pipe_fd, host_name, directory_name));


   /* get the full name of the current directory */
   {
     Tt_status tt_status;

     full_directory_name = ResolveLocalPathName( host_name,
                                                 directory_name,
                                                 NULL,
                                                 home_host_name,
                                                 &tt_status );
     if( TT_OK != tt_status )
     {
       rc = -1;
       DPRINTF(("UpdateSomeProcess: sending errno %d (stat failed)\n", rc));
       PipeWriteError(pipe_fd, &pb, rc, NULL);
       PipeBufFree(&pb);
       return 1;
     }
   }
   (void) DtEliminateDots (full_directory_name);

   /* stat the directory to get the timestamp */
   if (stat(full_directory_name, &stat_buf) < 0 ||
       ! (stat_buf.st_mode & S_IXUSR) )
   {
      /* send an error code back through the pipe */
      rc = errno;
      DPRINTF(("UpdateSomeProcess: sending errno %d (stat failed)\n", rc));
      PipeWriteError(pipe_fd, &pb, rc, NULL);
      PipeBufFree(&pb);
      XtFree( full_directory_name );
      return 1;
   }
   DirStampSet(&stamp, &stat_buf, False);

   dir_fd = open(full_directory_name, O_RDONLY | O_DIRECTORY);
   stamp.local_fs = IsLocalFileSystem(dir_fd);
   IsToolBox = DirectoryIsToolBox(directory);
   batch = FileDataBatchCreate(pipe_fd);
   TypeAttrCacheBegin();

   /*  Loop through the list of modified files  */
   for (i = 0; i < directory->modified_count; i++)
   {
      /* get the info, and send it through the pipe */
      FileDataBatchAdd(batch,
                       ReadFileData2At(FileDataBatchNext(batch), dir_fd,
                                       full_directory_name,
                                       directory->modified_list[i],
                                       IsToolBox));
   }
   FileDataBatchFlush(batch, False);

   TypeAttrCacheEnd();
   XtFree((char *)batch);
   if (dir_fd >= 0)
      close(dir_fd);
   XtFree(full_directory_name);

   /* send a 'done' msg through the pipe */
   DPRINTF(("UpdateSomeProcess: sending DONE\n"));
   PipeWriteDone(pipe_fd, &pb, &stamp);
   PipeBufFree(&pb);
   return 0;
}


/*--------------------------------------------------------------------
 *  ReuseOldFileData
 *    For files in the old list that still exist in the new one we need
 *    to re-use the old FileData structures.
 *    Reason: the code in GetFileData relies on this to preserve the
 *    position_info and selection list.
 *    Each entry of the new list that also exists in the old list is
 *    replaced by the old structure (with the new contents), which is
 *    taken off the old list.
 *------------------------------------------------------------------*/

static void
ReuseOldFileData(
        Directory *directory)
{
   FileData *new_data, **new_nextp;
   FileData *old_data, **old_nextp;
   FileData **olds;
   char **old_names;
   char *taken;
   NameIndex old_index;
   int n_old, i;

   if (directory->new_data == NULL || directory->file_data == NULL)
      return;

   n_old = 0;
   for (old_data = directory->file_data; old_data; old_data = old_data->next)
      n_old++;
   olds = (FileData **) XtMalloc(n_old * sizeof(FileData *));
   old_names = (char **) XtMalloc(n_old * sizeof(char *));
   taken = XtCalloc(n_old, sizeof(char));
   for (i = 0, old_data = directory->file_data;
        old_data;
        i++, old_data = old_data->next)
   {
      olds[i] = old_data;
      old_names[i] = old_data->file_name;
   }
   NameIndexInit(&old_index, old_names, n_old);

   for (new_nextp = &directory->new_data;
        (new_data = *new_nextp) != NULL;
        new_nextp = &new_data->next)
   {
      if (new_data->file_name == NULL)
         continue;
      i = NameIndexFind(&old_index, old_names, new_data->file_name, taken);
      if (i >= 0)
      {
         /* (taken entries are never looked at again, so their
            names may be freed) */
         taken[i] = True;
         old_data = olds[i];

         FreeFileData(old_data, False);
         memcpy(old_data, new_data, sizeof(FileData));

         XtFree((char *)new_data);
         *new_nextp = new_data = old_data;
      }
   }

   /* the old list keeps the entries that were not taken, in order */
   old_nextp = &directory->file_data;
   for (i = 0; i < n_old; i++)
   {
      if (!taken[i])
      {
         *old_nextp = olds[i];
         old_nextp = &olds[i]->next;
      }
   }
   *old_nextp = NULL;

   NameIndexFree(&old_index);
   XtFree((char *)olds);
   XtFree((char *)old_names);
   XtFree(taken);
}


static Boolean
StringsEqual(
        const char *a,
        const char *b)
{
   if (a == NULL || b == NULL)
      return a == b;
   return strcmp(a, b) == 0;
}


/* is there more data waiting in the pipe? */
static Boolean
PipeHasData(
        int fd)
{
   struct pollfd pfd;

   pfd.fd = fd;
   pfd.events = POLLIN;
   pfd.revents = 0;
   return poll(&pfd, 1, 0) > 0;
}


/*--------------------------------------------------------------------
 *  ReaddirPipeCallback
 *	Callback routine that reads directory entry information sent
 *	through the pipe from the background process.
 *	It handles every message that is already waiting in the pipe,
 *	for up to READDIR_CALLBACK_BUDGET milliseconds, instead of one
 *	message per trip through the event loop.
 *------------------------------------------------------------------*/

#define READDIR_CALLBACK_BUDGET 20

static void
ReaddirPipeCallback(
   XtPointer client_data,
   int *fd,
   XtInputId *id)
{
   static int whined_fd = 0;
   PipeCallbackData *pipe_data = (PipeCallbackData *)client_data;
   Directory *directory = pipe_data->directory;
   FileMgrData *file_mgr_data;
   FileMgrRec *file_mgr_rec;
   ActivityStatus activity;
   Boolean done;
   short msg;
   Boolean update_due;
   FileData *new_data = NULL, **new_nextp;
   FileData *old_data;
   int i, n;
   int rc;
   DirStamp stamp;
   Boolean reread = False;
   char dirname[MAX_PATH];
   short file_data_count;
   struct timeval start_time, now;

   gettimeofday(&start_time, NULL);
   DirStampSet(&stamp, NULL, False);

   for (;;)
   {
   /* verify that the directory still exists */
   if (DirectoryGone(directory))
   {
      /*
       * The directory is no longer present:
       * close the pipe and kill the reader.
       */
      close(*fd);
      XtRemoveInput(*id);
      kill(pipe_data->child, SIGKILL);
      XtFree( client_data );
      ScheduleActivity(NULL);
      return;
   }

   /* read the next msg from the pipe */
   msg = -1;
   n = PipeRead(*fd, &msg, sizeof(short));
   activity = directory->activity;
   done = False;

   switch (msg)
   {
      case PIPEMSG_PATH_LOGICAL_TYPES:
      {
         char **path_logical_types;
         char *tt_path;
         Boolean changed;

         /* get the number of path components */
         n = 0;
         PipeRead(*fd, &n, sizeof(int));
         if (n < 0)
            n = 0;

         /* get the logical types and the tt_path */
         path_logical_types = (char **) XtMalloc((n + 1) * sizeof(char *));
         for (i = 0; i < n; i++)
           path_logical_types[i] = PipeReadString(*fd);
         tt_path = PipeReadString(*fd);

         /* the path icons only need redrawing if a type changed (or
            the database was reloaded: a type may have a new icon) */
         changed = (n != directory->path_count ||
                    directory->path_generation != db_generation);
         directory->path_generation = db_generation;
         for (i = 0; i < n && !changed; i++)
           changed = !StringsEqual(path_logical_types[i],
                                   directory->path_logical_types[i]);

         /* install the new values */
         for (i = 0; i < directory->path_count; i++)
           XtFree(directory->path_logical_types[i]);
         XtFree((char *)directory->path_logical_types);
         directory->path_logical_types = path_logical_types;
         directory->path_count = n;

         if (!StringsEqual(tt_path, directory->tt_path_name))
           changed = True;
         XtFree(directory->tt_path_name);
         directory->tt_path_name = tt_path;

         /* update all views */
         for (i = 0; i < directory->numOfViews; i++)
         {
            file_mgr_data = directory->directoryView[i].file_mgr_data;
            file_mgr_rec = (FileMgrRec *)file_mgr_data->file_mgr_rec;
            UpdateHeaders(file_mgr_rec, file_mgr_data, changed);
            XmUpdateDisplay(file_mgr_rec->file_window);
         }
         /* (XmUpdateDisplay syncs; just send what the exposures drew) */
         XFlush(XtDisplay(toplevel));
         break;
      }

      case PIPEMSG_FILEDATA3:
      {
         int file_data_length;
         FileData2 *file_data_buffer;
         char *file_data_buf_ptr;

         file_data_count = 0;
         file_data_length = 0;
         PipeRead(*fd, &file_data_count, sizeof(short));
         PipeRead(*fd, &file_data_length, sizeof(int));

         if (file_data_count & 0x8000)
         {
           file_data_count &= 0x7fff;
           update_due = True;
         }
         else
           update_due = False;

         if (file_data_length < 0 ||
             file_data_length > FILEDATABUF * sizeof(FileData2))
         {
           file_data_count = 0;
           file_data_length = 0;
         }

         /* append to the end of the new list */
         if (directory->new_data == NULL || directory->new_tail == NULL)
         {
           for (new_nextp = &directory->new_data;
                *new_nextp;
                new_nextp = &(*new_nextp)->next)
             ;
         }
         else
           new_nextp = directory->new_tail;

         file_data_buffer = (FileData2 *) XtMalloc(file_data_length + 1);
         n = PipeRead(*fd, file_data_buffer, file_data_length);
         if (n != file_data_length)
           file_data_count = 0;
         file_data_buf_ptr = (char *)file_data_buffer;

         for (i = 0; i < file_data_count; i++)
         {
           /* get next FileData out of buffer */
           new_data =
             FileData2toFileData((FileData2 *)file_data_buf_ptr, &n);
           file_data_buf_ptr += n;

           /* append new_data to end of list */
           *new_nextp = new_data;
           new_data->next = NULL;
           new_nextp = &new_data->next;
         }
         directory->new_tail = new_nextp;
         XtFree((char *)file_data_buffer);
         if (file_data_count > 0)
           directory->update_changed = True;

         if (activity == activity_reading)
         {
           /* update file counts in all views */
           for (i = 0; i < directory->numOfViews; i++)
           {
             file_mgr_data = directory->directoryView[i].file_mgr_data;
             file_mgr_rec = (FileMgrRec *)file_mgr_data->file_mgr_rec;
             file_mgr_data->busy_detail += file_data_count;
             if (update_due &&
                 (file_mgr_data->busy_status == initiating_readdir ||
                  file_mgr_data->busy_status == busy_readdir) &&
                 file_mgr_data->busy_detail > 2)
             {
               if (file_mgr_data->show_status_line)
               {
                 char buf[256];
                 XmString label_string;
                 Arg args[2];

                 GetStatusMsg(file_mgr_data, buf);
                 label_string =
                   XmStringCreateLocalized(buf);
                 XtSetArg (args[0], XmNlabelString, label_string);
                 XtSetValues(file_mgr_rec->status_line, args, 1);
                 XmStringFree(label_string);
               }
               else if (file_mgr_data->show_iconic_path)
               {
                 DtUpdateIconicPath(file_mgr_rec, file_mgr_data, False);
               }
               else if (file_mgr_data->show_current_dir)
               {
                 DrawCurrentDirectory(file_mgr_rec->current_directory,
                                      file_mgr_rec, file_mgr_data);
               }
             }
           }
         }
         break;
      }

      case PIPEMSG_POSITION_INFO:
         directory->update_changed = True;

         /* free old position info names */
         for (i = 0; i < directory->position_count; i++)
            XtFree(directory->position_info[i].name);

         /* get number of positions and realloc array, if necessary */
         n = 0;
         PipeRead(*fd, &n, sizeof(int));
         if (n < 0)
            n = 0;
         if (directory->position_count != n)
         {
            directory->position_count = n;
            directory->position_info = (PositionInfo *) XtRealloc(
                     (char *)directory->position_info, n*sizeof(PositionInfo));
         }

         /* read new position info */
         for (i = 0; i < n; i++)
            PipeReadPositionInfo(*fd, &directory->position_info[i]);
         break;

      case PIPEMSG_DONE:
         PipeRead(*fd, &stamp, sizeof(DirStamp));
         if (directory->errnum != 0)
            directory->update_changed = True;
         directory->errnum = 0;
         directory->errmsg_needed = False;
         done = True;
         break;

      case PIPEMSG_ERROR:
         PipeRead(*fd, &rc, sizeof(int));
         PipeRead(*fd, &stamp, sizeof(DirStamp));
         if (rc != directory->errnum)
         {
            /*
             * An update that fails leaves the old entries in place;
             * read the directory again, so that the views show the
             * error (as when the timer finds it unreadable).
             */
            if (activity != activity_reading && directory->errnum == 0)
               reread = True;
            directory->errnum = rc;
            directory->errmsg_needed = True;
            directory->update_changed = True;
         }
         done = True;
         break;

      default:
	 if (whined_fd != *fd)
	 {
	     whined_fd = *fd;
             fprintf(stderr,
	       "ReaddirPipeCallback: badmsg=%d, ppid=%d pid=%d fd=%d activ'y=%d\n",
	       msg, getppid(), getpid(), *fd, activity);
	 }
         directory->errnum = -1;
         directory->errmsg_needed = False;
         directory->update_changed = True;
         done = True;
   }

   if (done)
      break;

   /* go on with the next message if it is already there */
   if (!PipeHasData(*fd))
      return;
   gettimeofday(&now, NULL);
   if ((now.tv_sec - start_time.tv_sec) * 1000 +
       (now.tv_usec - start_time.tv_usec) / 1000 >= READDIR_CALLBACK_BUDGET)
      return;
   }

   /* we are done */
#ifdef DT_PERFORMANCE
   /* Aloke Gupta: As suggested by Dana Dao */
      _DtPerfChkpntMsgSend("Done  Read Directory");
#endif
      DPRINTF(("ReaddirPipeCallback: done, errno %d, time %ld\n",
               directory->errnum, stamp.modify_time));

      /* close the pipe and cancel the callback */
      close(*fd);
      XtRemoveInput(*id);

      /*
       * @@@ what if a drag is active ???
       */

      /*
       * For files in the old list that still exist in the new
       * one we need to re-use the old FileData structures.
       */
      ReuseOldFileData(directory);

      /*
       * If this was a complete re-read, we free all FileData still left
       * in the old list.  Otherwise, if this was just a partial update,
       * we append the old data that's still left to the end of the
       * new list.
       */
      if (activity == activity_reading)
      {
         /* This was a complete re-read: free all old data still left. */
         while (directory->file_data)
         {
            old_data = directory->file_data;
            directory->file_data = old_data->next;
            FreeFileData(old_data, True);
         }

         /* replace the old list by the new list */
         directory->file_data = directory->new_data;
         directory->new_data = NULL;
      }
      else
      {
         FileData * tmp_ptr = NULL;
         FileData ** old_nextp;

         /* remove any directory entries that no longer exist
            in the new list.
         */
         new_nextp = &directory->new_data;
         while ((new_data = *new_nextp) != NULL)
         {
            if (new_data->errnum == ENOENT)
            {
               *new_nextp = new_data->next;
               FreeFileData(new_data, True);
            }
            else
            {
               tmp_ptr = *new_nextp;
               new_nextp = &new_data->next;
            }
         }

         /* remove any directory entries that no longer exist
            in the old list.
         */
         old_nextp = &directory->file_data;
         while ((old_data = *old_nextp) != NULL)
         {
            if (old_data->errnum == ENOENT)
            {
               *old_nextp = old_data->next;
               FreeFileData(old_data, True);
            }
            else
               old_nextp = &old_data->next;
         }

         /* Append the old list to the end of the new list
            Replace the old list pointer with the new list pointer
         */
         if( tmp_ptr != NULL )
         {
            tmp_ptr->next = directory->file_data;
            directory->file_data = directory->new_data;
            directory->new_data = NULL;
         }
      }
      directory->new_tail = NULL;

      /* update the file count */
      directory->file_count = 0;
      directory->has_links = False;
      for (new_data = directory->file_data; new_data; new_data = new_data->next)
      {
         directory->file_count++;
         if (new_data->link != NULL)
            directory->has_links = True;
      }

      /* update directory timestamp */
      if (activity == activity_reading ||
          activity == activity_update_all ||
          directory->was_up_to_date)
      {
         DirStampStore(directory, &stamp);
      }
      if ((activity == activity_reading || activity == activity_update_all) &&
          directory->errnum == 0)
         directory->local_fs = stamp.local_fs;

      /*
       * Flush the Motif icon file cache for this directory.  If the
       * directory is on the icon search path, Motif has a cached
       * listing of it; dropping that lets icons added to it be found.
       * (This only drops that one listing, if it exists at all.)
       */
      strcpy (dirname, directory->path_name);
      DtEliminateDots(dirname);
      _DtFlushIconFileCache(dirname);

      /* reset busy flags */
      directory->busy[activity] = False;
      directory->activity = activity_idle;
      directory->was_up_to_date = True;
      directory->link_check_needed = False;


      /*
       * Fill dir_data field with information on the directory itself.
       * This data will be read when querying this view's top directory,
       * if the parent directory isn't already cached (tree mode)
       */
      for (new_data = directory->file_data;
           new_data != NULL;
           new_data = new_data->next)
        if (strcmp(new_data->file_name, ".") == 0)
        {

	    /*
	     * Found current directory information, now we make
	     * dir_data info from "." info
	     */

	    /* If we already have allocated space for dir_data free it */
	    if ( directory->dir_data != NULL )
	      FreeFileData(directory->dir_data, True);

	    directory->dir_data = (FileData *)XtMalloc(sizeof(FileData));

	    memcpy(directory->dir_data, new_data, sizeof(FileData));

	    /*
	     * Doctor up some of the information fields so that this doesn't
	     * seem to be a "." entry
	     */
	    directory->dir_data->next = NULL;
	    directory->dir_data->file_name =
	                         XtNewString(DName(directory->directory_name));
	    directory->dir_data->action_name = NULL;
	    if (directory->path_count > 0)
	    {
	      directory->dir_data->logical_type = XtNewString(
	             directory->path_logical_types[directory->path_count - 1]);
	    }
	    else
	      directory->dir_data->logical_type = NULL;
	    directory->dir_data->link = NULL;
	    directory->dir_data->final_link = NULL;
	    directory->dir_data->is_subdir = True;

	    break;
        }

      /*
       * Cause all views on this directory to be redrawn.  An update
       * that found nothing new (no entry, error or position changed)
       * leaves the views as they are: redrawing re-sorts, re-filters
       * and rebuilds every icon.
       */
      for (i = 0; i < directory->numOfViews; i++)
      {
         file_mgr_data = directory->directoryView[i].file_mgr_data;
         file_mgr_rec = (FileMgrRec *)file_mgr_data->file_mgr_rec;
         if (activity != activity_reading && !directory->update_changed &&
             file_mgr_data->desktop_file == NULL)
         {
            DPRINTF(("ReaddirPipeCallback: %s unchanged\n",
                     directory->directory_name));
            _DtTurnOffHourGlass(file_mgr_rec->shell);
            continue;
         }
         FileMgrRedisplayFiles(file_mgr_rec, file_mgr_data, False);
 	 if(file_mgr_data->desktop_file)
 	 {
             SelectDesktopFile(file_mgr_data);
             XtFree(file_mgr_data->desktop_file);
             file_mgr_data->desktop_file = NULL;
         }
      }
      XtFree(client_data);

      /* watch it for changes, or poll it */
      if (directory->errnum == 0)
         DirectoryWatch(directory);
      if (directory->ev_pending)
         EventTimerArm(EVENT_DELAY_MS);
      if (directory->wd <= 0 || directory->has_links)
         RestartTimer();

      /* an update failed: read it again to show the error */
      if (reread && !directory->busy[activity_reading])
         ReadDirectoryFiles(NULL, directory);

      /* schedule the next background activity */
      ScheduleActivity(directory);
}


/*--------------------------------------------------------------------
 *  ReadDirectoryFiles
 *    This routine is called to read a directory if the directory
 *    wasn't found in the cached, or if the directory has to be
 *    re-read because it changed.  This routine schedules a background
 *    process to be started that will do the actual work.
 *------------------------------------------------------------------*/

static void
ReadDirectoryFiles(
        Widget w,
        Directory *directory)
{
   FileMgrData *file_mgr_data;
   FileMgrRec *file_mgr_rec;
   int i;

#ifdef DT_PERFORMANCE
   /* Aloke Gupta */
      _DtPerfChkpntMsgSend("Begin Read Directory");
#endif

   /* make sure positionFileName is initialized */
   if (positionFileName == NULL)
      InitializePositionFileName();

   /* mark the directory busy reading */
   directory->busy[activity_reading] = True;

   /* arrange for background process to be started */
   ScheduleActivity(directory);

   /* make sure all views on this directory are marked busy */
   for (i = 0; i < directory->numOfViews; i++)
   {
      file_mgr_data = directory->directoryView[i].file_mgr_data;
      if (file_mgr_data->busy_status == not_busy)
      {
         file_mgr_data->busy_status = busy_readdir;
         file_mgr_data->busy_detail = 0;
         file_mgr_rec = (FileMgrRec *)file_mgr_data->file_mgr_rec;
         FileMgrRedisplayFiles(file_mgr_rec, file_mgr_data, False);
      }
      file_mgr_data->busy_status = busy_readdir;
   }
   return;
}


/*--------------------------------------------------------------------
 *  ReadDirectory
 *    Given a directory name, see if the directory is already cached.
 *    If so, return the file data list, otherwise, read the directory.
 *------------------------------------------------------------------*/

static Boolean
ReadDirectory(
        Widget w,
        char *host_name,
        char *directory_name,
        FileData **file_data,
        int *file_count,
        FileMgrData *file_mgr_data)
{
   Directory *directory;
   int i;
   char *err_msg;

   /* initialize return values */
   if (file_data != NULL)
   {
      *file_count = 0;
      *file_data = NULL;
   }

   /* see if the directory is already in the cache */
   directory = FindDirectory(host_name, directory_name);

   if ((directory != NULL) &&
       (strcmp(directory_name, directory->directory_name) == 0))
   {
      /* The directory is already in the cache. */
      directory->viewed = True;

      /* Look for the view in the view list */
      for (i = 0; i < directory->numOfViews; i++)
         if (directory->directoryView[i].file_mgr_data == file_mgr_data)
            break;

      /* If view not found, add to the view list */
      if (i == directory->numOfViews)
      {
         directory->directoryView = (DirectoryView *)
                                 XtRealloc ((char *) directory->directoryView,
                                            sizeof(DirectoryView) * (i + 1));
         directory->numOfViews++;
         directory->directoryView[i].file_mgr_data = file_mgr_data;
      }

      /* set mapped flag for the view */
      directory->directoryView[i].mapped = file_mgr_data->mapped;

      /* a cached directory no window showed: bring it up to date (now
         that the view is listed: the update types for it, e.g. toolbox) */
      if (directory->lru_stamp != 0)
         DirectoryRevalidate(directory);

      /* check if we need to popup an error message */
      if (directory->errmsg_needed &&
          !directory->busy[activity_reading] &&
          w != NULL)
      {
         err_msg = XtNewString(GetSharedMessage(CANNOT_READ_DIRECTORY_ERROR));
         FileOperationError (w, err_msg, directory_name);
         XtFree(err_msg);
         directory->errmsg_needed = False;
      }

      DPRINTF2(("ReadDirectory(\"%s\", \"%s\") returns cached\n",
                host_name, directory_name));
   }

   else
   {
      Tt_status tt_status;

      /* The directory is not yet in the cache. */

      /*  Expand the directory set array, if necessary.  */
      if (directory_count == directory_set_size)
      {
         directory_set_size += 10;
         directory_set = (Directory **) XtRealloc((char *)directory_set,
                                    sizeof(Directory **) * directory_set_size);
      }


      /*  Create and initialize a new directory entry  */
      directory_set[directory_count] = directory =
                                  (Directory *) XtCalloc (1, sizeof (Directory));
      directory_count++;

      directory->host_name = XtNewString (host_name);
      directory->directory_name = XtNewString (directory_name);
      directory->path_name = ResolveLocalPathName (host_name,
                                                   directory_name,
                                                   NULL,
                                                   home_host_name,
                                                   &tt_status );
      if (directory->path_name == NULL)
      {
         directory->path_name = (char *) XtMalloc(sizeof(char));
         directory->path_name[0]='\0';
      }
      directory->tt_path_name = NULL;
      directory->viewed = True;
      directory->file_count = 0;
      directory->numOfViews = 1;
      directory->errnum = 0;
      directory->errmsg_needed = False;
      directory->last_check = 0;
      directory->link_check_needed = False;
      directory->file_count = 0;
      directory->file_data = NULL;
      directory->new_data = NULL;
      directory->dir_data = NULL;
      directory->path_count = 0;
      directory->path_logical_types = NULL;
      directory->position_count = 0;
      directory->position_info = NULL;
      directory->modify_begin = 0;
      directory->modified_count = 0;
      directory->was_up_to_date = True;
      directory->modified_list = NULL;
      directory->modified_size = 0;
      directory->modified_index = NULL;
      directory->activity = activity_idle;
      for (i = 0; i < activity_idle; i++)
        directory->busy[i] = False;

      directory->directoryView = (DirectoryView *)
                                       XtMalloc (sizeof(DirectoryView));
      directory->directoryView[0].file_mgr_data = file_mgr_data;
      directory->directoryView[0].mapped = file_mgr_data->mapped;

      /*  Open the directory for reading and read the files.  */
      ReadDirectoryFiles (w, directory);
   }

   /* Restart refresh timer, if necessary */
   if (file_mgr_data->mapped)
      RestartTimer();

   /* return the file data */
   if (file_data != NULL && !directory->busy[activity_reading])
   {
      *file_count = directory->file_count;
      *file_data = directory->file_data;
   }

   return directory->busy[activity_reading];
}


/*--------------------------------------------------------------------
 *  _ReadDir
 *    Internal routine that recursively read a directory plus
 *    subdirectories down to a depth given by read_level.
 *------------------------------------------------------------------*/

static int
_ReadDir(
  Widget w,
  FileMgrData *file_mgr_data,
  char *host_name,
  char *directory_name,
  FileViewData *dp,         /* directory info */
  int level,                /* tree level of this directory */
  int read_level,           /* deepest level to be read */
  char **branch_list)       /* list of tree branches to expand */
/*
 * Recursively read a directory plus subdirectories down to a depth
 * given by read_level.
 */
{
  char subdir_name[MAX_PATH];
  FileData *fp, *file_data;
  FileViewData **lp = NULL, *ip;
  int i, n, rc;
  TreeShow ts;
  Boolean busy_reading;

  DPRINTF2(("_ReadDir(\"%s\", \"%s\"): level %d, read_level %d\n",
            host_name, directory_name, level, read_level));

  /* initialize list of descendents and counts */
  if (dp)
  {
    dp->desc = NULL;
    dp->ndir = dp->nfile = 0;
    lp = &dp->desc;
  } else
    ip = NULL;

  /* Read the directory content */
  busy_reading = ReadDirectory(w, host_name, directory_name,
		               &file_data, &n, file_mgr_data);
  if (busy_reading)
  {
    file_mgr_data->busy_status = busy_readdir;
    return 0;
  }

  if (n <= 0)
    return -1;

  level++;

  for (i = 0, fp = file_data; i < n && fp; i++, fp = fp->next) {

    /* initialize new dir entry */
    if (dp)
    {
       ip = (FileViewData *)XtMalloc(sizeof(FileViewData));
       memset(ip, 0, sizeof(FileViewData));
       ip->file_data = fp;
       ip->parent = dp;
       ip->ts = tsNotRead;
    }

    /* read subdirectory */
    if (fp->is_subdir)
    {
        /* construct sub directory name */
        strncpy(subdir_name, directory_name, MAX_PATH - 1);
        if (strlen(subdir_name) > 0
            && subdir_name[strlen(subdir_name) - 1] != '/')
            strncat(subdir_name, "/", MAX_PATH - 1);

        strncat(subdir_name, fp->file_name, MAX_PATH - 1);
        subdir_name[MAX_PATH - 1] = 0;

      /* see if we know this entry from branch_list */
      if (!QueryBranchList(file_mgr_data, branch_list, subdir_name, &ts))
        /* not known: assume we shouldn't read this subdir */
        ts = tsNotRead;

      if (level < read_level || ts != tsNotRead) {

        rc = _ReadDir(w, file_mgr_data, host_name, subdir_name, ip,
                      level, read_level, branch_list);
        if (ip == NULL)
         ;
        else if (rc)
          ip->ts = tsError;
        else if (ts >= tsReading)
          ip->ts = ts;
        else if (level >= file_mgr_data->tree_show_level)
          ip->ts = tsNone;
        else if (file_mgr_data->tree_files == TREE_FILES_ALWAYS)
          ip->ts = tsAll;
        else
          ip->ts = tsDirs;
      }
    }

    /* add new entry to linked list */
    if (dp && lp)
    {
      *lp = ip;
      lp = &ip->next;
    }
  }

  return 0;
}


/*--------------------------------------------------------------------
 *  ReadDir
 *    This is the main external entry point for reading directories.
 *------------------------------------------------------------------*/

int
ReadDir(
  Widget w,
  FileMgrData *file_mgr_data,
  char *host_name,
  char *directory_name,
  FileViewData *dp,         /* directory info */
  int level,                /* tree level of this directory */
  int read_level,           /* deepest level to be read */
  char **branch_list)       /* list of tree branches to expand */
{
   /* initially assume we are not busy */
   if (file_mgr_data->busy_status == not_busy)
      file_mgr_data->busy_detail = 0;

   file_mgr_data->busy_status = initiating_readdir;

   /* first pass: just check if any directory we need is busy */
   _ReadDir(w, file_mgr_data, host_name, directory_name, NULL, level,
            read_level, branch_list);

   /* if a directory we need is busy, return now */
   if (file_mgr_data->busy_status == busy_readdir)
      return 0;

   /*
    * All directories wee need are available.
    * Make a second pass for real.
    */
   file_mgr_data->busy_status = not_busy;
   return _ReadDir(w, file_mgr_data, host_name, directory_name, dp, level,
                   read_level, branch_list);
}


/*====================================================================
 *
 *  Routines that update the directory cache
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 *  FileWindowMapUnmap
 *    Update mapped flag in view lists.
 *------------------------------------------------------------------*/

void
FileWindowMapUnmap(
        FileMgrData *file_mgr_data)

{
   int i, j;

   for (i = 0; i < directory_count; i++)
   {
      for (j = 0; j < directory_set[i]->numOfViews; j++)
      {
         if (file_mgr_data == directory_set[i]->directoryView[j].file_mgr_data)
         {
            directory_set[i]->directoryView[j].mapped = file_mgr_data->mapped;
            break;
         }
      }
   }

   if (file_mgr_data->mapped)
   {
      RestartTimer();

      /* changes seen while it was iconified */
      for (i = 0; i < directory_count; i++)
         if (directory_set[i]->ev_pending)
         {
            EventTimerArm(EVENT_DELAY_MS);
            break;
         }
   }
}


/*--------------------------------------------------------------------
 *  RereadDirectory
 *    Read a directory already cached and update its contents.
 *------------------------------------------------------------------*/

void
RereadDirectory(
        Widget w,
        char *host_name,
        char *directory_name )
{
   Directory *directory;

   DPRINTF(("RereadDirectory(%s, %s)\n", host_name, directory_name));

   /*  Find the directory set entry.  */
   directory = FindDirectory(host_name, directory_name);
   if (directory != NULL)
   {
      /* reset errnum to make sure we'll get an error message */
      directory->errnum = 0;

      /* Read the directory. */
      if (!directory->busy[activity_reading])
         ReadDirectoryFiles(w, directory);
   }
}


/*--------------------------------------------------------------------
 *  UpdateDirectory
 *    Check if any files were added or deleted in a directory
 *    and update the directory contents accordingly.
 *------------------------------------------------------------------*/

void
UpdateDirectory(
        Widget w,
        char *host_name,
        char *directory_name )
{
   Directory *directory;

   DPRINTF(("UpdateDirectory(%s, %s)\n", host_name, directory_name));

   /*  Find the directory set entry.  */
   directory = FindDirectory(host_name, directory_name);
   if (directory != NULL)
   {
      /* arrange for directory contents to be checked */
      if (!directory->busy[activity_update_all])
      {
         directory->busy[activity_update_all] = True;
         ScheduleActivity(directory);
      }
   }
}


/*====================================================================
 *
 *  Directory modification routines:
 *
 *  The following routines are provided to avoid unnecessary
 *  re-reads of whole directories.  For example, if the user
 *  renames a file, it's only necessary to remove the old file
 *  from the directory and add it back under its new name; there
 *  is no need to read the whole directory again.  Similarly,
 *  when a file is dropped on a directory, it's only necessary
 *  to add the one new file to the directory.
 *
 *  To accomplish this, the routines that rename or copy files
 *  make the following calls:
 *
 *    DirectoryBeginModify():  called before doing the operation
 *    DirectoryFileModified(): called once for each affected file
 *    DirectoryEndModify():    called when the operation is completed
 *
 *  The routines remember which files were modified, and when
 *  DirectoryEndModify is called, a background process is started,
 *  that re-stats and types just those files.
 *
 *  A complication arises from automatic re-reads triggered by
 *  a periodic timer (routine TimerEvent).  Since renaming or
 *  copying files changes the timestamp on the directory, the
 *  automatic re-read would re-read the whole directory soon after
 *  the operation is done, nullifying our efforts to avoid
 *  unnecessary re-reads.  Therefore:
 *
 *    - We don't do any automatic re-reads between calls to
 *      DirectoryBeginModify and DirectoryEndModify.
 *
 *    - If the directory timestamp hadn't changed at the time
 *      of the DirectoryBeginModify, then when the directory
 *      update triggered by DirectoryEndModify finishes, we
 *      set the modify_time in the directory_set to the current
 *      timestamp of the directory.  This means that the next
 *      automatic re-read won't be triggered unless the directory
 *      is modified again after the DirectoryEndModify.
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 *  DirectoryAbortModify
 *    Decrement the modify_begin counter.
 *------------------------------------------------------------------*/

void
DirectoryAbortModify(
        char *host_name,
        char *directory_name)
{
   Directory *directory;

   DPRINTF(("DirectoryAbortModify(%s, %s)\n", host_name, directory_name));

   /*  Find the directory set entry.  */
   directory = FindDirectory(host_name, directory_name);
   if (directory != NULL)
   {
      directory->modify_begin--;

      if (directory->modify_begin == 0)
         directory->was_up_to_date = True;

      DPRINTF(("   modify_begin %d, up_to_date %d\n",
               directory->modify_begin, directory->was_up_to_date));
   }
}


/*--------------------------------------------------------------------
 *  DirectoryBeginModify
 *    Increment the modify_begin counter to suspend automatic
 *    re-reads until DirectoryEndModify is called.
 *------------------------------------------------------------------*/

void
DirectoryBeginModify(
        char *host_name,
        char *directory_name)
{
   Directory *directory;

   DPRINTF(("DirectoryBeginModify(%s, %s)\n", host_name, directory_name));

   /*  Find the directory set entry.  */
   directory = FindDirectory(host_name, directory_name);
   if (directory != NULL)
   {
      if (directory->modify_begin == 0)
         /* until we know better, assume the directory changed */
         directory->was_up_to_date = False;

      /* increment the modify_begin counter */
      directory->modify_begin++;

      DPRINTF(("   modify_begin %d, up_to_date %d\n",
               directory->modify_begin, directory->was_up_to_date));
   }
}


/*--------------------------------------------------------------------
 *  DirectoryModifyTime
 *    This routine should be called after DirectoryBeginModify and
 *    before doing any operation on the directory.  The parameter
 *    modify_time should be the current timestamp of the directory.
 *    By comparing the value to the modify_time stored in the
 *    directory set we decide whether the directory had already
 *    changed before the update operation began.
 *    Note: the reason for supplying a separate call for this check,
 *    instead of doing it inside DirectoryBeginModify(), is that we
 *    want to do the stat call that determines the current timestamp
 *    of the directory in a background process.  The background
 *    process that we start fo do the actual update is a convenient
 *    place to do this.
 *------------------------------------------------------------------*/

void
DirectoryModifyTime(
        char *host_name,
        char *directory_name,
        long modify_time)
{

   DPRINTF(("DirectoryModifyTime(%s, %s)\n", host_name, directory_name));

#ifdef SMART_DIR_UPDATE
   Directory *directory;

   /*  Find the directory set entry.  */
   directory = FindDirectory(host_name, directory_name);

   if (directory != NULL)
   {
      /* mark directory up-to-date if unchanged since last read */
      if (modify_time <= directory->modify_time)
         directory->was_up_to_date = True;
      DPRINTF(("   modify_begin %d, up_to_date %d\n",
               directory->modify_begin, directory->was_up_to_date));
   }
#endif
}


/*--------------------------------------------------------------------
 *  DirectoryFileModified
 *    This routine is called when we know that a file in a directory
 *    has been modified, added or removed.  The file name is added
 *    to the list of modified files.  The next time an update
 *    background process is started, it will check all the files
 *    on the modfied list and update the corresponding FileData.
 *------------------------------------------------------------------*/

void
DirectoryFileModified(
        char *host_name,
        char *directory_name,
        char *file_name)
{
   Directory *directory;
   int i;

   DPRINTF(("DirectoryFileModified(%s, %s, %s)\n",
            host_name, directory_name, file_name));

   /*  Find the directory set entry.  */
   directory = FindDirectory(host_name, directory_name);
   if (directory != NULL)
   {
      /* see if the file is already on the list (hashed: a move or copy
       * of K files calls this K times) */
      if (directory->modified_index == NULL)
      {
         directory->modified_index = XtNew(NameIndex);
         directory->modified_index->slots = NULL;
      }
      else if (NameIndexFind(directory->modified_index,
                             directory->modified_list, file_name, NULL) >= 0)
        return;

      /* add the file to the modified_list */
      i = directory->modified_count++;
      if (i >= directory->modified_size)
      {
         directory->modified_size = i < 8 ? 16 : 2 * i;
         directory->modified_list = (char **)
           XtRealloc((char *)directory->modified_list,
                     directory->modified_size * sizeof(char *));
      }
      directory->modified_list[i] = XtNewString(file_name);
      NameIndexAdd(directory->modified_index, directory->modified_list, i);
   }
}


/*--------------------------------------------------------------------
 *  DirectoryEndModify
 *    Start an update background process (will check all the files
 *    on the modfied list and update the corresponding FileData).
 *------------------------------------------------------------------*/

void
DirectoryEndModify(
        char *host_name,
        char *directory_name)
{
   Directory *directory;

   DPRINTF(("DirectoryEndModify(%s, %s)\n", host_name, directory_name));

   /*  Find the directory set entry.  */
   directory = FindDirectory(host_name, directory_name);

   /* arrange for an update background process to be scheduled */
   if (directory != NULL)
   {
      directory->modify_begin--;
      DPRINTF(("   modify_begin %d, up_to_date %d, modified_count %d\n",
               directory->modify_begin,
               directory->was_up_to_date,
               directory->modified_count));
      if (directory->modified_count > 0)
      {
         Directory *subdir;
         char subdir_name[MAX_PATH + 1];
         char *p;
         int i;

         /*
          * If any of the modifed files is a subdirectory that we have
          * cached, schedule an activity_checking_dir to make sure that
          * the subdirectory is still readable.
          */
         strcpy(subdir_name, directory_name);
         p = subdir_name + strlen(subdir_name);
         if (p[-1] != '/')
           *p++ = '/';

         for (i = 0; i < directory->modified_count; i++)
         {
            strcpy(p, directory->modified_list[i]);
            subdir = FindDirectory(host_name, subdir_name);
            if (subdir)
            {
               DPRINTF(("   schedule check for subdir \"%s\"\n",
                        directory->modified_list[i]));
               subdir->busy[activity_checking_dir] = True;
               ScheduleActivity(subdir);
            }
         }

#ifdef SMART_DIR_UPDATE
         /* schedule a partial update of the modfied directory */
         if (directory->was_up_to_date)
            directory->busy[activity_update_some] = True;
         else
            directory->busy[activity_update_all] = True;
#else
         /* schedule a full update of the modfied directory */
         directory->busy[activity_update_all] = True;
#endif
         ScheduleActivity(directory);
      }
   }
}


/*--------------------------------------------------------------------
 *  UpdateDirectorySet
 *    We call this when we do a database update. It loops through
 *    the directory_set list and rereads each directory.
 *------------------------------------------------------------------*/

void
UpdateDirectorySet( void )
{
   int i;

   DPRINTF(("UpdateDirectorySet ...\n"));

   /* sticky procs type with the database they were forked with */
   db_generation++;
   StickyProcsRetire();

   for (i = 0; i < directory_count; i++)
   {
      /* a cached directory nobody looks at is read when shown again */
      if (directory_set[i]->lru_stamp != 0)
         directory_set[i]->reread_needed = True;
      else if (!directory_set[i]->busy[activity_reading])
         ReadDirectoryFiles (NULL, directory_set[i]);
   }
}



/*--------------------------------------------------------------------
 *  UpdateCachedDirectories
 *    Update view list for all cached directories.
 *    Throw out any directories that are no longer being viewed.
 *------------------------------------------------------------------*/

void
UpdateCachedDirectories(
        View **view_set,
        int view_count)
{
   DialogData * dialog_data;
   FileMgrData * file_mgr_data;
   int i, j, k, n;
   Directory *directory;

   /*
    * First step:
    *   clear the view list in all directory set entries
    */
   for (i = 0; i < directory_count; i++)
   {
      if( !(strcmp(directory_set[i]->directory_name, trash_dir) == 0) )
      {
         XtFree ((char *) directory_set[i]->directoryView);
         directory_set[i]->numOfViews = 0;
         directory_set[i]->directoryView = NULL;
         directory_set[i]->viewed = False;
      }
   }


   /*
    * Second step:
    *   reconstruct view lists by adding each directory found in the view
    *   set to the view list for the corresponding directory set entry
    */
   for (j = 0; j < view_count; j++)
   {
      dialog_data = (DialogData *) view_set[j]->dialog_data;
      file_mgr_data = (FileMgrData *) dialog_data->data;

      /* loop through all direcories in this view */
      for (k = 0; k < file_mgr_data->directory_count; k++)
      {
         /* find the directory in the directory set */
         directory = FindDirectory(view_set[j]->host_name,
                                   file_mgr_data->directory_set[k]->name);

         /* we expect the directory to be found; if not, something is wrong */
         if (directory == NULL)
         {
            fprintf(stderr, "Warning: %s:%s not found in directory set.\n",
                    view_set[j]->host_name,
                    file_mgr_data->directory_set[k]->name);
            continue;
         }

         /* add the directory to the view list */
         n = directory->numOfViews;
         directory->directoryView = (DirectoryView *)
                                  XtRealloc ((char *) directory->directoryView,
                                             sizeof(DirectoryView) * (n + 1));
         directory->directoryView[n].file_mgr_data = file_mgr_data;
         directory->directoryView[n].mapped = file_mgr_data->mapped;
         directory->numOfViews++;
         directory->viewed = True;
         if (directory->lru_stamp != 0)
            DirectoryRevalidate(directory);
      }
   }


   /*
    * Third step:
    *   directories that have empty view lists stay cached for a while
    *   (unless they could not be read); the least recently used of
    *   them go when there are too many.
    */
   i = 0;
   while (i < directory_count)
   {
      directory = directory_set[i];
      if (directory->numOfViews > 0 ||
          strcmp(directory->directory_name, trash_dir) == 0)
      {
         /* Keep this directory in the directory set. */
         i++;
      }
      else if (directory->errnum == 0)
      {
         /* Keep it cached, unviewed. */
         if (directory->lru_stamp == 0)
            directory->lru_stamp = ++lru_clock;
         i++;
      }
      else
      {
         /* Delete the file data and remove from the directory set. */
         DirectoryCacheRemove(i);
      }
   }
   DirectoryCacheTrim();

   /* Restart refresh timer, if necessary */
   RestartTimer();
}


/*====================================================================
 *
 * Routines that return directory data
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 * UserName, GroupName
 *   Small caches of user and group names for GetLongName.  (A file
 *   view by attributes used to look up the owner of every file unless
 *   it was the same as the previous file's.)
 *------------------------------------------------------------------*/

#define ID_NAME_CACHE_SIZE 16
#define ID_NAME_LEN        64

typedef struct
{
   Boolean used;
   long    id;
   char    name[ID_NAME_LEN];
} IdName;

static IdName user_names[ID_NAME_CACHE_SIZE];
static IdName group_names[ID_NAME_CACHE_SIZE];
static int    next_user_slot, next_group_slot;

static IdName *
FindIdName(
        IdName *cache,
        long id)
{
   int i;

   for (i = 0; i < ID_NAME_CACHE_SIZE; i++)
      if (cache[i].used && cache[i].id == id)
         return &cache[i];
   return NULL;
}

static char *
UserName(
        uid_t uid)
{
   IdName *e = FindIdName(user_names, (long)uid);
   struct passwd * user_data;

   if (e == NULL)
   {
      e = &user_names[next_user_slot];
      next_user_slot = (next_user_slot + 1) % ID_NAME_CACHE_SIZE;
      e->used = True;
      e->id = (long)uid;
      user_data = getpwuid (uid);
      if (user_data)
         snprintf(e->name, sizeof(e->name), "%s", user_data->pw_name);
      else
         snprintf(e->name, sizeof(e->name), "%ld", (long)uid);
   }
   return e->name;
}

static char *
GroupName(
        gid_t gid)
{
   IdName *e = FindIdName(group_names, (long)gid);
   struct group * group_data;

   if (e == NULL)
   {
      e = &group_names[next_group_slot];
      next_group_slot = (next_group_slot + 1) % ID_NAME_CACHE_SIZE;
      e->used = True;
      e->id = (long)gid;
      group_data = getgrgid (gid);
      if (group_data && group_data->gr_name[0] != '\0')
         snprintf(e->name, sizeof(e->name), "%s", group_data->gr_name);
      else
         strcpy(e->name, "root");
   }
   return e->name;
}


/*--------------------------------------------------------------------
 * GetLongName
 *   Return a string that contains file information similar to "ls -l",
 *   including: permissions, owner, modified time, link (if any).
 *   Used for "view by attributes"
 *
 *   Example:
 *     -rw-r--r--  dld  staff  108314  Jul 26 15:16:36 1993 Directory.c
 *
 *------------------------------------------------------------------*/

char *
GetLongName(
        FileData *file_data )
{
#ifdef NLS16
   struct tm * tms;
   struct tm tm_buf;
   char time_string[100];
#else
   char * time_string;
#endif /* NLS16 */
   char link_path[MAX_PATH + 5];
   char * group_name = "";
   char * user_name = "";
   char long_name[MAX_PATH * 3];
   time_t long_modify_time;
   char permission;
   char usr_read_priv, usr_write_priv, usr_exec_priv;
   char grp_read_priv, grp_write_priv, grp_exec_priv;
   char oth_read_priv, oth_write_priv, oth_exec_priv;

   /*  Generate the long list name (copied to the heap at the end).  */
   long_name[0]='\0';

   /* Initially, assume their is not a soft link */
   link_path[0] = '\0';

   if (file_data->errnum == 0)
   {
     group_name = GroupName(file_data->stat.st_gid);
     user_name = UserName(file_data->stat.st_uid);
   }
   else
   {
      char error_msg[1024];
      int msg_len;

      /* determine how much space we have for an error message */
      long_modify_time = 747616435;
                         /* just needed to determine the length of a date */
#ifdef NLS16
      tms = localtime_r(&long_modify_time, &tm_buf);
      strftime( time_string, 100,
                GetSharedMessage(DIRECTORY_DATE_FORMAT),
                tms);
#else
      time_string = ctime ((time_t *)&long_modify_time);
      time_string[strlen(time_string)-1] = 0x0;
      time_string += 4;
#endif
      msg_len = 10 + 3 + 9 + 1 + 9 + 1 + 9 + 1 + strlen(time_string);

      /* generate the error message */
      strcpy(error_msg, "(");
      strncpy(error_msg + 1, strerror(file_data->errnum), msg_len - 2);
      error_msg[msg_len - 1] = '\0';
      strcat(error_msg, ")");

      sprintf( long_name, "%-28.28s  %s  %9d %s",
                            file_data->file_name,
                            time_string,
                            0, error_msg );

      return XtNewString(long_name);
   }


   /* Build the permission string  */
   switch( file_data->stat.st_mode & S_IFMT )
   {
   case S_IFDIR:
     permission = 'd';
     break;
   case S_IFCHR:
     permission = 'c';
     break;
   case S_IFBLK:
     permission = 'b';
     break;
   case S_IFLNK:
     permission = 'l';
     break;
   default :
     permission = OPTION_OFF;
     break;
   }

   if (file_data->stat.st_mode & S_IRUSR) usr_read_priv = READ_PRIV;
   else usr_read_priv = OPTION_OFF;

   if (file_data->stat.st_mode & S_IWUSR) usr_write_priv = WRITE_PRIV;
   else usr_write_priv = OPTION_OFF;

   if (file_data->stat.st_mode & S_IXUSR) usr_exec_priv = EXEC_PRIV;
   else usr_exec_priv = OPTION_OFF;


   if (file_data->stat.st_mode & S_IRGRP) grp_read_priv = READ_PRIV;
   else grp_read_priv = OPTION_OFF;

   if (file_data->stat.st_mode & S_IWGRP) grp_write_priv = WRITE_PRIV;
   else grp_write_priv = OPTION_OFF;

   if (file_data->stat.st_mode & S_IXGRP) grp_exec_priv = EXEC_PRIV;
   else grp_exec_priv = OPTION_OFF;


   if (file_data->stat.st_mode & S_IROTH) oth_read_priv = READ_PRIV;
   else oth_read_priv = OPTION_OFF;

   if (file_data->stat.st_mode & S_IWOTH) oth_write_priv = WRITE_PRIV;
   else oth_write_priv = OPTION_OFF;

   if (file_data->stat.st_mode & S_IXOTH) oth_exec_priv = EXEC_PRIV;
   else oth_exec_priv = OPTION_OFF;


   long_modify_time = file_data->stat.st_mtime;
#ifdef NLS16
   tms = localtime_r(&long_modify_time, &tm_buf);
   strftime( time_string, 100,
             GetSharedMessage(DIRECTORY_DATE_FORMAT),
             tms);
#else
   time_string = ctime ((time_t *)&long_modify_time);
   time_string[strlen(time_string)-1] = 0x0;
   time_string += 4;
#endif

   /* Fill in the name of where the link goes */
   if (file_data->link)
   {
     strcpy( link_path, " -> " );
     strcpy( link_path + 4, file_data->link );
   }

   {
#define ELLIPSIS " (...) "
#define NAME_PRECISION 28
     if (! is_multibyte)
     {
       int len = strlen( file_data->file_name );
       if( len > NAME_PRECISION )
       {
	 char name[NAME_PRECISION];
	 sprintf( name, "%-20.20s%s", file_data->file_name, ELLIPSIS);

	 sprintf( long_name, "%-28.28s  %s  %9ld  %c%c%c%c%c%c%c%c%c%c  %-9s  %-9s  %s",
		  name,
		  time_string,
		  (long)file_data->stat.st_size,
		  permission,
		  usr_read_priv, usr_write_priv, usr_exec_priv,
		  grp_read_priv, grp_write_priv, grp_exec_priv,
		  oth_read_priv, oth_write_priv, oth_exec_priv,
		  user_name, group_name,
		  link_path );
       }
       else
       {
	 sprintf( long_name, "%-28.28s  %s  %9ld  %c%c%c%c%c%c%c%c%c%c  %-9s  %-9s  %s",
		  file_data->file_name,
		  time_string,
		  (long)file_data->stat.st_size,
		  permission,
		  usr_read_priv, usr_write_priv, usr_exec_priv,
		  grp_read_priv, grp_write_priv, grp_exec_priv,
		  oth_read_priv, oth_write_priv, oth_exec_priv,
		  user_name, group_name,
		  link_path );
       }
     } else {
       /* MULTIBYTE
	*
	* sprintf() counts width in bytes (not characters), moreover,
	* it fails (returns -1 and produces no output) if input string is not
	* a valid multibyte string (at least the glibc version), but we can't fail
	* to display a file because it's name has some invalid characters). So it looks
	* that instead of using sprintf() we have to format the file name part manually.
	*/
       int len = DtCharCount( file_data->file_name );
       int copy_len =  len > NAME_PRECISION ?  NAME_PRECISION - sizeof(ELLIPSIS) + 1 : len;
       int byte_len = 0;
       int count;

       long_name[0]='\0';
       /* properly copy copy_len characters of the multibyte string
	  replacing invalid chars with '?' */
       for (count = 0;
	    (count < copy_len) && *(file_data->file_name + byte_len);
	    count ++)
       {
	 int chr_bytes = mblen(file_data->file_name + byte_len, MB_CUR_MAX);
	 if (chr_bytes > 0)
	 {
	   strncpy(long_name + byte_len, file_data->file_name + byte_len, chr_bytes);
	 }
	 else if (chr_bytes < 0)
	 { /* invalid char */
	   chr_bytes = 1;
	   long_name[byte_len]='?';
	 }
	 else
	 {
	   /* null-wide character, won't really happen */
	   break;
	 }
	 byte_len+=chr_bytes;
       }
       if (copy_len < len)
       {
	 /* truncated name, add ellipsis */
	 strncpy(long_name + byte_len, ELLIPSIS, sizeof(ELLIPSIS) - 1);
	 byte_len+= sizeof(ELLIPSIS) - 1;
       }
       else
       {
	 /* full name, pad it with spaces up to the proper length */
	 for (; count <  NAME_PRECISION ; count++)
	 {
	   long_name[byte_len++]=' ';
	 }
       }
       sprintf( long_name + byte_len, "  %s  %9ld  %c%c%c%c%c%c%c%c%c%c  %-9s  %-9s  %s",
		time_string,
		(long)file_data->stat.st_size,
		permission,
		usr_read_priv, usr_write_priv, usr_exec_priv,
		grp_read_priv, grp_write_priv, grp_exec_priv,
		oth_read_priv, oth_write_priv, oth_exec_priv,
		user_name, group_name,
		link_path );
     } /* is_multibyte */
   }

   return XtNewString(long_name);
}



/*--------------------------------------------------------------------
 *  DirectoryBusy
 *    See if path has a directory view of it or if any sub-directories
 *    of it are viewed.   The path parameter is of the for /foo/bar
 *------------------------------------------------------------------*/

Boolean
DirectoryBusy(
        char *path )
{
   FileMgrData * file_mgr_data;
   FileViewData  * sub_root;
   int i, j, k;
   int len = strlen(path);

   for (i = 0; i < directory_count; i++)
   {
      /* check if this directory is equal to 'path' or a subdir of 'path' */
      if (directory_set[i]->viewed &&
          (strcmp (directory_set[i]->path_name, path) == 0 ||
           (strncmp (directory_set[i]->path_name, path,len) == 0 &&
             directory_set[i]->path_name[len] == '/')
           ||
           (directory_set[i]->tt_path_name != NULL &&
           (strcmp (directory_set[i]->tt_path_name, path) == 0 ||
            (strncmp (directory_set[i]->tt_path_name, path,len) == 0 &&
              directory_set[i]->tt_path_name[len] == '/')))))
      {
         /* check the views in the view list */
         for (j = 0; j < directory_set[i]->numOfViews; j++)
         {
            file_mgr_data = directory_set[i]->directoryView[j].file_mgr_data;

            /* find the dir in the directory set for this view */
            for (k = 0; k < file_mgr_data->directory_count; k++)
               if (strcmp(file_mgr_data->directory_set[k]->name,
                          directory_set[i]->directory_name) == 0)
               {
                  break;
               }
            if (k == file_mgr_data->directory_count)
              continue;  /* not found ... something must be wrong! */

            /*
             * Check if this directory is acutally visible.
             * If the directory is in a tree branch that is not currently
             * expanded, it is not visible and would not be considered busy.
             */

            /* the tree root is always considered busy */
            if (k == 0)
              return True;

            /* a subdir is considered busy if it is visible and at least
             * partially expanded */
            sub_root = file_mgr_data->directory_set[k]->sub_root;
            if (sub_root->displayed &&
                ((sub_root->ts == tsDirs && sub_root->ndir > 0)  ||
                 (sub_root->ts == tsAll &&
                   sub_root->ndir + sub_root->nfile > 0)))
            {
              return True;
            }
         }
      }
   }

   return (False);
}


/*--------------------------------------------------------------------
 *  GetDirectoryLogicalType
 *     Get logical type for the iconic path.
 *------------------------------------------------------------------*/

char *
GetDirectoryLogicalType(
	FileMgrData *file_mgr_data,
	char *path)
{
   int len;
   int n;
   Directory *directory;
   char *ptr;

   /* 'path' must be a prefix of the current directory */
   len = strlen(path);
   if (strncmp(file_mgr_data->current_directory, path, len) != 0 ||
       (len > 1 &&
       file_mgr_data->current_directory[len] != '/' &&
       file_mgr_data->current_directory[len] != '\0'))
   {
      DPRINTF(("GetDirectoryLogicalType(%s): len %d, cur_dir %s\n",
                path, len, file_mgr_data->current_directory));
      return NULL;
   }

   /*  Find the directory set entry.  */
   directory = FindDirectory(file_mgr_data->host,
                             file_mgr_data->current_directory);
   if ((directory != NULL) &&
       (strcmp(file_mgr_data->current_directory,
               directory->directory_name) == 0))
   {
      /* if we don't have path_logical_types yet, we don't know */
      if (directory->path_logical_types == NULL)
         return NULL;

      /* count the number of components in path */
      if (strcmp(path, "/") == 0)
         n = 0;
      else
      {
         n = 1;
         ptr = path + 1;
         while ((ptr = DtStrchr(ptr, '/')) != NULL)
         {
            ptr = ptr + 1;
            if (*ptr == '\0')
              break;
            else
              n++;
         }
      }

      DPRINTF2(("GetDirectoryLogicalType(%s): n %d, type %s\n",
                path, n, directory->path_logical_types[n]));

      /* return type form path_logical_types array */
      return directory->path_logical_types[n];
   }

   /* directory not found in directory_set */

   return NULL;
}


/*====================================================================
 *
 * Routines for accessing position information
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 *  GetDirectoryPositionInfo
 *     Get cached position info
 *------------------------------------------------------------------*/

int
GetDirectoryPositionInfo(
        char *host_name,
        char *directory_name,
        PositionInfo **position_info)
{
   Directory *directory;

   directory = FindDirectory(host_name, directory_name);
   if (directory == NULL)
      return -1;

   *position_info = directory->position_info;

   return directory->position_count;
}


/*--------------------------------------------------------------------
 *  WritePosInfoProcess
 *    Main routine of the background process that writes the
 *    postion information file.
 *------------------------------------------------------------------*/

/* write (or, if position_count is 0, remove) a position file */
static int
WritePosInfoFile(
        const char *fileName,
        int position_count,
        PositionInfo *position_info)
{
   FILE *f;
   int i, rc;

   /* Remove old files, if no position information for this view */
   if (position_count <= 0)
      rc = unlink(fileName);
   else
   {
      /* open the file for writing */
      f = fopen(fileName, "w");

      if (f == NULL)
      {
         /* Assume read-only directory, if we can't open the file */
         rc = 0;
      }
      else
      {
         chmod(fileName, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

         fprintf(f, "%d\n", position_count);
         for (i = 0; i < position_count; i++)
         {
            fprintf(f, "%s\n%d %d %d\n",
                    position_info[i].name,
                    position_info[i].x,
                    position_info[i].y,
                    position_info[i].stacking_order);
         }

         fclose(f);
         rc = 0;
      }
   }
   return rc;
}

static char *
PosInfoFileName(
	Directory *directory)
{
   Tt_status tt_status;

   /* Don't have to check for tt_status
      directory->host_name is home_host_name and ResolveLocalPathName will
      always return a good path
   */
   return ResolveLocalPathName( directory->host_name,
                                directory->directory_name, positionFileName,
                                home_host_name, &tt_status );
}

static int
WritePosInfoProcess(
        int pipe_fd,
	Directory *directory,
	ActivityStatus activity)
{
   char *fileName;
   int rc;

   /* construct the full file name */
   fileName = PosInfoFileName(directory);
   DPRINTF(("WritePosInfoProcess: count %d, file %s\n",
            directory->position_count, fileName));

   rc = WritePosInfoFile(fileName, directory->position_count,
                         directory->position_info);

   /* send result back thorugh the pipe */
   DPRINTF(("WritePosInfoProcess: done (rc %d)\n", rc));
   write(pipe_fd, &rc, sizeof(int));
   XtFree( fileName );
   return 0;
}


/*--------------------------------------------------------------------
 *  StartPosInfoThread
 *    Write the position information file in a thread instead of a
 *    forked copy of the whole process: it is a few lines of text.  The
 *    thread works on a copy of the data, uses no X, Xt or ToolTalk
 *    calls, and reports through the pipe like WritePosInfoProcess, so
 *    WritePosInfoPipeCallback handles both.  Returns 0 and the pipe's
 *    read end in *pipe_fd, or -1 (then the caller forks as before).
 *------------------------------------------------------------------*/

typedef struct
{
   char         *file_name;
   int           count;
   PositionInfo *info;
   int           pipe_fd;
} PosInfoJob;

static void
PosInfoJobFree(
        PosInfoJob *job)
{
   int i;

   for (i = 0; i < job->count; i++)
      free(job->info[i].name);
   free(job->info);
   free(job->file_name);
   free(job);
}

/*
 * A forked writer finished its file even if dtfile exited meanwhile; a
 * thread dies with the process, leaving a truncated file.  So exit()
 * waits (a little) for the writer threads still running.
 */
static pthread_mutex_t posinfo_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  posinfo_cond = PTHREAD_COND_INITIALIZER;
static int             posinfo_threads = 0;
static pid_t           posinfo_pid = 0;     /* the process that has them */

static void
PosInfoThreadsWait(void)
{
   struct timespec deadline;

   /* (forked children inherit the count, but not the threads) */
   if (getpid() != posinfo_pid)
      return;

   clock_gettime(CLOCK_REALTIME, &deadline);
   deadline.tv_sec += 2;
   pthread_mutex_lock(&posinfo_lock);
   while (posinfo_threads > 0)
      if (pthread_cond_timedwait(&posinfo_cond, &posinfo_lock,
                                 &deadline) != 0)
         break;
   pthread_mutex_unlock(&posinfo_lock);
}

static void
PosInfoThreadsAdd(
        int n)
{
   pthread_mutex_lock(&posinfo_lock);
   posinfo_threads += n;
   if (posinfo_threads == 0)
      pthread_cond_broadcast(&posinfo_cond);
   pthread_mutex_unlock(&posinfo_lock);
}

static void *
PosInfoThread(
        void *arg)
{
   PosInfoJob *job = (PosInfoJob *)arg;
   int rc;
   ssize_t n;

   rc = WritePosInfoFile(job->file_name, job->count, job->info);
   PosInfoThreadsAdd(-1);
   do
      n = write(job->pipe_fd, &rc, sizeof(int));
   while (n < 0 && errno == EINTR);
   close(job->pipe_fd);
   PosInfoJobFree(job);
   return NULL;
}

static int
StartPosInfoThread(
	Directory *directory,
	int *pipe_fd)
{
   PosInfoJob *job;
   pthread_attr_t attr;
   pthread_t thread;
   char *name;
   int fds[2];
   int i, rc;

   job = (PosInfoJob *) calloc(1, sizeof(PosInfoJob));
   if (job == NULL)
      return -1;
   name = PosInfoFileName(directory);
   job->file_name = name ? strdup(name) : NULL;
   XtFree(name);
   job->count = directory->position_count > 0 ? directory->position_count : 0;
   job->info = (PositionInfo *) calloc(job->count ? job->count : 1,
                                       sizeof(PositionInfo));
   if (job->file_name == NULL || job->info == NULL)
   {
      job->count = 0;
      PosInfoJobFree(job);
      return -1;
   }
   for (i = 0; i < job->count; i++)
   {
      job->info[i] = directory->position_info[i];
      job->info[i].name = strdup(directory->position_info[i].name);
      if (job->info[i].name == NULL)
      {
         job->count = i;
         PosInfoJobFree(job);
         return -1;
      }
   }

   if (pipe(fds) < 0)
   {
      PosInfoJobFree(job);
      return -1;
   }
   (void) fcntl(fds[0], F_SETFD, FD_CLOEXEC);
   (void) fcntl(fds[1], F_SETFD, FD_CLOEXEC);
   job->pipe_fd = fds[1];

   if (posinfo_pid == 0)
   {
      posinfo_pid = getpid();
      atexit(PosInfoThreadsWait);
   }

   PosInfoThreadsAdd(1);
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   rc = pthread_create(&thread, &attr, PosInfoThread, job);
   pthread_attr_destroy(&attr);
   if (rc != 0)
   {
      PosInfoThreadsAdd(-1);
      close(fds[0]);
      close(fds[1]);
      PosInfoJobFree(job);
      return -1;
   }

   DPRINTF(("StartPosInfoThread: %d positions, file %s\n",
            directory->position_count, directory->directory_name));
   *pipe_fd = fds[0];
   return 0;
}


/*--------------------------------------------------------------------
 *  WritePosInfoPipeCallback
 *    Callback routine that reads the return code sent through the
 *    pipe from the WritePosInfoProcess background process.
 *------------------------------------------------------------------*/

static void
WritePosInfoPipeCallback(
   XtPointer client_data,
   int *fd,
   XtInputId *id)
{
   PipeCallbackData *pipe_data = (PipeCallbackData *)client_data;
   Directory *directory = pipe_data->directory;
   int rc;

   /* get return code from the pipe */
   rc = -1;
   PipeRead(*fd, &rc, sizeof(int));

   /* close the pipe and cancel the callback */
   close(*fd);
   XtFree( client_data );
   XtRemoveInput(*id);

   /* verify that the directory still exists */
   if (DirectoryGone(directory))
   {
      ScheduleActivity(NULL);
      return;
   }

   DPRINTF(("WritePosInfoPipeCallback: rc %d\n", rc));

   /* reset the busy flag and schedule new work, if any */
   directory->busy[activity_writing_posinfo] = False;
   directory->activity = activity_idle;
   ScheduleActivity(directory);
}


/*--------------------------------------------------------------------
 *  SetDirectoryPositionInfo
 *     Update cached position info.  This routine schedules a
 *     background process that writes the modified information
 *     to the position info file.
 *------------------------------------------------------------------*/

int
SetDirectoryPositionInfo(
        char *host_name,
        char *directory_name,
        int position_count,
        PositionInfo *position_info)
{
   Directory *directory;
   Boolean unchanged;
   int i, j;

   /* find the directory */
   directory = FindDirectory(host_name, directory_name);
   if (directory == NULL)
      return -1;

   /* check if anything has changed (a hash of the old names: in "as
    * placed" mode every file has a position, and comparing each new
    * name with all old ones was O(N^2)) */
   if (directory->position_count == position_count)
   {
      NameIndex old_index;
      char **old_names;

      old_names = (char **) XtMalloc((position_count + 1) * sizeof(char *));
      for (j = 0; j < position_count; j++)
         old_names[j] = directory->position_info[j].name;
      NameIndexInit(&old_index, old_names, position_count);

      unchanged = True;
      for (i = 0; i < position_count && unchanged; i++)
      {
         j = NameIndexFind(&old_index, old_names, position_info[i].name, NULL);
         if (j < 0)
            j = position_count;

         if (j == position_count ||
             position_info[i].x != directory->position_info[j].x ||
             position_info[i].y != directory->position_info[j].y ||
             position_info[i].stacking_order !=
                                    directory->position_info[j].stacking_order)
         {
            unchanged = False;
         }
      }
      NameIndexFree(&old_index);
      XtFree((char *)old_names);

      /* if nothing changed, don't do anything */
      if (unchanged)
         return 0;
   }

   /* free old position info  names*/
   for (i = 0; i < directory->position_count; i++)
      XtFree(directory->position_info[i].name);

   /* realloc array, if necessary */
   if (directory->position_count != position_count)
   {
      directory->position_count = position_count;
      directory->position_info =
            (PositionInfo *) XtRealloc((char *)directory->position_info,
                                        position_count * sizeof(PositionInfo));
   }

   /* replace old position info */
   for (i = 0; i < position_count; i++)
   {
      directory->position_info[i].name = XtNewString(position_info[i].name);
      directory->position_info[i].x = position_info[i].x;
      directory->position_info[i].y = position_info[i].y;
      directory->position_info[i].stacking_order =
                                              position_info[i].stacking_order;
   }

   /* make sure positionFileName is initialized */
   if (positionFileName == NULL)
      InitializePositionFileName();

   /* start background process that writes the position info file */
   directory->busy[activity_writing_posinfo] = True;
   ScheduleActivity(directory);

   return 0;
}


/*====================================================================
 *
 * Timer functions
 *   These function are periodically called to scan all cached
 *   directories to see if any have been modified since last read.
 *   If any have, a background process is scheduled to re-read
 *   the directory.
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 * SkipRefresh:
 *   Decide whether to skip an automatic re-read.
 *   (We don't do re-reads on directories that aren't currently
 *    being viewed and on the trash directory, if trash is currently
 *    being emptied.)
 *------------------------------------------------------------------*/

static Boolean
SkipRefresh(
	Directory *directory)
{
   int i;

   /* don't refresh while the directory is being modified */
   if (directory->modify_begin > 0)
      return True;

   /* verify that the directory is still being viewed */
   if (!directory->viewed)
      return True;

   for (i = 0; i < directory->numOfViews; i++)
      if (directory->directoryView[i].mapped)
         break;
   if (i == directory->numOfViews)
      return True;

   /* if trash is being emptied and this is the trash dir, skip it */
   if (removingTrash && strcmp(directory->directory_name, trash_dir) == 0)
      return True;

   return False;
}


/*--------------------------------------------------------------------
 * LinkChanged
 *   Is the symbolic link name in the directory full_name (namep
 *   points past its '/') no longer a link, or no longer of the kind
 *   (LINK_VALID, LINK_RECURSIVE, LINK_BROKEN) it was?
 *------------------------------------------------------------------*/

static Boolean
LinkChanged(
        char *full_name,
        char *namep,
        const char *name,
        int prev_link_kind)
{
   struct stat stat_buf;
   int cur_link_kind;

   if (namep + strlen(name) >= full_name + MAX_PATH)
      return False;
   strcpy(namep, name);

   /* Check if the file is still a symbolic link */
   if (lstat(full_name, &stat_buf) != 0 || !S_ISLNK(stat_buf.st_mode))
      return True;

   /* Check what kind of link it is now */
   if (_DtFollowLink(full_name) == NULL)
      cur_link_kind = LINK_RECURSIVE;
   else if (stat(full_name, &stat_buf) != 0)
      cur_link_kind = LINK_BROKEN;
   else
      cur_link_kind = LINK_VALID;

   /* now we can tell if the link has changed */
   return prev_link_kind != cur_link_kind;
}


/*--------------------------------------------------------------------
 * TimerEventProcess
 *   Main routine for the background process that checks directory
 *   timestamps and the status of links.
 *------------------------------------------------------------------*/

static int
TimerEventProcess(
        int pipe_fd,
	Directory *directory,
	ActivityStatus activity)
{
   struct stat stat_buf;
   long modify_time;
   DirStamp stamp;
   Boolean link_changed;
   FileData *file_data;
   char full_name[MAX_PATH];
   char *namep;
   int rc;
   int i;
   PipeBuf pb = { NULL, 0, 0 };

   /*
    * Do a stat on the directory to get its last-modified time.
    * Also check if we  still have read and execute/search permisssion.
    *
    * Note:
    *  It is important to get the timstamp in exactly the same way that the
    *  ReadDirectoryProcess does it; otherwise, we might get into a loop,
    *  where TimerEventProcess detects that the directory has changed
    *  and triggers ReadDirectoryProcess, but ReadDirectoryProcess won't
    *  be able to get a new timestamp and update the directory structure,
    *  so the next time TimerEventProcess runs it will trigger another
    *  ReadDirectoryProcess, and so on ...
    */
   if (CheckAccess(directory->path_name, R_OK | X_OK) != 0 ||
       stat(directory->path_name, &stat_buf) != 0)
   {
      /* stat or access failed */
      rc = errno;
      modify_time = 0;
      DirStampSet(&stamp, NULL, False);
   }
   else
   {
      /* stat succeeded and the directory is still readable */
      rc = 0;
      modify_time = stat_buf.st_mtime;
      DirStampSet(&stamp, &stat_buf, False);
   }

   /*
    * If requested, also check if any links broken.
    *
    * Again: it is important that we determine the kind of link
    * (valid, recursive, or broken) in exactly the same way that
    * ReadDirectoryProcess does it (see comment above)!
    */
   link_changed = False;
   if (rc == 0 && activity == activity_checking_links)
   {
      strcpy(full_name, directory->path_name);
      namep = full_name + strlen(full_name);
      if (namep[-1] != '/')
        *namep++ = '/';

      /* check all links: the ones of the last request, if this process
         was reused, else those it knows from the main process */
      if (sticky_link_count >= 0)
      {
         for (i = 0; i < sticky_link_count && !link_changed; i++)
            link_changed = LinkChanged(full_name, namep,
                                       sticky_links[i].name,
                                       sticky_links[i].kind);
      }
      else
      {
         for (file_data = directory->file_data;
              file_data && !link_changed;
              file_data = file_data->next)
         {
            /* Only worry about links */
            if (file_data->link == NULL || file_data->file_name == NULL)
               continue;
            link_changed = LinkChanged(full_name, namep,
                                       file_data->file_name,
                                       LinkKind(file_data));
         }
      }
   }

   /* send result back through the pipe */
   PipeBufAdd(&pb, &rc, sizeof(int));
   PipeBufAdd(&pb, &modify_time, sizeof(long));
   PipeBufAdd(&pb, &link_changed, sizeof(Boolean));
   PipeBufAdd(&pb, &stamp, sizeof(DirStamp));
   PipeBufFlush(pipe_fd, &pb);
   PipeBufFree(&pb);
   return 0;
}


/*--------------------------------------------------------------------
 * StickyProcIdle:
 *   Mark sticky background process as idle.  If there are too
 *   may idle sticky procs, cause some of them to exit.
 *------------------------------------------------------------------*/

static void
StickyProcIdle(
   ActivityStatus activity,
   StickyProcDesc *sticky_proc,
   int max_procs)
{
   StickyProcDesc *p, *next;
   int n;

   /* mark the process as idle */
   sticky_proc->idle = True;

   /*
    * If there are too many idle procs, make some of them go away;
    * also the ones that still type with an old database.
    */
   n = 0;
   for (p = ActivityTable[activity].sticky_procs; p; p = next)
   {
      next = p->next;
      if (!p->idle)
         continue;
      if (n < max_procs && p->generation == db_generation)
         n++;
      else
         StickyProcRemove(activity, p, True);
   }
}


/*--------------------------------------------------------------------
 * TimerPipeCallback
 *   Callback routine that reads information sent through the
 *   pipe from the TimerEventProcess background process.
 *------------------------------------------------------------------*/

static void
TimerPipeCallback(
   XtPointer client_data,
   int *fd,
   XtInputId *id)
{
   PipeCallbackData *pipe_data = (PipeCallbackData *)client_data;
   Directory *directory = pipe_data->directory;
   int rc;
   long modify_time = 0;
   Boolean link_changed = False;
   DirStamp stamp;
   Boolean ok;

   /* get return code from the pipe */
   rc = -1;
   DirStampSet(&stamp, NULL, False);
   ok = PipeRead(*fd, &rc, sizeof(int)) == sizeof(int) &&
        PipeRead(*fd, &modify_time, sizeof(long)) == sizeof(long) &&
        PipeRead(*fd, &link_changed, sizeof(Boolean)) == sizeof(Boolean) &&
        PipeRead(*fd, &stamp, sizeof(DirStamp)) == sizeof(DirStamp);
   if (!ok)
      rc = -1;

   /* close the pipe and cancel the callback */
   if (pipe_data->sticky_proc && ok)
      StickyProcIdle(pipe_data->activity, pipe_data->sticky_proc,
                     maxRereadProcsPerTick);
   else if (pipe_data->sticky_proc)
      StickyProcRemove(pipe_data->activity, pipe_data->sticky_proc, False);
   else
      close(*fd);
   XtFree( client_data );
   XtRemoveInput(*id);

   /* verify that the directory still exists */
   if (DirectoryGone(directory))
   {
      ScheduleActivity(NULL);
      return;
   }

   DPRINTF2((
     "TimerPipeCallback: rc %d (was %d), time %ld (was %ld), link change %d\n",
     rc, directory->errnum, modify_time, (long)directory->modify_time, link_changed));

   /* reset the busy flag and schedule new work, if any */
   directory->busy[directory->activity] = False;
   directory->activity = activity_idle;
   ScheduleActivity(directory);
   if (directory->ev_pending)
      EventTimerArm(EVENT_DELAY_MS);

   /* the check failed (its process died): try again next time */
   if (!ok)
      return;

   /* if directory-read already in progress, nothing more to do here */
   if (directory->busy[activity_reading] ||
       directory->busy[activity_update_all])
      return;

   /* skip this directory if it is no longer being viewed */
   if (SkipRefresh(directory))
      return;

   /* if the directory was modified or links changed, re-read it */
   if (rc == 0)
   {
      if (link_changed)
      {
         /* (an update re-types the links whose target changed) */
         DPRINTF(("TimerPipeCallback: %s link changed\n",
                  directory->directory_name));
         directory->busy[activity_update_all] = True;
         ScheduleActivity(directory);
      }
      else if (!TimespecEqual(&stamp.mtim, &directory->mtim) ||
               !TimespecEqual(&stamp.ctim, &directory->ctim) ||
               directory->errnum != 0)
      {
         DPRINTF(("TimerPipeCallback: %s modified\n",
                  directory->directory_name));
         directory->busy[activity_update_all] = True;
         ScheduleActivity(directory);
      }
   }
   else
   {
      if (directory->errnum == 0)
      {
         directory->errnum = rc;
         directory->errmsg_needed = True;
         ReadDirectoryFiles(NULL, directory);
      }
   }
}


/*--------------------------------------------------------------------
 * CheckDesktopProcess
 *   Main routine for the background process that checks each desktop
 *   objects to see if the file that it refers to has disappeared
 *   or has changed type.
 *------------------------------------------------------------------*/

static int
CheckDesktopProcess(
        int pipe_fd,
	Directory *directory,
	ActivityStatus activity)
{
   int i, n, count;
   DesktopState *objects, *obj;
   char *full_path;
   Tt_status tt_status;
   struct stat stat_buf;
   FileData2 file_data2;
   FileData *new_data;
   Boolean IsToolBox;
   PipeBuf pb = { NULL, 0, 0 };

   /*
    * The desktop objects: those of the last request, if this process
    * was reused, else the ones it knows from the main process.
    */
   if (sticky_desktop_count >= 0)
   {
      objects = sticky_desktop;
      count = sticky_desktop_count;
   }
   else
   {
      count = desktop_data->numIconsUsed;
      objects = (DesktopState *) XtMalloc((count + 1) * sizeof(DesktopState));
      for (i = 0; i < count; i++)
      {
         DesktopRec *desktopWindow = desktop_data->desktopWindows[i];
         FileData *old_data = desktopWindow->file_view_data->file_data;

         objects[i].host = desktopWindow->host;
         objects[i].dir_linked_to = desktopWindow->dir_linked_to;
         objects[i].file_name = desktopWindow->file_name;
         objects[i].physical_type = old_data->physical_type;
         objects[i].logical_type = old_data->logical_type;
      }
   }

   if(directory->directoryView && directory->directoryView->file_mgr_data)
       IsToolBox = directory->directoryView->file_mgr_data->toolbox;
   else
       IsToolBox = False;

   for (i = 0; i < count; i++)
   {
      obj = &objects[i];

      full_path = ResolveLocalPathName( obj->host,
                                        obj->dir_linked_to,
                                        obj->file_name,
                                        home_host_name, &tt_status);
      if (full_path == NULL)
         continue;

      /* Check if the file still exists */
      errno = 0;
      if (lstat(full_path, &stat_buf) < 0)
      {
         /* the real file no longer exists */
         DPRINTF2((
           "CheckDesktopProcess: sending PIPEMSG_DESKTOP_REMOVED for %s\n",
           full_path));
         PipeBufAddMsg(&pb, PIPEMSG_DESKTOP_REMOVED);
         PipeBufAddString(&pb, obj->host);
         PipeBufAddString(&pb, obj->dir_linked_to);
         PipeBufAddString(&pb, obj->file_name);
         PipeBufFlush(pipe_fd, &pb);
      }
      else
      {
         /* See if the type has changed */
         ReadFileData2(&file_data2, full_path, NULL,IsToolBox);
         new_data = FileData2toFileData(&file_data2, &n);

         if (new_data->physical_type != obj->physical_type ||
             !StringsEqual(new_data->logical_type, obj->logical_type))
         {
            /* the type has changed */
            DPRINTF2((
              "CheckDesktopProcess: sending PIPEMSG_DESKTOP_CHANGED for %s\n",
              full_path));
            DPRINTF2((
              "  old type %d %s, new type %d %s\n",
              obj->physical_type, obj->logical_type,
              new_data->physical_type, new_data->logical_type));

            PipeBufAddMsg(&pb, PIPEMSG_DESKTOP_CHANGED);
            PipeBufAddString(&pb, obj->host);
            PipeBufAddString(&pb, obj->dir_linked_to);
            PipeBufAddString(&pb, obj->file_name);
            PipeBufFlush(pipe_fd, &pb);

            PipeWriteFileData(pipe_fd, new_data);
         }

         FreeFileData(new_data, True);
      }
      XtFree(full_path);
      full_path = NULL;
   }

   if (objects != sticky_desktop)
      XtFree((char *)objects);

   /* send a 'done' msg through the pipe */
   DPRINTF2(("CheckDesktopProcess: sending DONE\n"));
   PipeBufAddMsg(&pb, PIPEMSG_DONE);
   PipeBufFlush(pipe_fd, &pb);
   PipeBufFree(&pb);
   return 0;
}


/*--------------------------------------------------------------------
 * CheckDesktopPipeCallback
 *   Callback routine that reads information sent through the
 *   pipe from the CheckDesktopProcess background process.
 *------------------------------------------------------------------*/

static void
CheckDesktopPipeCallback(
   XtPointer client_data,
   int *fd,
   XtInputId *id)
{
   PipeCallbackData *pipe_data = (PipeCallbackData *)client_data;
   Directory *directory = pipe_data->directory;
   short msg;
   char *host, *dir_linked_to, *file_name;
   FileData *new_data, *old_data;
   Boolean found;
   DesktopRec *desktopWindow;
   int i;

   /* read the next msg from the pipe */
   msg = -1;
   if (PipeRead(*fd, &msg, sizeof(short)) != sizeof(short))
      msg = -1;

   if (msg == PIPEMSG_DESKTOP_REMOVED ||
       msg == PIPEMSG_DESKTOP_CHANGED)
   {
      /* get information from pipe */
      host = PipeReadStringNonNull(*fd);
      dir_linked_to = PipeReadStringNonNull(*fd);
      file_name = PipeReadStringNonNull(*fd);
      if (msg == PIPEMSG_DESKTOP_CHANGED)
         new_data = PipeReadFileData(*fd);
      else
         new_data = NULL;

      DPRINTF2((
        "CheckDesktopPipeCallback: msg %d: host %s, dir %s, name %s\n",
        msg, host, dir_linked_to, file_name));


      /* find the desktop object */
      found = False;
      for (i = 0; i < desktop_data->numIconsUsed; i++)
      {
         desktopWindow = desktop_data->desktopWindows[i];

         if (strcmp(host, desktopWindow->host) == 0 &&
             strcmp(dir_linked_to, desktopWindow->dir_linked_to) == 0 &&
             strcmp(file_name, desktopWindow->file_name) == 0)
         {
            found = True;
            break;
         }
      }

      /* remove or update the desktop object, if found */
      if (! found)
      {
        /* nothing to do */
      }
      else if (msg == PIPEMSG_DESKTOP_REMOVED)
      {
         /* remove the desktop object */
         DesktopObjectRemoved(desktopWindow);
      }
      else /* msg == PIPEMSG_DESKTOP_CHANGED */
      {
         /* replace file data */
         old_data = desktopWindow->file_view_data->file_data;
         FreeFileData(old_data, False);
         memcpy(old_data, new_data, sizeof(FileData));
         XtFree((char *)new_data);
         new_data = NULL;

         /* update the desktop object */
         DesktopObjectChanged(desktopWindow);
      }

      /* free storage */
      XtFree(host);
      XtFree(dir_linked_to);
      XtFree(file_name);
      if (new_data)
        FreeFileData(new_data, True);
   }

   else
   {
      /* done, or the process died: close the pipe and cancel the callback */
      if (pipe_data->sticky_proc && msg == PIPEMSG_DONE)
         StickyProcIdle(pipe_data->activity, pipe_data->sticky_proc, 1);
      else if (pipe_data->sticky_proc)
         StickyProcRemove(pipe_data->activity, pipe_data->sticky_proc, False);
      else
         close(*fd);
      XtFree( client_data );
      XtRemoveInput(*id);

      /* reset the busy flag and schedule new work, if any */
      directory->busy[directory->activity] = False;
      directory->activity = activity_idle;
      ScheduleActivity(directory);
   }
}


/*--------------------------------------------------------------------
 *
 * CheckDesktop
 *   Arrange for a CheckDesktopProcess background process to be
 *   started (checks each desktop objects to see if the file that
 *   it refers to has disappeared or has changed type).
 *
 *------------------------------------------------------------------*/

void
CheckDesktop( void )
{
  dummy_directory->busy[activity_checking_desktop] = True;
  ScheduleActivity(dummy_directory);
}


/*--------------------------------------------------------------------
 * TimerEvent
 *   This routine is called periodically.  It schedules a
 *   TimerEventProcess background process to be started for every
 *   directory in the cache.
 *------------------------------------------------------------------*/

/* comparison routine for qsort */
static int
CheckListCmp(
        int *p1,
        int *p2 )
{
   return directory_set[*p1]->last_check - directory_set[*p2]->last_check;
}


static void
TimerEvent(
        XtPointer client_data,
        XtIntervalId *id )
{
   static int *check_list = NULL;
   static int check_alloc = 0;
   int i, j, n;
   long now, next;
   Boolean need_poll, need_links, links_left;
   Directory *directory;


   DPRINTF2(("Directory::TimerEvent\n"));

   poll_timer = 0;      /* (this one has fired) */

   if (dragActive)
   {
      /*
       * Don't change any directories while a drag is active.
       *
       * Reason: drag callbacks are called with a pointer to a FileViewData
       * structure; if a directory is reread while a drag is active,
       * the pointer would become invalid, causing unpredictable behavior.
       *
       * Schedule the next TimerEvent in 1/2 second, so that check will
       * be done soon after the drag is finished.
       */
      PollTimerArm(500);
      return;
   }

   /* update tick count */
   tick_count++;
   now = MonotonicMs();

   /*
    * Determine if we should also check for broken links this time
    * (every checkBrokenLink seconds; half a tick early is on time).
    * Only directories that have links need the check.
    */
   if (checkBrokenLink > 0 &&
       now - lastLinkCheckMs >= checkBrokenLink * 1000L - tickTime * 500L)
   {
     /* set link_check_needed flag on all directores */
     for (i = 0; i < directory_count; i++)
     {
        directory = directory_set[i];

        /* skip this directory if no view is mapped */
        if (SkipRefresh(directory) || !directory->has_links)
           continue;

        /* if a check is already in progress, don't start another one */
        if (directory->busy[activity_checking_links])
           continue;

        /* arrange for background process to be scheduled */
        directory->link_check_needed = True;
     }

     lastLinkCheckMs = now;
   }

   /* make sure check_list array is big enough */
   if (directory_count > check_alloc)
   {
      check_alloc = directory_count + 5;
      check_list =
         (int *)XtRealloc((char *)check_list, check_alloc*sizeof(int));
   }

   /*
    * Get a list of all directories that need to be checked: the ones
    * that are not watched (see DirectoryWatch), and the ones that are
    * due for a link check.
    */
   n = 0;
   need_poll = need_links = False;
   for (i = 0; i < directory_count; i++)
   {
      directory = directory_set[i];

      /* skip this directory if no view is mapped */
      if (SkipRefresh(directory))
         continue;

      if (directory->wd <= 0)
         need_poll = True;
      if (directory->has_links)
         need_links = True;

      /* if a stat is already in progress, don't start another one */
      if (directory->busy[activity_checking_dir] ||
          directory->busy[activity_checking_links])
         continue;

      /* a watched directory reports its changes itself */
      if (directory->wd > 0 && !directory->link_check_needed)
         continue;

      /* add this directory to the check list */
      check_list[n++] = i;
   }

   /*
    * Next we want to schedule a background process to be started
    * for each directory in the check_list.  However, the variable
    * maxRereadProcsPerTick puts a limit on the number of such
    * background processes started per clock tick (i.e., per call
    * to this routine).  Hence we sort check_list by last_check
    * (records the tick count when a directory was last read or
    * checked) and schedule backround processes on those dirs that
    * haven't been checked in the longest time.
    */
   qsort(check_list, n, sizeof(int), (int (*)())CheckListCmp);

   /* arrange for background process to be started */
   links_left = False;
   for (j = 0; j < n; j++)
   {
      i = check_list[j];
      if (j >= maxRereadProcsPerTick)
      {
         if (directory_set[i]->link_check_needed)
            links_left = True;
         continue;
      }
      if (directory_set[i]->link_check_needed)
      {
         directory_set[i]->link_check_needed = False;
         directory_set[i]->busy[activity_checking_links] = True;
      }
      else
         directory_set[i]->busy[activity_checking_dir] = True;
      ScheduleActivity(directory_set[i]);
      directory_set[i]->last_check = tick_count;
   }

   /*
    * Reset the timeout for the next interval: every tick while some
    * directory is polled, else in time for the next link check.  With
    * nothing to do, the timer stays off until RestartTimer.
    */
   if (!SomeWindowMapped())
   {
      timer_suspended = True;
      return;
   }
   if (need_poll || links_left)
      next = tickTime * 1000L;
   else if (need_links && checkBrokenLink > 0)
   {
      next = lastLinkCheckMs + checkBrokenLink * 1000L - now;
      if (next < tickTime * 1000L)
         next = tickTime * 1000L;
   }
   else
   {
      timer_suspended = True;
      return;
   }
   PollTimerArm(next);
}


/*--------------------------------------------------------------------
 * TimerEventBrokenLinks
 *   This routine is called periodically.  It checks whether any
 *   desktop object is broken (i.e., the object it refers to no
 *   longer exists.
 *------------------------------------------------------------------*/

void
TimerEventBrokenLinks(
        XtPointer client_data,
        XtIntervalId *id )
{

   DPRINTF2(("Directory::TimerEventBrokenLinks\n"));

   if (!dragActive)
   {
      /* go check the desktop objects */
      if (desktop_data->numIconsUsed > 0)
         CheckDesktop();
   }

   /*  Reset the timeout for the next interval.  */
   if (desktop_data->numIconsUsed > 0)
   {
     checkBrokenLinkTimerId = XtAppAddTimeOut( app_context,
                                               checkBrokenLink * 1000,
                                               TimerEventBrokenLinks,
                                               NULL );
   }
   else
   {
     checkBrokenLinkTimerId = None;
   }
}


/*====================================================================
 *
 * Background process scheduler
 *
 *   The routines below schedule background activity, making sure
 *   that there aren't too many processes running at the same time.
 *
 *==================================================================*/

/*--------------------------------------------------------------------
 *  ScheduleDirectoryActivity
 *    If there is any work to do for a directory, and if there is
 *    no backgroud process currently running for that directory,
 *    then fork a process to do the work.
 *------------------------------------------------------------------*/

static void
ScheduleDirectoryActivity(
   Directory *directory)
{
   static char *pname = "ScheduleActivity";
   ActivityStatus activity;
   PipeCallbackData *pipe_data;
   Boolean all_views_active;
   Boolean this_view_active;
   int i, j, k;
   int n_active, n_checking;
   int save_last_check = 0;
   FileMgrData *file_mgr_data;
   Boolean sticky;
   StickyProcDesc *p;
   int pipe_s2m_fd[2] = {-1, -1};  /* for msgs from backgroundnd proc (slave to master) */
   int pipe_m2s_fd[2] = {-1, -1};  /* for msgs to backgroundnd proc (master to slave) */
   pid_t pid = 0;
   char *s;
   int rc;

   /* If already active, don't start anything new. */
   if (directory->activity != activity_idle)
      return;

   /* Decide what to do next */
   for (activity = 0; activity < activity_idle; activity++)
      if (directory->busy[activity])
         break;

   /* If nothing to do, return */
   if (activity == activity_idle)
      return;

   DPRINTF2(("ScheduleActivity: activity %d, busy %c%c%c%c%c%c%c, dir %s\n",
             directory->activity,
             directory->busy[activity_writing_posinfo]? 'W': '-',
             directory->busy[activity_reading]?         'R': '-',
             directory->busy[activity_update_all]?      'A': '-',
             directory->busy[activity_update_some]?     'U': '-',
             directory->busy[activity_checking_links]?  'B': '-',
             directory->busy[activity_checking_desktop]? 'D': '-',
             directory->busy[activity_checking_dir]?    'C': '-',
             directory->directory_name));

   /* Don't start more than a certain number of background processed */
   n_active = 0;
   n_checking = 0;
   for (j = 0; j < directory_count; j++)
   {
      if (directory_set[j]->activity != activity_idle)
         n_active++;
      if (directory_set[j]->activity == activity_checking_links ||
          directory_set[j]->activity == activity_checking_dir)
         n_checking++;
   }
   if (dummy_directory->activity != activity_idle)
   {
      n_active++;
      n_checking++;
   }
   if (n_active >= maxDirectoryProcesses ||
       n_checking >= maxRereadProcesses)
   {
      DPRINTF2(("ScheduleActivity:  too many processes\n"));
      return;
   }

   /*
    * We don't want to start more than one background process per view.
    * In tree mode one view may show more than one directory.
    * Hence we go through the view list for this directory and for each
    * view, we check if the same view appears on the view list of some
    * other directory that currently has active background activity.
    * If all vies on this directory have other activity, then we won't
    * start anything new.
    */
   if (directory->numOfViews > 0)
   {
     all_views_active = True;
     for (i = 0; i < directory->numOfViews; i++)
     {
      /* get file_mgr_data for this view */
       file_mgr_data = directory->directoryView[i].file_mgr_data;

      /* see if the same view appears in the view list of a non-idle dir */
       this_view_active = False;
       for (j = 0; j < directory_count && !this_view_active; j++)
       {
         /* we are only interested in directories that are not idle */
         if (directory_set[j]->activity == activity_idle)
           continue;

         /* see if the view appears in the view list */
         for (k = 0; k < directory_set[j]->numOfViews; k++)
         {
           if (directory_set[j]->directoryView[k].file_mgr_data ==
               file_mgr_data)
           {
             this_view_active = True;
             break;
           }
         }
       }

       if (!this_view_active)
       {
         all_views_active = False;
         break;
       }
     }

     if (all_views_active)
     {
       DPRINTF2(("ScheduleActivity:  all views busy\n"));
       return;
     }
   }

   /*
    * A read or update sees every change made before it starts: take in
    * the queued change events, and clear the directory's mark.
    */
   if (activity == activity_reading || activity == activity_update_all)
   {
      DirectoryEventsRead();
      directory->ev_pending = False;
      directory->stale = False;
   }
   if (activity == activity_reading || activity == activity_update_all ||
       activity == activity_update_some)
      directory->update_changed = False;

   /* now we are ready to start the next activity */
   directory->activity = activity;
   if (activity == activity_reading ||
       activity == activity_update_all ||
       activity == activity_checking_dir ||
       activity == activity_checking_links)
   {
      save_last_check = directory->last_check;
      directory->last_check = tick_count;
   }

   /*
    * Special optimization for periodic background processes
    * (the directory, link and desktop checks):
    * Since this is done frequently, we don't want to fork new process each
    * time.  Hence, instead of exiting when it's done, the background process
    * is "sticky", i.e., it will stay around waiting for a message on stdin,
    * so it can be re-used the next time around.  A linked list of sticky
    * procs that are currently active is maintained in the ActivityTable.
    */
   sticky = ActivityTable[activity].sticky;
   if (sticky)
   {
      /* see if we can find an idle sticky proc that can do the work
         (one that types with the current database) */
      StickyProcsRetire();
      for (p = ActivityTable[activity].sticky_procs; p; p = p->next)
         if (p->idle)
            break;
   }
   else
      p = NULL;

   if (p)
   {
      PipeBuf pb = { NULL, 0, 0 };
      void (*oldPipe)(int);

      /* We found an idle sticky proc that can be used */
      DPRINTF2(("ScheduleActivity:  use sticky proc %ld\n", (long)p->child));

      /* Send the directory name (and data) to the sticky proc */
      StickyAddRequest(&pb, directory, activity);
      oldPipe = signal(SIGPIPE, SIG_IGN);
      rc = PipeBufFlush(p->pipe_m2s_fd, &pb);
      signal(SIGPIPE, oldPipe);
      PipeBufFree(&pb);

      if (rc < 0)
      {
        /* the pipe is broken, remove the old proc then start a new one */
        StickyProcRemove(activity, p, False);
        p = NULL;
      }
      else
      {
        pipe_s2m_fd[0] = p->pipe_s2m_fd;
        pid = p->child;
        p->idle = False;
      }
   }


   if (!p && activity == activity_writing_posinfo &&
       StartPosInfoThread(directory, &pipe_s2m_fd[0]) == 0)
   {
      /* (no process: pid stays 0) */
   }
   else if (!p)
   {
      /* Need to fork a new background process */

      /* create a pipe for reading data from the background proc */
      pipe(pipe_s2m_fd);

      /* creating a new sticky proc? */
      if (sticky)
      {
         /* also need a pipe for sending msgs to the sticky proc */
         pipe(pipe_m2s_fd);

         /* add entry to the list of sticky procs */
         p = (StickyProcDesc *) XtMalloc(sizeof(StickyProcDesc));
         p->next = ActivityTable[activity].sticky_procs;
         ActivityTable[activity].sticky_procs = p;

         p->pipe_s2m_fd = pipe_s2m_fd[0];
         p->pipe_m2s_fd = pipe_m2s_fd[1];
         p->idle = False;
         p->generation = db_generation;
      }

      /* fork a background process */
      pid = fork();

      if (pid == -1)
      {
          DBGFORK(("%s:  fork failed for activity %d: %s\n",
		    pname, activity, strerror(errno)));

          fprintf(stderr,
		"%s:  fork failed, ppid %d, pid %d, activity %d: error %d=%s\n",
		pname, getppid(), getpid(), activity, errno, strerror(errno));

	  directory->activity = activity_idle;
          directory->last_check = save_last_check;

          /* close unused pipe connections */
          close(pipe_s2m_fd[0]);    /* child won't read from this pipe */
          close(pipe_s2m_fd[1]);    /* parent won't write to this pipe */
          if (sticky)
	  {
             close(pipe_m2s_fd[1]); /* child won't write to this pipe */
             close(pipe_m2s_fd[0]); /* parent won't read from this pipe */
             /* forget the proc that was never started (it is first) */
             ActivityTable[activity].sticky_procs = p->next;
             XtFree((char *)p);
	  }
	  return;
      }

      if (pid == 0)
      {
         /* child process */
         pid = getpid();
         DBGFORK(("%s:  child activity %d, s2m %d, m2s %d\n",
		  pname, activity, pipe_s2m_fd[1], pipe_m2s_fd[0]));

         /* close unused pipe connections */
         close(pipe_s2m_fd[0]);    /* child won't read from this pipe */
         if (sticky)
            close(pipe_m2s_fd[1]); /* child won't write to this pipe */
         if (inotify_fd >= 0)
            close(inotify_fd);

         /* run main routine for this activity from ActivityTable */
         for (;;)
         {
            rc = (*ActivityTable[activity].main)(pipe_s2m_fd[1],
                                                 directory, activity);
            if (!sticky || rc != 0)
               break;

            /* wait for a message in the pipe */
            s = PipeReadString(pipe_m2s_fd[0]);
            if (s == NULL)
               break;

            if (directory == dummy_directory)
               XtFree(s);         /* (its names are not allocated) */
            else
            {
               XtFree(directory->path_name);
               directory->path_name = s;
            }
            if (StickyReadRequest(pipe_m2s_fd[0], activity) != 0)
               break;

            DPRINTF2(("StickyActivity:  activity %d, dir %s\n", activity,
                      directory->path_name));
         }

         /* close pipes and end this process */
         close(pipe_s2m_fd[1]);
         if (sticky)
            close(pipe_m2s_fd[0]);

         DBGFORK(("%s:  completed activity %d, (rc %d)\n",pname, activity, rc));

         exit(rc);
      }

      DBGFORK(("%s:  forked child<%d> for activity %d, s2m %d, m2s %d\n",
		  pname, pid, activity, pipe_s2m_fd[0], pipe_m2s_fd[1]));

      /* parent process */
      if (sticky)
         p->child = pid;

      /*
       * If a directory read or update was started:
       * clear the modifile_list, now that the
       * background process has it's own copy.
       */
      if (activity == activity_reading ||
          activity == activity_update_all ||
          activity == activity_update_some)
      {
         FreeModifiedList(directory);
      }

      /* close unused pipe connections */
      close(pipe_s2m_fd[1]);    /* parent won't write to this pipe */
      if (sticky)
         close(pipe_m2s_fd[0]); /* parent won't read from this pipe */

   }

   /* set up callback to get the pipe data */
   DPRINTF2(("ScheduleActivity:  setting up pipe callback\n"));
   pipe_data = (PipeCallbackData *)XtMalloc(sizeof(PipeCallbackData));
   pipe_data->directory = directory;
   pipe_data->child = pid;
   pipe_data->sticky_proc = p;
   pipe_data->activity = activity;

   XtAppAddInput(XtWidgetToApplicationContext(toplevel),
                 pipe_s2m_fd[0], (XtPointer)XtInputReadMask,
                 ActivityTable[activity].callback, (XtPointer)pipe_data);
}


/*--------------------------------------------------------------------
 *  ScheduleActivity
 *    See if any new background work should be started.
 *------------------------------------------------------------------*/

static void
ScheduleActivity(
   Directory *directory)
{
   int i;

   /* first try to schedule new activity for this directory */
   if (directory != NULL)
     ScheduleDirectoryActivity(directory);

   /* see if there is anything else we can schedule now */
   if (directory == NULL || directory->activity == activity_idle)
   {
      for (i = 0; i < directory_count; i++)
         if (directory_set[i] != directory)
            ScheduleDirectoryActivity(directory_set[i]);
   }
   ScheduleDirectoryActivity(dummy_directory);
 }

static void
SelectDesktopFile(
FileMgrData *file_mgr_data)
{
    DirectorySet *directory_data;
    FileViewData *file_view_data = NULL;
    int j;

    directory_data = file_mgr_data->directory_set[0];

    for (j = 0; j < directory_data->file_count; j++)
    {
        file_view_data = directory_data->file_view_data[j];

        if (file_view_data->filtered != True &&
            strcmp(file_mgr_data->desktop_file,
                   file_view_data->file_data->file_name) == 0)
        {
            SelectFile (file_mgr_data, file_view_data);
            break;
        }
    }
    ActivateSingleSelect(file_mgr_data->file_mgr_rec,
                         file_mgr_data->selection_list[0]->file_data->logical_type);
    if(file_view_data) {
        PositionFileView(file_view_data, file_mgr_data);
    }
}
