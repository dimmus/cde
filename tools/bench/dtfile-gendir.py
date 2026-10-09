#!/usr/bin/env python3
#
# CDE benchmarks: generate a directory for dtfile.
#
# Licensed under the LGPL 2.1 license.
#
"""Generate a directory of N entries for the dtfile benchmarks.

  dtfile-gendir.py [-n N] DIR

DIR (created; must not exist or be empty) gets N entries (default 10000)
in the mix dtsvcbench types: 40% files with common suffixes (.c .h .txt
.html .ps .gif ...), 20% text files without a suffix (typed by content),
10% directories, 10% ELF executables, 10% symlinks to files and 10%
symlinks to directories.
"""

import argparse
import os
import sys

SUFFIXES = [".c", ".h", ".txt", ".html", ".ps", ".gif", ".tar", ".Z",
            ".sh", ".dt", ".xpm", ".pdf", ".sdl", ".tiff", ".man", ".o"]
TEXT = b"Plain text without a suffix, for the content rules.\n"
ELF = bytes([0x7f, ord("E"), ord("L"), ord("F"), 2, 1, 1, 0] + [0] * 8 +
            [2, 0, 0x3e, 0, 1, 0, 0, 0] + [0] * 40)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-n", type=int, default=10000)
    ap.add_argument("dir")
    a = ap.parse_args()
    os.makedirs(a.dir, exist_ok=True)
    if os.listdir(a.dir):
        sys.exit("dtfile-gendir: %s is not empty" % a.dir)
    for i in range(a.n):
        k = i % 10
        if k < 4:
            name = "file%06d%s" % (i, SUFFIXES[(i // 10) % len(SUFFIXES)])
            with open(os.path.join(a.dir, name), "wb") as f:
                f.write(TEXT)
        elif k < 6:
            with open(os.path.join(a.dir, "NOSUFFIX%06d" % i), "wb") as f:
                f.write(TEXT)
        elif k == 6:
            os.mkdir(os.path.join(a.dir, "dir%06d" % i))
        elif k == 7:
            p = os.path.join(a.dir, "prog%06d" % i)
            with open(p, "wb") as f:
                f.write(ELF)
            os.chmod(p, 0o755)
        elif k == 8:
            os.symlink("file%06d%s" % (i - 8, SUFFIXES[((i - 8) // 10) %
                                                    len(SUFFIXES)]),
                       os.path.join(a.dir, "link%06d" % i))
        else:
            os.symlink("dir%06d" % (i - 3),
                       os.path.join(a.dir, "dirlink%06d" % i))


if __name__ == "__main__":
    main()
