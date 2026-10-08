MAINTAINERCLEANFILES = Makefile.in

# this sets the LANG variables
include $(top_srcdir)/programs/localized/templates/English.am

DBTOMAN = $(top_srcdir)/programs/dtdocbook/doc_utils/dtdocbook2man $(LANG)

# first, the man pages broken down by sections
MANSEC1 = man1/tttrace.1 man1/tttar.1 man1/ttsession.1 man1/ttrmdir.1	\
    man1/ttrm.1 man1/ttmv.1 man1/ttcp.1 man1/tt_type_comp.1		\
    man1/ksh-cde.1 man1/huffcode.1 man1/dtwm.1 man1/dtudcfonted.1		\
    man1/dtudcexch.1 man1/dttypes.1 man1/dtterm.1 man1/dtstyle.1	\
    man1/dtsrload.1 man1/dtsrkdump.1 man1/dtsrindex.1 man1/dtsrhan.1	\
    man1/dtsrdbrec.1 man1/dtsrcreate.1 man1/dtsession_res.1		\
    man1/dtsession.1 man1/dtsearchpath.1 man1/dtscreen.1		\
    man1/dtprintinfo.1 man1/dtpdmd.1 man1/dtpdm.1 man1/dtpad.1		\
    man1/dtmailpr.1 man1/dtmail.1 man1/dtlp.1 man1/dtlogin.1		\
    man1/dtksh.1 man1/dtdocbook2infolib.1 man1/dtinfo.1			\
    man1/dtimsstart.1 man1/dticon.1 man1/dthelpview.1 man1/dthelptag.1	\
    man1/dthelpprint.1 man1/dthelpgen.1 man1/dthelp_htag2.1		\
    man1/dthelp_htag1.1 man1/dthelp_ctag1.1 man1/dthello.1		\
    man1/dtgreet.1 man1/dtfplist.1 man1/dtfile_error.1			\
    man1/dtfile_copy.1 man1/dtfile.1 man1/dtexec.1 man1/dtdocbook.1	\
    man1/dtcreate.1 man1/dtconvertvf.1 man1/dtconfig.1			\
    man1/dtcodegen.1 man1/dtcm_lookup.1 man1/dtcm_insert.1		\
    man1/dtcm_editor.1 man1/dtcm_delete.1 man1/dtcm.1			\
    man1/dtchooser.1 man1/dtcalc.1 man1/dtbuilder.1			\
    man1/dtappintegrate.1 man1/dtappgather.1 man1/dtaction.1		\
    man1/dsdm.1

MANSEC1m = man1m/ttdbserverd.1m man1m/ttdbserver.1m man1m/ttdbck.1m	\
    man1m/dtspcd.1m man1m/rpc.cmsd.1m

