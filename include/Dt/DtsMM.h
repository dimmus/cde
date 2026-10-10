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
/* $XConsortium: DtsMM.h /main/8 1996/08/28 14:27:26 rswiston $ */
/*
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */
#ifndef DT_DTS_MM_H
#define DT_DTS_MM_H

#include <stdio.h>
#include <Dt/DtShmDb.h>
#include <X11/Intrinsic.h>
#include "Dt/DbReader.h"

#define	DTDTSDB_TMPDATABASENAME	"%s/.dt/.tmp_dt_db_cache.%s\0"
#define	DTDTSDB_DATABASENAME	"%s/.dt/.dt_db_cache.%s\0"
#define	_DTDTSMMTEMPFILE	"dtdbcache_"

/*
 * NOTE: _DTDTSMMTEMPDIR affects the location of the dtdbcache
 * file, and therefore affects the Xsession.src, Xreset.src, and
 * Xstartup.src scripts in dtlogin/config.
 */
#define _DTDTSMMTEMPDIR	"/tmp"

typedef	int	DtDtsMMSeqNo;		/* the order it occures in db */
typedef	int	DtDtsMMFieldCount;	/* number of fields in record */
typedef	int	DtDtsMMRecordCount;	/* number of records in field */
typedef	int	DtDtsMMDataBaseCount;	/* how many databases */
typedef	int	DtDtsMMFieldStart;	/* index in table where field list starts */
typedef	int	DtDtsMMRecordStart;	/* index in table where record list starts */
typedef	int	DtDtsMMDataBaseStart;	/* index in table where database list starts */
typedef	int	DtDtsMMIndexOffset;
typedef	int	DtDtsMMNameIndex;
typedef	int	DtDtsMMPathHash;

/*
 * The dtdbcache file starts with this header.
 *
 * The first word of the old (version 1) format was a "path hash", a sum
 * of bytes, which readers compared first, rejecting the file on a
 * mismatch.  DTDTSMM_MAGIC is negative, which that sum never
 * is, so those readers reject the files written now; readers now reject
 * any file without the magic number and version.
 *
 * Version 2:
 *   - the file list (files_offset) is DtDtsMMStamp entries: the
 *     database directories in search path order, each followed by its
 *     database files, with their mtimes and ctimes in nanoseconds;
 *   - searchpath is the expanded DTDATABASESEARCHPATH it was built from;
 *   - the string table (strtab.c) hashes with FNV-1a into twice as many
 *     buckets as strings, and links entries with 32-bit indices; the
 *     name index tables (inttab.c) link with 32-bit indices.
 */
#define	DTDTSMM_MAGIC		(-0x2824353B)	/* 0xD7DBCAC5 */
#define	DTDTSMM_VERSION		2

typedef	struct
{
	int			magic;		/* DTDTSMM_MAGIC */
	int			version;	/* DTDTSMM_VERSION */
	int			size;		/* bytes in the file */
	DtShmBoson		searchpath;	/* expanded search path */
	DtDtsMMDataBaseCount	num_db;		/* number of databases */
	DtDtsMMDataBaseStart	db_offset;	/* index to databases */
	DtDtsMMNameIndex	name_list_offset;	/* index to name list */
	DtDtsMMNameIndex	no_name_offset;		/* index to nonunique names */
	DtDtsMMNameIndex	buffer_start_index;	/* index to list of buffers */
	DtDtsMMIndexOffset	str_tbl_offset;		/* index to table of strings */
	DtDtsMMIndexOffset	files_count;		/* number of DtDtsMMStamps */
	DtDtsMMIndexOffset	files_offset;		/* index to the DtDtsMMStamps */
} DtDtsMMHeader;

/* What a directory or database file was like when the cache was built. */
typedef	struct
{
	DtShmBoson		path;		/* absolute path name */
	int			is_dir;		/* a database directory */
	int			error;		/* errno of stat(), or 0 */
	int			size;		/* st_size (low 32 bits) */
	int			mtime[3];	/* st_mtim: seconds (low, high), ns */
	int			ctime[3];	/* st_ctim: likewise */
} DtDtsMMStamp;

struct stat;
void	_DtDtsMMFillStamp(DtDtsMMStamp *stamp, const struct stat *st, int error);
char *	_DtDtsMMSearchPath(void);

/*
 * The override argument of _DtDtsMMCreateDb(): replace the cache file
 * named, but if that fails use a private one (rather than fail).
 */
#define	DTDTSMM_SHARED_OR_PRIVATE	2

/* one set of attribute/pair */
typedef	struct
{
	DtShmBoson		fieldName;	/* name of attribute */
	DtShmBoson		fieldValue;	/* value of attribute */
} DtDtsMMField;

/* typedefs for casting comparison functions if needed */
typedef	int	(*_DtDtsMMFieldCompare)(DtDtsMMField *fld1, DtDtsMMField *fld2);

