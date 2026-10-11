MAINTAINERCLEANFILES = Makefile.in

include ../dtdocbook.am

# Wrappers that run this tree's DocBook tools (dtdocbook2infolib, the
# infolib programs, instant) on this tree's DocBook data, for the doc/
# build.  The installed tools find both through the environment variables
# dtdcbk_libexecdir and dtdcbk_datarootdir, so nothing here is compiled
# again with build-tree paths in its flags.

noinst_SCRIPTS = dtdocbook2infolib dtdocbook2man dtdocbook2sdl
CLEANFILES = $(noinst_SCRIPTS) $(noinst_SCRIPTS:=.tmp)

dtdcbk_datarootdir = $(abs_top_srcdir)/programs/dtdocbook
dtdcbk_libexecdir = $(abs_top_builddir)/programs/dtdocbook
dtdcbk_env = dtdcbk_libdir=$(dtdcbk_datarootdir) \
	     dtdcbk_libexecdir=$(dtdcbk_libexecdir) \
	     dtdcbk_datarootdir=$(dtdcbk_datarootdir)

dtdocbook2infolib: ../dtdocbook2infolib$(EXEEXT) Makefile
	echo "#!/bin/sh" > $@.tmp
	echo "$(dtdcbk_env) exec $(dtdcbk_libexecdir)/dtdocbook2infolib \"\$$@\"" >> $@.tmp
	chmod +x $@.tmp && mv -f $@.tmp $@

dtdocbook2man: ../dtdocbook2man Makefile
	echo "#!/bin/sh" > $@.tmp
	echo "$(dtdcbk_env) exec sh $(dtdcbk_libexecdir)/dtdocbook2man \"\$$@\"" >> $@.tmp
	chmod +x $@.tmp && mv -f $@.tmp $@

dtdocbook2sdl: ../dtdocbook2sdl Makefile
	echo "#!$(KSH)" > $@.tmp
	echo "$(dtdcbk_env) exec $(KSH) $(dtdcbk_libexecdir)/dtdocbook2sdl \"\$$@\"" >> $@.tmp
	chmod +x $@.tmp && mv -f $@.tmp $@
