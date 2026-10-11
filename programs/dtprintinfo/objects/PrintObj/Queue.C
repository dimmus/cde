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
/* $TOG: Queue.C /main/4 1998/07/24 16:17:58 mgreess $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "Queue.h"
#include "PrintJob.h"
#include "ParseJobs.h"
#include "PrintSubSys.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern "C" {
#include <Dt/DtNlUtils.h>
}

#ifdef aix
const char *GET_ATTRS = "lsque -cq%s |awk -F: 'NR == 2 {print $2,$6,$9}' OFS=:";
const char *GET_QUEUE_STATUS = "LANG=C enq -As -P%s | "
			       "egrep 'READY|RUNNING' > /dev/null";
const char *GET_DEVICE_STATUS = "LANG=C enq -As -P%s | "
			        "egrep 'READY|RUNNING' > /dev/null";
const char *START_QUEUE_CMD = "enq -U -P%s";
const char *STOP_QUEUE_CMD = "enq -D -P%s";
#else
const char *GET_ATTRS = "LANG=C lpstat -v %s 2>&1 | nawk '"
			    "BEGIN { device=\"\"; rhost=\"\"; rp=\"\" } "
                            "/device for/ { device = $4 } "
                            "/system for/ { rhost = $4; x = match($7, /\\)/); "
                            "               if (x == 0) "
                            "                  rp = substr($3, 1, match($3, /:/) - 1); "
                            "               else "
                            "                  rp = substr($7, 1, x - 1) } "
                            "END { print device,rhost,rp }' OFS=:";
const char *GET_QUEUE_STATUS = "LANG=C lpstat -a%s | awk '"
			       "{if ($2 == \"not\") {exit 1} else {exit 0}}'";
const char *GET_DEVICE_STATUS = "LANG=C lpstat -p%s | "
			        "awk '/disabled/ {exit 1}'";
const char *START_QUEUE_CMD = "/usr/sbin/accept %s";
const char *STOP_QUEUE_CMD = "/usr/sbin/reject %s";
const char *START_PRINTING_CMD = "enable %s";
const char *STOP_PRINTING_CMD = "disable %s";

#endif

// Object Class Name
const char *QUEUE = "Queue";

// Actions
const char *START_QUEUE            = "StartQueue";
const char *STOP_QUEUE             = "StopQueue";
#ifndef aix
const char *START_PRINTING         = "StartPrinting";
const char *STOP_PRINTING          = "StopPrinting";
#endif

// Attributes
const char *ICON_NAME              = "IconName";
const char *PRINTER_QUEUE          = "PrinterQueue";
const char *QUEUE_DEVICE           = "QueueDevice";

Queue::Queue(BaseObj *parent,
	     char *_name)
	: BaseObj(parent, _name)
{
 if (getenv("DO_ADMIN"))
  {
   AddAction(&Queue::Start, START_QUEUE, MESSAGE(StartChoiceL),
	     MESSAGE(StartMnemonicL));
   AddAction(&Queue::Stop, STOP_QUEUE, MESSAGE(StopChoiceL),
	     MESSAGE(StopMnemonicL));
#ifndef aix
   AddAction(&Queue::StartPrint, START_PRINTING, MESSAGE(EnableChoiceL),
	     MESSAGE(EnableMnemonicL));
   AddAction(&Queue::StopPrint, STOP_PRINTING, MESSAGE(DisableChoiceL),
	     MESSAGE(DisableMnemonicL));
#endif
  }

#ifdef aix
   local_devices = NULL;
   n_devices = 0;
#endif
   remote_server = NULL;
   remote_printer = NULL;
   is_remote = false;
   _loaded_attributes = false;

   char *Help = NULL, *ContextualHelp = NULL, *Listing = NULL;
   Characteristics Mask = EDITABLE_AFTER_CREATE;
   ValueList ValueListType = NO_LIST;

   // AddAttribute(ICON_NAME, MESSAGE(IconNameL),
		// Help, ContextualHelp, Mask, ValueListType, Listing);

   Mask = OPTIONAL;
   AddAttribute(PRINTER_QUEUE, MESSAGE(PrintQueueL),
		Help, ContextualHelp, Mask, ValueListType, Listing);
   AddAttribute(QUEUE_DEVICE, MESSAGE(DeviceL), 
		Help, ContextualHelp, Mask, ValueListType, Listing);
}

Queue::~Queue()
{
#ifdef aix
   int i;
   for (i = 0; i < n_devices; i++)
      delete local_devices[i];
   delete local_devices;
#endif
   delete remote_server;
   delete remote_printer;
}

#ifndef aix
// Split an awk record into its first max_fields fields (in place).
static int SplitFields(char *line, char **field, int max_fields)
{
   int nf = 0;
   while (nf < max_fields)
    {
      while (*line && isspace((unsigned char)*line))
	 line++;
      if (!*line)
	 break;
      field[nf++] = line;
      while (*line && !isspace((unsigned char)*line))
	 line++;
      if (*line)
	 *line++ = '\0';
    }
   return nf;
}

// What the GET_ATTRS command prints for queue name, computed from the
// "lpstat -v" output of all queues instead of from "lpstat -v name".
static char *DeviceAttributes(const char *device_list, const char *name)
{
   char *list = strdup(device_list);
   if (!list)
      return NULL;
   size_t name_len = strlen(name);
   const char *device = "", *rhost = "", *rp = "";
   char *line, *next;
   for (line = list; line && *line; line = next)
    {
      if ((next = strchr(line, '\n')))
	 *next++ = '\0';
      boolean is_device = strstr(line, "device for") ? true : false;
      boolean is_system = strstr(line, "system for") ? true : false;
      if (!is_device && !is_system)
	 continue;
      char *field[7];
      int nf = SplitFields(line, field, 7);
      // "lpstat -v name" lists only name: "device for name: ..."
      if (nf < 3 || strncmp(field[2], name, name_len) ||
	  strcmp(field[2] + name_len, ":"))
	 continue;
      for (; nf < 7; nf++)
	 field[nf] = (char *)"";
      if (is_device)
	 device = field[3];
      if (is_system)
       {
	 rhost = field[3];
	 char *paren = strchr(field[6], ')');
	 if (paren)
	  {
	    *paren = '\0';
	    rp = field[6];
	  }
	 else
	  {
	    char *colon = strchr(field[2], ':');
	    if (colon)
	       *colon = '\0';
	    rp = colon ? field[2] : "";
	  }
       }
    }
   char *output = (char *)malloc(strlen(device) + strlen(rhost) +
				 strlen(rp) + 4);
   if (output)
      sprintf(output, "%s:%s:%s\n", device, rhost, rp);
   free(list);
   return output;
}
#endif

void Queue::LoadAttributes(int /*n_attrs*/, Attribute **attrs)
{
   char *output = NULL;
#ifndef aix
   if (Parent() && !strcmp(Parent()->ObjectClassName(), PRINTSUBSYSTEM))
    {
      const char *device_list = ((PrintSubSystem *)Parent())->DeviceList();
      if (device_list)
	 output = DeviceAttributes(device_list, Name());
    }
#endif
   if (!output)
    {
      char *command = new char[500];
      snprintf(command, 500, GET_ATTRS, Name());
      RunCommand(command, &output);
      delete [] command;
    }

   char *s = output, *s1;
   char *dollar[3];
   int i;
   for (i = 0; i < 3; i++)
    {
      // Missing fields are empty (the output may be empty)
      if (!s)
       {
	 dollar[i] = (char *)"";
	 continue;
       }
      if ((s1 = strchr(s, ':')))
         *s1++ = '\0';
      else if ((s1 = strchr(s, '\n')))
         *s1++ = '\0';
      dollar[i] = s;
      s = s1;
    }
   i = 0;
   attrs[i]->Value = strdup(Name());
   attrs[i]->DisplayValue = strdup(Name());
   i++;
   if (_loaded_attributes == false)
    {
      if (*dollar[2]) // It's a remote printer
       {
#ifdef aix
	 n_devices = 1;
	 local_devices = new char *[1];
	 local_devices[0] = new char[strlen(Name()) + strlen(dollar[0]) + 2];
	 sprintf(local_devices[0], "%s:%s", Name(), dollar[0]);
#endif
	 is_remote = true;
	 char *new_value = new char [strlen(MESSAGE(PrinterOnServerL)) + 
				     strlen(dollar[1]) + strlen(dollar[2])];
         remote_server = strdup(dollar[1]);
         remote_printer = strdup(dollar[2]);
	 sprintf(new_value, MESSAGE(PrinterOnServerL), remote_printer, 
		 remote_server);
         attrs[i]->Value = strdup(new_value);
         attrs[i]->DisplayValue = strdup(new_value);
	 delete [] new_value;
       }
      else // It's a local printer
       {
#ifdef aix
	 if (strchr(dollar[0], ',')) // AIX can have multiple devices per queue
	  {
            DeleteAttribute(QUEUE_DEVICE);
	    char *device = new char [strlen(MESSAGE(DeviceNL)) + 4];
	    s = dollar[0];
            while (s && *s)
             {
               if (s1 = strchr(s, ','))
                  s1++;
               s = s1;
	       n_devices++;
	     }
	    local_devices = new char *[n_devices];
	    n_devices = 0;
	    s = dollar[0];
            while (s && *s)
             {
	       sprintf(device, MESSAGE(DeviceNL), n_devices + 1);
               AddAttribute(QUEUE_DEVICE, device,
		            NULL, NULL, OPTIONAL, NO_LIST, NULL);
               if (s1 = strchr(s, ','))
                  *s1++ = '\0';
               _attributes[i]->Value = strdup(s);
               _attributes[i]->DisplayValue = strdup(s);
	       local_devices[n_devices] = new char[strlen(Name()) + strlen(s)+2];
	       sprintf(local_devices[n_devices], "%s:%s", Name(), s);
	       i++;
               s = s1;
	       n_devices++;
             }
	    delete [] device;
	  }
	 else
#endif
	  {
#ifdef aix
	    n_devices = 1;
	    local_devices = new char *[1];
	    local_devices[0] = new char[strlen(Name()) + strlen(dollar[0]) + 2];
	    sprintf(local_devices[0], "%s:%s", Name(), dollar[0]);

#endif
            attrs[i]->Value = strdup(dollar[0]);
            attrs[i]->DisplayValue = strdup(dollar[0]);
	  }
       }
      _loaded_attributes = true;
    }
   free(output);
}

