# Help benchmarks

The help harness of `TODO.md` Phase 0: the help dialog of libDtHelp
driven in-process (topics, Backtrack, a man page paged down, the Index
dialog over all volumes), `dthelpprint -allTopics` over every installed
volume, `dthelpview` started on a volume and on a man page, and the
reading of a dtinfo infolib through libDtMmdb, and DtSearch queries.  Like
`..`, this directory is not part of the automake build; it links against
the libraries of the built tree and runs its uninstalled programs.

```sh
make -C tools/bench/help                     # also builds ../libcdebench_preload.so
tools/bench/help/run.py -o help.json         # everything, median of 5
tools/bench/help/run.py --quick print view   # one pass of two groups (helpbench print view mmdb dtsearch)
../bench.py compare old.json new.json        # the helpbench cases compare like the C benchmarks
```

Needs at run time: `Xvfb`, `xdotool` (for `../apptime.py`), `python3`,
`nroff` (the man page), the help volumes under
`/usr/dt/appconfig/help/C` (`-d` for another directory), and optionally
`taskset`, `strace` and `perf`.

## Files

| File | What it does |
|---|---|
| `helpbench.c` | Creates a `DtHelpDialog` on a private display and drives it like a user: the cases below, each REPEAT times (`-r`, default 5), reporting the median per operation and the first run separately.  Re-executes itself with `../libcdebench_preload.so` (mallocs, X requests, round trips) and `libhelpbench_preload.so` preloaded.  `-V VOL,...` picks the volumes (default `Filemgr`, 77 topics, many with TIFF figures), `-w WORD` the index search word, `-m PAGE` the man page, `-S DIR` writes a PPM screenshot of the help window after each topic, page and search (one run unless `-r` is given; later runs are `NAME.runN.ppm`, to check what is shown again), so that two builds can be compared with `cmp`; `-v` prints the result counts.  `HELPBENCH_WATCHDOG=SECONDS` prints a backtrace and exits if a case hangs. |
| `helppreload.c` → `libhelpbench_preload.so` | Counts the Xlib calls the help system makes per view: `XAllocColor`, `XAllocNamedColor`, `XLookupColor`, `XParseColor`, `XLoadQueryFont`, `XLoadFont`, `XQueryFont`, `XCreateFontSet`, `XListFonts`, `XPutImage`, `XCreatePixmap`, `XCreateGC`.  helpbench reads them in-process; any other program reports them at exit (and on SIGTERM with `CDEBENCH_REPORT_SIGTERM`) as a `helpbench pid=...` line in the `CDEBENCH_REPORT` file. |
| `mmdbbench.C` | Opens a dtinfo information library as dtinfo does (`OLIAS_DB::openInfoLib`) and reads every section's data and every graphic through the smart pointers dtinfo's node views use, REPEAT times in one process (`-r`, default 3: run 1 is cold for libDtMmdb's page cache); time and CPU per object and the bytes read; `-o FILE` writes the data of the first run, to compare two builds. |
| `srbench.c` | DtSearch queries (default: collocation and boolean queries for the `mksrdb.sh` database) with `DtSearchQuery()` and, when libDtSearch exports it, with the asynchronous `DtSearchQueryStart()`/`DtSearchQueryContinue()`: time, hits, the number of calls and the longest call of the asynchronous search, and whether both gave the same results; then an asynchronous search stopped with `DtSearchQueryStop()`.  Exits 1 on a difference. |
| `mksrdb.sh`, `mksrdb.py` | Build a DtSearch database that stores its documents (AusText flavor: the CDE infolibs' databases only index, and refuse collocation queries) from 3,000 generated documents, with the tree's `dtsrcreate`, `dtsrload` and `dtsrindex` and a Huffman encode table made from the decode tree in `lib/DtSearch/ophuf.c`. |
| `man` | A `man(1)` for hosts without one: DtHelp reads `man NAME` through `popen()`; this formats the page with `nroff -man` (overstrike output, as man gives to a pipe).  `run.py` puts it first on `PATH`. |
| `run.py` | Runs the three groups on a private Xvfb (pinned to CPU 0 with `taskset` unless `--no-pin`) and writes one JSON document. |

## helpbench cases

| Case | One operation |
|---|---|
| `dialog` | create a help dialog on the volume's home topic, show it, destroy it |
| `topics` | one topic, in tree order over every topic of the volumes, as a hyperlink shows it (only `DtNlocationId` set); run 1 is the first visit of each topic |
| `requests` | the same, but setting `DtNhelpVolume` with each topic, as an application's help request does (the dialog then reopens the volume and reformats its title) |
| `revisit` | one of 4 topics just visited, shown again as hyperlinks back and forth do |
| `backtrack` | one Backtrack, after visiting 20 topics |
| `manpage` | load the man page (ksh) |
| `pagedown` | one page down through it, to the end |
| `selresize` | one change of the window width with all of the man page selected (the page is laid out again and the selection carried over) |
| `index_open` | Index... in a new help dialog: it opens on the complete index of the current volume |
| `index_all` | Index... in a new dialog, then "Complete Index" of "All Volumes": the dialog's first scan of the volumes and the search |
| `index_again` | the same search again in the same dialog |
| `index_stop` | Index... in a new dialog, a complete-index search of all volumes stopped at once, the current volume, all volumes again (the result must be `index_all`'s); then a dialog destroyed while its search runs |
| `index_word` | Index... in a new dialog, "Entries with" WORD (default `file`) in all volumes |

Each operation ends with the event queue drained and one `XSync`, so the
round trips per operation include that one.  The counts (mallocs,
requests, round trips, Xlib calls) are exact and are what a regression
gate should hold; times vary with the load of the machine.

## print and view

`print` runs `dthelpprint -helpVolume V -allTopics -outputFile F` for
every volume: wall time of the whole set (median), CPU, the preloads'
totals of all the runs, a SHA-256 of all the text printed (identical for
two builds that print the same), `strace -f -c` syscall totals (the top
twelve) and `perf stat` task-clock and page faults of one pass (both of
these include the `sh` that runs the pass).

`view` starts `dthelpview -helpVolume Filemgr` and `dthelpview -manPage
ksh` under `../apptime.py`: time to the first window and to idle, CPU,
and the same counts.

## mmdb

`mmdb` runs `mmdbbench` on the infolib (`--infolib`, default
`/usr/dt/infolib/C/cde.dti`): open time, time and CPU per section and
per graphic for each run, the bytes read, and the syscalls of the whole
run (`strace -f -c`).

## dtsearch

`dtsearch` builds the `mksrdb.sh` database in a temporary directory
(500 documents with `--quick`) and runs `srbench` on it.
