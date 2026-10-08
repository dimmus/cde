MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libDtSvc.la

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtSvc_la_LDFLAGS = -version-info 2:1:0
else
libDtSvc_la_LDFLAGS = -version-info 3:0:1
endif


# USe CPPFLAGS so it will apply to both C and C++ components
libDtSvc_la_CPPFLAGS = -I./include -DMULTIBYTE -DNLS16 -I../include \
	@DT_INCDIR@ -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
       -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\"

if SOLARIS
libDtSvc_la_CPPFLAGS += -DNO_REGCOMP
endif

# OSVERSION hackery. This database works by comparing a number
# computed with the OS major/minor versions to determine appropriate
# entries in the Xlocale DB.  These are currently hardcoded per OS in
# configure.ac.  Well, no more.  Now they will be hardcoded per OS
# here - the only place where they are really needed currently, until
# some time in the future when old and irrelevant entries are removed
# from the DB and we won't need this at all.

if LINUX
libDtSvc_la_CPPFLAGS += -DOSMAJORVERSION=4 -DOSMINORVERSION=15
endif

if FREEBSD
libDtSvc_la_CPPFLAGS += -DOSMAJORVERSION=10 -DOSMINORVERSION=0
endif

if OPENBSD
libDtSvc_la_CPPFLAGS += -DOSMAJORVERSION=6 -DOSMINORVERSION=2
endif

if NETBSD
libDtSvc_la_CPPFLAGS += -DOSMAJORVERSION=8 -DOSMINORVERSION=0
endif

if SOLARIS
libDtSvc_la_CPPFLAGS += -DOSMAJORVERSION=5 -DOSMINORVERSION=10
endif


libDtSvc_la_SOURCES = 	DtCodelibs/buf.C \
	DtCodelibs/buf.h \
	DtCodelibs/filegen.C \
	DtCodelibs/mbschr.C \
	DtCodelibs/pathcollapse.C \
	DtCodelibs/privbuf.C \
	DtCodelibs/shellscan.C \
	DtCodelibs/strcase.C \
	DtCodelibs/strend.C \
	DtCodelibs/strhash.C \
	DtCodelibs/stringio.C \
	DtCodelibs/stringio.h \
	DtCodelibs/strtokx.C \
	DtCodelibs/strwcmp.C \
	DtEncap/bmsglob.c \
	DtEncap/connect.c \
	DtEncap/local.c \
	DtEncap/MemoryMgr.c \
	DtEncap/nls.c \
	DtEncap/noio.c \
	DtEncap/pathwexp.c \
	DtEncap/pipe.c \
	DtEncap/pty.c \
	DtEncap/remote.c \
	DtEncap/SbEvent.c \
	DtEncap/sbstdinc.c \
	DtEncap/scoop.c \
	DtEncap/spc.c \
	DtEncap/spc-env.c \
	DtEncap/spc-error.c \
	DtEncap/spc-exec.c \
	DtEncap/spc-net.c \
	DtEncap/spc-obj.c \
	DtEncap/spc-proto.c \
	DtEncap/spc-sm.c \
	DtEncap/spc-termio.c \
	DtEncap/spc-util.c \
	DtEncap/spc-xt.c \
	DtEncap/stringbuf.c \
	DtEncap/Symbolic.c \
	DtEncap/usersig.c \
	DtUtil1/Action.c \
	DtUtil1/ActionDb.c \
	DtUtil1/ActionFind.c \
	DtUtil1/ActionTt.c \
	DtUtil1/ActionUtil.c \
	DtUtil1/CmdMain.c \
	DtUtil1/CmdProcess.c \
	DtUtil1/CmdSpc.c \
	DtUtil1/CmdUtilityP.c \
	DtUtil1/DbLoad.c \
	DtUtil1/DbReader.c \
	DtUtil1/DbUtil.c \
	DtUtil1/DndBuff.c \
	DtUtil1/Dnd.c \
	DtUtil1/DndDrag.c \
	DtUtil1/DndDrop.c \
	DtUtil1/DndFile.c \
	DtUtil1/DndIcon.c \
	DtUtil1/DndText.c \
	DtUtil1/Dt.c \
	DtUtil1/DtHash.c \
	DtUtil1/Dts.c \
	DtUtil1/DtsDb.c \
	DtUtil1/DtsInit.c \
	DtUtil1/DtsMM.c \
	DtUtil1/DtsSort.c \
	DtUtil1/GetMwmW.c \
	DtUtil1/GetVWmHint.c \
	DtUtil1/intarray.c \
	DtUtil1/inttab.c \
	DtUtil1/MMDb.c \
	DtUtil1/myassertP.h \
	DtUtil1/Qualify.c \
	DtUtil1/Saver.c \
	DtUtil1/SetVWmHint.c \
	DtUtil1/SmComm.c \
	DtUtil1/SmUtil.c \
	DtUtil1/strtab.c \
	DtUtil1/WmAddDelWs.c \
	DtUtil1/WmBackWin.c \
	DtUtil1/WmChBackD.c \
	DtUtil1/WmGWsInfo.c \
	DtUtil1/WmGWsList.c \
	DtUtil1/WmMarquee.c \
	DtUtil1/WmRequest.c \
	DtUtil1/WmRestart.c \
	DtUtil1/WmRmWsFcn.c \
	DtUtil1/WmSetTitle.c \
	DtUtil1/WmSetWs.c \
	DtUtil1/WmWsCallB.c \
	DtUtil1/WmWsHints.c \
	DtUtil1/WmWsOccupy.c \
	DtUtil2/ActIndicator.c \
	DtUtil2/addToRes.c \
	DtUtil2/bitmaps.h \
	DtUtil2/ChkpntClient.c \
	DtUtil2/ChkpntListen.c \
	DtUtil2/CmdUtility.c \
	DtUtil2/DtEnvMap.c \
	DtUtil2/DtGetMessage.c \
	DtUtil2/DtNlUtils.c \
	DtUtil2/DtUtil.c \
	DtUtil2/EnvControl.c \
	DtUtil2/FileUtil.c \
	DtUtil2/GetDispRes.c \
	DtUtil2/GetEmbed.c \
	DtUtil2/GetMessage.c \
	DtUtil2/Hash.c \
	DtUtil2/HourGlass.c \
	DtUtil2/Info.c \
	DtUtil2/LocaleXlate.c \
	DtUtil2/lock.c \
	DtUtil2/MsgCat.c \
	DtUtil2/MsgLog.c \
	DtUtil2/PrintXErr.c \
	DtUtil2/SharedProcs.c \
	DtUtil2/SmCreateDirs.c \
	DtUtil2/SunDtHelp.c \
	DtUtil2/SvcTT.c \
	DtUtil2/UErrNoBMS.c \
	DtUtil2/Utility.c \
	DtUtil2/XlationSvc.c \
	DtUtil2/XmWrap.c

if HAS_PAM_LIBRARY
libDtSvc_la_SOURCES += DtUtil2/SvcPam.c
libDtSvc_la_LIBADD = $(PAMLIB)
endif
