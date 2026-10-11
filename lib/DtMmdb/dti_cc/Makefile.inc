noinst_LTLIBRARIES += %reldir%/libdti_cc.la

%canon_reldir%_libdti_cc_la_CXXFLAGS = $(CDE_CXXFLAGS)

%canon_reldir%_libdti_cc_la_SOURCES = %reldir%/CC_Dlist.C \
	%reldir%/CC_Listbase.C %reldir%/CC_Slist.C %reldir%/CC_Stack.C \
	%reldir%/CC_String.C %reldir%/CC_Tokenizer.C %reldir%/cc_exceptions.C \
	%reldir%/cc_hdict.C %reldir%/cc_povec.C %reldir%/cc_pvect.C \
	%reldir%/cc_vvect.C %reldir%/hdictinit.C %reldir%/vectinit.C

noinst_HEADERS += %reldir%/CC_Stack.h %reldir%/CC_String.h \
	%reldir%/CC_Tokenizer.h %reldir%/cc_exceptions.h %reldir%/cc_povec.h \
	%reldir%/cc_pvect.h