/* entry of a list of attribute/pairs */
typedef	struct
{
	DtShmBoson		recordName;	/* name of this entry */
	DtShmBoson		pathId;		/* file entry is located in */
	DtDtsMMSeqNo		seq;		/* sequence this got loaded */
	DtDtsMMFieldCount	fieldCount;	/* number of fields in record */
	DtDtsMMFieldStart	fieldList;	/* index to field table */
} DtDtsMMRecord;

/* typedefs for casting record comparison functions if needed */
typedef	int	(*_DtDtsMMRecordCompare)(DtDtsMMRecord *rec1, DtDtsMMRecord *rec2);

/* a "database" of a collection of entrys (i.e. OBJECT-TYPE, ACTION, FILE-TYPE 
	This is a private Structure to the DtDtsMM component.
*/
typedef	struct
{
	DtShmBoson		databaseName;	/* name of database */
	DtDtsMMIndexOffset	nameIndex;	/* index for DataCriteria quick find */
	DtDtsMMRecordCount	recordCount;	/* number of records */
	DtDtsMMRecordStart	recordList;	/* index to records table */
} DtDtsMMDatabase;


/* Db Internal pointers */
int *			_DtDtsMMGetDCNameIndex(int *size);
int *			_DtDtsMMGetBufferIndex(int *size);
int *			_DtDtsMMGetNoNameIndex(int *size);
void *			_DtDtsMMGetPtr(int index);
DtShmInttab		_DtDtsMMGetFileList(void);
int			_DtDtsMMGetPtrSize(int index);
int			_DtDtsMMInit(int);
void			_DtDtsMMPrint(FILE *org_fd);
int			_DtDtsMMCreateDb(DtDirPaths *dirs, const char *CacheFile, int override);
int			_DtDtsMMCreateFile(DtDirPaths *dirs, const char *CacheFile, int fallback);
char *			_DtDtsMMCacheName(int);
int			_DtDtsMMapDB(const char *CacheFile);
int			_DtDtsMMapFd(int fd);
int			_DtDtsMMapNewFd(int fd);
void			_DtDtsMMStampDirs(DtDirPaths *dirs);

const char *		_DtDtsMMBosonToString(DtShmBoson boson);
DtShmBoson		_DtDtsMMStringToBoson(const char *string);

extern	int	use_in_memory_db;


/* returns the handle for the database where name is the Database name */
extern	DtDtsMMDatabase		*_DtDtsMMGet(const char *name);
extern	char			**_DtDtsMMListDb(void);

/* FIXME: document */
extern int *_DtDtsMMGetDbName(DtDtsMMDatabase *db, DtShmBoson boson);


/* Name Comparison functions:
 * These routines can be passed in to the corresponding sort function to
 * sort by name.
 *
 */
extern int _DtDtsMMCompareRecordNames(DtDtsMMRecord *entry1, DtDtsMMRecord *entry2);
extern int _DtDtsMMCompareFieldNames(DtDtsMMField *entry1, DtDtsMMField *entry2);

/* retrieves the Record that matches the specified entry from the record */
extern	DtDtsMMField	*_DtDtsMMGetField(DtDtsMMRecord *record,
					const char *value);
extern const char *_DtDtsMMGetFieldByName(DtDtsMMRecord *rec, const char *name);

/* retrieves the entry of the specified entry from the specified database */
extern	DtDtsMMRecord	*_DtDtsMMGetRecord(DtDtsMMDatabase *database,
					DtDtsMMRecord *value);
extern	DtDtsMMRecord	*_DtDtsMMGetRecordByName(DtDtsMMDatabase *database,
					const char *value);

/* Get By Name functions:
 * retrieves the entry of the specified name from the specified database
 * ** IF ** the _DtDtsMM*Sort routine has been called with the corresponding
 * _DtDtsMMCompare*Name comparison function. Otherwise use the standard
 * _DtDtsMMGet* functions. 
*/


char *	_DtDtsMMExpandValue(const char *value);
/*
 * Like _DtDtsMMExpandValue(), but when VALUE (a string from the mapped
 * database) has nothing to expand, VALUE itself is returned instead of a
 * copy.  The result is read-only and must be released with
 * _DtDtsMMSafeFree().
 */
char *	_DtDtsMMExpandValueNoCopy(const char *value);
void	_DtDtsMMSafeFree(char *value);
int	_DtDtsMMIsMemory(const char *value);

/*
 * A number that changes whenever the database is mapped or unmapped.
 * Callers caching bosons or pointers into the mapping compare it with
 * the value they saw when they filled their cache.
 */
unsigned int	_DtDtsMMGeneration(void);

extern	DtShmBoson	_DtDtsMMNameStringToBoson(const char *string);


#endif /* DT_DTS_MM_H */
