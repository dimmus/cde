noinst_LTLIBRARIES += %reldir%/libHardCopy.la

%canon_reldir%_libHardCopy_la_CXXFLAGS = $(CDE_CXXFLAGS) -DEXPAND_TEMPLATES -DCDE_NEXT

%canon_reldir%_libHardCopy_la_SOURCES = %reldir%/FPset.C \
	%reldir%/HardCopyFP.C %reldir%/TemplatesAutoNumber.C \
	%reldir%/autoNumber.C %reldir%/autoNumberFP.C

noinst_HEADERS += %reldir%/FPset.h %reldir%/HardCopyFP.h \
	%reldir%/autoNumber.h %reldir%/autoNumberFP.h
