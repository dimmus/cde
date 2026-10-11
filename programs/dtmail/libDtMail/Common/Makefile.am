MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libCommon.a

libCommon_a_CXXFLAGS = $(CDE_CXXFLAGS) -I../../include -I../../include/utils -I$(srcdir)/lib \
		       -DDL_NOT_DYNAMIC -DUSE_SOCKSTREAM

if SOLARIS
libCommon_a_CXXFLAGS += -DMMAP_NORESERVE
endif

libCommon_a_SOURCES = APOPServer.C \
		      AUTOServer.C \
		      Buffer.C \
		      DetermineFileLocality.C \
		      DtMailBodyPart.C \
		      DtMailEnvelope.C \
		      DtMailError.C \
		      DtMailMailBox.C \
		      DtMailMessage.C \
		      DtMailRc.C \
		      DtMailServer.C \
		      DtMailTransport.C \
		      DtMailValuesBuiltin.C \
		      DtMailXtProc.C \
		      DtVirtArray.C \
		      DynamicLib.C \
		      FileShare.C \
		      HashTable.C \
		      IMAPServer.C \
		      IO.C \
		      LanguagePortability.C \
		      ObjectKey.C \
		      NotDynamic.C \
		      POP2Server.C \
		      POP3Server.C \
		      Process.C \
		      Session.C \
		      Threads.C \
		      str_utils.C

# The MD5 code is dtcm's (C, with C++ guards in md5.h).  The per-target
# CFLAGS name its object libCommon_a-md5.o, apart from dtcm's md5.o in the
# same directory.
libCommon_a_SOURCES += ../../../dtcm/dtcm/md5.c
libCommon_a_CFLAGS = $(CDE_CFLAGS)
