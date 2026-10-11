MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtmailpr

dtmailpr_CXXFLAGS = $(CDE_CXXFLAGS) -I../include -I../../dtcompat -I../../../. \
		    -I$(srcdir)/lib

dtmailpr_LDADD = ../libDtMail/libDtMail.a $(LIBSVC) $(LIBTT) $(LTLIBICONV)

dtmailpr_SOURCES = main.C message.C mailbox.C utils.C
