MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtrandr

DTRANDR_DEFINES = -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
	-DDTRANDR_VERSION=\"$(PACKAGE_VERSION)\"

# Everything but main.c and the X backend: the logic (profiles, matching,
# layout, the order of applying) and the fake display of the tests, which
# the unit test links without X
LOGIC_SOURCES = dtrandr.h backend.h apply.c layout.c profile.c util.c \
	fake.c

dtrandr_CPPFLAGS = $(CDE_CPPFLAGS) $(DTRANDR_DEFINES)
dtrandr_SOURCES = main.c xbackend.c $(LOGIC_SOURCES)
dtrandr_LDADD = $(XRANDRLIB) $(X11LIB) $(MATHLIB)

# A troff page here: the other man pages are made from the DocBook
# sources in doc/
dist_man1_MANS = dtrandr.1

# make check: unit tests of the logic, the program against a fake
# display (tests/fake/), and against Xvfb and Xephyr if they are there
check_PROGRAMS = tests/unit tests/xsetedid
tests_unit_SOURCES = tests/unit.c $(LOGIC_SOURCES)
tests_unit_LDADD = $(MATHLIB)
tests_xsetedid_SOURCES = tests/xsetedid.c
tests_xsetedid_LDADD = $(XRANDRLIB) $(X11LIB)

TESTS = tests/unit tests/fake-tests.sh tests/x-tests.sh
AM_TESTS_ENVIRONMENT = \
	DTRANDR=$(abs_builddir)/dtrandr; \
	XSETEDID=$(abs_builddir)/tests/xsetedid; \
	srcdir=$(srcdir); \
	export DTRANDR XSETEDID srcdir;

EXTRA_DIST = tests/fake-tests.sh tests/x-tests.sh tests/lib.sh \
	tests/fake tests/profiles

clean-local:
	-rm -rf tests/work
