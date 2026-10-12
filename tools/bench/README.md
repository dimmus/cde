# CDE benchmarks

Harnesses for the "Measure first" phase of `TODO.md` (Phase 0): they
time the hot paths the audit found, count X requests, X round trips and
mallocs, and write JSON so that two builds can be compared.

This directory is **not** part of the automake build.  Its plain
`Makefile` links the programs against the libraries of this build tree
(`lib/*/.libs`, by relative path with an rpath), so that they measure the
tree rather than an installed CDE; the scripts run the tree's uninstalled
binaries (`programs/*/.libs/*`) with the tree's libraries on
`LD_LIBRARY_PATH` (see `cdeenv.sh`).  The tree must be configured and
built first.

```sh
make -C tools/bench                 # build (needs pkg-config, X11, Xt, Xm, Xtst)
tools/bench/bench.py run -o out.json          # everything, median of 5
tools/bench/bench.py run --syscalls -o out.json   # and system call counts
tools/bench/bench.py run --quick dtsvc tt     # a quick look at two groups
tools/bench/bench.py compare base.json out.json   # exit 1 on a regression
tools/bench/bench.py profile dtwm map         # perf profile of one case
```

Requirements at run time: `Xvfb`, `xdotool`, `xdpyinfo`, `python3`,
`less`, `taskset` (optional), `strace` (`--syscalls`, `-S`), `perf`
(`profile`), and the CDE action/datatype database installed under
`/usr/dt` or `/etc/dt` (dtsvcbench, the front panel of fp-switch).
`ci/bench.yml` is a GitHub Actions workflow that builds the base and
the head of a change on one runner and runs `bench.py compare` on them;
it is meant to be copied to `.github/workflows/`.

## What is here