#ifdef aix
char *Queue::Device(int index)
{
   if (_loaded_attributes == false)
      ReadAttributes();
   if (index >= 0 && index <= n_devices)
      return local_devices[index];
   else
      return NULL;
}

int Queue::NumberDevices()
{
   if (_loaded_attributes == false)
      ReadAttributes();
   return n_devices;
}

#endif

int Queue::Start(BaseObj *obj, char **output, BaseObj * /*requestor*/)
{
   Queue *queue = (Queue *)obj;
   int rc;
   char *command = new char[100];

#ifdef aix
   if (queue->n_devices > 1)
    {
      sprintf(command, START_QUEUE_CMD, "$d");
      int i, len;
      len = 30 + strlen(command) + queue->n_devices;
      for (i = 0; i < queue->n_devices; i++)
	 len += strlen(queue->local_devices[i]);
      char *cmd = new char[len];
      strcpy(cmd, "for d in");
      for (i = 0; i < queue->n_devices; i++)
       {
         strcat(cmd, " ");
         strcat(cmd, queue->local_devices[i]);
       }
      strcat(cmd, " ; do ");
      strcat(cmd, command);
      strcat(cmd, "; done");
      rc = queue->RunCommand(cmd, NULL, output);
      delete [] cmd;
    }
   else
#endif
    {
      sprintf(command, START_QUEUE_CMD, queue->Name());
      rc = queue->RunCommand(command, NULL, output);
    }
    delete [] command;
    return rc;
}

