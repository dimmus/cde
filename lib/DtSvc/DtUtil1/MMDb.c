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
/* $TOG: MMDb.c /main/19 1998/10/23 13:48:52 mgreess $ */
/*
 * +SNOTICE
 * 
 * Copyright 1995 Sun Microsystems, Inc.  All rights reserved.
 * 
 * +ENOTICE
 */
#include <stdio.h>
#include <sys/types.h>

#include <unistd.h>
#include <sys/utsname.h>
#include <stdlib.h>

#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

#ifdef NLS16
#include <limits.h>
#endif

#include <sys/stat.h>
#include <sys/param.h>		/* MAXPATHLEN, MAXHOSTNAMELEN */
#define X_INCLUDE_DIRENT_H
#define XOS_USE_XT_LOCKING
#include <X11/Xos_r.h>
#include <Dt/DbReader.h>
#include <Dt/DtsDb.h>
#include <Dt/DtsMM.h>
#include <Dt/DtShmDb.h>
#include <Dt/Dts.h>
#include <Dt/ActionP.h>
#include <Dt/ActionDbP.h>
#include <Dt/ActionUtilP.h>
#include <Dt/DtNlUtils.h>
#include <Dt/UserMsg.h>
#include "myassertP.h"
#include "DtSvcLock.h"

static void build_file_list(DtShmProtoIntList, DtDirPaths *,
			    DtDtsMMHeader *);
extern	int	cde_dc_field_compare(DtDtsDbField **, DtDtsDbField **);
extern	int	cde_dc_compare(DtDtsDbRecord **, DtDtsDbRecord **);
static void	_DtMMSortDataTypes(DtShmProtoStrtab str_handle);
static void	_DtMMAddActionsToDataAttribute(DtDtsDbDatabase *db_ptr);
static int	write_db(DtDtsMMHeader *header, void *index, int size,
			 const char *CacheFile, int quiet);
static int	build_new_db(DtShmProtoStrtab, DtShmProtoIntList, int, DtDtsDbDatabase **);
static int	build_name_list(DtDtsDbDatabase *, DtShmProtoIntList, DtDtsMMHeader *);

static	DtShmProtoStrtab	shm_handle = 0;
static	DtShmProtoIntList	int_handle = 0;

#define QtB(a)	_DtShmProtoAddStrtab(shm_handle, XrmQuarkToString(a), &isnew)

/*
 * The database directories and files, as _DtDtsMMStampDirs() found them
 * before the files were read (so that a change made while they are read
 * makes the cache stale).
 */
static DtDtsMMStamp	*stamps;
static char		**stamp_paths;	/* the path of each stamp */
static int		stamp_count;
static int		stamp_max;
static char		*stamp_searchpath;

static void
free_stamps(void)
{
	int	i;

	for (i = 0; i < stamp_count; i++)
	{
		free(stamp_paths[i]);
	}
	free(stamps);
	free(stamp_paths);
	XtFree(stamp_searchpath);
	stamps = NULL;
	stamp_paths = NULL;
	stamp_searchpath = NULL;
	stamp_count = stamp_max = 0;
}

static void
add_stamp(const char *path, int is_dir, const struct stat *st, int error)
{
	if (stamp_count == stamp_max)
	{
		stamp_max = stamp_max ? 2 * stamp_max : 128;
		stamps = realloc(stamps, stamp_max * sizeof(*stamps));
		stamp_paths = realloc(stamp_paths,
				      stamp_max * sizeof(*stamp_paths));
		if (!stamps || !stamp_paths)
		{
			_DtSimpleError(DtProgName, DtError, NULL,
				       "out of memory", NULL);
			abort();
		}
	}
	_DtDtsMMFillStamp(&stamps[stamp_count], st, error);
	stamps[stamp_count].is_dir = is_dir;
	stamps[stamp_count].path = 0;
	stamp_paths[stamp_count] = strdup(path);
	stamp_count++;
}

/*
 * Records the search path, and each database directory followed by its
 * database files (the files _DtDbRead() reads: those
 * _DtFindMatchingFiles() finds), with what stat() says about them.
 * MMValidateDb() (DtsMM.c) checks them the same way.
 */
