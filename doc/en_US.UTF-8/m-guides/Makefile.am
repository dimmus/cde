MAINTAINERCLEANFILES = Makefile.in

BUILT_SOURCES = common/ManLinks.sgm
CLEANFILES = $(BUILT_SOURCES)

# A symbolic link: make sees the mtime of the file it points to.
common/ManLinks.sgm: $(srcdir)/../guides/common/ManLinksMotif.sgm
	$(MKDIR_P) common
	$(RM) $@
	$(LN_S) ../../guides/common/ManLinksMotif.sgm $@


