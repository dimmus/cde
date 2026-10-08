MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libWWL.a

libWWL_a_CXXFLAGS = -I../include

if SOLARIS
libWWL_a_CXXFLAGS += -DSYSV -DSVR4
endif

libWWL_a_SOURCES = WArgList.C      WCallback.C     WComposite.C \
		   WObject.C       WXmList.C       WXmMenu.C \
		   WXmRadioBox.C   WXmString.C
