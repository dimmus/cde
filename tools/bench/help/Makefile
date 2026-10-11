# CDE benchmarks, help: a standalone Makefile, not part of the automake
# build (like ../Makefile).  Links helpbench against libDtHelp,
# mmdbbench against libDtMmdb and srbench against libDtSearch, of this
# configured and built tree.
#
#   make -C tools/bench/help          (also builds ../libcdebench_preload.so)
#   tools/bench/help/run.py           see README.md

TOP      ?= ../../..
CC       ?= cc
CFLAGS   ?= -O2 -g
WARN     := -Wall -Wextra
PKG      := pkg-config

ABSTOP   := $(abspath $(TOP))
LIBDIRS  := $(ABSTOP)/lib/DtHelp/.libs $(ABSTOP)/lib/DtSvc/.libs \
	    $(ABSTOP)/lib/DtPrint/.libs $(ABSTOP)/lib/DtXinerama/.libs \
	    $(ABSTOP)/lib/tt/lib/.libs
RPATH    := $(foreach d,$(LIBDIRS),-L$(d) -Wl,-rpath,$(d))
INCS     := -I$(ABSTOP)/include
XTLIBS   := -lXm $(shell $(PKG) --libs xt xext x11)

CXX      ?= c++
MMDBINC  := -I$(ABSTOP)/lib/DtMmdb -I$(ABSTOP)/include -DNO_DB_LOCK
MMDBLIB  := -L$(ABSTOP)/lib/DtMmdb/.libs -Wl,-rpath,$(ABSTOP)/lib/DtMmdb/.libs

all: helpbench libhelpbench_preload.so mmdbbench srbench \
	../libcdebench_preload.so

srbench: srbench.c
	$(CC) -std=gnu11 $(CFLAGS) $(WARN) $(INCS) -o $@ srbench.c \
		-L$(ABSTOP)/lib/DtSearch/.libs \
		-Wl,-rpath,$(ABSTOP)/lib/DtSearch/.libs -lDtSearch -ldl

mmdbbench: mmdbbench.C
	$(CXX) $(CFLAGS) -Wall $(MMDBINC) -o $@ mmdbbench.C $(MMDBLIB) \
		-lDtMmdb

helpbench: helpbench.c helpbench.h ../benchutil.c ../benchutil.h
	$(CC) -std=gnu11 $(CFLAGS) $(WARN) $(INCS) -rdynamic -o $@ helpbench.c \
		../benchutil.c $(RPATH) -lDtHelp -lDtSvc $(XTLIBS) -ldl

libhelpbench_preload.so: helppreload.c helpbench.h
	$(CC) -std=gnu11 $(CFLAGS) $(WARN) -fPIC -shared -o $@ \
		helppreload.c -ldl -pthread

../libcdebench_preload.so:
	$(MAKE) -C .. libcdebench_preload.so

clean:
	rm -f helpbench libhelpbench_preload.so mmdbbench srbench

.PHONY: all clean
