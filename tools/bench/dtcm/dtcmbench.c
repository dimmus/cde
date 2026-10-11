/*
 * CDE benchmarks: dtcm, libcsa and rpc.cmsd.
 *
 * Licensed under the LGPL 2.1 license.
 *
 *   dtcmbench populate [-c CAL] [-n ONETIME] [-R REPEATING]
 *	Create the calendar CAL (default: user@host, as dtcm does) on the
 *	running rpc.cmsd if it does not exist and add ONETIME one-time
 *	appointments (default 5000, spread over 2025-2027) and REPEATING
 *	repeating ones (default 200: daily, weekly, biweekly, monthly by
 *	date and by weekday, yearly, Monday-Friday, every 3 days; some
 *	forever, some for a count).  The appointments are built by
 *	libDtCmP's validate_appt(), the code dtcm_insert uses, so they
 *	carry the attributes dtcm itself writes.  Every 1000th one-time
 *	appointment has "zebra" in its summary, every 3rd one "standup".
 *
 *   dtcmbench bench [-r REPEAT] [-s SCALE] [-j FILE] [-l] [CASE...]
 *	Time the lookups dtcm makes, on the calendar CAL ($DTCMBENCH_CAL or
 *	user@host):
 *	  month       a month view: the month's range in one
 *	              csa_list_entries() plus query_appt_struct() per entry,
 *	              as monthglance.c/allocate_paint_cache() (12 months of
 *	              2026 per run)
 *	  week        the same for the 52 weeks of 2026 (weekglance.c)
 *	  year        a whole year (2026) in one range lookup
 *	  find_all    Find "all" for a frequent word, as find.c does it:
 *	              52-week chunks with the summary matched by the server
 *	              (CSA_MATCH_CONTAIN), bot..eot
 *	  find_rare   the same for a word in 5 appointments
 *	  find_old    Find "all" the pre-TODO way: 4-week chunks, no server
 *	              match, every entry read and matched here
 *	When $DTCMBENCH_CMSD_LOG names the stderr of an rpc.cmsd started with
 *	-d, the RPCs of each case are counted from its log (one line per
 *	call) and reported per operation in the table and the JSON.
 *
 * run.sh starts a private rpc.cmsd and runs all of it; see README.md.
 */

#include <EUSCompat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <nl_types.h>
#include <csa.h>
#include "util.h"
#include "cm_tty.h"
#include "timeops.h"
#include "props.h"

#include "../benchutil.h"

extern void scrub_attr_list(Dtcm_appointment *);

static CSA_session_handle cal;
static int cal_version;
static char *cal_name;

#define MAXCASES 16
static struct {
	const char *name;
	double rpcs;		/* RPCs per operation, -1: not counted */
	double srv_ns;		/* rpc.cmsd CPU ns per operation, or -1:
				 * the median over the runs */
	double srv_runs[32];
	int nruns;
	long entries;		/* entries returned per operation */
} stats[MAXCASES];
static int nstats;

/* --- RPC count from the rpc.cmsd -d log -------------------------------- */

/* Calls in the log so far; reads only what was added since the last call. */
static long
rpc_count(void)
{
	static FILE *f;
	static long n;
	static int failed;
	const char *log = getenv("DTCMBENCH_CMSD_LOG");
	char line[512];

	if (failed || log == NULL)
		return -1;
	if (f == NULL && (f = fopen(log, "r")) == NULL) {
		failed = 1;
		return -1;
	}
	clearerr(f);
	while (fgets(line, sizeof line, f))
		if (strstr(line, "_svc called"))
			n++;
	return n;
}

/* CPU time of rpc.cmsd ($DTCMBENCH_CMSD_PID) in ns, or -1 */
static double
server_cpu_ns(void)
{
	const char *pid = getenv("DTCMBENCH_CMSD_PID");
	char path[64];
	double ns;
	FILE *f;

	if (pid == NULL || *pid == '\0')
		return -1;
	snprintf(path, sizeof path, "/proc/%s/schedstat", pid);
	if ((f = fopen(path, "r")) == NULL)
		return -1;
	if (fscanf(f, "%lf", &ns) != 1)
		ns = -1;
	fclose(f);
	return ns;
}

