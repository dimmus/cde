# CDE speed and efficiency TODO

## Status as of this branch

Branch `claude/todo-round2`: 129 commits on top of `master` @ `c575f3f66`
(901 files changed, +28,067 / −137,036 lines), checked on 2026-10-12. About 125,000 of the deleted lines
are the unbuilt dtudcfonted, dtudcexch and lib/pam sources. Round 1
(`claude/todo-implementation`, 165 commits on `d907a737d`) is already in
`master`.

Every item below ends in a *(status: ...)* note. Ticked items are done;
a part that was measured and found to bring nothing ("no change needed")
counts as done. Unticked items are either **partial** (work that is still
worth doing; the note says what is missing and why) or **won't do** (the
remaining change was measured or tried and declined, because it would
change behaviour, break compatibility, or belongs to another tree; the
note gives the numbers). The Top 20 table and the bug table have a Status
column.

### What was done

Checkbox items per section:

| Section | Done | Partial | Won't do | Done after round 1 |
|---|---|---|---|---|
| 0.1 Harnesses | 10 | 2 | 0 | 1 |
| 1.1 Fixed sleeps | 13 | 0 | 0 | 12 |
| 1.2 Polling | 7 | 0 | 2 | 7 |
| 2.1 dtlogin/dtgreet | 5 | 0 | 0 | 4 |
| 2.2 Xsession | 3 | 1 | 0 | 1 |
| 2.3 dtsession | 10 | 0 | 1 | 8 |
| 3.1 DtSvc typing/database | 16 | 0 | 2 | 15 |
| 3.2 DtSvc startup/IPC | 7 | 0 | 1 | 7 |
| 3.3 ToolTalk | 19 | 0 | 3 | 14 |
| 3.4 DtWidget | 11 | 0 | 0 | 7 |
| 4.1–4.4 dtwm | 36 | 0 | 2 | 31 |
| 5.1 dtfile | 24 | 0 | 3 | 18 |
| 5.2 dtterm | 14 | 0 | 0 | 12 |
| 5.3 dtmail | 8 | 1 | 0 | 7 |
| 5.4 dtcm | 6 | 0 | 0 | 4 |
| 5.5 Help/dtinfo | 9 | 1 | 0 | 5 |
| 5.6 Other apps | 9 | 0 | 0 | 7 |
| 6.1–6.6 Build | 36 | 0 | 3 | 27 |
| 7 Warnings | 4 | 0 | 0 | 3 |
| **Total** | **247** | **5** | **17** | **190** |

Round 2 in short:
- **Harnesses (Phase 0):** syscall counts per operation, a CI benchmark
  gate (`.github/workflows/bench.yml`), `TT_STATS` RPC counters in libtt,
  new dtwm, dtterm, dtfile and dtmail cases, and new dtcm and help
  harnesses. Startup-trace stamps in dtlogin, dtgreet, the login scripts,
  dthello, dtdbcache, dtsession and dtwm.
- **Login:** in-process font-path append in dtlogin, cached profile
  syntax checks and a cached `dtsearchpath` result in Xsession, an exact
  in-place dtappgather, restored clients forked before dtwm is ready
  (`Dtsession*startClientsEarly`), a pre-created lock dialog.
- **ToolTalk:** `tt_open` in one RPC (OPEN_PROCID), batched
  ADD/DEL_PATTERNS, an AF_UNIX transport under `$XDG_RUNTIME_DIR`, dtexec
  without an X connection or libDtSvc, hashed pattern and file-scope
  tables, an O(depth) `_tt_realpath`. All new procedures fall back to the
  old ones against an older ttsession, in both directions.
- **DtSvc/DtWidget:** libpam loaded with `dlopen`, DtCodelibs in C (no
  libstdc++ in libDtSvc), a negative `.DtDirDataType` cache,
  `_DtDtsDataToDataTypeAt` for dtfile, Replace All in runs, asynchronous
  spell check, a cheaper Format, no round trip on any drag start.
- **dtwm:** no `XSync` per workspace switch (ordered message queue),
  shared backdrop windows, `f.refresh` one round trip per tree level,
  fewer icon-label queries.
- **Applications:** dtfile persistent workers, `update_some` with inotify,
  path-type cache, O(K) trash, fast desktop-object load; dtterm cell
  arrays and a ring of line pointers with int rows; dtmail non-blocking
  lock waits, header filter, tail-only saves on CoW filesystems; dtcm
  server without deep copies and incremental month repaint; help
  parsed-topic cache, table-cell index, incremental volume scan,
  asynchronous DtSearch API, mmap'ed infolib stores; lazy dtcreate icons;
  dtksh parse-tree cache; dtprintinfo through libcups.
- **Build:** non-recursive DtMmdb, tt/lib, dtinfo and dthelp parser;
  VPATH builds and `make distcheck` (`.github/workflows/distcheck.yml`);
  batched man pages (32.3 s -> 2.5 s); a faster infolib build; ksh93
  leaves no file outside `ksh93/arch`; a curated `-Wextra` with every site
  fixed and `--enable-werror` (CI job in `.github/workflows/build.yml`);
  md5 de-duplicated; dtudcfonted, dtudcexch and lib/pam deleted.

Bugs found and fixed in round 2 (not in the audit):
- **MD5 was wrong on LP64** (`UINT4` was `unsigned long`): dtmail's APOP
  digest and the `Content-MD5` headers of dtmail and dtcm were incorrect.
- **dtterm:** double or triple click on an empty line crashed in any UTF-8
  locale; `-sl 40000` crashed and `-sl 70000` silently kept 4,464 lines.
- **dtwm:** a client reading `_DT_WORKSPACE_INFO` on a TITLE notice got the
  old title (3 times in 60).
- **dtlogin:** every failed login appended the font directories again
  (`xset fp+` duplicates) on a server that does not reset.
- **dtksh:** `${CB_CALL_DATA.EVENT.TYPE}` was usually empty (4-byte value
  read as a long). `make check` was broken.
- **DtCodelibs:** `~user` used `getpwnam_r` storage of a returned frame;
  empty `` `cmd` `` output read `result[-1]`; `doit()` returned an
  uninitialised character. DtSvc `islink()` answered yes for any file once
  a `MODE l` test had run.
- **-Wextra fixes:** missing `break`s in dtstyle and dtmail (wrong
  messages); `dtsrclean -russell` fell into `-m`; DtSearch never rejected
  an invalid date on LP64; dtcm `mblen() < 0` on a `size_t`; DtTerm
  softkey length wrapped at 256 and overflowed; DtMmdb `disk_hash` MAX of
  an unsigned difference and `buffer::skip` throwing unconditionally; csa
  `abs()` of a long.
- **Others:** dtprintinfo showed `(null)` dates and owners like `alice@lp
  is ready` with CUPS's lpq; ksh93's `mamake -jN` could hang (lost
  SIGCHLD, 4 of 10 runs at -j2); dtinfo's store destructor could throw at
  exit; GifUtils.c violated strict aliasing; the Application Manager
  folder was briefly empty at every login.

### Verification on this branch (2026-10-12, GCC 16.2.1, 2 CPUs)

- **Clean rebuild.** `./autogen.sh --disable-docs --prefix=/usr && make clean && make -j3`
  exits 0 in **837 s**, ksh93 included. A no-op `make -j3` takes **3 s**.
  `git status` is clean after the build: ksh93 leaves nothing outside
  `ksh93/arch`.
- **Warnings: 0 in CDE code** at the enabled level (`-Wall` plus the
  curated `-Wextra` set). Remaining diagnostics, none from CDE sources:
  - 14 × `-Wdiscarded-qualifiers` (on by default in GCC) and 2 ×
    `'SHOPT_DYNAMIC' redefined` in the vendored ksh93 (libast, libdll,
    libcmd, `sh/*.c`), which is compiled without CDE's warning flags;
  - bison grammar warnings in `lib/DtMmdb/StyleSheet/style.yy` (1 useless
    nonterminal, 2 useless rules, 5 shift/reduce conflicts);
  - 1 gawk dynamic-regexp warning from `programs/dtprintinfo/msg.awk`.
- **Documentation build.** `./configure --prefix=/usr` (docs, help, man
  pages, infolib and doc tools on), `make clean && make -j3`: exits 0 in
  **913 s** with the same diagnostics and nothing else; it regenerated the
  19 help volumes, 661 man pages and the CDEDOC infolib with its DtSearch
  index. The `--disable-docs` configuration was restored afterwards
  (`make -j3` then had nothing to do: 4 s, 0 warnings).
- **Smoke tests** under Xvfb with the in-tree libraries, the in-tree
  ttsession (`ttsession -s -d`) and dtwm running, two runs:

  | Program | Result |
  |---|---|
  | dtwm | Up for the whole run, empty stderr |
  | `ewmhtest all`, `ewmhtest -f all` | Pass (8 of 8 runs after the fix below) |
  | `dtterm -e sh -c 'echo ok'` | Exits 0; the child ran 5 of 5 times in both runs |
  | `dtfile -folder <dir>` | Alive after 6 s |
  | `dtpad -standAlone <file>` | Alive after 5 s |
  | dtcalc | Alive after 5 s |
  | `dthelpview -helpVolume Help4Help` | Alive after 5 s |
  | dtprintinfo | Alive after 5 s (no CUPS scheduler: lpstat fallback) |
  | `dtmail -f <2-message mbox>` | Alive after 6 s; mbox not rewritten |
  | `dtksh -c 'echo ok'` | Prints `ok`, exits 0 |
  | ttsession | Still up at the end |

  No crash text, no X error and no core dump (`coredumpctl`). Expected
  noise only: dtmail's missing `dtsymbol` font warning, dtprintinfo's
  `lpstat: not found`, and dtfile's `PipeRead: broken pipe` from a worker
  when dtfile is killed.
  - `ewmhtest -f actions` failed about 1 run in 3 with "odd extents for a
    title and a border": a race in the test, which read the frame before
    dtwm had applied new MWM decorations. Fixed in the test (0efa96cf5);
    10 of 10 runs pass.

### Measured numbers, round 2 (details in each item's note)

Round 1's table is in git history (`git show c575f3f66:TODO.md`).

| Area | Before → after |
|---|---|
| Xsetup per greeter cycle | 17.5 ms → 11.6 ms |
| Xsession run (stub clients) | 55.3 ms → 44.6 ms, 2 processes fewer |
| Restored session, all windows up (median) | 513 ms → 440 ms; 7 apps ~575 → 500 ms |
| Lock request → keyboard grabbed | 42 ms → 10 ms (median) |
| Startup trace (Xvfb) | T_greeter 0.108 s; wm-ready 0.60 s, last of 5 restored clients 0.69 s |
| `tt_open` | 3 sync RPCs → 1; open-close 288 µs → 205 µs (AF_UNIX) |
| `ttdt_session_join` | 12 ADD_PATTERN RPCs → 1; 1.53–1.73 → 0.77–1.09 ms |
| ttsession with 10k patterns | client CPU 1,472 → 35 µs per pattern |
| dtexec spawn → `_DtActDtexecID` | 13.6–16.6 → 10.3–11.9 ms; DSOs 44 → 23 |
| libDtSvc client | no libpam, libaudit, libcap-ng; no libstdc++ from libDtSvc |
| DtEditor Replace All, 100k hits | 3.7 s → 32 ms |
| Spell check with a 0.5 s filter | returns in 517 ms → 4 ms |
| dtwm workspace switch | 2.43 → 0 round trips; shared backdrop: Xvfb CPU 780 → 20 ms per 400 switches |
| dtwm `f.refresh`, 3020 windows | 240–714 ms → 43–47 ms |
| dtfile navigation, 5 folders and back | 1 fork in total instead of one per read/update |
| dtfile one-file change in 3000 entries | update process 6.6 ms → 1.1 ms |
| dtfile 200 desktop objects | CPU 1.9 s → 0.17 s; RSS 29 → 18 MB |
| dtfile trash restore, 100k entries | main-loop CPU 87 ms → 5.2 ms |
| dtterm CJK cursor-addressed edits | 1160 ms → 777 ms CPU |
| dtterm `yes`, 16 MB, C.UTF-8 | 3881 ms → 2770 ms CPU; `saveLines` limit ~32k → 2^23 |
| dtmail with a held lock (42 s) | UI blocked 14.4 s → 0 s |
| dtcm month view | rpc.cmsd CPU 15.3 → 5.0 ms; per change 142 → 6 X requests |
| Help revisit (Filemgr) | 2.3 → 1.36 ms CPU, 1840 → 154 mallocs |
| Help topic with a 1,500-item table | 346 ms → 27 ms |
| dtcreate icon folder (285 icons) | 1.0–1.2 s → 0.03 s |
| dtksh 40,000-step callback stress | 16.3 s → 2.5 s |
| dtprintinfo, 40 s, two open queues | 25 child processes → 0 |
| Man pages (`doc/en_US.UTF-8/man`) | 32.3 s → 2.5 s |
| en_US infolib | 6.9–8.3 s → 6.2–6.6 s (NodeParser 3.7 → 1.42 s CPU) |
| Whole-tree no-op `make` | 3.5–3.8 s → 2.9–3.2 s |

Benchmark run of the final tree: `tools/bench/bench.py run --quick --syscalls`
(976 s, every group passes) against `tools/bench/baseline.json` (taken
mid-round at `99a05633b`):
- ToolTalk open-close: 4 → 2 RPCs and 125 → 109 syscalls per operation;
  `ttdt_open` + join: 32 → 8 RPCs and 464 → 220 syscalls (B3, B5).
- dtfile on a 1k-entry folder: idle after 0.21–0.23 s → 0.13–0.15 s.
- Every other case is within run-to-run noise. `bench.py compare -t 25`
  flags two cases, both artifacts of `--quick`'s single round:
  `editor-set-nul` (4.2–4.7 ms with `-r 5`, baseline 3.9 ms) and
  `hold-window-outline` 3 → 4 round trips (the extra one is libXm's first
  `XmGetPixmap` of the outline stipple; 3 with `-r 3`).
- Loader: 0 unused direct dependencies in 57 binaries; dtprintinfo has the
  most startup relocations (18,159, 56 DSOs, now with libcups).

### What remains (most valuable first)

1. **lib/DtSvc:** move the dtdbcache file to `$XDG_RUNTIME_DIR` or
   `~/.dt/cache`, keyed on the search path. The name comes from
   `shared_cache_name()`/`_DtDtsMMCacheName()` and `_DTDTSMMTEMPDIR`, and
   every libDtSvc client must agree on it; the login scripts need no change.
2. **DtSearch:** declare `DtSearchQueryStart/Continue/Stop` in
   `include/Dt/Search.h`, export them in `build-aux/symbols/libDtSearch.sym`,
   then use them from dtinfo.
3. **Startup trace:** ttsession entry/exit stamps (lib/tt), and a T_desktop
   run under a real dtlogin as root (round 2 measured non-root).
4. **dtfile:** `Desktop.c` sleeps 2 s after the last failed
   `DtWsmGetWorkspaceList` when there is no window manager, even with
   `retryLoadDesktop: 0`.
5. **libXm** (`/home/dimmus/motif`): icon-box inserts are O(N²) in
   `XmeNavigChangeManaged` and BulletinBoard layout (see 4.4).
6. **dtmail:** time the FICLONERANGE save on a CoW filesystem; a tail-only
   save on other filesystems needs a design that keeps the crash guarantee.
7. **Build harness:** a `remake --profile` run (remake is not installed).
8. **Small fixes still open from round 1:**
   - the `%n` in dthelpprint `PrintTopics.c` (built with `_FORTIFY_SOURCE=1`);
   - dtmail `RoamApp.C` `formatPropPair` (built at level 2);
   - the 15 `--enable-lto` ODR warnings;
   - the stale `_XmPutScaledImage` prototype in `include/Xm/XmPrivate.h`;
   - the `ScreenInfo` leak in dtfile `Encaps.c`;
   - dtwm `UI.c` `GetIconName` per animation frame.
9. **CI:** the build (`-Werror`, docs), bench and distcheck workflows have
   not run on GitHub yet.

### Risks to know before merging

- **New ToolTalk wire procedures** (OPEN_PROCID 49, ADD_PATTERNS 50,
  DEL_PATTERNS 51) and the **AF_UNIX transport**. Old clients never use
  them, and new clients fall back on PROCUNAVAIL or a missing socket, so a
  mixed old/new set works (tested all four ways). The socket lives in
  `$XDG_RUNTIME_DIR/tooltalk/`; `TT_UNIX_SOCKET=0` turns it off.
- **libtt's internal C++ ABI** changed in round 1 with the same soname; a
  stale pre-branch `/usr/bin/ttsession` aborts against the new libtt.
  Round 2 kept class layouts unchanged, and an attempted layout change in
  `_Tt_object_table` confirmed the crash and was reverted. Upgrade
  `ttsession`, `rpc.ttdbserver` and the tt tools together.
- **libDtSvc** loads libpam with `dlopen` (LIBPAM_SONAME, then
  `libpam.so.0/.1/.6`): a system without a libpam of those names gets
  `PAM_SYSTEM_ERR` from the `_DtSvcPam*` calls instead of a link failure.
  DtCodelibs was rewritten from C++ to C with the same exports.
- **Defaults and files that users will notice (new in round 2):**
  - `Dtsession*startClientsEarly: True` (restored clients start before
    dtwm is ready; set False to get the old order);
  - Xsession writes `~/.dt/cache/syntax-ok.*` and a `dtsearchpath` cache;
  - dtlogin sets the font path itself when Xsetup mentions
    `DTLOGIN_FONTPATH` (the shipped one does; site copies keep `xset`);
  - MD5 output changed: `Content-MD5` headers and APOP digests are now
    correct, so they differ from what the old binaries produced;
  - dtprintinfo builds against libcups when found (`--without-cups` turns
    it off) and shows the real job owner, date and title;
  - dtwm links `libX11-xcb` and `libxcb`; its CPU per workspace switch is
    about 10% higher (more, smaller event batches) while wall time and
    round trips are lower.
- **Defaults from round 1, still in force:** `Dtwm*freezeOnConfig: False`
  and `useWindowOutline: True`; dthello timeout 15 s;
  `Dtlogin*restartDelay` 2 s; ttsession RPC timeout 10 s
  (`TT_RPC_TIMEOUT`); dtterm stops blinking after 30 s idle, 100 ms visual
  bell, bells within 200 ms dropped; `NDEBUG` and hardening in release
  builds; dtdbcache format v2; export maps hide symbols not used in the
  tree or declared in installed headers.
- **Large rewrites validated by differential harnesses, not by long use:**
  dtterm's cell-array buffer and line ring, DtEditor Replace All, the
  dtksh parse-tree cache, dtfile's sticky workers and `update_some`, the
  help parsed-topic cache (about 22 KB per kept topic, 16 per display
  area), dtcm's lookups without deep copies.
- **Removed sources:** programs/dtudcfonted, programs/dtudcexch and lib/pam
  (not built since round 1).
- **Not exercised at runtime:** unlock followed by a second lock; the
  FICLONERANGE success path (no CoW filesystem here); dtlogin as root with
  the new font-path code and trace stamps; TT_QUEUE file observers; an
  `XTHREADS` build; the round-1 list (getty loop, real XIM servers,
  DPMS-off in dtscreen, Xprint printing in dtcm, DtEncap PTY mode, BSD
  code paths, long-lived rpc.ttdbserver databases).

---

