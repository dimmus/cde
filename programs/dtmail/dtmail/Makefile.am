MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtmail

# FORTIFY_SOURCE_2: RoamApp.C formatPropPair() sizes its sprintf buffer
# with int arithmetic, which _FORTIFY_SOURCE=3 reports (-Wformat-overflow)
dtmail_CXXFLAGS = $(CDE_CXXFLAGS) $(FORTIFY_SOURCE_2) -I../include -I../include/MotifApp -I../include/utils \
		  -I../include/DtMail -I../libDtMail/RFC -I$(x_includes) \
		  -I$(srcdir)/lib -I../../dthelp -DRELEASE_NOTES \
		  -DDTMAIL_TOOLTALK -DDTEDITOR

dtmail_LDADD = ../libDtMail/libDtMail.a ../MotifApp/libMotifApp.a \
		$(LIBPRINT) $(LIBHELP) $(LIBWIDGET) $(LIBSVC) $(LIBTT) \
		$(XMLIB) $(XTLIB) $(X11LIB) $(TIRPCLIB)

dtmail_SOURCES = AliasListUiItem.C               AlternatesListUiItem.C \
		 AntiCheckBoxUiItem.C            AttachArea.C \
		 Attachment.C			 CheckBoxUiItem.C \
		 CheckForMailUiItem.C		 ComposeCmds.C \
		 CustomListUiItem.C		 Dialog.C \
		 DialogShell.C			 DmxMailbox.C \
		 DmxMessage.C			 DmxPrintJob.C \
		 DmxPrintOptions.C		 DmxPrintOutput.C \
                 DmxPrintSetup.C		 DmxUtils.C \
		 DtEditor.C			 DtMailEditor.C \
		 DtMailGenDialog.C		 DtMailWDM.C \
		 Editor.C			 EncryptedTextFieldUiItem.C \
		 FindDialog.C			 Fonts.C \
		 Icon.C				 IgnoreListUiItem.C \
		 Image.C			 InboxTextFieldUiItem.C \
		 IndexedOptionMenu.C		 IndexedOptionMenuUiItem.C \
		 ListUiItem.C			 MailRcSource.C \
		 MailRetrievalOptions.C		 MailSession.C \
		 MoveMenuListUiItem.C            MsgHndArray.C \
		 MsgScrollingList.C              NoOpCmd.C \
		 OptCmd.C                        PasswordDialogManager.C \
		 PropUi.C                        QueryDialogManager.C \
		 RoamApp.C                       RoamCmds.C \
		 RoamInterruptibleCmd.C          RoamMenuWindow.C \
		 ScaleUiItem.C                   SendMsgDialog.C \
		 Sort.C                          SortCmd.C \
		 SpinBoxUiItem.C                 StringTab.c \
		 TemplateListUiItem.C            TextFieldUiItem.C \
		 Undelete.C                      ViewMsgDialog.C \
		 WMSaveSession.C                 XmStrCollector.C \
		 XmTextEditor.C                  XtArgCollector.C \
		 dtb_utils.C                     options_stubs.C \
		 options_ui.C                    options_util.C

# Don't fail if we can't set the mail group which might not exist on
# some systems
install-exec-hook:
	-(chgrp mail $(DESTDIR)$(bindir)/dtmail && chmod 2755 $(DESTDIR)$(bindir)/dtmail)
