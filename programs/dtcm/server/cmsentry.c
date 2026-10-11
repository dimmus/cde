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
/* $XConsortium: cmsentry.c /main/4 1995/11/09 12:41:44 rswiston $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#include <EUSCompat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmsentry.h"
#include "cmsdata.h"
#include "nametbl.h"
#include "attr.h"

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
static CSA_return_code
_ExtractEntryAttrsFromEntry(uint srcsize, cms_attribute *srcattrs,
	uint *dstsize, cms_attribute **dstattrs, boolean_t time_only);

/*****************************************************************************
 * extern functions
 *****************************************************************************/

/*
 * Given a hashed name table, initialize a new cms_entry with
 * the given list of attribute values
 */
extern CSA_return_code
_DtCmsMakeHashedEntry(
	_DtCmsCalendar	*cal,
	uint		num,
	cms_attribute	*attrs,
	cms_entry	**entry)
{
	int		i, index;
	cms_entry	*eptr;
	CSA_return_code	stat = CSA_SUCCESS;

	if (cal == NULL || entry == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((eptr = _DtCm_make_cms_entry(cal->entry_tbl)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0; i < num && stat == CSA_SUCCESS; i++) {
		index = _DtCm_get_index_from_table(cal->entry_tbl,
			attrs[i].name.name);

		if (index > 0) {
			/* check type */
			if (index > _DtCM_DEFINED_ENTRY_ATTR_SIZE &&
			    attrs[i].value &&
			    attrs[i].value->type != cal->types[index])
				stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			else
				stat = _DtCm_copy_cms_attr_val(attrs[i].value,
					&eptr->attrs[index].value);
		} else if (attrs[i].value) {
			if ((stat = _DtCmExtendNameTable(attrs[i].name.name, 0,
			    attrs[i].value->type, _DtCm_entry_name_tbl,
			    _DtCM_DEFINED_ENTRY_ATTR_SIZE,
			    _CSA_entry_attribute_names, &cal->entry_tbl,
			    &cal->types)) == CSA_SUCCESS) {

				attrs[i].name.num = cal->entry_tbl->size;

				stat = _DtCmGrowAttrArray(&eptr->num_attrs,
					&eptr->attrs, &attrs[i]);
			}
		}
	}

	if (stat != CSA_SUCCESS) {
		_DtCm_free_cms_entry(eptr);
		return (stat);
	}

	*entry = eptr;
	return (CSA_SUCCESS);
}

extern void
_DtCmsFreeEntryAttrResItem(cms_get_entry_attr_res_item *elist)
{
	cms_get_entry_attr_res_item *ptr;

	while (elist) {
		ptr = elist->next;

		_DtCm_free_cms_attributes(elist->num_attrs, elist->attrs);
		free(elist);

		elist = ptr;
	}
}

extern CSA_return_code
_DtCmsGetCmsEntryForClient(cms_entry *e, cms_entry **e_r, boolean_t time_only)
{
	cms_entry *ptr;
	CSA_return_code stat;

	if (e == NULL || e_r == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((ptr = (cms_entry *)calloc(1, sizeof(cms_entry))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if ((stat = _ExtractEntryAttrsFromEntry(e->num_attrs, e->attrs,
	    &ptr->num_attrs, &ptr->attrs,time_only)) != CSA_SUCCESS) {
		free(ptr);
		return (stat);
	} else {
		ptr->key = e->key;
		*e_r = ptr;
		return (CSA_SUCCESS);
	}
}

/*
 * What _DtCmsGetCmsEntryForClient() returns, for a reply that is encoded
 * right away and then freed: the entry and its attribute array in one
 * allocation, with the names and values of 'e' itself rather than copies
 * of them.  Only what differs from 'e' is kept in the block: the start
 * and end dates ('start' and 'end', for an instance of a repeating
 * entry; else those of 'e'), the empty string the copy makes of a NULL
 * string, and the emptied summary of a time-only entry.  'e' must not be
 * changed or freed before the reply is sent; free the list with
 * _DtCmsFreeReplyEntries(), which frees the blocks only.
 *
 * A reply of 30,000 instances is 30,000 allocations instead of about
 * 900,000 (and as many frees), with the same bytes on the wire.
 */
extern CSA_return_code
_DtCmsGetCmsEntryForReply(
	cms_entry	*e,
	char		*start,
	char		*end,
	boolean_t	time_only,
	cms_entry	**e_r)
{
	static char		empty[1];
	cms_entry		*ptr;
	cms_attribute		*attrs, *src;
	cms_attribute_value	*vals;
	char			*strs;
	size_t			nattrs = 0, nvals = 0, nstrs = 0, size;
	int			i, j, k;

	if (e == NULL || e_r == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if (start == NULL &&
	    e->attrs[CSA_ENTRY_ATTR_START_DATE_I].value != NULL)
		start = e->attrs[CSA_ENTRY_ATTR_START_DATE_I].value->
			item.date_time_value;
	if (end == NULL && e->attrs[CSA_ENTRY_ATTR_END_DATE_I].value != NULL)
		end = e->attrs[CSA_ENTRY_ATTR_END_DATE_I].value->
			item.date_time_value;

	/* size the block; refuse what _DtCm_copy_cms_attr_val() refuses */
	for (i = 0; i <= e->num_attrs; i++) {
		cms_attribute_value *v = e->attrs[i].value;

		if (v == NULL)
			continue;
		nattrs++;
		switch (v->type) {
		case CSA_VALUE_STRING:
		case CSA_VALUE_DATE_TIME:
		case CSA_VALUE_DATE_TIME_RANGE:
		case CSA_VALUE_TIME_DURATION:
		case CSA_VALUE_CALENDAR_USER:
			if (i == CSA_ENTRY_ATTR_START_DATE_I && start) {
				nvals++;
				nstrs += strlen(start) + 1;
			} else if (i == CSA_ENTRY_ATTR_END_DATE_I && end) {
				nvals++;
				nstrs += strlen(end) + 1;
			} else if (v->item.string_value == NULL ||
			    (time_only && i == CSA_ENTRY_ATTR_SUMMARY_I &&
			     v->item.string_value[0] != '\0'))
				nvals++;
			break;
		case CSA_VALUE_REMINDER:
			if (v->item.reminder_value == NULL)
				return (CSA_E_INVALID_ATTRIBUTE_VALUE);
			break;
		case CSA_VALUE_BOOLEAN:
		case CSA_VALUE_ENUMERATED:
		case CSA_VALUE_FLAGS:
		case CSA_VALUE_UINT32:
		case CSA_VALUE_SINT32:
		case CSA_VALUE_ACCESS_LIST:
		case CSA_VALUE_DATE_TIME_LIST:
		case CSA_VALUE_OPAQUE_DATA:
			break;
		default:
			return (CSA_E_INVALID_ATTRIBUTE_VALUE);
		}
	}

	/*
	 * One spare zeroed attribute after the last: the v4 conversion
	 * (_DtCmsCmsentriesToAppt4ForClient) reads num_attrs + 1 of them,
	 * which in the copy is the calloc()ed slack.
	 */
	size = sizeof(cms_entry) + (nattrs + 1) * sizeof(cms_attribute) +
		nvals * sizeof(cms_attribute_value) + nstrs;
	if ((ptr = (cms_entry *)malloc(size)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);
	attrs = (cms_attribute *)(ptr + 1);
	vals = (cms_attribute_value *)(attrs + nattrs + 1);
	strs = (char *)(vals + nvals);
	memset(&attrs[nattrs], 0, sizeof(cms_attribute));

	ptr->key = e->key;
	ptr->next = NULL;
	ptr->num_attrs = nattrs;
	ptr->attrs = nattrs ? attrs : NULL;

	for (i = 0, j = 0, k = 0; i <= e->num_attrs; i++) {
		char *s = NULL;

		src = &e->attrs[i];
		if (src->value == NULL)
			continue;
		attrs[j] = *src;
		switch (src->value->type) {
		case CSA_VALUE_STRING:
		case CSA_VALUE_DATE_TIME:
		case CSA_VALUE_DATE_TIME_RANGE:
		case CSA_VALUE_TIME_DURATION:
		case CSA_VALUE_CALENDAR_USER:
			if (i == CSA_ENTRY_ATTR_START_DATE_I && start) {
				s = strcpy(strs, start);
				strs += strlen(start) + 1;
			} else if (i == CSA_ENTRY_ATTR_END_DATE_I && end) {
				s = strcpy(strs, end);
				strs += strlen(end) + 1;
			} else if (src->value->item.string_value == NULL ||
			    (time_only && i == CSA_ENTRY_ATTR_SUMMARY_I &&
			     src->value->item.string_value[0] != '\0'))
				s = empty;
			if (s != NULL) {
				vals[k].type = src->value->type;
				vals[k].item.string_value = s;
				attrs[j].value = &vals[k++];
			}
			break;
		default:
			break;
		}
		j++;
	}

	*e_r = ptr;
	return (CSA_SUCCESS);
}

extern void
_DtCmsFreeReplyEntries(cms_entry *entries)
{
	cms_entry *next;

	for (; entries != NULL; entries = next) {
		next = entries->next;
		free(entries);
	}
}

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

static CSA_return_code
_ExtractEntryAttrsFromEntry(
	uint		srcsize,
	cms_attribute	*srcattrs,
	uint		*dstsize,
	cms_attribute	**dstattrs,
	boolean_t	time_only)
{
	CSA_return_code	stat = CSA_SUCCESS;
	int		i, j;
	cms_attribute	*attrs;

	if (dstsize == NULL || dstattrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	*dstsize = 0;
	*dstattrs = NULL;

	if (srcsize == 0)
		return (CSA_SUCCESS);

	if ((attrs = calloc(1, sizeof(cms_attribute) * srcsize)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0, j = 0; i <= srcsize; i++) {
		if (srcattrs[i].value == NULL)
			continue;

		if ((stat = _DtCm_copy_cms_attribute(&attrs[j], &srcattrs[i],
		    B_TRUE)) != CSA_SUCCESS)
			break;
		else {
			if ( (i == CSA_ENTRY_ATTR_SUMMARY_I) && (time_only == B_TRUE) ) {
				if (attrs[j].value && attrs[j].value->item.string_value)
					attrs[j].value->item.string_value[0] = '\0';
			}

			j++;
		}
	}

	if (stat == CSA_SUCCESS && j > 0) {
		*dstsize = j;
		*dstattrs = attrs;
	} else {
		_DtCm_free_cms_attributes(j, attrs);
		free(attrs);
	}

	return (stat);
}


