# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

noinst_LIBRARIES += %reldir%/libWidgets.a
%canon_reldir%_libWidgets_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS)
%canon_reldir%_libWidgets_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libWidgets_a_SOURCES = %reldir%/Pixmap.c %reldir%/Tree.c %reldir%/Panner.c \
	%reldir%/Simple.c %reldir%/Porthole.c %reldir%/XawInitFake.c \
	%reldir%/TabButton.c %reldir%/Restraint.c %reldir%/WXmOpButton.C
%canon_reldir%_libWidgets_a_AR = $(LD) -r -o