MANSEC3 = man3/DtSearchGetMessages.3 man3/DtSearchFreeMessages.3	\
    man3/DtSearchAddMessage.3 man3/DtDndVaDropRegister.3		\
    man3/DtDndVaDragStart.3 man3/DtInitialize.3 man3/tttk_string_op.3	\
    man3/tttk_op_string.3 man3/tttk_message_reject.3			\
    man3/tttk_message_fail.3 man3/tttk_message_destroy.3		\
    man3/tttk_message_create.3 man3/tttk_message_abandon.3		\
    man3/tttk_block_while.3 man3/tttk_Xt_input_handler.3		\
    man3/ttmedia_ptype_declare.3 man3/ttmedia_load_reply.3		\
    man3/ttmedia_load.3 man3/ttmedia_Deposit.3				\
    man3/ttdt_subcontract_manage.3 man3/ttdt_session_quit.3		\
    man3/ttdt_session_join.3 man3/ttdt_sender_imprint_on.3		\
    man3/ttdt_open.3 man3/ttdt_message_accept.3				\
    man3/ttdt_file_request.3 man3/ttdt_file_quit.3			\
    man3/ttdt_file_notice.3 man3/ttdt_file_join.3			\
    man3/ttdt_file_event.3 man3/ttdt_close.3 man3/ttdt_Save.3		\
    man3/ttdt_Revert.3 man3/ttdt_Get_Modified.3				\
    man3/tt_xcontext_quit.3 man3/tt_xcontext_join.3			\
    man3/tt_trace_control.3 man3/tt_thread_session_set.3		\
    man3/tt_thread_session.3 man3/tt_thread_procid_set.3		\
    man3/tt_thread_procid.3 man3/tt_status_message.3			\
    man3/tt_spec_write.3 man3/tt_spec_type_set.3 man3/tt_spec_type.3	\
    man3/tt_spec_propnames_count.3 man3/tt_spec_propname.3		\
    man3/tt_spec_prop_set.3 man3/tt_spec_prop_count.3			\
    man3/tt_spec_prop_add.3 man3/tt_spec_prop.3 man3/tt_spec_move.3	\
    man3/tt_spec_file.3 man3/tt_spec_destroy.3 man3/tt_spec_create.3	\
    man3/tt_spec_bprop_set.3 man3/tt_spec_bprop_add.3			\
    man3/tt_spec_bprop.3 man3/tt_session_types_load.3			\
    man3/tt_session_quit.3 man3/tt_session_propnames_count.3		\
    man3/tt_session_propname.3 man3/tt_session_prop_set.3		\
    man3/tt_session_prop_count.3 man3/tt_session_prop_add.3		\
    man3/tt_session_prop.3 man3/tt_session_join.3			\
    man3/tt_session_bprop_set.3 man3/tt_session_bprop_add.3		\
    man3/tt_session_bprop.3 man3/tt_release.3				\
    man3/tt_ptype_undeclare.3 man3/tt_ptype_opnum_callback_add.3	\
    man3/tt_ptype_exists.3 man3/tt_ptype_declare.3			\
    man3/tt_ptr_error.3 man3/tt_procid_session.3			\
    man3/tt_prequest_create.3 man3/tt_pointer_error.3			\
    man3/tt_pnotice_create.3 man3/tt_pattern_xcontext_add.3		\
    man3/tt_pattern_xarg_add.3 man3/tt_pattern_user_set.3		\
    man3/tt_pattern_user.3 man3/tt_pattern_unregister.3			\
    man3/tt_pattern_state_add.3 man3/tt_pattern_session_add.3		\
    man3/tt_pattern_sender_ptype_add.3 man3/tt_pattern_sender_add.3	\
    man3/tt_pattern_scope_add.3 man3/tt_pattern_register.3		\
    man3/tt_pattern_print.3 man3/tt_pattern_otype_add.3			\
    man3/tt_pattern_opnum_add.3 man3/tt_pattern_op_add.3		\
    man3/tt_pattern_object_add.3 man3/tt_pattern_icontext_add.3		\
    man3/tt_pattern_iarg_add.3 man3/tt_pattern_file_add.3		\
    man3/tt_pattern_disposition_add.3 man3/tt_pattern_destroy.3		\
    man3/tt_pattern_create.3 man3/tt_pattern_context_add.3		\
    man3/tt_pattern_class_add.3 man3/tt_pattern_category_set.3		\
    man3/tt_pattern_category.3 man3/tt_pattern_callback_add.3		\
    man3/tt_pattern_bcontext_add.3 man3/tt_pattern_barg_add.3		\
    man3/tt_pattern_arg_add.3 man3/tt_pattern_address_add.3		\
    man3/tt_otype_osig_op.3 man3/tt_otype_osig_count.3			\
    man3/tt_otype_osig_args_count.3 man3/tt_otype_osig_arg_type.3	\
    man3/tt_otype_osig_arg_mode.3 man3/tt_otype_opnum_callback_add.3	\
    man3/tt_otype_is_derived.3 man3/tt_otype_hsig_op.3			\
    man3/tt_otype_hsig_count.3 man3/tt_otype_hsig_args_count.3		\
    man3/tt_otype_hsig_arg_type.3 man3/tt_otype_hsig_arg_mode.3		\
    man3/tt_otype_deriveds_count.3 man3/tt_otype_derived.3		\
    man3/tt_otype_base.3 man3/tt_orequest_create.3 man3/tt_open.3	\
    man3/tt_onotice_create.3 man3/tt_objid_objkey.3			\
    man3/tt_objid_equal.3 man3/tt_netfile_file.3			\
    man3/tt_message_xcontext_set.3 man3/tt_message_xarg_add.3		\
    man3/tt_message_user_set.3 man3/tt_message_user.3			\
    man3/tt_message_uid.3 man3/tt_message_status_string_set.3		\
    man3/tt_message_status_string.3 man3/tt_message_status_set.3	\
    man3/tt_message_status.3 man3/tt_message_state.3			\
    man3/tt_message_session_set.3 man3/tt_message_session.3		\
    man3/tt_message_sender_ptype_set.3 man3/tt_message_sender_ptype.3	\
    man3/tt_message_sender.3 man3/tt_message_send_on_exit.3		\
    man3/tt_message_send.3 man3/tt_message_scope_set.3			\
    man3/tt_message_scope.3 man3/tt_message_reply.3			\
    man3/tt_message_rejecters_count.3 man3/tt_message_rejecter.3	\
    man3/tt_message_reject.3 man3/tt_message_receive.3			\
    man3/tt_message_print.3 man3/tt_message_pattern.3			\
    man3/tt_message_otype_set.3 man3/tt_message_otype.3			\
    man3/tt_message_opnum.3 man3/tt_message_op_set.3			\
    man3/tt_message_op.3 man3/tt_message_object_set.3			\
    man3/tt_message_object.3 man3/tt_message_id.3			\
    man3/tt_message_icontext_set.3 man3/tt_message_iarg_add.3		\
    man3/tt_message_handler_set.3 man3/tt_message_handler_ptype_set.3	\
    man3/tt_message_handler_ptype.3 man3/tt_message_handler.3		\
    man3/tt_message_gid.3 man3/tt_message_file_set.3			\
    man3/tt_message_file.3 man3/tt_message_fail.3			\
    man3/tt_message_disposition_set.3 man3/tt_message_disposition.3	\
    man3/tt_message_destroy.3 man3/tt_message_create_super.3		\
    man3/tt_message_create.3 man3/tt_message_contexts_count.3		\
    man3/tt_message_context_xval.3 man3/tt_message_context_val.3	\
    man3/tt_message_context_slotname.3 man3/tt_message_context_set.3	\
    man3/tt_message_context_ival.3 man3/tt_message_context_bval.3	\
    man3/tt_message_class_set.3 man3/tt_message_class.3			\
    man3/tt_message_callback_add.3 man3/tt_message_bcontext_set.3	\
    man3/tt_message_barg_add.3 man3/tt_message_args_count.3		\
    man3/tt_message_arg_xval_set.3 man3/tt_message_arg_xval.3		\
    man3/tt_message_arg_val_set.3 man3/tt_message_arg_val.3		\
    man3/tt_message_arg_type.3 man3/tt_message_arg_mode.3		\
    man3/tt_message_arg_ival_set.3 man3/tt_message_arg_ival.3		\
    man3/tt_message_arg_bval_set.3 man3/tt_message_arg_bval.3		\
    man3/tt_message_arg_add.3 man3/tt_message_address_set.3		\
    man3/tt_message_address.3 man3/tt_message_accepters_count.3		\
    man3/tt_message_accepter.3 man3/tt_message_accept.3			\
    man3/tt_message_abstainers_count.3 man3/tt_message_abstainer.3	\
    man3/tt_mark.3 man3/tt_malloc.3 man3/tt_is_err.3			\
    man3/tt_int_error.3 man3/tt_initial_session.3			\
    man3/tt_icontext_quit.3 man3/tt_icontext_join.3			\
    man3/tt_host_netfile_file.3 man3/tt_host_file_netfile.3		\
    man3/tt_free.3 man3/tt_file_quit.3 man3/tt_file_objects_query.3	\
    man3/tt_file_netfile.3 man3/tt_file_move.3 man3/tt_file_join.3	\
    man3/tt_file_destroy.3 man3/tt_file_copy.3				\
    man3/tt_feature_required.3 man3/tt_feature_enabled.3 man3/tt_fd.3	\
    man3/tt_error_pointer.3 man3/tt_error_int.3 man3/tt_error.3		\
    man3/tt_default_session_set.3 man3/tt_default_session.3		\
    man3/tt_default_ptype_set.3 man3/tt_default_ptype.3			\
    man3/tt_default_procid_set.3 man3/tt_default_procid.3		\
    man3/tt_default_file_set.3 man3/tt_default_file.3			\
    man3/tt_context_quit.3 man3/tt_context_join.3 man3/tt_close.3	\
    man3/tt_bcontext_quit.3 man3/tt_bcontext_join.3			\
    man3/tt_X_session.3 man3/csa_x_process_updates.3			\
    man3/csa_update_entry_attributes.3					\
    man3/csa_update_calendar_attributes.3				\
    man3/csa_unregister_callback.3 man3/csa_save.3 man3/csa_restore.3	\
    man3/csa_register_callback.3 man3/csa_read_next_reminder.3		\
    man3/csa_read_entry_attributes.3					\
    man3/csa_read_calendar_attributes.3					\
    man3/csa_query_configuration.3 man3/csa_look_up.3			\
    man3/csa_logon.3 man3/csa_logoff.3 man3/csa_list_entry_sequence.3	\
    man3/csa_list_entry_attributes.3 man3/csa_list_entries.3		\
    man3/csa_list_calendars.3 man3/csa_list_calendar_attributes.3	\
    man3/csa_free_time_search.3 man3/csa_free.3				\
    man3/csa_delete_entry.3 man3/csa_delete_calendar.3			\
    man3/csa_call_callbacks.3 man3/csa_add_entry.3			\
    man3/csa_add_calendar.3 man3/DtWsmSetWorkspaceTitle.3		\
    man3/DtWsmSetWorkspacesOccupied.3 man3/DtWsmSetCurrentWorkspace.3	\
    man3/DtWsmRemoveWorkspaceFunctions.3				\
    man3/DtWsmRemoveWorkspaceCallback.3					\
    man3/DtWsmOccupyAllWorkspaces.3 man3/DtWsmGetWorkspacesOccupied.3	\
    man3/DtWsmGetWorkspaceList.3 man3/DtWsmGetWorkspaceInfo.3		\
    man3/DtWsmGetCurrentWorkspace.3					\
    man3/DtWsmGetCurrentBackdropWindow.3				\
    man3/DtWsmFreeWorkspaceInfo.3 man3/DtWsmDeleteWorkspace.3		\
    man3/DtWsmChangeBackdrop.3						\
    man3/DtWsmAddWorkspaceModifiedCallback.3				\
    man3/DtWsmAddWorkspaceFunctions.3 man3/DtWsmAddWorkspace.3		\
    man3/DtWsmAddCurrentWorkspaceCallback.3 man3/DtTermSubprocSend.3	\
    man3/DtTermSubprocReap.3 man3/DtTermInitialize.3			\
    man3/DtTermDisplaySend.3 man3/DtTerm.3 man3/DtSrAPI.3		\
    man3/DtSpinBoxSetItem.3 man3/DtSpinBoxDeletePos.3			\
    man3/DtSpinBoxAddItem.3 man3/DtSpinBox.3 man3/DtSessionSavePath.3	\
    man3/DtSessionRestorePath.3 man3/DtSearchValidDateString.3		\
    man3/DtSearchSortResults.3 man3/DtSearchSetMaxResults.3		\
    man3/DtSearchRetrieve.3 man3/DtSearchReinit.3			\
    man3/DtSearchQuery.3 man3/DtSearchHasMessages.3			\
    man3/DtSearchMergeResults.3 man3/DtSearchInit.3			\
    man3/DtSearchHighlight.3 man3/DtSearchGetMaxResults.3		\
    man3/DtSearchGetKeytypes.3 man3/DtSearchFreeResults.3		\
    man3/DtSearchExit.3 man3/DtSaverGetWindows.3			\
    man3/DtPrintSetupProc.3 man3/DtPrintSetupBox.3			\
    man3/DtPrintResetConnection.3 man3/DtPrintFreeSetupData.3		\
    man3/DtPrintFillSetupData.3 man3/DtPrintCopySetupData.3		\
    man3/DtMsgLogSetHandler.3 man3/DtMsgLogOpenFile.3			\
    man3/DtMsgLogMessage.3 man3/DtMrmInitialize.3			\
    man3/DtMmdbTocGetParentId.3 man3/DtMmdbTocGetNumOfChildren.3	\
    man3/DtMmdbTocGetChildIds.3 man3/DtMmdbStylesheetGetName.3		\
    man3/DtMmdbStylesheetGetData.3 man3/DtMmdbSectionGetTocLoc.3	\
    man3/DtMmdbSectionGetStyleSheetId.3					\
    man3/DtMmdbSectionGetShortTitle.3					\
    man3/DtMmdbSectionGetLongTitle.3 man3/DtMmdbSectionGetLoc.3		\
    man3/DtMmdbSectionGetDataSize.3 man3/DtMmdbSectionGetData.3		\
    man3/DtMmdbSectionGetBookId.3 man3/DtMmdbQuit.3			\
    man3/DtMmdbOpenInfoLib.3 man3/DtMmdbLocatorGetSectionObjectId.3	\
    man3/DtMmdbLocatorGetSectionLoc.3 man3/DtMmdbInit.3			\
    man3/DtMmdbInfoLibGetInfo.3 man3/DtMmdbInfoLibFreeInfo.3		\
    man3/DtMmdbGraphicGetInfo.3 man3/DtMmdbGraphicGetData.3		\
    man3/DtMmdbGetBookCaseByName.3 man3/DtMmdbGetBookCaseByLocs.3	\
    man3/DtMmdbGetBookCaseByLoc.3 man3/DtMmdbGetBookCaseByIndex.3	\
    man3/DtMmdbFreeHandleList.3 man3/DtMmdbFreeHandle.3			\
    man3/DtMmdbFreeGraphicInfo.3 man3/DtMmdbDlpGetPrevSectionId.3	\
    man3/DtMmdbDlpGetNextSectionId.3 man3/DtMmdbCloseInfoLib.3		\
    man3/DtMmdbBookGetTocObjectId.3 man3/DtMmdbBookGetTabList.3		\
    man3/DtMmdbBookGetShortTitle.3 man3/DtMmdbBookGetSeqNum.3		\
    man3/DtMmdbBookGetLongTitle.3 man3/DtMmdbBookGetLicense.3		\
    man3/DtMmdbBookCaseGetInfo.3 man3/DtMmdbBookCaseFreeInfo.3		\
    man3/DtMenuButton.3 man3/DtInfoShowTopic.3				\
    man3/DtHelpSetCatalogName.3 man3/DtHelpReturnSelectedWidgetId.3	\
    man3/DtHelpQuickDialogGetChild.3 man3/DtHelpQuickDialog.3		\
    man3/DtHelpDialog.3 man3/DtEditorUndoEdit.3				\
    man3/DtEditorTraverseToEditor.3					\
    man3/DtEditorSetInsertionPosition.3					\
    man3/DtEditorSetContentsFromFile.3 man3/DtEditorSetContents.3	\
    man3/DtEditorSelectAll.3 man3/DtEditorSaveContentsToFile.3		\
    man3/DtEditorReset.3 man3/DtEditorReplaceFromFile.3			\
    man3/DtEditorReplace.3 man3/DtEditorPasteFromClipboard.3		\
    man3/DtEditorInvokeSpellDialog.3					\
    man3/DtEditorInvokeFormatDialog.3					\
    man3/DtEditorInvokeFindChangeDialog.3				\
    man3/DtEditorInsertFromFile.3 man3/DtEditorInsert.3			\
    man3/DtEditorGoToLine.3 man3/DtEditorGetSizeHints.3			\
    man3/DtEditorGetMessageTextFieldID.3				\
    man3/DtEditorGetLastPosition.3					\
    man3/DtEditorGetInsertionPosition.3 man3/DtEditorGetContents.3	\
    man3/DtEditorFormat.3 man3/DtEditorFind.3				\
    man3/DtEditorEnableRedisplay.3 man3/DtEditorDisableRedisplay.3	\
    man3/DtEditorDeselect.3 man3/DtEditorDeleteSelection.3		\
    man3/DtEditorCutToClipboard.3 man3/DtEditorCopyToClipboard.3	\
    man3/DtEditorClearSelection.3					\
    man3/DtEditorCheckForUnsavedChanges.3 man3/DtEditorChange.3		\
    man3/DtEditorAppendFromFile.3 man3/DtEditorAppend.3			\
    man3/DtEditor.3 man3/DtDtsSetDataType.3 man3/DtDtsRelease.3		\
    man3/DtDtsLoadDataTypes.3 man3/DtDtsIsTrue.3			\
    man3/DtDtsFreeDataTypeNames.3 man3/DtDtsFreeDataType.3		\
    man3/DtDtsFreeAttributeValue.3 man3/DtDtsFreeAttributeList.3	\
    man3/DtDtsFindAttribute.3 man3/DtDtsFileToDataType.3		\
    man3/DtDtsFileToAttributeValue.3 man3/DtDtsFileToAttributeList.3	\
    man3/DtDtsDataTypeToAttributeValue.3				\
    man3/DtDtsDataTypeToAttributeList.3 man3/DtDtsDataTypeNames.3	\
    man3/DtDtsDataTypeIsAction.3 man3/DtDtsDataToDataType.3		\
    man3/DtDtsBufferToDataType.3 man3/DtDtsBufferToAttributeValue.3	\
    man3/DtDtsBufferToAttributeList.3 man3/DtDndDropUnregister.3	\
    man3/DtDndDropRegister.3 man3/DtDndDragStart.3			\
    man3/DtDndCreateSourceIcon.3 man3/DtDbReloadNotify.3		\
    man3/DtDbLoad.3 man3/DtCreateTerm.3 man3/DtCreateSpinBox.3		\
    man3/DtCreatePrintSetupDialog.3 man3/DtCreatePrintSetupBox.3	\
    man3/DtCreateMenuButton.3 man3/DtCreateHelpQuickDialog.3		\
    man3/DtCreateHelpDialog.3 man3/DtCreateEditor.3			\
    man3/DtCreateComboBox.3 man3/DtComboBoxSetItem.3			\
    man3/DtComboBoxSelectItem.3 man3/DtComboBoxDeletePos.3		\
    man3/DtComboBoxAddItem.3 man3/DtComboBox.3 man3/DtAppInitialize.3	\
    man3/DtActionLabel.3 man3/DtActionInvoke.3 man3/DtActionIcon.3	\
    man3/DtActionExists.3 man3/DtActionDescription.3			\
    man3/DtActionCallbackProc.3