Audit date: 2026-10-09. Baseline: `master` @ `a813c89ec` ("Fix iconv deps and
XmeFlushIconFileCache build error"). Every `file:line` below refers to that
commit; later commits (for example the `-Wall` cleanup) shift line numbers.

How the audit was done:
- Nine read-only review passes, one per area:
  1. login and session startup;
  2. DtSvc, DtWidget and the action/datatype database;
  3. ToolTalk;
  4. dtwm, cross-checked against the mwm fixes already made in the sister
     Motif tree (`/home/dimmus/motif`, see its `TODO.md` Phase 3);
  5. dtfile;
  6. dtterm and DtEditor/dtpad;
  7. help, dtinfo and DtSearch;
  8. the other applications;
  9. build system and toolchain.
- Inspection of the main checkout build (GCC 16.2.1, `--disable-docs`, 2 CPUs):
  `nm -D`, `ldd -u -r`, `LD_DEBUG=statistics`, artifact mtimes, and a few
  timings under Xvfb.
- Repo-wide counts in `lib/` and `programs/`:

  | Pattern | Count |
  |---|---|
  | `XSync` | 139 |
  | `sleep()` | 143 |
  | `usleep` | 8 |
  | `XtAppAddTimeOut` | 135 |
  | `system()` | 100 |
  | `popen` | 39 |
  | `fork` | 94 |
  | `XmListAddItem*` in loops | 60 |
  | `sprintf` | 3,574 |
  | `strcat` | 1,546 |

Items marked **[verified]** were checked by reading the code at the cited
lines. Cost figures are estimates from the code unless they say "measured".

Priority legend:
- **P0**: broken; fix before anything else.
- **P1**: large, user-visible win (every login, every app start, every file
  shown, every message), or many build minutes.
- **P2**: meaningful.
- **P3**: polish.

---

## Top 20: biggest wins for the least work

| # | Item | Where | Win | Status |
|---|------|-------|-----|--------|
| 1 | Delete `sleep(5)` before the greeter's main loop | `dtlogin/vgmain.c:614` | −5 s on every greeter | done |
| 2 | Compile out the `GettyRunning()` stub loop | `dtlogin/dm.c:940-951` | −5 s at boot | done |
| 3 | Stop the 100 ms `poll()` sleep and per-call mount-table reparse in ToolTalk path mapping | `lib/tt/lib/util/tt_file_system.C:390-437` | −100 ms or more per file-scoped message, folder open and ttdbserver RPC | done |
| 4 | Use the existing name index in `_DtDtsMMGetRecordByName` | `lib/DtSvc/DtUtil1/DtsMM.c:462-489` | O(1) instead of about 350 compares per attribute lookup; dtfile does about 10 per file | done |
| 5 | Skip `XGetGeometry` in DtIcon; compare image names with `strcmp` | `lib/DtWidget/Icon.c:1761,2097,2241,2315,2353` | −1 round trip per icon per refresh in dtfile | done |
| 6 | Cache the action/datatype DB across logins and validate it instead of rebuilding | `dtlogin/config/Xsession.src:295-300`, `DtsMM.c:326-334` | −130–190 ms per login (measured) | partial: validated and kept, not moved to $XDG_RUNTIME_DIR (name is fixed in lib/DtSvc) |
| 7 | Use `close_range()` instead of closing or `fcntl`ing every fd up to `RLIMIT_NOFILE` | `DtEncap/spc-exec.c:165`, `dtsession/SmRestore.c:4755`, `CmdMain.c:1005`, `tt/slib/mp_ptype.C:391` | Up to 1M syscalls (0.1–1 s) per spawn under high fd limits | done |
| 8 | Port mwm's move/resize fixes: no `XQueryPointer` polling, no `FlashOutline` `XSync` spin | `dtwm/WmWinConf.c:3613-3679,1516-1547` | No more 100% CPU while dragging | done |
| 9 | dtterm: non-blocking read loop, damage tracking and one frame per ~16 ms | `DtTerm/TermPrim/TermPrim.c:2870`, `TermPrimScroll.c:509` | Roughly 10–50× `cat` throughput | done |
| 10 | dtterm: ring-buffer scrollback instead of shifting pointers and copying every line | `TermPrimRender.c:731-936`, `TermPrimBuffer.c:1768` | O(1) per scrolled line | done (ring of line pointers, int row types) |
| 11 | dtfile: inotify/kqueue instead of a 3 s stat poll plus 180 s forks | `dtfile/Directory.c:4614-4757`, `Main.c:811-818` | Idle wakeups to zero; instant updates | done (inotify; no kqueue) |
| 12 | dtfile: no rebuild of every icon gadget on every refresh | `dtfile/File.c:5986,6066,7257` | O(changed) instead of O(N) refresh | done |
| 13 | `dtfile_copy`: `copy_file_range` and no `XSync` per 2 KB block | `dtfile/dtcopy/fsrtns.c:91-153`, `main_dtcopy.c:505-532` | −512k round trips per GB copied | done |
| 14 | dtmail: O(1) message handles instead of `indexof` scans | `dtmail/libDtMail/RFC/RFCMailBox.C:988,1050` | O(N) instead of O(N²) mailbox open | done |
| 15 | dtmail: don't dirty every message without `Content-Length` | `RFCMessage.C:999` | No full mbox rewrite after open | done (tail-only write on CoW filesystems via FICLONERANGE) |
| 16 | dtprintinfo: libcups or one async `lpstat` instead of 6 forks per printer every 30 s on the UI thread | `dtprintinfo/objects/PrintObj/Queue.C:58-61`, `util/Invoke.C:121-183` | UI no longer freezes | done (libcups when available, async lpstat otherwise) |
| 17 | `LT_INIT([disable-static])` | `configure.ac:33` | −751 compiles (about 21% of the build) | done |
| 18 | Per-program `LDADD`, real library `_LIBADD`, `-Wl,--as-needed` | `configure.ac:176-177`, `lib/*/Makefile.am` | About −10 DSOs and −30% relocations per process start | done |
| 19 | Drop `.NOTPARALLEL` with grouped targets or stamps | `dtappbuilder/src/ab/Makefile.am:3`, `dthelp/parser/**`, `ttsnoop` | About 280 TUs compiled in parallel | done |
| 20 | Build the bundled ksh93 with `-j` and `$(CC)` | `programs/dtksh/Makefile.am:61-62` | −1 to 2 min per clean build (132 s measured, serial) | done |

---

## Phase 0 — Measure first

### 0.1 Harnesses
- [x] Add `tools/bench/` (or `tests/bench/`), modelled on Motif's `xmbench`, `mwmbench` and `xmbench-proxy` (`/home/dimmus/motif/src/tests/bench`). *(status: done; syscalls per op from strace with begin/end markers (syscount.py, `bench.py run --syscalls`); compare fails on >10% more syscalls or RPCs, more DSOs or unused deps, >5% more relocations; CI job .github/workflows/bench.yml (base vs head on one runner, not yet run on GitHub))*
  - Run each case headless under `xvfb-run -s "-screen 0 1920x1080x24 +extension RENDER"`.
  - Pin with `taskset` and take the median of 5 runs.
  - Write JSON and fail CI on a regression above 5% or on any increase in round trips.
  - Metrics per operation:
    - ns;
    - mallocs (LD_PRELOAD counter);
    - syscalls (`strace -c` / `perf trace -s`);
    - X requests (`XNextRequest` delta);
    - **round trips**, through an LD_PRELOAD on `_XReply`, or through the delay proxy so that they show up in wall time.
- [ ] **Startup trace.** Add an opt-in `CDE_STARTUP_TRACE=<file>` that appends `CLOCK_MONOTONIC` stamps. *(status: partial: stamps in dtlogin, dtgreet, Xsetup/Xstartup/Xsession, dthello, dtdbcache, dtsession (99a05633b) and dtwm start/ready/frontpanel-mapped (b142f74d1); dtlogin writes the clock line itself. Measured under Xvfb: T_greeter 0.108 s median; restored session of 5 clients wm-ready 0.60 s, last client mapped 0.69 s (Xsession run as the user, auth-ok stamped at its start). Missing: ttsession entry/exit stamps (lib/tt))*
  - In C use `clock_gettime`; in ksh use `printf '%(%s.%N)T'`, which needs no fork.
  - Stamp points:
    - dtlogin: master start, server exec, SIGUSR1, greeter fork;
    - dtgreet: entering the main loop, first Expose painted;
    - authentication OK; Xstartup done;
    - Xsession: start and each section;
    - dtdbcache, ttsession and dtsession: entry and exit;
    - dtsession: after `RestoreResources`, after `XtOpenDisplay`, after color init, dtwm fork, `_DT_WM_READY`, each client fork, READY;
    - dtwm: ready;
    - first and last restored client mapped.
  - Report three headline numbers:
    - **T_greeter**: X ready → greeter painted.
    - **T_desktop**: Enter → front panel mapped.
    - **T_restore**: Enter → last restored client mapped.
- [x] **`dtsvcbench`** (links libDtSvc and libDtWidget): *(status: done; tools/bench/dtsvcbench.c covers every case listed; the sshfs run was not done)*
  - `_DtDtsMMInit(0)` with a valid cache and with a stale one;
  - `DtDtsDataToDataType` over a synthetic 10k-entry tree (suffixed files, files without suffix, directories, binaries, symlinks), also on sshfs;
  - `DtDtsDataTypeToAttributeValue` ×100k;
  - `DtActionLabel` / `DtActionIcon` / `DtActionExists` ×100k;
  - `XeSPCSpawn("/bin/true")` ×100 under `ulimit -n 1024` and under `ulimit -n 1048576`;
  - `DtComboBoxAddItem` / `DtSpinBoxAddItem` ×1k;
  - DtEditor cases (1 MB with 100k NUL runs; Replace All with 10k hits; 10k cursor moves with the status line on).
- [x] **`ttbench`** (links libtt; started with `ttsession -S`, plus an optional `rpc.ttdbserver`): *(status: done; TT_STATS=1 per-procedure counters (sync/one-way, time, failures, per-interface connections) in libtt and around every rpc.ttdbserver clnt_call (lib/tt/lib/util/tt_stats.C); ttbench-rpcs.py gives rpcs_per_op; file-notice-db runs end to end against a user-started `rpc.ttdbserver -S`)*
  - `tt_open`/`tt_close` ×200;
  - `ttdt_open` + `ttdt_session_join`;
  - request/reply ping ×10k;
  - a burst of 1k notices;
  - 1k and 10k patterns across 50 procids;
  - `tt_message_file_set` / `tt_file_netfile` ×100 (watch for `poll([],0,100)`);
  - file-scoped notices with and without rpc.ttdbserver.
  - Add `TT_STATS=1` counters to `_Tt_rpc_client::call` (per procedure, sync vs one-way) and to `_Tt_db_client`.
- [x] **dtwm**: reuse `mwmbench -m dtwm` together with `libmwmbench_preload.so`. Add these cases: *(status: done; added ws-switch, fp-switch(-tt), backdrop (Xvfb RSS instead of xrestop, not installed), refresh/refresh-cover, drag-move-xor and 4 hold-still cases, all with wakeup counts; TTSESSION_CMD stops the installed ttsession from being autostarted)*

  | Case | What it does |
  |---|---|
  | map | 500 clients, plain and with `WM_CLASS` Dtterm (icon image path) |
  | workspace switch | 1000 switches with 100 clients over 4 workspaces |
  | front panel switch | XTest clicks, with and without ttsession |
  | titles | 10k `WM_NAME` / `_NET_WM_NAME` changes |
  | `WM_HINTS` | 10k urgency toggles |
  | drags | 4 variants: freezeOnConfig, outline, opaque, useWindowOutline; hold still for 3 s |
  | backdrop | 100 fit/fill backdrop changes (watch pixmap memory with `xrestop`) |
  | `f.refresh` | with 200 clients |

- [x] **dtterm**: *(status: done; vtgen.py/dtterm-vtebench.sh (the 6 vtebench cases, one screen per write), ttywrite.sh (requests and round trips of the output alone), requests per 24-line screen, dtterm-less.py page-down behind cdebench-proxy -d 2)*
  - Time `cat` of 100 MB of base64 text and `yes | head -c 200M`, in `LANG=C` and in UTF-8, with `-sl 4s` and with `-sl 10000`.
  - vtebench cases: `dense_cells`, `scrolling`, `scrolling_in_region_*`, `unicode`, `cursor_motion`.
  - Requests per screenful, and round trips per output burst (the target is 0).
  - Round trips per `less` page-down with 2 ms of added delay.
- [x] **dtfile**: *(status: done; dtfile-time.sh -S (strace, readers apart), -x (requests per opcode via cdebench-proxy; the installed xtrace is glibc's), -w (idle wakeups from /proc); dtfile-copy.sh; dtfile-perf.sh builds a -DDT_PERFORMANCE dtfile. Found: dtfile sleeps 2 s without a WM (Desktop.c), so runs now use the tree's dtwm)*
  - Directories of 1k/10k/100k entries: time to first paint and to all done, for each view mode, cold and warm.
  - `strace -f -c` for the reader.
  - `xtrace -c` counts of `GetGeometry`, `GetInputFocus` and `ClearArea`.
  - Idle wakeups over 10 min (`perf stat -e sched:sched_process_fork,sched:sched_wakeup`).
  - `dtfile_copy` of 1 GB against `cp`.
  - The existing `-DDT_PERFORMANCE` timers.
- [x] **dtmail**: synthetic mboxes of 1k/10k/50k messages, without `Content-Length` and with 30% multipart. Measure time to first paint, Sort, delete all, and 10 idle minutes under `strace -c`. *(status: done; dtmail-time.sh runs By Sender/Size/Date sorts, Select All + Delete and a 60 s idle window, -S adds strace; 10k messages: each sort 0.16-0.20 s, idle 8.5 wakeups/min, 0 forks)*
- [x] **dtcm**: 5k one-time entries plus 200 repeating entries. Time month and year lookups and Find "all", and count RPCs and `/etc/localtime` opens. *(status: done; tools/bench/dtcm/: rpc.cmsd in a user namespace (no root), dtcmbench populate/bench/dump/change, run.sh with strace and a dtcm-on-Xvfb pass; baseline month view 912 entries, 71,966 /etc/localtime stats per 12 month views)*
- [x] **help**: *(status: done; tools/bench/help/: helpbench.c (13 in-process DtHelpDialog cases with mallocs/requests/round trips and per-call Xlib counts), run.py groups print/view/mmdb/dtsearch, -S screenshots for cmp)*
  - `dthelpprint -allTopics` (no canvas; still needs Xvfb) under `perf`/callgrind.
  - `dthelpview` back/forward over topics that contain TIFFs.
  - Page-down through `dthelpview -manPage ksh`.
  - Index "complete" over all volumes.
- [ ] **Build**: *(status: partial: tools/bench/build-time.sh times configure, clean, no-op and `config.status && make` in a shared clone (used for the 6.3 P1 numbers) plus the cc-trace Chrome trace and a two-clone ccache mode; `remake --profile` (-R) not run: remake is not installed)*
  - `time make -j$(nproc)` for a clean build, a no-op rebuild, and `./config.status && make` (120 objects are rebuilt today).
  - A `CC` wrapper that logs per-TU start and end times, converted to Chrome trace JSON.
  - `remake --profile`.
  - `ccache -s` across two worktrees.
- [x] **Loader cost**: track `LD_DEBUG=statistics`, `ldd -u -r` and `nm -D --defined-only | wc -l` per binary and per library in CI. Today `dtcalc` loads 48 DSOs and does 11,047 relocations. *(status: done; loader group gated by `bench.py compare` (any more DSOs or unused deps, >5% more relocations) and run in CI; 57 binaries, unused deps 0 everywhere)*

---

## Phase 1 — Stop sleeping and polling (P1, small changes)

### 1.1 Fixed sleeps on user-visible paths
- [x] **[verified]** `programs/dtlogin/vgmain.c:614`: `sleep(5)` after the greeter is realized and raised, before `XtAddInput(RequestCB)` and `XtMainLoop()`. The greeter is mapped but unpainted for 5 s at every boot and every logout. It dates from the original import and has no stated reason. *(status: done; sleep removed; greeter visible 5.15 s -> 0.17 s after CHALLENGE (Xvfb))*
  - Fix: delete it.
- [x] **[verified]** `programs/dtlogin/dm.c:940-951`: `while (bootup++ < 5) { if (GettyRunning(d)) …; else sleep(1); }`. `GettyRunning()` is a stub that returns FALSE (`dm.c:1554-1557`), so the loop always sleeps 5 s before `StartServer()`. *(status: done; loop compiled only under GETTY_RUNNING_IMPLEMENTED; -5 s at first server start (not run, needs root))*
  - Fix: compile the loop out where the stub is used.
- [x] **[verified]** `dm.c:231`, `dm.c:1580-1605`: `CheckRestartTime` sleeps up to 30 s after a dtlogin restart or crash. *(status: done; new Dtlogin*restartDelay resource (default 2 s, 0 disables), skipped under systemd (-nodaemon + INVOCATION_ID))*
  - Fix: lower it to 1–2 s or make it a resource; skip it under systemd or `-nodaemon`.
- [x] **[verified]** `lib/tt/lib/util/tt_file_system.C:390-437`: on Linux, `/etc/mtab` is a symlink to `/proc/self/mounts`, so `stat` reports `st_size` 0. That sends the code into a 1993 automounter workaround that calls `poll(NULL,0,100)`, i.e. **sleeps 100 ms** (`:421-428`). *(status: done; one persistent /proc/self/mounts fd, reloaded only on POLLPRI; no sleep; tt_file_netfile 112 ms -> 0.02 ms; also fixes rpc.ttdbserverd)*
  - The mtime comparison can never short-circuit, because `:437` records the symlink's mtime.
  - The destructor resets `lastMountTime` (`:156`), and every caller creates a temporary `_Tt_file_system`.
  - The re-read is gated on `event_counter`, which every API call (`tt_audit.C:96`) and every ttdbserver RPC (`db_server_svc.C:767`) increments.
  - Hit by:
    - `tt_message_file_set` (`api_message.C:2675`);
    - `tt_pattern_file_add` (`mp_pattern.C:225`);
    - `tt_file_netfile`;
    - every file-scoped send (`mp_mp.C:139`);
    - every rpc.ttdbserver file RPC (`db_server_functions.C:2244`, about 30 handlers).
  - Opening a dtfile folder therefore spends at least 200 ms asleep (`dtfile/Main.c:1316-1338,3193`).
  - Fix:
    - Keep one persistent mount cache in `_tt_global`.
    - Read `/proc/self/mountinfo` directly and use `poll(POLLPRI)` on it to detect changes.
    - Never sleep on the call path.
- [x] **[verified]** `lib/tt/lib/mp/mp_c_session.C:97-104`, `:210`, `:220`: the ttsession ping or autostart loop retries with `sleep(1)`/`sleep(2)`. `mp_desktop.C:162-174` retries `XOpenDisplay` 20 × `sleep(1)`. *(status: done; _tt_backoff() from 10 ms capped at 250 ms within the old budgets; losing autostart race 2518 -> 81 ms)*
  - Fix: exponential backoff starting at about 10 ms, with an overall deadline.
- [x] **[verified]** `lib/tt/lib/mp/mp_rpc_interface.h:86` and `tt_db_client.C:73`: every synchronous RPC uses a timeout of 1,000,000 s, so a wedged ttsession hangs every client. *(status: done; 10 s for ttsession RPCs, 30 s for rpc.ttdbserverd, TT_RPC_TIMEOUT override; a SIGSTOPped ttsession now gives TT_ERR_NOMP after 10 s instead of hanging)*
  - Fix: bound control RPCs at 5–10 s.
- [x] **[verified]** `dtsession/SmAuth.c:83-85,199-215`: a stale `.ICEauthority` lock costs 10 × `sleep(2)` = 20 s, and the fallback path repeats it, for up to 40 s. *(status: done; lockAuthFile(): libICE-compatible -c/-l protocol, 100 ms polling, 10 s stale time; orphaned lock 20 s+failure -> 11.0 s, unwritable fallback 20 s -> 0 ms)*
  - Fix: about 10 s dead time with 100 ms retries, or `flock()`.
- [x] **[verified]** `lib/tt/lib/mp/mp_auth.h:62-64`, `mp_auth_functions.C:160-191`: the `~/.TTauthority` lock costs 10 × `sleep(2)`, and this happens **while the X server is grabbed** (see 3.3). *(status: done; 100 ms polling, 10 s dead time; orphaned lock 20 s+failure -> 11.0 s; the X grab is no longer held (D1))*
- [x] **[verified]** `dtsession/SmLock.c:591,599`: grab retries use `sleep(1)` (up to 6 s of frozen session manager). *(status: done; retries from a 100 ms XtAppAddTimeOut, 5 s total; dtsession no longer freezes)*
  - Fix: retry from an `XtAppAddTimeOut` (50–100 ms).
- [x] **[verified]** `dtmail/libDtMail/RFC/RFCMailBox.C:4001,4014` and `FileShare.C:352`: on lock contention dtmail does `sleep(5)` per retry, on the UI thread, for up to 5 min. *(status: done; background check and autosave never wait for a busy .lock (retried from an Xt timer, 50 ms doubling to 6.4 s; old stale-lock rule after 5 min); blocking waits end on inotify of the lock's directory with timed backoff for NFS; UI blocked 14.4 s -> 0 with a 42 s holder, lock taken 0-7 ms after release. FileShare (ToolTalk locking) handover still synchronous, see B8)*
  - Fix: backoff from about 10 ms, driven by an Xt timer.
- [x] **[verified]** `dtudcfonted/dtcpftogpf/cpftogpf.c:376`: an unconditional `sleep(1)` after `wait()`. *(status: done; sleep removed, waitpid on the child; dtudcfonted is not built, syntax-checked only)*
  - Fix: delete it.
- [x] **[verified]** `dtappbuilder/src/ab/ui_util.c:1086-1209`: `ui_sync_display_of_widget` waits for 5 quiet 100 ms ticks (at least 0.5 s, at most 5 s) on project load, import and module delete. *(status: done; XSync + dispatch + XmUpdateDisplay until a 30 ms quiet poll; 606 -> 36 ms on dtbuilder.bip)*
  - Fix: `XSync` + `XmUpdateDisplay`.
- [x] **[verified]** `lib/tt/lib/slib/mp_typedb.C:939-952`: a stale `.tt_lock` costs 5 × `sleep(2)`. *(status: done; precedence fixed; 'pid host' lock with backoff and dead-pid removal; dead-pid lock 10.2 s+failure -> 50 ms)*
  - Bug: `fd = open(...) == -1` assigns 0 or 1, so `close(fd)` closes stdin or stdout.
  - Fix: parenthesise correctly and use `O_EXCL` plus a pid check.

### 1.2 Polling where events exist
- [x] **[verified]** dtfile re-stats viewed directories every 3 s (`rereadTime`, `Main.c:811-812`; `TimerEvent` `Directory.c:4614-4723`). *(status: done; inotify watches per local directory through XtAppAddInput feeding update_all; st_mtim/st_ctim polling fallback only for unwatched dirs; link/desktop checks are sticky workers (0 forks); changes visible in ~105 ms; idle 20 -> 0 context switches/30 s. kqueue (BSD) not done)*
  - Each tick costs about 2 process wakeups and 15 syscalls.
  - Only one directory is checked per tick (`maxRereadProcsPerTick=1`), so with D views detection latency is 3·D s.
  - Detection uses `st_mtime` only, so a second change within the same second is missed.
  - A link and desktop check forks a child every 180 s, even with no window mapped (`Main.c:816-818`, `Directory.c:4734-4757`).
  - Fix:
    - inotify (Linux) or kqueue (BSD) watches through `XtAppAddInput`, feeding an update of only the changed entries.
    - Keep polling as a fallback (NFS) using `st_mtim` and `st_ctim`.
- [ ] **[verified]** dtmail polls every 15 s (`RoamApp.C:1460-1466,1629-1641`). Every 60 s while idle it shows the busy cursor on **every window** *before* checking anything (`RFCMailBox.C:2655`; `RoamApp.C:1712-1749`), then does `open(O_SYNC)` + a 1-byte read (`IO.C:369-401`). *(status: won't do (rest): size/mtime first and busy only on change were done in round 1; inotify measured: at most 24 of 321 syscalls per idle 10 min, and the 15 s tick must stay (autosave, inactivityinterval, retrieveinterval); inotify is used for lock waits instead (1.1))*
  - Fix: compare size and mtime first; show busy only on change; use inotify on local filesystems; cache mailrc values.
- [x] **[verified]** dtprintinfo: every 30 s each visible printer runs 2 × `ksh -c "lpstat … | awk"` (about 6 processes), plus `lpq` for expanded queues. This happens **synchronously on the UI thread** (`DtPrinterIcon.C:457-492`, `Queue.C:58-61`, `util/Invoke.C:121-183`). `wait()` reaps any child. *(status: done; one shared timer, one async `lpstat -a -p` per tick through XtAppAddInput, async lpq, waitpid on the exact pid, ticks skipped while hidden; 65 -> 18 forks/40 s, UI stalls 7.7 s -> 0.45 s. libcups since round 2 (5dda73247, configure --with-cups 33d4f8078): one CUPS-Get-Printers per tick and one Get-Jobs per open queue from threads, 25 -> 0 child processes per 40 s; lpstat/lpq kept when no scheduler answers)*
  - Fix: libcups (`cupsGetDests`/`cupsGetJobs`), or one `lpstat -a -p -o` for all queues; asynchronous through `XtAppAddInput`; `waitpid` on the exact pid; one shared timer.
- [x] **[verified]** Front panel mail and monitor controls `stat()` every 30 s per control (`lib/DtWidget/Control.c:448-529`). *(status: done; one shared inotify fd on local filesystems, 1 s quiet debounce; poll kept for NFS etc.; 0 stat()s idle, changes in 1-2 s)*
  - Fix: inotify where available.
- [x] **[verified]** dtimsstart polls the input-method selection owner once a second for up to 180 s (`start.c:141-157`, `win.c:1697-1746`, `xims.h:82-84`), which adds about 0.5 s to CJK logins. *(status: done; exponential polling 20 ms -> 250 ms; D=0: 1.17 -> 0.20 s. XFixes not used (needs a configure dependency))*
  - Fix: `XFixesSelectSelectionInput`, or 20 ms exponential polling.
- [ ] **[verified]** dtterm's cursor blink at 250 ms (`TermPrim.c:194-197`) means 4 wakeups/s forever. *(status: won't do (rest): timer re-arm and 30 s idle stop done in round 1 (0 syscalls after 35 s); blinking costs ~24 syscalls/s only during activity. 250 ms is the documented default of the public DtTerm blinkRate resource (Term.sgm, dtterm(1), Terminal help); Dtterm*blinkRate: 500 or 0 already opts out)*
  - Fix: 500 ms; stop blinking after N s idle; re-arm the timer only when it is not already armed (`TermPrimCursor.c:341-346,507-511`).
- [x] **[verified]** dticon's selection "marching ants" timer re-arms every 300 ms indefinitely, even while iconified (`graphics.c:178,266,325`). *(status: done; timer paused while unmapped or unfocused; 10 -> 0 draws/3 s when unfocused)*
  - Fix: stop it on unmap or focus-out.
- [x] **[verified]** dtscreen: the main loop is `callback(); XSync(); usleep(delay)`. "hop" has delay 0, so it is a 100% CPU spin. "blank" round-trips every 5 s for a no-op (`dtscreen.c:163-184`, `resource.c:120,128`). *(status: done; CLOCK_MONOTONIC deadline loop with poll(), capped at 60 fps; blank blocks in XNextEvent; DPMS-off stops drawing (untested: Xvfb has no DPMS); hop: 16411 frames/121 ticks -> 300/7 per 5 s)*
  - Fix: deadline-based `poll()` on the connection, capped at 60 fps; for blank, block in `XNextEvent`; stop drawing while DPMS is off.
- [x] **[verified]** The dtwm clock re-arms every 60 s from startup, not aligned to the minute (`Clock.c:318-406`). It can show the wrong minute for up to 59 s. *(status: done; aligned to the next multiple of clockInterval + 20 ms; hands move at :00.07 instead of up to 38 s late)*
  - Fix: schedule at `60 - tm_sec`.

---

## Phase 2 — Login and session startup (P1/P2)

Today's critical path is serial: dtlogin → `system(xrdb)` (cpp) → Xsetup (ksh, xset) → dtgreet → Xstartup → Xsession (15–25 forks, dtsearchpath) → **dtdbcache -init** → **ttsession** (waits for types) → dtsession_res (ksh, xrdb, cpp) → color server → dtwm → **wait for `_DT_WM_READY`** → restored clients.

### 2.1 dtlogin and dtgreet
- [x] **[verified]** `dtlogin/session.c:648-711`: greeter resources are merged in-process, written to a temp file, then loaded through `system("xrdb … -load")`. That is sh, xrdb and cpp (driver plus `cc1`) on every greeter cycle. *(status: done; RESOURCE_MANAGER set in-process with XChangeProperty; xrdb -nocpp only as fallback; 20.6 -> 0.18 ms per greeter cycle; also fixes cpp mangling of values)*
  - Fix: `XChangeProperty(RESOURCE_MANAGER)` directly from the in-memory Xrm database; at minimum pass `-nocpp`.
- [x] **[verified]** Xsetup forks `xset fp+` per greeter (`config/_common.ksh.src:283-303`), then `ApplyFontPathMods` does `XSetFontPath` + `XSync` (`fontpath.c:168-169`), and dtsession sets the font path again (`SmRestore.c:1301`). *(status: done; dtlogin appends the alias and server font directories in-process (one XGetFontPath, skips present entries, one XSetFontPath, the script's fallbacks), exports DTLOGIN_FONTPATH=done and Xsetup skips xset; only when the Xsetup file mentions DTLOGIN_FONTPATH, so edited copies keep their xset calls. Xsetup 17.5 -> 11.6 ms per greeter; repeated failed logins no longer grow the path (e1ec943ff))*
  - Fix: one in-process append that skips entries already present; skip no-op sets.
- [x] **[verified]** `config/Xreset.src:43-46`: `dtprintinfo -populate` runs synchronously at logout, before the next greeter. It forks lpstat/awk per queue plus `system(mkdir/cp)` per printer (`DtPrinterIcon.C:86-92,277,296`). *(status: done; runs in the background; -populate itself 38 -> 22 ms (one lpstat -v, in-process mkdir/cp); 'skip when unchanged' not done)*
  - Fix: run it in the background or at boot; skip it when the printer list is unchanged.
- [x] **[verified]** `server.c:197,214,399`, `resource.c:293-296`: the server-readiness wait uses 5 s steps when SIGUSR1 never arrives (Xephyr, Xwayland, wrappers). *(status: done; socket/pid polling from 50 ms to 250 ms plus XOpenDisplay backoff; forking server wrapper 5.00 -> 0.13 s; also fixes wait() reaping other displays' children and a 1 s sleep after every child death)*
  - Fix: retry `XOpenDisplay` with backoff from 50 ms, or use `-displayfd`.
- [x] `util.c:441-610` (`MakeLangList`): O(n²) `strcat` and an insertion sort under a 30 s alarm. It is cheap today; bound it. *(status: done; running length, d_type, qsort+dedupe; byte-identical output; fixes a lang[500] stack overflow)*

### 2.2 Xsession and friends
- [ ] **[verified] P1** The action/datatype cache is rebuilt from scratch on every login, synchronously, and deleted three times. *(status: partial: round 1 parts stand (-init without X, validate and keep 36.8 -> 14.9 ms, parallel with ttsession, no rm's). Moving the cache to $XDG_RUNTIME_DIR/~/.dt/cache keyed on the search path is not done: the name comes from lib/DtSvc (DtsMM.c shared_cache_name, _DTDTSMMTEMPDIR in include/Dt/DtsMM.h) and every libDtSvc client must agree on it; no script change would be needed)*
  - Built at `Xsession.src:295,547-565`; deleted at `Xsession.src:300,570`, `Xstartup.src:45-49` and `Xreset.src:68-76`.
  - `dtdbcache -init` calls `_DtDtsMMInit(1)` → `_DtDtsMMCreateDb`. That parses about 93 `.dt` files and writes about 220 KB, after a full `XtAppInitialize` it does not need (`dtdbcache/Main.c:95-106,161`). Measured at **130–190 ms**.
  - Fix:
    - Keep the cache under `$XDG_RUNTIME_DIR` or `~/.dt/cache`, keyed on the database search path.
    - In `-init`, rebuild only if validation fails (see 3.1 B4).
    - Drop the X initialization.
    - Run it in parallel with ttsession.
    - Remove the three `rm`s.
- [x] **[verified] P2** `lib/tt/bin/ttsession/mp_server.C:391-448`: the ttsession parent waits until the child has finished `init_types()`. Clients may already connect before that (see the comment at `:395-401`). *(status: done; child signals the parent right after s_init(), before init_types())*
  - Fix: signal the parent right after `s_init()`.
- [x] **[verified] P2** Xsession runs about 15–25 short forks before dtsession: *(status: done; ksh93 CheckSyntax keeps the text of a profile that passed -n in ~/.dt/cache/syntax-ok.* (content compare, no fork; rechecks when the shell is newer); `dtsearchpath -ksh -cache` writes a ksh script valid only while every env variable and -d/-e/-f/-L test it used still holds (no cache on first login, ToolTalk mapping or changes); session environment byte-identical; 55.3 -> 44.6 ms per run, 2 processes fewer (9dbe4d9b3))*
  - `$(date)`;
  - log-rotation `rm`/`mv`/`touch`;
  - `hostname`;
  - `touch` and then `rm` of the same file (`:322,330`);
  - `ps -p $PPID | awk` (`_common.ksh.src:38`);
  - two `ksh -n` syntax pre-checks (`:384,516`);
  - `$(ls …/Xsession.d)`;
  - `uname -n`;
  - several `$(GetFirst …)` subshells.
  - Fix: ksh93 builtins (`printf '%(%c)T'`, `$(</proc/$PPID/comm)`, `: >file`, globs); drop or mtime-gate the `-n` checks; cache the dtsearchpath output keyed on directory mtimes.
- [x] **[verified] P3** `dtappgather` (`dtappg/dtappgather.C:117-131,270-300`) rebuilds the app-manager directory on every login. It calls `regcomp` **inside** the per-entry loop and never calls `regfree` (`libCliSrv/UnixEnv.C:291-302`). *(status: done; exact in-place update instead of an mtime gate: links with the right target are kept (readlink), only stale ones removed and missing ones made, so no cache file is written by the setuid binary; steady state makes no unlink/symlink and the Application Manager is never briefly empty; 189 -> 119 syscalls, 3.9 -> 3.6 ms (background) (7e2a6c54e))*
  - Fix: compile the regex once, use `d_type`, and skip when the source directory mtimes are unchanged.

### 2.3 dtsession
- [x] **[verified] P1** `SmMain.c:276-288`, `SmRestore.c:819-933`, `dtloadresources.src:159-169`: before dtsession opens its display it runs ksh → `tr`, `cat` → `xrdb` → cpp, which is 6–8 processes and **75–110 ms** (measured), with a blocking `wait()`. *(status: done; new SmResCache.c caches the dtsession_res result keyed on file contents, env, server visuals and tool stats: miss 23-44 ms, hit 0.5-3 ms; dt.resources is now written sorted so saves are stable. -nocpp for dt.resources not adopted (changes value semantics))*
  - Fix:
    - Cache the cpp output keyed on the input mtimes and the cpp symbols (WIDTH/HEIGHT/PLANES/CLASS); on a hit, do one `XChangeProperty`.
    - Load the saved `dt.resources` with `-nocpp` or in-process.
- [x] **[verified] P1 (non-dtwm WMs)** `SmRestore.c:646-701,3919-3949`: `WaitForWM` waits up to 60 s for `_DT_WM_READY`, which only dtwm sends. *(status: done; WM_S<n> owner, MANAGER ClientMessage or 200 ms owner poll; 10 s cap for non-dtwm WMs; mwm 60.3 -> 0.36 s, twm 60.2 -> 10.1 s)*
  - Fix: also accept ICCCM readiness (the `WM_S<n>` selection owner or a MANAGER ClientMessage); use a short fallback timeout.
- [x] **[verified] P2** Restored clients are not forked until the window manager is fully up (`SmRestore.c:2112-2141,4331-4400`). *(status: done; Dtsession*startClientsEarly (default True): BeginWM/FinishWM split; clients forked once dtwm has SubstructureRedirect and DtWsmGetWorkspaceList succeeds (10 ms poll), ~160 ms after dtwm starts; not used with contManagement handshaking or another WM. Placement identical in 76 runs with real dtwm; all windows up 30-70 ms sooner (513 -> 440 ms median), 7-app session ~575 -> 500 ms (4d974261f))*
  - Fix: fork dtwm and then the clients immediately, behind a resource, and measure placement.
- [x] **[verified] P2** `SrvPalette.c:1242-1285,1448-1449,485`: on TrueColor, about 40 synchronous `XAllocColor` calls plus an `XSync` per screen. *(status: done; pixels computed locally on a verified TrueColor visual (0 mismatches on 20000 colours at depth 24/30); one XSync after the loop)*
  - Fix: compute pixels from the visual masks; pipeline the rest.
- [x] **[verified] P2** `MarkFileDescriptors()` (`SmRestore.c:4755-4779`, called at `:909,3631,3805`) does `fcntl(F_SETFD)` for every fd up to `_SC_OPEN_MAX`, inside a `vfork` child, so the parent is suspended the whole time. The `vfork` children also call `putenv(strdup())` and `chdir`, which is undefined behaviour after `vfork`. *(status: done; all vfork() -> fork() with _exit(255) on exec failure; SmMarkCloseOnExec() uses close_range(CLOEXEC), /proc/self/fd or a loop. posix_spawn not used (children need setregid, setsid, chdir, putenv))*
  - Fix: `close_range(3,~0U,CLOSE_RANGE_CLOEXEC)` or `/proc/self/fd`; `posix_spawn`.
- [x] **[verified] P2** `dthello.c:609,653,667-675`: the full-screen "Starting CDE" cover stays up until the WM reparents a probe window. The fallback is 240 s. *(status: done; exits on a WM_S<n> owner, a MANAGER message or the new _DT_SM_WM_READY property; default timeout 240 s -> 15 s)*
  - Fix: also exit on `WM_S0` or on a property set by dtsession at READY; default timeout about 15 s.
- [x] **[verified] P3** `lib/DtSvc/DtUtil2/addToRes.c:440-506`: every `_DtAddToResource` fetches, re-parses (with an O(n²) duplicate check), sorts and rewrites **both** `RESOURCE_MANAGER` and `_DT_SM_PREFERENCES`, then calls `XSync`. That is about 4 round trips per call. *(status: done; addToRes.c: qsort dedup, one XSync per call (3000 entries 27.3 -> 6.0 ms); dtsession OutputResource batched into one call)*
  - Callers: `SrvPalette.c:319,658`, `SmRestore.c:1021`, and in dtwm `WmBackdrop.c:749-750`, `WmWrkspace.c:298,3311-3340,3737` (once per workspace in a loop).
  - Fix: batch everything into one call per save; hash-based dedup; no `XSync`.
- [ ] **[verified] P3** `SmMain.c:318,323`: a second full `XtOpenDisplay` for the color server. *(status: won't do: InitializeDtcolor runs at once and owns the palette selections, so it cannot be lazy; a shared connection breaks dialog colours (ColorObj asks the colour server during its own initialisation); presetting the Xrm database is ignored by libXt. Measured cost: 0.3 ms median (1.1 ms with a 58 KB RESOURCE_MANAGER) once per login)*
  - Fix: open it lazily, or share the connection.
- [x] **[verified] P3** `SmRestore.c:1301,1445,1486,1504-1521`: `RestoreSettings` re-applies the font path, pointer mapping, keyboard mapping (broadcasting MappingNotify to every client) and modifier mapping unconditionally. *(status: done; font path, pointer, keyboard and modifier mappings are set only when they differ from the server's)*
  - Fix: compare with the server's current state first.
- [x] **[verified] P3** `SmLock.c:527,540,563-575`: two `XSync`s and a duplicate pointer grab on lock. *(status: done; XSyncs and duplicate grab removed in round 1; the lock dialog of the kind the saver settings imply is pre-created in an idle work proc at startup and after each unlock (TrueColor/DirectColor only; colormapped displays keep lazy creation); lock request to keyboard grab 42 -> 10 ms median, locked screen pixel-identical (8c677b51a). Unlock + second lock not tested (no password headless))*
  - Fix: remove the duplicates; pre-create the lock dialog in an idle work proc.
- [x] **[verified] P3** `SrvFile_io.c:247-284` opendirs a whole directory to test whether one file exists; use `access()`. `SmGlobals.c:1316-1477` forks `mv`/`rm -rf` and waits at save and logout; use `rename(2)` and `nftw`. *(status: done; FindPalette does one lstat; MoveDirectory uses rename(2), RemoveFiles an in-process walk; mv/rm -rf only as fallback)*

---

## Phase 3 — Core libraries

### 3.1 DtSvc: data typing and the action/datatype database

dtfile performs about 10 of these lookups per file shown (`SharedProcs.c:678-935`).

- [x] **[verified] P1 (T1)** `DtsMM.c:462-489` `_DtDtsMMGetRecordByName` scans all of the roughly 700 DATA_ATTRIBUTES records linearly. The comment calls it "slow but sure". A boson→record `nameIndex` already exists (`MMDb.c:370-411`) and is used for actions (`DtsMM.c:116-120`). *(status: done; uses the nameIndex; byte-identical output)*
  - Fix: `int *p = _DtDtsMMGetDbName(db, boson); return p ? &rec_list[*p] : NULL;`
  - Also makes the dtfile Filter dialog (`Filter.c:1451-1457`) O(n) instead of O(n²).
- [x] **[verified] P1 (T2)** `Dts.c:1470-1680`, `DtsMM.c:683-697`: each attribute fetch costs about 7 mallocs and runs `expand_keyword` twice. `expand_shell` always allocates, so the second pass always runs. *(status: done; strdup fast path, _DtDtsMMExpandValueNoCopy for internal callers; 8.4-9.2 -> 1.6-1.9 us/file)*
  - Fix: a fast path when the value has no `$`, `%`, `` ` `` or `\`: return the mmapped string (`_DtDtsMMSafeFree` already accepts it).
  - Same pattern in `DtDtsDataTypeIsAction` (`Dts.c:1833`) and `DtDtsFindAttribute` (`:1909-1920`), which dtmail calls per MIME part.
- [x] **[verified] P1 (T3)** `Dts.c:1297-1298`: every evaluated criteria field does `malloc(1024)` plus `_DtDbFillVariables`, which runs `mblen` per character and scans all of `environ` per `$VAR` (`DbReader.c:496-508`). Only about 1 of 300 shipped criteria contains `$`. *(status: done; no FillVariables or malloc for values without $ or \; getenv(); 1024-byte overflow fixed)*
  - Fix: expand once and cache; use `getenv`.
  - Also: `strcpy` into a fixed 1024-byte buffer overflows for long values (`DtsMM.c:692`).
- [x] **[verified] P1 (T4)** `Dts.c:695-732`: content typing `mmap`s the whole file, or `read`s all of it when mmap fails (FUSE, network filesystems). The largest shipped CONTENT offset is 64 bytes. *(status: done; pread of a precomputed bounded prefix, O_CLOEXEC|O_NOATIME; 2 MB PostScript 63.5 -> 10.4 us. The bound is computed per mapping, so the cache format is unchanged)*
  - Fix: compute `max_content_end` at database build time and `pread` only that many bytes; use `O_NOATIME|O_CLOEXEC`.
- [x] **[verified] P2 (T5)** `Dts.c:309-514`: per-file setup does `calloc`, `strdup`, a copy of the caller's `struct stat`, `getcwd` and two `strcat`. *(status: done; type_info_t on the stack, borrowed buffers; 22-26 -> 4.5-5.6 us/file; max_buf overflow fixed)*
  - Fix: stack-allocate it and borrow the caller's buffers.
  - **Bug:** `max_buf` (`:301-305`) reallocs with the *old* size, so the `strncpy` at `:1321` overflows the heap for sub-expressions longer than 100 bytes.
- [x] **[verified] P2 (T6)** `Dts.c:127-287`, `DtNlUtils.c:659-710`: `gmatch` calls `mbtowc` per character, `_MBADV` calls `mblen` twice per step, and `next_sep` → `_is_previous_single` rescans from the start (O(n²)). *(status: done; byte-wise matcher for pure-ASCII strings (0 mismatches on 5M random pairs in 3 locales); wide path kept for non-ASCII)*
  - Fix: detect UTF-8 once and use byte-wise `fnmatch`/`strpbrk`.
- [x] **[verified] P3 (T7)** `Dts.c:742-744,782-783`: the "cached" database pointer is a non-static local, so it is looked up on every call. *(status: done; cached in statics keyed on a new mapping generation counter)*
- [ ] **[verified] P3 (T8)** `DtsMM.c:73-101`: `_DtDtsMMGetPtr` takes the process lock on every address computation, and typing holds the global locks across blocking file I/O. *(status: won't do: no process lock once mapped (round 1); the remaining locks expand to nothing without XTHREADS (nm: no XtProcessLock/XtAppLock in Dts.o/DtsMM.o), so narrowing them changes nothing; an XTHREADS build would also need a reference-counted mapping)*
- [x] **[verified] P3 (T9)** `Dts.c:1028-1064`: an extra `open("<dir>/.DtDirDataType")` per directory, with an unbounded `sprintf`. When `fstat` fails, `strstr(NULL)` crashes. *(status: done; folders without .DtDirDataType remembered by dev/ino while their mtime is unchanged (not for folders changed in the last 2 s, errors other than ENOENT, or a dangling link of that name); 65,536 entries, cleared by DtDtsSetDataType; 7.7 -> 4.8 us per folder, no open() on rereads; types byte-identical (83e744059))*
- [ ] **[verified] P3 (T10)** `Dts.c:1594-1651`: backticks in an attribute value `popen` a shell on every lookup, i.e. per file in dtfile. *(status: won't do: values without a backquote skip expand_shell (round 1); typing all 3,284 attribute values of the 404 shipped types forks nothing (the 11 backquotes are in EXEC_STRINGs); caching command output would change what user commands return)*
- [x] **[verified] P2 (A1)** `ActionFind.c:297-310,517-529,666-678`: label, icon and description lookups stop early only when they compare bosons, which are in insertion order, so they usually scan to the end. *(status: done; scans only the run of matching records; NULL start crash fixed)*
  - Fix: `for (n=*start; n<cnt && list[n].recordName==q; n++)`.
  - Bug: NULL `start` is dereferenced at `:295,665`.
- [x] **[verified] P2 (A2)** `ActionFind.c:753-777,1876-1895`: field lookups re-hash constant names and allocate; this happens 4× per candidate record on every action lookup. *(status: done; field names quarked once per mapping, in-place compares; 7.5 -> 3.1 ms for 1,428 lookups; 1 KB stack strcpy and leaks fixed)*
  - Fix: precompute the field bosons, as `Dts.c:1270-1284` already does.
- [x] **[verified] P1 (B1)** `DtsMM.c:336-363`, `DbLoad.c:299-308`: when the shared cache fails validation, every client builds a private cache under a `tmpnam` name and unlinks it, i.e. it re-parses about 100 `.dt` files per process start. *(status: done; stale/missing shared cache rebuilt under the shared name (mkstemp + rename, mapped from the fd); private caches are unlinked files, no tmpnam)*
  - Fix: rebuild into the shared name. `write_db` already does `mkstemp` + `rename`, so this is race-safe.
- [x] **[verified] P2 (B2)** `DbLoad.c:122-125`, `DtsDb.c:393-404,438`: building the database is O(actions × records), about 0.5M compares. `_DtMMAddActionsToDataAttribute` (`MMDb.c:304-323`) adds about 400k more, and `add_if_missing` runs `qsort` after each insert (`:285`). *(status: done; name->record hash for unsorted dbs, sorted insert in add_if_missing; cache build 17.4 -> 6.5-6.9 ms, identical dumps)*
  - Fix: a temporary quark→record hash; sort once.
- [x] **[verified] P3 (B3)**: *(status: done; geometric growth, qsort, no strlen in the loop, single allocation)*
  - Arrays grow by +10 (`DtsDb.c:439-450,502-513`).
  - Bubble sort with `strcoll` (`DbUtil.c:568-575`).
  - `strlen` in the loop condition of `clean_line` (`DbReader.c:621-622`).
  - `realloc` +1 (`MMDb.c:600-633`).
- [x] **[verified] P2 (B4/V1)** `DtsMM.c:490-543,640-680`: validation does `getcwd` + `chdir` into every database directory and `stat`s every entry *before* the suffix test. The "hash" is a byte sum. `stat` errors are ignored. A newly added `.dt` file is never noticed, which is why the cache gets deleted "to be safe". *(status: done; new cache format v2 with magic/version, stored search path and ns stamps; fstatat validation, no chdir/getcwd; 202 -> 69-89 us)*
  - Fix: `openat`/`fstatat` with `d_type`; store directory mtimes and the real path list (or a strong hash) in the header; check errors.
- [x] **[verified] P3 (B5)** `strtab.c:100-107,270-277`: *(status: done; FNV-1a, 2x buckets, 32-bit links; format version bumped)*
  - `j += *s++ << i++` is undefined behaviour for strings longer than 31 bytes.
  - The load factor is 1.
  - `unsigned short` indices cap the table at 65,535 entries.
  - Fix: FNV-1a, 2× sizing, 32-bit links; bump the cache format version.
- [x] **[verified] P3 (B6)** `DbUtil.c:441-451,660-672`: `DtCharCount` twice per entry and a `stat` per match. *(status: done; d_type and memcmp suffix test; per-match stat gone)*
  - Fix: `d_type` and a `memcmp` suffix test.

### 3.2 DtSvc: startup, IPC, subprocesses
- [x] **[verified] P1 (S1)** `DtUtil2/EnvControl.c:258-303`: when `$SESSION_MANAGER` is unset, `DtInitialize` calls `XOpenDisplay(NULL)` + `XInternAtom` + `XGetWindowProperty` + `XCloseDisplay`. That is a whole second connection per app start. *(status: done; session manager lookup resolved on the caller's display in DtInitialize; private connection only for apps that never call it; 2 -> 1 connections)*
  - Fix: pass the caller's `Display*` down.
- [x] **[verified] P2 (S2)** `GetMwmW.c:119-150`: every `DtWsm*` call (44 call sites) does `XGetWindowProperty` + `XQueryTree(root)` + a linear search. `SmComm.c:847-868` does the same. *(status: done; per-(display, root) cache, revalidated on BadWindow; 3 -> 1 round trip per call)*
  - Fix: cache per display and screen; revalidate on BadWindow or `PropertyNotify`.
- [x] **[verified] P1 (E1)** `DtEncap/spc-exec.c:165-172` (`SPC_Close_Unused`, used by `pipe.c:250`, `pty.c:657`, `noio.c:349`) `close()`s every fd up to `sysconf(_SC_OPEN_MAX)` on every action launch. *(status: done; _DtSvcCloseFrom (close_range/closefrom) in DtEncap and CmdMain; dtsession, tt (mp_ptype, mp_server, tttrace) and dtexec/dtimsstart fixed too; spawn at RLIMIT_NOFILE 524288: 48-57 -> ~7 ms)*
  - Fix: `close_range`/`closefrom` or `/proc/self/fd`.
  - The same pattern exists in:
    - `CmdMain.c:1005-1014`
    - `dtsession/SmRestore.c:4755`
    - `tt/slib/mp_ptype.C:391-395`
    - `tt/bin/ttsession/mp_server.C:513-516`
  - Bug: `dtexec/Main.c:686` only covers fds below `FOPEN_MAX` (16 on glibc), which leaks fds.
- [x] **[verified] P3 (E2)** `local.c:196-208`: an extra `select(max_fds,…)` per subprocess-output event. With `max_fds > FD_SETSIZE` this is undefined behaviour. *(status: done; poll() on the one fd; pty.c busy loop and FIONREAD size fixed too)*
  - Fix: `poll()`, or drop it.
- [x] **[verified] P3 (E3)** `Action.c:3627-3646`: command strings are built with `strcat` (O(n²)). `_DtIsSameHost` (`ActionUtil.c:452-512`) does two uncached `gethostbyname` calls per argument. *(status: done; length-tracking buffer (10000 args 194.6 -> 0.29 ms); _DtIsSameHost memoized 60 s; 65-byte host buffer overflow fixed)*
- [x] **[verified] P2 (X1)** `XlationSvc.c:1190-1239,1919-2021`: every locale translation enumerates the whole Xrm database and does `regcomp`/`regexec`/`regfree` per candidate. dtmail does this 3–5× per body part. *(status: done; 256-entry memo per db (10-13 ms -> 35-80 us warm); use-after-free in _DtXlateMergeDbs fixed. Precompiled regexes not done)*
  - Fix: a memo cache; precompiled regexes; compare quark ids.
- [ ] **[verified] P3 (M1)** `DtXinerama.c:41-68`: `XineramaQueryScreens` is a round trip on every call, and it leaks. dtfile calls it per dialog placement (`Encaps.c:2495`). *(status: won't do: the library cannot know when to drop a cache (XSelectInput on the root would replace the client's mask; RandR needs libXrandr and an event hook non-Xt callers lack); _DtXineramaInit costs 59-196 us against 1.3-3.3 ms per dialog; dtwm already caches; SharedProcs leaks fixed in round 1)*
  - Fix: cache per display; invalidate on RandR events.
- [x] **[verified] P3** `DtHelp/Environ_c.c:86` runs `popen("uname -n")`; use `uname(2)`. *(status: done; uname(2))*

### 3.3 ToolTalk (lib/tt, ttsession, rpc.ttdbserver)
- [x] **[verified] P2 (A2)** `realpath` runs twice per mapping (`tt_db_network_path.C:57` and `tt_file_system.C:252`; `tt_path.C:246-252`). For non-existent paths it is O(depth²). *(status: done; netfile paths resolved once (round 1); _tt_realpath stats each prefix and calls realpath() only on the first that exists (EACCES prefixes still go to realpath); 0 differences over 180k generated paths; 10 missing components 197 -> 45 syscalls; dm_server.C's unused copy removed)*
- [ ] **[verified] P1 (A3)** `mp_c_message.C:186-270`, `tt_db_file.C:400-417`: every file-scoped send makes a synchronous ttdbserver RPC. *(status: won't do (rest): passing the cache level skips only the property read (~17 us CPU) and is unsafe (a re-created file restarts at level 0, so a cached session list goes stale); a session TTL cache would starve sessions that join inside the TTL; one-way TT_FILE dispatch loses qmsg_info, which the sender uses to queue for TT_QUEUE observers. Measured round trip 78-430 us. File scope verified end to end with rpc.ttdbserver (join/quit, ADD/DELETE_FILE_PROP))*
  - The client defeats the protocol's own cache by forcing `dbFilePropertiesCacheLevel = -1` (`:406`).
  - The server then does about 6 ISAM reads plus the 100 ms mount sleep.
  - TT_FILE sends use the synchronous dispatch.
  - `check_if_sessions_alive` pings on every send.
  - Fix: pass the cache level through; cache sessions per `_Tt_file` with a TTL; use one-way dispatch.
- [x] **[verified] P2 (A4/A5)** When rpc.ttdbserver is unreachable there is no negative cache: each retry costs `gethostbyname` + portmapper + syslog (`tt_db_hostname_global_map_ref.C:76-90`). `tt_host_file_netfile` opens a new dbserver connection per call (`api_filemap.C:182,243`). *(status: done; 8-entry 30 s negative cache per host; per-host connection reuse (2.16 -> 0.04 ms per call))*
  - Fix: a 30 s per-host negative cache; reuse connections through `getDB`.
- [x] **[verified] P3 (A6)** `tt_host_equiv.C:76-81,298`: the host-equivalence DNS cache is created per instance and thrown away. *(status: done; one shared table, 300 s / 30 s TTLs; h_addr_list copy and cursor reset fixed)*
  - **Bug:** `:298` copies the bytes of the `h_addr_list` pointer array, not the address.
- [x] **[verified] P1 (B1)** `mp_session.C:212-219`, `mp_desktop.C:162-174,382-390`: every libtt client opens and **keeps** a second X connection just to read one property. *(status: done; the X connection is opened only when needed and closed after reading TT_SESSION; 4 -> 3 sockets per client)*
  - Fix: close it after reading, or export the address in the environment from dtsession.
- [x] **[verified] P1 (B2)** `mp_rpc_client.C:152-157`: port 0 forces an rpcbind query on every connect. The AF_UNIX transport exists but is disabled on Linux (`tt_options.h:232-233`). *(status: done; port field in the address (round 1) plus AF_UNIX RPC on $XDG_RUNTIME_DIR/tooltalk/<pid>.<prog>: directory must be a real 0700 dir owned by the user, SO_PEERCRED checked both ways, TCP fallback (no XDG_RUNTIME_DIR, long path, old or killed ttsession, TT_UNIX_SOCKET=0); replaces the never-built OPT_UNIX_SOCKET_RPC; open-close 288 -> 205 us, ping 208 -> 182 us)*
  - Fix: add the port as an 8th address field (old clients ignore it); enable unix sockets under `$XDG_RUNTIME_DIR`.
- [x] **[verified] P1 (B3)** `tt_open` makes at least 3 synchronous ttsession round trips plus a connect-back: `VRFY_SESSION`, `ALLOC_PROCID_KEY`, then `SET_FD_CHANNEL` with the server connecting back (`mp_c_session.C:97`, `mp_c_procid.C:395,777-788`, `slib/mp_s_procid.C:1292`). *(status: done; new TT_RPC_OPEN_PROCID (49) = ALLOC_PROCID_KEY + SET_FD_CHANNEL and replaces the VRFY_SESSION ping when there is no start token; per tt_open 3 sync RPCs -> 1; older ttsession answers PROCUNAVAIL and the client falls back (remembered per address); exports, class layouts and address format unchanged; all four old/new combinations tested)*
  - Fix: one combined OPEN RPC.
- [x] **[verified] P1 (B4)** `tt_global_env.C:77-111`, `tt_host.C:172,222,255`: the client does `gethostbyaddr` on the session IP and `gethostbyname` twice on every `tt_open`, which can mean resolver timeouts of seconds. *(status: done; one forward lookup, no reverse lookup on the client (gethostbyname 2 -> 1, gethostbyaddr 1 -> 0))*
  - Fix: use the dotted address only; one forward lookup.
- [x] **[verified] P1 (B5)** `tttk/ttdtprocid.C:75-77`, `ttdesktop.C:156`, `mp_c_procid.C:96-102`: `ttdt_session_join` makes about 13 synchronous `ADD_PATTERN` RPCs plus `JOIN_SESSION` per application. *(status: done; TT_RPC_ADD_PATTERNS/DEL_PATTERNS (50/51), up to 256 per call with one status each; tttk registers and destroys its pattern sets in one call; per ttdt_session_join 12 ADD_PATTERN -> 1 (ttdt-open-join 1.53-1.73 -> 0.77-1.09 ms); PROCUNAVAIL fallback; API tracing unchanged)*
  - Fix: a batched `ADD_PATTERNS` RPC or a one-way variant.
- [x] **[verified] P2 (B6)** `dtexec/Main.c:945,1247-1262`: every ToolTalk-tracked action pays the full `tt_open` cost. B1–B4 fix it transitively; also inherit `TT_SESSION`. *(status: done; dtexec sets TT_SESSION from the caller's -ttprocid address only around tt_open() (child already forked), so libtt opens no X connection; libDtSvc dlopen()ed on first log message (ldd 44 -> 23 DSOs); spawn to _DtActDtexecID 13.6-16.6 -> 10.3-11.9 ms, syscalls 803 -> 509 (789fd269c))*
- [ ] **[verified] P2 (B8)** `dtmail/libDtMail/Common/FileShare.C:55,247-278`: mailbox open makes two blocking file-scoped requests with 900 s timeouts. *(status: won't do: FileShare runs only with cdetooltalklock; without a handler ttsession fails the request at once; the 900 s only elapses when another dtmail holds the lock and does not reply, and a shorter timeout would take over a live session's mailbox; dispatching X events during the wait would let the user re-enter the half-done open)*
- [x] **[verified] P2 (C1)** `slib/mp_s_procid.C:846-879`: ttsession pushes only the top message per `NEXT_MESSAGE` RPC, so N notices cost N round trips; the client already accepts a list (`mp_c_procid.C:502-509`). *(status: done; up to 32 queued notices per NEXT_MESSAGE reply; 1000 notices 500-1085 -> 210-250 ms; old/new clients interoperate)*
  - Fix: batch them, or finish `OPT_ADDMSG_DIRECT` (`mp_s_procid.C:160-181,944-1065`).
- [x] **[verified] P2 (C2)** `slib/mp_s_message.C:1051-1065`: the opless pattern list is copied on every delivery attempt. *(status: done; match_patterns walks both lists without copying)*
- [x] **[verified] P3 (C3)**, correctness: `mp_s_message.C:1176-1181` uses `return(0)` where `continue` is meant, so one inactive registrant aborts matching for every remaining pattern. *(status: done; continue instead of return(0))*
- [ ] **[verified] P3 (C4/C5/C6)**: *(status: won't do (rest): C6 done (round 1); C4 done: file-scope refcount map and client pattern handles hashed by id (patterns-10k client CPU 1,472 -> 35 us per pattern); C5 done in ttsession (opful_pats unordered_map, 10k ops lookup 1.72 -> 0.28 us). libtt's _Tt_object_table left as is: changing its layout crashed the installed ttsession and a new hash would change handler choice and types.xdr order)*
  - Pattern and file-scope bookkeeping uses linear lists keyed by strings (`mp_s_procid.C:126-150,350-387`, `mp_s_mp.C:624-674`).
  - Hash tables have a fixed size and a weak hash (`tt_string.C:901-916`, `mp_s_mp.C:86-91`).
  - The client handle table is a linear list searched per message (`api_handle.C:108-281`).
- [x] **[verified] P3 (C7/C8/C9)**: *(status: done; 2 sigaction calls instead of 3; operator[] remembers the last position (96 -> 0.6 ms for 5000 args); poll() everywhere)*
  - 3 SIGPIPE syscalls per RPC (`mp_rpc_client.C:304-337`).
  - `operator[]` on lists is O(n) (`tt_object_list.C:265-271`), so per-arg APIs are O(n²).
  - `select()` is passed `RLIMIT_NOFILE` on a 1024-bit set (`mp_mp.C:295`).
- [x] **[verified] P1 (D1)** `slib/mp_s_session.C:209-262`: ttsession **holds an X server grab** for the whole of: *(status: done; grab held only for a compare-and-swap of the property (2 GetProperty + 2 ChangeProperty); 4 concurrent starts -> 1 survivor)*
  - checking for a live session (an RPC with the effectively infinite timeout);
  - portmapper registration (about 100 program-number probes, `mp_rpc_server.C:447-492`);
  - locking the auth file with 10 × `sleep(2)`.
  - The whole display freezes at login, for 20 s or more with a stale lock.
  - Fix: grab only around read-property/write-property, or use `XSetSelectionOwner` as the lock.
- [x] **[verified] P3 (D3/D4)**: *(status: done; run_until uses poll()/svc_getreq_poll; type install is one pass (synthetic db 102-177 -> 34-54 ms))*
  - The `fd_set` is rebuilt on each wakeup and `select(FD_SETSIZE)` is used (`mp_rpc_server.C:299-322`); move to `poll`/`svc_getreq_poll`.
  - Type install is O(ptypes × signatures) (`mp_s_mp.C:364-429`).
- [x] **[verified] P2 (E2)** `mini_isam/isamhooks.c:62-65`: the ISAM buffer cache is invalidated after every ISAM call. *(status: done; buffers kept across calls while changestamps match; read() syscalls 870k -> 392k in the benchmark)*
  - Fix: keep buffers while `changestamp1` is unchanged.
- [x] **[verified] P2 (E3)** `tt_db_server_db.C:1205-1262`: `getFileKey` reads to EOF on a miss, and on any path of 120 characters or more. *(status: done; exact-key lookup (2000 lookups 3.94 -> 0.74 s); paths >= 120 bytes now stored and found)*
  - Fix: stop as soon as the prefix differs.
- [x] **[verified] P2 (E4/E5)**: *(status: done; in-place cache-level counters (5000 sets 0.655 -> 0.39-0.44 s CPU); one isfsync per table (8013 -> 12 fsyncs for 1000 files))*
  - Cache-level bookkeeping triples the cost of each write (`db_server_functions.C:600-634,2257`).
  - `_tt_delete_session_1` fsyncs after every deleted row (`:1880-1970`).
- [x] **[verified] P3 (E6)** `dm_server.C:1940-1959`: a linear 128-slot open-file search. *(status: done; cheap tests first; no hash needed)*

### 3.4 DtWidget
- [x] **[verified] P1 (W1)** `Icon.c:1761-1766,2241-2243,2315-2317,2353-2355`: a blocking `XGetGeometry` per pixmap set. The same file already avoids it in `LoadPixmap` (`:3661`) via `XmeGetPixmapData`. *(status: done; pixmap sizes from XmeGetPixmapData (100 icons: 146 -> 46 replies); image names were already compared with strcmp (the pointer claim was wrong); height clamp fixed)*
  - Also, `SetValues` compares image-name **pointers** (`:2097-2098`), so dtfile's fresh strings force a full reload every time.
  - Bug: `:3663` clamps the height with `G_MaxPixmapWidth`.
- [x] **[verified] P3 (W2)** `MenuButton.c:637-643` and `DtUtil1/DndIcon.c:127-166` make the same blocking round trip (2 per drag start). *(status: done; DtDndCreateSourceIcon reads size and depth from Motif's pixmap cache (_XmGetPixmapData, does not add pixmaps), XGetGeometry only for uncached pixmaps; no drag start round-trips any more (8328af3a8))*
- [x] **[verified] P2 (W3)** `ComboBox.c:1973-2025,2980-3066`, `SpinBox.c:1986-1990,2533-2621`: each add or delete re-measures **every** item with `XmStringExtent`, and SpinBox grows its array by one. Adding n items is O(n²). *(status: done; running max per widget in ItemExtent.c; 150M -> 85k XmStringExtent calls; SpinBox grows geometrically; DtSpinBoxDeletePos off-by-one fixed)*
  - Fix: keep a running max; grow geometrically.
- [x] **[verified] P2 (W4)** `Editor.c:4617-4632`: the status line passes `FORCE`, so `XmTextFieldSetString` runs on every cursor move. *(status: done; status field set only when its text differs; 38985 -> 19005 requests per 1000 moves)*
  - Fix: `DONT_FORCE`.
- [x] **[verified] P2 (W5)** `SearchCalls.c:607-660`: Replace All does one `XmTextReplace` per hit, each with a redraw, a line-table update and undo bookkeeping. *(status: done; hits found on one copy emulating ReplaceAll exactly, runs of hits replaced with one XmTextReplace each (never across cursor, selection ends or view edges), undo rebuilt per hit; word wrap and some multibyte cases still go hit by hit; identical on 20,000+ random cases; 100k hits 3.7 s -> 32 ms (5f43d28a1))*
  - Fix: one pass and one replace, inside `DtEditorDisableRedisplay`.
- [x] **[verified] P2 (W6)** `EditAreaData.c:153-189` `StripEmbeddedNulls`: `strlen` from the start plus a `memcpy` of the tail per NUL run, which is O(n·k). The overlapping `memcpy` is undefined behaviour. *(status: done; one compaction pass)*
  - Fix: one compaction pass.
- [x] **[verified] P2** `EditAreaData.c:119-138,1407-1581`: *(status: done; no per-character probe or second copy; CopySubstring walks the string once; 15 MB UTF-8 doc peak RSS 185 -> 122 MB; mb_str_loc bug gone)*
  - `Check4EnoughMemory` mallocs and frees 2.1·N bytes on load and save.
  - Save sizes its buffer as `N·MB_CUR_MAX`, i.e. about 12.6× the file size in UTF-8.
  - `CopySubstring` fetches the whole document (`XmTextGetString`).
  - Fix: drop the probe; stream the output with `XmTextGetSubstring`.
  - Check correctness: `mb_str_loc` is indexed from the start of the string but used relative to `startPos` (`:1447-1468`).
- [x] **[verified] P2** `EditCalls.c:263-319`: continuous deletion appends to undo with `strlen`/`strcpy`/`strcat` of the whole run, which is O(n²). *(status: done; XtRealloc + memcpy/memmove; 20000 backspaces 1.65 -> 0.95 s)*
  - Fix: a growable buffer.
- [x] **[verified] P3** `SearchCalls.c:95-200`: spell check copies the text, uses `tmpnam` (a security issue) and reads the pipe synchronously, adding one list item at a time. *(status: done; mkstemp, allocated command line, one XmListAddItemsUnselected (round 1); filter output now read through XtAppAddInput, dialog managed at end of output; a 0.5 s filter returns in 4 ms instead of 517 ms (dc3bd0454))*
  - Fix: `mkstemp`, `XtAppAddInput`, and one `XmListAddItems`.
- [x] **[verified] P3** `Editor.c:5346-5530`: Format allocates per line and writes indentation one byte at a time. *(status: done; fixLeftMarginAndNewlines fetches the range once; identical on 1,800 random cases; 1 MB: 34,283 -> 7,434 mallocs (6a262acb0))*
- [x] **P3** `Control.c:1863-1884,1970`: front-panel animation frames are all loaded eagerly at dtwm startup. *(status: done; frames stored by name and loaded on first animation; 67 -> 1 request when adding 22 frames)*
  - Fix: load them on first push.

---

## Phase 4 — dtwm (port from Motif where possible)

### 4.1 Move, resize, placement
- [x] **[verified] P1** `WmWinConf.c:3583-3731` `GetConfigEvent`: up to 300 `XQueryPointer` round trips per idle step (`CONFIG_POLL_COUNT`, `:69`). With `freezeOnConfig` off it is an unbounded spin. *(status: done; ported 5953033e and d4bb5c3c; PGRAB_MASK always; 3.09 -> 1.01 round trips per opaque drag step)*
  - Fix: port motif `5953033e`: block in `XWindowEvent` and query once per motion hint. Also use `PGRAB_MASK` (`WmCPlace.c:53,231,645`).
  - Apply `d4bb5c3c` too: dtwm builds the `WindowOutline` path.
- [x] **[verified] P1** `WmWinConf.c:1516-1547` `FlashOutline`: `while (!XtAppPending) { draw; XSync; }` is a 100% CPU busy loop. *(status: done; single draw/erase pair; flashing paced from poll(); idle drag CPU 26% -> 0.8%)*
  - Fix: port `5953033e`; pace the flash from a timer.
- [x] **[verified] P2** `MoveOpaque` → `PullExposureEvents` (`WmWinConf.c:1387`, `WmEvent.c:2550-2565`): 2 × `XSync` (both connections) per motion step. Motif still has this too. *(status: done; PullQueuedExposureEvents without XSync; exposures handled from poll() while idle; 0 stale pixels)*
  - Fix: drain only events already queued.
- [x] **[verified] P2** `WindowOutline` (`WmWinConf.c:1669-1748`): unmaps and remaps 4 windows per step, which forces repaints underneath, then calls `PullExposureEvents`. *(status: done; outline windows stay mapped and are only moved; idle CPU 29% -> 0.04%)*
  - Fix: keep them mapped and only move them.
- [x] **[verified] P2** `WmWinConf.c:2653-2696`: the default configuration (`freezeOnConfig`, outline move) holds an `XGrabServer` for the whole drag. *(status: done; Dtwm.defs ships freezeOnConfig: False and useWindowOutline: True; no server grab during drags)*
  - Fix: after the fixes above, ship `freezeOnConfig: False` or `moveOpaque: True` in `Dtwm.defs.src`.

### 4.2 Workspace switching
- [x] **[verified] P1** A front-panel switch goes panel → `DtWsmSetCurrentWorkspace` → ttsession → dtwm `RequestMsgCB` → `ChangeToWorkspace` → tt notice → ttsession → panel `WorkspaceModifyCB` (`Callback.c:815-905`, `WmIPC.c:727-751,465-500`, `WmWrkspace.c:246`, `WmFP.c:906`). That is two IPC hops, and nothing happens at all without ttsession. *(status: done; SwitchButtonCB calls ChangeToWorkspace directly; click to property 3.0-3.3 -> 1.9-2.0 ms)*
  - Fix: call `ChangeToWorkspace` directly and update the buttons in-process.
- [x] **[verified] P1** `Callback.c:581`: `WorkspaceModifyCB` always calls `DtWsmGetWorkspaceInfo`, about 3 round trips including `XQueryTree(root)`, but only ADD and TITLE use the result. *(status: done; workspace info fetched only for ADD and TITLE)*
- [x] **[verified] P2** `Callback.c:784-786,878-880`: panel title and icon name are written twice per switch, which triggers dtwm's own PropertyNotify handling. *(status: done; written once per switch)*
- [x] **[verified] P2** `WmWrkspace.c:117-249`: old windows are unmapped first (which paints the old backdrop), then the backdrop changes, then new windows are mapped in array order. Everything is painted twice. *(status: done; backdrop first, top-down map, old windows hidden last; repaint area 1800 -> 209 kpixels per switch; byte-identical screenshots)*
  - Fix: switch the backdrop first, map in stacking order from the top down, then unmap the old windows.
- [x] **[verified] P2** `WmWinState.c:192-193,335-345,538-550,766-807`: hiding a client rewrites an unchanged `WM_STATE`, and shown clients are mapped twice. Transient trees are walked twice. *(status: done; SetWMStateOnce skips unchanged writes; the duplicate XMapWindow pair kept (no round trip))*
  - Fix: remember the last `WM_STATE` value written.
- [x] **[verified] P3** `WmProperty.c:1258-1272`: an `XSync` on every switch ("XFlush didn't work here, why?"). `:1516-1524`: an `XFlush` per client in `SetWorkspacePresence`. *(status: done; XSync removed: workspace notices and replies wait in a queue behind a zero-length property append and go out once its serial is processed (250 ms timer fallback), which also fixes a TITLE/ADD/DELETE race (old title 3/60 -> 0/60); duplicate front-panel PropertyNotify and WM_ICON_NAME read-back gone; round trips per switch 2.43 -> 0, 2.97-3.03 -> 2.74-2.76 ms per switch, dtwm CPU +10% (6128587b9))*

### 4.3 Mapping new clients
- [x] **[verified] P1** `WmEvent.c:2477-2524` `GetTimestamp`: property change + `XSync` + an O(queue) `XCheckWindowEvent`, at 31 call sites. In motif's measurements it made up 42% of CPU when 500 clients mapped. *(status: done; ported 00e4c1e7 (SYNC SERVERTIME counter))*
  - Fix: port motif `00e4c1e7` (the SYNC SERVERTIME counter).
- [x] **[verified] P2** `WmManage.c:701`: a timestamp for a withdrawn client is never used; pass `CurrentTime` (port `27b2dea6`). `:673` + `:728/733`: merge the `XSync` and the timestamp (port `ee31dae9`). *(status: done; ported 27b2dea6 and ee31dae9; map: 18 -> 10 round trips and 634 -> 291 us dtwm CPU per client)*
- [x] **[verified] P2** Four property reads skip the `HasProperty` check: *(status: done; ported 6fe46a70)*
  - `WmWinInfo.c:858,905`
  - `WmProperty.c:811,1046`
  - Fix: port `6fe46a70`.
- [x] **[verified] P2** `WmOL.c:115-134`: an inverted check makes almost every non-Motif client pay an extra `_OL_WIN_ATTR` round trip; the property is then read twice (`:330`). *(status: done; HasProperty-guarded, read once; up to 3 round trips saved per non-Motif client)*
- [x] **[verified] P3** `WmCEvent.c:2783-2806`: duplicate `XGetWindowAttributes` (port `721f7a7c`). `WmMenu.c:1538` round-trips on every menu activation; use `XtScreen(w)`. *(status: done; ported 721f7a7c; menu screen taken from the widget)*
- [x] **[verified] P1** `WmImage.c:160-266` `MakeNamedIconPixmap`: for **every** new client with an `iconImage`, which includes every dtterm: *(status: done; NamedIconCache with negative results; also fixes a per-client pixmap leak; 14 -> 10 round trips per Dtterm map)*
  - the failed `XReadBitmapFile` is not cached;
  - the color XPM is re-decoded (libXm does not cache it);
  - `XSync` + `XGetGeometry`;
  - a copy into a new pixmap;
  - then `XmDestroyPixmap`, so the next client repeats everything.
  - Fix: cache per (path, fg, bg, size), including negative results; `XmeGetPixmapData`.
- [x] **[verified] P2** `WmWinInfo.c:1250-1300`: every `WM_HINTS` change rebuilds the icon pixmap (an `XGetGeometry` plus a copy) even when the ids are unchanged. *(status: done; icon rebuilt only when the pixmap/mask ids change)*
  - Fix: store the last ids.
- [x] **[verified] P3** `WmManage.c:612-651`: key grabs on the shared root icon frame are repeated once per workspace. *(status: done; skipped for already handled icon frames)*
- [x] **[verified] P3** `WmWrkspace.c:774`: an atom is interned on every manage. *(status: done; static Atom)*
- [x] **[verified] P3** `WmXSMP.c:464-597`, `WmWrkspace.c:976-1000`: proxy session matching reads `WM_COMMAND` and `WM_CLIENT_MACHINE` twice and scans the whole proxy database, i.e. O(clients × entries). *(status: done; proxy list built once per client DB, match remembered; WM_COMMAND/WM_CLIENT_MACHINE not read without proxy entries)*

### 4.4 Titles, decorations, backdrops
- [x] **[verified] P2** `WmCEvent.c:872-881`: title PropertyNotify events are processed one by one; coalesce stale ones (port `28c47e40`). *(status: done; ported 28c47e40; 10000 WM_NAME changes 40.5 -> 1.7 us each)*
- [x] **[verified] P2** `WmWinInfo.c:2147-2160,2383-2391`: each `WM_NAME` change reads `_NET_WM_NAME` *and* `WM_NAME` (2 round trips). *(status: done; titleFromNetWmName flags; 2 -> 1 round trips per change)*
  - Fix: keep a `hasNetWmName` flag.
- [x] **[verified] P3** `WmGraphics.c:1111-1139`: `XmStringWidth` on every title draw. Cache it (port `7242649d`). *(status: done; width cached per title and font list)*
- [x] **[verified] P3** `WmCEvent.c:854-869`: a `_MOTIF_WM_HINTS` change rewrites the **client's** `WM_NORMAL_HINTS`, which is also a correctness bug. *(status: done; ProcessWmNormalHintsNoMaxSize works in memory; the client's property is no longer rewritten)*
- [x] **[verified] P3** `WmProperty.c:1953-2002`: `_NET_WM_STATE` is updated read-modify-write. `WmIDecor.c:1815,1840,2023` queries values dtwm already knows. *(status: done; _NET_WM_STATE: no change needed (WmEwmh.c computes and writes only on change); WmIDecor: XGetGeometry dropped from ShowActiveIconText and 3 redundant MoveActiveIconText calls removed, icon focus 2 -> 1 and icon move 3 -> 1 round trips (a680edf26))*
- [x] **[verified] P2** `WmResource.c:5155,5158`, `WmBackdrop.c:300-429`: each backdrop is processed twice at startup, and TILED backdrops are copied into a duplicate pixmap. *(status: done; TILED backdrops skip the second pass and share the Xm-cache pixmap)*
- [x] **[verified] P2** `WmBackdrop.c:722`: pixmaps that dtwm created are freed with `XmDestroyPixmap`, which fails for pixmaps not in Xm's cache, so **each backdrop change leaks a pixmap**. Each workspace also keeps a full-screen window and, for fit or fill modes, a full-screen pixmap (about 33 MB per workspace at 4K). *(status: done; leak and pixmap sharing (round 1); workspaces with the same pixmap or colour now share one backdrop window (copy on write on change, destroyed when unused), so switching between them maps nothing; 400 switches Xvfb CPU 780 -> 20 ms; root hashes identical in 6 setups (59675b258). One window with swapped background for different backdrops: no gain measured)*
  - Fix: `XFreePixmap`; share identical backdrops; use one backdrop window and swap its background.
- [x] **[verified] P3** `WmBackdrop.c:344,395-402`: Render version and format queries on every scale. *(status: done; no change needed: libXrender caches version and formats per display (3 requests on first call, 0 after))*
- [x] **[verified] P3** `WmWinState.c:809-1095`: the subpanel slide calls `XSync` at every step, and its duration depends on height. *(status: done; time-based 75 ms slide, no XSync; new slideSubpanels resource)*
  - Fix: a fixed duration, no sync, and a resource to turn it off.
- [ ] **[verified] P3** `Callback.c:301,313,334,390,443`: `XSync` on panel button presses. *(status: won't do: each sync orders an ungrab on DISPLAY1 before a grab on DISPLAY (or the raise's Exposes before a repaint), which no request on the other connection can do; one ~90 us round trip per click)*
- [x] **[verified] P3** `UI.c:2016-2045`: workspace titles are read from the server at panel startup although they are in `pSD->pWS[i].title`. *(status: done; titles taken from pSD->pWS[i].title)*
- [x] **[verified] P3** `WmFunction.c:3000-3026`: `f.refresh` does recursive `XQueryTree` + `XClearArea` over every window. *(status: done; refreshByClearing default kept; RefreshByClearing sends all QueryTree requests of a tree level through XCB before reading replies (one round trip per level), still clearing every window; 3020 windows 240-714 -> 43-47 ms; dtwm links -lX11-xcb -lxcb (e623ee6e1))*
  - Fix: use the cover-window path by default.
- [x] **[verified] P3** `WmMenu.c:1592`: 2 × `XSync` per menu unpost. *(status: done; no round trip on unpost; the two syncs only before XGrabServer)*
- [x] **[verified] P2** `WmSignal.c:231-240`: the signal handler calls Xlib and Xt directly. Port `c84ebfaa` (`XtNoticeSignal`). *(status: done; XtNoticeSignal; 20/20 deadlocks -> 0/20)*
- [ ] **[verified] P3** `WmIPlace.c:342-361`, `WmIconBox.c:1969-2635`: icon-box inserts are O(N²). *(status: won't do in this tree: perf with 800 icons: the O(N^2) is in libXm (XmeNavigChangeManaged -> XmGetVisibility per insert, BulletinBoard layout); dtwm's placement is 6.3%. Fix belongs to /home/dimmus/motif (skip the in-view check of an unchanged focus item; lay out only new children); deferring XtManageChild in dtwm would change focus traversal timing)*
- [x] **[verified] P3** `WmResParse.c:5477-5497,5948-5975`: when `cppCommand` is set, `system(cpp)`, `tmpnam` and `system("/bin/rm")` are used. Port `ece1a79f`. *(status: done; mkstemp, file names passed as $1/$2 through execl, unlink instead of system(rm))*
- [x] **P2** Build dtwm, and all of CDE, against the modernised libXm (`/home/dimmus/motif`). Its Phase 3 removed Xft color round trips on every Label/Text/title draw, `_XmIsISO10646` round trips per segment, and spot-location IM round trips. That is free speed for every CDE client. *(status: done (already the case): configure uses pkg-config motif 2.5.0; dtwm, dtfile, dtterm, dtsession and libDtWidget resolve /usr/lib/libXm.so.5 (XM_2.5), whose headers match /home/dimmus/motif/build; perf shows code only the modernised Traversal.c has)*

---

## Phase 5 — Applications

### 5.1 dtfile

N = entries in the directory, K = files in one operation, T = data types.

- [x] **[verified] P1 (A1)** `Directory.c:4991`, `:331-337`: every directory activity (read, update, link check, position write) forks the whole X/Motif/ToolTalk process. Only the directory check is persistent. *(status: done; reads, update_all and update_some share a pool of sticky workers (at most 2 idle) with framed bulk requests (1 MB pipe via F_SETPIPE_SZ); dead workers replaced, retired on database reload; 5 folders and back plus 3 changes and New Folder: 1 fork instead of one per activity (each fork blocked the main loop 0.5-1.5 ms at 3000 icons); screenshots identical)*
  - Fix: one persistent worker (a `posix_spawn`ed helper or a thread) with a framed, buffered pipe.
- [x] **[verified] P1 (A2)** `Directory.c:1016-1308` `ReadFileData2`, per entry: *(status: done; ReadFileData2At types non-link entries through _DtDtsDataToDataTypeAt (dir fd + lstat buffer, openat for content and .DtDirDataType), exported from libDtSvc (326313a6a), with a weak reference and fallback to DtDtsDataToDataType; 533 files opened with openat(dirfd) instead of full paths)*
  - `lstat`, plus `stat` for links;
  - `DtDtsDataToDataType` without the lstat buffer, so it may do an extra `readlink`;
  - content mmap;
  - `.DtDirDataType` open;
  - 2 × `access`;
  - `DtActionExists`;
  - a LABEL lookup.
  - Fix: pass the lstat result; `fstatat(dirfd)` and `d_type`; a per-type label and is-action cache; `faccessat(AT_EACCESS)`.
- [x] **[verified] P2 (A3)** `Directory.c:1388-1458,1757-1830`: path components are retyped and `GetTTPath`'d on every read and update, followed by two `XSync`s (`:2208-2212`). *(status: done; workers cache each path component's type (valid while its lstat, link target stat and .DtDirDataType stat are unchanged; 512 entries) and each folder's ToolTalk path; 13-deep folder 94-99 -> 74-84 syscalls before readdir, access() 26 -> 5)*
- [x] **[verified] P2 (A4)** `FileOp.c:259-331`, `Directory.c:635-647,1676-1683,1991-2110`: the pipe protocol writes field by field with 2 × `signal()` per string, one ~8 KB `FileData2` per file, and one message per Xt dispatch. *(status: done; one write() per message, FILEDATA3 batches of 50, the main process drains the pipe for up to 20 ms per callback, no signal() calls)*
  - Fix: batch everything with `writev`; ignore SIGPIPE once; drain the buffer per callback.
- [x] **[verified] P2 (A5)** `Directory.c:2257-2286`: appending to the list walks it from the head (O(N²)). Keep a tail pointer. *(status: done; tail pointer)*
- [x] **[verified] P2 (A6)** `Directory.c:1932-1960`: update_all re-`lstat`s everything with an `XtMalloc` per entry and does O(N·K) list scans. *(status: done; hash indexes, fstatat; symlink bug fixed (603 -> 2 records per refresh in the test))*
  - **Bug:** symlinks always compare as modified, so they are retyped on every refresh.
- [x] **[verified] P3 (A7/A8/A9)**: *(status: done; A7 hash merge, A9 faccessat(AT_EACCESS); A8 kept on purpose (it makes newly added icons visible and costs almost nothing))*
  - An O(N·M) old/new merge (`:2409-2430`).
  - `_DtFlushIconFileCache` after every read (`:2520`; the comment says nobody knows why).
  - `CheckAccess` costs 7 syscalls (`FileManip.c:112-157`); use `faccessat`.
- [x] **[verified] P1 (B3)** `Directory.c:3177,3288-3296`: `SMART_DIR_UPDATE` is never defined, so any change → full rescan → full re-sort, re-filter and re-gadget of all N entries. *(status: done; update_some (SMART_DIR_UPDATE) on with inotify event names, used only while every change since the last full read is named (cleared on overflow, lost watch, nameless event, too many names, first refresh after a new watch); 3000-entry folder with one change: update process 6.6 -> 1.1 ms; screenshots identical)*
  - Fix: incremental view updates.
- [x] **[verified] P3 (B4)** `Directory.c:3331-3432`: no cache of past directories, so Back/Up always forks and retypes. *(status: done; LRU of 8 directories / 20,000 entries; going back to a 10k directory costs no fork and no read)*
  - Fix: a small LRU.
- [x] **[verified] P1 (C1)** `File.c:5986,6066-6071`: every refresh unmanages all gadgets and rebuilds each one: an XmString, 3 attribute lookups plus `XmGetIconFileName`, a ~13-argument `XtSetValues`, callback re-adds, and `SetHotRects` with 3 more lookups plus a drop-site update. *(status: done; gadgets diffed; 3000 files: CPU 23-30 -> 3-5 ticks, X requests 15400 -> 6834 per refresh; pixel-identical in 7 view modes)*
  - Fix: diff each `FileViewData`; leave unchanged gadgets alone.
- [x] **[verified] P1 (C2)** `File.c:5712,5758` together with `Icon.c:2097,2222-2243`: one `XGetGeometry` round trip per icon per refresh, because image-name pointers are compared (see 3.4 W1). *(status: done; 500 -> 0 XGetGeometry per refresh; first display of 3000 files 750-1060 -> 160-290 ms)*
- [x] **[verified] P1 (C3)** `File.c:7257-7258`: the display work proc handles **one icon per main-loop iteration** and `XtMalloc`s an `order_count`-sized array each time (`:6697`), which is O(N²) bytes in total. *(status: done; batches of 8 icons within 8 ms; array reused; ToBeManaged is a hash lookup)*
  - Fix: a time budget (about 8 ms) and reuse of the array.
- [x] **[verified] P2 (C4)** `File.c:7181,7308`: managing in batches of 100 into an `XmDrawingArea` with `XmRESIZE_GROW` relays out all children per batch, i.e. O(N²/100). *(status: done; one XtManageChildren for the off-screen icons; XmRESIZE_GROW kept on purpose)*
  - Fix: manage everything in one call; use `XmRESIZE_NONE`.
- [ ] **[verified] P2 (C5)** One gadget per file, and every icon is a drop site (`File.c:4141-4228`). DrawingArea redisplays every gadget on every expose. *(status: won't do (rest): non-droppable icons get a plain Motif XmDROP_NOOP site instead of a DtDnd registration (10,000 data files: drop-site work 95-122 -> 22-45 ms, same feedback and refusals). No site at all would let drops fall through to the folder; a virtualized icon view means rewriting File.c's selection, traversal, DnD and rename)*
  - Fix: register drop sites only for droppable types. Longer term, a virtualized icon view.
- [x] **[verified] P2 (C6)** `FileMgr.c:4027-4136`: widget-reuse matching is O(N_old·N_new), about 10⁸ compares for 10k files. *(status: done; hash map keyed on the FileData pointer)*
  - Fix: a back-pointer.
- [ ] **[verified] P2 (C7)** `File.c:484-602,863`: about 5 `strcoll` calls per sort comparison. The type sub-sort compares *pointers* to per-file string copies, so every group has size 1. *(status: won't do (rest): strxfrm keys differ in sign from strcoll for 22 of 2.25M glibc en_US.UTF-8 pairs (icon order would change) and cost as much as the sort; type comparisons cost at most ~2 ms per 10k sort. Round 1 fixed the type grouping bug and cut strcoll to 1 per comparison)*
  - Fix: precomputed flags and `strxfrm` keys; interned type ids.
- [x] **[verified] P2 (C8/C9)** `File.c:951,1085-1105`: filtering is O(N·T). In total there are about 10 DtDts lookups per file per display. *(status: done; _DtGetTypeInfo per-type cache flushed in ReloadDatabases; filter entries hashed)*
  - Fix: one per-type attribute cache cleared in `ReloadDatabases` (`Main.c:4940-4950`).
- [x] **[verified] P2 (C10)** `IconWindow.c:191-222`, `File.c:7434,7777`: every ConfigureNotify relays out all N icons and calls `XSync`. *(status: done; relayout from a work proc, at most every 40 ms)*
  - Fix: coalesce resizes.
- [ ] **[verified] P2 (C11)** `File.c:1196-1208,6512-6620`: tree view walks all rows on every Expose with an ancestor linear search, and expanding a branch rebuilds the whole tree. *(status: won't do (rest): Expose fixed in round 1; expand/collapse of a 3000-row tree costs 1.4-3.9 ms CPU and already reads and adds only the branch; the first read of a new branch triggers a full redisplay (12.6 ms), and a branch-only GetFileData would duplicate widget reuse, selection, rename and drag remapping at high risk of dangling pointers)*
- [x] **[verified] P2 (C12)** `File.c:3752-3790,4097-4105`: "as placed" mode does O(children·objects) ordering per commit and creates 2 Regions per object per expose. *(status: done; XRectInRegion, PtrMap ordering)*
- [x] **[verified] P3 (C13/C14/C15)**: *(status: done; C13 one allocation and memmove, C14 stack buffer + uid/gid caches + localtime_r (also fixes a 20-byte name overflow), C15 already fixed)*
  - Selection reallocs +1 and prepends (`File.c:2707-2969`).
  - `GetLongName` allocates 3·MAX_PATH per file, has a 1-entry uid cache, and calls `localtime` per file (`Directory.c:3476-3602`).
  - `WidgetCmp` truncates a pointer difference to int (`File.c:5349`).
- [x] **[verified] P1 (D1)** `dtcopy/fsrtns.c:91-153`, `main_dtcopy.c:505-640`: `dtfile_copy` uses 2 KB blocks and calls `XSync` after each, plus `XmUpdateDisplay` per file. That is about 512k round trips per GB. *(status: done; copy_file_range with read/write fallback, events checked at most every 50 ms without XSync; 5100 small files 1.1-4.5 s -> 0.11 s, 100 MB 1.47 -> 0.08 s)*
  - Fix: `copy_file_range`/`FICLONE` with a buffer of 1 MB or more; check events at most every 50 ms without syncing.
- [x] **[verified] P1 (D2)** `FileManip.c:1076-1189`, `MkDir.c:92-160`: copies use 1 KB blocks, moves are `link`+`unlink` instead of `rename`, and directory moves fork `/bin/mv` and `wait()`. *(status: done; rename() for moves, fsCopyData for copies, rename() for directory moves (/bin/mv only for EXDEV), posix_spawn for dtfile_copy, waitpid in MkDir.c)*
- [x] **[verified] P2 (D3)** `FileOp.c:155,2160-2290`, `Directory.c:3219-3227`: the UI blocks in `select()` for up to 2 s, and deduplication is O(K²) (about 10⁸ `strcmp` for 10k dropped files). *(status: done; no 2 s select() wait; hashed dedup; collision dialog 2.63-2.73 -> 0.65-0.69 s)*
- [x] **[verified] P2 (D4)** `Trash.c:636-735,2276,3089,3621-3969,4211-4270`: *(status: done; persistent name/path hash indexes with holes and lazy compaction (O(K) per restore/shred), children write .trashinfo themselves (kept if a generation counter is unchanged), startup checks entries against one readdir instead of one lstat each; 100k entries main-loop CPU per restore 87 -> 5.2 ms, startup with 10k entries 13,813 -> 3,830 syscalls (6d672d480, e22de4224). The permission walk before the rename is kept on purpose (it decides the error shown first))*
  - the trash list grows by 10;
  - the whole `.trashinfo` file is rewritten after every operation;
  - restore and remove are O(K·N);
  - move-to-trash walks the whole subtree for permissions before what is just a rename.
- [x] **[verified] P2 (D5)** `Find.c:1916-2337,2864-2903`: Find forks `ksh -c find` and reads with a 1-byte `fread` per callback; results stall in the stdio buffer. *(status: done; posix_spawnp of find/grep, 16 KB non-blocking reads, batched list adds, cached typing; 10,100 matches 0.52-0.55 -> 0.20-0.42 s)*
  - Per result it does a `stat` and an uncached typing call (and leaks the type), then a single `XmListAddItemUnselected`.
  - Content search builds its buffer with `strcat` (O(N²)) and runs `sh -c grep` per directory.
- [x] **[verified] P3 (D6/D7)**: *(status: done; separate shells kept on purpose (per-object workspace occupancy, stacking, SHAPE). The real cost was XmAddToPostFromList re-grabbing the popup accelerator on every shell (337,600 XGrabKey for 200 objects) and a per-object wait for the WM: 200 objects CPU 1.9 -> 0.17 s, load 2.6-3.0 -> 0.64 s, RSS 29 -> 18 MB (2000fd5f2). Typing stays synchronous on purpose (3.8% of load CPU; async would flash placeholder icons))*
  - Each desktop object is a top-level shell that re-parses its translations (`Desktop.c:280-327`).
  - Desktop objects are typed synchronously at load (`:488-499`).
  - Icon position saves fork (`Directory.c:3894-3955`).

### 5.2 dtterm (lib/DtTerm)
- [x] **[verified] P1 (T1)** `TermPrim.c:2870-3006`: one `read()` of at most about 4 KB per Xt dispatch, plus a zero-timeout `select()` (`moreInput`, `:2850-2866`). *(status: done; 64 KB buffer, read until EAGAIN or 8 ms; select 1000 -> 56 per 4 MB)*
  - Fix: `O_NONBLOCK`; read in a loop into a 64 KB buffer until `EAGAIN` or a 5–8 ms budget runs out.
- [x] **[verified] P1 (T2)** `TermPrimScroll.c:129-139,247-252,509-523`: jump scroll repaints the whole screen every `rows` lines with no frame cap. 100 MB of output is about 54k full repaints and 2.6M requests. *(status: done; frame pacing at 16 ms; 40 MB base64 at 80x24: 6.6-6.9 s -> 1.6-1.9 s (C))*
  - Fix: accumulate damage; render at most every 16 ms or on `EAGAIN`.
- [x] **[verified] P1 (T3)** `TermPrimRender.c:1182-1188,1279-1285,1430-1452`, `TermPrimRenderMb.c:771-899`: every text run is drawn as soon as it is inserted. *(status: done; per-row dirty spans painted once per frame; '\r progress' output: 601k -> 36 draw requests)*
  - Fix: per-row dirty spans; paint once per frame.
- [x] **[verified] P1 (T7)** `TermPrimRender.c:1054-1109,1360-1380`: in UTF-8, every byte goes through `mblen` → `mbtowc` → `mblen` again, with a malloc per run, an O(n²) `memmove` per invalid byte, and a copy of the whole read when a sequence straddles the boundary. *(status: done; inline UTF-8 decoder with ASCII fast path (0 mismatches vs mbrtowc on 143M sequences); 40 MB UTF-8 user CPU 5.3-5.5 -> 2.25-2.28 s)*
  - Fix: an inline UTF-8 DFA with an ASCII fast path and a reusable buffer.
- [x] **[verified] P1 (T11)** `TermPrimRender.c:731-936`, `TermPrimBuffer.c:1768-1770`: every scrolled line shifts all H history pointers, `XtMalloc(BUFSIZ)`s, and copies characters and enhancements one at a time through function pointers. *(status: done; int row types throughout the buffer and tpd (saveLines clipped to 2^23; -sl 100000 keeps every line, was a crash at 40000), line pointers in a ring so scrolling a full buffer writes no pointer, prefetch of the line to recycle, no 64-bit division without a selection; yes 16 MB C.UTF-8 CPU 3881 -> 2770 ms (709e23ead, 9513b1e83))*
  - Fix: one ring of `TermLine*`; widen the `short` row types.
- [x] **[verified] P1 (T12)** `TermPrimScroll.c:37-98,201-294,875,893`: every `XCopyArea` scroll blocks on GraphicsExpose/NoExpose, i.e. one round trip per scroll in less/vim, per scrollbar event, and per autoscroll tick. *(status: done; GraphicsExpose/NoExpose handled asynchronously in jump mode; 977 -> 0 blocking waits per 1000 lines; smooth scroll still waits by design)*
  - Fix: handle GraphicsExpose asynchronously, use `graphics_exposures=False` while unobscured, or use a back buffer.
- [x] **[verified] P2 (T4/T5/T6)**: *(status: done; per-widget buffers; pending chunks up to 64 KB parsed in place; paste written up to 4 KB per callback (4 MB paste 0.51-0.59 s corrupted -> 0.21-0.35 s identical))*
  - 8 KB or 32 KB malloc per inserted run (`TermPrimRender.c:960`, `TermPrimRenderMb.c:617`).
  - Pending text is processed 1 KB per main-loop pass (`TermPrimPendingTextP.h:41`).
  - Paste is written 128 bytes per callback (`:69`).
- [x] **[verified] P2 (T8)** `TermPrimRenderMb.c:228-322,715`, `TermPrimBuffer.c:900-950`: `wcwidth` is recomputed everywhere and column lookup is O(cols). *(status: done; CELL_WIDTH inlined for printable ASCII (cat base64 C.UTF-8 2041 -> 1214 ms); multibyte line buffers are xterm-style cell arrays with continuation cells, O(1) column lookup and widths computed once on insert; CJK cursor-addressed edits 1160 -> 777 ms; harness over 32 streams identical to the old code; fixed a double-click crash on empty lines in UTF-8 (eaa1f6eab, bdc888208, b4fabcebd))*
  - Fix: an xterm-style cell array.
- [x] **[verified] P2 (T9)** `TermPrimRenderFontSet.c:78-187`: `XmbTextExtents` on every run plus a second conversion for drawing, and `renderGC.fid = 0` defeats the GC cache. *(status: done; fixExtents decided from font-set metrics at create time (53 -> 7 extents calls); the GC reset costs no requests (Xlib GC cache). Xft path not attempted)*
  - Fix: decide `fixExtents` once at create time. Longer term, an Xft/XRender path.
- [x] **[verified] P2 (T15)** `TermPrimLineDraw.c:567-641`: line-drawing characters cost 2 requests per glyph. *(status: done; batched XDrawSegments/XFillRectangles; 3 MB of box chars 0.19 -> 0.12 s)*
  - Fix: batch per run.
- [x] **[verified] P2 (T16)** `TermPrimRender.c:108-145`: the visual bell does 2 × `XSync` per BEL, with no rate limit. *(status: done; 200 ms suppress window, flash undone by a timer; 2066 -> 0 XSync per 1000 BELs)*
  - Fix: a suppress window of about 200 ms; undo the flash from a timer.
- [x] **[verified] P2 (T19)** `TermPrimCursor.c:248-346,507-511`: per output burst the cursor code sets the IM spot location (a synchronous XIM round trip with ibus or fcitx), updates the scrollbar, and re-arms the blink timer. *(status: done; IM spot coalesced (first move immediate, then 50 ms), scrollbar and status once per frame (T14), blink timer not re-armed per burst; not tested with a real XIM server)*
  - Fix: coalesce after about 50 ms idle.
- [x] **[verified] P2 (T21)** `TermView/TermViewMenu.c:405-580`: 7 `XListFonts` calls at startup; all menus are built eagerly. *(status: done; Options menu built on first cascade; startup XListFonts 7 -> 0)*
  - Fix: build them lazily.
- [x] **[verified] P3 (T10/T13/T14/T17/T18/T20/T22)**: *(status: done; T13, T14, T17, T18, T20, T22 done (bold font lazy: UTF-8 startup round trips 772 -> 122); T10 left as is (no extra requests, needed for wide characters))*
  - Wide-char refresh repaints extra cells.
  - Refresh flags are shifted O(rows) per scroll.
  - The scrollbar is updated per burst.
  - Expose compression is off (`TermPrim.c:522`, `Term.c:424`).
  - The parser does linear table scans plus a locked debug check per byte (`TermPrimParser.c:62-220`).
  - Selection disown makes a blocking round trip for a timestamp (`TermPrimSelect.c:79-111,898`).
  - The bold font is loaded eagerly (`TermPrim.c:1087-1240`).

### 5.3 dtmail
- [x] **[verified] P1** `MsgScrollingList.C:461-468`, `RFCMailBox.C:988,1022,1050`, `DtVirtArray.C:91-101`: opening a mailbox does two linear `indexof` scans per message, i.e. O(N²), about 2.5·10⁹ compares at 50k messages. *(status: done; slot stored in the handle with a validated live set; 50k open: list 1.43 -> 0.36 s, close 0.91 -> 0.06 s)*
  - Fix: store the slot index in the handle.
- [x] **[verified] P1** `MsgScrollingList.C:480,611,1164-1196`, `RFCMessage.C:283-293,1434-1488`: building the list parses the MIME bodies of every multipart message, which defeats lazy parsing. *(status: done; multipart glyph from a header-and-boundary scan, full parse only for alternative/V3/single-part; 0 glyph mismatches)*
  - Fix: decide the attachment glyph from headers only.
- [ ] **[verified] P1** `RFCMessage.C:999`, `RFCEnvelope.C:403-447`: a synthetic `Content-Length` header marks every message dirty, so the whole mbox is rewritten (with `fsync`) on autosave. Any flag change also rewrites the whole file. *(status: partial: no rewrite after open (round 1); a save still writes a temp file and renames it, but the leading unchanged messages are cloned with FICLONERANGE, so on btrfs/XFS/bcachefs only the tail is really written; byte-identical output. ext4/tmpfs/NFS refuse the ioctl and write everything on purpose (an in-place tail rewrite gives up the crash guarantee). Success path untimed: no CoW filesystem here)*
  - Fix: don't persist the synthetic header; rewrite only from the first dirty message.
- [x] **[verified] P2** `RFCMessage.C:771-786,965-975`: a `strncmp` per body byte to find message boundaries. *(status: done; memmem/memchr; 10k open 0.49-0.65 -> 0.07-0.14 s)*
  - Fix: `memchr`/`memmem`.
- [x] **[verified] P2** `RFCMailBox.C:2344-2375,3358-3367,3587`: incorporating new mail re-walks the whole mailbox, and removal is O(N) per message. *(status: done; partials tracked, one-pass removal; incorporating one message into 50k 36.7 -> 0.1 ms)*
- [x] **[verified] P2** `RFCEnvelope.C:288-310,684-745`: every header lookup is a double linear scan with a mutex per header, about 10 lookups per list row. *(status: done; 64-bit header-name presence filter per envelope, generation counter, cached New/DeletePending/Multipart/Partial flags, inline DtVirtArray accessors; 50k mbox summary pass 0.075 -> 0.058 s, flags + second pass 0.163 -> 0.113 s; headers, flags and written files identical)*
  - Fix: a header index; cache flag bits.
- [x] **[verified] P2** `MsgScrollingList.C:2330-2481`, `Sort.C:101,138,370`: sort, display-property changes and selection restore are all O(N²). *(status: done; insert_all/remove_all merges and hashed selection restore (50k insert 1.3 s -> 1 ms); the count bug was already fixed)*
  - **Bug:** `:2460` passes a count that includes deleted messages; `delete` should be `delete[]`.
- [x] **[verified] P2** `MsgScrollingList.C:764-796`, `Undelete.C:136-168`, `MsgHndArray.C:111-226`: delete is O(N·k), and `compact` recurses, about 50k levels deep for "select all, delete". *(status: done; non-recursive compact (50k: 0.73 s -> 0.2 ms); undelete all 10k 13.9 -> 0.23 s CPU)*
- [x] **[verified] P3**: *(status: done; madvise on Linux (MADV_NORMAL after parse), wordexp(WRDE_NOCMD), cheaper formatHeader; makeHeaderLine is void)*
  - `madvise` is compiled out on Linux (`RFCMailBox.C:555-566,2236-2334`).
  - `expandPath` uses `popen("echo …")` (`Session.C:570-600`); use `wordexp(WRDE_NOCMD)`.
  - Per-row mailrc lookups and XmString churn (`MsgScrollingList.C:2493-2770`).
  - **Bug:** `RFCMailBox::makeHeaderLine` has no `return` (`RFCMailBox.C:4418-4452`).

### 5.4 dtcm, lib/csa, rpc.cmsd
- [x] **[verified] P1** `lib/csa/iso8601.c:48-155`: every ISO-8601 → tick conversion does `putenv`+`tzset` to GMT and back, i.e. 2 zoneinfo loads per call, and it is not thread-safe. It is called once per entry per attribute, on both client and server. *(status: done; fixed-width parser, arithmetic UTC, no tzset; 14.6 us -> 23 ns; pointer compare fixed)*
  - Fix: a fixed-width parser plus `timegm`.
  - **Bug:** `:251` compares pointers, not values.
- [x] **[verified] P1** `dtcm/find.c:606-748`: Find "all" makes about 880 sequential 4-week RPCs, adds one `XmListAddItem` per hit, and matches with naive `strncasecmp`. *(status: done; server-side CSA_MATCH_CONTAIN in 52-week chunks (888 -> 69 RPCs), one XmListAddItems; rare match 3.4-5.9 s -> 0.018-0.05 s)*
  - Fix: one RPC with a `CSA_MATCH_CONTAIN` filter; `XmListAddItems`; `strcasestr`.
- [x] **[verified] P2** `server/lookup.c:228-233,461-558`: each query scans every repeating entry with no early exit, builds results by sorted insertion (O(K²)), and deep-copies each instance. *(status: done; O(K log R) merge (round 1); no deep copy (instances point at the calendar's own data, da6e2a67d); time zone checked once per wakeup (/etc/localtime stats 71,966 -> 12 per 12 month views); ISO 8601 without sprintf; libcsa moves decoded values. Month view server CPU 15.3 -> 5.0 ms, year 178 -> 51 ms, old Find 1.41 -> 0.38 s; dumps identical. Early exit measured at 0.3% of a lookup, not done)*
- [x] **[verified] P2** `server/callback.c:560-682`: `clnt_create` for every client on every change. *(status: done; CLIENT* cached per registration; also fixed callbacks never being delivered (zero timeout))*
  - Fix: cache the `CLIENT*`.
- [x] **[verified] P2** `dtcm/calendarA.c:3269-3284`, `editor.c:1103-1140`, `todo.c:2000`: any change triggers a full repaint and three RPCs, and the lists are refilled one item at a time. *(status: done; update_canvas compares a paint cache: unchanged entries paint nothing, year view needs no fetch, month view repaints only changed day boxes (pixel-identical to a full repaint); per change 142 -> 6 X requests (0 for another year), CPU 24.5 -> 17 ms (a753b0955). Editor and to-do refills unchanged on purpose (they clear the list selection))*
- [x] **[verified] P3**: *(status: done; GC dump uses one fsync instead of O_SYNC per entry; per-record O_SYNC append kept (one flush per record is required); O(1) tail insert (20k repeating entries 27-33 -> 0.8-1.2 s); GC out of the signal handler; one RPC per printed page; CmDataList cursor)*
  - The log is reopened `O_SYNC` per record (`server/log.c:136-1016`).
  - Loading repeating entries is O(R²) (`list.c:148-191`).
  - GC runs inside a SIGALRM handler with 10 s pings (`cmscalendar.c:617-645`, `svcmain.c:489-507`).
  - Printing makes one RPC per day or hour, twice (`monthglance.c:718-812`, `dayglance.c:806-818`, `weekglance.c:157-163`).
  - `CmDataListGetData` is O(position) (`libDtCmP/util.c:2159-2181`).

### 5.5 Help, dtinfo, DtSearch (runtime)
- [x] **[verified] P1 (B1)** `XInterface.c:2059-2150`, `Graphics.c:2364-2480`: there is no graphic cache; every topic view re-decodes every TIFF and XPM and re-allocates colours. *(status: done; GraphicCache.c, reference-counted LRU (64 graphics / 16 MB); 368 XPutImage -> 0 per repeated pass)*
  - Fix: a per-screen LRU keyed on (path, mtime, visual, colormap, fg/bg).
- [x] **[verified] P2 (B2)** `Graphics.c:845-1847,1984`, `GifUtils.c:551-716`: one `XAllocColor` round trip per colour; XWD DirectColor does a per-pixel linear search. *(status: done; _ilXComputeColor after 9 verified samples (0 mismatches on 80000 colours); XWD DirectColor hashed; help window first view 263 -> 16 XAllocColor)*
  - Fix: compute TrueColor pixels; an rgb→pixel cache.
- [x] **[verified] P2 (B3)** `UtilSDL.c:579-760`, `FormatUtil.c:271-280`: parsing calls `mblen` per byte and `_DtHelpCeAddCharToBuf` per character, growing the buffer by 32 bytes. *(status: done; ASCII fast path, inline appends, geometric growth; -29% dthelpprint samples with B4)*
  - Fix: an ASCII fast path, `memcpy` of runs, geometric growth.
- [x] **[verified] P3 (B4–B7)**: *(status: done; B5 parsed-topic cache (16 topics plus TOC per display area, validated on volume and graphic dev/ino/size/mtime, never for SDL-script topics) and the volume title kept: Filemgr revisit 2.3 -> 1.36 ms CPU, 770 requests 2.77 -> 1.99 s; B6 remaining growing appends use _DtCvAddPtrToArrayN and selection lookup is no longer O(L^2) (ksh(1) resize 104 -> 19.5 ms); 1,297 screenshots identical)*
  - ID lookups are linear (`AccessSDL.c:1156-1250`).
  - Topics are re-read and re-parsed on every visit (`Format.c:1027-1085`).
  - `_DtCvAddPtrToArray` walks to the end on every append (`CvString.c:352-390`).
  - TOSS style matching is linear (`UtilSDL.c:1511-1580`).
- [x] **[verified] P2 (C1)** `LayoutUtil.c:1373-1540`: line layout re-measures the rest of the run for every line (O(N·L)); each measurement is an `XmbTextEscapement` in UTF-8. *(status: done; segment length computed once, prefix measuring, ASCII fast paths; 20,000-word topic UTF-8 1.39 -> 0.084 s)*
- [x] **[verified] P2 (C2)** `Canvas.c:2167-2265,226-275`: each expose or scroll scans every line, and `CheckAround` makes it O(visible·N). *(status: done; y index with binary search for topics of 128+ lines; 400 scroll steps on 30,000 lines 2.6 -> 0.62 s)*
  - Fix: a y-sorted line index.
- [x] **[verified] P3 (C3–C7)**: *(status: done; C5: the O(cells^2) was the cell lookup by id, now a hash index in ResolveCell and ResolveAsyncBlock (1,500-item list 346 -> 27 ms); layout algorithm untouched, so column widths cannot change; output of all 19 volumes identical)*
  - Canvas arrays grow by +10 (`Layout.c:114`, `LayoutUtil.c:72`).
  - `FindChar` is O(n²) per mouse motion (`Canvas.c:297-362`).
  - Table re-layout is O(rows²) (`Layout.c:2403-2492`).
  - Fonts are resolved per chunk with no negative cache and no XLFD dedup (`XInterface.c:800-865`, `Font.c:366-460,1006-1100`).
  - `XAllocNamedColor` is repeated and leaked per redraw (`XInterface.c:1067-1112`).
- [x] **[verified] P2 (D1–D3)** `GlobSearch.c`: *(status: done; D3: incremental volume scan (_DtHelpFileListScanBegin/Step/End, same order) run one volume per search work proc call; longest stall in the first complete-index search 20-53 -> 6-16 ms; results identical)*
  - Index display is O(K²) (`:1129-1146,2775-2800`).
  - `regcomp` runs per index entry (`:2440-2444`).
  - The volume scan parses every volume synchronously and unloads it immediately (`:426-523,1691-1731`, `Access.c:1956-1968`).
- [ ] **[verified] P3 (E1)** `boolsrch.c:1123-1140`: proximity queries scan synchronously (the comment says "rewrite as its own workproc"). *(status: partial: collocation checks 32 records per workproc call; new DtSearchQueryStart/Continue/Stop in lib/DtSearch (0.7-1.5 s queries run in 99-195 slices of at most 23-46 ms, identical results); DtSearchQuery unchanged. Missing: declarations in include/Dt/Search.h and libDtSearch.sym exports, then a dtinfo caller; the CDE infolib DBs store no documents, so collocation is rejected on them anyway)*
- [x] **[verified] P3 (F1/F2)**: *(status: done; F2: read-only stores mmap'ed at first read (fstream fallback), mapping dropped on write/truncate/remove; also fixed an lru::remove throw at dtinfo exit; reading the whole infolib 3x: syscalls 21,321 -> 279 (CPU within noise, 55% is LZSS))*
  - dtinfo re-allocates named colours for every node view and never frees them (`NodeViewInfo.C:856-905`).
  - The DtMmdb page cache is 800 KB, with `seekg`+`read` per miss (`page_cache.h:31`, `unixf_storage.C:203-221`).
  - Fix: mmap the read-only store; raise the default cache.

### 5.6 Other applications
- [x] **[verified] P2 dtstyle** `Backdrop.c:687-825`: opening the dialog loads **every** backdrop into a server pixmap. The comment says "workprocs 10 at a time", but there is no work proc. *(status: done; images listed without loading and loaded on first draw; 47.8 -> 18.1 ms, 30 -> 9 XPutImage)*
  - Fix: load the selection only, or thumbnails.
- [x] **[verified] P2 dtstyle** `Backdrop.c:877-887,1012-1161`: after a workspace switch, the reset of `newColors` is commented out, so every expose or click reloads the full backdrop (plus 2 property round trips). *(status: done; colours fetched once per switch, pixmaps rebuilt only when fg/bg change; 94 -> 18 round trips)*
- [x] **[verified] P2 dtstyle** `ColorMain.c:734-857`, `ColorEdit.c:437-535,1002-1021`: on TrueColor, about 40 `XAllocColor` calls per palette click and 4 per slider drag event. *(status: done; StyleAllocColor computes TrueColor pixels after verification; 5 palette clicks 100 XAllocColor -> 0)*
  - Fix: compute pixels locally; coalesce drag updates.
- [x] **[verified] P3 dtstyle** `ColorMain.c:680,1468`: one `XmListAddItem` per palette (each XmString leaks), and `XSync` in `AddName`. *(status: done; batched XmListAddItems, XmStrings freed, XFlush in AddName)*
- [x] **[verified] P3 dtcalc** `motif.c:2429-2455`: `XSync` on every display update, i.e. per keypress. *(status: done; XFlush; 204 -> 104 round trips per 110 keys (the rest is Motif's))*
- [x] **[verified] P3 dticon** `utils.c:707-760`: every repaint does `XGetImage` of the whole icon plus 2 requests per magnified pixel. *(status: done; one magnified XImage and one XPutImage per repaint, grid with one XDrawSegments)*
  - Fix: a client-side XImage and one scaled `XPutImage`.
- [x] **[verified] P2 dtcreate** `icon_selection_dialog.c:281,681-850`: per-gadget manage, every icon loaded, a global `XmeFlushIconFileCache(NULL)`, and a `stat` per entry. *(status: done; icons load only when they scroll into view (placeholder of the right size from the XPM/XBM header); screenshots identical; opening 285 icons busy 1.0-1.2 -> 0.03 s, client CPU 0.55 -> 0.02 s (3c8d388f0))*
- [x] **[verified] P3 dtksh** `extra.c:70-76`, `dtkcmds.c:3559-6342`, `widget.c:430-540`: every callback or translation re-parses its command string; widget→wtab lookup is linear. *(status: done; ksh_eval keeps up to 512 parse trees, reused only while options, lexer state, locale, aliases, function/builtin tables, namespace and start line are unchanged (functions, here-docs, subshells, verbose/noexec bypass it); semantics script and 6000-step stress identical; 40000-step stress 16.3 -> 2.5 s (25cf0213c); fixed ${CB_CALL_DATA.EVENT.TYPE} (d2c7705e2))*
- [x] **[verified] P3 dtprintinfo** `libUI/MotifUI/MotifUI.C:121-140`: `XGetWindowAttributes` or `XQueryTree` per printer per tick for visibility. *(status: done; visibility tracked from Map/Unmap/VisibilityNotify; ticks skipped while hidden)*
  - Fix: track `VisibilityNotify`.

---

## Phase 6 — Build system and toolchain

### 6.1 Broken or misleading (P0)
- [x] **[verified]** `configure.ac:189-231`: the `--enable-*` options are inverted. The action-if-given branch runs for both `--enable-X` and `--disable-X`, so `--enable-docs` *disables* docs and `--disable-german` *enables* German. *(status: done; AC_ARG_ENABLE keeps $enableval; checked through config.status)*
  - Fix: `[enable_x=$enableval],[enable_x=default]` and test the value.
- [x] **[verified]** The help build needs `compress`, which nothing checks for: *(status: done; dthelp_htag2 compresses in-process since d907a737d; no build step needs compress)*
  - `dthelp_htag2` calls `system("compress -f …")` (`programs/dthelp/parser/pass2/htag2/sdl.c:1216-1227,1291`);
  - `dtdocbook2sdl` always passes `-otc` (`dtdocbook2sdl.in:425-428`);
  - `install-deps.sh` does not install `ncompress`;
  - the main checkout has 22 leftover `book.<pid>.log` failures.
  - Fix: in-process LZW (the decoder is already in `lib/DtHelp/decompress.c`), or `AC_PATH_PROG(COMPRESS)` plus the dependency.
- [x] **[verified]** `programs/ttsnoop/Makefile.am:158-163` overwrites the tracked `ttsnoop.c`. The `dtcodegen -merge` rules write tracked `*_stubs.c` files in the source directory. ksh93 drops about 20 untracked files into the tree. *(status: done; dtcodegen in a scratch dir (round 1); ksh93 built without `flat`, so everything stays in ksh93/arch/HOSTTYPE (ignored upstream); an old flat view is removed once; no file outside arch/ after a full build (3f0668d95))*
  - Fix: generate into the build directory.

### 6.2 Compiler flags
- [x] **[verified] P1** `configure.ac:462-463,622-625`: configure appends `-DOPT_TIRPC -I/usr/include/tirpc -std=c99 … -pthread` to the **user** variables `CFLAGS`/`CXXFLAGS`, so `make CFLAGS=…` (PGO, sanitizers, `-O0`) breaks the build. *(status: done; configure sets CDE_* and AM_* only; libtirpc via PKG_CHECK_MODULES; `make CFLAGS="-O0 -g"` works)*
  - Fix: `AM_CPPFLAGS`/`AM_CFLAGS` through a common include; `PKG_CHECK_MODULES([TIRPC],[libtirpc])`.
- [x] **[verified] P2** `configure.ac:26,74-75,624`: C is strict `-std=c99` with a pile of `_BSD_SOURCE`/`_SVID_SOURCE` defines. C++ has no `-std`, so it silently follows the compiler default (gnu++20 on GCC 16) for 1990s code. *(status: done; -std=gnu11 / -std=gnu++17; _BSD_SOURCE/_SVID_SOURCE/_POSIX_SOURCE dropped, _DEFAULT_SOURCE kept)*
  - Fix: `-std=gnu11`/`gnu17` and `-std=gnu++17`; drop the deprecated feature macros.
- [x] Warning baseline: `-Wall` for C and C++, with the whole tree warning-free (see Phase 7). The `-Wno-*` probes in `m4/compiler_flag_chk.m4` are no-ops, because GCC accepts any unknown `-Wno-*`; probe the positive form instead. *(status: done; m4/compiler_flag_chk.m4 now probes the positive form of -Wno-X (CDE_CHECK_CFLAG/CXXFLAG))*
- [ ] **[verified] P3** `configure.ac:354-355`: `-fno-strict-aliasing` is applied to the whole tree. *(status: won't do: the GifUtils.c violation is fixed (260bfa8d2), but -Wstrict-aliasing=2 reports 818 warnings at 529 places in ~150 files across 30+ directories (rpcgen XDR, Xt class records, ...), so the flag cannot be limited to DtSearch/il; strict aliasing changes 889 objects but shrinks .text only 0.13%, and benchmarks show noise. Numbers recorded in configure.ac (2f3ddab33))*
  - Fix: limit it to the code that needs it (DtSearch/raima, il) and enable `-Wstrict-aliasing=2` elsewhere.
- [x] **[verified] P2** There is no release/debug split and nothing defines `NDEBUG`, so about 380 `assert`s are live (dtinfo 142, dtmail 142, dtappbuilder 57, tt 40). `programs/dticon/constants.h:34` hard-codes `#define DEBUG True`. *(status: done; --enable-debug; release builds use -DNDEBUG; asserts audited; $(KEEP_ASSERTS) for code that relies on them. dticon constants.h DEBUG left alone)*
  - Fix: add `--enable-debug`; audit the asserts for side effects before setting `-DNDEBUG`.
- [x] **[verified] P2** No hardening flags: no `_FORTIFY_SOURCE` (no `__*_chk` imports), no BIND_NOW, only partial RELRO, including on the root daemons `dtlogin`, `dtspcd`, `rpc.ttdbserver` and `rpc.cmsd`. *(status: done; --enable-hardening (default on): _FORTIFY_SOURCE=3 (via cde-fortify.h when optimizing), stack protector, clash protection, relro/now; dtmail uses level 2, dthelpprint level 1 (see below))*
  - Fix: `--enable-hardening`, on by default: `-D_FORTIFY_SOURCE=3 -fstack-protector-strong -fstack-clash-protection -Wl,-z,relro,-z,now`.
- [x] **[verified] P3** configure.ac hygiene: *(status: done; all five fixed)*
  - `LT_INIT` is called twice (`:33,315`).
  - The arch conditionals compare literals (`test "is_mips" = yes`, `:156-159`).
  - `EXTRA_INCS` ends up in `XTOOLLIB` (`:528`).
  - The dead gettext m4 files dirty the tree on every `autoreconf`.
  - `xournal.dt` is listed twice (`types.am:20,23`).

### 6.3 Parallel and incremental builds
- [x] **[verified] P1** Recursive SUBDIRS serialise the tree: 277 `Makefile.am`, about 100 s for `lib/` of which `lib/DtMmdb` is about 50 s, and DtMmdb is independent of every other library. *(status: done; lib/DtMmdb, lib/tt/lib, dtinfo/src and dthelp/parser each one Makefile (Makefile.inc with %reldir%); lib/ runs its 11 libraries as parallel jobs; same objects, flags, exports and sizes; no-op make processes 21 -> 2, 11 -> 2, 31 -> 2, 40 -> 1; whole-tree no-op 3.5-3.8 -> 2.9-3.2 s; clean build noise-level on 2 CPUs)*
  - Fix: non-recursive `lib/DtMmdb` (19 SUBDIRS), `lib/tt/lib`, `programs/dtinfo/dtinfo/src` and `dthelp/parser/*` with `include …/Makefile.inc` and `%reldir%`; later, all of `lib/`.
- [x] **[verified] P1** `.NOTPARALLEL` serialises about 280 TUs: *(status: done; all 18 .NOTPARALLEL removed; grouped targets with a fallback for make < 4.3; ab -j3 41 s vs 57 s serial)*

  | Location | TUs |
  |---|---|
  | `dtappbuilder/src/ab/Makefile.am:3` | 101 |
  | `dthelp/parser/**` (19 files) | about 220 |
  | `ttsnoop` | — |

  - The root cause is multi-output generator rules written as `a b c: deps`.
  - Fix: grouped targets (`&:`, GNU make 4.3 or later) or stamp files.
- [x] **[verified] P1** `programs/dtksh/Makefile.am:61-67`: ksh93 builds serially outside the jobserver, with its own `cc -Os` (132 s measured, about 572 objects, no ccache). `ksh93/bin/ksh:` has no prerequisites, and `$(KSH93SLIBSHELL)` is a typo for `KSH93LIBSHELL`. *(status: done; ksh93.stamp with source deps, -jN from MAKEFLAGS, CC=$(CC), CCFLAGS with hardening; typo fixed; full ksh93 build 217 -> 182 s. System libshell option not done)*
  - Fix: `package flat make -j$(NPROC) CC='$(CC)' CCFLAGS='$(CFLAGS) …'` with a `+` recipe prefix, a source stamp, and the typo fixed. Optionally use the system ksh93's libshell.
- [x] **[verified] P2** `configure.ac:982`: `include/Dt/Dt.h` is an `AC_CONFIG_FILES` output with a fresh mtime every time, so every reconfigure rebuilds about 120 objects. *(status: done; move-if-change through AC_CONFIG_COMMANDS + BUILT_SOURCES; reconfigure rebuilds 0 tree objects (was ~120))*
  - Fix: move-if-change, or put the content in `AC_CONFIG_HEADERS`.
- [x] **[verified] P2** 178 recipes end in `|| $(RM) $@` (exit 0 with a missing file), and 110 `$(GENCPP) … > $@` leave truncated targets. *(status: done; .DELETE_ON_ERROR in 40 Makefile.am + sdl-docs.am; the 178 recipes write $@.tmp and mv)*
  - Fix: `.DELETE_ON_ERROR:` and `$@.tmp` + `mv`.
- [x] **[verified] P2** Generated files with no prerequisites never rebuild: *(status: done; VPATH builds (also read-only srcdir and --enable-doc-tools) and `make distcheck` pass: $(srcdir) -I flags and recipes, generated headers found in builddir, nothing written to srcdir, sdl-data.am uses $(wildcard) (178 -> 1 processes per dry run), make dist in any configuration, root-free install/uninstall; fixed a ksh93 mamake -jN hang; .github/workflows/distcheck.yml)*
  - `dtinfo/src/Makefile.am:34,48`
  - `doc/common/help/sdl-docs.am:21`
  - `doc/en_US.UTF-8/m-guides/Makefile.am:6`
  - Also `msg.am:4` uses `$(shell ls *.msg)`, which works only in-tree.
  - Fix: real prerequisites, `$(srcdir)`, and a `make distcheck` CI job (VPATH builds are not possible today).
- [x] **[verified] P3**: *(status: done; empty overrides removed, libraries moved to LDADD, ${X_LIB} removed, SUFFIXES target gone)*
  - Empty `distclean:`/`install:` overrides (`util/Makefile.am:5`, `dtdocbook/Makefile.am:16`, `doc_utils/Makefile.am:72,74`).
  - `SUFFIXES:` is written as a target in `tttypes/Makefile.am:7`.
  - Libraries are passed in `LDFLAGS` (`dtdocbook/Makefile.am:10`, `doc_utils/Makefile.am:22,31`).
  - `${X_LIB}` is undefined (`ab/Makefile.am:49`).
- [x] **[verified] P2** The 9 serial `tt_type_comp` runs in `programs/tttypes` each `popen` cpp and **autostart ttsession** when `DISPLAY` is set; that then pays the A1 sleep and dbserver attempts (`mp_typedb.C:1005,1060-1108`, `mp_c_session.C:193-279`). *(status: done; one tt_type_comp run over all 9 ptypes with DISPLAY/TT_SESSION unset (1.89 -> 0.14 s))*
  - Fix: `env -u DISPLAY -u TT_SESSION`, or one invocation for all files.

### 6.4 Docs and help toolchain
- [x] **[verified] P1** `htag2` runs `system("compress")` per topic page and for the volume structure: about 2,500 fork/execs per locale. See 6.1. *(status: done; in-process LZW since d907a737d)*
- [x] **[verified] P1** `doc/Makefile.am:3-23`, `doc/en_US.UTF-8/Makefile.am:3`, `guides/Makefile.am:35-37`, `dtdocbook2infolib.c:1324-1500`: locales and subdirectories are serial, and the infolib build is one sequential `system()` pipeline. *(status: done; doc/ parallel per directory (round 1); NCFGen/MixedGen cannot run concurrently (each needs the OIDs the previous dbdrv load stored) and cost 0.74 s of 11.5 s; instead NodeParser 3.7 -> 1.42 s CPU (O(1) SGMLName lookup, inert subtasks skipped) and dtdocbook2infolib pipes directly: en_US infolib 6.9-8.3 -> 6.2-6.6 s, identical outputs (3a8923882, ebab1ed98))*
  - Fix: a non-recursive `doc/`; run the independent NCFGen/MixedGen passes concurrently.
- [x] **[verified] P2** About 657 man pages each run a 9-process pipeline that re-parses the DocBook DTD (`dtdocbook2man.in:79-135`), and all of them depend on `ManLinks.sgm`. *(status: done; instant -B translates each child of the document element as its own document; dtdocbook2man -b runs one onsgmls+instant over all pages of a batch; doc/en_US.UTF-8/man rebuilds only changed pages via a stamp; 32.3 -> 2.5 s, 655 pages byte-identical apart from the date (d3180e469))*
  - Fix: batch them per section, or ship pre-generated pages.
- [x] **[verified] P2** `--disable-docs` still builds and requires `dthelp` (about 220 TUs), `dtdocbook`/`instant` with Tcl and `onsgmls`, `dtsr`, and 7 `dtdocbook/locales` directories that ignore the language switches. *(status: done; --enable-doc-tools gates dthelp/parser, dtdocbook and dtsr; --disable-help-docs/--disable-man-pages/--disable-infolib; dtdocbook locales follow the language switches)*
  - Fix: gate them on `BUILD_DOCS`; split the switch into help/man/infolib.
- [x] **[verified] P2** `programs/dthelp/parser/{pass1,canon1,pass2}`: 52 util files are identical except for RCS ids. *(status: done; programs/dthelp/parser/util/libhelputil.a; 44 fewer compiles)*
  - Fix: one shared `libhelputil.a`.
- [x] **[verified] P2** `dthelp_htag*`, `ctag1` and `instant` are text tools that link the whole Dt+Motif stack (22 of 22 direct libraries unused, 48 DSOs per run). *(status: done; htag1/ctag1/htag2 need only libc, instant only libtcl and libc)*
  - Fix: link only libc and `-ltcl`, so they build early and can be cross-built.
- [ ] **[verified] P3**: *(status: won't do (rest): instant, GraphicsTask and validator done in round 1, NodeParser and the TOC pass faster (6.4 P1). dtsrindex+dtsrload are 0.79 s of a 6.5 s infolib build and live in programs/dtsr and lib/DtSearch; folding the TOC pass into the book build means restructuring dtdocbook2infolib for at most 2.8 s CPU per language)*
  - `instant` calls `Tcl_GetEncoding` per element and leaks it, and runs `Tcl_Eval` on fresh strings (`translate.c:324-331`, `util.c:742-815`).
  - `GraphicsTask` reads with `getc` per byte (`GraphicsTask.C:220-226`).
  - `dtsrload` runs with an 8-page cache, and `dtsrindex` does seek/read/seek/write per word (`dtsrload.c:916`, `dio.c:203,981-1018`, `dtsrindex.c:822-958`).
  - Each guide is parsed twice (TOC plus build).
  - `doc_utils` compiles `validator` a second time.

### 6.5 Libraries, linking, startup cost
- [x] **[verified] P1** `configure.ac:33`: static and shared libraries are both built, so all 751 `.lo` are compiled twice (about 21% of compiles) and 61 MB of `.a` files are installed that nothing uses. *(status: done; LT_INIT([disable-static]))*
  - Fix: `LT_INIT([disable-static])`.
- [x] **[verified] P1** The libraries are underlinked. Unresolved symbols per library (no matching `NEEDED`): *(status: done; real _LIBADD with -no-undefined -Wl,-z,defs; unresolved symbols 0 in every library)*

  | Library | Unresolved |
  |---|---|
  | libDtWidget | 265 |
  | libDtHelp | 224 |
  | libDtTerm | 216 |
  | libDtSvc | 211 |
  | libDtPrint | 113 |
  | libDtMmdb | 106 |

  - libDtMmdb lacks lmdb and libstdc++.
  - Fix: a real `_LIBADD` for each library, `-no-undefined -Wl,-z,defs`, then `-Wl,-O1,--as-needed,--sort-common` by default.
- [x] **[verified] P1** `DTCLIENTLIBS` (`configure.ac:176-177`, used by 38 Makefiles) links everything into everything. Direct libraries the binary does not use (`ldd -u -r`): *(status: done; DTCLIENTLIBS removed; per-program LDADD via build-aux/cde-libs.am; ldd -u -r reports 0 unused libraries; dtcalc NEEDED 23 -> 7, dtexec 23 -> 3)*

  | Binary | Unused direct libs | Notes |
  |---|---|---|
  | `dtcalc` | 16 of 23 | — |
  | `dtexec` | 20 of 22 | runs on every action |
  | `dtspcd` | 20 of 22 | — |
  | `rpc.cmsd` | 21 | — |

  - `dtcalc` loads 48 DSOs and does 11,047 relocations, which costs about 3–10 ms of loader time.
  - Fix: per-program `LDADD`.
- [x] **[verified] P1** `configure.ac:451,454`: `-lcrypt -lm` go into the global `LIBS`, so every library and program gets them. *(status: done; MATHLIB/CRYPTLIB per target; LIBS is empty)*
  - Fix: `AC_SEARCH_LIBS` with an empty action and per-target `CRYPT_LIBS`/`MATH_LIBS`.
- [x] **[verified] P2** `lib/DtSvc/Makefile.am:176-179`: `SvcPam.c` and libpam are in libDtSvc, so every client loads libpam. Only dtlogin and dtsession use it. *(status: done; SvcPam.c dlopen()s libpam (RTLD_NOW|RTLD_GLOBAL, LIBPAM_SONAME then .so.0/.1/.6) on first use, PAM_SYSTEM_ERR if absent; libDtSvc NEEDED has no libpam; same 254 exports; a libDtSvc client maps 3 fewer DSOs; PAM auth unchanged (213f489d6))*
- [x] **[verified] P2** `lib/csa/Makefile.am:10`: libcsa links all of Motif and X for one `XtAppAddInput`. `ttsession` and `tt_type_comp` link `XTOOLLIB` and use none of it. *(status: done; libcsa NEEDED 14 -> 3; tt tools no longer link X (ttsession 17 -> 5 NEEDED))*
- [x] **[verified] P1** There is no export control: 10,161 exported symbols, of which only 1,404 are referenced in-tree. *(status: done; version scripts from build-aux/symbols/*.sym (no version node); libtt 3299 -> 1441, libDtMmdb 4030 -> 894 exports; -fvisibility-inlines-hidden/-Bsymbolic-functions not used)*

  | Library | Exported | Referenced in-tree |
  |---|---|---|
  | libDtMmdb | 4,028 | 154 |
  | libtt | 3,286 | 655 |

  - About 9,160 symbolic relocations and PLT calls.
  - Fix: `-export-symbols-regex` or version scripts; `-fvisibility-inlines-hidden`; `-fno-semantic-interposition`/`-Bsymbolic-functions`.
- [x] **[verified] P3** libDtSvc pulls in libstdc++ for 4 `new`/`delete` symbols from `DtCodelibs/*.C`; convert those files to C. *(status: done; all 12 DtCodelibs files are C (privbuf macros, structs with init/free, strtokx(char**) same ABI); 3 latent bugs fixed; libDtSvc linked by cc, no libstdc++/libgcc_s NEEDED, same 254 exports; outputs byte-identical over a shellscan/pattern harness (3e801cd0c). Processes still map libstdc++ through libtt)*
- [x] **[verified] P3** `programs/dtudcfonted` and `dtudcexch` are not built, but their app-defaults and catalogs are generated. `lib/pam` is not built. Decide whether to build or delete them. *(status: done; programs/dtudcfonted, programs/dtudcexch and lib/pam deleted (178 files, ~125,000 lines) with their app-defaults templates, catalogs in every locale (still generated and installed until now), .gitignore and debian/copyright lines; the reference-manual pages and the UDC guide chapter are kept (54494b0f5))*

### 6.6 Dead code and toolchain extras
- [ ] **[verified] P3** ToolTalk dead weight: *(status: won't do (rest): demo, mapfiles, trace grammar, OPT_CLASSING_ENGINE and bin/scripts removed in round 1. The TT 1.0 dbserver protocol is a live wire fallback (_Tt_db_client drops to dbVersion 1; the server answers old clients); ttdbck, tttar, ttcp/ttmv/ttrm and tttrace are documented commands (ttsnoop runs tttrace), and a switch would keep them in the default build anyway)*
  - `lib/tt/demo/` and Solaris mapfiles;
  - the never-compiled `tt_trace` grammar files and the duplicate `mp_types_gram.y`/`lex.l`;
  - `bin/scripts` (only used under the never-defined `OPT_CLASSING_ENGINE`) plus about 1,000 lines of `#ifdef`;
  - the TT 1.0 dbserver protocol (about 7,000 lines);
  - optional `dbck`/`tttar`/`ttcp`/`tttrace` tools (about 7,100 lines) that could go behind a switch.
- [x] **[verified] P3** `md5` is duplicated (`dtcm/dtcm/md5.c`, `dtmail/libDtMail/Common/md5.C`). C++ templates are expanded by textual inclusion (`-DEXPAND_TEMPLATES`, 24 headers), giving 1,283 weak exports in libDtMmdb. *(status: done; dtmail uses dtcm's md5.c/md5.h (b93a0826e, 86ae19386); UINT4 is uint32_t, so MD5 is now right on LP64 (APOP digests and Content-MD5 headers were wrong) (15a6a7bff). EXPAND_TEMPLATES: no change needed: libDtMmdb now exports 85 weak symbols, 4 of them template instantiations used by consumers, and the linker already merges COMDAT copies)*
  - Fix: explicit instantiation in one TU.
- [x] **[verified] P2** ccache breakers: *(status: done; no $(PWD)/$(abs_top_srcdir) in -D flags, ksh93 built without the CDE git hash, -ffile-prefix-map by default, ksh93 uses $(CC))*
  - `$(abs_top_srcdir)` and `$(PWD)` baked into defines (`doc_utils/Makefile.am:24-70`);
  - the ksh93 git-hash version header;
  - ksh93 ignores `$(CC)`.
  - Fix: `-ffile-prefix-map`.
- [x] **P2** `--enable-lto` (`-flto=auto -fno-semantic-interposition`, `AR=gcc-ar`) for libtt, libDtMmdb, libDtHelp and libDtSvc. PGO only after the Phase 0 benchmarks exist. *(status: done; --enable-lto (opt-in, whole tree); builds and runs, but adds 15 -Wodr/-Wlto-type-mismatch warnings outside the build system. PGO not attempted)*
- [x] **P3** Document or offer `-fuse-ld=mold` (about 100 links, many of them libtool relinks). *(status: done; --with-linker=NAME adds -fuse-ld=NAME after a link check)*

---

## Phase 7 — Warnings

- [x] Add `-Wall` to the C and C++ flags (`configure.ac`, `C_FLAG_CHECK`/`CXX_FLAG_CHECK`).
- [x] Fix every `-Wall` warning in the default build (`--disable-docs`).
  - The baseline build had 10,343 warning lines: 9,889 distinct diagnostics at 5,478 distinct sites, after collapsing the per-enum-value `-Wswitch` lines (4,748 of them) into 337 switch statements.
  - No pragmas and no blanket `-Wno-*` flags were used. Generated code was fixed at the generator: the rpcgen post-filter in `lib/csa/Makefile.am`, dtcodegen (`dtappbuilder/src/abmf`), dthelp's `build`/`eltdef` tools, flex `%option nounput`, and bison directives.
  - X11 `Xos_r.h` thread-safety buffers are kept with `(void) var; /* unused unless XTHREADS */`.
  - Still reported, all outside `-Wall`: *(status: all fixed: the dtinfo calls were replaced in 26fdb3afd, and dtksh compiles its init.c copy through its own ksh93-init.o rule without the extra warnings)*
    - Motif `-Wdeprecated-declarations`: dtmail 49 sites, dtinfo 9 (`XmListGetSelectedPos`, `Xm*GetChild`, `XmStringGetLtoR`). The replacements are not drop-in.
    - `-Wfree-nonheap-object` from `lib/DtMmdb/dti_cc/cc_hdict.C:12`, instantiated from `dtinfo/src/OnlineRender/FontCache.C:86`.
    - `-Wdiscarded-qualifiers` in `programs/dtksh/init.c`, a build-time copy of the vendored ksh93 source.
    - Linker "dangerous" notices for `tmpnam`/`tempnam`/`mktemp`: dtwm, dtlogin, dtspcd, dticon, dtcalc, dtcm, dtcreate, dtpdmd, dtpad, dtfile, dtfile_copy, dtmail, dtstyle, dtinfo, ttsnoop. Move them to `mkstemp`; that also closes the `/tmp` races.
- [x] Remove `-Wno-format-truncation` (part of `-Wall`) and fix what it reports. *(status: done; dropped from configure.ac; all 55 sites fixed; 0 format-truncation warnings with and without docs)*
- [x] Replace the deprecated Motif calls above. Then add a curated `-Wextra`, as in motif TODO 2.3, and `-Werror` in CI. *(status: done; deprecated calls replaced (dtmail round 1, dtinfo 26fdb3afd); configure enables a curated -Wextra set warning by warning (17 warnings, ~590 sites fixed, real bugs among them, 0cb3be2df); unused-parameter, sign-compare, cast-function-type and missing-field-initializers left out with the counts in configure.ac; --enable-werror; CI -Werror job in .github/workflows/build.yml (not yet run on GitHub))*

### 7.1 Bugs the `-Wall` cleanup exposed and fixed (behaviour changes)

| Location | Bug and fix |
|---|---|
| `lib/csa/match.c` | `defalut:` label typo; the function fell off the end and returned garbage. |
| `dtcm/server/cmsfunc.c` | Two use-after-free bugs: `free(appt)` before the reply in `cms_update_entry_5_svc`, and `free(log); unlink(log)`. |
| `dtcm/server/cmscalendar.c` `_DtCmsRbToCsaStat` | Missing return. |
| dtmail `DtMailServer`, `RFCFormat` | Destructors were not virtual, so POP3/APOP destructors never ran. |
| dtmail | `delete` changed to `delete[]`. |
| dtmail `RFCTransport.C` `concatValue` | 1-byte heap overflow. |
| dtmail `MenuBar.C` | Pointer `==` on strings meant per-menu help callbacks were never installed; now `strcmp`. |
| `dtprintinfo/UI/DtApp.C` | A local shadowed the `old_uid` member, so `setuid()` received garbage. |
| dtprintinfo | `delete[]` mismatches. |
| dtprintinfo `Icon.c` `QueryGeometry` | Missing return. |
| `lib/tt/bin/ttauth/process.c` | Uninitialised `status`. |
| `lib/DtSvc/DtUtil1/ActionTt.c` | Missing return. |
| `lib/DtSvc/DtUtil2/SvcPam.c` | Uninitialised `status`. |
| `dtwm/WmBackdrop.c` | Uninitialised `status`. |
| `dtpad/main.c` `HostCB` | Missing return. |
| dtfile `Desktop.c` and `Trash.c` | `sprintf(buf, "%s…", buf)`, which is undefined behaviour. |
| `dthelp/parser/*/parser/scan.c` | Out-of-bounds write. |
| `canon1/helptag/xref.c` | Writes after `fclose`. |
| `dthelpprint/PrintTopics.c` | Overlapping `strcpy`. |
| `dtsr/dtsrkdump.c` | Negative array index. |
| `dtappbuilder/src/abmf/resource_file.c` | Garbage return. |
| `dtappbuilder/src/ab/cgen_utils.c` | Double `closedir`. |
| `dtsearchpath` `Environ.h` | Virtual destructor added. |
| `fontaliases/test_fonts_alias.c` | `goto` skipped an initialisation. |
| DtHelp `Graphics.c` and `il/ilpipe.c` | Missing returns. |
| `dtsession/SmError.c` `ToolkitError` and `dtprintinfo/libUI/MotifUI/Debug.c` `_XtError` | Xt fatal-error handlers could return; they now always exit and are marked `_X_NORETURN`. |

### 7.2 Suspicious code the warnings exposed (semantics kept; decide and fix)

Line numbers are those after the cleanup.

- **DtSvc**
  - `include/Dt/ActionP.h:314` `IS_DIR_OBJ()` is always 0 (`==` binds tighter than `&`). A dropped directory is never used as the working directory (`Action.c` `__ExtractCWD`). *(fixed (a45d588b5))*
  - `lib/DtSvc/DtUtil2/UErrNoBMS.c`: `DtFatalError:`/`DtInternalError:`/`DtInformation:` were goto labels, not `case`s, so information messages are logged as errors. *(fixed (a45d588b5))*
  - `WmGWsInfo.c`: `rcode = X(...) >= Success` stores the comparison result, not the status. *(fixed (a45d588b5))*
- **DtHelp**
  - `HelpUtil.c` `_DtHelpSetButtonPositions` compares a variable with itself; probably meant `> minFormWidth`. *(fixed (65c2c9e3b))*
  - `FileListUtils.c` `_DtHelpFileListAddFile` never sets `nameKey`. *(fixed (65c2c9e3b))*
  - `CCDFUtil.c`: `GetCmdData`'s `strip` argument has no effect. *(kept and documented: every command is stripped anyway)*
  - `Layout.c` `BlankTableCell`: border width is 3. *(fixed: width 0 (65c2c9e3b))*
- **DtTerm**
  - `TermViewMenu.c`: the cascade buttons are created even for repeat popups (missing braces around the `PULLDOWN_ACCELERATORS` block). *(fixed (f5e4919aa))*
  - `Term/TermFunction.c` `termFuncErase`: `eraseFromCol0` falls through. *(fixed (f5e4919aa))*
- **ToolTalk**
  - `tt_old_db.C`: the `uid == -1` test can never be true. *(fixed (88b6b849b))*
  - `ttdbserverd/db_server_svc.C`: `read() < sizeof` never detects `-1`. *(fixed (88b6b849b))*
  - `ttdesktop.C`: `TTDT_GET_MAPPED` tests `TTDT_SET_ICONIFIED`. *(kept on purpose and documented (88b6b849b))*
  - `ttdt_Get_Locale` passes `handler` instead of `_handler`. *(fixed (88b6b849b))*
  - `tt_tracefile_parse.C`: missing `break`. *(fixed (88b6b849b))*
  - `mp_s_mp.C` `init_self` drops errors. *(fixed: returns and logs the status (88b6b849b))*
- **dtmail**
  - `SafeWrite/SafeRead() < size_t` never detects `-1` (`Attachment.C`, `ComposeCmds.C`, `RoamCmds.C`). *(fixed (12195a7c3))*
  - `IO.C` `SockOpen` never detects `INADDR_NONE`. *(fixed (12195a7c3))*
  - `RFCMailBox.C` `writeToDumpFile` runs without the `_errorLogging` guard. *(fixed (12195a7c3))*
  - `RoamCmds.C`: the vacation buffer is never freed. *(fixed (12195a7c3))*
- **dtfile**
  - `FileManip.c`: `else` binding in the move/link path. *(fixed (52a2c774d))*
  - `dtcopy/fsrtns.c:465`: `replace && a || b` precedence. *(fixed (9ed14ac3c))*
  - `Desktop.c` `LoadDesktopInfo`: `fgets(NULL)` after the buffer is cleared. *(fixed (9ed14ac3c))*
- **dtprintinfo**
  - `Button.C`: the arrow switch has no `break`s. *(fixed)*
  - `Icon.c`: dangling `else`. *(fixed)*
  - `IconObj.C`: NULL dereference via `par && A || B`. *(fixed)*
  - `BaseObj.C` `SendAction`: NULL `Action*`. *(fixed)*
- **dtwm and small tools**
  - dtwm `DataBaseLoad.c` `ResolveDuplicates`: `strcmp(NULL)` for boxes. *(fixed (3f6e85a0e))*
  - dtcalc `ds_popup.c`: a no-op statement that was probably meant to be `space = …`. *(fixed)*
  - dticon `main.c`: fd tested against 0, and a NULL dereference after `strchr`. *(fixed (also mkstemp))*
- **dtstyle**
  - `Mouse.c`: `_DtAddToResource` runs unconditionally. *(fixed)*
  - `ColorMain.c`: a `defaultName_restore` test that is always false. *(fixed)*
- **dthelp parser** (all 3 copies)
  - `param.c`: the whitespace-collapse block sits outside its loop. *(fixed (f83ea8f54))*
  - `actutil.c`: `%s` used with `M_WCHAR*`. *(fixed (f83ea8f54))*
- **dtappbuilder**
  - `libABil/bil.c`: `AB_BIL_UNDEF` is 259, not 0, so the unknown-value tests are always true. *(fixed)*
  - `gil_loadact.c`: load errors are ignored. *(fixed)*
  - `abobj_set.c`: `a || b && c` precedence. *(fixed)*
  - `pal_panedwin.c`: an unconditional reparent. *(no change needed: intended)*
  - `dtb_session_restore`: an inverted test leaks. *(fixed, also in the dtcodegen string table)*
  - `abmf/resource_file.c`: `assert()` has a side effect. *(fixed)*
- **dtcm**
  - `monthglance.c`: bitwise `&` on booleans. *(fixed)*
  - `x_graphics.c`: a `GR_DEBUG` build does not compile. *(fixed (plus a double free in the same file))*

---

## Correctness bugs found in passing (fix together with the code above)

| Area | Location | Bug | Status |
|---|---|---|---|
| dtterm | `TermPrimPendingText.c:303-326` | `_DtTermPrimPendingTextAppend` never advances `text`; appends over 1 KB repeat the first KB. Fix this before benchmarking with `DtTermDisplaySend`. | fixed (f5e4919aa) |
| dtterm | `TermPrimRenderMb.c:650` | `XtMalloc(wcBufferLen)` is missing `* sizeof(wchar_t)`, a heap overflow. | fixed (f5e4919aa) |
| dtterm | `TermPrimScroll.c:241,257` | `offsetX` is used where `offsetY` is meant. | fixed (f5e4919aa) |
| dtterm | `TermPrimScroll.c:922-925` | The `return` runs unconditionally. | restructured; behaviour was already right (f5e4919aa) |
| dtterm | `TermPrimRender.c:1098` | Mixes byte and character counts. | fixed (f5e4919aa) |
| DtSvc | `Dts.c:301-305` | `max_buf` realloc with the old size, a heap overflow. | fixed (42ec255ae) |
| DtSvc | `Dts.c:1028-1064` | `strstr(NULL)` when `fstat` fails. | fixed (42ec255ae) |
| DtSvc | `DtsMM.c:692` | 1024-byte `strcpy` overflow. | fixed (42ec255ae, be587d36b) |
| DtSvc | `ActionFind.c:295,665` | NULL dereference. | fixed (c1865aaa3) |
| DtSvc | `strtab.c:100-107` | Shift is undefined behaviour for strings longer than 31 bytes. | fixed (FNV-1a) |
| DtSvc | `DtsMM.c:667-676` | Ignores `stat` errors. | fixed (c7a8949d3) |
| DtWidget | `Icon.c:3663` | Height is clamped by the width limit. | fixed (af304b121) |
| DtWidget | `EditAreaData.c:178` | Overlapping `memcpy`. | fixed (af304b121) |
| DtWidget | `EditAreaData.c:1447-1468` | `mb_str_loc` offset is wrong when `startPos > 0`. | fixed (af304b121) |
| ToolTalk | `tt_host_equiv.C:298` | Copies the pointer array, not the address. | fixed (2ceab3bd3) |
| ToolTalk | `mp_s_message.C:1176-1181` | `return` instead of `continue`. | fixed (88b6b849b) |
| ToolTalk | `mp_typedb.C:939-952` | `fd = open() == -1` precedence bug closes stdin/stdout. | fixed (63b2dd541) |
| ToolTalk | `mp_mp.C:295` | `select()` beyond `FD_SETSIZE`. | fixed (04126b7dd) |
| dtfile | `Directory.c:1108` + `:1942` | Symlinks always look modified. | fixed (52a2c774d) |
| dtfile | `File.c:5349` | `WidgetCmp` truncates a pointer difference to int. | fixed (9ed14ac3c) |
| dtfile | `Find.c:2175` | The data type is leaked. | fixed (9ed14ac3c) |
| dtmail | `RFCMailBox.C:4418-4452` | `makeHeaderLine` has no `return`. | fixed (function is void) |
| dtmail | `MsgScrollingList.C:2460` | Out-of-range count, and `delete` instead of `delete[]`. | fixed (12195a7c3) |
| dtcm | `lib/csa/iso8601.c:251` | Compares pointers, not values. | fixed |
| dtcm | `cmscalendar.c` | GC runs in a signal handler. | fixed (pselect loop) |
| dtwm | `WmProperty.c:1905,1921` | Uninitialised free. | fixed (3f6e85a0e) |
| dtwm | `WmIPC.c:736,762` | Unchecked screen index from a ToolTalk message. | fixed (3f6e85a0e) |
| dtwm | `Session.c:794` | `strlen(NULL)`. | fixed (3f6e85a0e) |
| dtwm | `WmMultiHead.c:70-110` | Stale Xinerama cache after RandR changes. | fixed on root ConfigureNotify (3f6e85a0e); same-size head changes need RandR |
| dtwm | `WmBackdrop.c:722` | Pixmap leak. | fixed (3f6e85a0e) |
| dtwm | `WmCEvent.c:854-869` | Clobbers the client's `WM_NORMAL_HINTS`. | fixed (3f6e85a0e) |
| DtEncap | `local.c:196` | `select()` beyond `FD_SETSIZE`. | fixed (poll) |
| dtexec | `Main.c:686` | Only fds below 16 are marked close-on-exec. | fixed (677dd1aa9) |
| dtimsstart | `start.c:33-34` | Only fds below 16 are closed. | fixed (1a820582b) |
| dtsession | `vfork` children | `putenv`/`chdir` after `vfork` is undefined behaviour. | fixed (fork + _exit) |
| DtEditor | spell check | `tmpnam`. | fixed (mkstemp) |

---

## Suggested execution order

1. **Day 1, the sleeps** (Phase 1.1):
   - dtgreet `sleep(5)`, dtlogin getty loop, the ToolTalk 100 ms mount poll;
   - dtcpftogpf, dtappbuilder sync, the restart throttle.
   - These are a few lines each and save whole seconds per login and per folder open.
2. **Phase 0 harness**: startup trace, dtsvcbench, ttbench, mwmbench for dtwm, dtterm throughput. Record a baseline before larger changes.
3. **Shared hot paths**, which every client benefits from:
   - DtSvc T1–T4 and B1;
   - `close_range` everywhere;
   - DtIcon `XGetGeometry`/`strcmp`;
   - DtInitialize second connection (S1);
   - ToolTalk B1–B5 and D1.
4. **Build quick wins**: `disable-static`, `--enable-*` inversion, `compress` dependency, ksh93 `-j`/`$(CC)`, grouped targets instead of `.NOTPARALLEL`, `Dt.h` move-if-change.
5. **Login pipeline**: dtdbcache persistence and parallelism, dtsession_res cache, ttsession early ready, WM-ready fallback.
6. **dtwm**: port the Motif mwm commits (`5953033e`, `d4bb5c3c`, `00e4c1e7`, `27b2dea6`, `ee31dae9`, `6fe46a70`, `721f7a7c`, `28c47e40`, `7242649d`, `c84ebfaa`, `ece1a79f`), then the direct front-panel switch, icon-image cache and backdrop leak. Link against the modernised libXm.
7. **dtfile**: inotify, persistent worker, attribute cache, incremental refresh, work-proc batching, `copy_file_range`.
8. **dtterm**: read loop, frame pacing and damage, UTF-8 fast path, ring scrollback, async GraphicsExpose.
9. **Linking**: real `_LIBADD`, per-program `LDADD`, `--as-needed`, export maps, pam split. Measure with `LD_DEBUG=statistics`.
10. **Applications**: dtmail, dtcm, dtprintinfo, help, dtstyle, in order of user impact.
11. **Non-recursive build** for DtMmdb, tt, dtinfo and dthelp; then docs parallelism; then LTO and PGO.
