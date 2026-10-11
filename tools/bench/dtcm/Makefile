# CDE benchmarks, dtcm: a standalone Makefile, not part of the automake
# build (like ../Makefile).  Links dtcmbench against libDtCmP.a and the
# libraries of this configured and built tree.
#
#   make -C tools/bench/dtcm
#   tools/bench/dtcm/run.sh          see README.md

TOP      ?= ../../..
CC       ?= cc
CFLAGS   ?= -O2 -g
WARN     := -Wall -Wextra -Wno-unused-parameter
PKG      := pkg-config

ABSTOP   := $(abspath $(TOP))
LIBDIRS  := $(ABSTOP)/lib/csa/.libs $(ABSTOP)/lib/DtSvc/.libs \
	    $(ABSTOP)/lib/tt/lib/.libs
RPATH    := $(foreach d,$(LIBDIRS),-L$(d) -Wl,-rpath,$(d))
INCS     := -DHAVE_CONFIG_H -I$(ABSTOP)/include -I$(ABSTOP)/include/Dt \
	    -I$(ABSTOP)/include/csa -I$(ABSTOP)/lib/csa \
	    -I$(ABSTOP)/programs/dtcm/libDtCmP \
	    -D_DEFAULT_SOURCE -DOPT_TIRPC \
	    $(shell $(PKG) --cflags libtirpc 2>/dev/null)
DTCMP    := $(ABSTOP)/programs/dtcm/libDtCmP/libDtCmP.a

all: dtcmbench

dtcmbench: dtcmbench.c ../benchutil.c ../benchutil.h $(DTCMP)
	$(CC) -std=gnu11 $(CFLAGS) $(WARN) $(INCS) -o $@ dtcmbench.c \
		../benchutil.c $(DTCMP) $(RPATH) -lcsa -lDtSvc -ltt \
		$(shell $(PKG) --libs xt x11 libtirpc) -ldl

clean:
	rm -f dtcmbench

.PHONY: all clean
