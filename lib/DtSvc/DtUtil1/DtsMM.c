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
 *	$TOG: DtsMM.c /main/16 1998/10/23 13:48:28 mgreess $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *	(c) Copyright 1993,1994,1995 Sun Microsystems, Inc. 
 *		All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define SUN_DB
#ifdef	SUN_DB
#include <sys/utsname.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/param.h>
#endif
#include <string.h>
#define X_INCLUDE_DIRENT_H
#define XOS_USE_XT_LOCKING
#include <X11/Xos_r.h>
#include <Dt/DbReader.h>
#include "Dt/DtsMM.h"
#include "Dt/DtNlUtils.h"
#include <Dt/UserMsg.h>
#include "DtSvcLock.h"

extern char *strdup(const char *);
static int MMValidateDb(DtDirPaths *dirs);
static int _debug_print_name(char *name, char *label);

typedef	int	(*genfunc)(const void *, const void *);

static	DtDtsMMDatabase	*db_list;
static	caddr_t		mmaped_db = 0;
static	size_t		mmaped_size = 0;
static	int		mmaped_fd = 0;
static	DtDtsMMHeader	*head = 0;
static	unsigned int	mm_generation = 1;

int _DtDtsMMUnLoad(void);

extern void _DtDbFillVariables (char **line );
extern void _DtDtsClear(void);

/*
 * Returns the base of the mapped database, mapping it first if needed.
 * Once the database is mapped this is a plain load: the process lock
 * is only needed to serialise the first mapping.  (Taking it on every
 * address computation did not protect anything anyway, since the
 * address is used after the lock is released.)
 */
static caddr_t
mm_base(void)
{
	caddr_t	db = __atomic_load_n(&mmaped_db, __ATOMIC_ACQUIRE);

	if(!db)
	{
		_DtSvcProcessLock();
		if(!mmaped_db)
		{
			_DtDtsMMInit(0);
		}
		db = mmaped_db;
		_DtSvcProcessUnlock();
	}
	return(db);
}

unsigned int
_DtDtsMMGeneration(void)
{
	return(mm_generation);
}

void *
_DtDtsMMGetPtr(int index)
{
	DtShmIntList  int_list;

	int_list = (DtShmIntList)&mm_base()[sizeof(DtDtsMMHeader)];
	return((void *)&int_list[index]);
}

int
_DtDtsMMGetPtrSize(int index)
{
	DtShmIntList  int_list;

	int_list = (DtShmIntList)&mm_base()[sizeof(DtDtsMMHeader)];
	return(int_list[index-1]);
}

int *
_DtDtsMMGetDCNameIndex(int *size)
{
        int *result;

	mm_base();
	_DtSvcProcessLock();
	*size = _DtDtsMMGetPtrSize(head->name_list_offset);	
	result = (int*) _DtDtsMMGetPtr(head->name_list_offset);
	_DtSvcProcessUnlock();
	return(result);
}

int *
_DtDtsMMGetDbName(DtDtsMMDatabase *db, DtShmBoson boson)
{
	DtShmInttab	tab = (DtShmInttab)_DtDtsMMGetPtr(db->nameIndex);
	return((int *)_DtShmFindIntTabEntry(tab, boson));
}

int *
_DtDtsMMGetNoNameIndex(int *size)
{
        int *result;

	mm_base();
	_DtSvcProcessLock();

	if(head->no_name_offset == -1)
	{
		*size = 0;
		_DtSvcProcessUnlock();
		return(0);
	}
	*size = _DtDtsMMGetPtrSize(head->no_name_offset);	
	result = (int *) _DtDtsMMGetPtr(head->no_name_offset);
	_DtSvcProcessUnlock();
	return(result);
}

/* returns the pointer to buffer only name list */
int *
_DtDtsMMGetBufferIndex(int *size)
{
	int	*list = (int*)_DtDtsMMGetNoNameIndex(size);
	int	*bufferIndex;

	_DtSvcProcessLock();
	*size -= head->buffer_start_index;
	bufferIndex = &list[head->buffer_start_index];
	_DtSvcProcessUnlock();

	return(bufferIndex);
}

