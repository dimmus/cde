noinst_LTLIBRARIES += %reldir%/libdb.la

%canon_reldir%_libdb_la_CXXFLAGS = $(CDE_CXXFLAGS) -I$(top_srcdir)/lib/tt/mini_isam

%canon_reldir%_libdb_la_SOURCES = %reldir%/db_server_clnt.C \
	%reldir%/db_server_xdr.C %reldir%/old_db_server_functions.C \
	%reldir%/tt_client_isam_file.C %reldir%/tt_client_isam_file_utils.C \
	%reldir%/tt_client_isam_key_descriptor.C \
	%reldir%/tt_client_isam_key_descriptor_utils.C \
	%reldir%/tt_client_isam_record.C %reldir%/tt_client_isam_record_utils.C \
	%reldir%/tt_db_access_utils.C %reldir%/tt_db_client.C \
	%reldir%/tt_db_client_utils.C %reldir%/tt_db_create_objid.C \
	%reldir%/tt_db_file.C %reldir%/tt_db_file_utils.C \
	%reldir%/tt_db_hostname_global_map_ref.C \
	%reldir%/tt_db_hostname_redirection_map.C %reldir%/tt_db_key.C \
	%reldir%/tt_db_key_utils.C %reldir%/tt_db_network_path.C \
	%reldir%/tt_db_object.C %reldir%/tt_db_object_utils.C \
	%reldir%/tt_db_objid_to_key.C %reldir%/tt_db_property.C \
	%reldir%/tt_db_property_utils.C %reldir%/tt_db_rpc_message_routines.C \
	%reldir%/tt_db_rpc_routines.C %reldir%/tt_old_db.C \
	%reldir%/tt_old_db_message_info.C \
	%reldir%/tt_old_db_message_info_utils.C \
	%reldir%/tt_old_db_partition_map_ref.C %reldir%/tt_old_db_utils.C
