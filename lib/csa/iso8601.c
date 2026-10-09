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
/* $TOG: iso8601.c /main/2 1997/12/29 10:46:50 bill $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#include <EUSCompat.h>
#define XOS_USE_NO_LOCKING
#define X_INCLUDE_TIME_H
#include <X11/Xos_r.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "iso8601.h"

/* strlen("CCYYMMDDThhmmssZ") */
#define ISO8601_LEN	16

/*
 * Parse exactly 'n' decimal digits at 's'.  Returns -1 if any of them
 * is not a digit.
 */
static int
get_digits(const char *s, int n)
{
	int	v = 0;

	while (n-- > 0) {
		if (*s < '0' || *s > '9')
			return (-1);
		v = v * 10 + (*s++ - '0');
	}
	return (v);
}

/*
 * Days from 1970-01-01 to the given proleptic Gregorian date.  A day of
 * the month past the end of the month is carried into the next month,
 * the way mktime() and timegm() normalise it (February 31 is March 3,
 * or March 2 in a leap year).
 */
static long
days_from_civil(long y, int m, int d)
{
	long	era, yoe, doy, doe;

	if (m <= 2)
		y--;
	era = (y >= 0 ? y : y - 399) / 400;
	yoe = y - era * 400;
	doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
	doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	return (era * 146097 + doe - 719468);
}

/*
 * _csa_iso8601_to_tick - convert a standard date/time string to a tick
 *
 * Note 1:This function supports a limited subset of the iso8601 standard.
 *	  Only one of the variations described by the standard is
 *        supported, namely:
 *
 *		CCYYMMDDThhmmssZ
 *
 *        ...known in the standard as "Complete Representation, Basic Format"
 *        for calendar date and "Coordinated Universal Time (UTC), Basic Format"
 *        for time.
 *
 *	  This can carry all the information required for date+time by
 *	  the CDE 1.0 Calendar component.  More general support, if ever
 *	  needed (say for interoperability or finer granularity) can be
 *	  implemented inside this function without modifying its interface.
 *
 * Note 2:All output time information is in UTC, and all input 
 *        time information is assumed to be pre-converted to UTC.
 *
 *	  dac 19940728T224055Z  :-)
 *
 * Note 3:This is called once per date attribute of every entry, on both
 *	  the client and the server.  It used to switch TZ to GMT and
 *	  back around mktime() (two tzset() calls, i.e. two zoneinfo
 *	  loads, per call, and not thread-safe).  It is now a fixed-width
 *	  parser plus plain UTC arithmetic, equivalent to timegm().
 */ 
int
_csa_iso8601_to_tick(char *buf, time_t *tick_out)
{
	int	year, month, day, hour, min, sec;

	/* validation rules:
	 *	- length of input is fixed: strlen("CCYYMMDDThhmmssZ")
	 *	- every field is all digits, 'T' and 'Z' are where they belong
	 *	  (the last char must be Z, indicating UTC time)
	 *	- crude range check on each numerical value scanned.
	 */
	if (buf == NULL || strlen(buf) != ISO8601_LEN ||
	    buf[8] != 'T' || buf[15] != 'Z')
		return (-1);

	year  = get_digits(buf, 4);
	month = get_digits(buf + 4, 2);
	day   = get_digits(buf + 6, 2);
	hour  = get_digits(buf + 9, 2);
	min   = get_digits(buf + 11, 2);
	sec   = get_digits(buf + 13, 2);

	if ((year<1970) || (year>2038))	return (-1);
	if ((month<1) || (month>12))	return (-1);
	if ((day<1) || (day>31))	return (-1);
	if ((hour<0) || (hour>24))	return (-1);
	if ((min<0) || (min>59))	return (-1);
	if ((sec<0) || (sec>59))	return (-1);

	*tick_out = (time_t)days_from_civil(year, month, day) * 86400 +
		    hour * 3600 + min * 60 + sec;

	return (0);
}

/*
 * _csa_tick_to_iso8601 - convert from tick to iso8601 time string
 *
 * Note 1: Similar comments to the above.  This function complements
 *         _csa_iso8601_to_tick, providing bi-directional conversion.
 *
 * Note 2: All input and output time information is UTC.
 */
