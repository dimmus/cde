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
tools/bench/bench.py run --quick dtsvc tt     # a quick look at two groups
tools/bench/bench.py compare base.json out.json   # exit 1 on a regression
```

Requirements at run time: `Xvfb`, `xdotool`, `xdpyinfo`, `python3`,
`taskset` (optional), and the CDE action/datatype database installed
under `/usr/dt` or `/etc/dt` (dtsvcbench).

## What is here

| File | What it does |
|---|---|
| `preload.c` → `libcdebench_preload.so` | LD_PRELOAD counters: malloc/calloc/moving realloc calls, X requests (the `requests` argument of `xcb_writev`, i.e. the XNextRequest delta of every display) and round trips (waits in `xcb_wait_for_reply*` after requests were sent: what counting `_XReply` gives, also where libX11 is `-Bsymbolic-functions`).  The benchmarks read them in-process; any program reports its totals at exit with `CDEBENCH_REPORT=FILE` (or `1` for stderr), one line per process with its pid and ppid; a forked child counts from zero, and `CDEBENCH_REPORT_SIGTERM=1` also reports on a default-action SIGTERM.  `CDEBENCH_REPLY_BACKTRACE=1` prints a backtrace per round trip. |
| `benchutil.[ch]` | The harness of the C benchmarks: case table, `-r REPEAT` (median, default 5), `-s SCALE`, `-j FILE` JSON, re-exec with the preload library. |
| `dtsvcbench.c` | libDtSvc/libDtWidget: `_DtDtsMMInit(0)` with a valid and a stale dtdbcache, `DtDtsDataToDataType` per entry of a generated 10k-entry tree (or `CDEBENCH_DTS_DIR`, e.g. an sshfs mount), `DtDtsDataTypeToAttributeValue`, `DtActionLabel`/`Icon`/`Exists`, `XeSPCSpawn("/bin/true")` under `RLIMIT_NOFILE` 1024 and the hard limit, `DtComboBoxAddItem`/`DtSpinBoxAddItem` ×1k, DtEditor: 1 MB with ~100k NUL runs, Replace All with 10k hits, 10k cursor moves with the status line. |
| `ttbench.c` | ToolTalk under a private `ttsession -s -c`: `tt_open`/`tt_close`, `ttdt_open`+`ttdt_session_join`, request/reply ping, a burst of notices, 1k and 10k patterns over 50 procids, ping with 10k patterns registered, `tt_file_netfile`, `tt_message_file_set`, `TT_FILE_IN_SESSION` notices, `TT_FILE` notices (with `CDEBENCH_TTDBSERVER=1` and a running rpc.ttdbserver). |
| `dtwmbench.c`, `dtwmpreload.c`, `dtwmbench.h` | Motif's `mwmbench` ported to dtwm (front panel off): map 500 clients (also as WM_CLASS Dtterm, which makes dtwm look up an icon image), destroy, 10k `WM_NAME`/`_NET_WM_NAME` changes, 10k urgency-hint toggles, opaque/outline move and resize drags through XTest.  Reports dtwm's own CPU, mallocs, requests and round trips per operation, and the X server's CPU. |
| `proxy.c` → `cdebench-proxy` | Motif's `xmbench-proxy`: an X proxy that adds latency per direction and counts requests, replies, events and round trips per connection (`cdebench-proxy -d 2 9 /tmp/.X11-unix/X0` then `DISPLAY=127.0.0.1:9`), to make round trips visible in wall time ("less page-down with 2 ms added delay"). |
| `apptime.py` | Launch an X program, report the time to its first visible window (`map_s`), to idle (`idle_s`: no more CPU for `--idle-ms`), its CPU time and the preload's totals for that process (also when it is stopped with SIGTERM, through `CDEBENCH_REPORT_SIGTERM`), plus its forked children's mallocs. |
| `dtterm-throughput.sh` | dtterm writing 100 MB of base64 and `yes \| head -c 16M`, in `C` and UTF-8, with `-sl 4s` and `-sl 10000`; MB/s, requests and round trips. |
| `dtfile-gendir.py`, `dtfile-time.sh` | A 1k/10k/100k-entry folder (the dtsvcbench mix), and dtfile's time to first paint and to idle on it, cold and warm, per view mode. |
| `mbox-gen.py`, `dtmail-time.sh` | A synthetic mbox (1k/10k/50k messages, no Content-Length, 30% multipart) and dtmail's time to open it; the script checks that dtmail did not rewrite it. |
| `startup-trace.h`, `startup-trace.ksh`, `startup-trace.py` | `CDE_STARTUP_TRACE=<file>` stamps: a header-only C function and a ksh function (`printf '%(%s.%N)T'`, no fork) that append `<time> <pid> <component> <event>`, and the summariser that prints T_greeter, T_desktop and T_restore. |
| `loader-cost.sh` | Per binary and library: DSOs loaded, unused direct dependencies (`ldd -u -r`), exported symbols, and startup relocations (`LD_DEBUG=statistics`). |
| `cc-trace.sh`, `cc-trace2json.py` | A `CC`/`CXX` wrapper that logs per-TU compile times, and its conversion to Chrome trace JSON with a parallelism summary. |
| `bench.py` | Runs the groups (`dtsvc tt dtwm dtterm dtfile dtmail loader`), each on its own Xvfb (`-displayfd`, 1920x1080x24 +RENDER), pinned with `taskset -c 0` together with the program under test, writes one JSON document; `compare` fails on a slowdown above 5% or on any increase in round trips. |

## Startup trace stamp names

`startup-trace.py` computes the headline numbers from these stamps
(component, event); the instrumentation itself belongs in the programs
(dtlogin, dtgreet, Xsession, dtsession, dtwm) and is not part of this
directory yet:

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
- strace and perf are not available here, so syscall counts are not
  collected; the scripts leave a place for them (`apptime.py` runs the
  command as given, so `strace -f -c -o FILE` can be prepended).
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

One run of `bench.py run -r 5 --script-rounds 1 -o baseline.json` on the
build at the time this directory was added (GCC 16, `-O2`, 2-CPU virtual
machine shared with other builds, everything pinned to CPU 0, Xvfb
1920x1080x24).  It is the state of the branch *after* the first fixes of
the TODO had landed (ToolTalk path mapping, dtterm read loop, dtwm
drags...), not of `a813c89ec`.  The raw numbers, with the 61 binaries of
the loader table, are in `baseline.json`.

What stands out:

- `XeSPCSpawn` costs 3 ms under `ulimit -n 1024` and **46 ms** under the
  hard limit (524288 here): the child closes every descriptor (TODO 3.2,
  top-20 #7).
- A stale dtdbcache costs 19 ms and 40k mallocs to rebuild in-process,
  against 0.11 ms to map and validate a good one.
- DtEditor: setting 1 MB of text with ~100k NUL runs takes **3.3 s** of
  CPU; each cursor move with the status line shown costs 401 X requests.
- dtterm writes 0.24-0.6 MB/s of `yes` output (one X request per byte of
  "y\n" lines), and 4-5 MB/s of base64; 10000 lines of scrollback make
  both 1.3-2.6 times slower.
- dtwm makes 18 round trips per mapped client (31 for WM_CLASS Dtterm,
  which looks up an icon image), 2 per `WM_NAME` change and 1 per
  `WM_HINTS` change.
- dtfile shows its first window 2.1 s after launch whatever the folder
  size; a 10k-entry folder then costs 1 s of CPU, 3.2M mallocs and
  10,844 round trips (about one per entry).
- ToolTalk: a request/reply round trip between two processes is 0.2 ms;
  registering a pattern costs 0.1 ms with 1k registered and 1 ms with
  10k, and most of it is the client's own CPU (0.05 then 0.87 ms per
  pattern): registration is linear in the patterns libtt already holds;
  `tt_open`+`tt_close` costs 2.3 ms.

Tree `775b066d3-dirty`, 2026-10-10T05:04:13, 5 rounds (C benchmarks), 1 (scripts), pinned: True.

`dts-type` was measured again later (same machine, less loaded: the old
binary then took 6.0 us per entry) after a fix to the generated tree:
its symlinks to files used to be broken, so 9% of the entries were
skipped but still counted.  Typing those links through their targets
makes it 21 mallocs per entry instead of 17.

### dtsvcbench

| case | n | ns/op | cpu ns/op | mallocs/op | requests/op | round trips/op |
|---|---:|---:|---:|---:|---:|---:|
| mm-init-valid | 20 | 112,513 | 111,133 | 42.00 | 0.00 | 0.000 |
| mm-init-stale | 5 | 19,246,325 | 18,472,962 | 39,850 | 0.00 | 0.000 |
| dts-type | 10,000 | 7,569 | 7,297 | 20.93 | 0.00 | 0.000 |
| dts-attr | 100,000 | 209 | 201 | 0.36 | 0.00 | 0.000 |
| action-label | 100,000 | 268 | 260 | 1.60 | 0.00 | 0.000 |
| action-icon | 100,000 | 198 | 193 | 0.80 | 0.00 | 0.000 |
| action-exists | 100,000 | 89.5 | 85.4 | 0.00 | 0.00 | 0.000 |
| spc-spawn-nofile-1k | 100 | 3,034,843 | 3,515 | 0.00 | 0.00 | 0.000 |
| spc-spawn-nofile-max | 100 | 46,157,646 | 2,524 | 0.00 | 0.00 | 0.000 |
| combobox-additem | 1,000 | 30,363 | 24,748 | 3.60 | 12.47 | 0.001 |
| spinbox-additem | 1,000 | 17,803 | 11,469 | 20.01 | 28.00 | 0.002 |
| editor-set-nul | 1 | 3,336,995,351 | 3,212,781,151 | 8.00 | 34.00 | 1.000 |
| editor-replace-all | 10,000 | 8,171 | 6,747 | 5.00 | 9.00 | 0.000 |
| editor-cursor-status | 10,000 | 386,180 | 82,314 | 46.01 | 401 | 0.006 |

### ttbench

| case | n | ns/op | cpu ns/op | mallocs/op | requests/op | round trips/op |
|---|---:|---:|---:|---:|---:|---:|
| open-close | 200 | 2,281,454 | 729,173 | 361 | 0.00 | 0.000 |
| ttdt-open-join | 50 | 4,099,493 | 1,368,145 | 1,182 | 0.00 | 0.000 |
| ping | 10,000 | 202,098 | 45,467 | 57.00 | 0.00 | 0.000 |
| burst | 1,000 | 60,474 | 4,773 | 22.06 | 0.00 | 0.000 |
| patterns-1k | 1,000 | 98,591 | 52,744 | 36.00 | 0.00 | 0.000 |
| patterns-10k | 10,000 | 1,059,240 | 871,989 | 36.00 | 0.00 | 0.000 |
| ping-10k-patterns | 1,000 | 222,608 | 41,996 | 57.00 | 0.00 | 0.000 |
| netfile | 100 | 5,055 | 4,738 | 72.00 | 0.00 | 0.000 |
| message-file-set | 100 | 3,805 | 3,817 | 34.00 | 0.00 | 0.000 |
| file-notice | 100 | 45,881 | 8,274 | 34.57 | 0.00 | 0.000 |

### dtwmbench

| case | n | ns/op | dtwm cpu ns/op | X cpu ns/op | mallocs/op | requests/op | round trips/op |
|---|---:|---:|---:|---:|---:|---:|---:|
| map | 500 | 1,922,517 | 583,998 | 1,227,978 | 151 | 103 | 18.000 |
| map-dtterm | 500 | 3,134,728 | 1,200,088 | 1,706,131 | 272 | 138 | 31.000 |
| hints-urgency | 10,000 | 15,559 | 8,269 | 6,805 | 7.00 | 1.00 | 1.000 |
| destroy | 500 | 195,261 | 42,358 | 130,528 | 14.96 | 7.11 | 0.006 |
| title | 10,000 | 42,445 | 21,423 | 19,108 | 15.00 | 6.00 | 2.000 |
| title-paced | 2,000 | 85,768 | 36,965 | 35,260 | 18.00 | 7.00 | 2.000 |
| title-net | 10,000 | 27,141 | 13,170 | 12,334 | 8.00 | 5.00 | 1.000 |
| drag-move | 500 | 132,123 | 31,592 | 90,073 | 5.21 | 6.45 | 1.006 |
| drag-move-outline | 500 | 61,779 | 18,923 | 34,230 | 5.05 | 7.29 | 1.006 |
| drag-resize | 500 | 91,538 | 25,378 | 52,790 | 5.10 | 7.29 | 1.008 |

### dtterm-throughput.sh

| case | MB | seconds | MB/s | requests | round trips |
|---|---:|---:|---:|---:|---:|
| cat-base64-C-sl4s | 101.3 | 18.85 | 5.37 | 2,760,954 | 89 |
| yes-C-sl4s | 16.0 | 25.82 | 0.62 | 16,770,905 | 337 |
| cat-base64-C-sl10000 | 101.3 | 25.53 | 3.97 | 2,766,209 | 81 |
| yes-C-sl10000 | 16.0 | 67.52 | 0.24 | 16,774,900 | 337 |
| cat-base64-utf8-sl4s | 101.3 | 18.86 | 5.37 | 2,764,523 | 82 |
| yes-utf8-sl4s | 16.0 | 26.13 | 0.61 | 16,778,487 | 338 |
| cat-base64-utf8-sl10000 | 101.3 | 24.44 | 4.15 | 2,763,879 | 84 |
| yes-utf8-sl10000 | 16.0 | 67.59 | 0.24 | 16,770,904 | 337 |

### dtfile-time.sh

| case | first window s | idle s | cpu s | mallocs | requests | round trips |
|---|---:|---:|---:|---:|---:|---:|
| dtfile-1000-by_name_and_icon-cold | 2.18 | 2.27 | 0.12 | 157,011 | 6,888 | 1,565 |
| dtfile-1000-by_name_and_icon-warm | 2.12 | 2.27 | 0.13 | 156,990 | 6,888 | 1,565 |
| dtfile-10000-by_name_and_icon-cold | 2.12 | 3.39 | 0.89 | 3,177,277 | 44,184 | 10,845 |
| dtfile-10000-by_name_and_icon-warm | 2.13 | 3.46 | 0.93 | 3,177,303 | 44,184 | 10,845 |

The dtfile rows were measured again after a fix to the preload's
reporting: the counters used to be those of whichever dtfile process
reported last, which was a forked folder-reading child carrying a copy
of the parent's counts from the time of the fork (85k mallocs, 1,276
requests and 91 round trips for 1k entries).  They are now the main
process's totals when it is stopped once idle; the forked children's
own mallocs are in `children_mallocs` of `baseline.json` (18,860 and
178,621).

### dtmail-time.sh

| case | first window s | idle s | cpu s | mallocs | requests | round trips |
|---|---:|---:|---:|---:|---:|---:|
| dtmail-1000 | 0.10 | 0.15 | 0.10 | - | - | - |
| dtmail-10000 | 0.10 | 0.74 | 0.58 | - | - | - |

### loader-cost.sh (programs with the most startup relocations)

| binary | DSOs | unused deps | relocations | from cache |
|---|---:|---:|---:|---:|
| programs/dtdocbook/infolib/.libs/NCFGen | 49 | 23 | 13,386 | 12,269 |
| programs/dtdocbook/infolib/.libs/MixedGen | 49 | 23 | 13,382 | 12,270 |
| programs/dtdocbook/infolib/.libs/StyleUpdate | 49 | 23 | 13,379 | 12,281 |
| programs/dtdocbook/infolib/.libs/NodeParser | 49 | 23 | 13,375 | 12,295 |
| programs/dtdocbook/infolib/.libs/restore | 49 | 23 | 13,372 | 12,263 |
| programs/dtdocbook/infolib/.libs/valBase | 49 | 23 | 13,372 | 12,263 |
| programs/dtdocbook/infolib/.libs/validator | 49 | 23 | 13,369 | 12,263 |
| programs/dtdocbook/infolib/.libs/dbdrv | 49 | 23 | 13,367 | 12,263 |
| programs/dtdocbook/instant/.libs/instant | 48 | 22 | 12,113 | 11,273 |
| programs/dtksh/.libs/dtksh | 47 | 14 | 11,313 | 10,586 |
| programs/ttsnoop/.libs/ttsnoop | 49 | 16 | 11,312 | 11,012 |
| programs/dtterm/.libs/dtterm | 49 | 18 | 11,271 | 11,021 |
| programs/dtcm/server/.libs/rpc.cmsd | 48 | 21 | 11,223 | 10,691 |
| programs/dtcm/dtcm/.libs/dtcm | 48 | 14 | 11,196 | 10,696 |
| programs/dtcm/dtcm/.libs/dtcm_editor | 48 | 15 | 11,185 | 10,567 |

61 binaries and libraries measured; see `baseline.json` for all.

