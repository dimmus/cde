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
/*
 * tt_stats.C -- RPC counters for libtt (TT_STATS=1), see tt_stats.h.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "util/tt_stats.h"

int _tt_stats_state = 0;

// Procedure numbers: ttsession 0..48 and 400 (VRFY_SESSION, kept in the
// last slot); rpc.ttdbserver 1..24 (old ISAM interface) and 101..137.
#define SESSION_SLOTS	51
#define SESSION_VRFY	49
#define SESSION_OTHER	50
#define DB_SLOTS	140
#define DB_OTHER	139

struct _Tt_stats_slot {
	unsigned long	calls[2];	// [0] synchronous, [1] one-way
	unsigned long	failed[2];
	long long	ns[2];
};

static _Tt_stats_slot session_slots[SESSION_SLOTS];
static _Tt_stats_slot db_slots[DB_SLOTS];
static unsigned long connects[2][2];	// [iface][ok]

static const char * const session_names[SESSION_SLOTS] = {
	"NULLPROC", "DISPATCH", "UPDATE_MSG", "JOIN_FILE", "QUIT_FILE",
	"JOIN_SESSION", "QUIT_SESSION", "NEXT_MESSAGE", "ADD_PATTERN",
	"DEL_PATTERN", "DECLARE_PTYPE", "SET_FD_CHANNEL", "SET_PROP",
	"ADD_PROP", "GET_PROP", "PROP_COUNT", "PROP_NAME",
	"PROP_NAMES_COUNT", "ALLOC_PROCID_KEY", "CLOSE_PROCID",
	"OTYPE_DERIVEDS_COUNT", "OTYPE_DERIVED", "OTYPE_BASE",
	"OTYPE_IS_DERIVED", "OTYPE_OSIG_COUNT", "OTYPE_HSIG_COUNT",
	"OTYPE_OSIG_OP", "OTYPE_HSIG_OP", "OTYPE_OSIG_ARGS_COUNT",
	"OTYPE_HSIG_ARGS_COUNT", "OTYPE_OSIG_ARG_MODE",
	"OTYPE_HSIG_ARG_MODE", "OTYPE_OSIG_ARG_TYPE",
	"OTYPE_HSIG_ARG_TYPE", "HDISPATCH", "HUPDATE_MSG", "DISPATCH_2",
	"UPDATE_MSG_2", "MSGREAD_2", "DISPATCH_ON_EXIT",
	"UNDECLARE_PTYPE", "EXISTS_PTYPE", "UNBLOCK_PTYPE",
	"JOIN_CONTEXT", "QUIT_CONTEXT", "DISPATCH_WITH_CONTEXT",
	"DISPATCH_2_WITH_CONTEXT", "ADD_PATTERN_WITH_CONTEXT",
	"LOAD_TYPES", "VRFY_SESSION", "other"
};

static const char * const db_old_names[25] = {
	0, "MIN_AUTH_LEVEL", "ISADDINDEX", "ISBUILD", "ISCLOSE", "ISCNTL",
	"ISDELREC", "ISERASE", "ISOPEN", "ISREAD", "ISREWREC", "ISSTART",
	"ISWRITE", "TEST_AND_SET", "TRANSACTION", "MFS", "GETOIDACCESS",
	"SETOIDUSER", "SETOIDGROUP", "SETOIDMODE", "READSPEC", "WRITESPEC",
	"ADDSESSION", "DELSESSION", "GETTYPE"
};

static const char * const db_names[38] = {
	0, "GET_MIN_AUTH_LEVEL", "GET_FILE_PARTITION", "CREATE_FILE",
	"CREATE_OBJ", "REMOVE_FILE", "REMOVE_OBJ", "MOVE_FILE",
	"SET_FILE_PROPS", "SET_FILE_PROP", "ADD_FILE_PROP",
	"DELETE_FILE_PROP", "GET_FILE_PROP", "GET_FILE_PROPS",
	"GET_FILE_OBJS", "SET_FILE_ACCESS", "GET_FILE_ACCESS",
	"SET_OBJ_PROPS", "SET_OBJ_PROP", "ADD_OBJ_PROP", "DELETE_OBJ_PROP",
	"GET_OBJ_PROP", "GET_OBJ_PROPS", "SET_OBJ_TYPE", "GET_OBJ_TYPE",
	"SET_OBJ_FILE", "GET_OBJ_FILE", "SET_OBJ_ACCESS", "GET_OBJ_ACCESS",
	"IS_FILE_IN_DB", "IS_OBJ_IN_DB", "QUEUE_MESSAGE",
	"DEQUEUE_MESSAGES", "FILE_NETFILE", "NETFILE_FILE",
	"GET_ALL_SESSIONS", "GARBAGE_COLLECT", "DELETE_SESSION"
};

static const char *
db_name(int slot)
{
	if (slot >= 1 && slot <= 24) {
		return db_old_names[slot];
	}
	if (slot >= 101 && slot <= 137) {
		return db_names[slot - 100];
	}
	return "other";
}

static void
print_table(FILE *f, const char *iface, _Tt_stats_slot *slots, int n,
	    int dbtable)
{
	for (int i = 0; i < n; i++) {
		for (int w = 0; w < 2; w++) {
			_Tt_stats_slot *s = &slots[i];
			const char *name;

			if (s->calls[w] == 0) {
				continue;
			}
			name = dbtable ? db_name(i) : session_names[i];
			fprintf(f, "tt_stats: %-9s %-24s %-4s %8lu calls"
				" %6lu failed %10.3f ms %8.1f us/call\n",
				iface, name, w ? "1way" : "sync",
				s->calls[w], s->failed[w],
				s->ns[w] / 1e6,
				s->ns[w] / 1e3 / s->calls[w]);
		}
	}
}

static void
report()
{
	const char *where = getenv("TT_STATS");
	FILE *f = stderr;
	char comm[64];
	FILE *cf;

	if (where != 0 && where[0] == '/') {
		f = fopen(where, "a");
		if (f == 0) {
			return;
		}
	}
	comm[0] = '\0';
	cf = fopen("/proc/self/comm", "r");
	if (cf != 0) {
		if (fgets(comm, sizeof(comm), cf) == 0) {
			comm[0] = '\0';
		}
		fclose(cf);
		comm[strcspn(comm, "\n")] = '\0';
	}
	fprintf(f, "tt_stats: pid %ld %s\n", (long)getpid(), comm);
	fprintf(f, "tt_stats: %-9s connections %lu (%lu failed)\n",
		"ttsession", connects[0][0] + connects[0][1], connects[0][0]);
	fprintf(f, "tt_stats: %-9s connections %lu (%lu failed)\n",
		"dbserver", connects[1][0] + connects[1][1], connects[1][0]);
	print_table(f, "ttsession", session_slots, SESSION_SLOTS, 0);
	print_table(f, "dbserver", db_slots, DB_SLOTS, 1);
	if (f != stderr) {
		fclose(f);
	} else {
		fflush(f);
	}
}

int
_tt_stats_init()
{
	const char *v = getenv("TT_STATS");

	if (v == 0 || v[0] == '\0' || strcmp(v, "0") == 0) {
		_tt_stats_state = 1;
		return 0;
	}
	_tt_stats_state = 2;
	atexit(report);
	return 1;
}

long long
_tt_stats_start()
{
	struct timespec ts;

	if (!_tt_stats_on()) {
		return 0;
	}
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void
_tt_stats_call(int iface, unsigned long proc, int oneway, long long t0,
	       int status)
{
	_Tt_stats_slot *s;
	struct timespec ts;
	int w = oneway ? 1 : 0;

	if (!_tt_stats_on()) {
		return;
	}
	if (iface == _TT_STATS_DBSERVER) {
		s = &db_slots[proc < DB_SLOTS ? proc : DB_OTHER];
	} else if (proc == 400) {
		s = &session_slots[SESSION_VRFY];
	} else {
		s = &session_slots[proc < SESSION_VRFY ? proc : SESSION_OTHER];
	}
	clock_gettime(CLOCK_MONOTONIC, &ts);
	s->calls[w]++;
	if (status != 0) {
		s->failed[w]++;
	}
	if (t0 != 0) {
		s->ns[w] += (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec
			    - t0;
	}
}

void
_tt_stats_connect(int iface, int ok)
{
	if (!_tt_stats_on()) {
		return;
	}
	connects[iface ? 1 : 0][ok ? 1 : 0]++;
}
