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
/* $XConsortium: PrintSubSys.h /main/3 1995/11/06 09:47:33 rswiston $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef PRINTSUBSYS_H
#define PRINTSUBSYS_H

#include "BaseObj.h"
#include "dtprintinfomsg.h"

// Object Class Name
extern const char *PRINTSUBSYSTEM;

// List Children command;
extern const char *LIST_QUEUES;

// Sorted queue names, one per line; free() the result.  The exit status of
// the listing command is stored in *status when status is not NULL.  If
// device_list is not NULL, *device_list gets a copy of the raw
// "lpstat -v" output (NULL where there is none); free() it.
extern char *ListPrintQueues(int *status = NULL, char **device_list = NULL);

class PrintSubSystem : public BaseObj {

 protected:

   char *_device_list;

   void InitChildren();

 public:

   // "lpstat -v" output for all queues, read when the queues were listed,
   // or NULL.  Queue::LoadAttributes() uses it instead of running
   // "lpstat -v QUEUE" for every queue.
   const char *DeviceList() { return _device_list; }

   PrintSubSystem(BaseObj *parent);
   virtual ~PrintSubSystem();

   virtual const char *const ObjectClassName() { return PRINTSUBSYSTEM; }

};

#endif // PRINTSUBSYS_H