static void
note_stats(const char *name, long rpcs, double srv_ns, long ops,
	   long entries)
{
	int i;

	for (i = 0; i < nstats; i++)
		if (strcmp(stats[i].name, name) == 0)
			break;
	if (i == nstats) {
		if (nstats == MAXCASES)
			return;
		nstats++;
	}
	stats[i].name = name;
	stats[i].rpcs = rpcs < 0 ? -1 : (double)rpcs / ops;
	if (srv_ns < 0) {
		stats[i].srv_ns = -1;
	} else {
		if (stats[i].nruns < 32)
			stats[i].srv_runs[stats[i].nruns++] = srv_ns / ops;
		stats[i].srv_ns = bench_median(stats[i].srv_runs,
					       stats[i].nruns);
	}
	stats[i].entries = entries / ops;
}

/* --- calendar ----------------------------------------------------------- */

static char *
default_cal(void)
{
	char *c = getenv("DTCMBENCH_CAL");

	return c && *c ? c : cm_get_credentials();
}

static void
logon(int create)
{
	CSA_calendar_user user;
	CSA_return_code stat;

	memset(&user, 0, sizeof user);
	user.user_name = cal_name;
	user.calendar_address = cal_name;
	stat = csa_logon(NULL, &user, NULL, NULL, NULL, &cal, NULL);
	if (stat != CSA_SUCCESS && create) {
		stat = csa_add_calendar(0, &user, 0, NULL, NULL);
		if (stat != CSA_SUCCESS)
			bench_die("csa_add_calendar(%s): error %d "
				  "(is rpc.cmsd running?)\n", cal_name,
				  (int)stat);
		stat = csa_logon(NULL, &user, NULL, NULL, NULL, &cal, NULL);
	}
	if (stat != CSA_SUCCESS)
		bench_die("csa_logon(%s): error %d (is rpc.cmsd running? "
			  "populated?)\n", cal_name, (int)stat);
	cal_version = get_data_version(cal);
}

/* --- populate ----------------------------------------------------------- */

static boolean_t
yes(void *unused)
{
	(void)unused;
	return B_TRUE;
}

static int
add_appt_h(Props *p, nl_catd catd, const char *date, const char *start,
	   const char *end, const char *what, const char *repeat,
	   const char *count, CSA_entry_handle *entry_r)
{
	Dtcm_appointment *appt;
	CSA_entry_handle entry;
	Validate_op op;
	CSA_return_code stat;

	appt = allocate_appt_struct(appt_write, cal_version, NULL);
	load_appt_defaults(appt, p);
	op = validate_appt(catd, appt, (char *)start, (char *)end,
			   (char *)date, 0, (char *)what, (char *)repeat,
			   (char *)count, yes, NULL, cal_version);
	if (op != VALID_APPT) {
		fprintf(stderr, "dtcmbench: invalid appointment %s %s %s "
			"(%d)\n", date, start, what, (int)op);
		free_appt_struct(&appt);
		return -1;
	}
	scrub_attr_list(appt);
	stat = csa_add_entry(cal, appt->count, appt->attrs, &entry, NULL);
	free_appt_struct(&appt);
	if (stat != CSA_SUCCESS) {
		fprintf(stderr, "dtcmbench: csa_add_entry: error %d\n",
			(int)stat);
		return -1;
	}
	if (entry_r)
		*entry_r = entry;
	else
		csa_free((CSA_buffer)entry);
	return 0;
}

static int
add_appt(Props *p, nl_catd catd, const char *date, const char *start,
	 const char *end, const char *what, const char *repeat,
	 const char *count)
{
	return add_appt_h(p, catd, date, start, end, what, repeat, count,
			  NULL);
}

static void
mdy(int dayno, char *buf, size_t len)
{
	struct tm tm;

	memset(&tm, 0, sizeof tm);
	tm.tm_year = 125;		/* 2025-01-01 + dayno */
	tm.tm_mday = 1 + dayno;
	tm.tm_hour = 12;
	tm.tm_isdst = -1;
	mktime(&tm);
	snprintf(buf, len, "%d/%d/%d", tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_year + 1900);
}

