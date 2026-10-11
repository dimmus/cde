noinst_LTLIBRARIES += %reldir%/libutil.la

%canon_reldir%_libutil_la_CXXFLAGS = $(CDE_CXXFLAGS) $(TT_VERSION_DEFINE)

%canon_reldir%_libutil_la_SOURCES = %reldir%/copyright.C \
	%reldir%/tt_map_entry_utils.C %reldir%/tt_Xlib.C %reldir%/tt_new.C \
	%reldir%/tt_audit.C %reldir%/tt_new_ptr.C %reldir%/tt_base64.C \
	%reldir%/tt_object.C %reldir%/tt_enumname.C %reldir%/tt_object_list.C \
	%reldir%/tt_file_system.C %reldir%/tt_object_table.C \
	%reldir%/tt_file_system_entry_utils.C %reldir%/tt_path.C \
	%reldir%/tt_gettext.C %reldir%/tt_port.C %reldir%/tt_global_env.C \
	%reldir%/tt_string.C %reldir%/tt_host.C %reldir%/tt_string_map.C \
	%reldir%/tt_host_equiv.C %reldir%/tt_string_map_utils.C \
	%reldir%/tt_host_equiv_utils.C %reldir%/tt_string_match.C \
	%reldir%/tt_host_utils.C %reldir%/tt_threadspecific.C \
	%reldir%/tt_int_rec.C %reldir%/tt_trace.C %reldir%/tt_iostream.C \
	%reldir%/tt_trace_parser.C %reldir%/tt_ldpath.C \
	%reldir%/tt_trace_stream.C %reldir%/tt_log.C \
	%reldir%/tt_tracefile_parse.C %reldir%/tt_map_entry.C \
	%reldir%/tt_xdr_utils.C %reldir%/tt_entry_pt.C %reldir%/tt_stats.C