DtShmInttab
_DtDtsMMGetFileList(void)
{
	DtShmInttab file_index;

	_DtSvcProcessLock();
	file_index = (DtShmStrtab)_DtDtsMMGetPtr(head->files_offset);
	_DtSvcProcessUnlock();
	return(file_index);
}

const char *
_DtDtsMMBosonToString(DtShmBoson boson)
{
	DtShmStrtab str_table;

	if (boson == 0)
		return(0);

	mm_base();
	str_table = (DtShmStrtab)_DtDtsMMGetPtr(head->str_tbl_offset);

	return(_DtShmBosonToString(str_table, boson));
}

DtShmBoson
_DtDtsMMStringToBoson(const char *string)
{
	DtShmStrtab str_table;

	if ((string == (char *)NULL) || (*string == '\0'))
		return(-1);

	mm_base();
	str_table = (DtShmStrtab)_DtDtsMMGetPtr(head->str_tbl_offset);

	return(_DtShmStringToBoson(str_table, string));
}

void
_DtDtsMMPrintFld(int fld, DtDtsMMField *fld_ptr, FILE *fd_in)
{
	const	char	*tmp;
	const	char	*tmpv;
	FILE	*fd = fd_in;

	if(!fd) fd = stdout;

	tmp = _DtDtsMMBosonToString(fld_ptr->fieldName);
	tmpv = _DtDtsMMBosonToString(fld_ptr->fieldValue);
	fprintf(fd, "\t\t[%d]\t%s(%d)\t%s(%d)\n", fld, tmp,fld_ptr->fieldName,
		tmpv?tmpv:"(NULL)", fld_ptr->fieldValue);
}

void
_DtDtsMMPrintRec(int rec, DtDtsMMRecord	*rec_ptr, FILE *fd_in)
{
	int		fld;
	DtDtsMMField	*fld_ptr;
	DtDtsMMField	*fld_ptr_list;
	const	char	*tmp;
	FILE	*fd = fd_in;

	if(!fd) fd = stdout;

	tmp = _DtDtsMMBosonToString(rec_ptr->recordName);
	fprintf(fd, "\tRec[%d] name = %s(%d)\n\t%d Fields\n", rec,
		tmp, rec_ptr->recordName,
		rec_ptr->fieldCount);
	fld_ptr_list = _DtDtsMMGetPtr(rec_ptr->fieldList);
	for(fld = 0; fld < rec_ptr->fieldCount; fld++)
	{
		fld_ptr = &fld_ptr_list[fld];
		_DtDtsMMPrintFld(fld, fld_ptr, fd);
	}
}

void
_DtDtsMMPrintDb(int db, DtDtsMMDatabase *db_ptr, FILE *fd_in)
{
	int		rec;
	DtDtsMMRecord	*rec_ptr;
	DtDtsMMRecord	*rec_ptr_list;
	const	char	*tmp;
	FILE	*fd = fd_in;

	if(!fd) fd = stdout;

	fprintf(fd, "DB[%d] ", db);
	tmp =  _DtDtsMMBosonToString(db_ptr->databaseName);
	fprintf(fd, "name = %s(%d)\n", tmp, db_ptr->databaseName);
	fprintf(fd, "%d Records\n", db_ptr->recordCount);
	rec_ptr_list = _DtDtsMMGetPtr(db_ptr->recordList);
	for(rec = 0; rec < db_ptr->recordCount; rec++)
	{
		rec_ptr = &rec_ptr_list[rec];
		_DtDtsMMPrintRec(rec, rec_ptr, fd);
	}
}

