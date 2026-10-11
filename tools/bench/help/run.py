#!/usr/bin/env python3
#
# CDE benchmarks, help: run the help harness and write one JSON document.
#
# Licensed under the LGPL 2.1 license.
#
"""Run the help benchmarks of TODO.md Phase 0 on a private Xvfb.

  run.py [-o FILE] [-r REPEAT] [-d HELPDIR] [--quick] [GROUP...]

Groups (all by default):

  helpbench  ./helpbench: the help dialog in-process (topics, Backtrack,
             man page, page down, Index over all volumes); per operation
             time, mallocs, X requests, round trips and XAllocColor /
             XLoadQueryFont / XCreateFontSet / XPutImage... counts
  print      dthelpprint -allTopics of every volume of HELPDIR (default
             /usr/dt/appconfig/help/C) into files: wall time (median of
             REPEAT), CPU, mallocs, X requests and round trips and the
             Xlib call counts of all the runs, a perf stat and an
             strace -c of one pass, and a SHA-256 of the text written
             (to check that two builds print the same)
  view       dthelpview on a volume and on the ksh(1) man page: time to
             the first window and to idle (../apptime.py), with the same
             counts

The man page is formatted by ./man (nroff), which is put first on PATH.
The programs are the uninstalled ones of this build tree.  Compare two
JSON documents with "../bench.py compare" (the helpbench cases have the
same members as the C benchmarks there).
"""

import argparse
import glob
import hashlib
import json
import os
import re
import shutil
import statistics
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
BENCH = os.path.dirname(HERE)
TOP = os.environ.get("CDE_TOP", os.path.abspath(os.path.join(BENCH, "..", "..")))
PRELOADS = [os.path.join(BENCH, "libcdebench_preload.so"),
            os.path.join(HERE, "libhelpbench_preload.so")]
XCALLS = ["XAllocColor", "XAllocNamedColor", "XLookupColor", "XParseColor",
          "XLoadQueryFont", "XLoadFont", "XQueryFont", "XCreateFontSet",
          "XListFonts", "XPutImage", "XCreatePixmap", "XCreateGC"]


def log(msg):
    print("help/run: " + msg, file=sys.stderr, flush=True)


def libpath():
    dirs = []
    for d in ["DtSvc", "DtWidget", "DtHelp", "DtPrint", "DtXinerama",
              "DtTerm", "DtSearch", "DtMmdb", "DtMrm", "csa", "tt/lib"]:
        p = os.path.join(TOP, "lib", d, ".libs")
        if os.path.isdir(p):
            dirs.append(p)
    old = os.environ.get("LD_LIBRARY_PATH")
    return ":".join(dirs + ([old] if old else []))


def cde_bin(name):
    for pat in ["programs/dthelp/%s/.libs/%s", "programs/*/.libs/%s",
                "programs/*/*/.libs/%s"]:
        hits = glob.glob(os.path.join(TOP, pat.replace("%s", name, 2)))
        if hits:
            return hits[0]
    return shutil.which(name)