MANSEC4 = man4/tttracefile.4 man4/startlog.4 man4/mt-libtt-intro.4	\
    man4/dtwmrc.4 man4/dtsrocffile.4 man4/dtsrlangfiles.4		\
    man4/dtsrhanfile.4 man4/dtsrfzkfiles.4 man4/dtsrdbfiles.4		\
    man4/dtspcdenv.4 man4/dtsessionfile.4 man4/dtsessiondb.4		\
    man4/dtsdlfile.4 man4/dtsdldtd.4 man4/dtresourcesfile.4		\
    man4/dtpmfile.4 man4/dtlocator.4 man4/dticonfile.4			\
    man4/dthffile.4 man4/dthelptagdtd.4 man4/dtfpfile.4			\
    man4/dtfile_config.4 man4/dtdtsfile.4 man4/dtdtfile.4		\
    man4/dtdpfile.4 man4/dtcm_entry.4 man4/dtcm_archive.4		\
    man4/dtbmfile.4 man4/dtactionfile.4 man4/Translate.4		\
    man4/Stopped.4 man4/Status.4 man4/Started.4 man4/Signal.4		\
    man4/Set_Situation.4 man4/Set_Mapped.4 man4/Set_Locale.4		\
    man4/Set_Iconified.4 man4/Set_Geometry.4 man4/Set_Environment.4	\
    man4/Saved.4 man4/Save.4 man4/Reverted.4 man4/Revert.4		\
    man4/Resume.4 man4/Raise.4 man4/Quit.4 man4/Print.4 man4/Pause.4	\
    man4/Modified.4 man4/Mail.4 man4/Lower.4 man4/Instantiate.4		\
    man4/Get_XInfo.4 man4/Get_Sysinfo.4 man4/Get_Status.4		\
    man4/Get_Situation.4 man4/Get_Modified.4 man4/Get_Mapped.4		\
    man4/Get_Locale.4 man4/Get_Iconified.4 man4/Get_Geometry.4		\
    man4/Get_Environment.4 man4/Edit.4 man4/DtInfo_ShowInfoAtLoc.4	\
    man4/DtInfo_Quit.4 man4/DtInfo_PrintInfoAtLoc.4			\
    man4/DtInfo_LoadInfoLib.4 man4/DtDndProtocol.4 man4/Display.4	\
    man4/Deposit.4 man4/BIL.4

MANSEC5 = man5/dttrashaction.5 man5/dttextaction.5			\
    man5/dttermaction.5 man5/dtterm.5 man5/dtstyleaction.5		\
    man5/dtsessionaction.5 man5/dtprintinfoaction.5			\
    man5/dtmanaction.5 man5/dtmailaction.5 man5/dtinfoaf.5		\
    man5/dtinfoaction.5 man5/dtinfoTOCdtd.5 man5/dtinfoStyledtd.5	\
    man5/dtinfoBookdtd.5 man5/dtimsstartaction.5 man5/dticonaction.5	\
    man5/dthelpaction.5 man5/dtfilsys.5 man5/dtfileaction.5		\
    man5/dtenvvar.5 man5/dtcalendaraction.5 man5/dtcalcaction.5		\
    man5/dtbuilderaction.5 man5/dtappaction.5 man5/dtactionaction.5	\
    man5/csacsa.5 man5/Tttttk.5 man5/Tttt_c.5 man5/DtWsm.5		\
    man5/DtTerm.5 man5/DtStdInterfaceFontNames.5			\
    man5/DtStdAppFontNames.5 man5/DtSpinBox.5 man5/DtSession.5		\
    man5/DtSearch.5 man5/DtSaver.5 man5/DtPrint.5 man5/DtMsgLog.5	\
    man5/DtMrm.5 man5/DtMmdbInfoRequest.5 man5/DtMmdbInfoLibInfo.5	\
    man5/DtMmdbHandle.5 man5/DtMmdbGraphicInfo.5			\
    man5/DtMmdbBookCase.5 man5/DtMenuButton.5 man5/DtInfoLib.5		\
    man5/DtHelpQuickD.5 man5/DtHelpDialog.5 man5/DtHelp.5		\
    man5/DtEditor.5 man5/DtDts.5 man5/DtDnd.5 man5/DtComboBox.5		\
    man5/DtAction.5 man5/Dt.5

MANSEC6 = man6/ttsnoop.6

# installation

cdeman1dir = $(mandir)/man1
dist_cdeman1_DATA = $(MANSEC1)

cdeman1mdir = $(mandir)/man1m
dist_cdeman1m_DATA = $(MANSEC1m)

cdeman3dir = $(mandir)/man3
dist_cdeman3_DATA = $(MANSEC3)

cdeman4dir = $(mandir)/man4
dist_cdeman4_DATA = $(MANSEC4)

cdeman5dir = $(mandir)/man5
dist_cdeman5_DATA = $(MANSEC5)

cdeman6dir = $(mandir)/man6
dist_cdeman6_DATA = $(MANSEC6)

# cleanup and accounting
BUILT_SOURCES = $(MANSEC1) $(MANSEC1m) $(MANSEC3) $(MANSEC4)	\
    $(MANSEC5) $(MANSEC6)

CLEANFILES = $(BUILT_SOURCES)


# Now the man rules, followed by the alias rules

MANDEPS = cdedecl.sgm ../guides/man/SEntity.sgm ../guides/common/ManLinks.sgm