static const char *const hours12[] = {
	"8:00am", "9:00am", "10:00am", "11:00am", "12:00pm", "1:00pm",
	"2:00pm", "3:00pm", "4:00pm", "5:00pm", "6:00pm"
};

static const struct {
	const char *repeat, *count;
} repeats[] = {
	{ "Daily", "30" },
	{ "Weekly", "forever" },
	{ "Every Two Weeks", "52" },
	{ "Monthly By Date", "forever" },
	{ "Yearly", "forever" },
	{ "Monday thru Friday", "260" },
	{ "Monthly By Weekday", "36" },
	{ "Every 3 days", "120" },
};

/*
 * Delete one instance (CSA_SCOPE_ONE: an exception) of every 10th
 * repeating sequence with an instance in March 2026.
 */
static long
delete_instances(void)
{
	CSA_attribute *range_attrs;
	CSA_enum *ops;
	CSA_entry_handle *list = NULL;
	CSA_uint32 num, i;
	Dtcm_appointment *appt;
	struct tm tm;
	long bad = 0, seen[64];
	int n, nseen = 0, k;

	memset(&tm, 0, sizeof tm);
	tm.tm_year = 126;
	tm.tm_mon = 2;
	tm.tm_mday = 1;
	tm.tm_isdst = -1;
	setup_range(&range_attrs, &ops, &n, mktime(&tm),
		    mktime(&tm) + 31 * 86400L - 1, CSA_TYPE_EVENT, 0, B_FALSE,
		    cal_version);
	if (csa_list_entries(cal, n, range_attrs, ops, &num, &list, NULL)
	    != CSA_SUCCESS)
		return 1;
	free_range(&range_attrs, &ops, n);
	appt = allocate_appt_struct(appt_read, cal_version,
				    CSA_ENTRY_ATTR_SUMMARY_I, NULL);
	for (i = 0; i < num && nseen < 64; i++) {
		long seq;
		char *w;

		if (query_appt_struct(cal, list[i], appt) != CSA_SUCCESS)
			continue;
		w = appt->what->value->item.string_value;
		if (w == NULL || sscanf(w, "Repeating %ld", &seq) != 1 ||
		    seq % 10 != 0)
			continue;
		for (k = 0; k < nseen && seen[k] != seq; k++)
			;
		if (k < nseen)
			continue;
		seen[nseen++] = seq;
		bad += csa_delete_entry(cal, list[i], CSA_SCOPE_ONE, NULL)
			!= CSA_SUCCESS;
	}
	free_appt_struct(&appt);
	csa_free(list);
	return bad;
}

static int
populate(int argc, char **argv)
{
	long nonce = 5000, nrep = 200, i, bad = 0;
	Props *p = NULL;
	nl_catd catd = (nl_catd)-1;
	char date[32], what[128];
	double t0;
	long r0;
	int c;

	cal_name = default_cal();
	while ((c = getopt(argc, argv, "c:n:R:")) != -1) {
		switch (c) {
		case 'c': cal_name = optarg; break;
		case 'n': nonce = atol(optarg); break;
		case 'R': nrep = atol(optarg); break;
		default:
			fprintf(stderr, "usage: dtcmbench populate [-c CAL] "
				"[-n ONETIME] [-R REPEATING]\n");
			return 2;
		}
	}
	cm_tty_load_props(&p);
	logon(1);
	r0 = rpc_count();
	t0 = bench_now_ns();

	for (i = 0; i < nonce; i++) {
		int h = i % 10;

		mdy((int)(i * 1095 / (nonce ? nonce : 1)), date, sizeof date);
		snprintf(what, sizeof what, "%s %ld room %ld%s",
			 i % 3 == 0 ? "Standup" : "Review", i, i % 17,
			 i % 1000 == 7 ? " zebra" : "");
		/* every 50th has no time (dtcm's "magic time") */
		bad += add_appt(p, catd, date, i % 50 == 3 ? NULL : hours12[h],
				i % 50 == 3 ? NULL : hours12[h + 1],
				what, NULL, NULL) != 0;
	}
	for (i = 0; i < nrep; i++) {
		int k = i % (int)(sizeof repeats / sizeof repeats[0]);
		int h = i % 10;

		mdy((int)(i * 365 / (nrep ? nrep : 1)), date, sizeof date);
		snprintf(what, sizeof what, "Repeating %ld %s", i,
			 repeats[k].repeat);
		bad += add_appt(p, catd, date, hours12[h], hours12[h + 1],
				what, repeats[k].repeat, repeats[k].count) != 0;
	}

	bad += delete_instances();

	printf("populate %s: %ld one-time + %ld repeating in %.2f s, "
	       "%ld failed", cal_name, nonce, nrep,
	       (bench_now_ns() - t0) / 1e9, bad);
	if (r0 >= 0)
		printf(", %ld RPCs", rpc_count() - r0);
	printf("\n");
	csa_logoff(cal, NULL);
	return bad ? 1 : 0;
}

