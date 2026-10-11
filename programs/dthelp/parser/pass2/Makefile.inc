# The pass2 pass of the help tag compiler.  Included by ../Makefile.am:
# paths are relative to programs/dthelp/parser (%reldir% is pass2).
#
#   build   reads the DTD and writes dtd.h, entity.h and arc.h (BDONE)
#   eltdef  reads the interface file and writes the action tables (EDONE)
#   htag2   the application code, a library
#   parser  the pass itself, dthelp_htag2
#
# Each of build, eltdef and parser also has the context tables that
# util/context makes from its context.dat (CDONE).  The generators write
# into their own directory, where they run; their objects are compiled
# with that directory first in the include path.

include %reldir%/build/Makefile.inc
include %reldir%/eltdef/Makefile.inc
include %reldir%/htag2/Makefile.inc
include %reldir%/parser/Makefile.inc

# headers, and util sources that the pass #includes
EXTRA_DIST += %reldir%/util