void
_DtDtsMMPrint(FILE *org_fd)
{
	int		db;
	DtDtsMMDatabase	*db_ptr;
	FILE		*fd = org_fd;
	const	char	*tmp;

	_DtSvcProcessLock();
	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}

	for(db = 0; db < head->num_db; db++)
	{
		db_ptr = &db_list[db];
		if(fd == 0)
		{
			chdir("/tmp");
			tmp = _DtDtsMMBosonToString(db_ptr->databaseName);
			if((fd = fopen(tmp, "w")) == NULL)
			{
			    _DtSimpleError(
					DtProgName, DtError, NULL,
					(char*) tmp, NULL);
			    continue;
			}
		}
		_DtDtsMMPrintDb(db, db_ptr, fd);
		if(org_fd == 0)
		{
			fclose(fd);
			fd = 0;
		}
	}
	_DtSvcProcessUnlock();
}

int
_DtDtsMMCompareRecordNames(DtDtsMMRecord *a, DtDtsMMRecord *b)
{
	return (a->recordName - b->recordName);
}

int
_DtDtsMMCompareFieldNames(DtDtsMMField *a, DtDtsMMField *b)
{
	return (a->fieldName - b->fieldName);
}

#include <Dt/Dts.h>

/* Results of mm_map_file() */
enum
{
	MM_MAPPED,	/* mapped */
	MM_ABSENT,	/* no such file */
	MM_UNUSABLE,	/* ours, but not a cache file of this version */
	MM_FOREIGN	/* not ours, not a plain file, or unreadable */
};

/* Results of MMValidateDb() */
enum
{
	MM_VALID,
	MM_STALE,	/* built from database files that have changed */
	MM_OTHER_PATH	/* built for another database search path */
};

static int	mm_map_file(const char *CacheFile);

/*
 * The name of the cache file shared by the clients of a display
 * (see _DTDTSMMTEMPDIR), or NULL if DISPLAY is not set.
 */
static char *
shared_cache_name(void)
{
	char	*dsp = getenv("DISPLAY");
	char	*results;
	char	*c;
	size_t	len;

	if(!dsp || !*dsp)
	{
		return NULL;
	}
	len = strlen(_DTDTSMMTEMPDIR) + strlen(_DTDTSMMTEMPFILE) +
	      strlen(dsp) + 2;
	results = malloc(len);
	if(!results)
	{
		return NULL;
	}
	snprintf(results, len, "%s/%s%s", _DTDTSMMTEMPDIR,
		 _DTDTSMMTEMPFILE, dsp);
	/* Drop the screen number.  (A DISPLAY without ':' used to crash.) */
	c = strrchr(results, ':');
	if(c && (c = strchr(c, '.')))
	{
		*c = '\0';
	}
	return results;
}

/*
 * Maps the action/data type database, from the cache file shared by
 * the clients of the display if it is up to date, or else from a new
 * one.  With override, a new shared cache file is always built.
 *
 * A client that finds the shared cache missing, unusable (an older
 * format, say) or out of date rebuilds it under the shared name for the
 * clients that follow; it used to build a private copy, so every client
 * started re-read all the database files until dtdbcache rebuilt the
 * shared one.  The new file is renamed into place, so other clients see
 * either the old or the new one.  A client whose database search path
 * differs from the one the shared cache was built for (another LANG or
 * DTDATABASESEARCHPATH) does not replace it, but builds a private one,
 * as before; so does a client that cannot replace it (the file is not
 * its own), and root does not create one.
 */
