MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libobject.la

libobject_la_CXXFLAGS = $(CDE_CXXFLAGS) -DPORTABLE_DB -I..

libobject_la_SOURCES = composite.C     compressed_pstring.C    cset.C \
		       dl_list.C       dl_list_cell.C          handler.C \
		       integer.C       long_pstring.C          oid.C \
		       oid_list.C      oid_t.C                 primitive.C \
		       pstring.C       random_gen.C            root.C \
		       short_list.C    tuple.C

noinst_HEADERS = \
    composite.h \
    compressed_pstring.h \
    cset.h \
    dl_list.h \
    dl_list_cell.h \
    handler.h \
    integer.h \
    long_pstring.h \
    oid.h \
    oid_list.h \
    oid_t.h \
    primitive.h \
    pstring.h \
    random_gen.h \
    root.h \
    short_list.h \
    tuple.h