/*
 * dtcmbench change [-c CAL] [-n N] [-i MS] [-d M/D/YYYY]
 *	Add N appointments on day D (default today), one every MS
 *	milliseconds (default 500), then delete them the same way: 2N
 *	changes, each of which makes rpc.cmsd call back every registered
 *	client (a running dtcm refreshes).
 */
static int
change(int argc, char **argv)
{
	long n = 10, ms = 500, i, bad = 0;
	Props *p = NULL;
	nl_catd catd = (nl_catd)-1;
	CSA_entry_handle *h;
	char date[32] = "", what[64];
	struct timespec ts;
	int c;

	cal_name = default_cal();
	while ((c = getopt(argc, argv, "c:n:i:d:")) != -1) {
		switch (c) {
		case 'c': cal_name = optarg; break;
		case 'n': n = atol(optarg); break;
		case 'i': ms = atol(optarg); break;
		case 'd': snprintf(date, sizeof date, "%s", optarg); break;
		default:
			fprintf(stderr, "usage: dtcmbench change [-c CAL] "
				"[-n N] [-i MS] [-d M/D/YYYY]\n");
			return 2;
		}
	}
	if (date[0] == '\0') {
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);

		snprintf(date, sizeof date, "%d/%d/%d", tm->tm_mon + 1,
			 tm->tm_mday, tm->tm_year + 1900);
	}
	if (n < 1 || (h = calloc(n, sizeof *h)) == NULL)
		return 2;
	cm_tty_load_props(&p);
	logon(0);
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000L;
	for (i = 0; i < n; i++) {
		snprintf(what, sizeof what, "Change %ld", i);
		bad += add_appt_h(p, catd, date, hours12[i % 10],
				  hours12[i % 10 + 1], what, NULL, NULL,
				  &h[i]) != 0;
		nanosleep(&ts, NULL);
	}
	for (i = 0; i < n; i++) {
		if (h[i] == 0)
			continue;
		bad += csa_delete_entry(cal, h[i], CSA_SCOPE_ALL, NULL)
			!= CSA_SUCCESS;
		csa_free((CSA_buffer)h[i]);
		nanosleep(&ts, NULL);
	}
	free(h);
	csa_logoff(cal, NULL);
	printf("change %s: %ld added and deleted on %s, %ld failed\n",
	       cal_name, n, date, bad);
	return bad ? 1 : 0;
}

/* --- lookups ------------------------------------------------------------ */

static time_t
mktick(int year, int mon, int day)
{
	struct tm tm;

	memset(&tm, 0, sizeof tm);
	tm.tm_year = year - 1900;
	tm.tm_mon = mon;
	tm.tm_mday = day;
	tm.tm_isdst = -1;
	return mktime(&tm);
}

