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
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 *+SNOTICE
 *
 *	$TOG: DtsDb.c /main/10 1998/10/23 13:48:04 mgreess $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef	SUN_DB
#include <sys/mman.h>
#endif
#include <string.h>
#include <Dt/DbReader.h>
#include "Dt/DtsDb.h"
#include <Dt/UserMsg.h>
#include "DtSvcLock.h"

extern int _MMWriteDb(DtDirPaths *dirs, int num_db, DtDtsDbDatabase **db_list,
                      const char *CacheFile, int fallback);



typedef	int	(*genfunc)(const void *, const void *);

static	DtDtsDbDatabase	**db_list;
static	int	num_db = 0;

/*
 * Room for n+1 entries in a record or field list holding n entries.
 * The lists grow geometrically: they are reallocated when n is 0 (to 8
 * entries) and when n is a power of two of at least 8 (to 2n), so the
 * capacity never needs to be stored, and deleting entries (which only
 * lowers n) keeps it valid.
 */
static void *
grow_list(void *list, int n, size_t size)
{
	size_t	cap;
	void	*newlist;

	if (n == 0)
		cap = 8;
	else if (n >= 8 && (n & (n - 1)) == 0)
		cap = 2 * (size_t)n;
	else
		return list;
	newlist = realloc(list, cap * size);
	if (!newlist)
	{
		_DtSimpleError(DtProgName, DtError, NULL,
			       "out of memory", NULL);
		abort();
	}
	return newlist;
}

/*
 * Record name -> first record of that name, for unsorted databases
 * (see DtDtsDbDatabase.nameIndex).  Open addressing; quarks are never 0
 * for a real name, so 0 marks a free slot.
 */
struct _DtDtsDbNameIndex
{
	unsigned int	mask;		/* slot count - 1 (power of two) */
	unsigned int	used;
	struct
	{
		XrmQuark	name;
		DtDtsDbRecord	*rec;
	}		*slot;
};

static void
name_index_drop(DtDtsDbDatabase *db)
{
	if (db->nameIndex)
	{
		free(db->nameIndex->slot);
		free(db->nameIndex);
		db->nameIndex = NULL;
	}
	db->nameIndexed = 0;
}

static unsigned int
name_hash(XrmQuark q)
{
	return (unsigned int)q * 2654435761u;
}

/*
 * Returns 0 if out of memory, or for a record that has no name yet (it
 * may get one later); the caller then searches linearly.
 */
static int
name_index_add(struct _DtDtsDbNameIndex *ix, DtDtsDbRecord *rec)
{
	unsigned int	i;

	if (rec->recordName == NULLQUARK)
		return 0;
	if (2 * (ix->used + 1) > ix->mask + 1)
	{
		struct _DtDtsDbNameIndex	big;
		unsigned int			j;

		big.mask = 2 * (ix->mask + 1) - 1;
		big.used = 0;
		big.slot = calloc(big.mask + 1, sizeof(*big.slot));
		if (!big.slot)
			return 0;
		for (j = 0; j <= ix->mask; j++)
		{
			if (ix->slot[j].name == NULLQUARK)
				continue;
			i = name_hash(ix->slot[j].name) & big.mask;
			while (big.slot[i].name != NULLQUARK)
				i = (i + 1) & big.mask;
			big.slot[i] = ix->slot[j];
			big.used++;
		}
		free(ix->slot);
		*ix = big;
	}
	i = name_hash(rec->recordName) & ix->mask;
	while (ix->slot[i].name != NULLQUARK)
	{
		if (ix->slot[i].name == rec->recordName)
			return 1;	/* keep the first record of this name */
		i = (i + 1) & ix->mask;
	}
	ix->slot[i].name = rec->recordName;
	ix->slot[i].rec = rec;
	ix->used++;
	return 1;
}

/*
 * Brings the index of db up to date.  Returns it, or NULL when it cannot
 * be used.
 */
static struct _DtDtsDbNameIndex *
name_index_update(DtDtsDbDatabase *db)
{
	struct _DtDtsDbNameIndex	*ix = db->nameIndex;

	if (!ix)
	{
		ix = malloc(sizeof(*ix));
		if (!ix)
			return NULL;
		ix->mask = 63;
		ix->used = 0;
		ix->slot = calloc(ix->mask + 1, sizeof(*ix->slot));
		if (!ix->slot)
		{
			free(ix);
			return NULL;
		}
		db->nameIndex = ix;
		db->nameIndexed = 0;
	}
	for (; db->nameIndexed < db->recordCount; db->nameIndexed++)
	{
		if (!name_index_add(ix, db->recordList[db->nameIndexed]))
		{
			name_index_drop(db);
			return NULL;
		}
	}
	return ix;
}