man1/dsdm.1: ../guides/man/man1/dsdm.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtaction.1: ../guides/man/man1_dt/action.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtappgather.1: ../guides/man/man1_dt/appgathe.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtappintegrate.1: ../guides/man/man1_dt/appinteg.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtbuilder.1: ../guides/man/man1_dt/builder.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtcalc.1: ../guides/man/man1_dt/calc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtchooser.1: ../guides/man/man1_dt/chooser.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtcm.1: ../guides/man/man1_dt/cm.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtcm_delete.1: ../guides/man/man1_dt/cm_delet.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtcm_editor.1: ../guides/man/man1_dt/cm_edito.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtcm_insert.1: ../guides/man/man1_dt/cm_inser.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtcm_lookup.1: ../guides/man/man1_dt/cm_looku.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtcodegen.1: ../guides/man/man1_dt/codegen.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtconfig.1: ../guides/man/man1_dt/config.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtconvertvf.1: ../guides/man/man1_dt/convertv.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtcreate.1: ../guides/man/man1_dt/create.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtdocbook.1: ../guides/man/man1_dt/docbook.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtexec.1: ../guides/man/man1_dt/exec.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtfile.1: ../guides/man/man1_dt/file.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtfile_copy.1: ../guides/man/man1_dt/file_cop.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtfile_error.1: ../guides/man/man1_dt/file_err.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtfplist.1: ../guides/man/man1_dt/fplist.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtgreet.1: ../guides/man/man1_dt/greet.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dthello.1: ../guides/man/man1_dt/hello.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dthelp_ctag1.1: ../guides/man/man1_dt/he_ctag1.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dthelp_htag1.1: ../guides/man/man1_dt/he_htag1.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dthelp_htag2.1: ../guides/man/man1_dt/he_htag2.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dthelpgen.1: ../guides/man/man1_dt/helpgen.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dthelpprint.1: ../guides/man/man1_dt/helpprin.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dthelptag.1: ../guides/man/man1_dt/helptag.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dthelpview.1: ../guides/man/man1_dt/helpview.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dticon.1: ../guides/man/man1_dt/icon.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtimsstart.1: ../guides/man/man1_dt/imsstart.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtinfo.1: ../guides/man/man1_dt/info.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtdocbook2infolib.1: ../guides/man/man1_dt/docbook2infolib.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtksh.1: ../guides/man/man1_dt/ksh.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtlogin.1: ../guides/man/man1_dt/login.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtlp.1: ../guides/man/man1_dt/lp.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtmail.1: ../guides/man/man1_dt/mail.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtmailpr.1: ../guides/man/man1_dt/mailpr.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtpad.1: ../guides/man/man1_dt/pad.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtpdm.1: ../guides/man/man1_dt/pdm.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtpdmd.1: ../guides/man/man1_dt/pdmd.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtprintinfo.1: ../guides/man/man1_dt/printinf.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtscreen.1: ../guides/man/man1_dt/screen.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtsearchpath.1: ../guides/man/man1_dt/searchpa.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtsession.1: ../guides/man/man1_dt/session.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtsession_res.1: ../guides/man/man1_dt/sess_res.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtsrcreate.1: ../guides/man/man1_dt/srcreate.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtsrdbrec.1: ../guides/man/man1_dt/srdbrec.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtsrhan.1: ../guides/man/man1_dt/srhan.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtsrindex.1: ../guides/man/man1_dt/srindex.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtsrkdump.1: ../guides/man/man1_dt/srkdump.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtsrload.1: ../guides/man/man1_dt/srload.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtstyle.1: ../guides/man/man1_dt/style.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtterm.1: ../guides/man/man1_dt/term.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dttypes.1: ../guides/man/man1_dt/types.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtudcexch.1: ../guides/man/man1_dt/udcexch.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtudcfonted.1: ../guides/man/man1_dt/udcfonte.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/dtwm.1: ../guides/man/man1_dt/wm.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/huffcode.1: ../guides/man/man1_dt/huffcode.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/ksh-cde.1: ../guides/man/man1/ksh93.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/tt_type_comp.1: ../guides/man/man1/tt_typ_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/ttcp.1: ../guides/man/man1/ttcp.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/ttmv.1: ../guides/man/man1/ttmv.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/ttrm.1: ../guides/man/man1/ttrm.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/ttrmdir.1: ../guides/man/man1/ttrmdir.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/ttsession.1: ../guides/man/man1/ttsessio.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/tttar.1: ../guides/man/man1/tttar.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1/tttrace.1: ../guides/man/man1/tttrace.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1m/dtspcd.1m: ../guides/man/man1m/dtspcd.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1m/rpc.cmsd.1m: ../guides/man/man1m/rpccmsd.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

