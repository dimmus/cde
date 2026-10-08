MAINTAINERCLEANFILES = Makefile.in

include ../dtdocbook.am

infolibdir = $(dtdocbooklibexecdir)/infolib

infolib_PROGRAMS = \
		   MixedGen \
		   NCFGen \
		   NodeParser \
		   StyleUpdate \
		   dbdrv \
		   valBase \
		   validator

infolib_SCRIPTS = ChangeCaseTitle DeInstallBase Librarian

noinst_PROGRAMS = restore

AM_LFLAGS = -Cf -L -8 -s
AM_YFLAGS = -d

AM_CXXFLAGS = \
	      -DCDE_NEXT \
	      -DDTDCBK_DATAROOTDIR=\"$(dtdocbookdatadir)\" \
	      -DDTSEARCH \
	      -DDtinfoClient \
	      -DEXPAND_TEMPLATES \
	      -I$(top_builddir)/lib/DtMmdb

AM_LDFLAGS = $(XTOOLLIB) $(LIBMMDB) $(DTCLIENTLIBS) $(LMDBLIB)

MixedGen_SOURCES = \
		   MixedGen.C \
		   AttributeList.C \
		   AttributeRec.C \
		   BTCollectable.C \
		   BookCaseDB.C \
		   DataBase.C \
		   FlexBuffer.C \
		   Handler.C \
		   RemoteId.C \
		   SGMLName.C \
		   StringList.C \
		   Token.C

NCFGen_SOURCES = \
		 NCFGen.C \
		 AttributeList.C \
		 AttributeRec.C \
		 BTCollectable.C \
		 BookCaseDB.C \
		 DataBase.C \
		 Handler.C \
		 SGMLName.C \
		 StringList.C \
		 StyleValidate.C \
		 Token.C

NodeParser_SOURCES = \
		     NodeParser.C \
		     AttributeData.C \
		     AttributeList.C \
		     AttributeRec.C \
		     AusText.C \
		     AusTextStorage.C \
		     BookCaseDB.C \
		     BookTasks.C \
		     ConcatTask.C \
		     Content.C \
		     ContentType.C \
		     DataBase.C \
		     DataRepository.C \
		     Dispatch.C \
		     EntityList.C \
		     ExprList.C \
		     Expression.C \
		     FirstOf.C \
		     FlexBuffer.C \
		     GenericId.C \
		     GraphicsTask.C \
		     Handler.C \
		     LcfTask.C \
		     NodeData.C \
		     NodeTask.C \
		     OLAF.C \
		     OL_Data.C \
		     OL_DataExpr.C \
		     ReplaceIdIdref.C \
		     SGMLDefn.C \
		     SGMLName.C \
		     SearchEng.C \
		     SearchPath.C \
		     StringList.C \
		     StyleTask.C \
		     StyleTaskDB.C \
		     TOCTask.C \
		     Task.C \
		     Token.C \
		     gr_type.C \
		     lex.ll

StyleUpdate_SOURCES = \
		      StyleUpdate.C \
		      AttributeData.C \
		      AttributeList.C \
		      AttributeRec.C \
		      ConcatTask.C \
		      Content.C \
		      ContentType.C \
		      Dispatch.C \
		      EntityList.C \
		      ExprList.C \
		      Expression.C \
		      FirstOf.C \
		      FlexBuffer.C \
		      GenericId.C \
		      Handler.C \
		      OLAF.C \
		      OL_Data.C \
		      OL_DataExpr.C \
		      SGMLDefn.C \
		      SGMLName.C \
		      SearchPath.C \
		      StyleTask.C \
		      StyleValidate.C \
		      Task.C \
		      Token.C \
		      lex.ll

dbdrv_SOURCES = dbdrv.C
valBase_SOURCES = valBase.C
validator_SOURCES = validator.C
restore_SOURCES = restore.C

CLEANFILES = \
	     OL_DataExpr.C \
	     OL_DataExpr.tab.h \
	     ContentType.C \
	     ReplaceIdIdref.C \
	     RemoteId.C \
	     gr_type.C \
	     lex.cc

OL_DataExpr.C: OL_DataExpr.yy
	$(YACC) $(AM_YFLAGS) -p ol_data -b OL_DataExpr $<
	mv OL_DataExpr.tab.c $@

ContentType.C: ContentType.ll OL_DataExpr.C
	$(LEX) $(AM_LFLAGS) -P ol_data -o $@ $<

ReplaceIdIdref.C: ReplaceIdIdref.ll
	$(LEX) $(AM_LFLAGS) -P nodedata -o $@ $<

RemoteId.C: RemoteId.ll
	$(LEX) $(AM_LFLAGS) -P remote -o $@ $<

gr_type.C: gr_type.ll
	$(LEX) $(AM_LFLAGS) -P graphics_task -o $@ $<