void
_DtDtsMMStampDirs(DtDirPaths *dirs)
{
	int		i;

	_DtSvcProcessLock();
	free_stamps();
	stamp_searchpath = _DtDtsMMSearchPath();
	for(i = 0; dirs->paths[i]; i++)
	{
		struct stat	buf;
		struct dirent	*entry;
		DIR		*dirp = NULL;
		int		dfd;

		/* The directory first: a file added later changes its mtime. */
		dfd = open(dirs->paths[i], O_RDONLY|O_DIRECTORY|O_CLOEXEC);
		if (dfd == -1 || fstat(dfd, &buf) == -1)
		{
			add_stamp(dirs->paths[i], 1, NULL, errno);
		}
		else
		{
			add_stamp(dirs->paths[i], 1, &buf, 0);
			dirp = fdopendir(dfd);
		}
		if (!dirp)
		{
			if (dfd != -1)
				close(dfd);
			continue;
		}
		while ((entry = readdir(dirp)) != NULL)
		{
			char	*pathname;
			size_t	len;
#ifdef DT_UNKNOWN
			unsigned char	d_type = entry->d_type;
#else
			unsigned char	d_type = 0;
#endif

			if (!_DtDbFileMatches(dirfd(dirp), entry->d_name,
					      d_type, ".dt"))
			{
				continue;
			}
			len = strlen(dirs->paths[i]) + strlen(entry->d_name) + 2;
			pathname = malloc(len);
			if (!pathname)
				continue;
			snprintf(pathname, len, "%s/%s", dirs->paths[i],
				 entry->d_name);
			if (fstatat(dirfd(dirp), entry->d_name, &buf, 0) == -1)
				add_stamp(pathname, 0, NULL, errno);
			else
				add_stamp(pathname, 0, &buf, 0);
			free(pathname);
		}
		closedir(dirp);
	}
	_DtSvcProcessUnlock();
}

/*
 * Writes the database to a new cache file, which replaces CacheFile, or,
 * if CacheFile is NULL (or replacing it fails and fallback is set), is
 * already unlinked.  Returns the file open for reading, or -1.
 */
int
_MMWriteDb(DtDirPaths *dirs, int num_db, DtDtsDbDatabase **db_list,
	   const char *CacheFile, int fallback)
{
	DtDtsMMHeader		header;
	int			tbl_size;
	void			*tbl_data;
	DtDtsDbDatabase	        *db;
	int			returnCode;
	int			isnew;

	_DtSvcProcessLock();
	memset(&header, '\0', sizeof(header));
	int_handle = _DtShmProtoInitIntLst(50000);
	shm_handle = _DtShmProtoInitStrtab(10000);

	if (!stamps)
	{
		_DtDtsMMStampDirs(dirs);
	}
	build_file_list(int_handle, dirs, &header);

	_DtMMSortDataTypes(shm_handle);
	db = (DtDtsDbDatabase	*) _DtDtsDbGet(DtDTS_DA_NAME);
	_DtMMAddActionsToDataAttribute(db);

	header.magic = DTDTSMM_MAGIC;
	header.version = DTDTSMM_VERSION;
	header.searchpath = _DtShmProtoAddStrtab(shm_handle,
			stamp_searchpath ? stamp_searchpath : "", &isnew);
	free_stamps();
	header.num_db = num_db;
	header.db_offset = build_new_db(shm_handle, int_handle, num_db,
db_list);
	db = (DtDtsDbDatabase	*) _DtDtsDbGet("DATA_CRITERIA");
	build_name_list(db, int_handle, &header);

	tbl_size = _DtShmProtoSizeStrtab(shm_handle);
	tbl_data = (void *) _DtShmProtoAddIntLst(int_handle,
				tbl_size/sizeof(int), &header.str_tbl_offset);
	_DtShmProtoCopyStrtab(shm_handle, tbl_data);


	tbl_size = _DtShmProtoSizeIntLst(int_handle);
	tbl_data = (void *)malloc(tbl_size);
	memset(tbl_data, '\0', tbl_size);
	tbl_data = (void *)_DtShmProtoCopyIntLst(int_handle, tbl_data);
	header.size = sizeof(header) + tbl_size;

	returnCode = write_db(&header, tbl_data, tbl_size, CacheFile, fallback);
	if (returnCode == -1 && CacheFile && fallback)
	{
		returnCode = write_db(&header, tbl_data, tbl_size, NULL, 0);
	}
	_DtShmProtoDestroyStrtab(shm_handle);
	_DtShmProtoDestroyIntLst(int_handle);
	_DtSvcProcessUnlock();
	free(tbl_data);

	return returnCode;
}