NOTHING: cmsd, SRC = ../guides/man/man1m/rpccmsd.sgm, DST = man1m/rpc.cmsd.1m
man1m/ttdbck.1m: ../guides/man/man1m/ttdbck.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1m/ttdbserver.1m: ../guides/man/man1m/rpcttdbs.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man1m/ttdbserverd.1m: ../guides/man/man1m/ttdbserv.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtActionCallbackProc.3: ../guides/man/m3_Dt/ActionCa.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtActionDescription.3: ../guides/man/m3_Dt/ActionDe.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtActionExists.3: ../guides/man/m3_Dt/ActionEx.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtActionIcon.3: ../guides/man/m3_Dt/ActionIc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtActionInvoke.3: ../guides/man/m3_Dt/ActionIn.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtActionLabel.3: ../guides/man/m3_Dt/ActionLa.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtAppInitialize.3: ../guides/man/m3_Dt/Initiali.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtComboBox.3: ../guides/man/m3_Dt/ComboBox.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtComboBoxAddItem.3: ../guides/man/m3_Dt/ComboAdd.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtComboBoxDeletePos.3: ../guides/man/m3_Dt/ComboDel.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtComboBoxSelectItem.3: ../guides/man/m3_Dt/ComboSel.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtComboBoxSetItem.3: ../guides/man/m3_Dt/ComboSet.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtCreateComboBox.3: ../guides/man/m3_Dt/CreCombo.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtCreateEditor.3: ../guides/man/m3_Dt/CreEdito.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtCreateHelpDialog.3: ../guides/man/m3_Dt/CreHelpD.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtCreateHelpQuickDialog.3: ../guides/man/m3_Dt/CreHelpQ.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtCreateMenuButton.3: ../guides/man/m3_Dt/CreMenuB.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtCreatePrintSetupBox.3: ../guides/man/m3_Dt/CreatePr.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtCreatePrintSetupDialog.3: ../guides/man/m3_Dt/CrePrntD.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtCreateSpinBox.3: ../guides/man/m3_Dt/CreSpinB.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtCreateTerm.3: ../guides/man/m3_Dt/CreTerm.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDbLoad.3: ../guides/man/m3_Dt/DbLoad.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDbReloadNotify.3: ../guides/man/m3_Dt/DbReload.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDndCreateSourceIcon.3: ../guides/man/m3_Dt/DndCreSI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDndDragStart.3: ../guides/man/m3_Dt/DndDragS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDndDropRegister.3: ../guides/man/m3_Dt/DndDropR.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDndDropUnregister.3: ../guides/man/m3_Dt/DndDropU.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsBufferToAttributeList.3: ../guides/man/m3_DtDts/BufToAtL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsBufferToAttributeValue.3: ../guides/man/m3_DtDts/BufToAtV.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsBufferToDataType.3: ../guides/man/m3_DtDts/BufToDaT.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsDataToDataType.3: ../guides/man/m3_DtDts/DatToDaT.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsDataTypeIsAction.3: ../guides/man/m3_DtDts/DaTIsAct.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsDataTypeNames.3: ../guides/man/m3_DtDts/DaTNames.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsDataTypeToAttributeList.3: ../guides/man/m3_DtDts/DaTToAtL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsDataTypeToAttributeValue.3: ../guides/man/m3_DtDts/DaTToAtV.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsFileToAttributeList.3: ../guides/man/m3_DtDts/FilToAtL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsFileToAttributeValue.3: ../guides/man/m3_DtDts/FilToAtV.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsFileToDataType.3: ../guides/man/m3_DtDts/FilToDaT.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsFindAttribute.3: ../guides/man/m3_DtDts/FindAttr.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsFreeAttributeList.3: ../guides/man/m3_DtDts/FreeAtL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsFreeAttributeValue.3: ../guides/man/m3_DtDts/FreeAtV.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsFreeDataType.3: ../guides/man/m3_DtDts/FreeDaT.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsFreeDataTypeNames.3: ../guides/man/m3_DtDts/FreeDaTN.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsIsTrue.3: ../guides/man/m3_DtDts/IsTrue.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsLoadDataTypes.3: ../guides/man/m3_DtDts/LoadDaTs.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsRelease.3: ../guides/man/m3_DtDts/Release.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtDtsSetDataType.3: ../guides/man/m3_DtDts/SetDaT.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditor.3: ../guides/man/m3_Dt/Editor.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorAppend.3: ../guides/man/m3_Dt/EdAppend.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorAppendFromFile.3: ../guides/man/m3_Dt/EdAppFF.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorChange.3: ../guides/man/m3_Dt/EdChange.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorCheckForUnsavedChanges.3: ../guides/man/m3_Dt/EdCheckF.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorClearSelection.3: ../guides/man/m3_Dt/EdClearS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorCopyToClipboard.3: ../guides/man/m3_Dt/EdCopyTo.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorCutToClipboard.3: ../guides/man/m3_Dt/EdCutToC.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorDeleteSelection.3: ../guides/man/m3_Dt/EdDelete.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorDeselect.3: ../guides/man/m3_Dt/EdDesele.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorDisableRedisplay.3: ../guides/man/m3_Dt/EdDisabl.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorEnableRedisplay.3: ../guides/man/m3_Dt/EdEnable.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorFind.3: ../guides/man/m3_Dt/EdFind.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorFormat.3: ../guides/man/m3_Dt/EdFormat.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorGetContents.3: ../guides/man/m3_Dt/EdGetCon.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorGetInsertionPosition.3: ../guides/man/m3_Dt/EdGetIns.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorGetLastPosition.3: ../guides/man/m3_Dt/EdGetLas.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorGetMessageTextFieldID.3: ../guides/man/m3_Dt/EdGetMes.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorGetSizeHints.3: ../guides/man/m3_Dt/EdGetSiz.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorGoToLine.3: ../guides/man/m3_Dt/EdGoToLi.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorInsert.3: ../guides/man/m3_Dt/EdInsert.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorInsertFromFile.3: ../guides/man/m3_Dt/EdInsFF.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorInvokeFindChangeDialog.3: ../guides/man/m3_Dt/EdInvFin.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorInvokeFormatDialog.3: ../guides/man/m3_Dt/EdInvFor.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorInvokeSpellDialog.3: ../guides/man/m3_Dt/EdInvSpe.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorPasteFromClipboard.3: ../guides/man/m3_Dt/EdPasteF.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorReplace.3: ../guides/man/m3_Dt/EdReplac.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorReplaceFromFile.3: ../guides/man/m3_Dt/EdReplFF.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorReset.3: ../guides/man/m3_Dt/EdReset.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorSaveContentsToFile.3: ../guides/man/m3_Dt/EdSaveCo.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorSelectAll.3: ../guides/man/m3_Dt/EdSelect.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorSetContents.3: ../guides/man/m3_Dt/EdSetCon.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorSetContentsFromFile.3: ../guides/man/m3_Dt/EdSetCFF.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorSetInsertionPosition.3: ../guides/man/m3_Dt/EdSetIns.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorTraverseToEditor.3: ../guides/man/m3_Dt/EdTraver.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtEditorUndoEdit.3: ../guides/man/m3_Dt/EdUndoEd.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtHelpDialog.3: ../guides/man/m3_Dt/HelpDial.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtHelpQuickDialog.3: ../guides/man/m3_Dt/HelpQDia.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtHelpQuickDialogGetChild.3: ../guides/man/m3_Dt/HelpQDGC.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtHelpReturnSelectedWidgetId.3: ../guides/man/m3_Dt/HelpRetu.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtHelpSetCatalogName.3: ../guides/man/m3_Dt/HelpSetC.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtInfoShowTopic.3: ../guides/man/m3_Dt/InfoShow.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMenuButton.3: ../guides/man/m3_Dt/MenuButt.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbBookCaseFreeInfo.3: ../guides/man/m3_Dt/MmdbBCFI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbBookCaseGetInfo.3: ../guides/man/m3_Dt/MmdbBCGI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbBookGetLicense.3: ../guides/man/m3_Dt/MmdbBkGL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbBookGetLongTitle.3: ../guides/man/m3_Dt/MmdbBGLT.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbBookGetSeqNum.3: ../guides/man/m3_Dt/MmdbBGSN.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbBookGetShortTitle.3: ../guides/man/m3_Dt/MmdbBGST.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbBookGetTabList.3: ../guides/man/m3_Dt/MmdbBGTL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbBookGetTocObjectId.3: ../guides/man/m3_Dt/MmdbBGTO.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbCloseInfoLib.3: ../guides/man/m3_Dt/MmdbClIL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbDlpGetNextSectionId.3: ../guides/man/m3_Dt/MmdbDGNS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbDlpGetPrevSectionId.3: ../guides/man/m3_Dt/MmdbDGPS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbFreeGraphicInfo.3: ../guides/man/m3_Dt/MmdbFrGI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbFreeHandle.3: ../guides/man/m3_Dt/MmdbFrHn.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbFreeHandleList.3: ../guides/man/m3_Dt/MmdbFrHL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbGetBookCaseByIndex.3: ../guides/man/m3_Dt/MmdbGBBI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbGetBookCaseByLoc.3: ../guides/man/m3_Dt/MmdbGBBL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbGetBookCaseByLocs.3: ../guides/man/m3_Dt/MmdbGBLs.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbGetBookCaseByName.3: ../guides/man/m3_Dt/MmdbGBBN.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbGraphicGetData.3: ../guides/man/m3_Dt/MmdbGrGD.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbGraphicGetInfo.3: ../guides/man/m3_Dt/MmdbGrGI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbInfoLibFreeInfo.3: ../guides/man/m3_Dt/MmdbILFI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbInfoLibGetInfo.3: ../guides/man/m3_Dt/MmdbILGI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbInit.3: ../guides/man/m3_Dt/MmdbInit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbLocatorGetSectionLoc.3: ../guides/man/m3_Dt/MmdbLGSL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbLocatorGetSectionObjectId.3: ../guides/man/m3_Dt/MmdbLGSO.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbOpenInfoLib.3: ../guides/man/m3_Dt/MmdbOpIL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbQuit.3: ../guides/man/m3_Dt/MmdbQuit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbSectionGetBookId.3: ../guides/man/m3_Dt/MmdbSGBI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbSectionGetData.3: ../guides/man/m3_Dt/MmdbSeGD.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbSectionGetDataSize.3: ../guides/man/m3_Dt/MmdbSGDS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbSectionGetLoc.3: ../guides/man/m3_Dt/MmdbSeGL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbSectionGetLongTitle.3: ../guides/man/m3_Dt/MmdbSGLT.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbSectionGetShortTitle.3: ../guides/man/m3_Dt/MmdbSGST.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbSectionGetStyleSheetId.3: ../guides/man/m3_Dt/MmdbSGSS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbSectionGetTocLoc.3: ../guides/man/m3_Dt/MmdbSGTL.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbStylesheetGetData.3: ../guides/man/m3_Dt/MmdbSsGD.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbStylesheetGetName.3: ../guides/man/m3_Dt/MmdbSsGN.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbTocGetChildIds.3: ../guides/man/m3_Dt/MmdbTGCI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbTocGetNumOfChildren.3: ../guides/man/m3_Dt/MmdbTGNC.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMmdbTocGetParentId.3: ../guides/man/m3_Dt/MmdbTGPI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMrmInitialize.3: ../guides/man/m3_Dt/MrmInit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMsgLogMessage.3: ../guides/man/m3_Dt/MsgLogM.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMsgLogOpenFile.3: ../guides/man/m3_Dt/MsgLogO.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtMsgLogSetHandler.3: ../guides/man/m3_Dt/MsgLogS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtPrintCopySetupData.3: ../guides/man/m3_Dt/PrintCop.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtPrintFillSetupData.3: ../guides/man/m3_Dt/PrintFil.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtPrintFreeSetupData.3: ../guides/man/m3_Dt/PrintFre.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtPrintResetConnection.3: ../guides/man/m3_Dt/PrintRes.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtPrintSetupBox.3: ../guides/man/m3_Dt/PrintSet.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtPrintSetupProc.3: ../guides/man/m3_Dt/PrSetupP.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSaverGetWindows.3: ../guides/man/m3_Dt/SaverGet.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchExit.3: ../guides/man/m3_Dt/SrchExit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchFreeResults.3: ../guides/man/m3_Dt/SrchFree.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchGetKeytypes.3: ../guides/man/m3_Dt/SrchGetK.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchGetMaxResults.3: ../guides/man/m3_Dt/SrchGetM.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchHighlight.3: ../guides/man/m3_Dt/SrchHigh.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchInit.3: ../guides/man/m3_Dt/SrchInit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchMergeResults.3: ../guides/man/m3_Dt/SrchMrge.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchHasMessages.3: ../guides/man/m3_Dt/SrchMsgs.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchQuery.3: ../guides/man/m3_Dt/SrchQery.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchReinit.3: ../guides/man/m3_Dt/SrchRein.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchRetrieve.3: ../guides/man/m3_Dt/SrchRetr.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchSetMaxResults.3: ../guides/man/m3_Dt/SrchSetM.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchSortResults.3: ../guides/man/m3_Dt/SrchSort.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSearchValidDateString.3: ../guides/man/m3_Dt/SrchVali.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSessionRestorePath.3: ../guides/man/m3_Dt/SessionR.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSessionSavePath.3: ../guides/man/m3_Dt/SessionS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSpinBox.3: ../guides/man/m3_Dt/SpinBox.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSpinBoxAddItem.3: ../guides/man/m3_Dt/SpinBoxA.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSpinBoxDeletePos.3: ../guides/man/m3_Dt/SpinBoxD.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSpinBoxSetItem.3: ../guides/man/m3_Dt/SpinBoxS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtSrAPI.3: ../guides/man/m3_Dt/SrAPI.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtTerm.3: ../guides/man/m3_Dt/Term.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtTermDisplaySend.3: ../guides/man/m3_Dt/TermDisp.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtTermInitialize.3: ../guides/man/m3_Dt/TermInit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtTermSubprocReap.3: ../guides/man/m3_Dt/TermSubR.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtTermSubprocSend.3: ../guides/man/m3_Dt/TermSubS.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmAddCurrentWorkspaceCallback.3: ../guides/man/m3_DtWsm/AddCWsCb.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmAddWorkspace.3: ../guides/man/m3_DtWsm/AddWs.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmAddWorkspaceFunctions.3: ../guides/man/m3_DtWsm/AddWsFun.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmAddWorkspaceModifiedCallback.3: ../guides/man/m3_DtWsm/AddWsMCb.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmChangeBackdrop.3: ../guides/man/m3_DtWsm/ChBackDr.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmDeleteWorkspace.3: ../guides/man/m3_DtWsm/DelWs.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmFreeWorkspaceInfo.3: ../guides/man/m3_DtWsm/FreeWsIn.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmGetCurrentBackdropWindow.3: ../guides/man/m3_DtWsm/GetCurBW.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmGetCurrentWorkspace.3: ../guides/man/m3_DtWsm/GetCurWs.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmGetWorkspaceInfo.3: ../guides/man/m3_DtWsm/GetWsInf.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmGetWorkspaceList.3: ../guides/man/m3_DtWsm/GetWsLis.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmGetWorkspacesOccupied.3: ../guides/man/m3_DtWsm/GetWsOcc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmOccupyAllWorkspaces.3: ../guides/man/m3_DtWsm/OccAllWs.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmRemoveWorkspaceCallback.3: ../guides/man/m3_DtWsm/RemWsCb.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmRemoveWorkspaceFunctions.3: ../guides/man/m3_DtWsm/RemWsFun.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmSetCurrentWorkspace.3: ../guides/man/m3_DtWsm/SetCurWs.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmSetWorkspacesOccupied.3: ../guides/man/m3_DtWsm/SetWsOcc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/DtWsmSetWorkspaceTitle.3: ../guides/man/m3_DtWsm/SetWsTi.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_add_calendar.3: ../guides/man/m3_csa/add_cale.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_add_entry.3: ../guides/man/m3_csa/add_entr.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_call_callbacks.3: ../guides/man/m3_csa/call_cal.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_delete_calendar.3: ../guides/man/m3_csa/del_cale.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_delete_entry.3: ../guides/man/m3_csa/del_entr.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_free.3: ../guides/man/m3_csa/free.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_free_time_search.3: ../guides/man/m3_csa/free_tim.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_list_calendar_attributes.3: ../guides/man/m3_csa/list_caa.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_list_calendars.3: ../guides/man/m3_csa/list_cal.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_list_entries.3: ../guides/man/m3_csa/list_ent.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_list_entry_attributes.3: ../guides/man/m3_csa/list_ena.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_list_entry_sequence.3: ../guides/man/m3_csa/list_ens.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_logoff.3: ../guides/man/m3_csa/logoff.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_logon.3: ../guides/man/m3_csa/logon.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_look_up.3: ../guides/man/m3_csa/look_up.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_query_configuration.3: ../guides/man/m3_csa/query_co.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_read_calendar_attributes.3: ../guides/man/m3_csa/read_cal.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_read_entry_attributes.3: ../guides/man/m3_csa/read_ent.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_read_next_reminder.3: ../guides/man/m3_csa/read_nex.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_register_callback.3: ../guides/man/m3_csa/register.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_restore.3: ../guides/man/m3_csa/restore.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_save.3: ../guides/man/m3_csa/save.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_unregister_callback.3: ../guides/man/m3_csa/unregist.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_update_calendar_attributes.3: ../guides/man/m3_csa/update_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_update_entry_attributes.3: ../guides/man/m3_csa/update_e.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/csa_x_process_updates.3: ../guides/man/m3_csa/x_proces.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_X_session.3: ../guides/man/m3_tt/X_sessio.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_bcontext_join.3: ../guides/man/m3_tt/bco_join.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_bcontext_quit.3: ../guides/man/m3_tt/bco_quit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_close.3: ../guides/man/m3_tt/close.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_context_join.3: ../guides/man/m3_tt/con_join.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_context_quit.3: ../guides/man/m3_tt/con_quit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_default_file.3: ../guides/man/m3_tt/def_file.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_default_file_set.3: ../guides/man/m3_tt/def_fi_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_default_procid.3: ../guides/man/m3_tt/def_proc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_default_procid_set.3: ../guides/man/m3_tt/def_pr_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_default_ptype.3: ../guides/man/m3_tt/def_ptyp.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_default_ptype_set.3: ../guides/man/m3_tt/def_pt_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_default_session.3: ../guides/man/m3_tt/def_sess.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_default_session_set.3: ../guides/man/m3_tt/def_se_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_error.3: ../guides/man/m3_tt/error.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_error_int.3: ../guides/man/m3_tt/err_int.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_error_pointer.3: ../guides/man/m3_tt/err_poin.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_fd.3: ../guides/man/m3_tt/fd.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_feature_enabled.3: ../guides/man/m3_tt/feat_ena.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_feature_required.3: ../guides/man/m3_tt/feat_req.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_file_copy.3: ../guides/man/m3_tt/fil_copy.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_file_destroy.3: ../guides/man/m3_tt/fil_dest.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_file_join.3: ../guides/man/m3_tt/fil_join.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_file_move.3: ../guides/man/m3_tt/fil_move.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_file_netfile.3: ../guides/man/m3_tt/fil_netf.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_file_objects_query.3: ../guides/man/m3_tt/fil_ob_q.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_file_quit.3: ../guides/man/m3_tt/fil_quit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_free.3: ../guides/man/m3_tt/free.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_host_file_netfile.3: ../guides/man/m3_tt/h_f_netf.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_host_netfile_file.3: ../guides/man/m3_tt/h_netf_f.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_icontext_join.3: ../guides/man/m3_tt/ico_join.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_icontext_quit.3: ../guides/man/m3_tt/ico_quit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_initial_session.3: ../guides/man/m3_tt/init_ses.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_int_error.3: ../guides/man/m3_tt/int_err.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_is_err.3: ../guides/man/m3_tt/is_err.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_malloc.3: ../guides/man/m3_tt/malloc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_mark.3: ../guides/man/m3_tt/mark.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_abstainer.3: ../guides/man/m3_tt_message/abstain.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_abstainers_count.3: ../guides/man/m3_tt_message/abstai_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_accept.3: ../guides/man/m3_tt_message/accept.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_accepter.3: ../guides/man/m3_tt_message/accepter.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_accepters_count.3: ../guides/man/m3_tt_message/accept_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_address.3: ../guides/man/m3_tt_message/address.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_address_set.3: ../guides/man/m3_tt_message/addr_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_add.3: ../guides/man/m3_tt_message/arg_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_bval.3: ../guides/man/m3_tt_message/arg_bval.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_bval_set.3: ../guides/man/m3_tt_message/arg_bv_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_ival.3: ../guides/man/m3_tt_message/arg_ival.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_ival_set.3: ../guides/man/m3_tt_message/arg_iv_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_mode.3: ../guides/man/m3_tt_message/arg_mode.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_type.3: ../guides/man/m3_tt_message/arg_type.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_val.3: ../guides/man/m3_tt_message/arg_val.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_val_set.3: ../guides/man/m3_tt_message/arg_va_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_xval.3: ../guides/man/m3_tt_message/arg_xval.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_arg_xval_set.3: ../guides/man/m3_tt_message/arg_xv_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_args_count.3: ../guides/man/m3_tt_message/args_cnt.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_barg_add.3: ../guides/man/m3_tt_message/barg_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_bcontext_set.3: ../guides/man/m3_tt_message/bcon_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_callback_add.3: ../guides/man/m3_tt_message/call_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_class.3: ../guides/man/m3_tt_message/class.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_class_set.3: ../guides/man/m3_tt_message/class_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_context_bval.3: ../guides/man/m3_tt_message/con_bval.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_context_ival.3: ../guides/man/m3_tt_message/con_ival.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_context_set.3: ../guides/man/m3_tt_message/con_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_context_slotname.3: ../guides/man/m3_tt_message/con_slot.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_context_val.3: ../guides/man/m3_tt_message/con_val.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_context_xval.3: ../guides/man/m3_tt_message/con_xval.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_contexts_count.3: ../guides/man/m3_tt_message/cons_cnt.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_create.3: ../guides/man/m3_tt_message/create.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_create_super.3: ../guides/man/m3_tt_message/cr_super.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_destroy.3: ../guides/man/m3_tt_message/destroy.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_disposition.3: ../guides/man/m3_tt_message/disposit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_disposition_set.3: ../guides/man/m3_tt_message/disp_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_fail.3: ../guides/man/m3_tt_message/fail.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_file.3: ../guides/man/m3_tt_message/file.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_file_set.3: ../guides/man/m3_tt_message/file_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_gid.3: ../guides/man/m3_tt_message/gid.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_handler.3: ../guides/man/m3_tt_message/handler.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_handler_ptype.3: ../guides/man/m3_tt_message/han_pty.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_handler_ptype_set.3: ../guides/man/m3_tt_message/han_pt_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_handler_set.3: ../guides/man/m3_tt_message/han_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_iarg_add.3: ../guides/man/m3_tt_message/iarg_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_icontext_set.3: ../guides/man/m3_tt_message/ico_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_id.3: ../guides/man/m3_tt_message/id.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_object.3: ../guides/man/m3_tt_message/object.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_object_set.3: ../guides/man/m3_tt_message/object_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_op.3: ../guides/man/m3_tt_message/op.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_op_set.3: ../guides/man/m3_tt_message/op_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_opnum.3: ../guides/man/m3_tt_message/opnum.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_otype.3: ../guides/man/m3_tt_message/otype.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_otype_set.3: ../guides/man/m3_tt_message/otype_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_pattern.3: ../guides/man/m3_tt_message/pattern.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_print.3: ../guides/man/m3_tt_message/print.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_receive.3: ../guides/man/m3_tt_message/receive.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_reject.3: ../guides/man/m3_tt_message/reject.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_rejecter.3: ../guides/man/m3_tt_message/rejecter.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_rejecters_count.3: ../guides/man/m3_tt_message/reject_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_reply.3: ../guides/man/m3_tt_message/reply.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_scope.3: ../guides/man/m3_tt_message/scope.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_scope_set.3: ../guides/man/m3_tt_message/scope_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_send.3: ../guides/man/m3_tt_message/send.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_send_on_exit.3: ../guides/man/m3_tt_message/snd_on_e.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_sender.3: ../guides/man/m3_tt_message/sender.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_sender_ptype.3: ../guides/man/m3_tt_message/snd_pty.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_sender_ptype_set.3: ../guides/man/m3_tt_message/snd_pt_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_session.3: ../guides/man/m3_tt_message/session.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_session_set.3: ../guides/man/m3_tt_message/sessio_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_state.3: ../guides/man/m3_tt_message/state.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_status.3: ../guides/man/m3_tt_message/status.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_status_set.3: ../guides/man/m3_tt_message/status_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_status_string.3: ../guides/man/m3_tt_message/st_str.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_status_string_set.3: ../guides/man/m3_tt_message/st_str_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_uid.3: ../guides/man/m3_tt_message/uid.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_user.3: ../guides/man/m3_tt_message/user.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_user_set.3: ../guides/man/m3_tt_message/user_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_xarg_add.3: ../guides/man/m3_tt_message/xarg_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_message_xcontext_set.3: ../guides/man/m3_tt_message/xco_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_netfile_file.3: ../guides/man/m3_tt/netf_fil.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_objid_equal.3: ../guides/man/m3_tt/objid_eq.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_objid_objkey.3: ../guides/man/m3_tt/objid_ob.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_onotice_create.3: ../guides/man/m3_tt/onot_cre.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_open.3: ../guides/man/m3_tt/open.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_orequest_create.3: ../guides/man/m3_tt/oreq_cre.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_base.3: ../guides/man/m3_tt_otype/base.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_derived.3: ../guides/man/m3_tt_otype/derived.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_deriveds_count.3: ../guides/man/m3_tt_otype/deri_cnt.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_hsig_arg_mode.3: ../guides/man/m3_tt_otype/h_arg_mo.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_hsig_arg_type.3: ../guides/man/m3_tt_otype/h_arg_ty.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_hsig_args_count.3: ../guides/man/m3_tt_otype/h_args_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_hsig_count.3: ../guides/man/m3_tt_otype/h_count.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_hsig_op.3: ../guides/man/m3_tt_otype/hsig_op.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_is_derived.3: ../guides/man/m3_tt_otype/is_deriv.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_opnum_callback_add.3: ../guides/man/m3_tt_otype/o_cb_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_osig_arg_mode.3: ../guides/man/m3_tt_otype/o_arg_mo.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_osig_arg_type.3: ../guides/man/m3_tt_otype/o_arg_ty.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_osig_args_count.3: ../guides/man/m3_tt_otype/o_args_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_osig_count.3: ../guides/man/m3_tt_otype/o_count.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_otype_osig_op.3: ../guides/man/m3_tt_otype/osig_op.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_address_add.3: ../guides/man/m3_tt_pattern/addr_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_arg_add.3: ../guides/man/m3_tt_pattern/arg_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_barg_add.3: ../guides/man/m3_tt_pattern/barg_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_bcontext_add.3: ../guides/man/m3_tt_pattern/bcon_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_callback_add.3: ../guides/man/m3_tt_pattern/call_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_category.3: ../guides/man/m3_tt_pattern/category.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_category_set.3: ../guides/man/m3_tt_pattern/cat_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_class_add.3: ../guides/man/m3_tt_pattern/clas_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_context_add.3: ../guides/man/m3_tt_pattern/cont_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_create.3: ../guides/man/m3_tt_pattern/create.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_destroy.3: ../guides/man/m3_tt_pattern/destroy.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_disposition_add.3: ../guides/man/m3_tt_pattern/disp_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_file_add.3: ../guides/man/m3_tt_pattern/file_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_iarg_add.3: ../guides/man/m3_tt_pattern/iarg_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_icontext_add.3: ../guides/man/m3_tt_pattern/icon_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_object_add.3: ../guides/man/m3_tt_pattern/obj_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_op_add.3: ../guides/man/m3_tt_pattern/op_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_opnum_add.3: ../guides/man/m3_tt_pattern/opnu_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_otype_add.3: ../guides/man/m3_tt_pattern/otyp_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_print.3: ../guides/man/m3_tt_pattern/print.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_register.3: ../guides/man/m3_tt_pattern/register.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_scope_add.3: ../guides/man/m3_tt_pattern/scop_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_sender_add.3: ../guides/man/m3_tt_pattern/send_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_sender_ptype_add.3: ../guides/man/m3_tt_pattern/s_pt_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_session_add.3: ../guides/man/m3_tt_pattern/sess_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_state_add.3: ../guides/man/m3_tt_pattern/stat_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_unregister.3: ../guides/man/m3_tt_pattern/unregist.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_user.3: ../guides/man/m3_tt_pattern/user.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_user_set.3: ../guides/man/m3_tt_pattern/user_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_xarg_add.3: ../guides/man/m3_tt_pattern/xarg_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pattern_xcontext_add.3: ../guides/man/m3_tt_pattern/xcon_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pnotice_create.3: ../guides/man/m3_tt/pnot_cre.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_pointer_error.3: ../guides/man/m3_tt/poin_err.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_prequest_create.3: ../guides/man/m3_tt/preq_cre.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_procid_session.3: ../guides/man/m3_tt/pr_sess.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_ptr_error.3: ../guides/man/m3_tt/ptr_err.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_ptype_declare.3: ../guides/man/m3_tt/pty_decl.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_ptype_exists.3: ../guides/man/m3_tt/pty_exis.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_ptype_opnum_callback_add.3: ../guides/man/m3_tt/pt_o_c_a.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_ptype_undeclare.3: ../guides/man/m3_tt/pty_unde.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_release.3: ../guides/man/m3_tt/release.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_bprop.3: ../guides/man/m3_tt_session/bprop.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_bprop_add.3: ../guides/man/m3_tt_session/bpro_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_bprop_set.3: ../guides/man/m3_tt_session/bpro_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_join.3: ../guides/man/m3_tt_session/join.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_prop.3: ../guides/man/m3_tt_session/prop.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_prop_add.3: ../guides/man/m3_tt_session/prop_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_prop_count.3: ../guides/man/m3_tt_session/prop_cnt.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_prop_set.3: ../guides/man/m3_tt_session/prop_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_propname.3: ../guides/man/m3_tt_session/propname.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_propnames_count.3: ../guides/man/m3_tt_session/propna_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_quit.3: ../guides/man/m3_tt_session/quit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_session_types_load.3: ../guides/man/m3_tt_session/types_ld.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_bprop.3: ../guides/man/m3_tt_spec/bprop.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_bprop_add.3: ../guides/man/m3_tt_spec/bpro_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_bprop_set.3: ../guides/man/m3_tt_spec/bpro_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_create.3: ../guides/man/m3_tt_spec/create.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_destroy.3: ../guides/man/m3_tt_spec/destroy.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_file.3: ../guides/man/m3_tt_spec/file.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_move.3: ../guides/man/m3_tt_spec/move.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_prop.3: ../guides/man/m3_tt_spec/prop.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_prop_add.3: ../guides/man/m3_tt_spec/prop_add.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_prop_count.3: ../guides/man/m3_tt_spec/prop_cnt.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_prop_set.3: ../guides/man/m3_tt_spec/prop_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_propname.3: ../guides/man/m3_tt_spec/propname.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_propnames_count.3: ../guides/man/m3_tt_spec/propna_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_type.3: ../guides/man/m3_tt_spec/type.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_type_set.3: ../guides/man/m3_tt_spec/type_set.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_spec_write.3: ../guides/man/m3_tt_spec/write.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_status_message.3: ../guides/man/m3_tt/status_m.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_thread_procid.3: ../guides/man/m3_tt/thr_proc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_thread_procid_set.3: ../guides/man/m3_tt/thr_pr_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_thread_session.3: ../guides/man/m3_tt/thr_sess.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_thread_session_set.3: ../guides/man/m3_tt/thr_se_s.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_trace_control.3: ../guides/man/m3_tt/trace_co.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_xcontext_join.3: ../guides/man/m3_tt/xco_join.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tt_xcontext_quit.3: ../guides/man/m3_tt/xco_quit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_Get_Modified.3: ../guides/man/m3_ttdt/Get_Modi.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_Revert.3: ../guides/man/m3_ttdt/Revert.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_Save.3: ../guides/man/m3_ttdt/Save.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_close.3: ../guides/man/m3_ttdt/close.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_file_event.3: ../guides/man/m3_ttdt/f_event.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_file_join.3: ../guides/man/m3_ttdt/f_join.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_file_notice.3: ../guides/man/m3_ttdt/f_notice.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_file_quit.3: ../guides/man/m3_ttdt/f_quit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_file_request.3: ../guides/man/m3_ttdt/f_reques.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_message_accept.3: ../guides/man/m3_ttdt/m_accept.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_open.3: ../guides/man/m3_ttdt/open.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_sender_imprint_on.3: ../guides/man/m3_ttdt/s_imp_on.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_session_join.3: ../guides/man/m3_ttdt/ses_join.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_session_quit.3: ../guides/man/m3_ttdt/ses_quit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttdt_subcontract_manage.3: ../guides/man/m3_ttdt/subc_man.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttmedia_Deposit.3: ../guides/man/m3_ttmedia/Deposit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttmedia_load.3: ../guides/man/m3_ttmedia/load.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttmedia_load_reply.3: ../guides/man/m3_ttmedia/load_rep.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/ttmedia_ptype_declare.3: ../guides/man/m3_ttmedia/pty_decl.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tttk_Xt_input_handler.3: ../guides/man/m3_tttk/Xt_inp_h.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tttk_block_while.3: ../guides/man/m3_tttk/block_wh.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tttk_message_abandon.3: ../guides/man/m3_tttk/m_abando.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tttk_message_create.3: ../guides/man/m3_tttk/m_create.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tttk_message_destroy.3: ../guides/man/m3_tttk/m_destro.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tttk_message_fail.3: ../guides/man/m3_tttk/m_fail.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tttk_message_reject.3: ../guides/man/m3_tttk/m_reject.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tttk_op_string.3: ../guides/man/m3_tttk/op_str.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man3/tttk_string_op.3: ../guides/man/m3_tttk/str_op.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/BIL.4: ../guides/man/man4/bil.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Deposit.4: ../guides/man/man4/Deposit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Display.4: ../guides/man/man4/Display.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/DtDndProtocol.4: ../guides/man/man4/DtDndPro.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/DtInfo_LoadInfoLib.4: ../guides/man/man4/ifloadil.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/DtInfo_PrintInfoAtLoc.4: ../guides/man/man4/ifprnial.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/DtInfo_Quit.4: ../guides/man/man4/ifquit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/DtInfo_ShowInfoAtLoc.4: ../guides/man/man4/ifshwial.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Edit.4: ../guides/man/man4/Edit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_Environment.4: ../guides/man/man4/Get_Env.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_Geometry.4: ../guides/man/man4/Get_Geom.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_Iconified.4: ../guides/man/man4/Get_Icon.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_Locale.4: ../guides/man/man4/Get_Loca.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_Mapped.4: ../guides/man/man4/Get_Map.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_Modified.4: ../guides/man/man4/Get_Modi.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_Situation.4: ../guides/man/man4/Get_Situ.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_Status.4: ../guides/man/man4/Get_Stat.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_Sysinfo.4: ../guides/man/man4/Get_Sysi.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Get_XInfo.4: ../guides/man/man4/Get_XInf.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Instantiate.4: ../guides/man/man4/Instanti.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Lower.4: ../guides/man/man4/Lower.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Mail.4: ../guides/man/man4/Mail.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Modified.4: ../guides/man/man4/Modified.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Pause.4: ../guides/man/man4/Pause.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Print.4: ../guides/man/man4/Print.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Quit.4: ../guides/man/man4/Quit.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Raise.4: ../guides/man/man4/Raise.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Resume.4: ../guides/man/man4/Resume.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Revert.4: ../guides/man/man4/Revert.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Reverted.4: ../guides/man/man4/Reverted.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Save.4: ../guides/man/man4/Save.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Saved.4: ../guides/man/man4/Saved.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Set_Environment.4: ../guides/man/man4/Set_Env.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Set_Geometry.4: ../guides/man/man4/Set_Geom.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Set_Iconified.4: ../guides/man/man4/Set_Icon.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Set_Locale.4: ../guides/man/man4/Set_Loca.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Set_Mapped.4: ../guides/man/man4/Set_Map.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Set_Situation.4: ../guides/man/man4/Set_Situ.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Signal.4: ../guides/man/man4/Signal.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Started.4: ../guides/man/man4/Started.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Status.4: ../guides/man/man4/Status.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Stopped.4: ../guides/man/man4/Stopped.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/Translate.4: ../guides/man/man4/Translat.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtactionfile.4: ../guides/man/man4/dtaction.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtbmfile.4: ../guides/man/man4/dtbmfile.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtcm_archive.4: ../guides/man/man4/dtcm_arc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtcm_entry.4: ../guides/man/man4/dtcm_ent.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtdpfile.4: ../guides/man/man4/dtdpfile.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtdtfile.4: ../guides/man/man4/dtdtfile.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtdtsfile.4: ../guides/man/man4/dtdtsfil.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtfile_config.4: ../guides/man/man4/dtfile_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtfpfile.4: ../guides/man/man4/dtfpfile.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dthelptagdtd.4: ../guides/man/man4/dthelpta.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dthffile.4: ../guides/man/man4/dthffile.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dticonfile.4: ../guides/man/man4/dticonfi.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtlocator.4: ../guides/man/man4/genloc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtpmfile.4: ../guides/man/man4/dtpmfile.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtresourcesfile.4: ../guides/man/man4/dtresour.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtsdldtd.4: ../guides/man/man4/dtsdldtd.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtsdlfile.4: ../guides/man/man4/dtsdlfil.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtsessiondb.4: ../guides/man/man4/dtsessdb.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtsessionfile.4: ../guides/man/man4/dtsessio.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtspcdenv.4: ../guides/man/man4/dtspcden.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtsrdbfiles.4: ../guides/man/man4/dtsrdbfl.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtsrfzkfiles.4: ../guides/man/man4/dtsrfzkf.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtsrhanfile.4: ../guides/man/man4/dtsrhanf.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtsrlangfiles.4: ../guides/man/man4/dtsrlngf.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtsrocffile.4: ../guides/man/man4/dtsrocfl.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/dtwmrc.4: ../guides/man/man4/dtwmrc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/mt-libtt-intro.4: ../guides/man/man4/mt-libtt.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/startlog.4: ../guides/man/man4/startlog.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man4/tttracefile.4: ../guides/man/man4/tttracef.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/Dt.5: ../guides/man/man5/Dt.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtAction.5: ../guides/man/man5/DtAction.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtComboBox.5: ../guides/man/man5/DtComboB.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtDnd.5: ../guides/man/man5/DtDnd.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtDts.5: ../guides/man/man5/DtDts.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtEditor.5: ../guides/man/man5/DtEditor.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtHelp.5: ../guides/man/man5/DtHelp.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtHelpDialog.5: ../guides/man/man5/DtHelpDi.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtHelpQuickD.5: ../guides/man/man5/DtHelpQD.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtInfoLib.5: ../guides/man/man5/DtInfoLib.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtMenuButton.5: ../guides/man/man5/DtMenuBu.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtMmdbBookCase.5: ../guides/man/man5/dtinfobc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtMmdbGraphicInfo.5: ../guides/man/man5/mmdbgifo.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtMmdbHandle.5: ../guides/man/man5/mmdbhndl.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtMmdbInfoLibInfo.5: ../guides/man/man5/dtinfdlb.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtMmdbInfoRequest.5: ../guides/man/man5/mmdbireq.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtMrm.5: ../guides/man/man5/DtMrm.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtMsgLog.5: ../guides/man/man5/DtMsgLog.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtPrint.5: ../guides/man/man5/DtPrint.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtSaver.5: ../guides/man/man5/DtSaver.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtSearch.5: ../guides/man/man5/dtsearch.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtSession.5: ../guides/man/man5/DtSessio.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtSpinBox.5: ../guides/man/man5/DtSpinBo.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtStdAppFontNames.5: ../guides/man/man5/DtStdApp.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtStdInterfaceFontNames.5: ../guides/man/man5/DtStdInt.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtTerm.5: ../guides/man/man5/DtTerm.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/DtWsm.5: ../guides/man/man5/DtWsm.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/Tttt_c.5: ../guides/man/man5/Tttt_c.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/Tttttk.5: ../guides/man/man5/Tttttk.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/csacsa.5: ../guides/man/man5/csacsa.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtactionaction.5: ../guides/man/man5/dtactact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtappaction.5: ../guides/man/man5/dtappact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtbuilderaction.5: ../guides/man/man5/dtbuiact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtcalcaction.5: ../guides/man/man5/dtclcact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtcalendaraction.5: ../guides/man/man5/dtcalact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtenvvar.5: ../guides/man/man5/dtenvvar.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtfileaction.5: ../guides/man/man5/dtfilact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtfilsys.5: ../guides/man/man5/dtfilsys.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dthelpaction.5: ../guides/man/man5/dthelact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dticonaction.5: ../guides/man/man5/dticoact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtimsstartaction.5: ../guides/man/man5/imssact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtinfoBookdtd.5: ../guides/man/man5/dtinfobk.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtinfoStyledtd.5: ../guides/man/man5/dtinfost.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtinfoTOCdtd.5: ../guides/man/man5/dtinfotc.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtinfoaction.5: ../guides/man/man5/dtinfact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtinfoaf.5: ../guides/man/man5/dtinfoaf.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtmailaction.5: ../guides/man/man5/dtmaiact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtmanaction.5: ../guides/man/man5/dtmanact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtprintinfoaction.5: ../guides/man/man5/dtpriact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtsessionaction.5: ../guides/man/man5/dtsesact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtstyleaction.5: ../guides/man/man5/dtstyact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dtterm.5: ../guides/man/man5/dttermes.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dttermaction.5: ../guides/man/man5/dtteract.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dttextaction.5: ../guides/man/man5/dttexact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man5/dttrashaction.5: ../guides/man/man5/dttraact.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

man6/ttsnoop.6: ../guides/man/man6/ttsnoop.sgm $(MANDEPS)
	$(DBTOMAN) cdedecl.sgm $< $@

# man alias rules
man3/DtInitialize.3: man3/DtAppInitialize.3
	echo .so $< >$@

man3/DtDndVaDragStart.3: man3/DtDndDragStart.3
	echo .so $< >$@

man3/DtDndVaDropRegister.3: man3/DtDndDropRegister.3
	echo .so $< >$@

man3/DtSearchAddMessage.3: man3/DtSearchHasMessages.3
	echo .so $< >$@

man3/DtSearchFreeMessages.3: man3/DtSearchHasMessages.3
	echo .so $< >$@

man3/DtSearchGetMessages.3: man3/DtSearchHasMessages.3
	echo .so $< >$@

