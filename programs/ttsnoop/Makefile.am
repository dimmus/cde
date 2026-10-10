MAINTAINERCLEANFILES = Makefile.in

appdefaultsdir = $(CDE_INSTALLATION_TOP)/app-defaults/C

dist_appdefaults_DATA = Ttsnoop

AB_MAKEFILES = \
	Makefile.aix \
	Makefile.sunos \
	Makefile.linux \
	Makefile.freebsd \
	Makefile.netbsd \
	Makefile.openbsd

DTCODEGENCAT = NLSPATH=$(top_builddir)/programs/dtappbuilder/src/abmf/dtcodegen.cat

DTCODEGEN = $(DTCODEGENCAT) $(top_builddir)/programs/dtappbuilder/src/abmf/dtcodegen

DTCODEGEN_SCRATCH = $(SHELL) $(top_srcdir)/build-aux/dtcodegen-scratch \
	"NLSPATH=$(abs_top_builddir)/programs/dtappbuilder/src/abmf/dtcodegen.cat $(abs_top_builddir)/programs/dtappbuilder/src/abmf/dtcodegen"

CLEANFILES = Ttsnoop Ttsnoop.tmp .dtcodegen.log *.BAK *.delta *.patched \
	     $(AB_MAKEFILES) *_stubs.C *_ui.c *_ui.h ttsnoop.h dtb_utils.c \
	     dtb_utils.h

# The generator rules below each write several files.  They are grouped
# targets ("&:", GNU make 4.3 or later), so a parallel make runs each
# recipe once.  Older GNU makes have no grouped targets and would run the
# recipe once per output, concurrently: build serially there.
$(if $(filter grouped-target,$(.FEATURES)),,$(eval .NOTPARALLEL:))

.DELETE_ON_ERROR:

bin_PROGRAMS = ttsnoop

ttsnoop_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB) $(LIBTERM)

BUILT_SOURCES = ttsnoop_ui.h apiTracer_ui.h ttChooser_ui.h patternProps_ui.h \
		stringChooser_ui.h messageProps_ui.h sessionChooser_ui.h \
		fileChooser_ui.h argChooser_ui.h callbackChooser_ui.h \
		ttsnoop.h dtb_utils.h

ttsnoop_SOURCES = stringChooser_stubs.C ttChooser_stubs.C \
		  ttsnoop_ui.c 		apiTracer_stubs.C \
	    	  argChooser_stubs.C 	callbackChooser_stubs.C \
		  fileChooser_stubs.C 	messageProps_stubs.C \
	       	  patternProps_stubs.C 	sessionChooser_stubs.C \
		  ttsnoop.C 		ttsnoop_stubs.C \
	  	  apiTracer_ui.c	argChooser_ui.c \
		  callbackChooser_ui.c  dtb_utils.c \
		  fileChooser_ui.c      messageProps_ui.c \
		  patternProps_ui.c     sessionChooser_ui.c \
		  stringChooser_ui.c    ttChooser_ui.c \
	 	  tt_c++.C 		DtTt.C

ttsnoop_ui.c ttsnoop_ui.h ttsnoop_stubs.C &: ttsnoop.bip ttsnoop.bil ttsnoop_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i ttsnoop_stubs.C.src=ttsnoop_stubs.c \
		ttsnoop_ui.c ttsnoop_ui.h ttsnoop_stubs.c=ttsnoop_stubs.C \
		-- -changed -merge -p ttsnoop.bip ttsnoop.bil
	cat externC.h ttsnoop_ui.h closeBrace.h > ttsnoop_ui.h.patched
	mv ttsnoop_ui.h.patched ttsnoop_ui.h
	$(SED) \
	-e 's/^DtbTt/extern String snooperArgv[];DtbTt/' \
	-e 's/XmNheight, 257,//' \
	-e 's/XmNwidth, 580,//' \
	-e 's/XmNcolumns, 80,//' \
	-e 's/XmNrows, 24/DtNsubprocessArgv, snooperArgv/' \
	ttsnoop_ui.c > ttsnoop_ui.c.patched
	mv ttsnoop_ui.c.patched ttsnoop_ui.c

apiTracer_ui.c apiTracer_ui.h apiTracer_stubs.C &: ttsnoop.bip apiTracer.bil apiTracer_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i apiTracer_stubs.C.src=apiTracer_stubs.c \
		apiTracer_ui.c apiTracer_ui.h apiTracer_stubs.c=apiTracer_stubs.C \
		-- -changed -merge -p ttsnoop.bip apiTracer.bil
	cat externC.h apiTracer_ui.h closeBrace.h > apiTracer_ui.h.patched
	mv apiTracer_ui.h.patched apiTracer_ui.h
	$(SED) -e 's/^DtbApi/extern String apiTracerArgv[];DtbApi/' \
	-e 's/XmNrows, 24/XmNrows, 24, DtNsubprocessArgv, apiTracerArgv/' \
	apiTracer_ui.c > apiTracer_ui.c.patched
	mv apiTracer_ui.c.patched apiTracer_ui.c

ttChooser_ui.c ttChooser_ui.h ttChooser_stubs.C &: ttsnoop.bip ttChooser.bil ttChooser_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i ttChooser_stubs.C.src=ttChooser_stubs.c \
		ttChooser_ui.c ttChooser_ui.h ttChooser_stubs.c=ttChooser_stubs.C \
		-- -changed -merge -p ttsnoop.bip ttChooser.bil
	cat externC.h ttChooser_ui.h closeBrace.h > ttChooser_ui.h.patched
	mv ttChooser_ui.h.patched ttChooser_ui.h

