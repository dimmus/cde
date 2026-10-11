noinst_LTLIBRARIES += %reldir%/libtttk.la

%canon_reldir%_libtttk_la_CXXFLAGS = $(CDE_CXXFLAGS) $(TT_VERSION_DEFINE)

%canon_reldir%_libtttk_la_SOURCES = %reldir%/ttdesktop.C \
	%reldir%/ttdtfile.C %reldir%/ttdtprocid.C %reldir%/ttmedia.C \
	%reldir%/tttk.C %reldir%/tttk2free.C %reldir%/tttkmessage.C \
	%reldir%/tttkpattern.C %reldir%/tttkutils.C
