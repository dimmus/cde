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
 *+SNOTICE
 *
 *	$TOG: MsgHndArray.C /main/6 1998/09/02 15:54:28 mgreess $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>

#include "MsgHndArray.hh"
#include "MemUtils.hh"

MsgStruct*
MsgHndArray::at(int indx)
{
    return(_contents[indx]);
}

void
MsgHndArray::clear()
{
    memset(_contents, 0, sizeof(MsgStruct *)*_size);
}

int
MsgHndArray::length()
{
    return(_length);
}


int
MsgHndArray::indexof(MsgStruct *a_msg_struct)
{
    int tmp;

    for (tmp = 0; tmp < _length; tmp++)
      {
	  if ((_contents[tmp]->sessionNumber == a_msg_struct->sessionNumber) &&
	      (_contents[tmp]->message_handle == a_msg_struct->message_handle))
	    {
		return(tmp);
	    }
      }
    return(-1);
}

int
MsgHndArray::indexof(DtMailMessageHandle a_message_handle)
{
    int tmp;

    for (tmp = 0; tmp < _length; tmp++)
      {
	  if (_contents[tmp]->message_handle == a_message_handle)
	    {
		return(tmp);
	    }
      }
    return(-1);
}
    
void
MsgHndArray::remove_entry(MsgStruct *ms)
{
    for (int i=0; i<_length; i++)
    {
	if (ms == _contents[i]) 
	{
	    remove_entry(i);
	    return;
	}
    }
}


void
MsgHndArray::remove_entry(int position)
{
    int i;

    if ((position < 0) || (position >= _length)) return;

    for (i=position; i<(_length-1); i++)
      _contents[i] = _contents[i+1];

    _length -= 1;
    _contents[_length] = NULL;
    
}


int
MsgHndArray::insert(
    MsgStruct *a_msg_struct
)
{
    int i, pos;
    int sess_num = a_msg_struct->sessionNumber;

    // Insert before the first entry with a higher session number, or
    // at the end.
    for (pos = 0; pos < _length; pos++)
      if (_contents[pos]->sessionNumber > sess_num)
	break;

    // Keep room for one more entry than we hold (append() and
    // remove_entry() rely on it). Inserting at the end used to skip
    // this, so the next insert or append could write past the array.
    if (_length + 1 >= _size) {
	int orig_size = _size;
	_size += (_size >> 2) + 2;
	_contents = (MsgStruct **)realloc(_contents, 
					  _size * sizeof(MsgStruct *));
	// Zero only the part that was added.
	memset(_contents+orig_size, 0,
	       sizeof(MsgStruct *)*(_size - orig_size));
    }

    for (i = _length; i > pos; i--)
      _contents[i] = _contents[i-1];
    _contents[pos] = a_msg_struct;
    _length++;

    return(pos);
}

void
MsgHndArray::append(
    MsgStruct *a_msg_struct
)

{

    _contents[_length] = a_msg_struct;
    _length++;

    // If we hit size, then grow by 25% to allow more entries.

    if (_length == _size) {
	_size += (_size >> 2) + 1;
	_contents = (MsgStruct **)realloc(_contents, 
					 _size * sizeof(MsgStruct *));
    }

}

void
MsgHndArray::mark_for_delete(int indx)
{
    _contents[indx]->is_deleted = TRUE;
}

void
MsgHndArray::compact(
    int start_pos
)
{
    int i, kept;

    if ((_length <= 0) || (start_pos < 0) || (start_pos >= _length))
	return;

    // Drop every entry from start_pos on that is marked for delete,
    // keeping the order of the rest, in one pass. (This used to remove
    // them one at a time and recurse after each, which was O(N*k) and
    // k levels deep.)
    for (i = kept = start_pos; i < _length; i++) {
	MsgStruct *tmpMS = _contents[i];
	if (tmpMS->is_deleted) {
	    tmpMS->is_deleted = FALSE;
	    continue;
	}
	_contents[kept++] = tmpMS;
    }

    for (i = kept; i < _length; i++)
      _contents[i] = NULL;
    _length = kept;
}

    
//
// Replace the MsgStruct at 'position' with the passed MsgStruct.
// The MsgStruct previously located at 'position' is *not* destroyed.
//
void
MsgHndArray::replace(
	int position,
	MsgStruct *a_msg_struct
)
{
    if (position < 0 || position >= _length) {
	return;
    }

    _contents[position] = a_msg_struct;

    return;
}