patternProps_ui.c patternProps_ui.h patternProps_stubs.C &: ttsnoop.bip patternProps.bil patternProps_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i patternProps_stubs.C.src=patternProps_stubs.c \
		patternProps_ui.c patternProps_ui.h patternProps_stubs.c=patternProps_stubs.C \
		-- -changed -merge -p ttsnoop.bip patternProps.bil
	cat externC.h patternProps_ui.h closeBrace.h > patternProps_ui.h.patched
	mv patternProps_ui.h.patched patternProps_ui.h

stringChooser_ui.c stringChooser_ui.h stringChooser_stubs.C &: ttsnoop.bip stringChooser.bil stringChooser_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i stringChooser_stubs.C.src=stringChooser_stubs.c \
		stringChooser_ui.c stringChooser_ui.h stringChooser_stubs.c=stringChooser_stubs.C \
		-- -changed -merge -p ttsnoop.bip stringChooser.bil
	cat externC.h stringChooser_ui.h closeBrace.h > stringChooser_ui.h.patched
	mv stringChooser_ui.h.patched stringChooser_ui.h

messageProps_ui.c messageProps_ui.h messageProps_stubs.C &: ttsnoop.bip messageProps.bil messageProps_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i messageProps_stubs.C.src=messageProps_stubs.c \
		messageProps_ui.c messageProps_ui.h messageProps_stubs.c=messageProps_stubs.C \
		-- -changed -merge -p ttsnoop.bip messageProps.bil
	cat externC.h messageProps_ui.h closeBrace.h > messageProps_ui.h.patched
	mv messageProps_ui.h.patched messageProps_ui.h

sessionChooser_ui.c sessionChooser_ui.h sessionChooser_stubs.C &: ttsnoop.bip sessionChooser.bil sessionChooser_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i sessionChooser_stubs.C.src=sessionChooser_stubs.c \
		sessionChooser_ui.c sessionChooser_ui.h sessionChooser_stubs.c=sessionChooser_stubs.C \
		-- -changed -merge -p ttsnoop.bip sessionChooser.bil
	cat externC.h sessionChooser_ui.h closeBrace.h > sessionChooser_ui.h.patched
	mv sessionChooser_ui.h.patched sessionChooser_ui.h

fileChooser_ui.c fileChooser_ui.h fileChooser_stubs.C &: ttsnoop.bip fileChooser.bil fileChooser_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i fileChooser_stubs.C.src=fileChooser_stubs.c \
		fileChooser_ui.c fileChooser_ui.h fileChooser_stubs.c=fileChooser_stubs.C \
		-- -changed -merge -p ttsnoop.bip fileChooser.bil
	cat externC.h fileChooser_ui.h closeBrace.h > fileChooser_ui.h.patched
	mv fileChooser_ui.h.patched fileChooser_ui.h

argChooser_ui.c argChooser_ui.h argChooser_stubs.C &: ttsnoop.bip argChooser.bil argChooser_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i argChooser_stubs.C.src=argChooser_stubs.c \
		argChooser_ui.c argChooser_ui.h argChooser_stubs.c=argChooser_stubs.C \
		-- -changed -merge -p ttsnoop.bip argChooser.bil
	cat externC.h argChooser_ui.h closeBrace.h > argChooser_ui.h.patched
	mv argChooser_ui.h.patched argChooser_ui.h

callbackChooser_ui.c callbackChooser_ui.h callbackChooser_stubs.C &: ttsnoop.bip callbackChooser.bil callbackChooser_stubs.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i callbackChooser_stubs.C.src=callbackChooser_stubs.c \
		callbackChooser_ui.c callbackChooser_ui.h callbackChooser_stubs.c=callbackChooser_stubs.C \
		-- -changed -merge -p ttsnoop.bip callbackChooser.bil
	cat externC.h callbackChooser_ui.h closeBrace.h > callbackChooser_ui.h.patched
	mv callbackChooser_ui.h.patched callbackChooser_ui.h

# dtcodegen -main merges into ttsnoop.c, which is a tracked (stale) file
# here: generate in a scratch directory instead and keep only the results.
ttsnoop.C ttsnoop.h dtb_utils.c dtb_utils.h &: ttsnoop.bip ttsnoop.C.src externC.h closeBrace.h
	$(DTCODEGEN_SCRATCH) -i ttsnoop.C.src=ttsnoop.c \
		ttsnoop.c=ttsnoop.C ttsnoop.h dtb_utils.c dtb_utils.h \
		-- -changed -merge -p ttsnoop.bip -main
	chmod -w ttsnoop.C
	cat externC.h dtb_utils.h closeBrace.h > dtb_utils.h.patched
	mv dtb_utils.h.patched dtb_utils.h
	$(SED) \
	-e 's/\(MoreButton;\)/\1Boolean _DtCanHelp( const char * );/' \
	-e 's/\(XtManageChild(Quick_help\)/XtSetSensitive(MoreButton,_DtCanHelp(help_data->help_text));\1/' \
	-e 's/\(buffer, vol = NULL, loc = NULL;\)/\1Boolean _DtHelped( Widget );/' \
	-e 's/\(if( (cp=strrchr\)/if (_DtHelped( help_dialog)) { return; }\1/' \
	dtb_utils.c > dtb_utils.c.patched
	mv dtb_utils.c.patched dtb_utils.c

Ttsnoop: Ttsnoop.src
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && mv -f $@.tmp $@
