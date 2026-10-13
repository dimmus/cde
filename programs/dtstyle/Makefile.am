MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

bin_PROGRAMS = dtstyle

dtstyle_CPPFLAGS = $(CDE_CPPFLAGS) -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\"	\
	-DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" -I$(srcdir)/../dthelp	\
	-DCDE_DATA_TOP=\"$(CDE_DATA_TOP)\" \
	$(DT_INCDIR)

# Scalable (Xft) fonts in the Font dialog, when configure found fontconfig
# and Xft.  FontXft.c is always built; without DTSTYLE_HAVE_XFT the dialog
# offers bitmap fonts only.
if DTSTYLE_XFT
dtstyle_CPPFLAGS += -DDTSTYLE_HAVE_XFT $(XFT_CFLAGS)
DTSTYLE_XFT_LIBS = $(XFT_LIBS)
else
DTSTYLE_XFT_LIBS =
endif

dtstyle_LDADD = $(LIBHELP) $(LIBWIDGET) $(LIBSVC) $(XMLIB) $(DTSTYLE_XFT_LIBS) \
	$(XTLIB) $(X11LIB)

dtstyle_SOURCES = Main.c MainWin.c Font.c FontXft.c Audio.c Startup.c Mouse.c	\
		  SaveRestore.c Backdrop.c Screen.c Keyboard.c		\
		  ColorMain.c ColorPalette.c ColorFile.c ColorEdit.c	\
		  Resource.c Protocol.c Dtwm.c Help.c HelpCB.c		\
		  I18nMain.c I18nEnv.c I18nUtil.c TrueColor.c Audio.h ColorMain.h	\
		  Help.h Keyboard.h OWsync.h Screen.h Backdrop.h	\
		  ColorPalette.h I18nEnv.h Main.h Protocol.h		\
		  Startup.h ColorEdit.h Dtwm.h I18nMain.h MainWin.h	\
		  Resource.h ColorFile.h Font.h FontXft.h I18nUtil.h Mouse.h	\
		  TrueColor.h						\
		  SaveRestore.h

if SOLARIS
dtstyle_SOURCES += OWsync.c
endif

LOCAL_CPP_DEFINES = -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) \
	-DCDE_DATA_TOP=$(CDE_DATA_TOP)

CPP_TARGETS = Dtstyle Backdrops Palettes

BUILT_SOURCES = $(CPP_TARGETS)
CLEANFILES = $(CPP_TARGETS)

Dtstyle: Dtstyle.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

Backdrops: Backdrops.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

Palettes: Palettes.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@


