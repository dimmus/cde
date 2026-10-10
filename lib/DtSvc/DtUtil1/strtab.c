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
/* $XConsortium: strtab.c /main/4 1996/05/09 04:23:54 drk $ */
/*
  routines to implement a string -> unique ptr table & access functions
  suitable for use with shared memory
  */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "DtHash.h"
#include "DtShmDb.h"

/*
  The table as stored (in the dtdbcache file, see DTDTSMM_VERSION in
  DtsMM.h): this header, the strings (a boson is the offset of a string
  from the start of the strings), the entries (one per string), and the
  buckets.  A bucket holds the index of the first entry whose string
  hashes to it, an entry the index of the next one.
  */
typedef struct strtab {
  unsigned int st_size;		/* size in bytes						*/
  unsigned int st_stroffset;	/* offset (bytes) from this structure to contained strings 	*/
  unsigned int st_taboffset;	/* offset (bytes) from this structure to the entries            */
  unsigned int st_count;	/* number of elements in this string table			*/
  unsigned int st_bucketoffset;	/* offset (bytes) from this structure to the buckets		*/
  unsigned int st_nbuckets;	/* number of buckets, a power of two				*/
} strtab_t;

typedef struct strtab_entry {
  unsigned int key;		/* offset into contained strings				*/
  unsigned int next;		/* index of next entry to hash to the same bucket		*/
} strtab_entry_t;

typedef struct strlist {
  DtHashTbl 		sl_hash;	/* hash table to hold strings during definition		*/
  DtHashTbl		sl_bosons;	/* reverse hash table for Nolan's lookup function	*/		
  unsigned int    	sl_charcount;	/* count of current bytes of strings			*/
} strlist_t;

struct strtab_build {
  unsigned int	   	index;			/* iteration 			*/
  unsigned char *  	strstart;		/* start of strings		*/
  strtab_entry_t * 	tabstart;		/* start of table		*/	
  strtab_t * 		strtab;			/* pointer to head        	*/
};

#define NOT_AN_INDEX ((unsigned int) 0xffffffff)

/* FNV-1a */
static unsigned int
strtab_hash(const unsigned char * s)
{
  unsigned int h = 2166136261u;

  while(*s)
    h = (h ^ *s++) * 16777619u;
  return(h);
}

/* Buckets for count strings: a power of two, at least twice count. */
static unsigned int
strtab_nbuckets(unsigned int count)
{
  unsigned int n = 1;

  while(n < 2 * count)
    n *= 2;
  return(n);
}

struct strtab_layout {
  unsigned int count;		/* number of strings		*/
  unsigned int stroffset;
  unsigned int taboffset;
  unsigned int bucketoffset;
  unsigned int nbuckets;
  unsigned int size;
};

static void strtab_layout(DtShmProtoStrtab strlist, struct strtab_layout * l);

static void inc_it  (int * a, int * b, unsigned char * key);
static void build_it(int a, struct strtab_build * ptr, unsigned char * key);
#if !defined(__linux__)
extern char * strdup(const char *);
#endif
typedef	int (*des_func)(void *);

/*
  client routine
  
  returns offset in string table where string is found,
  else -1 if string is not found.
  */

const char *    _DtShmBosonToString(DtShmStrtab strtab, int boson)
{
  const char * s = (const char *) strtab + ((strtab_t *) strtab)->st_stroffset;
  return(s+boson);
}


DtShmBoson _DtShmStringToBoson(DtShmStrtab strtab, const char * string)
{
  const strtab_t * head = (const strtab_t *) strtab;
  const strtab_entry_t * ptr = (const strtab_entry_t *)
    ((const unsigned char *) strtab + head->st_taboffset);
  const unsigned int * bucket = (const unsigned int *)
    ((const unsigned char *) strtab + head->st_bucketoffset);
  const char * s = (const char *) strtab + head->st_stroffset;
  unsigned int i;

  i = bucket[strtab_hash((const unsigned char *) string) &
	     (head->st_nbuckets - 1)];

  while(i != NOT_AN_INDEX) {
    if(strcmp(s + ptr[i].key, string) == 0)
      return(ptr[i].key);
    i = ptr[i].next;
  }
  return(-1);
}



DtShmProtoStrtab
_DtShmProtoInitStrtab(int sizeguess)
{
  strlist_t * ptr = (strlist_t *) malloc(sizeof(*ptr));
  if(!ptr)
    return(NULL);

  if(!(ptr->sl_hash =  _DtUtilMakeHash(sizeguess))) {
    free(ptr);
    return(NULL);
  }

  if(!(ptr->sl_bosons =  _DtUtilMakeIHash(sizeguess))) {
    (void)_DtUtilDestroyHash(ptr->sl_hash, NULL, NULL);
    free(ptr);
    return(NULL);
  }

  ptr->sl_charcount = 1;
  return((DtShmProtoStrtab) ptr);
}