int
_DtDtsMMInit(int override)
{
	DtDirPaths *dirs = _DtGetDatabaseDirPaths();
	char	*CacheFile = shared_cache_name();
	int	ok;

	if(override)
	{
		/* Without DISPLAY there is no shared name: build a private one. */
		ok = _DtDtsMMCreateDb(dirs, CacheFile, CacheFile ? override : 0);
		if (ok)
		{
			_debug_print_name(CacheFile ? CacheFile : "(private)",
					  "Init");
		}
	}
	else
	{
		int	replace = 0;
		int	status = MM_FOREIGN;

		if(CacheFile)
		{
			status = mm_map_file(CacheFile);
		}
		if(status == MM_MAPPED)
		{
			switch(MMValidateDb(dirs))
			{
			case MM_VALID:
				_debug_print_name(CacheFile, "Mapped");
				free(CacheFile);
				_DtFreeDatabaseDirPaths(dirs);
				return 1;
			case MM_STALE:
				replace = 1;
				break;
			default:
				break;
			}
		}
		else if(status == MM_UNUSABLE)
		{
			replace = 1;
		}
		else if(status == MM_ABSENT)
		{
			/*
			 * Create it, unless we are root: a root client
			 * (the login greeter, an application run with
			 * sudo) would leave a file the user's clients
			 * cannot replace.
			 */
			replace = (getuid() != 0);
		}
		if(geteuid() != getuid())
		{
			/* The file would not be owned by our real user. */
			replace = 0;
		}

		if(replace)
		{
			_debug_print_name(CacheFile, "Rebuilt");
			ok = _DtDtsMMCreateDb(dirs, CacheFile,
					      DTDTSMM_SHARED_OR_PRIVATE);
		}
		else
		{
			_debug_print_name("(private)", "Private");
			ok = _DtDtsMMCreateDb(dirs, NULL, 0);
		}
	}
	free(CacheFile);
	_DtFreeDatabaseDirPaths(dirs);
	return ok ? 1 : 0;
}

char **
_DtsMMListDb(void)
{
	int	i;
	char	**list;

	_DtSvcProcessLock();
	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}

	list = (char **)malloc((head->num_db+1)*sizeof(char *));
	for ( i = 0; i < head->num_db; i++ )
	{
		list[i] = (char *)_DtDtsMMBosonToString(db_list[i].databaseName);
	}
	list[i] = 0;
	_DtSvcProcessUnlock();
	return(list);
}


DtDtsMMDatabase *
_DtDtsMMGet(const char *name)
{
	int		i;
	DtShmBoson	boson = _DtDtsMMStringToBoson(name);
	DtDtsMMDatabase *ret_db;

	_DtSvcProcessLock();
	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}
	for(i = 0; i < head->num_db; i++)
	{
		if(db_list[i].databaseName == boson)
		{
			ret_db = &db_list[i];
		        _DtSvcProcessUnlock();

			return(ret_db);
		}
	}
	_DtSvcProcessUnlock();
	return(NULL);
}


DtDtsMMField *
_DtDtsMMGetField(DtDtsMMRecord *rec, const char *name)
{
	int i;
	DtDtsMMField	*fld_ptr;
	DtDtsMMField	*fld_ptr_list;

	/*
	 * Field names have been quarked so quark 'name' and
	 * do a linear search for the quark'ed field name.
	 */
	DtShmBoson	tmp = _DtDtsMMStringToBoson (name);

	fld_ptr_list = _DtDtsMMGetPtr(rec->fieldList);
	for (i = 0; i < rec->fieldCount; i++)
	{
		fld_ptr = &fld_ptr_list[i];
		if (fld_ptr->fieldName == tmp)
		{
			return (fld_ptr);
		}
	}
	return(NULL);
}

const char *
_DtDtsMMGetFieldByName(DtDtsMMRecord *rec, const char *name)
{
	DtDtsMMField	*result;

	result = _DtDtsMMGetField(rec, name);
	if(result)
	{
		return(_DtDtsMMBosonToString(result->fieldValue));
	}
	else
	{
		return(NULL);
	}

}

DtDtsMMRecord *
_DtDtsMMGetRecordByName(DtDtsMMDatabase *db, const char *name)
{
	DtShmBoson 	name_quark;
	DtDtsMMRecord	*rec_ptr_list;
	int		*idx;

	if (!db || (name_quark = _DtDtsMMStringToBoson(name)) == -1)
	{
		return NULL;
	}

	/*
	 * The cache builder (build_new_db() in MMDb.c) indexes every
	 * database by record name, pointing at the first record of each
	 * run of equal names.  The databases this is used for are sorted
	 * by name, so that is the first record of that name, which is
	 * what a linear search would find.
	 */
	idx = _DtDtsMMGetDbName(db, name_quark);
	if (!idx || *idx < 0 || *idx >= db->recordCount)
	{
		return NULL;
	}
	rec_ptr_list = _DtDtsMMGetPtr(db->recordList);
	if (rec_ptr_list[*idx].recordName != name_quark)
	{
		return NULL;
	}
	return (&rec_ptr_list[*idx]);
}
/*
 * The name of the shared cache file of the display, with override (and
 * DISPLAY set); otherwise a name for a private one (no longer used by
 * libDtSvc, whose private caches have no name).
 */
