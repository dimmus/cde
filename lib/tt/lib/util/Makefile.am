MAINTAINERCLEANFILES = Makefile.in

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

noinst_LTLIBRARIES = libutil.la

libutil_la_CXXFLAGS = $(TT_VERSION_DEFINE) -I../../lib 

libutil_la_SOURCES = copyright.C	    tt_map_entry_utils.C \
	tt_Xlib.C			    tt_new.C \
	tt_audit.C			    tt_new_ptr.C \
	tt_base64.C			    tt_object.C \
	tt_enumname.C			    tt_object_list.C \
	tt_file_system.C		    tt_object_table.C \
	tt_file_system_entry_utils.C	    tt_path.C \
	tt_gettext.C			    tt_port.C \
	tt_global_env.C 		    tt_string.C \
	tt_host.C			    tt_string_map.C \
	tt_host_equiv.C 		    tt_string_map_utils.C \
	tt_host_equiv_utils.C		    tt_string_match.C \
	tt_host_utils.C 		    tt_threadspecific.C \
	tt_int_rec.C			    tt_trace.C \
	tt_iostream.C			    tt_trace_parser.C \
	tt_ldpath.C			    tt_trace_stream.C \
	tt_log.C			    tt_tracefile_parse.C \
	tt_map_entry.C			    tt_xdr_utils.C \
	tt_entry_pt.C