int 
_DtShmProtoDestroyStrtab(DtShmProtoStrtab strlist)
{
  strlist_t * ptr = (strlist_t *) strlist;
  
  _DtUtilDestroyHash(ptr->sl_hash, NULL, NULL);
  _DtUtilDestroyHash(ptr->sl_bosons, (des_func)free, NULL);
  free(ptr);
  return(0);
}

DtShmBoson
_DtShmProtoAddStrtab(DtShmProtoStrtab strlist, const char * string, int * isnew)
{
  strlist_t * ptr = (strlist_t *) strlist;

  intptr_t * bucket = (intptr_t *) _DtUtilGetHash(ptr->sl_hash, (const unsigned char *)string);
  
  intptr_t ret = *bucket;

  if(*bucket == 0) /* new */ {
    unsigned char ** sptr;
    *isnew = 1;
    *bucket = ret = ptr->sl_charcount;
    sptr = (unsigned char**)_DtUtilGetHash(ptr->sl_bosons, (const unsigned char *)ret);
    *sptr = (unsigned char*)strdup(string);
    ptr->sl_charcount += strlen(string) + 1;
  } 

  else {
    *isnew = 0;
  }
    

  return((DtShmBoson)ret);
}

const char *		
_DtShmProtoLookUpStrtab (DtShmProtoStrtab prototab, DtShmBoson boson)
{
  strlist_t * ptr = (strlist_t *) prototab;
  unsigned char ** sptr;
  
  sptr = (unsigned char **) _DtUtilFindHash(ptr->sl_bosons, (const unsigned char *) (intptr_t) boson);

  return(sptr?((const char *)*sptr):NULL);
}


static void
strtab_layout(DtShmProtoStrtab in, struct strtab_layout * l)
{
  strlist_t * strlist = (strlist_t *) in;
  int foo[2];

  foo[1] = foo[0] = 0;
  _DtUtilOperateHash(strlist->sl_hash, inc_it, &foo);

  /* The strings take sl_charcount bytes: offset 0 is not a boson. */
  l->count = foo[0];
  l->stroffset = sizeof(strtab_t);
  l->taboffset = (l->stroffset + strlist->sl_charcount + 3) & ~3u;
  l->bucketoffset = l->taboffset + l->count * sizeof(strtab_entry_t);
  l->nbuckets = strtab_nbuckets(l->count);
  l->size = l->bucketoffset + l->nbuckets * sizeof(unsigned int);
}

int 
_DtShmProtoSizeStrtab(DtShmProtoStrtab strlist)
{
  struct strtab_layout l;

  strtab_layout(strlist, &l);
  return(l.size);		/* a multiple of 4 */
}

DtShmStrtab 
_DtShmProtoCopyStrtab(DtShmProtoStrtab in, void * destination)
{
  strlist_t * 	strlist = (strlist_t *) in;
  strtab_t * 	ptr = (strtab_t *) destination;
  struct strtab_layout l;
  struct strtab_build building;

  strtab_layout(in, &l);

  memset((char *) ptr, 0, l.taboffset);
  memset((char *) ptr + l.taboffset, 255, l.size - l.taboffset);

  ptr-> st_size = l.size;
  ptr-> st_stroffset = l.stroffset;
  ptr-> st_taboffset = l.taboffset;
  ptr-> st_count = l.count;
  ptr-> st_bucketoffset = l.bucketoffset;
  ptr-> st_nbuckets = l.nbuckets;

  building.index = 0;
  building.strstart = (unsigned char *) ptr  + ptr->st_stroffset;
  building.tabstart = (strtab_entry_t *) ((unsigned char *) ptr  + ptr->st_taboffset);
  building.strtab = ptr;

  _DtUtilOperateHash(strlist->sl_hash, build_it, & building);
  return((DtShmStrtab) destination);
}

static void build_it(int a, struct strtab_build * ptr, unsigned char * key)
{
  strtab_t * head = ptr->strtab;
  unsigned int * bucket = (unsigned int *)
    ((unsigned char *) head + head->st_bucketoffset);
  unsigned int b;
  strtab_entry_t * e;

  strcpy((char *) ptr->strstart + a, (const char *)key);

  b = strtab_hash(key) & (head->st_nbuckets - 1);

  e = ptr->tabstart + ptr->index;
  e -> key = a; /* save key value in our block */
  e -> next = bucket[b];
  bucket[b] = ptr->index++;
}

/* ARGSUSED */
static void inc_it(int * a, int * b, unsigned char * key)
{
  b[0]++;
  b[1] += strlen((const char *)key) + 1;
}