char *
_DtDtsMMCacheName(int override)
{
	char	*results = override ? shared_cache_name() : NULL;
	size_t	len;
	int	fd;

	if(!results)
	{
		/*
		 * A name not in use.  mkstemp() makes sure of that, and
		 * the file is removed again: the caller creates it.
		 * (tmpnam(), used before, is not safe.)
		 */
		len = strlen(_DTDTSMMTEMPDIR) + strlen(_DTDTSMMTEMPFILE) + 8;
		results = malloc(len);
		if(!results)
		{
			return NULL;
		}
		snprintf(results, len, "%s/%sXXXXXX", _DTDTSMMTEMPDIR,
			 _DTDTSMMTEMPFILE);
		if((fd = mkstemp(results)) != -1)
		{
			close(fd);
			unlink(results);
		}
	}
	return(results);
}


/*
 * Maps the cache file open as fd (and takes over fd: it is closed when
 * the database is unloaded, or now on failure), first unloading the
 * database mapped before.  The file must be a plain file, owned by our
 * real user unless we have just written it ourselves (written, it is
 * owned by the effective user: a setuid client's private cache), and a
 * cache file of this version.  Returns an MM_* status.
 */
static int
mm_map_fd(int fd, int written)
{
	struct	stat	buf;
	int	status = MM_FOREIGN;
	caddr_t	db;

	_DtSvcProcessLock();

	if (mmaped_db)
	{
		/*
		 * Already have a file memory-mapped.  Unload it.  (This
		 * tested mmaped_fd > 0, but the fd can be 0 when stdin is
		 * closed, and the old mapping was then leaked.)
		 */
		_DtDtsMMUnLoad();
	}

	if(fstat(fd, &buf) == 0 && S_ISREG(buf.st_mode) &&
	   (written || buf.st_uid == getuid()))
	{
		status = MM_UNUSABLE;
		if(buf.st_size < (off_t)sizeof(DtDtsMMHeader))
		{
			/* empty or truncated */
		}
		else if((db = (char *)mmap(NULL,
				buf.st_size,
				PROT_READ,
#if defined(sun)
				/* MAP_NORESERVE is only supported
				   on sun and novell platforms */
				MAP_SHARED|MAP_NORESERVE,
#else
				MAP_SHARED,
#endif
				fd,
				0)) == (void *) -1)
		{
			status = MM_FOREIGN;
			_DtSimpleError(DtProgName, DtError, NULL,
				       "mmap of dts_cache file", NULL);
		}
		else if(((DtDtsMMHeader *)db)->magic != DTDTSMM_MAGIC ||
			((DtDtsMMHeader *)db)->version != DTDTSMM_VERSION ||
			((DtDtsMMHeader *)db)->size != buf.st_size)
		{
			munmap(db, buf.st_size);
		}
		else
		{
			DtShmIntList	int_list;

			status = MM_MAPPED;
			mmaped_fd = fd;
			mmaped_size = buf.st_size;
			head = (DtDtsMMHeader *)db;
			int_list = (DtShmIntList)&db[sizeof(DtDtsMMHeader)];
			db_list = (DtDtsMMDatabase *)&int_list[head->db_offset];
			mm_generation++;
			/* Publish the mapping only once it is usable. */
			__atomic_store_n(&mmaped_db, db, __ATOMIC_RELEASE);
		}
	}
	if(status != MM_MAPPED)
	{
		close(fd);
		mmaped_db = 0;
	}
	_DtSvcProcessUnlock();
	return(status);
}

int
_DtDtsMMapFd(int fd)
{
	return(mm_map_fd(fd, 0) == MM_MAPPED);
}