int Queue::Stop(BaseObj *obj, char **output, BaseObj * /*requestor*/)
{
   Queue *queue = (Queue *)obj;
   char *command = new char[100];
   int rc;

#ifdef aix
   if (queue->n_devices > 1)
    {
      sprintf(command, STOP_QUEUE_CMD, "$d");
      int i, len;
      len = 30 + strlen(command) + queue->n_devices;
      for (i = 0; i < queue->n_devices; i++)
	 len += strlen(queue->local_devices[i]);
      char *cmd = new char[len];
      strcpy(cmd, "for d in");
      for (i = 0; i < queue->n_devices; i++)
       {
         strcat(cmd, " ");
         strcat(cmd, queue->local_devices[i]);
       }
      strcat(cmd, " ; do ");
      strcat(cmd, command);
      strcat(cmd, "; done");
      rc = queue->RunCommand(cmd, NULL, output);
      delete [] cmd;
    }
   else
#endif
    {
        snprintf(command, 100, STOP_QUEUE_CMD, queue->Name());
      rc = queue->RunCommand(command, NULL, output);
    }
    delete [] command;
    return rc;
}

#ifndef aix
int Queue::StartPrint(BaseObj *obj, char **output, BaseObj * /*requestor*/)
{
   Queue *queue = (Queue *)obj;
   char *command = new char[100];
   int rc;

   snprintf(command, 100, START_PRINTING_CMD, queue->Name());
   rc = queue->RunCommand(command, NULL, output);
   delete [] command;
   return rc;
}