int
_csa_tick_to_iso8601(time_t tick, char *buf_out)
{
	struct tm	*time_str;
	time_t		tk=tick;
	_Xgtimeparams	gmtime_buf;

	/* tick must be +ve to be valid */
	if (tick < 0) {
	   return(-1);
	}

	(void) gmtime_buf;	/* unused unless XTHREADS */
	if ((time_str = _XGmtime(&tk, gmtime_buf)) == NULL)
		return (-1);

	/* format string forces fixed width (zero-padded) fields */
	sprintf(buf_out, "%04d%02d%02dT%02d%02d%02dZ",
		time_str->tm_year + 1900,
		time_str->tm_mon + 1,
		time_str->tm_mday,
		time_str->tm_hour,
		time_str->tm_min,
		time_str->tm_sec);

	return (0);
}

/*
 * Convert iso8601 date time range to a start tick and an end tick
 *
 * iso8601 range is:
 *                   <start> "/" <end>
 *
 * start and end are iso8601 strings in CCYYMMDDThhmmssZ format.
 */

int
_csa_iso8601_to_range(char *buf, time_t *start, time_t *end)
{
    size_t nchars;
    char tmpstr[ISO8601_LEN + 1];
    char *p;

    if ((p = strchr(buf, '/')) == NULL) {
        return (-1);
    }

    nchars = (size_t)(p - buf);
    if (nchars != ISO8601_LEN)
        return (-1);
    memcpy(tmpstr, buf, nchars);
    tmpstr[nchars]='\0';

    if (_csa_iso8601_to_tick(tmpstr, start) != 0) {
        return (-1);
    }

    p++;
    if (_csa_iso8601_to_tick(p, end) != 0) {
        return (-1);
    }

    /* compare the times, not the pointers to them */
    if (*end < *start)
	return (-1);
    else
	return(0);
}

/*
 * Convert time range specified as start/end ticks to iso8601 format
 *
 * iso8601 result is:
 *                   <start> "/" <end>
 *
 * start and end are iso8601 strings in CCYYMMDDThhmmssZ format.
 */
int
_csa_range_to_iso8601(time_t start, time_t end, char *buf)
{
    char tmpstr1[BUFSIZ], tmpstr2[BUFSIZ];

    /* validate: ticks must be +ve, and end can't precede start */
    if ((start < 0) || (end < 0) || (end < start)) {
        return(-1);
    }

    if (_csa_tick_to_iso8601(start, tmpstr1) != 0) {
        return (-1);
    }
    if (_csa_tick_to_iso8601(end, tmpstr2) != 0) {
        return (-1);
    }

    sprintf(buf, "%s/%s", tmpstr1, tmpstr2);
    return(0);
}

static int
not_sign(char c)
{
   if ((c=='+') || (c=='-'))
      return (0);
   else
      return (1);
}

/*
 * This converts from a string representation of a quantity of time,
 * to (signed) integer number of * seconds.
 *   The first character (byte) must be a '+' or * a '-', indicating
 * the sense of the period.  This can be used however you like - it's
 * just a way of carrying round the sign, while keeping the main part
 * of the string in ISO 8601.
 *   The string must be in the format described by ISO 8601, clause
 * 5.5.1 (b), with the added restriction that only seconds may be specified.
 * format: [+/-]PTnS
 */
int
_csa_iso8601_to_duration(char *buf, time_t *sec)
{
   /* buf must begin with '+' or '-', then 'P', end with 'S' */
   char	sign, *ptr, *ptr2, *numptr;
   int	num=0;

   ptr2 = ptr = buf;
   sign = *ptr++;

   if (not_sign(sign)) {
	if (*ptr2++ != 'P' || *ptr2++ != 'T') {
		return (-1);
	}
   } else if (not_sign(sign) || *ptr++ != 'P' || *ptr++ != 'T') {
	return (-1);
   }

   if (not_sign(sign))
	ptr = ptr2;

   numptr = ptr;
   while (*ptr >= '0' && *ptr <= '9') ptr++;

   if (numptr == ptr || !(*ptr && *ptr++ == 'S' && *ptr == '\0'))
	return (-1);
   else {
	num = atoi(numptr);
	*sec = (sign == '-') ? -num : num;
	return (0);
   }
}

/*
 * This converts from a (signed) integer number of seconds to a string
 * representation.  The string format is a sign character followed by
 * an IS0 8601 string as described above for _csa_iso8601_to_duration.
 */
int
_csa_duration_to_iso8601(time_t sec, char *buf)
{
    sprintf(buf, "%cPT%ldS", (sec < 0) ? '-': '+',
	    (sec < 0) ? -(long)sec : (long)sec);

    return(0);
}
