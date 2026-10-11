#!/usr/bin/env python3
# CDE benchmarks, help: the input of mksrdb.sh.
#
#   mksrdb.py OPHUF_C DIR RECORDS
#
# DIR/ophuf.huf: the Huffman encode table matching the decode tree in
# libDtSearch's ophuf.c (dtsrload compresses the documents with it, the
# engine decompresses them with ophuf.c).  DIR/TESTDB.fzk: RECORDS
# documents of 150-600 words from a 36-word vocabulary (fixed seed), in
# the .fzk format of dtsrload and dtsrindex.
import random
import re
import sys

src = open(sys.argv[1]).read()
hid = int(re.search(r'hctree_id\s*=\s*(\d+)', src).group(1))
root = int(re.search(r'hctree_root\s*=\s*(\d+)', src).group(1))
arr = src[src.index('hctree_array[]'):]
arr = arr[arr.index('{') + 1:arr.index('}')]
arr = re.sub(r'/\*.*?\*/', '', arr, flags=re.S)
vals = [int(x) for x in arr.replace('\n', ' ').split(',') if x.strip()]
codes = {}


def walk(node, prefix):
    for bit, child in ((0, vals[2 * node]), (1, vals[2 * node + 1])):
        if child < 0:
            codes[child + 257] = prefix + str(bit)    # leaf: char + 257
        else:
            walk(child, prefix + str(bit))


walk(root, '')
with open(sys.argv[2] + "/ophuf.huf", "w") as f:
    f.write("%d\tHCTREE_ID\n" % hid)
    for c in range(257):
        if c in codes:
            f.write("%d\t%s\t1\tx\n" % (c, codes[c]))
        else:
            f.write("%d\t \t0\tx\n" % c)     # literal coded (or unused)

random.seed(7)
vocab = ["alpha", "bravo", "charlie", "delta", "echo", "foxtrot", "golf",
         "hotel", "india", "juliet", "kilo", "lima", "mike", "november",
         "oscar", "papa", "quebec", "romeo", "sierra", "tango", "uniform",
         "victor", "whiskey", "xray", "yankee", "zulu", "window", "menu",
         "file", "manager", "editor", "text", "search", "volume", "print",
         "style"]
with open(sys.argv[2] + "/TESTDB.fzk", "w") as f:
    for r in range(int(sys.argv[3])):
        words = [random.choice(vocab) for _ in range(random.randint(150, 600))]
        # fzkey, abstract, key (keytype D), date; the text; ETX
        f.write("\nABSTRACT: Abstract of record %d\nD%05d\n\n" % (r, r))
        for i in range(0, len(words), 12):
            f.write(" ".join(words[i:i + 12]) + "\n")
        f.write("\f\n")