/* One range as a view paints it: the list, then each entry's attributes. */
static long
view_range(time_t start, time_t stop)
{
	CSA_attribute *range_attrs;
	CSA_enum *ops;
	CSA_entry_handle *list = NULL;
	CSA_uint32 total = 0, i;
	Dtcm_appointment *appt;
	int n;

	setup_range(&range_attrs, &ops, &n, start, stop, CSA_TYPE_EVENT, 0,
		    B_FALSE, cal_version);
	if (csa_list_entries(cal, n, range_attrs, ops, &total, &list, NULL)
	    != CSA_SUCCESS)
		bench_die("csa_list_entries failed\n");
	free_range(&range_attrs, &ops, n);

	appt = allocate_appt_struct(appt_read, cal_version,
				    CSA_ENTRY_ATTR_START_DATE_I,
				    CSA_ENTRY_ATTR_END_DATE_I,
				    CSA_ENTRY_ATTR_SUMMARY_I,
				    CSA_X_DT_ENTRY_ATTR_SHOWTIME_I, NULL);
	for (i = 0; i < total; i++)
		if (query_appt_struct(cal, list[i], appt) != CSA_SUCCESS)
			bench_die("query_appt_struct failed\n");
	free_appt_struct(&appt);
	csa_free(list);
	return total;
}

static boolean_t
contains_nocase(const char *str, const char *sub)
{
	size_t len = strlen(sub);

	for (; *str != '\0'; str++)
		if (strncasecmp(str, sub, len) == 0)
			return B_TRUE;
	return B_FALSE;
}

/* find.c's add_summary_match() */
static void
add_summary_match(CSA_attribute **attrs, CSA_enum **ops, int *count,
		  const char *str)
{
	*attrs = realloc(*attrs, (*count + 1) * sizeof(CSA_attribute));
	*ops = realloc(*ops, (*count + 1) * sizeof(CSA_enum));
	if (*attrs == NULL || *ops == NULL)
		bench_die("out of memory\n");
	memset(&(*attrs)[*count], 0, sizeof(CSA_attribute));
	initialize_entry_attr(CSA_ENTRY_ATTR_SUMMARY_I, &(*attrs)[*count],
			      appt_write, cal_version);
	(*attrs)[*count].value->item.string_value = strdup(str);
	(*ops)[*count] = CSA_MATCH_CONTAIN;
	(*count)++;
}

/* find.c's find_appts() loop for "all"; returns the matches. */
static long
find(const char *str, int filter)
{
	time_t start = get_bot(), eot = get_eot(), stop;
	time_t chunk = (filter ? 52 : 4) * 7 * 86400L;
	CSA_attribute *range_attrs;
	CSA_enum *ops;
	CSA_entry_handle *entries;
	CSA_uint32 num, i;
	Dtcm_appointment *appt;
	long matches = 0;
	int n;

	appt = allocate_appt_struct(appt_read, cal_version,
				    CSA_ENTRY_ATTR_START_DATE_I,
				    CSA_ENTRY_ATTR_SUMMARY_I,
				    CSA_X_DT_ENTRY_ATTR_SHOWTIME_I, NULL);
	stop = start + chunk;
	if (stop > eot || stop < 0)
		stop = eot;
	for (; stop <= eot;) {
		setup_range(&range_attrs, &ops, &n, start, stop,
			    CSA_TYPE_EVENT, 0, B_FALSE, cal_version);
		if (filter)
			add_summary_match(&range_attrs, &ops, &n, str);
		entries = NULL;
		if (csa_list_entries(cal, n, range_attrs, ops, &num, &entries,
				     NULL) != CSA_SUCCESS)
			bench_die("csa_list_entries failed\n");
		for (i = 0; i < num; i++) {
			char *what;

			if (query_appt_struct(cal, entries[i], appt)
			    != CSA_SUCCESS)
				bench_die("query_appt_struct failed\n");
			what = appt->what->value->item.string_value;
			if (what != NULL && contains_nocase(what, str))
				matches++;
		}
		csa_free(entries);
		free_range(&range_attrs, &ops, n);
		if (stop == eot)
			break;
		start = stop + 1;
		if (start > eot)
			break;
		stop = start + chunk;
		if (stop > eot || stop < 0)
			stop = eot;
	}
	free_appt_struct(&appt);
	return matches;
}

/*
 * dtcmbench dump [-c CAL] [-y FIRST_YEAR] [-Y LAST_YEAR] [-f WORD]
 *	Print every entry of every month of the years (default 2024-2028),
 *	each with all its attributes, and of Find "all" for WORD (if given):
 *	what libcsa hands dtcm.  Two builds of rpc.cmsd or libcsa must print
 *	the same (the creation and update times are left out).
 */