static void
build_file_list(DtShmProtoIntList int_handle, DtDirPaths *dirs,
		DtDtsMMHeader *header)
{
	void			*data;
	int			i;
	int			isnew;

	/* Theses here to make sure it gets into the string tables
	   because actions uses it in its "types" field. */
	_DtShmProtoAddStrtab(shm_handle, DtDTS_DT_UNKNOWN, &isnew);
	_DtShmProtoAddStrtab(shm_handle, DtDTS_DT_RECURSIVE_LINK, &isnew);
	_DtShmProtoAddStrtab(shm_handle, DtDTS_DT_BROKEN_LINK, &isnew);

	for(i = 0; i < stamp_count; i++)
	{
		stamps[i].path = _DtShmProtoAddStrtab(shm_handle,
					stamp_paths[i], &isnew);
	}
	data = _DtShmProtoAddIntLst(int_handle,
			stamp_count * sizeof(DtDtsMMStamp) / sizeof(int),
			&header->files_offset);
	if (stamp_count)
	{
		memcpy(data, stamps, stamp_count * sizeof(DtDtsMMStamp));
	}
	header->files_count = stamp_count;
	return;
}

static void
_DtMMSortDataTypes(DtShmProtoStrtab str_handle)
{
	DtDtsDbDatabase	*dc;
	DtDtsDbDatabase	*da;
	int		i;

	_DtSvcProcessLock();       
	dc = (DtDtsDbDatabase *) _DtDtsDbGet(DtDTS_DC_NAME);
	da = (DtDtsDbDatabase *) _DtDtsDbGet(DtDTS_DA_NAME);

/*_DtDtsDbPrintRecords(dc, stdout);*/
	for(i = 0; i < dc->recordCount; i++)
	{
		if(dc->recordList[i]->compare != cde_dc_field_compare)
		{
			_DtDtsDbFieldSort(dc->recordList[i], 
				cde_dc_field_compare);
		}
	}
	_DtDtsDbRecordSort(dc, cde_dc_compare);

	for(i = 0; i < da->recordCount; i++)
	{
		if(da->recordList[i]->compare !=
				_DtDtsDbCompareFieldNames)
		{
			_DtDtsDbFieldSort(da->recordList[i], 
				_DtDtsDbCompareFieldNames);
		}
	}

	_DtDtsDbRecordSort(da, _DtDtsDbCompareRecordNames);
/*_DtDtsDbPrintRecords(dc, stdout);*/
	_DtSvcProcessUnlock();
}

/*
 * Adds the field name=value to rec_ptr unless it has one of that name
 * (then value, malloc'ed, is freed).  The fields are kept sorted by
 * name: the new one goes where sorting the list would put it, without
 * sorting the whole list again.
 */
static void
add_if_missing(DtDtsDbRecord *rec_ptr, XrmQuark name, char *value)
{
	DtDtsDbField	*fld_ptr;
	int		fld;
	int		pos;

	for(fld = 0; fld < rec_ptr->fieldCount; fld++)
	{
		fld_ptr = rec_ptr->fieldList[fld];
		if(name == fld_ptr->fieldName)
		{
			free(value);
			return;
		}
	}

	fld_ptr = _DtDtsDbAddField(rec_ptr);
	fld_ptr->fieldName = name;
	fld_ptr->fieldValue = value;

	if(rec_ptr->compare != _DtDtsDbCompareFieldNames)
	{
		_DtDtsDbFieldSort(rec_ptr, 0);
		return;
	}
	/* The other fields are sorted by name; none has this name. */
	for(pos = rec_ptr->fieldCount - 1;
	    pos > 0 && rec_ptr->fieldList[pos - 1]->fieldName > name;
	    pos--)
	{
		rec_ptr->fieldList[pos] = rec_ptr->fieldList[pos - 1];
	}
	rec_ptr->fieldList[pos] = fld_ptr;

	return;
}