/* Like _DtDtsMMapFd(), for a cache file we have just written. */
int
_DtDtsMMapNewFd(int fd)
{
	return(mm_map_fd(fd, 1) == MM_MAPPED);
}

static int
mm_map_file(const char *CacheFile)
{
	int	fd;

	/* The shared cache is a plain file; do not follow a symbolic link. */
	fd = open(CacheFile, O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
	if(fd == -1)
	{
		return(errno == ENOENT ? MM_ABSENT : MM_FOREIGN);
	}
	return(mm_map_fd(fd, 0));
}

int
_DtDtsMMapDB(const char *CacheFile)
{
	return(mm_map_file(CacheFile) == MM_MAPPED);
}

void
_DtDtsMMFillStamp(DtDtsMMStamp *stamp, const struct stat *st, int error)
{
	unsigned long long	t;

	memset(stamp, 0, sizeof(*stamp));
	if(!st)
	{
		stamp->error = error ? error : -1;
		return;
	}
	stamp->size = (int)st->st_size;
	t = (unsigned long long)st->st_mtime;
	stamp->mtime[0] = (int)(unsigned int)t;
	stamp->mtime[1] = (int)(unsigned int)(t >> 32);
	t = (unsigned long long)st->st_ctime;
	stamp->ctime[0] = (int)(unsigned int)t;
	stamp->ctime[1] = (int)(unsigned int)(t >> 32);
	/* POSIX 2008: struct timespec st_mtim, st_ctim. */
	stamp->mtime[2] = (int)st->st_mtim.tv_nsec;
	stamp->ctime[2] = (int)st->st_ctim.tv_nsec;
}

static int
stamp_matches(const DtDtsMMStamp *old, const struct stat *st, int error)
{
	DtDtsMMStamp	now;

	_DtDtsMMFillStamp(&now, st, error);
	now.path = old->path;
	now.is_dir = old->is_dir;
	if(old->error && now.error)
	{
		/* Still not there (or not readable): no change. */
		return 1;
	}
	return(memcmp(&now, old, sizeof(now)) == 0);
}

/*
 * Whether the mapped cache file was built from the database files as
 * they are now.  It must have been built for the same database search
 * path (else MM_OTHER_PATH); the directories it was built from must
 * still be the existing database directories, in the same order, and
 * none of them, or of the database files, may have changed since (else
 * MM_STALE).  Adding, removing or renaming a file changes its directory.
 * (This used to compare a sum of bytes of the path names, change into
 * each database directory and stat() every entry, and ignore errors.)
 */
static int
MMValidateDb(DtDirPaths *dirs)
{
	const DtDtsMMStamp	*stamp;
	char			*searchpath;
	const char		*path;
	int			count;
	int			i;
	int			d = 0;
	int			dfd = -1;
	int			result = MM_VALID;

	_DtSvcProcessLock();
	count = head->files_count;
	stamp = _DtDtsMMGetPtr(head->files_offset);

	searchpath = _DtDtsMMSearchPath();
	path = _DtDtsMMBosonToString(head->searchpath);
	if(!searchpath || !path || strcmp(searchpath, path) != 0)
	{
		XtFree(searchpath);
		_DtSvcProcessUnlock();
		return(MM_OTHER_PATH);
	}
	XtFree(searchpath);

	for(i = 0; i < count && result == MM_VALID; i++)
	{
		struct stat	buf;
		int		error = 0;

		if(!(path = _DtDtsMMBosonToString(stamp[i].path)))
		{
			result = MM_STALE;
			break;
		}
		if(stamp[i].is_dir)
		{
			/* A database directory, in search path order. */
			if(!dirs->paths[d] || strcmp(dirs->paths[d], path) != 0)
			{
				result = MM_STALE;
				break;
			}
			d++;
			if(dfd != -1)
			{
				close(dfd);
			}
			dfd = open(path, O_RDONLY|O_DIRECTORY|O_CLOEXEC);
			if(dfd == -1 || fstat(dfd, &buf) == -1)
			{
				error = errno;
			}
		}
		else
		{
			/* A database file of the last directory. */
			const char	*base = strrchr(path, '/');

			if(dfd != -1 && base
			   ? fstatat(dfd, base + 1, &buf, 0) == -1
			   : stat(path, &buf) == -1)
			{
				error = errno;
			}
		}
		if(!stamp_matches(&stamp[i], error ? NULL : &buf, error))
		{
			result = MM_STALE;
		}
	}
	if(dfd != -1)
	{
		close(dfd);
	}
	if(result == MM_VALID && dirs->paths[d])
	{
		/* A new database directory. */
		result = MM_STALE;
	}

	_DtSvcProcessUnlock();
	return(result);
}

/*
 * _DtDbFillVariables() only changes a value that holds a '$' (variable
 * reference) or a '\\' (escape, removed by clean_line()).  Neither byte
 * can be the trailing byte of a multibyte character that matters here:
 * '$' never is, and a '\\' trailing byte (e.g. in IBM-932) is found by the
 * same byte search, which only makes us take the slow path.
 */
#define	NEEDS_EXPANSION(v)	(strpbrk((v), "$\\") != NULL)

/* _DtDbFillVariables() assumes the buffer holds at least this many bytes. */
#define	FILL_VARIABLES_MIN	1024

char *
_DtDtsMMExpandValue(const char *value)
{
	char	*newval;
	size_t	len;

	if(!value)
	{
		return NULL;
	}
	if(!NEEDS_EXPANSION(value))
	{
		return(strdup(value));
	}
	len = strlen(value) + 1;
	newval = (char *)malloc(len < FILL_VARIABLES_MIN ?
				FILL_VARIABLES_MIN : len);
	memcpy(newval, value, len);
	_DtDbFillVariables(&newval);
	return(newval);
}

char *
_DtDtsMMExpandValueNoCopy(const char *value)
{
	if(value && !NEEDS_EXPANSION(value) && _DtDtsMMIsMemory(value))
	{
		return((char *)value);
	}
	return(_DtDtsMMExpandValue(value));
}

void
_DtDtsMMSafeFree(char *value)
{
	if(value && !_DtDtsMMIsMemory(value))
	{
		free(value);
	}
}

int
_DtDtsMMIsMemory(const char *value)
{
	int	result;

	_DtSvcProcessLock();
	result = mmaped_db != 0 && (caddr_t)value >= mmaped_db &&
		 (caddr_t)value < mmaped_db+mmaped_size;
	_DtSvcProcessUnlock();
	return(result);
}

int
_DtDtsMMUnLoad(void)
{
	int	error = 0;

	_DtSvcProcessLock();
	_DtDtsClear();
	if(mmaped_db == 0)
	{
	        _DtSvcProcessUnlock();
		return(error);
	}
	if(munmap(mmaped_db, mmaped_size) == -1)
	{
		_DtSimpleError(DtProgName, DtError, NULL,
			       "munmap of dts_cache file", NULL);
		error = -1;
	}
	if(close(mmaped_fd) == -1)
	{
		_DtSimpleError(DtProgName, DtError, NULL,
			       "close of dts_cache file", NULL);
	}

	db_list = 0;
	__atomic_store_n(&mmaped_db, 0, __ATOMIC_RELEASE);
	mmaped_size = 0;
	mmaped_fd = 0;
	head = 0;
	mm_generation++;
	_DtSvcProcessUnlock();
	return(error);
}

#include "Dt/UserMsg.h"

static int
_debug_print_name(char *name, char *label)
{
#ifdef DEBUG
	static char	*db = (char *)-1;

	_DtSvcProcessLock();
	if(db == (char *)-1)
	{
		db = getenv("MMAP_DEBUG");
	}
	_DtSvcProcessUnlock();

	if(db)
		_DtSimpleError(db,
			DtInformation,
			NULL,
			"%s - db name = %s\n", label,
			name);
#endif /* DEBUG */
	return(0);
}