int Queue::StopPrint(BaseObj *obj, char **output, BaseObj * /*requestor*/)
{
   Queue *queue = (Queue *)obj;
   char *command = new char[100];
   int rc;

   sprintf(command, STOP_PRINTING_CMD, queue->Name());
   rc = queue->RunCommand(command, NULL, output);
   delete [] command;
   return rc;
}
#endif

void Queue::InitChildren()
{
   if (_loaded_attributes == false)
      ReadAttributes();
   ProcessJobs();
}

void Queue::ProcessJobs(char *jobs)
{
   char *job_list;
   int n_jobs;

   // Get remote jobs first
   if (is_remote)
    {
      int rc;
      if (jobs)
	 rc = ParseRemotePrintJobs(remote_printer, jobs, &job_list, &n_jobs);
      else
         rc = RemotePrintJobs(remote_server, remote_printer, &job_list,
			      &n_jobs);
      remote_up = rc ? true : false;

      ParseOutput(job_list, n_jobs);
#ifdef sun
      return;
#endif
    }

   // Get local jobs next
#ifdef aix
   if (is_remote)
      LocalPrintJobs(local_devices[0], &job_list, &n_jobs);
   else
      LocalPrintJobs((char*)Name(), &job_list, &n_jobs);
#else
   LocalPrintJobs((char*)Name(), &job_list, &n_jobs);
#endif
   ParseOutput(job_list, n_jobs);
}

void Queue::ParseOutput(char *job_list, int n_jobs)
{
   int i;
   DtStrtok(job_list, "|");
   for (i = 0; i < n_jobs; i++)
    {
      char *JobName = DtStrtok(NULL, "|");
      char *JobNumber = DtStrtok(NULL, "|");
      char *Owner = DtStrtok(NULL, "|");
      char *Date = DtStrtok(NULL, "|");
      char *Time = DtStrtok(NULL, "|");
      char *tmp = DtStrtok(NULL, "\n");
      char *Size = new char [strlen(tmp) + strlen(MESSAGE(BytesL)) + 2];
      sprintf(Size, "%s %s", tmp, MESSAGE(BytesL));

      new PrintJob(this, JobName, JobNumber, Owner, Date, Time, Size);
      delete [] Size;
      DtStrtok(NULL, "|");
    }
}

#ifdef HAVE_LOCAL_PRINT_JOBS_COMMAND
void Queue::ParseLocalStatus(char *output)
{
   SetInitChildren();
   DeleteChildren();
   if (_loaded_attributes == false)
      ReadAttributes();

   char *job_list;
   int n_jobs;
   ParseLocalPrintJobs((char *)Name(), output ? output : (char *)"",
		       &job_list, &n_jobs);
   ParseOutput(job_list, n_jobs);
}
#endif

void Queue::ParseRemoteStatus(char *output)
{
   SetInitChildren();
   DeleteChildren();
   if (_loaded_attributes == false)
      ReadAttributes();
   ProcessJobs(output);
}

boolean Queue::IsRemote()
{
   if (_loaded_attributes == false)
      ReadAttributes();
   return is_remote;
}

const char *Queue::RemotePrinter()
{
   if (_loaded_attributes == false)
      ReadAttributes();
   return remote_printer;
}

const char *Queue::Server()
{
   if (_loaded_attributes == false)
      ReadAttributes();
   return remote_server;
}

#ifndef aix

// One "lpstat -a -p" for the state of all queues (or of the one queue
// name, if not NULL) replaces running GET_QUEUE_STATUS and
// GET_DEVICE_STATUS for every queue.  free() the result.
char *QueueStatusCommand(const char *name)
{
   const char *all = "LANG=C LC_ALL=C lpstat -a -p";
   if (!name)
      return strdup(all);

   // -a'name' -p'name', with any ' in name quoted as '\''
   size_t quotes = 0;
   const char *s;
   for (s = name; *s; s++)
      if (*s == '\'')
	 quotes++;
   size_t qlen = strlen(name) + 3 * quotes + 2;
   char *quoted = (char *)malloc(qlen + 1);
   char *cmd = (char *)malloc(strlen(all) + 2 * qlen + 1);
   if (!quoted || !cmd)
    {
      free(quoted);
      free(cmd);
      return strdup(all);
    }
   char *q = quoted;
   *q++ = '\'';
   for (s = name; *s; s++)
    {
      if (*s == '\'')
       {
	 memcpy(q, "'\\''", 4);
	 q += 4;
       }
      else
	 *q++ = *s;
    }
   *q++ = '\'';
   *q = '\0';
   sprintf(cmd, "LANG=C LC_ALL=C lpstat -a%s -p%s", quoted, quoted);
   free(quoted);
   return cmd;
}

