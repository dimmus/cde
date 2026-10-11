#!/usr/bin/env python3
#
# CDE benchmarks: build a variant of one program of the tree elsewhere.
#
# Licensed under the LGPL 2.1 license.
#
"""Build a program of the tree with extra flags, outside the tree.

  perfbuild.py [-D MACRO]... [--include HEADER]... [-o OUTDIR]
               PROGRAM_DIR PROGRAM

Compiles the sources of PROGRAM (its PROGRAM_SOURCES) with the flags its
generated Makefile in PROGRAM_DIR uses, plus the -D options, into
OUTDIR (default a new temporary directory), links it against the
libraries of the build tree (*.la -> -L.../.libs -lNAME, with an rpath),
and prints the path of the binary.  Nothing in the tree is written: the
Makefile is read as text (its variables expanded here), not run, so that
make never decides to regenerate it.

  perfbuild.py -D DT_PERFORMANCE --include Dt/ChkpntP.h programs/dtfile dtfile

builds the dtfile with the old DT_PERFORMANCE timers (dtfile-perf.sh;
that code calls the checkpoint functions without declaring them).
"""

import argparse
import os
import re
import shlex
import subprocess
import sys
import tempfile

VAR = re.compile(r'\$\(([A-Za-z0-9_]+)\)')


def read_vars(path):
    """The variable assignments of a generated Makefile (NAME = value,
    with continuation lines); later ones win, += appends."""
    vars_ = {}
    with open(path) as f:
        text = f.read().replace("\\\n", " ")
    for line in text.splitlines():
        if line.startswith("\t") or line.lstrip().startswith("#"):
            continue
        m = re.match(r'^([A-Za-z0-9_]+)\s*(\+?=|:=)\s*(.*)$', line)
        if not m:
            continue
        name, op, value = m.groups()
        if op == "+=" and name in vars_:
            vars_[name] += " " + value
        else:
            vars_[name] = value
    return vars_


def expand(value, vars_, depth=0):
    if depth > 30:
        return value
    return VAR.sub(lambda m: expand(vars_.get(m.group(1), ""), vars_,
                                    depth + 1), value)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-D", dest="defines", action="append", default=[])
    ap.add_argument("--include", action="append", default=[])
    ap.add_argument("-o", "--outdir")
    ap.add_argument("dir")
    ap.add_argument("program")
    a = ap.parse_args()
    pdir = os.path.abspath(a.dir)
    v = read_vars(os.path.join(pdir, "Makefile"))
    p = a.program.replace("-", "_").replace(".", "_")
    # Split as the shell make runs would: -DX=\"/usr/dt\" is -DX="/usr/dt".
    get = lambda name: shlex.split(expand(v.get(name, ""), v))  # noqa: E731
    cpp = get(p + "_CPPFLAGS") if p + "_CPPFLAGS" in v else get("AM_CPPFLAGS")
    cfl = get(p + "_CFLAGS") if p + "_CFLAGS" in v else get("AM_CFLAGS")
    cc = get("CC") or ["cc"]
    flags = (get("DEFS") + get("DEFAULT_INCLUDES") + cpp + get("CPPFLAGS")
             + cfl + get("CFLAGS") + ["-D" + d for d in a.defines])
    for h in a.include:
        flags += ["-include", h]
    out = a.outdir or tempfile.mkdtemp(prefix="perfbuild.")
    os.makedirs(out, exist_ok=True)
    objs = []
    for src in get(p + "_SOURCES"):
        if not src.endswith(".c"):
            continue
        obj = os.path.join(out, os.path.basename(src)[:-2] + ".o")
        cmd = cc + flags + ["-c", src, "-o", obj]
        if subprocess.call(cmd, cwd=pdir) != 0:
            print("perfbuild: failed: " + " ".join(cmd), file=sys.stderr)
            return 1
        objs.append(obj)
    libs = []
    ldadd = get(p + "_LDADD") if p + "_LDADD" in v else get("LDADD")
    for item in ldadd + get("LIBS"):
        if item.endswith(".la"):
            path = os.path.normpath(os.path.join(pdir, item))
            d = os.path.join(os.path.dirname(path), ".libs")
            name = os.path.basename(path)[3:-3]
            libs += ["-L" + d, "-Wl,-rpath," + d, "-l" + name]
        elif item.endswith(".o") or item.endswith(".a"):
            libs.append(os.path.normpath(os.path.join(pdir, item)))
        else:
            libs.append(item)
    exe = os.path.join(out, a.program)
    cmd = cc + get("CFLAGS") + get("AM_LDFLAGS") + get("LDFLAGS") + \
        ["-o", exe] + objs + libs
    if subprocess.call(cmd, cwd=pdir) != 0:
        print("perfbuild: link failed: " + " ".join(cmd), file=sys.stderr)
        return 1
    print(exe)
    return 0


if __name__ == "__main__":
    sys.exit(main())
