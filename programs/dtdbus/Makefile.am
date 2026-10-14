MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtdbus

dtdbus_CPPFLAGS = $(CDE_CPPFLAGS) $(SDBUS_CFLAGS) \
	-DDTDBUS_VERSION=\"$(PACKAGE_VERSION)\"
dtdbus_SOURCES = dtdbus.c
dtdbus_LDADD = $(LIBTT) $(SDBUS_LIBS) $(XMLIB) $(XTLIB) -lXss $(X11LIB)

# A troff page here, like dtrandr's
dist_man1_MANS = dtdbus.1
