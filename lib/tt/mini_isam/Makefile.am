MAINTAINERCLEANFILES = Makefile.in

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

noinst_LIBRARIES= libisam.a

libisam_a_CFLAGS = $(TT_VERSION_DEFINE)

libisam_a_SOURCES = isaddindex.c isamaddindex.c isopen.c isamopen.c \
             isdelcurr.c isdelrec.c isamdelrec.c isbuild.c \
             isvars.c isrewrec.c iserase.c iscntl.c isstart.c \
             isaddprimary.c isrewcurr.c isclose.c isread.c \
             iswrite.c isamwrite.c issync.c isapplmr.c ismngfcb.c \
             issort.c ischange1key.c isfab.c isdiskbufs2.c isfd.c isbtree.c \
             isdel1key.c isfcb.c isfixrec.c issignals.c isapplmw.c \
             isfcbwatchfd.c isamhooks.c isfcbindex.c isalloc.c isbytearray.c \
             isdatconv.c isminmax.c isfname.c isvarrec.c isperm.c iserror.c \
             iskeycmp.c iskeycalc.c iskeyvalid.c isamerror.c iskeyaux.c \
             isfileio.c ispageio.c isdlink.c isbtree2.c \
             isbtree3.c isindfreel.c isindexconv.c isbsearch.c isadd1key.c \
             iskeyconv.c isversion.c isrepair.c iswrrec.c isrename.c \
	     isindexinfo.c isgarbage.c iscurpos.c
