#!/usr/bin/env python3
#
# CDE benchmarks: dtterm round trips per "less" page-down over a slow link.
#
# Licensed under the LGPL 2.1 license.
#
"""Page down through a file with less in dtterm, over a delayed X link.

  dtterm-less.py [-d MS] [-p PAGES] [-l LINES] [--label NAME]

Runs the uninstalled dtterm of this tree ($DTTERM to override), 80x24,
on a private Xvfb through cdebench-proxy with MS milliseconds (default
2) of added delay each way, so that every round trip costs at least 2*MS
of wall time, as on a remote display; inside it "less" shows a text file
of LINES lines (default 20000).  Once dtterm is idle it sends PAGES
(default 50) page-downs (space) through XTest, each once dtterm has
gone idle again, and prints one JSON object: per page, the X requests
and round trips of dtterm's connections (from the proxy's snapshots
before and after) and the time from the key to dtterm's last CPU use
(its scheduler CPU time, sampled every 2 ms; the cursor does
not blink).  The target is 0 round trips per page.
"""

import argparse
import json
import os
import shutil
import signal
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import apptime  # noqa: E402


def proxy_counts(path):
    """requests, round trips over the connections of the last snapshot
    (the lines that say "live": true, after the last non-live block)."""
    snaps = []
    cur = []
    try:
        lines = open(path).read().splitlines()
    except OSError:
        return 0, 0
    for line in lines:
        try:
            c = json.loads(line)
        except ValueError:
            continue
        if c.get("live"):
            if cur and c["conn"] <= cur[-1]["conn"]:
                snaps.append(cur)
                cur = []
            cur.append(c)
    if cur:
        snaps.append(cur)
    if not snaps:
        return 0, 0
    last = snaps[-1]
    return (sum(c["requests"] for c in last),
            sum(c["round_trips"] for c in last))


def sched_ns(pid):
    """CPU time of pid in ns, from the scheduler (no clock tick rounding)."""
    try:
        with open("/proc/%d/schedstat" % pid) as f:
            return int(f.read().split()[0])
    except (OSError, ValueError, IndexError):
        return 0


def wait_quiet(pid, quiet_s, timeout, start_s=0.0, cpu0=None):
    """Wait until pid has used no CPU for quiet_s; returns the time of its
    last use.  With start_s, first wait that long for it to start (to use
    more than cpu0)."""
    t0 = time.monotonic()
    last_cpu = sched_ns(pid) if cpu0 is None else cpu0
    last = None
    while time.monotonic() - t0 < start_s:
        c = sched_ns(pid)
        if c != last_cpu:
            last_cpu, last = c, time.monotonic()
            break
        time.sleep(0.001)
    if last is None:
        last = time.monotonic()
    while True:
        now = time.monotonic()
        c = sched_ns(pid)
        if c != last_cpu:
            last_cpu, last = c, now
        elif now - last >= quiet_s:
            return last
        if now - t0 > timeout:
            return last
        time.sleep(0.002)