static boolean Word(const char *word, const char *s, size_t len)
{
   return (s && strlen(word) == len && !strncmp(word, s, len)) ? true : false;
}

// Parses the output of QueueStatusCommand().  For every queue it gives the
// result of the old per-queue commands:
//   GET_QUEUE_STATUS   "lpstat -aQ | awk '{if ($2 == "not") {exit 1} ...}'"
//                      down if the second word of Q's first line is "not";
//   GET_DEVICE_STATUS  "lpstat -pQ | awk '/disabled/ {exit 1}'"
//                      down if any line of Q's "printer Q ..." entry
//                      contains "disabled".
// A queue that is not listed is up, as with the old commands.
QueueStatusTable::QueueStatusTable(const char *output)
{
   entries = NULL;
   n_entries = 0;
   _text = strdup(output ? output : "");
   if (!_text)
      return;

   int max_entries = 0;
   int current = -1;     // the "printer Q" entry that continuation lines extend
   char *line, *next;
   for (line = _text; line && *line; line = next)
    {
      if ((next = strchr(line, '\n')))
	 *next++ = '\0';
      if (isspace((unsigned char)*line))
       {
	 // continuation of the previous entry, e.g. "\treason unknown"
	 if (current >= 0 && strstr(line, "disabled"))
	    entries[current].device_up = false;
	 continue;
       }
      current = -1;

      // First three words
      const char *word[3] = { NULL, NULL, NULL };
      size_t len[3] = { 0, 0, 0 };
      const char *s = line;
      int nw;
      for (nw = 0; nw < 3; nw++)
       {
	 while (*s && isspace((unsigned char)*s))
	    s++;
	 if (!*s)
	    break;
	 word[nw] = s;
	 while (*s && !isspace((unsigned char)*s))
	    s++;
	 len[nw] = s - word[nw];
       }
      if (nw == 0)
	 continue;

      // "printer Q is idle.  enabled since ..." / "printer Q disabled ..."
      // rather than the -a lines of queues called "printer":
      // "printer accepting requests ..." / "printer not accepting ..."
      boolean is_printer = (Word("printer", word[0], len[0]) && word[1] &&
	    !(Word("accepting", word[1], len[1]) &&
	      Word("requests", word[2], len[2])) &&
	    !(Word("not", word[1], len[1]) &&
	      Word("accepting", word[2], len[2]))) ? true : false;
      int which = is_printer ? 1 : 0;

      int i;
      for (i = 0; i < n_entries; i++)
	 if (strlen(entries[i].name) == len[which] &&
	     !strncmp(entries[i].name, word[which], len[which]))
	    break;
      if (i == n_entries)
       {
	 if (n_entries == max_entries)
	  {
	    int n = max_entries ? 2 * max_entries : 16;
	    QueueStatusEntry *tmp = (QueueStatusEntry *)
	       realloc(entries, n * sizeof(QueueStatusEntry));
	    if (!tmp)
	       break;
	    entries = tmp;
	    max_entries = n;
	  }
	 // Names point into _text; terminate them there
	 ((char *)word[which])[len[which]] = '\0';
	 entries[i].name = word[which];
	 entries[i].queue_up = true;
	 entries[i].device_up = true;
	 entries[i].have_queue = false;
	 n_entries++;
       }
      if (is_printer)
       {
	 current = i;
	 // The terminator written above may have cut "disabled" off;
	 // test the text after the name as well as before it.
	 if (strstr(word[2] ? word[2] : "", "disabled") ||
	     strstr(line, "disabled"))
	    entries[i].device_up = false;
       }
      else if (entries[i].have_queue == false)
       {
	 entries[i].have_queue = true;
	 entries[i].queue_up = Word("not", word[1], len[1]) ? false : true;
       }
    }
}

QueueStatusTable::~QueueStatusTable()
{
   free(entries);
   free(_text);
}

void QueueStatusTable::Lookup(const char *name, boolean *queue_up,
			      boolean *device_up)
{
   *queue_up = true;
   *device_up = true;
   int i;
   for (i = 0; i < n_entries; i++)
      if (!strcmp(entries[i].name, name))
       {
	 *queue_up = entries[i].queue_up;
	 *device_up = entries[i].device_up;
	 return;
       }
}

#endif // aix
