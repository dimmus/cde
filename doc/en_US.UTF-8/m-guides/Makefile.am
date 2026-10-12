MAINTAINERCLEANFILES = Makefile.in

# (made by "all", not a BUILT_SOURCES, which make dist would make too)
noinst_DATA = common/ManLinks.sgm
CLEANFILES = $(noinst_DATA)

# A symbolic link: make sees the mtime of the file it points to.
common/ManLinks.sgm: $(srcdir)/../guides/common/ManLinksMotif.sgm
	$(MKDIR_P) common
	$(RM) $@
	$(LN_S) ../../guides/common/ManLinksMotif.sgm $@