def snapshot(proxy, path):
    size = os.path.getsize(path) if os.path.exists(path) else 0
    proxy.send_signal(signal.SIGUSR1)
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline:
        if os.path.exists(path) and os.path.getsize(path) > size:
            time.sleep(0.05)
            break
        time.sleep(0.01)
    return proxy_counts(path)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-d", "--delay", type=float, default=2.0)
    ap.add_argument("-p", "--pages", type=int, default=50)
    ap.add_argument("-l", "--lines", type=int, default=20000)
    ap.add_argument("--label")
    ap.add_argument("--idle-ms", type=int, default=150)
    a = ap.parse_args()
    top = os.environ.get("CDE_TOP", os.path.join(HERE, "..", ".."))
    dtterm = os.environ.get("DTTERM") or os.path.join(
        top, "programs", "dtterm", ".libs", "dtterm")
    if not shutil.which("less"):
        raise SystemExit("dtterm-less: needs less")
    work = tempfile.mkdtemp(prefix="dttermless.")
    helpers = []
    try:
        xvfb = apptime.start_xvfb()
        helpers.append(xvfb)
        display = os.environ["DISPLAY"]
        text = os.path.join(work, "text")
        with open(text, "w") as f:
            for i in range(a.lines):
                f.write("%06d %s\n" % (i, "the quick brown fox jumps over "
                                       "the lazy dog " * 4)[:79])
        port = apptime.free_proxy_display()
        stats = os.path.join(work, "proxy")
        proxy = subprocess.Popen(
            [os.path.join(HERE, "cdebench-proxy"), "-d", str(a.delay), "-s",
             stats, str(port), "/tmp/.X11-unix/X" + display.lstrip(":")],
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        proxy.stdout.readline()
        helpers.append(proxy)
        # The tree's libraries, as cdeenv.sh sets them.
        libs = [os.path.join(top, "lib", d, ".libs") for d in
                ("DtSvc", "DtWidget", "DtHelp", "DtPrint", "DtXinerama",
                 "DtTerm", "DtSearch", "DtMmdb", "DtMrm", "csa", "tt/lib")]
        lp = ":".join(d for d in libs if os.path.isdir(d))
        if os.environ.get("LD_LIBRARY_PATH"):
            lp += ":" + os.environ["LD_LIBRARY_PATH"]
        env = dict(os.environ, DISPLAY="127.0.0.1:%d" % port, HOME=work,
                   LESS="", LESSHISTFILE="-", LD_LIBRARY_PATH=lp)
        # No cursor blinking, which would make dtterm busy every 0.5 s.
        term = subprocess.Popen([dtterm, "-geometry", "80x24", "-xrm",
                                 "*blinkRate: 0", "-e", "less", text],
                                env=env, stdin=subprocess.DEVNULL,
                                stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL)
        helpers.append(term)
        win = None
        deadline = time.monotonic() + 60
        while time.monotonic() < deadline and not win:
            out = subprocess.run(["xdotool", "search", "--onlyvisible",
                                  "--class", "Dtterm"],
                                 capture_output=True, text=True).stdout.split()
            win = out[-1] if out else None
            time.sleep(0.1)
        if not win:
            raise SystemExit("dtterm-less: no dtterm window")
        quiet = a.idle_ms / 1000.0
        wait_quiet(term.pid, quiet, 60)
        subprocess.run(["xdotool", "mousemove", "--window", win, "100", "100"])
        subprocess.run(["xdotool", "windowfocus", "--sync", win])
        wait_quiet(term.pid, quiet, 60)
        r0, t0 = snapshot(proxy, stats)
        times = []
        for _ in range(a.pages):
            # Sample while xdotool runs: dtterm may be done before it
            # has exited.
            cpu0 = sched_ns(term.pid)
            k = time.monotonic()
            key = subprocess.Popen(["xdotool", "key", "space"])
            last = wait_quiet(term.pid, quiet, 60, start_s=1.0, cpu0=cpu0)
            key.wait()
            times.append(max(last - k, 0.0))
        r1, t1 = snapshot(proxy, stats)
        times.sort()
        res = {"bench": "dtterm-less",
               "name": a.label or "less-pagedown-%gms" % a.delay,
               "delay_ms": a.delay, "pages": a.pages,
               "requests_per_page": round((r1 - r0) / a.pages, 2),
               "round_trips_per_page": round((t1 - t0) / a.pages, 3),
               "seconds": round(sum(times) / a.pages, 4),
               "median_page_s": round(times[len(times) // 2], 4)}
        print(json.dumps(res), flush=True)
        return 0
    finally:
        for h in reversed(helpers):
            if h.poll() is None:
                h.terminate()
                try:
                    h.wait(10)
                except subprocess.TimeoutExpired:
                    h.kill()
        if "DISPLAY" in os.environ:
            apptime.remove_dtdbcache(os.environ["DISPLAY"])
        shutil.rmtree(work, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