static void
_DtMMAddActionsToDataAttribute(DtDtsDbDatabase *db_ptr)
{
	int		rec;
	DtDtsDbRecord	*rec_ptr;
	XrmQuark	desc_qrk = XrmStringToQuark(DtDTS_DA_DESCRIPTION);
	XrmQuark	icon_qrk = XrmStringToQuark(DtDTS_DA_ICON);
	XrmQuark	label_qrk = XrmStringToQuark(DtDTS_DA_LABEL);

	for(rec = 0; rec < db_ptr->recordCount; rec++)
	{
		char	*obj_type;

		rec_ptr = db_ptr->recordList[rec];
		obj_type = XrmQuarkToString(rec_ptr->recordName);

		if ( _DtDtsDbGetFieldByName(rec_ptr,
				DtDTS_DA_IS_ACTION) == 0 )
		{
			continue;
		}
		add_if_missing(rec_ptr, desc_qrk, 
				DtActionDescription(obj_type));
		add_if_missing(rec_ptr, icon_qrk, DtActionIcon(obj_type));
		add_if_missing(rec_ptr, label_qrk, DtActionLabel(obj_type));
	}
}

static int
build_new_db(DtShmProtoStrtab shm_handle, DtShmProtoIntList int_handle, int num_db, DtDtsDbDatabase **db_list)
{
	DtDtsMMDatabase		*new_db_list;
	int			db;
	DtDtsDbDatabase		*db_ptr;
	DtDtsMMDatabase		*new_db_ptr;
	int			rec;
	DtDtsDbRecord		*rec_ptr;
	DtDtsMMRecord		*new_rec_ptr;
	DtDtsMMRecord		*new_rec_ptr_list;
	int			fld;
	DtDtsDbField		*fld_ptr;
	DtDtsMMField		*new_fld_ptr;
	DtDtsMMField		*new_fld_ptr_list;
	int			index;
	int			db_index;
	int			isnew;
	char			*tmp;

	/* create a space to hold the list of database structures */
	new_db_list = (DtDtsMMDatabase *)_DtShmProtoAddIntLst(int_handle,
			num_db*sizeof(DtDtsMMDatabase)/sizeof(int),
			&db_index);
	for(db = 0; db < num_db; db++)
	{
		int	last_boson = -1;
		DtShmProtoInttab	nameIndex;
		int		size;
		int		*idx;

		new_db_ptr = &new_db_list[db];
		db_ptr = db_list[db];

		new_db_ptr->databaseName = _DtShmProtoAddStrtab(shm_handle, db_ptr->databaseName, &isnew);
		new_db_ptr->recordCount = db_ptr->recordCount;
		/* create space to hold record list */
		new_rec_ptr_list = (DtDtsMMRecord *)_DtShmProtoAddIntLst(int_handle,
				db_ptr->recordCount*sizeof(DtDtsMMRecord)/sizeof(int),
				&index);

		new_db_ptr->recordList = index;
		/* create index to names list */
		nameIndex = _DtShmProtoInitInttab(db_ptr->recordCount);
		for(rec = 0; rec < db_ptr->recordCount; rec++)
		{
			new_rec_ptr = &new_rec_ptr_list[rec];
			rec_ptr = db_ptr->recordList[rec];
			new_rec_ptr->recordName = QtB(rec_ptr->recordName);

			if(new_rec_ptr->recordName != last_boson)
			{
				/* save name position */
				_DtShmProtoAddInttab(nameIndex, new_rec_ptr->recordName, rec);

				last_boson = new_rec_ptr->recordName;
			}
			new_rec_ptr->pathId = _DtShmProtoAddStrtab(shm_handle,
				tmp = _DtDbPathIdToString(rec_ptr->pathId),
						 &isnew);
			XtFree(tmp);
			new_rec_ptr->seq = rec_ptr->seq;
			new_rec_ptr->fieldCount = rec_ptr->fieldCount;

			/* create space for field list */
			new_fld_ptr_list = (DtDtsMMField *)_DtShmProtoAddIntLst(int_handle,
				rec_ptr->fieldCount*sizeof(DtDtsMMField)/sizeof(int),
				&index);

			new_rec_ptr->fieldList = index;
			for(fld = 0; fld < rec_ptr->fieldCount; fld++)
			{
				new_fld_ptr = &new_fld_ptr_list[fld];
				fld_ptr = rec_ptr->fieldList[fld];

				new_fld_ptr->fieldName  = QtB(fld_ptr->fieldName);
				new_fld_ptr->fieldValue = fld_ptr->fieldValue?_DtShmProtoAddStrtab(shm_handle,
					fld_ptr->fieldValue, &isnew):0;
			}
		}
		/* create table for index and save it */
		size = _DtShmProtoSizeInttab(nameIndex);
		idx = _DtShmProtoAddIntLst(int_handle, size/sizeof(int), &new_db_ptr->nameIndex);
		_DtShmProtoCopyInttab(nameIndex, (void *)idx);
		_DtShmProtoDestroyInttab(nameIndex);
	}
	return(db_index);
}