void
_DtDtsDbPrintFields(DtDtsDbRecord  *rec_ptr, FILE *fd)
{
	int		fld;
	DtDtsDbField	*fld_ptr;

	for(fld = 0; fld < rec_ptr->fieldCount; fld++)
	{
		fld_ptr = rec_ptr->fieldList[fld];
		fprintf(fd, "\t\t[%d]\t%s\t%s\n", fld,
			XrmQuarkToString(fld_ptr->fieldName),
			fld_ptr->fieldValue?
				fld_ptr->fieldValue:"(NULL)");
	}
}

void
_DtDtsDbPrintRecords(DtDtsDbDatabase  *db_ptr, FILE *fd)
{
	int		rec;
	DtDtsDbRecord	*rec_ptr;

	_DtSvcProcessLock();
	fprintf(fd, "%d Records\n", db_ptr->recordCount);
	for(rec = 0; rec < db_ptr->recordCount; rec++)
	{
		rec_ptr = db_ptr->recordList[rec];
		fprintf(fd, "\tRec[%d] name = %s\n\t%d Fields\n", rec,
			XrmQuarkToString(rec_ptr->recordName),
			rec_ptr->fieldCount);
		_DtDtsDbPrintFields(rec_ptr, fd);
	}
	_DtSvcProcessUnlock();
}

void
_DtDtsDbPrint(FILE *org_fd)
{
	int		db;
	DtDtsDbDatabase	*db_ptr;
	FILE		*fd = org_fd;

	_DtSvcProcessLock();
	for(db = 0; db < num_db; db++)
	{
		if(!db_list[db])
		{
			continue;
		}
		db_ptr = db_list[db];
		if(fd == 0)
		{
			chdir("/tmp");
			if((fd = fopen(db_ptr->databaseName, "w")) == NULL)
			{
			    _DtSimpleError(
					DtProgName, DtError, NULL,
					db_ptr->databaseName, NULL);
			    continue;
			}
		}
		fprintf(fd, "DB[%d] ", db);
		fprintf(fd, "name = %s\n", db_ptr->databaseName);
		_DtDtsDbPrintRecords(db_ptr, fd);
		if(org_fd == 0)
		{
			fclose(fd);
			fd = 0;
		}
	}
	_DtSvcProcessUnlock();
}

int
_DtDtsDbCompareRecordNames(DtDtsDbRecord **a, DtDtsDbRecord **b)
{
	return ((*a)->recordName - (*b)->recordName);
}

int
_DtDtsDbCompareFieldNames(DtDtsDbField **a, DtDtsDbField **b)
{
	return ((*a)->fieldName - (*b)->fieldName);
}

#include <Dt/Dts.h>

DtDtsDbDatabase **
_DtDtsDbInit(void)
{
	DtDtsDbDatabase	**db;

	_DtSvcProcessLock();
	num_db = 0;
	db = db_list = (DtDtsDbDatabase **)calloc(num_db+3, sizeof(DtDtsDbDatabase *));
	db_list[0] = (DtDtsDbDatabase *)calloc(1, sizeof(DtDtsDbDatabase));
	db_list[0]->databaseName = (char *)strdup(DtDTS_DC_NAME);
	db_list[0]->ActionSequenceNumber = 0;
	num_db++;

	db_list[1] = (DtDtsDbDatabase *)calloc(1, sizeof(DtDtsDbDatabase));
	db_list[1]->databaseName = (char *)strdup(DtDTS_DA_NAME);
	db_list[1]->ActionSequenceNumber = 0;
	num_db++;

	_DtSvcProcessUnlock();
	return(db);
}

char **
_DtsDbListDb(void)
{
	int	i;
	char	**list = 0;

	_DtSvcProcessLock();
	for ( i = 0; db_list[i]; i++ );

	if(i > 0)
	{
		list = (char **)calloc(i+1, sizeof(char *));
		for ( i = 0; db_list[i]; i++ )
		{
			list[i] = (char *)strdup(db_list[i]->databaseName);
		}
	}
	_DtSvcProcessUnlock();
	return(list);
}

