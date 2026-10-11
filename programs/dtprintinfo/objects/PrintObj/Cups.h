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

// The print queues, their states and their jobs read from the CUPS
// scheduler with libcups instead of from lpstat and lpq.
//
// Built in when HAVE_LIBCUPS is defined (Makefile.am defines it when
// cups-config is found) and <cups/cups.h> exists.  Used when a CUPS
// scheduler answered the first request (CupsDeviceList()); otherwise the
// lpstat and lpq commands are run as before.

#ifndef DTPRINTINFO_CUPS_H
#define DTPRINTINFO_CUPS_H

#if defined(HAVE_LIBCUPS) && defined(__has_include)
#if __has_include(<cups/cups.h>)
#define DTPRINTINFO_CUPS 1
#endif
#endif

#ifdef DTPRINTINFO_CUPS

// True once CupsDeviceList() reached a CUPS scheduler.
extern int CupsInUse();

// What "lpstat -v" prints: a "device for NAME: URI" line for every
// printer, from one CUPS-Get-Printers request.  malloc'd.  NULL if no
// CUPS scheduler answers; then CupsInUse() stays false.
extern char *CupsDeviceList();

// What "lpstat -a -p" prints for every printer (see QueueStatusTable),
// from one CUPS-Get-Printers request made in a thread of its own: read
// the returned descriptor until end of file.  -1 on failure.
extern int CupsQueueStatusAsync();

// The not-completed jobs of a printer as "printer|title|id|owner|date|
// time|size" lines, the form Queue::ParseOutput() takes.  malloc'd;
// *n_jobs gets the number of lines.
extern char *CupsPrintJobs(const char *printer, int *n_jobs);

// The same, read in a thread of its own: read the returned descriptor
// until end of file and count its lines with CupsCountJobs().  -1 on
// failure.
extern int CupsPrintJobsAsync(const char *printer);
extern int CupsCountJobs(const char *job_list);

// The default destination as "lpstat -d" names it ("name" or
// "name/instance"); malloc'd, NULL if there is none.
extern char *CupsDefaultPrinter();

#endif // DTPRINTINFO_CUPS

#endif // DTPRINTINFO_CUPS_H