struct  list
{
	DtShmBoson	boson;
	int		rec;
};

static int
srch(const void *a, const void *b)
{
	int results = ((struct list *)a)->boson - ((struct list *)b)->boson;

	if(results == 0)
	{
		results = ((struct list *)a)->rec - ((struct list *)b)->rec;
	}
	return(results);
}

static int
build_name_list(DtDtsDbDatabase *db,
		DtShmProtoIntList int_handle,
		DtDtsMMHeader	*head)
{
	struct list 	*other;
	int		i;
	char		*c;
	int		isnew;
	struct list	*name_index;
	int		next = 0;
	int		other_break = 0;
	DtShmProtoInttab	indexList = 0;
	DtShmBoson	last_boson = -1;
	int		*list_of_recs = 0;
	int		list_count = 0;
	int		index = 0;
	int		size;
	void		*space;

	/* create tmp space for two lists */
	name_index = (struct list *)calloc(db->recordCount*2,
					sizeof(struct list));
	other = (struct list *)calloc(db->recordCount, sizeof(struct list));

	/* step through all records */
	for(i = 0; i < db->recordCount; i++)
	{
		char	*attr;

		/* see if a name pattern exist */
		attr = _DtDtsDbGetFieldByName(db->recordList[i],
				DtDTS_NAME_PATTERN);
		if(!attr)
		{
			/* it didn't so check path pattern */
			attr = _DtDtsDbGetFieldByName(db->recordList[i],
				DtDTS_PATH_PATTERN);
			if(!attr)
			{
				/* neither exist so save it as plain buffer */
				if(!head->buffer_start_index)
				{
					head->buffer_start_index = other_break;
				}
				other[other_break++].rec = i;
				continue; /* go to next record */
			}
		}

		/* we have a name now find its final component */
		c = strrchr(attr, '/');
		if(c)
		{
			c++;
		}
		if(!c)
		{
			c = attr;
		}
		else
		{
			attr = c;
		}

		/* now see if that final component has any *,?,[ */
		while(c && *c &&
			  !(*c == '*' ||
			    *c == '[' ||
			    *c == '?' ||
			    *c == '$' ))
		{
			c++;
		}


		if(c && *c == '\0')
		{
			/* it doesn't so save it in the name index */
			name_index[next].boson = 
				_DtShmProtoAddStrtab(shm_handle,
					(const char *)attr, &isnew);
			name_index[next++].rec = i;
			continue; /* next record */
		}

		/* the name had something in it now lets get the suffix */
		c = strrchr(attr, '.');
		attr = c;

		/* lets see if the suffix has any  *,?,[ */
		while(c && *c &&
			  !(*c == '*' ||
			    *c == '[' ||
			    *c == '?' ||
			    *c == '$' ))
		{
			c++;
		}
		if(c && *c == '\0')
		{
			/* it doesn't so save it in the name index */
			name_index[next].boson = 
			_DtShmProtoAddStrtab(shm_handle,
					(const char *)attr, &isnew);
			name_index[next++].rec = i;
		}
		else
		{
			/* couldn't find any thing so save it as other */
			other[other_break++].rec = i;
		}

	}

	if (next > 0)
	{
		qsort(name_index, next, sizeof(struct list), srch);
	}

/*
showtable(db, name_index, other, head, other_break);
printf("                    next = %d\n", next);
printf("             other_break = %d\n", other_break);
printf("head->buffer_start_index = %d\n", head->buffer_start_index);
*/
	/* create a table and add the records to it. However
	   duplicates need to be in separate lists.
	*/
	indexList = _DtShmProtoInitInttab(next+3);
	/* A list never holds more than all next records (it used to grow
	   by one entry at a time). */
	list_of_recs = (int *)malloc((next > 0 ? next : 1)*sizeof(int));
	for(i = 0; i <= next; i++)
	{
		if(i != next && (last_boson == -1 || name_index[i].boson == last_boson))
		{
			/* this a new list of records or an addition to one */
			++list_count;
			last_boson = name_index[i].boson;
			list_of_recs[list_count-1] = name_index[i].rec;
		}
		else
		{
			/* we reached the end of a list now we check how many
				are in the list. 
			*/
			if(list_count == 1)
			{
				/* if just one just add it in the index */
				_DtShmProtoAddInttab(indexList,
						last_boson, list_of_recs[0]);
			}
			else
			{
				/* if there are multiple items in the list
				   create a table for them */
				int	*list = _DtShmProtoAddIntLst(int_handle,
						list_count, &index);

				/* write the list to the to the table */
				memcpy(list, list_of_recs,
						list_count*sizeof(int));

				/* then index on the negative of the boson
				   so that we know it is a list */
				_DtShmProtoAddInttab(indexList,
						last_boson, -index);
				list_count = 1;
			}
			if ( i != next )
			{
				/* reset for the next set */
				last_boson = name_index[i].boson;
				list_of_recs[list_count-1] = name_index[i].rec;
			}
		}

	}

	/* same thing but they all go into a separate list */
	if(other_break > 0)
	{
		/* create the space */
		int	*list = _DtShmProtoAddIntLst(int_handle,
				other_break, &head->no_name_offset);

		/* copy it into the list */
		for(i = 0; i < other_break; i++)
		{
			list[i] = other[i].rec;
		}
	}
	else
	{
		head->no_name_offset = -1;
	}

	/* make the real space */
	size = _DtShmProtoSizeInttab(indexList);
	space = _DtShmProtoAddIntLst(int_handle, size/sizeof(int), 
				&head->name_list_offset);
	_DtShmProtoCopyInttab(indexList, space);
	_DtShmProtoDestroyInttab(indexList);
	free(name_index);
	free(list_of_recs);
	free(other);
	return(index);
}

