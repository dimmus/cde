#!/usr/bin/env python3
#
# CDE benchmarks: terminal output streams in the manner of vtebench.
#
# Licensed under the LGPL 2.1 license.
#
"""Write a vtebench-style terminal output stream to a file.

  vtgen.py [-c COLS] [-r ROWS] [-s SCREENS] CASE FILE

Cases (after alacritty's vtebench benchmarks):
  dense_cells            every cell rewritten each screen, each with its
                         own 256-colour foreground and background
  scrolling              full-width lines of text, scrolling the screen
  scrolling_in_region_top     lines scrolling inside a region (DECSTBM)
  scrolling_in_region_bottom  that leaves the first / the last line out
  unicode                lines of multi-byte UTF-8: accented Latin,
                         Greek, Cyrillic and box drawing (no wide chars)
  cursor_motion          a character at a pseudo-random position (CUP),
                         COLS*ROWS of them per screen

SCREENS (default 200) screens' worth of output are written: a screen is
ROWS lines of output, or COLS*ROWS cells.  Prints one JSON object: the
case, bytes, screens and the bytes of one screen (the burst size that
dtterm-vtebench.sh writes with).
"""

import argparse
import json
import random
import sys

WORDS = ("help action style message widget font color calendar terminal "
         "panel print session workspace desktop folder editor file icon "
         "backdrop window").split()
UNI = ("àéîõüçñ ΑΒΓΔΕΖΗΘ αβγδεζηθ АБВГДЕЖЗ абвгдежз ─│┌┐└┘├┤┬┴┼ "
       "ÀÉÎÕÜÇÑ λμνξοπρσ ийклмноп ═║╔╗╚╝╠╣╦╩╬")


def text_line(rnd, cols, alphabet=None):
    out = []
    n = 0
    while True:
        w = rnd.choice(WORDS) if alphabet is None else \
            "".join(rnd.choice(alphabet) for _ in range(rnd.randint(3, 9)))
        if n + len(w) + 1 > cols:
            break
        out.append(w)
        n += len(w) + 1
    return " ".join(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-c", "--cols", type=int, default=80)
    ap.add_argument("-r", "--rows", type=int, default=24)
    ap.add_argument("-s", "--screens", type=int, default=200)
    ap.add_argument("case")
    ap.add_argument("file")
    a = ap.parse_args()
    rnd = random.Random(1)
    cols, rows = a.cols, a.rows
    chunks = []
    screen_bytes = 0
    if a.case == "dense_cells":
        for s in range(a.screens):
            buf = ["\x1b[H"]
            for r in range(rows):
                for c in range(cols):
                    buf.append("\x1b[38;5;%dm\x1b[48;5;%dm%c" %
                               ((s + r + c) % 256, (s * 7 + r * 3 + c) % 256,
                                33 + (s + r * cols + c) % 94))
            buf.append("\x1b[0m")
            chunks.append("".join(buf))
    elif a.case == "scrolling":
        lines = [text_line(rnd, cols) for _ in range(rows * 4)]
        for s in range(a.screens):
            chunks.append("".join(lines[(s * rows + i) % len(lines)] + "\r\n"
                                  for i in range(rows)))
    elif a.case in ("scrolling_in_region_top", "scrolling_in_region_bottom"):
        top, bot = (2, rows) if a.case.endswith("top") else (1, rows - 1)
        lines = [text_line(rnd, cols) for _ in range(rows * 4)]
        head = "\x1b[%d;%dr\x1b[%d;1H" % (top, bot, bot)
        n = bot - top + 1
        for s in range(a.screens):
            chunks.append((head if s == 0 else "") +
                          "".join(lines[(s * n + i) % len(lines)] + "\r\n"
                                  for i in range(n)))
        chunks.append("\x1b[r")
    elif a.case == "unicode":
        alphabet = [ch for ch in UNI if ch != " "]
        lines = [text_line(rnd, cols, alphabet) for _ in range(rows * 4)]
        for s in range(a.screens):
            chunks.append("".join(lines[(s * rows + i) % len(lines)] + "\r\n"
                                  for i in range(rows)))
    elif a.case == "cursor_motion":
        for s in range(a.screens):
            buf = []
            for i in range(cols * rows):
                buf.append("\x1b[%d;%dH%c" % (rnd.randint(1, rows),
                                               rnd.randint(1, cols),
                                               33 + (s + i) % 94))
            chunks.append("".join(buf))
    else:
        ap.error("unknown case " + a.case)
    data = [c.encode("utf-8") for c in chunks]
    screen_bytes = max(len(d) for d in data)
    with open(a.file, "wb") as f:
        for d in data:
            f.write(d)
    print(json.dumps({"case": a.case, "bytes": sum(len(d) for d in data),
                      "screens": a.screens, "screen_bytes": screen_bytes,
                      "cols": cols, "rows": rows}))
    return 0


if __name__ == "__main__":
    sys.exit(main())