def start_xvfb(pin):
    rd, wr = os.pipe()
    proc = subprocess.Popen(pin + ["Xvfb", "-displayfd", str(wr), "-screen",
                                   "0", "1920x1080x24", "+extension",
                                   "RENDER", "-nolisten", "tcp", "-noreset"],
                            pass_fds=(wr,), stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    os.close(wr)
    number = b""
    while not number.endswith(b"\n"):
        chunk = os.read(rd, 16)
        if not chunk:
            break
        number += chunk
    os.close(rd)
    if not number.strip().isdigit():
        proc.kill()
        raise SystemExit("help/run: Xvfb did not start")
    return proc, ":" + number.decode().strip()


def read_report(path):
    """{pid: {counter: value}} from the preloads' report lines."""
    res = {}
    try:
        with open(path) as f:
            for line in f:
                kv = dict(t.split("=", 1) for t in line.split()[1:]
                          if "=" in t)
                if "pid" not in kv:
                    continue
                d = res.setdefault(int(kv["pid"]), {"comm": kv.get("comm")})
                for k, v in kv.items():
                    if k not in ("pid", "ppid", "comm") and v.isdigit():
                        d[k] = int(v)
    except OSError:
        pass
    return res


def sum_report(rep, comm=None):
    tot = {}
    for d in rep.values():
        if comm and d.get("comm") != comm:
            continue
        for k, v in d.items():
            if k != "comm":
                tot[k] = tot.get(k, 0) + v
    out = {"mallocs": tot.get("mallocs", 0),
           "requests": tot.get("requests", 0),
           "round_trips": tot.get("rtrips", 0)}
    for k in XCALLS:
        out[k] = tot.get(k, 0)
    return out


def run_helpbench(env, args, work):
    exe = os.path.join(HERE, "helpbench")
    out = os.path.join(work, "helpbench.json")
    cmd = args.pin + [exe, "-r", str(args.repeat), "-j", out]
    if args.volumes:
        cmd += ["-V", args.volumes]
    if args.quick:
        cmd += ["-r", "1"]
    log("helpbench")
    subprocess.run(cmd, env=env, check=True, stderr=subprocess.DEVNULL)
    with open(out) as f:
        return json.load(f)


def run_print(env, args, work):
    exe = cde_bin("dthelpprint")
    vols = sorted(glob.glob(os.path.join(args.helpdir, "*.sdl")))
    if not exe or not vols:
        log("print: no dthelpprint or no volumes; skipped")
        return None
    outdir = os.path.join(work, "print")
    os.makedirs(outdir, exist_ok=True)

    def one_pass(prefix=None, report=None):
        e = dict(env)
        if report:
            e["LD_PRELOAD"] = ":".join(PRELOADS)
            e["CDEBENCH_REPORT"] = report
        for v in vols:
            name = os.path.basename(v)[:-4]
            cmd = (prefix or []) + args.pin + [
                exe, "-helpVolume", v, "-allTopics",
                "-outputFile", os.path.join(outdir, name + ".txt")]
            subprocess.run(cmd, env=e, stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL)

    log("print: %d volumes" % len(vols))
    one_pass()                                  # warm the page cache
    walls, cpus = [], []
    for _ in range(1 if args.quick else args.repeat):
        r0 = os.times()
        t0 = time.monotonic()
        one_pass()
        walls.append(time.monotonic() - t0)
        r1 = os.times()
        cpus.append(r1.children_user + r1.children_system -
                    r0.children_user - r0.children_system)
    report = os.path.join(work, "print.report")
    one_pass(report=report)
    digest = hashlib.sha256()
    for v in vols:
        with open(os.path.join(outdir, os.path.basename(v)[:-4] + ".txt"),
                  "rb") as f:
            digest.update(f.read())
    res = {"volumes": len(vols), "wall_s": round(statistics.median(walls), 4),
           "cpu_s": round(statistics.median(cpus), 4),
           "output_sha256": digest.hexdigest()}
    res.update(sum_report(read_report(report), "dthelpprint"))

    # strace and perf: one pass over all volumes under one shell
    cmdfile = os.path.join(work, "print.sh")
    with open(cmdfile, "w") as f:
        for v in vols:
            f.write("%s -helpVolume %s -allTopics -outputFile /dev/null"
                    " >/dev/null 2>&1\n" % (exe, v))
    if shutil.which("strace"):
        sfile = os.path.join(work, "print.strace")
        subprocess.run(["strace", "-f", "-c", "-o", sfile, "sh", cmdfile],
                       env=env, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        calls = {}
        total = None
        with open(sfile) as f:
            for line in f:
                p = line.split()
                if len(p) >= 5 and p[0][0].isdigit() and p[-1] != "total":
                    try:
                        calls[p[-1]] = int(p[3])
                    except ValueError:
                        pass
                elif p and p[-1] == "total" and len(p) >= 4:
                    total = int(p[3])
        top = sorted(calls.items(), key=lambda kv: -kv[1])[:12]
        res["syscalls_total"] = total if total is not None else sum(calls.values())
        res["syscalls_top"] = dict(top)
    if shutil.which("perf"):
        pfile = os.path.join(work, "print.perf")
        subprocess.run(["perf", "stat", "-x,", "-o", pfile, "-e",
                        "task-clock,page-faults,context-switches",
                        "sh", cmdfile], env=env, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        perf = {}
        try:
            with open(pfile) as f:
                for line in f:
                    p = line.strip().split(",")
                    if len(p) > 3 and p[0] and p[0][0].isdigit():
                        perf[p[2].split(":")[0]] = float(p[0])
        except OSError:
            pass
        res["perf"] = perf
    return res


def run_view(env, args, work):
    exe = cde_bin("dthelpview")
    if not exe:
        log("view: no dthelpview; skipped")
        return None
    out = {}
    cases = [("volume", ["-helpVolume", "Filemgr"]),
             ("manpage", ["-manPage", "ksh"])]
    for name, extra in cases:
        report = os.path.join(work, "view-%s.report" % name)
        if os.path.exists(report):
            os.unlink(report)
        runs = []
        for _ in range(1 if args.quick else min(args.repeat, 3)):
            if os.path.exists(report):
                os.unlink(report)
            cmd = [sys.executable, os.path.join(BENCH, "apptime.py"),
                   "--no-preload", "--name", ".", "--idle-ms", "700",
                   "--", "env", "LD_PRELOAD=" + ":".join(PRELOADS),
                   "CDEBENCH_REPORT=" + report,
                   "CDEBENCH_REPORT_SIGTERM=1"] + args.pin + [exe] + extra
            log("view: " + name)
            p = subprocess.run(cmd, env=env, capture_output=True, text=True)
            try:
                r = json.loads(p.stdout.strip().splitlines()[-1])
            except (ValueError, IndexError):
                log("view: %s failed: %s" % (name, p.stderr.strip()))
                continue
            r.update(sum_report(read_report(report), "dthelpview"))
            runs.append(r)
        if runs:
            keys = [k for k in runs[0] if isinstance(runs[0][k], (int, float))]
            out[name] = {k: statistics.median(r[k] for r in runs
                                              if r.get(k) is not None)
                         for k in keys if all(r.get(k) is not None
                                              for r in runs)}
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-o", "--output", help="JSON file (default: stdout)")
    ap.add_argument("-r", "--repeat", type=int, default=5)
    ap.add_argument("-d", "--helpdir", default="/usr/dt/appconfig/help/C")
    ap.add_argument("-V", "--volumes", help="helpbench volumes (-V)")
    ap.add_argument("--quick", action="store_true")
    ap.add_argument("--no-pin", action="store_true")
    ap.add_argument("groups", nargs="*")
    args = ap.parse_args()
    groups = args.groups or ["helpbench", "print", "view"]
    for g in groups:
        if g not in ("helpbench", "print", "view"):
            ap.error("unknown group " + g)
    args.pin = [] if args.no_pin or not shutil.which("taskset") else \
        ["taskset", "-c", "0"]
    for f in PRELOADS + [os.path.join(HERE, "helpbench")]:
        if not os.path.exists(f):
            raise SystemExit("help/run: %s missing: make -C %s" % (f, HERE))

    work = tempfile.mkdtemp(prefix="helpbench.")
    xvfb, display = start_xvfb(args.pin)
    env = dict(os.environ)
    env.update({"DISPLAY": display, "LD_LIBRARY_PATH": libpath(),
                "PATH": HERE + os.pathsep + env.get("PATH", ""),
                "HELPBENCH_WATCHDOG": "900"})
    doc = {"bench": "help", "version": 1, "repeat": args.repeat,
           "helpdir": args.helpdir}
    try:
        if "helpbench" in groups:
            hb = run_helpbench(env, args, work)
            doc["cases"] = hb["cases"]
            doc["topics"] = hb.get("topics")
        if "print" in groups:
            doc["print"] = run_print(env, args, work)
        if "view" in groups:
            doc["view"] = run_view(env, args, work)
    finally:
        xvfb.terminate()
        try:
            xvfb.wait(10)
        except subprocess.TimeoutExpired:
            xvfb.kill()
        try:
            os.unlink("/tmp/dtdbcache_" + display)
        except OSError:
            pass
        shutil.rmtree(work, ignore_errors=True)
    text = json.dumps(doc, indent=2)
    if args.output:
        with open(args.output, "w") as f:
            f.write(text + "\n")
    else:
        print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