static int
write_all(int fd, const void *data, size_t size)
{
	const char	*p = data;

	while (size > 0)
	{
		ssize_t	n = write(fd, p, size);

		if (n == -1 && errno == EINTR)
			continue;
		if (n <= 0)
			return 0;
		p += n;
		size -= n;
	}
	return 1;
}

/*
 * Writes the cache file under a temporary name, then renames it to
 * CacheFile (so readers never see a partial file), or, if CacheFile is
 * NULL, unlinks it (a private cache).  Returns the file, open, or -1.
 * Errors are not reported when quiet is set.
 */
static int
write_db(DtDtsMMHeader *header, void *index, int size, const char *CacheFile,
	 int quiet)
{
	int	fd;
	mode_t	cmask = umask((mode_t)077);
	char	*tmpfile;

	if ((tmpfile = malloc(sizeof(_DTDTSMMTEMPDIR) +
	    sizeof(_DTDTSMMTEMPFILE) + 7)) == NULL) {
		umask(cmask);
		_DtSimpleError(DtProgName, DtError, NULL, "out of memory",
			       NULL);
		return -1;
	}

	sprintf(tmpfile, "%s/%sXXXXXX", _DTDTSMMTEMPDIR, _DTDTSMMTEMPFILE);
	fd = mkstemp(tmpfile);

	umask(cmask);

	if(fd ==  -1)
	{
		if (!quiet)
			_DtSimpleError(
				DtProgName, DtError, NULL,
				(char*) tmpfile, NULL);
		free(tmpfile);
		return(-1);
	}
	(void) fcntl(fd, F_SETFD, FD_CLOEXEC);

	/* Remove file on write failure - we don't */
	/* want a partial dtdbcache file. */
	if (!write_all(fd, header, sizeof(DtDtsMMHeader)) ||
	    !write_all(fd, index, size))
	{
		close(fd);
		unlink(tmpfile);
		free(tmpfile);
		return(-1);
	}

	if (!CacheFile)
	{
		unlink(tmpfile);
	}
	else if(rename((const char *)tmpfile, CacheFile) == -1)
	{
		if (!quiet)
			_DtSimpleError(
				DtProgName, DtError, NULL,
				(char*) CacheFile, NULL);
		close(fd);
		unlink(tmpfile);
		free(tmpfile);
		return(-1);
	}
	free(tmpfile);
	return(fd);
}


intptr_t _DtActionCompareRecordBoson(
        DtDtsMMRecord *record1,
        DtDtsMMRecord *record2 )
{
	int results = (int)record1->recordName - (int)record2->recordName;

	if (results)
		return(results);

	return((intptr_t)record1 - (intptr_t)record2);
}
