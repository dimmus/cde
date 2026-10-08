MAINTAINERCLEANFILES = Makefile.in

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

BUILT_SOURCES = mp_types_gram.h
CLEANFILES = mp_types_gram.h

mp_types_gram.h:
	$(RM) mp_types_gram.h
	$(LN_S) frozen.mp_types_gram.h mp_types_gram.h

bin_PROGRAMS = tt_type_comp

tt_type_comp_CXXFLAGS = -I../../lib -I../../slib \
			$(TT_VERSION_DEFINE) -DCPP_PROGRAM="\"$(CPP_PROGRAM)\""

tt_type_comp_SOURCES = mp_type_comp.C        mp_types_table.C \
		frozen.mp_types_lex.C frozen.mp_types_gram.C

tt_type_comp_LDADD = ../../slib/libstt.a $(LIBTT) $(TIRPCLIB) $(XTOOLLIB)

if SOLARIS
tt_type_comp_LDADD += $(XTOOLLIB) -lintl -lsocket -lnsl
endif

frozen.mp_types_lex.o: frozen.mp_types_gram.h