static void
dump_value(CSA_attribute_value *v)
{
	if (v == NULL) {
		printf("(null)");
		return;
	}
	printf("%d:", (int)v->type);
	switch (v->type) {
	case CSA_VALUE_BOOLEAN:
	case CSA_VALUE_ENUMERATED:
	case CSA_VALUE_FLAGS:
	case CSA_VALUE_SINT32:
		printf("%ld", (long)v->item.sint32_value);
		break;
	case CSA_VALUE_UINT32:
		printf("%lu", (unsigned long)v->item.uint32_value);
		break;
	case CSA_VALUE_STRING:
	case CSA_VALUE_DATE_TIME:
	case CSA_VALUE_DATE_TIME_RANGE:
	case CSA_VALUE_TIME_DURATION:
		printf("\"%s\"", v->item.string_value ?
		       v->item.string_value : "(null)");
		break;
	case CSA_VALUE_CALENDAR_USER:
		if (v->item.calendar_user_value)
			printf("user:%s", v->item.calendar_user_value->user_name
			       ? v->item.calendar_user_value->user_name : "");
		break;
	case CSA_VALUE_REMINDER:
		if (v->item.reminder_value) {
			CSA_reminder *r = v->item.reminder_value;
			printf("rem:%s/%s/%lu/%lu:%.*s", r->lead_time,
			       r->snooze_time ? r->snooze_time : "",
			       (unsigned long)r->repeat_count,
			       (unsigned long)r->reminder_data.size,
			       (int)r->reminder_data.size,
			       r->reminder_data.data ?
			       (char *)r->reminder_data.data : "");
		}
		break;
	case CSA_VALUE_DATE_TIME_LIST: {
		CSA_date_time_entry *d;
		for (d = v->item.date_time_list_value; d; d = d->next)
			printf("%s,", d->date_time);
		break;
	}
	case CSA_VALUE_OPAQUE_DATA:
		if (v->item.opaque_data_value)
			printf("opaque:%lu:%.*s", (unsigned long)
			       v->item.opaque_data_value->size,
			       (int)v->item.opaque_data_value->size,
			       v->item.opaque_data_value->data ? (char *)
			       v->item.opaque_data_value->data : "");
		break;
	default:
		printf("?");
	}
}

static void
dump_list(const char *label, CSA_uint32 num, CSA_entry_handle *list)
{
	CSA_uint32 i, j, na;
	CSA_attribute *attrs;

	printf("== %s: %lu\n", label, (unsigned long)num);
	for (i = 0; i < num; i++) {
		if (csa_read_entry_attributes(cal, list[i], 0, NULL, &na,
					      &attrs, NULL) != CSA_SUCCESS) {
			printf("  read failed\n");
			continue;
		}
		printf("  [%lu]", (unsigned long)i);
		for (j = 0; j < na; j++) {
			printf(" %s=", attrs[j].name);
			/* when the appointment was written, not what */
			if (strstr(attrs[j].name, "Date Created") ||
			    strstr(attrs[j].name, "Last Update"))
				printf("(time)");
			else
				dump_value(attrs[j].value);
		}
		printf("\n");
		csa_free(attrs);
	}
}