DtDtsDbDatabase *
_DtDtsDbAddDatabase( char *db_name )
{
	int i = 0;
	DtDtsDbDatabase	**new_db_list;
	DtDtsDbDatabase *ret_db;

	_DtSvcProcessLock();
	if ( !db_list )
	{
		_DtDtsDbInit();
	}
	for ( i = 0; db_list[i]; i++ )
	{
		if ( !strcmp(db_list[i]->databaseName,db_name) )
		{
			/*
			 * A database with the given name already exists.
			 * return a pointer to the existing database.
			 */
			ret_db = db_list[i];
		        _DtSvcProcessUnlock();

			return ret_db;
		}
	}	
	/*
	 * We now have a count of the existing databases.
	 * allocate enough space for the existing databases + the new one 
	 * + a NULL pointer to terminate the vector.
	 */

	new_db_list = (DtDtsDbDatabase **)calloc(i+2,sizeof(DtDtsDbDatabase *));

	memmove(new_db_list,db_list,sizeof(DtDtsDbDatabase *) * i );
	new_db_list[i] = (DtDtsDbDatabase *)calloc(1, sizeof(DtDtsDbDatabase));
	new_db_list[i]->databaseName = strdup(db_name);
	new_db_list[i]->ActionSequenceNumber = 0;
	free(db_list);
	db_list = new_db_list;
	num_db++;

	ret_db = db_list[i];
	_DtSvcProcessUnlock();

	return ret_db;
}

int
_DtDtsDbDeleteDb(DtDtsDbDatabase *db)
{
	int	i;
	int	flag = 0;

	_DtSvcProcessLock();
	_DtDtsDbDeleteRecords(db);
	name_index_drop(db);
	free(db->databaseName);
	free(db);

	for(i = 0; db_list[i]; i++)
	{
		if(db_list[i] == db)
		{
			flag = 1;
			db_list[i] = 0;
		}
		if(flag)
		{
			db_list[i] = db_list[i+1];
		}
	}
	if(db_list[0] == 0)
	{
		free(db_list);
		db_list = 0;
	}
	_DtSvcProcessUnlock();
	return(0);
}


DtDtsDbDatabase *
_DtDtsDbGet(char *name)
{
	DtDtsDbDatabase *ret_db;
	int		i;

	_DtSvcProcessLock();
	if(!db_list)
	{
		_DtDtsDbInit();
	}
	for(i = 0; db_list && db_list[i] && db_list[i]->databaseName; i++)
	{
		if(strcmp(db_list[i]->databaseName, name) == 0)
		{
			ret_db = db_list[i];
		        _DtSvcProcessUnlock();

			return(ret_db);
		}
	}
	_DtSvcProcessUnlock();
	return(NULL);
}

void
_DtDtsDbFieldSort(DtDtsDbRecord *rec, _DtDtsDbFieldCompare compare)
{
	if(compare == NULL)
	{
		compare = _DtDtsDbCompareFieldNames;
	}
	qsort(rec->fieldList,
		rec->fieldCount,
		sizeof(DtDtsDbField *),
		(genfunc)compare);
	rec->compare = compare;
}

void
_DtDtsDbRecordSort(DtDtsDbDatabase *db, _DtDtsDbRecordCompare compare)
{
	if(compare == NULL)
	{
		compare = _DtDtsDbCompareRecordNames;
	}
	qsort(db->recordList,
		db->recordCount,
		sizeof(DtDtsDbRecord *),
		(genfunc)compare);
	db->compare = compare;
	/* "First record of a name" has changed. */
	name_index_drop(db);
}

DtDtsDbField *
_DtDtsDbGetField(DtDtsDbRecord *rec, char *name)
{
	int i;

	/*
	 * Field names have been quarked so quark 'name' and
	 * do a linear search for the quark'ed field name.
	 */
	XrmQuark	tmp = XrmStringToQuark (name);

	for (i = 0; i < rec->fieldCount; i++)
	{
		if (rec->fieldList[i]->fieldName == tmp)
		{
			return (rec->fieldList[i]);
		}
	}
	return(NULL);
}

char *
_DtDtsDbGetFieldByName(DtDtsDbRecord *rec, char *name)
{
	DtDtsDbField	*result;

	result = _DtDtsDbGetField(rec, name);
	if(result)
	{
		return(result->fieldValue);
	}
	else
	{
		return(NULL);
	}

}

