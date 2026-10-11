MAINTAINERCLEANFILES = Makefile.in

# The help tag compiler: dthelptag (helptag/) runs three passes, each a
# program generated from a DTD and an interface file by tools built on
# the way (see pass1/Makefile.inc).  All of it is built from this one
# Makefile (each directory's Makefile.inc, included below) rather than by
# a make per directory, so that make sees the whole compiler at once: the
# three passes build in parallel, the generated tables are made once,
# before the objects that include them and only when their inputs
# change, and a no-op build reads one Makefile instead of eighteen.
# Paths in the Makefile.inc files are relative to this directory.
#
# Each object is compiled with the include path that the make in its
# directory gave it: its own directory first (where the generated tables
# are), then include/.  nostdinc turns off automake's own
# -I. -I$(top_builddir)/include.
AUTOMAKE_OPTIONS = nostdinc

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

HELP_CPPFLAGS = -I$(top_builddir)/include $(CDE_CPPFLAGS)
HELP_TOP_DEFINES = \
	-DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
	-DCDE_DATA_TOP=\"$(CDE_DATA_TOP)\" \
	-DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
	-DCDE_LOGFILES_TOP=\"$(CDE_LOGFILES_TOP)\"

HELPUTILLIB = util/libhelputil.a
HELPCONTEXT = util/context$(EXEEXT)

# util/context, run in the directory of its context.dat (it reads and
# writes fixed names there).  In a VPATH build that context.dat is in the
# source directory, and the CDONE rules copy it for the run.
RUN_CONTEXT = $(abs_builddir)/$(HELPCONTEXT)

noinst_LIBRARIES =
noinst_PROGRAMS =
bin_PROGRAMS =
bin_SCRIPTS =
CLEANFILES =
EXTRA_DIST =

include util/Makefile.inc
include pass1/Makefile.inc
include canon1/Makefile.inc
include pass2/Makefile.inc
include helptag/Makefile.inc
