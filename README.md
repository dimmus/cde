CDE — the Common Desktop Environment
===

The classic Unix desktop (the Motif-based environment of Solaris, HP-UX
and AIX), open-sourced by The Open Group in 2012 under the LGPL v2 (see
`COPYING`).

This repository is a fork of the upstream project
([SourceForge](https://sourceforge.net/p/cdesktopenv/) /
[GitHub mirror](https://github.com/jontrulson/cdesktopenv)). It tracks
upstream: everything up to upstream commit `c5727a779` (September 2026,
after the 2.5.3 release) is merged. On top of that it adds:

- a window manager that speaks the modern window manager standards
  (EWMH), so pagers, taskbars, docks, video players and today's
  GTK/Qt/Electron applications behave;
- a desktop that starts and runs much faster: no fixed sleeps on the
  login path, far fewer X round trips, linear instead of quadratic
  algorithms in the file manager, terminal, mail, calendar and help;
- hundreds of bug fixes, a `-Wall`-clean build with no use of
  deprecated or obsolete Motif API;
- automatic display setup (`dtrandr`): the right monitor layout at
  login and when monitors are plugged in;
- a modern, parallel, hardened build with configurable install paths.

It is developed on Arch Linux against
[dimmus/motif](https://github.com/dimmus/motif) (Motif 2.5).

## What is new

### dtwm: EWMH (Extended Window Manager Hints)

| Area | What dtwm now does |
|---|---|
| Window lists | `_NET_CLIENT_LIST` and `_NET_CLIENT_LIST_STACKING` (dtwm's own windows — front panel, subpanels, icon box — are not listed) |
| Desktops | CDE workspaces are EWMH desktops: count, names, current desktop, geometry, viewport, `_NET_WM_DESKTOP` per window (all workspaces = `0xFFFFFFFF`). Pagers and tools can switch, add and remove workspaces and move windows (`xdotool set_desktop`, `set_desktop_for_window`, …) |
| Window states | Maximized (both axes or one), full screen, above, below, sticky, modal, hidden, skip taskbar/pager, demands attention (also from the urgency hint), focused. A state set before the window maps is honoured, so players and games can start full screen |
| Window types | Desktop, dock, toolbar, menu, utility, splash, dialog, dropdown/popup menu, tooltip, notification, combo, DnD, normal: each gets suitable decorations, focus, layer, placement and workspace behaviour |
| Stacking layers | Desktop < below < normal < above < dock < full screen (while focused) < system modal, enforced for both the X stacking order and the published list |
| Struts / work area | `_NET_WM_STRUT(_PARTIAL)` and `_NET_WORKAREA`; maximize and placement keep out of reserved edges, per monitor. `Dtwm*frontPanelStrut: True` makes the front panel reserve its edge too |
| Requests | `_NET_ACTIVE_WINDOW`, `_NET_CLOSE_WINDOW`, `_NET_MOVERESIZE_WINDOW`, `_NET_RESTACK_WINDOW`, `_NET_REQUEST_FRAME_EXTENTS`, `_NET_SHOWING_DESKTOP` (iconify all, the CDE way), `_NET_WM_MOVERESIZE` (client-side decorations can start a move or a resize from any edge) |
| Other | `_NET_WM_ALLOWED_ACTIONS`, `_NET_FRAME_EXTENTS`, `_NET_WM_ICON` as the icon image, `_NET_WM_USER_TIME` 0 = do not focus |

Also fixed in dtwm: `_MOTIF_WM_HINTS` follow Motif's rule again (a
client can only remove functions and decorations), frames are rebuilt
when a window changes its decorations at run time, menus work on screens
other than 0, the hourglass shows at startup and on front panel
launches, and moves and resizes no longer poll the pointer.

Not implemented: shaded windows, `_NET_WM_PING`, `_NET_WM_SYNC_REQUEST`,
`_NET_DESKTOP_LAYOUT`, virtual roots.

### dtrandr: automatic display setup

`dtrandr` (new, in C on libXrandr) detects the connected monitors by
their EDIDs and applies the matching saved display setup — which
monitors are on, their modes, positions, rotation, scaling, gamma and
the primary one — like [autorandr](https://github.com/phillipberndt/autorandr),
whose profile format and hooks it uses (existing autorandr profiles work
as they are). `dtrandr --save NAME` saves the current setup. If no
profile matches, newly connected monitors are turned on to the right of
the others and unplugged ones are turned off. The login screen applies
the system profiles; each session applies the user's and follows
monitors being plugged in or out (`DTRANDR=no` in `~/.dtprofile` turns
it off). See `dtrandr(1)`.

### Speed

Every change was measured with the harnesses in `tools/bench`. A
selection (all numbers and methods are in `TODO.md`):

| Area | Before → after |
|---|---|
| Login greeter visible | 5.15 s → 0.17 s |
| First X server start | 5 s less (a fixed wait loop) |
| Session waiting for a non-CDE window manager | 60.3 s → 0.36 s |
| Starting a program with a high fd limit | 48–57 ms → ~7 ms |
| ToolTalk path mapping (`tt_file_netfile`) | 112 ms → 0.02 ms |
| Data typing per file (dtfile, actions) | 22–26 µs → 4.5–5.6 µs |
| dtwm window map | 18 → 9 round trips per window |
| dtwm window drag | 3.09 → 1.01 round trips per step; idle CPU 26% → 0.8% |
| dtfile: a change on disk becomes visible | 0–3 s (polling) → ~105 ms (inotify) |
| dtfile: copy 5100 small files | 1.1–4.5 s → 0.11 s |
| dtterm: `cat` 40 MB of text | 6.6–6.9 s → 1.6–1.9 s |
| dtmail: open a 50k-message mailbox | 5.2–6.0 s → 2.2 s, and no rewrite of the mbox |
| dtcm: load 20k repeating entries | 27–33 s → 0.8–1.2 s |
| Help: lay out a 20k-word topic | 1.39 s → 0.084 s |
| `dtcalc` start | 23 → 7 shared libraries, loader time −20% |

### Correctness and security

- The whole tree builds with `-Wall` (and `-Wformat-truncation`) without
  warnings; the bugs the warnings exposed are fixed, not silenced.
- Temporary files use `mkstemp`/`mkdtemp` everywhere (no
  `tmpnam`/`tempnam`/`mktemp`).
- No deprecated or obsolete Motif API is used: each call was replaced by
  what Motif itself does, checked call by call against the old one.
  DtHelp no longer calls Motif's private `_XmPutScaledImage`, whose
  signature changed in Motif 2.5.
- Fixed along the way: heap overflows (DtHelp printing, dtmail, dtcm,
  dtbuilder), use-after-free and double `closedir`, a stack overwrite in
  dtimsstart, a dtsession crash when a client closes with reasons, a
  dtwm crash creating workspaces, `_FORTIFY_SOURCE` aborts in DtSearch,
  broken `va_list` use, and many leaks.
- Shared libraries export only their API (export maps): libtt 3299 →
  1441 symbols, libDtMmdb 4030 → 894.

### Build system

- Fully parallel (`make -j`): no `.NOTPARALLEL`, ordering fixed for
  generated sources, the bundled ksh93 built with the tree's compiler,
  flags and `-j`.
- Each library and program links only what it uses (`--as-needed`,
  no unresolved symbols).
- Hardening on by default (`_FORTIFY_SOURCE`, stack protector, RELRO,
  `-z now`), `NDEBUG` unless `--enable-debug`.
- `--enable-lto`, `--with-linker=mold|lld|gold|bfd`, reproducible paths
  (`-ffile-prefix-map`).
- Reconfiguring rebuilds nothing that did not change; generated files
  are never left half-written.
- Configurable install roots for distribution packaging
  (`--with-cde-install-dir`, `--with-cde-data-dir`,
  `--with-cde-config-dir`, `--with-cde-state-dir`) with a checker,
  `util/check-fhs.sh`; `--with-pam-dir`; a systemd unit for dtlogin when
  systemd is in use; installation into a staging directory and as
  non-root.
- Documentation can be built per part (`--disable-help-docs`,
  `--disable-man-pages`, `--disable-infolib`) and in parallel.

### From upstream

Merged from upstream CDE: C23/GCC 15 strict prototypes, the dtsession
SmcCloseConnection crash fix, dtwm multi-screen menus, extra mouse
buttons and `_NET_ACTIVE_WINDOW`, Chinese (Simplified and Traditional)
locales, the configurable install paths, PAM directory and non-root
install, tic no longer writing the live terminfo database, and many
memory, buffer and parallel-build fixes. Where upstream and this fork
had fixed the same thing, the fork's fix was kept; a few upstream
changes were corrected on the way (see `git log`).

## Building

### Arch Linux

```sh
./install-deps.sh                 # build and runtime dependencies (pacman)
```

Motif is not taken from pacman: build and install
[dimmus/motif](https://github.com/dimmus/motif) first
(`install-deps.sh` checks for it and says what is missing).

```sh
./autogen.sh --prefix=/usr        # runs autoreconf and configure
make -j"$(nproc)"
sudo make install
```

Run `make` as yourself before `sudo make install`; otherwise the install
step may regenerate build files as root.

### Other systems

The upstream [CDE wiki](https://sourceforge.net/p/cdesktopenv/wiki/Home/)
has the prerequisites for other Linux distributions and the BSDs
(localization and locale set-up included). On the BSDs use `gmake` and
point configure at Tcl:

```sh
./autogen.sh --with-tcl=/usr/local/lib/tcl8.6 MAKE=gmake
gmake
sudo gmake install
```

### Useful configure options

| Option | Effect |
|---|---|
| `--prefix=/usr` | programs in `/usr/bin`, libraries in `/usr/lib` |
| `--with-cde-install-dir=DIR` | the CDE tree (default `/usr/dt`, whatever `--prefix` is) |
| `--with-cde-data-dir`, `--with-cde-config-dir`, `--with-cde-state-dir` | data (default: the CDE tree), configuration (`/etc/dt`) and state (`/var/dt`) |
| `--disable-docs`, `--disable-help-docs`, `--disable-man-pages`, `--disable-infolib` | skip all or part of the documentation (much faster builds) |
| `--enable-german`, `--enable-french`, `--enable-italian`, `--enable-spanish`, `--enable-japanese`, `--enable-chinese`, `--enable-chinese-tw` | extra locales |
| `--enable-lto`, `--with-linker=NAME` | link-time optimisation, faster linkers |
| `--enable-debug` | keep `assert()` checks |
| `--with-pam-dir=DIR`, `--with-systemdsystemunitdir=DIR` | where the PAM file and the dtlogin unit go |

The default layout is the traditional one (`/usr/dt`, `/etc/dt`,
`/var/dt`). A distribution-style layout, for example:

```sh
./autogen.sh --prefix=/usr --with-cde-install-dir=/usr \
    --with-cde-data-dir=/usr/share/cde --with-cde-config-dir=/etc/cde \
    --with-cde-state-dir=/var/lib/cde
```

## Tests and benchmarks

`tools/bench` holds the benchmark harnesses (login path, DtSvc, ToolTalk,
dtwm, dtfile, dtterm, dtmail, help, build and loader cost) and
`ewmhtest`, the functional test of dtwm's EWMH support. They run
against the build tree under Xvfb; see `tools/bench/README.md`.

```sh
make -C tools/bench
Xvfb :9 & DISPLAY=:9 tools/bench/ewmhtest all        # or -f: with the front panel
```

`TODO.md` is the performance and correctness audit this work follows:
every item, its status and its measured effect.

## License

LGPL v2; see `COPYING`. Contributors are listed in `CONTRIBUTORS`.