DtDtsDbRecord *
_DtDtsDbGetRecordByName(DtDtsDbDatabase *db, char *name)
{
	DtDtsDbRecord	srch;
	DtDtsDbRecord	**result;
	DtDtsDbRecord	*s = &srch;
	int i;
	XrmQuark 	name_quark = XrmStringToQuark(name);

	/*
	 * If the fields are not sorted in alphanumeric order
	 * by name a binary search will fail.  So find the first record
	 * of that name, through the name index (the converters do this
	 * for every record they add, which used to make building the
	 * database quadratic), or, failing that, by a linear search.
	 */
	if(db->compare != _DtDtsDbCompareRecordNames)
	{
		struct _DtDtsDbNameIndex	*ix;

		if (name_quark == NULLQUARK)
			return NULL;	/* no record has this name */
		if ((ix = name_index_update(db)))
		{
			unsigned int	j = name_hash(name_quark) & ix->mask;

			while (ix->slot[j].name != NULLQUARK)
			{
				if (ix->slot[j].name == name_quark)
					return ix->slot[j].rec;
				j = (j + 1) & ix->mask;
			}
			return NULL;
		}

		for (i = 0; i < db->recordCount; i++)
		{
			if (db->recordList[i]->recordName == name_quark)
			{
				return (db->recordList[i]);
			}
		}
		return NULL;
	}

	srch.recordName = name_quark;	

	if(db->recordCount == 0 || db->recordList == NULL)
	{
		result = NULL;
	}
	else
	{
		result = (DtDtsDbRecord **)bsearch(&s,
			db->recordList,
			db->recordCount,
			sizeof(DtDtsDbRecord *),
			(genfunc)_DtDtsDbCompareRecordNames);
	}

	if(result)
	{
		return(*result);
	}
	else
	{
		return(NULL);
	}

}

DtDtsDbRecord *
_DtDtsDbAddRecord(DtDtsDbDatabase *db)
{
	int		rec = db->recordCount;

	if (db->compare != (_DtDtsDbRecordCompare)NULL)
	{
		/* Sorted until now: the name index was not kept. */
		name_index_drop(db);
	}
	db->compare = (_DtDtsDbRecordCompare)NULL;
	db->recordList = grow_list(db->recordList, rec,
				   sizeof(DtDtsDbRecord *));
	db->recordList[rec] = (DtDtsDbRecord *)calloc(1, sizeof(DtDtsDbRecord));
	db->recordCount++;

	return(db->recordList[rec]);
}

int
_DtDtsDbDeleteRecord(DtDtsDbRecord *rec, DtDtsDbDatabase *db)
{
	int	i;

	/* Records after it move, and the first record of its name may change. */
	name_index_drop(db);
	_DtDtsDbDeleteFields(rec);
	free(rec);

	for(i = 0; i < db->recordCount; i++)
	{
		if(db->recordList[i] == rec)
		{
			memmove(	&(db->recordList[i]),
				&(db->recordList[i+1]),
				(db->recordCount - i - 1)*
					sizeof(DtDtsDbRecord *));
			db->recordCount--;
			return(1);
		}
	}

	return(0);
}

int
_DtDtsDbDeleteRecords(DtDtsDbDatabase *db)
{
	int	i;

	for(i = 0; i < db->recordCount; i++)
	{
		_DtDtsDbDeleteFields(db->recordList[i]);
		free(db->recordList[i]);
	}
	free(db->recordList);
	db->recordList = 0;
	db->recordCount = 0;
	name_index_drop(db);
	return(0);
}

DtDtsDbField *
_DtDtsDbAddField(DtDtsDbRecord *rec)
{
	int		flds = rec->fieldCount;

	rec->fieldList = grow_list(rec->fieldList, flds,
				   sizeof(DtDtsDbField *));
	rec->fieldList[flds] = (DtDtsDbField *)calloc(1, sizeof(DtDtsDbField));
	rec->fieldCount++;

	return(rec->fieldList[flds]);
}

int
_DtDtsDbDeleteField(DtDtsDbField *fld, DtDtsDbRecord *rec)
{
	int	i;

	free(fld);
	for(i = 0; i < rec->fieldCount; i++)
	{
		if(rec->fieldList[i] == fld)
		{
			memmove(	&(rec->fieldList[i]),
				&(rec->fieldList[i+1]),
				(rec->fieldCount - i - 1)*
					sizeof(DtDtsDbField *));
			rec->fieldCount--;
			return(1);
		}
	}

	return(0);
}

int
_DtDtsDbDeleteFields(DtDtsDbRecord *rec)
{
	int	i;

	for(i = 0; i < rec->fieldCount; i++)
	{
		free(rec->fieldList[i]->fieldValue);
		free(rec->fieldList[i]);
	}
	free(rec->fieldList);
	rec->fieldList = 0;
	rec->fieldCount = 0;
	return(0);
}

/*
 * Writes the database built in memory to a cache file: CacheFile, or a
 * private (unlinked) file if CacheFile is NULL or, with fallback set, if
 * CacheFile cannot be replaced.  Returns the file, open, or -1.
 */
int
_DtDtsMMCreateFile(DtDirPaths *dirs, const char *CacheFile, int fallback)
{
	return _MMWriteDb(dirs, num_db, db_list, CacheFile, fallback);
}