static int
dump(int argc, char **argv)
{
	int y0 = 2024, y1 = 2028, y, m, n, c;
	const char *word = NULL;
	CSA_attribute *range_attrs;
	CSA_enum *ops;
	CSA_entry_handle *list;
	CSA_uint32 num;
	char label[64];

	cal_name = default_cal();
	while ((c = getopt(argc, argv, "c:y:Y:f:")) != -1) {
		switch (c) {
		case 'c': cal_name = optarg; break;
		case 'y': y0 = atoi(optarg); break;
		case 'Y': y1 = atoi(optarg); break;
		case 'f': word = optarg; break;
		default:
			fprintf(stderr, "usage: dtcmbench dump [-c CAL] "
				"[-y YEAR] [-Y YEAR] [-f WORD]\n");
			return 2;
		}
	}
	logon(0);
	for (y = y0; y <= y1; y++)
		for (m = 0; m < 12; m++) {
			setup_range(&range_attrs, &ops, &n, mktick(y, m, 1),
				    mktick(y, m + 1, 1) - 1, CSA_TYPE_EVENT, 0,
				    B_FALSE, cal_version);
			list = NULL;
			if (csa_list_entries(cal, n, range_attrs, ops, &num,
					     &list, NULL) != CSA_SUCCESS)
				bench_die("csa_list_entries failed\n");
			free_range(&range_attrs, &ops, n);
			snprintf(label, sizeof label, "%04d-%02d", y, m + 1);
			dump_list(label, num, list);
			csa_free(list);
		}
	if (word) {
		time_t start = get_bot(), stop = get_eot();

		setup_range(&range_attrs, &ops, &n, start, stop,
			    CSA_TYPE_EVENT, 0, B_FALSE, cal_version);
		add_summary_match(&range_attrs, &ops, &n, word);
		list = NULL;
		if (csa_list_entries(cal, n, range_attrs, ops, &num, &list,
				     NULL) != CSA_SUCCESS)
			bench_die("csa_list_entries failed\n");
		free_range(&range_attrs, &ops, n);
		dump_list(word, num, list);
		csa_free(list);
	}
	csa_logoff(cal, NULL);
	return 0;
}

static long entries_seen;

static long
run_month(long n)
{
	long r, ops = 0;
	int m;

	for (r = 0; r < n; r++)
		for (m = 0; m < 12; m++, ops++)
			entries_seen += view_range(mktick(2026, m, 1),
						   mktick(2026, m + 1, 1) - 1);
	return ops;
}

static long
run_week(long n)
{
	long r, ops = 0;
	int w;

	for (r = 0; r < n; r++)
		for (w = 0; w < 52; w++, ops++)
			entries_seen += view_range(mktick(2026, 0, 4 + 7 * w),
						   mktick(2026, 0, 11 + 7 * w)
						   - 1);
	return ops;
}

static long
run_year(long n)
{
	long r;

	for (r = 0; r < n; r++)
		entries_seen += view_range(mktick(2026, 0, 1),
					   mktick(2027, 0, 1) - 1);
	return n;
}

static long run_noop(long n) { return n; }
static long run_find_all(long n)
{ long r; for (r = 0; r < n; r++) entries_seen += find("standup", 1); return n; }
static long run_find_rare(long n)
{ long r; for (r = 0; r < n; r++) entries_seen += find("zebra", 1); return n; }
static long run_find_old(long n)
{ long r; for (r = 0; r < n; r++) entries_seen += find("zebra", 0); return n; }

/*
 * Wrap each case's run so that RPCs and entries are counted around the
 * timed part (reading the log is outside the timing: the counts are
 * taken before and after).
 */
#define WRAP(name)							\
	static long wrap_##name(long n)					\
	{								\
		long r0 = rpc_count(), ops;				\
		double s0 = server_cpu_ns();				\
		entries_seen = 0;					\
		ops = run_##name(n);					\
		note_stats(#name, r0 < 0 ? -1 : rpc_count() - r0,	\
			   s0 < 0 ? -1 : server_cpu_ns() - s0, ops,	\
			   entries_seen);				\
		return ops;						\
	}
WRAP(noop) WRAP(month) WRAP(week) WRAP(year) WRAP(find_all) WRAP(find_rare)
WRAP(find_old)

static const struct bench_case cases[] = {
	{ "noop", "base", "nothing: the logon baseline of a syscall count", 1,
	  NULL, wrap_noop, NULL },
	{ "month", "dtcm", "month view: list + read, 12 months of 2026", 1,
	  NULL, wrap_month, NULL },
	{ "week", "dtcm", "week view: list + read, 52 weeks of 2026", 1,
	  NULL, wrap_week, NULL },
	{ "year", "dtcm", "one range lookup for 2026, list + read", 1,
	  NULL, wrap_year, NULL },
	{ "find_all", "dtcm", "Find all \"standup\" (server match, 52 wk)", 1,
	  NULL, wrap_find_all, NULL },
	{ "find_rare", "dtcm", "Find all \"zebra\" (server match, 52 wk)", 1,
	  NULL, wrap_find_rare, NULL },
	{ "find_old", "dtcm", "Find all \"zebra\", old way (4 wk, no match)",
	  1, NULL, wrap_find_old, NULL },
};

