MAINTAINERCLEANFILES = Makefile.in

INFOLIBDESC = "Biblioteca delle informazioni CDE e Motif"

# get some important variables
include $(top_srcdir)/doc/common/guides/guides_env.am

# prime the pump
BUILT_SOURCES =
CLEANFILES =

# this sets the LANG variables
include $(top_srcdir)/programs/localized/templates/Italian.am

# build guide TOCs and deps
include $(top_srcdir)/doc/common/guides/usersGuide.am
include $(top_srcdir)/doc/common/guides/infoUGuide.am
include $(top_srcdir)/doc/common/guides/sysAdminGuide.am

# build the infolib + search DB
INFOLIBDEPS = $(SGMLDEPS) $(top_srcdir)/doc/common/docbook/docbook.stl

$(INFOLIB)/$(BOOKCASE)/dtsearch/dtsearch.ocf: $(INFOLIBDEPS)
	$(MKDIR_P) $(INFOLIB)
	$(DGENV) $(DTINFOGEN) build -v -T $(DTINFOGENTMPDIR) $(DTINFOGENOPTIONS) $(DGOPTS) $(SSENTS) -l $(INFOLIB) bookcase.bc

BUILT_SOURCES += $(INFOLIB)/$(BOOKCASE)/dtsearch/dtsearch.ocf
CLEANFILES += $(INFOLIB)/$(BOOKCASE)/dtsearch/dtsearch.ocf

# installation - these files *may* change depending on if the docs
# ever change, but for now we will list them as they are currently
# installed.

cdedtidir = $(CDE_DATA_TOP)/infolib/$(LANG)/cde.dti
dist_cdedti_DATA = ../cde.dti/bookcase.map ../cde.dti/cde.oli

cdedtidocdir = $(cdedtidir)/CDEDOC
dist_cdedtidoc_DATA = ../cde.dti/CDEDOC/CDEDOC.dbd	\
		../cde.dti/CDEDOC/CDEDOC.dbi	\
		../cde.dti/CDEDOC/CDEDOC.sch

cdedtidocsearchdir = $(cdedtidocdir)/dtsearch
dist_cdedtidocsearch_DATA = ../cde.dti/CDEDOC/dtsearch/CDEDOC.d00	\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.d22			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.dbd			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.k21			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.sfx			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.d01			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.d23			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.k00			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.k22			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.stp			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.d21			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.d99			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.k01			\
	../cde.dti/CDEDOC/dtsearch/CDEDOC.k23			\
	../cde.dti/CDEDOC/dtsearch/dtsearch.ocf

generated_cdedtidocsearch_outputs = $(filter-out $(INFOLIB)/$(BOOKCASE)/dtsearch/dtsearch.ocf,$(dist_cdedtidocsearch_DATA))

BUILT_SOURCES += $(dist_cdedti_DATA) $(dist_cdedtidoc_DATA) $(dist_cdedtidocsearch_DATA)
CLEANFILES += $(dist_cdedti_DATA) $(dist_cdedtidoc_DATA) $(dist_cdedtidocsearch_DATA)

$(dist_cdedti_DATA) $(dist_cdedtidoc_DATA) $(generated_cdedtidocsearch_outputs): $(INFOLIB)/$(BOOKCASE)/dtsearch/dtsearch.ocf
