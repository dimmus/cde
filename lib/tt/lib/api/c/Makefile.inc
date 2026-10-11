noinst_LTLIBRARIES += %reldir%/libapi.la

%canon_reldir%_libapi_la_CXXFLAGS = $(CDE_CXXFLAGS)

%canon_reldir%_libapi_la_SOURCES = %reldir%/api_api.C %reldir%/api_auth.C \
	%reldir%/api_default.C %reldir%/api_error.C %reldir%/api_file.C \
	%reldir%/api_filemap.C %reldir%/api_handle.C %reldir%/api_message.C \
	%reldir%/api_mp.C %reldir%/api_mp_io.C %reldir%/api_mp_otype.C \
	%reldir%/api_objid_spec.C %reldir%/api_pattern.C \
	%reldir%/api_session_prop.C %reldir%/api_spec.C \
	%reldir%/api_spec_map_ref.C %reldir%/api_storage.C %reldir%/api_typecb.C \
	%reldir%/api_typecb_utils.C %reldir%/api_utils.C %reldir%/api_xdr.C
