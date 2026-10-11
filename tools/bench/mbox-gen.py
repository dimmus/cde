#!/usr/bin/env python3
#
# CDE benchmarks: generate a synthetic mbox for dtmail.
#
# Licensed under the LGPL 2.1 license.
#
"""Generate a synthetic mbox for the dtmail benchmarks.

  mbox-gen.py [-n N] [--multipart PCT] [--content-length] [--seed S] FILE

N messages (default 10000; the TODO asks for 1k, 10k and 50k), without
Content-Length headers unless --content-length, PCT percent (default 30)
of them multipart/mixed with a text part and a base64 attachment.  Bodies
contain "From " lines quoted as ">From " the way mbox requires, and the
dates go back one message per hour so that sorting has work to do.
"""

import argparse
import base64
import email.utils
import random
import time

WORDS = ("desktop window folder message calendar terminal action icon "
         "session workspace panel style help print mail file editor "
         "font color backdrop toolkit widget").split()


def body(rng, lines):
    out = []
    for _ in range(lines):
        out.append(" ".join(rng.choice(WORDS) for _ in range(rng.randint(5, 12))))
    if rng.random() < 0.1:
        out.append(">From the quoted line, as mbox wants it")
    return "\n".join(out) + "\n"


def message(rng, i, multipart, content_length):
    date = email.utils.formatdate(time.time() - 3600 * i, localtime=False)
    sender = "user%d@example.org" % rng.randint(1, 500)
    subject = "Message %d about the %s" % (i, rng.choice(WORDS))
    headers = [
        "From: %s" % sender,
        "To: bench@example.org",
        "Subject: %s" % subject,
        "Date: %s" % date,
        "Message-ID: <%d.%d@cdebench.example.org>" % (i, rng.randint(0, 1 << 30)),
        "MIME-Version: 1.0",
    ]
    if multipart:
        boundary = "cdebench-%d" % i
        att = base64.encodebytes(rng.randbytes(rng.randint(512, 8192))).decode()
        text = ("This is a multi-part message in MIME format.\n\n"
                "--%s\nContent-Type: text/plain; charset=us-ascii\n\n%s\n"
                "--%s\nContent-Type: application/octet-stream; name=\"data%d.bin\"\n"
                "Content-Transfer-Encoding: base64\n"
                "Content-Disposition: attachment; filename=\"data%d.bin\"\n\n%s\n"
                "--%s--\n" % (boundary, body(rng, rng.randint(3, 20)), boundary,
                              i, i, att, boundary))
        headers.append("Content-Type: multipart/mixed; boundary=\"%s\"" % boundary)
    else:
        text = body(rng, rng.randint(3, 40))
        headers.append("Content-Type: text/plain; charset=us-ascii")
    if content_length:
        headers.append("Content-Length: %d" % len(text.encode()))
    envelope = "From %s %s" % (sender, time.strftime(
        "%a %b %d %H:%M:%S %Y", time.gmtime(time.time() - 3600 * i)))
    return envelope + "\n" + "\n".join(headers) + "\n\n" + text + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-n", type=int, default=10000)
    ap.add_argument("--multipart", type=float, default=30)
    ap.add_argument("--content-length", action="store_true")
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("file")
    a = ap.parse_args()
    rng = random.Random(a.seed)
    with open(a.file, "w") as f:
        for i in range(a.n, 0, -1):
            f.write(message(rng, i, rng.random() * 100 < a.multipart,
                            a.content_length))


if __name__ == "__main__":
    main()
