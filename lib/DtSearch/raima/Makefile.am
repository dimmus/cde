MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libraima.la

libraima_la_CFLAGS = -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
		     $(DT_INCDIR) -I$(top_srcdir)/include \
		     -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
		     -DNO_TRANS -DUNIX -D_POSIX_SOURCE -DMULTIBYTE

if SOLARIS
libraima_la_CFLAGS += -D_XOPEN_SOURCE=500
else
libraima_la_CFLAGS += -D_XOPEN_SOURCE=700
endif


libraima_la_SOURCES = alloc.c     cmtype.c   connect.c \
		      cotype.c    crget.c    crread.c    crset.c \
		      crtype.c    crwrite.c  csmget.c    csmread.c \
		      csmset.c    csmwrite.c  csoget.c   csoread.c   csoset.c \
		      csowrite.c \
		      dbacode.c   dbdpath.c   dbfpath.c  dblfcns.c \
		      dbswab.c    dbuserid.c \
		      delete.c    destroy.c   dio.c      discon.c    disdel.c \
		      fillnew.c   findco.c    findfm.c   findlm.c    findnm.c \
		      findpm.c \
		      initial.c   inittab.c   ismember.c \
		      isowner.c   keydel.c    keyexist.c keyfcns.c \
		      keyfind.c   keyfrst.c   keylast.c  keynext.c   keyprev.c \
		      keystore.c  libfcns.c   makenew.c  mapchar.c \
		      members.c   oflag.c    opens.c     options.c \
		      pathfcns.c  recfcns.c   recfrst.c  reclast.c   recnext.c \
		      recprev.c   recread.c   recset.c \
		      recwrite.c  renfile.c  rwcurr.c    setdb.c \
		      setmm.c     setmo.c     setmr.c    setom.c     setoo.c \
		      setor.c     setrm.c     setro.c    startup.c