static void
extra_json(FILE *f)
{
	fprintf(f, ",\n  \"calendar\": \"%s\",\n  \"data_version\": %d",
		cal_name, cal_version);
}

/*
 * The per-case RPC and entry counts are known only after the cases ran,
 * and bench_main() writes the extra members first: append them to the
 * JSON document it wrote.
 */
static void
append_stats_json(const char *file)
{
	FILE *f;
	char *buf;
	long len, end;
	int i;

	if (file == NULL || strcmp(file, "-") == 0 || nstats == 0 ||
	    (f = fopen(file, "r+")) == NULL)
		return;
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	if (len <= 0 || (buf = malloc(len)) == NULL) {
		fclose(f);
		return;
	}
	rewind(f);
	len = (long)fread(buf, 1, len, f);
	for (end = len - 1; end >= 0 && buf[end] != '}'; end--)
		;
	while (end > 0 && (buf[end - 1] == '\n' || buf[end - 1] == ' '))
		end--;
	free(buf);
	if (end <= 0) {
		fclose(f);
		return;
	}
	fseek(f, end, SEEK_SET);
	fprintf(f, ",\n  \"dtcm\": {");
	for (i = 0; i < nstats; i++) {
		fprintf(f, "%s\n    \"%s\": {\"entries_per_op\": %ld",
			i ? "," : "", stats[i].name, stats[i].entries);
		if (stats[i].rpcs >= 0)
			fprintf(f, ", \"rpcs_per_op\": %.2f", stats[i].rpcs);
		if (stats[i].srv_ns >= 0)
			fprintf(f, ", \"server_cpu_ns_per_op\": %.0f",
				stats[i].srv_ns);
		fprintf(f, "}");
	}
	fprintf(f, "\n  }\n}\n");
	fclose(f);
}

int
main(int argc, char **argv)
{
	const char *json = NULL;
	int rc, i, listing;

	init_time();
	if (argc > 1 && strcmp(argv[1], "populate") == 0)
		return populate(argc - 1, argv + 1);
	if (argc > 1 && strcmp(argv[1], "change") == 0)
		return change(argc - 1, argv + 1);
	if (argc > 1 && strcmp(argv[1], "dump") == 0)
		return dump(argc - 1, argv + 1);
	if (argc < 2 || strcmp(argv[1], "bench") != 0) {
		fprintf(stderr, "usage: dtcmbench populate [-c CAL] "
			"[-n ONETIME] [-R REPEATING]\n"
			"       dtcmbench change [-c CAL] [-n N] [-i MS] "
			"[-d M/D/YYYY]\n"
			"       dtcmbench dump [-c CAL] [-y YEAR] [-Y YEAR] "
			"[-f WORD]\n"
			"       dtcmbench bench [-r REPEAT] [-s SCALE] "
			"[-j FILE] [-l] [CASE...]\n");
		return 2;
	}
	cal_name = default_cal();
	for (i = 2; i < argc; i++)
		if (strcmp(argv[i], "-j") == 0 && i + 1 < argc)
			json = argv[i + 1];
		else if (strcmp(argv[i], "-l") == 0 ||
			 strcmp(argv[i], "-h") == 0)
			break;
	listing = i < argc;
	if (!listing) {
		logon(0);
		rpc_count();	/* read the log so far, outside the timing */
	}
	rc = bench_main("dtcmbench", cases, sizeof cases / sizeof cases[0],
			argc - 1, argv + 1, extra_json);
	append_stats_json(json);
	if (nstats) {
		printf("\n%-12s %12s %10s %16s\n", "case", "entries/op",
		       "RPCs/op", "server-cpu-ns/op");
		for (i = 0; i < nstats; i++) {
			printf("%-12s %12ld ", stats[i].name, stats[i].entries);
			if (stats[i].rpcs >= 0)
				printf("%10.2f ", stats[i].rpcs);
			else
				printf("%10s ", "-");
			if (stats[i].srv_ns >= 0)
				printf("%16.0f\n", stats[i].srv_ns);
			else
				printf("%16s\n", "-");
		}
	}
	if (!listing)
		csa_logoff(cal, NULL);
	return rc;
}
