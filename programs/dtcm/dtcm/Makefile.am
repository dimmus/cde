MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtcm dtcm_delete dtcm_insert dtcm_lookup dtcm_editor

AM_CFLAGS = $(CDE_CFLAGS) $(DT_INCDIR) $(CSA_INCDIR) -I$(top_builddir)/lib/csa -I$(top_srcdir)/lib/csa	\
-I$(srcdir)/../libDtCmP

LDADD = ../libDtCmP/libDtCmP.a $(LIBCSA) $(LIBPRINT) $(LIBHELP) $(LIBWIDGET) $(LIBSVC) \
	$(LIBTT) $(XMLIB) $(XTLIB) $(X11LIB)

if FREEBSD
LDADD += -lcompat -lutil
endif

if NETBSD
LDADD += -lcompat
endif

dtcm_SOURCES = MonthPanel.c    RFCMIME.c       about.c \
               alarm.c         blist.c         browser.c \
               calendarA.c     cm_i18n.c       cmtt.c \
               datefield.c     dayglance.c     deskset.c \
               dnd.c           dssw.c          editor.c \
               find.c          fns.c           font.c \
               format.c        goto.c          graphics.c \
               group_editor.c  help.c          i18nxlate.c \
               icon.c          md5.c           misc.c \
               monthglance.c   namesvc.c       print.c \
               props.c         props_pu.c      reminders.c \
               rfp.c           select.c        session.c \
	       submit.c        tempbr.c        timezone.c \
               todo.c          weekglance.c    x_graphics.c \
               yearglance.c

dtcm_delete_SOURCES = cm_delete.c

dtcm_insert_SOURCES = cm_insert.c

dtcm_lookup_SOURCES = cm_lookup.c

dtcm_editor_SOURCES = datefield.c     deskset.c       dssw.c \
		      dtcm_editor.c   help.c          misc.c \
		      reminders.c     rfp.c

# rpc.cmsd keeps its calendars in _DtCMS_DEFAULT_DIR (server/log.h).
install-exec-hook:
	$(INSTALL) -m 0755 -d $(DESTDIR)/var/spool/calendar
