MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = tradcpp

tradcpp_SOURCES = array.c directive.c files.c macro.h output.h		\
	utils.c array.h directive.h files.h main.c place.c utils.h	\
	bool.h eval.c inlinedefs.h mode.h place.h version.h config.h	\
	eval.h macro.c output.c union.h



# a build tool whose own malloc checking lives in assert()s: keep them
tradcpp_CFLAGS = $(CDE_CFLAGS) $(KEEP_ASSERTS)
