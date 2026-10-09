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
#include <libgen.h>
#define X_INCLUDE_DIRENT_H
#define XOS_USE_XT_LOCKING
#include <X11/Xos_r.h>
#include <Dt/DbReader.h>
#include "Dt/DtsMM.h"
#include "Dt/DtNlUtils.h"
#include <Dt/UserMsg.h>
#include "DtSvcLock.h"

extern char *strdup(const char *);
static int MMValidateDb(DtDirPaths *dirs, char *suffix);
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

int
_DtDtsMMInit(int override)
{
	DtDirPaths *dirs = _DtGetDatabaseDirPaths();
	char	*CacheFile = _DtDtsMMCacheName(1);
	if(override)
	{
		if (!_DtDtsMMCreateDb(dirs, CacheFile, override))
		{
			free(CacheFile);
			_DtFreeDatabaseDirPaths(dirs);
			return 0;
		}
		_debug_print_name(CacheFile, "Init");
	}
	else
	{
		int success = _DtDtsMMapDB(CacheFile);
		if(success)
		{
			if(!MMValidateDb(dirs, ".dt"))
			{
				success = 0;
			}
			else
			{
				_debug_print_name(CacheFile, "Mapped");
			}
		}
		if(!success)
		{
			free(CacheFile);
			CacheFile = _DtDtsMMCacheName(0);
			_debug_print_name(CacheFile, "Private");
			/* Check return status, and pass status to caller. */
			if (!_DtDtsMMCreateDb(dirs, CacheFile, override))
			{
				free(CacheFile);
				_DtFreeDatabaseDirPaths(dirs);
				return 0;
			}
		}
	}
	free(CacheFile);
	_DtFreeDatabaseDirPaths(dirs);
	return 1;
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
int
_DtDtsMMPathHash(DtDirPaths *dirs)
{
	int	pathhash = 0;
	DIR	*dirp;
	int	suffixLen;
	int	nameLen;
	char	*file_suffix;
	char	*suffix = ".dt";
	int	i;
	char	*cur_dir = getcwd(0,MAXPATHLEN);
	struct	stat	buf;

	_Xreaddirparams dirEntryBuf;
	struct dirent *result;
	(void) dirEntryBuf; /* unused unless XTHREADS */

	for(i = 0; dirs->paths[i] ; i++)
	{
		if(chdir(dirs->paths[i]) == -1)
		{
			continue;
		}
		dirp = opendir (".");
		while ((result = _XReaddir(dirp, dirEntryBuf)) != NULL)
		{
			if ((int)strlen (result->d_name) >= (int)strlen(suffix))
			{
				suffixLen = DtCharCount(suffix);
				nameLen = DtCharCount(result->d_name);
				file_suffix = (char *)_DtGetNthChar(result->d_name,
						nameLen - suffixLen);
				stat(result->d_name, &buf);
				if (file_suffix &&
					(strcmp(file_suffix, suffix) == 0) &&
					(buf.st_mode&S_IFREG))
				{
					char *c = dirs->paths[i];
					while(*c)
					{
						pathhash += (int)*c;
						c++;
					}
					break;
				}
			}
		}
		closedir(dirp);
	}
	chdir(cur_dir);
	free(cur_dir);
	return(pathhash);
}

char *
_DtDtsMMCacheName(int override)
{
	char	*dsp = getenv("DISPLAY");
	char	*results = 0;
	char	*c;

	if(override && dsp)
	{
		results = malloc(strlen(_DTDTSMMTEMPDIR)+
				 strlen(_DTDTSMMTEMPFILE)+
				strlen(dsp)+3);
		sprintf(results, "%s/%s%s",
				_DTDTSMMTEMPDIR,
				_DTDTSMMTEMPFILE,
				dsp);
		c = strchr(results, ':');
		c = strchr(c, '.');
		if(c)
		{
			*c = '\0';
		}
	}
	else
	{
	/* tempnam(3) is affected by the TMPDIR environment variable. */
	/* This creates problems for rename() if "tmpfile" and "cacheFile" */
	/* are on different file systems.  Use tmpnam(3) to create the */
	/* unique file name instead. */
		char tmpnam_buf[L_tmpnam + 1];

		results = (char *)malloc(strlen(_DTDTSMMTEMPDIR) +
					 strlen(_DTDTSMMTEMPFILE) +
					 L_tmpnam + 3);
		tmpnam(tmpnam_buf);
		sprintf(results, "%s/%s%s", _DTDTSMMTEMPDIR, _DTDTSMMTEMPFILE,
			basename(tmpnam_buf));
	}
	return(results);
}


int
_DtDtsMMapDB(const char *CacheFile)
{
	struct	stat	buf;
	int	success = FALSE;

	_DtSvcProcessLock();

	if (mmaped_fd > 0)
	{
		/* Already have a file memory-mapped.  Unload it. */
		_DtDtsMMUnLoad();
	}

	mmaped_fd  = open(CacheFile, O_RDONLY|O_CLOEXEC, 0400);
	if(mmaped_fd !=  -1)
	{
		if(fstat(mmaped_fd, &buf) == 0 && buf.st_uid == getuid())
		{
			caddr_t	db = (char *)mmap(NULL,
					buf.st_size,
					PROT_READ,
#if defined(sun)
					/* MAP_NORESERVE is only supported
					   on sun and novell platforms */
					MAP_SHARED|MAP_NORESERVE,
#else
					MAP_SHARED,
#endif
					mmaped_fd,
					0);
			if(db != (void *) -1)
			{
				DtShmIntList	int_list;

				success = TRUE;
				mmaped_size = buf.st_size;
				head = (DtDtsMMHeader *)db;
				int_list = (DtShmIntList)&db[sizeof(DtDtsMMHeader)];
				db_list = (DtDtsMMDatabase *)&int_list[head->db_offset];
				mm_generation++;
				/* Publish the mapping only once it is usable. */
				__atomic_store_n(&mmaped_db, db, __ATOMIC_RELEASE);
			}
			else
			{
			    _DtSimpleError(
					DtProgName, DtError, NULL,
					(char*) CacheFile, NULL);
			}
		}
	}
	if(!success)
	{
		mmaped_db = 0;
	}
	_DtSvcProcessUnlock();
	return(success);
}

static int
MMValidateDb(DtDirPaths *dirs, char *suffix)
{
	struct stat		buf;
	DtShmBoson		*boson_list = 0;
	time_t			*mtime_list;
	int			count = 0;
	int			i;
	const char		*file;
	int			pathhash = _DtDtsMMPathHash(dirs);

	_DtSvcProcessLock();
	if(head->pathhash != pathhash)
	{
	        _DtSvcProcessUnlock();
		return(0);
	}

	count = head->files_count;
	mtime_list = _DtDtsMMGetPtr(head->mtimes_offset);
	boson_list = _DtDtsMMGetPtr(head->files_offset);

	for(i = 0; i < count; i++)
	{
		file = _DtDtsMMBosonToString(boson_list[i]);
		/* A file that is gone (or unreadable) invalidates the cache. */
		if(!file || stat(file, &buf) == -1 ||
		   mtime_list[i] != buf.st_mtime)
		{
		        _DtSvcProcessUnlock();
			return(0);
		}
	}

	_DtSvcProcessUnlock();
	return(1);

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
