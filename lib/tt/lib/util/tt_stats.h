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
 * tt_stats.h -- RPC counters for libtt, enabled by TT_STATS.
 *
 * With TT_STATS=1 in the environment, libtt counts the RPCs it makes
 * to ttsession and to rpc.ttdbserver (per procedure, synchronous and
 * one-way separately, with the time spent in each) plus the
 * connections it opens, and prints a table to stderr when the process
 * exits.  TT_STATS=/some/file appends the table to that file instead
 * (for processes whose stderr goes nowhere).  Without TT_STATS each
 * hook costs one test of a static flag.
 */
#ifndef _TT_STATS_H
#define _TT_STATS_H

enum _Tt_stats_iface {
	_TT_STATS_SESSION = 0,	// _Tt_rpc_client: ttsession, or a client
	_TT_STATS_DBSERVER = 1	// _Tt_db_client: rpc.ttdbserver
};

extern int _tt_stats_state;	// 0: not looked at yet, 1: off, 2: on
int _tt_stats_init();

inline int
_tt_stats_on()
{
	return _tt_stats_state == 2 ||
	       (_tt_stats_state == 0 && _tt_stats_init());
}

// Monotonic time in ns, for the start of a call (0 when stats are off).
long long _tt_stats_start();

// Count one call of procedure proc, started at t0 (from _tt_stats_start).
// A timeout of 0 means one-way (no reply awaited).  status is the
// clnt_stat the call returned (non-zero counts as a failure).
void _tt_stats_call(int iface, unsigned long proc, int oneway,
		    long long t0, int status);

// Count one connection attempt (ok is 0 if it failed).
void _tt_stats_connect(int iface, int ok);

#endif /* _TT_STATS_H */
