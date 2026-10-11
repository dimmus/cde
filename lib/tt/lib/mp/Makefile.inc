noinst_LTLIBRARIES += %reldir%/libmp.la

%canon_reldir%_libmp_la_CXXFLAGS = $(CDE_CXXFLAGS) -I$(top_srcdir)/lib/tt/mini_isam

%canon_reldir%_libmp_la_SOURCES = %reldir%/mp_arg.C \
	%reldir%/mp_arg_utils.C %reldir%/mp_auth.C %reldir%/mp_auth_functions.C \
	%reldir%/mp_c_file.C %reldir%/mp_c_file_utils.C %reldir%/mp_c_message.C \
	%reldir%/mp_c_message_utils.C %reldir%/mp_c_mp.C \
	%reldir%/mp_c_msg_context.C %reldir%/mp_c_msg_context_utils.C \
	%reldir%/mp_c_pattern.C %reldir%/mp_c_procid.C \
	%reldir%/mp_c_procid_utils.C %reldir%/mp_c_session.C \
	%reldir%/mp_c_session_prop.C %reldir%/mp_c_session_utils.C \
	%reldir%/mp_context.C %reldir%/mp_context_utils.C %reldir%/mp_desktop.C \
	%reldir%/mp_desktop_utils.C %reldir%/mp_file.C %reldir%/mp_file_utils.C \
	%reldir%/mp_message.C %reldir%/mp_message_utils.C %reldir%/mp_mp.C \
	%reldir%/mp_mp_utils.C %reldir%/mp_msg_context.C \
	%reldir%/mp_msg_context_utils.C %reldir%/mp_pat_context.C \
	%reldir%/mp_pat_context_utils.C %reldir%/mp_pattern.C \
	%reldir%/mp_pattern_utils.C %reldir%/mp_procid.C \
	%reldir%/mp_procid_utils.C %reldir%/mp_rpc_client.C \
	%reldir%/mp_rpc_client_utils.C %reldir%/mp_rpc_fns.C \
	%reldir%/mp_session.C %reldir%/mp_session_prop.C \
	%reldir%/mp_session_prop_utils.C %reldir%/mp_session_utils.C \
	%reldir%/mp_stream_socket.C %reldir%/mp_stream_socket_utils.C \
	%reldir%/mp_trace.C %reldir%/mp_xdr_functions.C