//
// Same result as calling insert() for each entry of 'others' in order,
// without shifting the array once per entry. insert() puts an entry
// before the first one with a higher session number, so as long as
// this array is in session-number order (it normally is), the result
// is the two arrays merged by session number, ties keeping this
// array's entries first and then 'others' in their order: that is a
// stable sort of the two concatenated. Otherwise fall back to insert().
//
void
MsgHndArray::insert_all(MsgHndArray *others)
{
    int i, n;

    if (others == NULL || (n = others->length()) == 0)
      return;

    for (i = 1; i < _length; i++)
      if (_contents[i-1]->sessionNumber > _contents[i]->sessionNumber)
	break;

    if (i < _length) {
	for (i = 0; i < n; i++)
	  insert(others->at(i));
	return;
    }

    int total = _length + n;
    std::vector<MsgStruct *> all;
    all.reserve(total);
    all.insert(all.end(), _contents, _contents + _length);
    for (i = 0; i < n; i++)
      all.push_back(others->at(i));

    std::stable_sort(all.begin(), all.end(),
		     [](const MsgStruct *a, const MsgStruct *b) {
			 return a->sessionNumber < b->sessionNumber;
		     });

    if (total >= _size) {
	int orig_size = _size;
	_size = total + (total >> 2) + 1;
	_contents = (MsgStruct **)realloc(_contents,
					  _size * sizeof(MsgStruct *));
	memset(_contents + orig_size, 0,
	       sizeof(MsgStruct *) * (_size - orig_size));
    }
    std::copy(all.begin(), all.end(), _contents);
    _length = total;
}

namespace {
struct MsgKeyHash {
    size_t operator()(const std::pair<DtMailMessageHandle, int> &k) const {
	return std::hash<DtMailMessageHandle>()(k.first) * 31 +
	       std::hash<int>()(k.second);
    }
};
}

//
// Same result as removing each entry of 'others' in turn (each removes
// the first remaining match), in a single pass over the array.
//
void
MsgHndArray::remove_all(MsgHndArray *others, Boolean by_identity)
{
    int i, kept, n;

    if (others == NULL || (n = others->length()) == 0 || _length == 0)
      return;

    if (by_identity) {
	std::unordered_map<MsgStruct *, int> pending;
	for (i = 0; i < n; i++)
	  pending[others->at(i)]++;

	for (i = kept = 0; i < _length; i++) {
	    auto it = pending.find(_contents[i]);
	    if (it != pending.end() && it->second > 0) {
		it->second--;
		continue;
	    }
	    _contents[kept++] = _contents[i];
	}
    }
    else {
	typedef std::pair<DtMailMessageHandle, int> Key;
	std::unordered_map<Key, int, MsgKeyHash> pending;
	for (i = 0; i < n; i++) {
	    MsgStruct *ms = others->at(i);
	    pending[Key(ms->message_handle, ms->sessionNumber)]++;
	}

	for (i = kept = 0; i < _length; i++) {
	    MsgStruct *ms = _contents[i];
	    auto it = pending.find(Key(ms->message_handle, ms->sessionNumber));
	    if (it != pending.end() && it->second > 0) {
		it->second--;
		continue;
	    }
	    _contents[kept++] = ms;
	}
    }

    for (i = kept; i < _length; i++)
      _contents[i] = NULL;
    _length = kept;
}