| File | What it does |
|---|---|
| `preload.c` → `libcdebench_preload.so` | LD_PRELOAD counters: malloc/calloc/moving realloc calls, X requests (the `requests` argument of `xcb_writev`, i.e. the XNextRequest delta of every display) and round trips (waits in `xcb_wait_for_reply*` after requests were sent: what counting `_XReply` gives, also where libX11 is `-Bsymbolic-functions`).  The benchmarks read them in-process; any program reports its totals at exit with `CDEBENCH_REPORT=FILE` (or `1` for stderr), one line per process with its pid and ppid; a forked child counts from zero, `CDEBENCH_REPORT_SIGTERM=1` also reports on a default-action SIGTERM, and `CDEBENCH_REPORT_SIGNAL=N` makes signal N append the totals so far (`event=signal`) without stopping, for the counts of one action as the difference of two snapshots.  `CDEBENCH_REPLY_BACKTRACE=1` prints a backtrace per round trip. |
| `benchutil.[ch]` | The harness of the C benchmarks: case table, `-r REPEAT` (median, default 5), `-s SCALE`, `-j FILE` JSON, re-exec with the preload library; with `CDEBENCH_MARKERS` set, every timed run is framed by markers (a failing `write(-1, "cdebench:begin CASE")`) that `syscount.py` splits strace logs by. |
| `syscount.py` | System calls from `strace -f -ttt -Y` logs: per process name and call (`syscount.py run -- CMD`, like `strace -f -c`), per case of a log with markers (`cases`, with the X server's log split by the markers' times), per time window (`summary --from --to`).  perf trace and the syscall tracepoints need tracefs and BPF, which `perf_event_paranoid=2` does not give a user, so strace it is: exact counts, meaningless times. |
| `perf-profile.sh` | `perf record -e cpu-clock:u` (no hardware counters in this VM, user space only) with DWARF call graphs of a command and its children: `report.txt` by process/object/symbol, `stacks.folded` for flame graphs, the top symbols on stdout (`-c COMM` for one process). |
| `dtsvcbench.c` | libDtSvc/libDtWidget: `_DtDtsMMInit(0)` with a valid and a stale dtdbcache, `DtDtsDataToDataType` per entry of a generated 10k-entry tree (or `CDEBENCH_DTS_DIR`, e.g. an sshfs mount), `DtDtsDataTypeToAttributeValue`, `DtActionLabel`/`Icon`/`Exists`, `XeSPCSpawn("/bin/true")` under `RLIMIT_NOFILE` 1024 and the hard limit, `DtComboBoxAddItem`/`DtSpinBoxAddItem` ×1k, DtEditor: 1 MB with ~100k NUL runs, Replace All with 10k hits, 10k cursor moves with the status line. |
| `ttbench-rpcs.py` | libtt's RPCs per operation of each ttbench case, from `TT_STATS` tables: each case runs at two scales and the difference cancels startup and teardown (`rpcs_per_op`, sync and one-way, per procedure). |
| `ttbench.c` | ToolTalk under a private `ttsession -s -c`: `tt_open`/`tt_close`, `ttdt_open`+`ttdt_session_join`, request/reply ping, a burst of notices, 1k and 10k patterns over 50 procids, ping with 10k patterns registered, `tt_file_netfile`, `tt_message_file_set`, `TT_FILE_IN_SESSION` notices, `TT_FILE` notices (with `CDEBENCH_TTDBSERVER=1` and a running rpc.ttdbserver). |
| `dtwmbench.c`, `dtwmpreload.c`, `dtwmbench.h` | Motif's `mwmbench` ported to dtwm (front panel off, no ToolTalk: `TTSESSION_CMD` keeps libtt from starting the first `ttsession` on PATH): map 500 clients (also as WM_CLASS Dtterm, which makes dtwm look up an icon image), destroy, 10k `WM_NAME`/`_NET_WM_NAME` changes, 10k urgency-hint toggles, opaque/outline/XOR move and resize drags through XTest; 1000 workspace switches with 100 clients over 4 workspaces (`ws-switch`: `_NET_CURRENT_DESKTOP`; `fp-switch`, `fp-switch-tt`: XTest clicks on the front panel's switch buttons, found by the probe through `XtAddCallback`, without and with the tree's ttsession); 100 fill/fit backdrop changes as dtstyle sends them (with the X server's RSS growth); `f.refresh` with 200 clients, `refreshByClearing` True and False; 3 s held still in a move (opaque, XOR outline, window outline, and XOR outline with `freezeOnConfig`, where the probe releases the button from inside dtwm since the server is grabbed).  Reports dtwm's own CPU, mallocs, requests, round trips and wakeups (context switches) per operation, and the X server's CPU. |
| `ewmhtest.c` | Functional tests of dtwm's EWMH support (not a benchmark): starts the tree's dtwm like dtwmbench (front panel off unless `-f`, more resources with `-x`) on a display without a window manager, acts as an EWMH client and checks what dtwm publishes, waiting for the PropertyNotify of each write.  `ewmhtest all`, or a test: `clientlist` (`_NET_CLIENT_LIST`, `_NET_CLIENT_LIST_STACKING`), `active` (`_NET_ACTIVE_WINDOW`), `desktops` and `xdotool` (desktops and workspaces), `states` (`_NET_WM_STATE`), `layers` and `types` (stacking layers, `_NET_WM_WINDOW_TYPE`, checked against the real X stacking order), `struts`, `xinerama` and `panel` (struts, `_NET_WORKAREA`; `xinerama` needs two 1920x1080 heads, `panel` needs `-f`), `actions` (allowed actions, frame extents, decorations changed at run time), `requests` (close, move/resize, restack, show desktop), `moveresize` (`_NET_WM_MOVERESIZE`, driven through XTest), `icon` (`_NET_WM_ICON`), `usertime` and `resize` (the screen changing size through RandR: backdrops, front panel, windows and icons follow; `-s WxH` sets the size dtwm starts with).  Exits 1 on a failure. |
| `proxy.c` → `cdebench-proxy` | Motif's `xmbench-proxy`: an X proxy that adds latency per direction and counts requests, replies, events and round trips per connection (`cdebench-proxy -d 2 9 /tmp/.X11-unix/X0` then `DISPLAY=127.0.0.1:9`), to make round trips visible in wall time ("less page-down with 2 ms added delay"); also requests and round trips per request type (`ops`, `op_round_trips`: core requests by name, extensions by the name their QueryExtension reply gave), which is the `xtrace -c` count (the `xtrace` installed here is glibc's function tracer); SIGUSR1 appends the counts of the open connections. |
| `apptime.py` | Launch an X program, report the time to its first visible window (`map_s`), to idle (`idle_s`: no more CPU for `--idle-ms`), its CPU time and the preload's totals for that process until idle, plus its forked children's mallocs.  Options: `--strace` (system calls to idle, the children's apart), `--idle-window S` (wakeups, CPU, forks and system calls while idle), `--action L=KEYS` (type keys, time to idle again, the action's own counts), `--xops` (requests per type through the proxy), `--wm` (the tree's dtwm manages the display), `--ttsession`, `--log`. |
| `dtterm-throughput.sh`, `ttywrite.sh`, `ttycounts.py` | dtterm writing 100 MB of base64 and 16 MB of `yes` output, in `C` and UTF-8, with `-sl 4s` and `-sl 10000`; MB/s, requests and round trips (totals) and the output's own (`output_*`, from preload snapshots that `ttywrite.sh` triggers when dtterm is idle before and has drawn everything after, both found with cursor position reports), requests per 24-line screen. |
| `vtgen.py`, `dtterm-vtebench.sh` | vtebench-style streams (`dense_cells`, `scrolling`, `scrolling_in_region_top/bottom`, `unicode`, `cursor_motion`) written to an 80x24 dtterm one screen per write: screens/s, requests per screen, round trips per burst (target 0). |
| `dtterm-less.py` | `less` in dtterm behind `cdebench-proxy -d 2`: requests and round trips per page-down, and the time to dtterm's last CPU use after the key. |
| `dtfile-gendir.py`, `dtfile-time.sh` | A 1k/10k/100k-entry folder (the dtsvcbench mix), and dtfile's time to first paint and to idle on it, cold and warm, per view mode, with the tree's dtwm running (`-M`: without); `-w S` idle window, `-S` system calls (the forked folder readers apart), `-x` requests per type (GetGeometry, GetInputFocus, ClearArea...). |
| `dtfile-copy.sh` | `dtfile_copy` against `cp -r` on a 1 GiB file and on 10k 4 KiB files (seconds, MB/s, CPU; `-S` system calls), copies checked. |
| `perfbuild.py`, `dtperf-shim.h`, `dtfile-perf.sh` | `perfbuild.py` builds one program of the tree with extra flags outside the tree (it reads the generated Makefile's variables as text, so make never regenerates anything); `dtfile-perf.sh` builds dtfile with `-DDT_PERFORMANCE` (the shim makes that bit-rotted code compile) and reports its phase timers. |
| `mbox-gen.py`, `dtmail-time.sh` | A synthetic mbox (1k/10k/50k messages, no Content-Length, 30% multipart), dtmail's time to open it, what it does idle for 60 s (`-w`), then View > By Sender, By Size, By Date/Time, Message > Select All and Delete, each timed to idle with its own counts; `-S` system calls; the script checks that dtmail did not rewrite the mbox. |
| `startup-trace.h`, `startup-trace.ksh`, `startup-trace.py` | `CDE_STARTUP_TRACE=<file>` stamps: a header-only C function and a ksh function (`printf '%(%s.%N)T'`, no fork) that append `<time> <pid> <component> <event>`, and the summariser that prints T_greeter, T_desktop and T_restore. |
| `loader-cost.sh` | Per binary and library: DSOs loaded, unused direct dependencies (`ldd -u -r`), exported symbols, and startup relocations (`LD_DEBUG=statistics`). |
| `cc-trace.sh`, `cc-trace2json.py` | A `CC`/`CXX` wrapper that logs per-TU compile times, and its conversion to Chrome trace JSON with a parallelism summary. |
| `build-time.sh` | Build times in a clone of HEAD (never this checkout): configure, clean, no-op, `./config.status && make` (objects rebuilt); `-T` per-TU trace, `-R` `remake --profile`, `-c` ccache across two trees (hit rate of the second build). |
| `bench.py` | Runs the groups (`dtsvc tt dtwm dtterm dtterm-vte dtterm-less dtfile dtfile-copy dtmail loader`, and `dtfile-perf` on request), each on its own Xvfb (`-displayfd`, 1920x1080x24 +RENDER), pinned with `taskset -c 0` together with the program under test, writes one JSON document; `--syscalls` adds an strace pass whose counts are merged in; `compare` fails on a slowdown above 5%, on any increase in round trips, on 10% more system calls or ToolTalk RPCs, and on more DSOs, more unused dependencies or 5% more relocations (loader); `profile` runs a group under `perf-profile.sh`. |
| `ci/bench.yml` | The CI job (see above). |

## Startup trace stamp names

`startup-trace.py` computes the headline numbers from these stamps
(component, event); the instrumentation itself belongs in the programs
(dtlogin, dtgreet, Xsession, dtsession, dtwm), which call their own copy
of `startup-trace.h`:

| Number | From | To |
|---|---|---|
| T_greeter | `dtlogin server-ready` | `dtgreet first-expose` |
| T_desktop | `dtgreet auth-ok` | `dtwm frontpanel-mapped` (else `dtwm ready`) |
| T_restore | `dtgreet auth-ok` | last `dtsession client-mapped` (else `dtsession ready`) |

Other useful stamps, all printed in order with their offsets: `dtlogin
start/server-exec/sigusr1/greeter-fork`, `dtgreet main-loop`, `Xstartup
done`, `Xsession start/<section>`, `dtdbcache start/exit`, `ttsession
start/exit`, `dtsession start/resources/display-open/colors/wm-fork/
wm-ready/client-fork/ready`.  ksh stamps use `CLOCK_REALTIME`; run
`startup-trace.py --stamp-clock FILE` once (for example from Xsetup) so
that they can be aligned with the C stamps' `CLOCK_MONOTONIC`.

## Notes on the method

- Per-operation numbers of the C benchmarks are medians over the runs;
  `requests` and `rtrips` are exact counts, so they are what a CI gate
  should hold (`bench.py compare` fails on any increase).
- On a 2-CPU virtual machine the wake-up latency between the client,
  dtwm/ttsession and the X server dominates wall times unless they share
  a CPU; `bench.py` pins them all to CPU 0.  Other load on that CPU shows
  up in the wall times: compare CPU times (`cpu_ns_per_op`) too.
- System calls are counted with strace in a pass of their own
  (`--syscalls`): strace stops the traced process at every call, so the
  counts are exact but the times of that pass are thrown away.  The
  counts of one run are not stable, though: how often a process wakes
  up and finds nothing to read depends on the scheduling (one run of
  ttbench `burst` makes 9 calls per notice, the next 18), so the C
  benchmarks run 5 times in that pass and the median run counts.  RPCs
  per operation, as the difference of two runs, vary by an RPC per run
  too: `compare` wants half an RPC per operation more before it fails.
  perf
  works for profiles (`cpu-clock:u` sampling; no hardware counters in
  this VM, no kernel samples with `perf_event_paranoid=2`) but not for
  `perf trace` (no tracefs, no BPF).  Idle wakeups are context switches
  from `/proc/PID/task/*/status`, which needs neither.
- dtfile without a window manager sleeps 2 s at startup: `Desktop.c`
  retries `DtWsmGetWorkspaceList` and sleeps after the last failed try
  even with `retryLoadDesktop: 0`.  The first-window times of the old
  baseline (2.1 s whatever the folder) were that; dtfile-time.sh now
  starts the tree's dtwm first (`-M` for the old way).
- Before dtwmbench set `TTSESSION_CMD`, every dtwm it started made libtt
  start the first `ttsession` on PATH (the installed one), which then
  outlived it; the old dtwm numbers were taken with that ttsession.
- The spc cases fork a child per run (the SPC library reads the fd limit
  once); the child waits for `/bin/true` itself, since an SPC client is
  never told when a local child ends (only dtspcd installs a SIGCHLD
  handler).  The time per spawn therefore includes the child closing
  every descriptor up to the limit before it execs (TODO item 7).
- dtwm runs as the real binary (`programs/dtwm/.libs/dtwm`) with
  `LD_LIBRARY_PATH`; through the libtool wrapper script the idle probe
  never fires.
- The DtEditor cases keep their two editors (status line on and off)
  and only unmanage them between runs: destroying a DtEditor created
  with `showStatusLine: False` crashes in libXm's drop-site tree update
  (`IntersectWithWidgetAncestors` from `TreeUpdateHandler`) on the next
  pass of the event loop.
- ToolTalk: `tt_close()` of one of several procids opened by a process
  invalidates the others (`tt_default_procid_set` of the first then
  fails with `TT_ERR_PROCID`), so ttbench runs the cases that open and
  close procids in fresh processes, which report their own times.

## Baseline

One run of `bench.py run -r 5 --script-rounds 1 --syscalls -o
baseline.json` (all default groups; GCC 16, `-O2`, 2-CPU virtual machine
shared with other builds and benchmarks, everything pinned to CPU 0,
Xvfb 1920x1080x24), plus `dtfile-perf.sh` (merged into `baseline.json`
as the dtfile-perf group).  The tree is this branch during the second
round of TODO fixes, so the numbers do not compare with the first
baseline of this directory (`git log -p baseline.json` keeps it): dtwm
no longer runs with a ttsession, dtfile runs with a window manager,
dtterm's counts are those of the output alone, and many fixes landed in
between.  The dtfile-copy times are from a second run: in the first,
right after 1 GiB had been written to tmpfs, `cp` took 6.1 s (5.6 s of
system time) for what takes 0.44 s when repeated; the system call
counts are from the first run's strace pass.  The ten-minute idle
windows of the TODO were run on their own (below).

What stands out:

- System calls the timings did not show: `_DtDtsMMInit` with a valid
  cache makes 119 (93 `newfstatat` validating the database files); a
  ToolTalk ping costs 74 over the three processes, and 3 RPCs (2
  synchronous `NEXT_MESSAGE`); `ttdt_open` + `ttdt_session_join` costs
  32 RPCs (12 `ADD_PATTERN`, 12 `DEL_PATTERN`).  dtmail stats every
  message three times per sort: 30k `newfstatat` for 10k messages.
- dtwm: `f.refresh` with 200 clients costs 24 ms, 666 wakeups, 5,885
  requests and 16.9k X server system calls per refresh with
  `refreshByClearing` (the default), against 5.3 ms, 59 wakeups and 163
  server system calls with the cover window (TODO 4.4).  Holding still
  in an XOR-outline move wakes dtwm 146 times in 3 s (the flashing); the
  other modes 9-11 times.  A workspace switch with 100 clients costs
  6.4 ms (1 ms of dtwm CPU, 4.4 ms of the X server's) and no round trip;
  a front-panel click costs about the same, with or without a ttsession.
- dtterm draws 2,300-6,200 screens/s of scrolling text with 0.3-1.1
  requests per screen and no round trip, but `dense_cells` (a new
  256-colour pair in every cell) makes 0.13 round trips per screen
  (colour allocation); a `less` page-down over a 2 ms link costs 38
  requests and 0.08 round trips.
- dtfile on a 10k-entry folder: window in 0.1 s, idle in 0.4 s, 869
  round trips; its forked readers make 33k of its 51k system calls.
  Without a window manager it first sleeps 2 s (see the method notes).
- dtfile_copy copies as fast as `cp` (both use `copy_file_range`).

Tree `2.5.2-369-g99a05633b-dirty`, 2026-10-11, 5 rounds (C benchmarks),
1 (scripts); `bench.py report baseline.json` prints these tables.

### dtsvc

| case | n | ns_per_op | cpu_ns_per_op | mallocs_per_op | requests_per_op | round_trips_per_op | syscalls_per_op |
|---|---:|---:|---:|---:|---:|---:|---:|
| mm-init-valid | 20 | 103,896 | 101,331 | 53.00 | 0 | 0 | 119.0 |
| mm-init-stale | 5 | 10,773,016 | 10,257,352 | 39,857 | 0 | 0 | 688.0 |
| dts-type | 10,000 | 9,225 | 8,495 | 20.93 | 0 | 0 | 1.55 |
| dts-attr | 100,000 | 99.50 | 96.12 | 0.359 | 0 | 0 | 0 |
| action-label | 100,000 | 127.9 | 124.0 | 1.60 | 0 | 0 | 0.001 |
| action-icon | 100,000 | 115.0 | 111.1 | 0.8 | 0 | 0 | 0 |
| action-exists | 100,000 | 35.60 | 35.11 | 0 | 0 | 0 | 0 |
| spc-spawn-nofile-1k | 100 | 2,098,900 | 3,344 | 0 | 0 | 0 | 87.72 |
| spc-spawn-nofile-max | 100 | 2,034,774 | 2,040 | 0 | 0 | 0 | 87.72 |
| combobox-additem | 1,000 | 12,497 | 8,185 | 3.61 | 12.47 | 0.001 | 0.193 |
| spinbox-additem | 1,000 | 13,220 | 8,449 | 19.03 | 28.00 | 0.002 | 0.153 |
| editor-set-nul | 1 | 3,867,463 | 3,635,238 | 8.00 | 34.00 | 1.00 | 27.00 |
| editor-replace-all | 10,000 | 185.6 | 174.8 | 1.00 | 0.0014 | 0.0001 | 0.005 |
| editor-cursor-status | 10,000 | 327,226 | 68,957 | 46.01 | 401.0 | 0.0062 | 12.04 |

### tt

| case | n | ns_per_op | cpu_ns_per_op | mallocs_per_op | rpcs_per_op | syscalls_per_op |
|---|---:|---:|---:|---:|---:|---:|
| open-close | 200 | 360,289 | 157,490 | 199.4 | 4.00 | 125.0 |
| ttdt-open-join | 50 | 2,222,226 | 840,623 | 1,034 | 32.00 | 463.9 |
| ping | 10,000 | 183,652 | 39,766 | 57.00 | 2.98 | 73.80 |
| burst | 1,000 | 23,283 | 7,278 | 22.06 | 1.00 | 16.70 |
| patterns-1k | 1,000 | 75,711 | 29,193 | 37.56 | 1.00 | 14.69 |
| patterns-10k | 10,000 | 70,120 | 22,444 | 38.31 | 1.00 | 12.30 |
| ping-10k-patterns | 1,000 | 247,189 | 48,916 | 57.00 | 2.98 | 73.75 |
| netfile | 100 | 5,043 | 4,649 | 72.00 | -0.05 | 5.54 |
| message-file-set | 100 | 3,474 | 3,492 | 34.00 | 0 | 5.44 |
| file-notice | 100 | 53,963 | 10,910 | 34.57 | 1.00 | 29.81 |

### dtwm

| case | n | ns_per_op | cpu_ns_per_op | server_cpu_ns_per_op | mallocs_per_op | requests_per_op | round_trips_per_op | wakeups_per_op | syscalls_per_op | server_syscalls_per_op |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| map | 500 | 1,538,598 | 258,441 | 1,104,242 | 109.1 | 94.03 | 9.00 | 19.13 | 106.8 | 78.06 |
| map-dtterm | 500 | 1,377,378 | 187,015 | 1,025,364 | 108.9 | 94.03 | 9.00 | 18.76 | 126.5 | 74.51 |
| hints-urgency | 10,000 | 12,682 | 6,512 | 5,920 | 7.00 | 1.00 | 1.00 | 1.10 | 9.61 | 7.89 |
| destroy | 500 | 148,911 | 28,981 | 101,732 | 8.47 | 5.34 | 0.006 | 2.42 | 12.65 | 6.59 |
| title | 10,000 | 1,034 | 462.4 | 509.4 | 2.00 | 0.0021 | 0.0004 | 0.0278 | 0.557 | 0.74 |
| title-paced | 2,000 | 78,017 | 31,391 | 31,871 | 15.00 | 6.00 | 1.00 | 2.13 | 39.94 | 19.91 |
| title-net | 10,000 | 867.7 | 356.4 | 394.7 | 2.00 | 0.0021 | 0.0004 | 0.0032 | 0.856 | 1.08 |
| drag-move | 500 | 72,636 | 20,948 | 43,461 | 5.20 | 6.45 | 1.01 | 2.74 | 27.40 | 19.43 |
| drag-move-outline | 500 | 143,919 | 32,551 | 96,665 | 6.46 | 20.26 | 1.01 | 2.93 | 27.31 | 19.21 |
| drag-resize | 500 | 156,247 | 35,067 | 104,130 | 5.39 | 17.74 | 1.01 | 2.68 | 27.19 | 18.93 |
| drag-move-xor | 500 | 56,016 | 17,920 | 30,556 | 5.05 | 7.37 | 1.01 | 2.35 | 24.37 | 19.63 |
| ws-switch | 1,000 | 6,424,732 | 1,031,796 | 4,431,553 | 372.7 | 320.9 | 0.002 | 63.74 | 344.9 | 162.0 |
| fp-switch | 1,000 | 5,706,888 | 1,324,897 | 3,719,496 | 753.0 | 381.3 | 0 | 61.62 | 488.7 | 167.6 |
| fp-switch-tt | 1,000 | 6,111,390 | 1,494,488 | 3,788,450 | 1,027 | 385.2 | 0 | 59.11 | 489.8 | 170.0 |
| backdrop | 100 | 1,772,291 | 304,592 | 1,371,308 | 126.1 | 29.01 | 9.00 | 20.41 | 91.19 | 76.67 |
| refresh | 50 | 23,881,130 | 11,135,055 | 11,804,884 | 14,864 | 5,885 | 1.00 | 665.6 | 16,930 | 16,869 |
| refresh-cover | 50 | 5,342,140 | 967,492 | 4,123,608 | 705.8 | 610.0 | 1.00 | 58.76 | 430.2 | 162.9 |
| hold-opaque | 1 | 3,003,989,010 | 1,190,867 | 1,960,765 | 25.00 | 36.00 | 3.00 | 9.00 | 83.00 | 58.00 |
| hold-xor | 1 | 3,001,416,308 | 19,821,204 | 21,902,339 | 27.00 | 153.0 | 3.00 | 146.0 | 715.0 | 595.0 |
| hold-window-outline | 1 | 3,001,506,371 | 259,602 | 622,958 | 34.00 | 61.00 | 3.00 | 10.00 | 102.0 | 65.00 |
| hold-freeze | 1 | 3,001,607,050 | 274,326 | 380,364 | 33.00 | 43.00 | 5.00 | 11.00 | 111.0 | 76.00 |

### dtterm

| case | bytes | seconds | mb_per_s | output_requests | output_round_trips | requests_per_screen |
|---|---:|---:|---:|---:|---:|---:|
| cat-base64-C-sl4s | 106,237,306 | 4.94 | 20.50 | 14,146 | 0 | 0.246 |
| yes-C-sl4s | 16,777,216 | 10.79 | 1.48 | 30,717 | 0 | 0.088 |
| cat-base64-C-sl10000 | 106,237,306 | 4.94 | 20.51 | 14,426 | 0 | 0.251 |
| yes-C-sl10000 | 16,777,216 | 11.36 | 1.41 | 32,301 | 0 | 0.092 |
| cat-base64-utf8-sl4s | 106,237,306 | 5.39 | 18.78 | 15,630 | 0 | 0.272 |
| yes-utf8-sl4s | 16,777,216 | 9.26 | 1.73 | 26,210 | 0 | 0.075 |
| cat-base64-utf8-sl10000 | 106,237,306 | 4.71 | 21.49 | 13,830 | 0 | 0.241 |
| yes-utf8-sl10000 | 16,777,216 | 9.39 | 1.70 | 26,855 | 0 | 0.077 |

### dtterm-vte

| case | screens | seconds | screens_per_s | requests_per_screen | round_trips_per_burst |
|---|---:|---:|---:|---:|---:|
| vte-dense_cells | 200 | 0.453 | 441.1 | 67.88 | 0.134 |
| vte-scrolling | 200 | 0.085 | 2,367 | 1.09 | 0 |
| vte-scrolling_in_region_top | 200 | 0.032 | 6,212 | 0.27 | 0 |
| vte-scrolling_in_region_bottom | 200 | 0.044 | 4,497 | 0.7 | 0 |
| vte-unicode | 200 | 0.049 | 4,047 | 0.84 | 0 |
| vte-cursor_motion | 200 | 0.184 | 1,088 | 1.21 | 0 |

### dtterm-less

| case | pages | seconds | requests_per_page | round_trips_per_page |
|---|---:|---:|---:|---:|
| less-pagedown-2ms | 50 | 0.0448 | 38.40 | 0.08 |

### dtfile

| case | map_s | idle_s | cpu_s | mallocs | requests | round_trips | syscalls | children_syscalls | idle_wakeups |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| dtfile-1000-by_name_and_icon-cold | 0.15 | 0.212 | 0.1 | 114,847 | 5,850 | 591 | 10,642 | 3,592 | - |
| dtfile-1000-by_name_and_icon-warm | 0.171 | 0.226 | 0.1 | 114,833 | 5,850 | 591 | 11,126 | 3,590 | 1 |
| dtfile-10000-by_name_and_icon-cold | 0.104 | 0.439 | 0.31 | 483,457 | 33,684 | 869 | 50,820 | 33,363 | - |
| dtfile-10000-by_name_and_icon-warm | 0.107 | 0.375 | 0.26 | 483,472 | 33,684 | 869 | 50,874 | 33,362 | 1 |

### dtfile-copy

| case | bytes | seconds | mb_per_s | user_s | sys_s | syscalls |
|---|---:|---:|---:|---:|---:|---:|
| copy-big-cp | 1,073,741,824 | 0.441 | 2,322 | 0.001 | 0.426 | 164 |
| copy-big-dtfile_copy | 1,073,741,824 | 0.485 | 2,111 | 0.011 | 0.485 | 2,915 |
| copy-small-cp | 40,960,000 | 0.124 | 315.0 | 0.01 | 0.103 | 100,954 |
| copy-small-dtfile_copy | 40,960,000 | 0.121 | 322.8 | 0.025 | 0.118 | 105,219 |

### dtmail

| case | map_s | idle_s | seconds | cpu_s | mallocs | requests | round_trips | syscalls | idle_wakeups |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| dtmail-1000 | 0.121 | 0.18 | - | 0.1 | 180,515 | 3,724 | 143 | 8,698 | 39 |
| dtmail-1000-sort-sender | - | - | 0.0123 | 0.03 | 50,743 | 2,373 | 22 | 3,612 | - |
| dtmail-1000-sort-size | - | - | 0.034 | 0.03 | 43,127 | 1,859 | 17 | 3,458 | - |
| dtmail-1000-sort-date | - | - | 0 | 0.01 | 40,378 | 2,418 | 17 | 3,550 | - |
| dtmail-1000-select-all | - | - | 0 | 0.01 | 4,784 | 9,001 | 23 | 586 | - |
| dtmail-1000-delete | - | - | 0 | 0 | 5,600 | 1,877 | 19 | 571 | - |
| dtmail-10000 | 0.16 | 0.906 | - | 0.56 | 1,175,415 | 3,724 | 143 | 54,414 | 23 |
| dtmail-10000-sort-sender | - | - | 0.173 | 0.16 | 489,210 | 2,349 | 22 | 30,577 | - |
| dtmail-10000-sort-size | - | - | 0.164 | 0.15 | 418,606 | 1,847 | 17 | 30,485 | - |
| dtmail-10000-sort-date | - | - | 0.195 | 0.18 | 388,858 | 2,402 | 17 | 30,553 | - |
| dtmail-10000-select-all | - | - | 0.83 | 0.07 | 31,763 | 72,192 | 22 | 916 | - |
| dtmail-10000-delete | - | - | 0 | 0.01 | 41,483 | 2,345 | 14 | 486 | - |

### loader

| case | dsos | unused | exported | relocations |
|---|---:|---:|---:|---:|
| programs/dtprintinfo/.libs/dtprintinfo | 56 | 0 | 22 | 18,148 |
| programs/ttsnoop/.libs/ttsnoop | 44 | 0 | 45 | 12,664 |
| programs/dtcm/dtcm/.libs/dtcm | 44 | 0 | 34 | 12,664 |
| programs/dtmail/dtmail/.libs/dtmail | 43 | 0 | 34 | 12,615 |
| programs/dtwm/.libs/dtwm | 43 | 0 | 25 | 12,484 |
| programs/dtfile/.libs/dtfile | 42 | 0 | 30 | 12,470 |
| programs/dtsession/.libs/dtsession | 43 | 0 | 11 | 12,349 |
| programs/dtpad/.libs/dtpad | 43 | 0 | 13 | 12,347 |
| programs/dtcm/dtcm/.libs/dtcm_editor | 43 | 0 | 28 | 12,341 |
| programs/dtksh/.libs/dtksh | 42 | 0 | 64 | 12,334 |
| programs/dtstyle/.libs/dtstyle | 42 | 0 | 10 | 12,209 |
| programs/dtcreate/.libs/dtcreate | 42 | 0 | 26 | 12,180 |
| programs/dticon/.libs/dticon | 42 | 0 | 28 | 12,160 |
| programs/dtterm/.libs/dtterm | 43 | 0 | 7 | 12,068 |
| programs/dtpdm/.libs/dtpdm | 42 | 0 | 18 | 11,785 |

### dtfile-perf (`-DDT_PERFORMANCE` phase timers, dtwm running)

| phase (s) | dtfile-perf-1000 | dtfile-perf-10000 |
|---|---:|---:|
| XtInitialize | 0.0128 | 0.0342 |
| InitializeToolTalkProcid | 0.0018 | 0.0056 |
| TrashCreateDialog | 0.0253 | 0.0409 |
| Setting up Desktop | 0.0017 | 0.0026 |
| Bringing up view | 0.0227 | 0.0217 |
| Finish | 0.0869 | 0.1402 |
| read directory | 0.0230 | 0.2543 |
| LayoutFileIcons | 0.0063 | 0.0052 |
| part 1, DisplaySomeIcons | 0.0190 | 0.2470 |
| part 2 DisplaySomeIcons | 0.0054 | 0.0793 |

Without a window manager (`dtfile-perf.sh -M`) "Setting up Desktop"
takes 2.002-2.007 s: the sleep in `Desktop.c`.

### Idle for 10 minutes

`dtmail-time.sh -n 10000 -w 600 -A` and `dtfile-time.sh -n 10000 -w 600`
(the warm run), each left alone for 600 s once idle:

| program | wakeups | per minute | CPU s | forks |
|---|---:|---:|---:|---:|
| dtmail, 10k messages | 85 | 8.5 | 0.03 | 0 |
| dtfile, 10k entries | 60 | 6.0 | 0.04 | 1 |

dtfile's one fork is a folder reader: it re-reads the folder
periodically.

### Profiles

`bench.py profile dtwm map ws-switch refresh` collected 758 user-space
samples of dtwm at 999 Hz (most of dtwm's time in these cases is in the
kernel, which `perf_event_paranoid=2` does not sample): 21% of them
under `F_Refresh` → `RefreshByClearing`, then `XFindContext` (3.4%),
`malloc` (2.8%), `ChangeToWorkspace` (1.7%).  A profile of `map` alone
put 10% of its samples in `XmeGetHomeDirName`, which `BitmapPathName`
calls for every client with an icon image.

### Build

Not run here: `build-time.sh` builds a clone from scratch (autogen,
configure and a full make: 784 s on this machine, see TODO.md), which
the rules of this round forbid while other work shares the machine.
remake is not installed (`-R` says so); ccache is (`-c`).
