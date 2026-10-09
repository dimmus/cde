# CDE benchmarks: a standalone Makefile, not part of the automake build.
#
#   make -C tools/bench            build everything
#   make -C tools/bench run        run every benchmark once under Xvfb
#   make -C tools/bench clean
#
# The programs link against the libraries of this (configured and built)
# tree, through relative -L paths and an rpath to them, so that they
# measure the tree, not the installed CDE.  TOP can point at another
# built tree.

TOP      ?= ../..
CC       ?= cc
CFLAGS   ?= -O2 -g
WARN     := -Wall -Wextra
PKG      := pkg-config

XLIBS    := $(shell $(PKG) --libs x11)
XTST     := $(shell $(PKG) --libs xtst)
XTLIBS   := -lXm $(shell $(PKG) --libs xt xmu xext x11)
TIRPC    := $(shell $(PKG) --cflags libtirpc 2>/dev/null)

ABSTOP   := $(abspath $(TOP))
LIBDIRS  := $(ABSTOP)/lib/DtSvc/.libs $(ABSTOP)/lib/DtWidget/.libs \
	    $(ABSTOP)/lib/DtHelp/.libs $(ABSTOP)/lib/DtPrint/.libs \
	    $(ABSTOP)/lib/DtXinerama/.libs $(ABSTOP)/lib/tt/lib/.libs
RPATH    := $(foreach d,$(LIBDIRS),-L$(d) -Wl,-rpath,$(d))
empty    :=
space    := $(empty) $(empty)
LIBPATH  := $(subst $(space),:,$(strip $(LIBDIRS)))
INCS     := -I$(ABSTOP)/include $(TIRPC)

PROGS    := libcdebench_preload.so libdtwmbench_preload.so dtsvcbench \
	    ttbench dtwmbench cdebench-proxy

all: $(PROGS)

libcdebench_preload.so: preload.c benchutil.h
	$(CC) $(CFLAGS) $(WARN) -fPIC -shared -o $@ preload.c -ldl

libdtwmbench_preload.so: dtwmpreload.c dtwmbench.h
	$(CC) $(CFLAGS) $(WARN) -fPIC -shared -o $@ dtwmpreload.c -ldl

dtsvcbench: dtsvcbench.c benchutil.c benchutil.h
	$(CC) $(CFLAGS) $(WARN) $(INCS) -o $@ dtsvcbench.c benchutil.c \
		$(RPATH) -lDtWidget -lDtHelp -lDtPrint -lDtXinerama -lDtSvc \
		-ltt $(XTLIBS) -ldl

ttbench: ttbench.c benchutil.c benchutil.h
	$(CC) $(CFLAGS) $(WARN) $(INCS) -o $@ ttbench.c benchutil.c \
		$(RPATH) -ltt $(XTLIBS) -ldl

dtwmbench: dtwmbench.c dtwmbench.h
	$(CC) $(CFLAGS) $(WARN) \
		-DDTWMBENCH_WM='"$(ABSTOP)/programs/dtwm/.libs/dtwm"' \
		-DDTWMBENCH_LIBPATH='"$(LIBPATH)"' \
		-DDTWMBENCH_PRELOAD='"$(abspath .)/libdtwmbench_preload.so"' \
		-o $@ dtwmbench.c $(XTST) $(XLIBS)

cdebench-proxy: proxy.c
	$(CC) $(CFLAGS) $(WARN) -o $@ proxy.c

run: all
	./bench.py run -o baseline.json

clean:
	rm -f $(PROGS) *.o

.PHONY: all run clean
