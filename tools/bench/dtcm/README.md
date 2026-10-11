# dtcm benchmarks

The dtcm harness of `TODO.md` Phase 0: a calendar of 5,000 one-time and
200 repeating appointments on a private `rpc.cmsd`, the lookups dtcm
makes on it (month, week and year views, Find "all"), and dtcm itself on
Xvfb.  Like `..`, this directory is not part of the automake build; it
links against the libraries of the built tree.

```sh
make -C tools/bench/dtcm
tools/bench/dtcm/run.sh -w /tmp/dtcm-run          # everything; JSON in /tmp/dtcm-run/dtcm.json
tools/bench/dtcm/run.sh -r 3 -G -S                # quick: lookups only
```

## Files

| File | What it does |
|---|---|
| `cmsd.sh` | `start WORKDIR` / `stop WORKDIR`: the tree's `rpc.cmsd -d` with its calendars in `WORKDIR/spool` and its log (one line per RPC) in `WORKDIR/cmsd.log`, without root (see below). |
| `dtcmbench.c` | `populate`: creates the calendar and adds the appointments through libDtCmP's `validate_appt()` (what `dtcm_insert` does, so they carry dtcm's attributes); every 50th one-time appointment has no time, every 10th sequence gets an exception.  `bench`: the lookup cases below, with the `../benchutil.c` harness (`-r`, `-j`, median), plus entries, RPCs and `rpc.cmsd` CPU per operation.  `change`: adds and deletes appointments on a day, to make a running dtcm refresh.  `dump`: every entry of every month of 2024-2028 with all attributes, to check that two builds of `rpc.cmsd`/libcsa return the same. |
| `run.sh` | All of it, into one JSON document: populate, `bench`, strace counts per case, dtcm startup and updates. |

## Cases

| Case | What |
|---|---|
| `month` | a month view, as `monthglance.c`/`allocate_paint_cache()`: one `csa_list_entries()` for the month, `query_appt_struct()` per entry; per op = one month (12 months of 2026 per run) |
| `week` | the same per week (52 weeks of 2026) |
| `year` | one lookup for all of 2026 |
| `find_all` | Find "all" for a word in 1/3 of the appointments, as `find.c` does it now: 52-week chunks, summary matched by the server (`CSA_MATCH_CONTAIN`) |
| `find_rare` | the same for a word in 5 appointments |
| `find_old` | Find "all" as it was before TODO 5.4: 4-week chunks, every entry fetched and matched in the client |
| `noop` | nothing; the logon baseline for the strace counts |

`run.sh` also reports, per case and minus the `noop` baseline, the
syscalls of the client and of `rpc.cmsd` and their accesses (open and
stat) to `/etc/localtime`, from `strace -f`; and for dtcm on Xvfb: the
time to the first window and to idle (`../apptime.py`), the RPCs and
`/etc/localtime` accesses at startup, and per change of the calendar
(20 changes on the day shown) dtcm's RPCs, CPU, mallocs and X requests.

## rpc.cmsd without root

`rpc.cmsd` refuses to run unless it is root, keeps its calendars in the
fixed `/var/spool/calendar` and registers program 100068 with rpcbind.
`cmsd.sh` runs it in a user namespace (`unshare -r -m`): root inside is
the user outside, uids 1-65536 map to the user's subordinate ids (from
`/etc/subuid` and `/etc/subgid`, for the `daemon` uid that `rpc.cmsd`
switches to), and a bind mount puts `WORKDIR/spool` over
`/var/spool/calendar` in the namespace only.  rpcbind accepts the
registration from the user over its local socket, and clients reach the
server as usual.  Needed:

- `rpcbind` running (`systemctl start rpcbind`), and no other
  `rpc.cmsd` registered;
- unprivileged user namespaces (the default on most distributions) and
  a subordinate id range for the user (`usermod --add-subuids
  100000-165535 --add-subgids 100000-165535 $USER`).

Otherwise start `rpc.cmsd -d` as root yourself (see the top of `cmsd.sh`),
set `DTCMBENCH_CMSD_LOG` to its stderr and `DTCMBENCH_CMSD_PID` to its
pid, and run `run.sh -x`.

## Notes

- On a busy machine the wall times vary; the CPU times (client
  `cpu_ns_per_op`, `server_cpu_ns_per_op`) and the counts (entries,
  RPCs, mallocs, syscalls) are the stable numbers.
- `rpc.cmsd -d` writes one line per call to its log; that is how RPCs
  are counted, for the bench and for dtcm alike.
