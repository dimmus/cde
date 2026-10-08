MAINTAINERCLENAFILES = Makefile.in

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

bin_PROGRAMS = tttar

tttar_CXXFLAGS = -I../../lib $(TT_VERSION_DEFINE)

tttar_LDADD = ../../slib/libstt.a ../../mini_isam/libisam.a \
	$(LIBTT) $(TIRPCLIB) $(XTOOLLIB)

if SOLARIS
tttar_LDADD += -lintl -lsocket -lnsl
endif

tttar_SOURCES = tttar.C \
		archiver.C \
		tttar_utils.C \
		tttar_api.C \
		tttar_spec.C \
		tttar_file_utils.C \
		tttar_string_map.C
