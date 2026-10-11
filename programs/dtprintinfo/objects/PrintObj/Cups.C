/*
 * CDE - Common Desktop Environment
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

// See Cups.h.

#include "Cups.h"

#ifdef DTPRINTINFO_CUPS

#include <cups/cups.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <iconv.h>
#include <langinfo.h>
#include <strings.h>
#include <wchar.h>

static int cups_in_use = 0;

int CupsInUse()
{
   return cups_in_use;
}

// A growing string
struct Text
{
   char *s;
   size_t len, size;
};

static void TextAdd(Text *t, const char *fmt, ...)
{
   va_list ap;
   if (!t->s)
    {
      t->size = 256;
      if (!(t->s = (char *)malloc(t->size)))
	 return;
      t->len = 0;
      t->s[0] = '\0';
    }
   for (;;)
    {
      va_start(ap, fmt);
      int n = vsnprintf(t->s + t->len, t->size - t->len, fmt, ap);
      va_end(ap);
      if (n < 0)
	 return;
      if ((size_t)n < t->size - t->len)
       {
	 t->len += n;
	 return;
       }
      char *s = (char *)realloc(t->s, 2 * t->size + n);
      if (!s)
       {
	 t->s[t->len] = '\0';
	 return;
       }
      t->s = s;
      t->size = 2 * t->size + n;
    }
}

static char *TextString(Text *t)
{
   return t->s ? t->s : strdup("");
}

// A connection of the calling thread's own: libcups is thread safe as
// long as no two threads share an http_t.
static http_t *Connect()
{
   return httpConnect2(cupsServer(), ippPort(), NULL, AF_UNSPEC,
		       cupsEncryption(), 1, 30000, NULL);
}

// s, a UTF-8 string as IPP sends them, in the locale's character set, as
// lpstat and lpq print it; NULL if no conversion is needed or possible.
static char *FromUTF8(const char *s)
{
   const char *codeset = nl_langinfo(CODESET);
   if (!codeset || !*codeset || !strcasecmp(codeset, "UTF-8") ||
       !strcasecmp(codeset, "UTF8"))
      return NULL;
   char to[64];
   snprintf(to, sizeof(to), "%s//TRANSLIT", codeset);
   iconv_t cd = iconv_open(to, "UTF-8");
   if (cd == (iconv_t)-1 && (cd = iconv_open(codeset, "UTF-8")) == (iconv_t)-1)
      return NULL;
   size_t len = strlen(s), size = 4 * len + 1;
   char *out = (char *)malloc(size);
   if (out)
    {
      char *in = (char *)s, *o = out;
      size_t in_left = len, out_left = size - 1;
      while (in_left > 0 &&
	     iconv(cd, &in, &in_left, &o, &out_left) == (size_t)-1)
       {
	 // a byte that is not UTF-8 or a character the locale lacks
	 if (errno == E2BIG || out_left == 0)
	    break;
	 *o++ = '?';
	 out_left--;
	 in++;
	 in_left--;
       }
      *o = '\0';
    }
   iconv_close(cd);
   return out;
}

// A copy of s fit for one field of a line: characters of the locale only
// (anything else becomes '?': DtStrtok() stops at an invalid character
// and the job parser would read NULL fields), no line breaks, no field
// separator SEP, never empty (the parsers skip empty fields).
static char *Field(const char *s, char sep, const char *empty)
{
   if (!s || !*s)
      s = empty;
   char *f = FromUTF8(s);
   if (!f && !(f = strdup(s)))
      return NULL;
   mbstate_t state;
   memset(&state, 0, sizeof(state));
   char *p = f;
   while (*p)
    {
      size_t n = mbrtowc(NULL, p, MB_CUR_MAX, &state);
      if (n == (size_t)-1 || n == (size_t)-2)
       {
	 *p++ = '?';
	 memset(&state, 0, sizeof(state));
       }
      else if (n == 1)
       {
	 if (*p == '\n' || *p == '\r' || *p == sep)
	    *p = ' ';
	 p++;
       }
      else
	 p += n;
    }
   if (!*f)
    {
      free(f);
      f = strdup(empty);
    }
   return f;
}

// CUPS-Get-Printers; NULL on failure
static ipp_t *GetPrinters(http_t *http)
{
   static const char * const attrs[] =
    {
      "printer-name",
      "printer-state",
      "printer-state-message",
      "printer-is-accepting-jobs",
      "device-uri"
    };
   ipp_t *request = ippNewRequest(IPP_OP_CUPS_GET_PRINTERS);
   ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
		 "requested-attributes", sizeof(attrs) / sizeof(attrs[0]),
		 NULL, attrs);
   ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
		"requesting-user-name", NULL, cupsUser());
   ipp_t *response = cupsDoRequest(http, request, "/");
   if (response && ippGetStatusCode(response) > IPP_STATUS_OK_CONFLICTING)
    {
      ippDelete(response);
      response = NULL;
    }
   return response;
}

// The attributes of one printer of a CUPS-Get-Printers response
struct PrinterAttrs
{
   const char *name;
   const char *device_uri;
   const char *message;
   int state;
   int accepting;
};

// Calls fn for every printer in response
static void ForEachPrinter(ipp_t *response,
			   void (*fn)(const PrinterAttrs *, Text *), Text *t)
{
   ipp_attribute_t *attr = ippFirstAttribute(response);
   while (attr)
    {
      // Skip to the next printer group
      while (attr && ippGetGroupTag(attr) != IPP_TAG_PRINTER)
	 attr = ippNextAttribute(response);
      if (!attr)
	 break;
      PrinterAttrs p = { NULL, NULL, NULL, IPP_PSTATE_IDLE, 1 };
      for (; attr && ippGetGroupTag(attr) == IPP_TAG_PRINTER;
	   attr = ippNextAttribute(response))
       {
	 const char *name = ippGetName(attr);
	 if (!name)
	    continue;
	 ipp_tag_t tag = ippGetValueTag(attr);
	 if (!strcmp(name, "printer-name") && tag == IPP_TAG_NAME)
	    p.name = ippGetString(attr, 0, NULL);
	 else if (!strcmp(name, "device-uri") && tag == IPP_TAG_URI)
	    p.device_uri = ippGetString(attr, 0, NULL);
	 else if (!strcmp(name, "printer-state-message") &&
		  tag == IPP_TAG_TEXT)
	    p.message = ippGetString(attr, 0, NULL);
	 else if (!strcmp(name, "printer-state") && tag == IPP_TAG_ENUM)
	    p.state = ippGetInteger(attr, 0);
	 else if (!strcmp(name, "printer-is-accepting-jobs") &&
		  tag == IPP_TAG_BOOLEAN)
	    p.accepting = ippGetBoolean(attr, 0);
       }
      if (p.name)
	 (*fn)(&p, t);
    }
}

static void DeviceLine(const PrinterAttrs *p, Text *t)
{
   // lpstat -v lists the printers that have a device
   if (p->device_uri)
      TextAdd(t, "device for %s: %s\n", p->name, p->device_uri);
}

char *CupsDeviceList()
{
   http_t *http = Connect();
   if (!http)
      return NULL;
   cups_in_use = 1;
   Text t = { NULL, 0, 0 };
   ipp_t *response = GetPrinters(http);
   if (response)
    {
      ForEachPrinter(response, DeviceLine, &t);
      ippDelete(response);
    }
   httpClose(http);
   return TextString(&t);
}

// The lines of "lpstat -a" and "lpstat -p" for one printer
static void StatusLines(const PrinterAttrs *p, Text *t)
{
   TextAdd(t, "%s %saccepting requests since -\n", p->name,
	   p->accepting ? "" : "not ");
   if (p->state == IPP_PSTATE_STOPPED)
      TextAdd(t, "printer %s disabled since -\n", p->name);
   else
      TextAdd(t, "printer %s is idle.  enabled since -\n", p->name);
   if (p->message && *p->message)
    {
      char *message = Field(p->message, '\t', "");
      if (message)
	 TextAdd(t, "\t%s\n", message);
      free(message);
    }
   else if (p->state == IPP_PSTATE_STOPPED)
      TextAdd(t, "\treason unknown\n");
}

static char *QueueStatus()
{
   http_t *http = Connect();
   if (!http)
      return NULL;
   Text t = { NULL, 0, 0 };
   ipp_t *response = GetPrinters(http);
   if (response)
    {
      ForEachPrinter(response, StatusLines, &t);
      ippDelete(response);
    }
   httpClose(http);
   return TextString(&t);
}

char *CupsPrintJobs(const char *printer, int *n_jobs)
{
   static const char * const attrs[] =
    {
      "job-id",
      "job-name",
      "job-originating-user-name",
      "job-k-octets",
      "time-at-creation"
    };
   *n_jobs = 0;
   http_t *http = Connect();
   if (!http)
      return strdup("");

   char uri[HTTP_MAX_URI];
   httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
		    "localhost", ippPort(), "/printers/%s", printer);
   ipp_t *request = ippNewRequest(IPP_OP_GET_JOBS);
   ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
		NULL, uri);
   ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
		"requesting-user-name", NULL, cupsUser());
   ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD, "which-jobs",
		NULL, "not-completed");
   ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
		 "requested-attributes", sizeof(attrs) / sizeof(attrs[0]),
		 NULL, attrs);
   ipp_t *response = cupsDoRequest(http, request, "/");
   httpClose(http);

   Text t = { NULL, 0, 0 };
   char *queue = Field(printer, '|', "-");
   ipp_attribute_t *attr = response ? ippFirstAttribute(response) : NULL;
   while (queue && attr)
    {
      while (attr && ippGetGroupTag(attr) != IPP_TAG_JOB)
	 attr = ippNextAttribute(response);
      if (!attr)
	 break;
      int id = 0, k_octets = 0;
      time_t created = 0;
      const char *title = NULL, *user = NULL;
      for (; attr && ippGetGroupTag(attr) == IPP_TAG_JOB;
	   attr = ippNextAttribute(response))
       {
	 const char *name = ippGetName(attr);
	 if (!name)
	    continue;
	 ipp_tag_t tag = ippGetValueTag(attr);
	 if (!strcmp(name, "job-id") && tag == IPP_TAG_INTEGER)
	    id = ippGetInteger(attr, 0);
	 else if (!strcmp(name, "job-k-octets") && tag == IPP_TAG_INTEGER)
	    k_octets = ippGetInteger(attr, 0);
	 else if (!strcmp(name, "time-at-creation") && tag == IPP_TAG_INTEGER)
	    created = (time_t)ippGetInteger(attr, 0);
	 else if (!strcmp(name, "job-name") && tag == IPP_TAG_NAME)
	    title = ippGetString(attr, 0, NULL);
	 else if (!strcmp(name, "job-originating-user-name") &&
		  tag == IPP_TAG_NAME)
	    user = ippGetString(attr, 0, NULL);
       }
      if (id <= 0)
	 continue;
      char *f_title = Field(title, '|', "unknown");
      char *f_user = Field(user, '|', "unknown");
      struct tm tm;
      memset(&tm, 0, sizeof(tm));
      localtime_r(&created, &tm);
      // The size as lpq gives it: job-k-octets * 1024 bytes
      if (f_title && f_user)
       {
	 // date|time as the other job parsers write them, in the
	 // locale's language
	 char when[100];
	 if (!strftime(when, sizeof(when), "%b %e|%T", &tm))
	    strcpy(when, "-|-");
	 TextAdd(&t, "%s|%s|%d|%s|%s|%.0f\n", queue, f_title, id, f_user,
		 when, 1024.0 * k_octets);
	 (*n_jobs)++;
       }
      free(f_title);
      free(f_user);
    }
   free(queue);
   ippDelete(response);
   return TextString(&t);
}

int CupsCountJobs(const char *job_list)
{
   int n = 0;
   for (; job_list && *job_list; job_list++)
      if (*job_list == '\n')
	 n++;
   return n;
}

char *CupsDefaultPrinter()
{
   http_t *http = Connect();
   if (!http)
      return NULL;
   char *name = NULL;
   cups_dest_t *dest = cupsGetNamedDest(http, NULL, NULL);
   if (dest)
    {
      size_t len = strlen(dest->name) + 2 +
		   (dest->instance ? strlen(dest->instance) : 0);
      if ((name = (char *)malloc(len)))
	 snprintf(name, len, "%s%s%s", dest->name, dest->instance ? "/" : "",
		  dest->instance ? dest->instance : "");
      cupsFreeDests(1, dest);
    }
   httpClose(http);
   return name;
}

// Requests made in threads of their own.  A thread writes its result to
// one end of a socket pair and closes it; the main loop reads the other
// end with XtAppAddInput (MotifThread), so the UI never waits for the
// scheduler.

struct Work
{
   int fd;
   char *printer;     // NULL: the queue states, else the jobs of printer
};

static void *Worker(void *arg)
{
   Work *w = (Work *)arg;
   char *text;
   if (w->printer)
    {
      int n_jobs;
      text = CupsPrintJobs(w->printer, &n_jobs);
    }
   else
      text = QueueStatus();
   if (text)
    {
      // MSG_NOSIGNAL: the reader may be gone
      size_t len = strlen(text), done = 0;
      while (done < len)
       {
	 ssize_t n = send(w->fd, text + done, len - done, MSG_NOSIGNAL);
	 if (n < 0 && errno == EINTR)
	    continue;
	 if (n <= 0)
	    break;
	 done += n;
       }
      free(text);
    }
   close(w->fd);
   free(w->printer);
   free(w);
   return NULL;
}

static int StartWorker(const char *printer)
{
   int sv[2];
   if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sv) < 0)
      return -1;
   Work *w = (Work *)calloc(1, sizeof(Work));
   if (!w || (printer && !(w->printer = strdup(printer))))
    {
      free(w);
      close(sv[0]);
      close(sv[1]);
      return -1;
    }
   w->fd = sv[1];

   // The thread takes no signals: they stay with the main thread
   sigset_t all, old;
   sigfillset(&all);
   pthread_sigmask(SIG_SETMASK, &all, &old);
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   pthread_t thread;
   int rc = pthread_create(&thread, &attr, Worker, w);
   pthread_attr_destroy(&attr);
   pthread_sigmask(SIG_SETMASK, &old, NULL);
   if (rc != 0)
    {
      free(w->printer);
      free(w);
      close(sv[0]);
      close(sv[1]);
      return -1;
    }
   return sv[0];
}

int CupsQueueStatusAsync()
{
   return StartWorker(NULL);
}

int CupsPrintJobsAsync(const char *printer)
{
   return StartWorker(printer);
}

#endif // DTPRINTINFO_CUPS
