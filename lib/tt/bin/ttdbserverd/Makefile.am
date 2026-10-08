MAINTAINERCLEANFILES = Makefile.in

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

bin_PROGRAMS = rpc.ttdbserver

rpc_ttdbserver_CXXFLAGS = -I../../slib -I../../lib -I../../mini_isam \
			$(TIRPC_DEFINES) $(TT_VERSION_DEFINE)

rpc_ttdbserver_LDADD = $(LIBTT) ${TIRPCLIB} $(X_LIBS) ../../slib/libstt.a \
		       ../../mini_isam/libisam.a

if LINUX
rpc_ttdbserver_LDADD += $(TIRPCLIB) $(XTOOLLIB)
endif

if SOLARIS
rpc_ttdbserver_LDADD += $(XTOOLLIB) -lintl -lsocket -lnsl
endif

if BSD
rpc_ttdbserver_LDADD += $(XTOOLLIB)
endif

rpc_ttdbserver_SOURCES = db_server_functions.C \
			 db_server_svc.C \
			 dm_access_cache.C \
			 dm_server.C \
			 tt_db_message_info_utils.C \
			 tt_db_msg_q_lock.C \
			 tt_db_msg_q_lock_entry_utils.C \
			 tt_db_partition_global_map_ref.C \
			 tt_db_partition_redirection_map.C \
			 tt_db_server_db.C \
			 tt_db_server_db_utils.C \
			 tt_isam_file.C \
			 tt_isam_file_utils.C \
			 tt_isam_key_descriptor.C \
			 tt_isam_key_descriptor_utils.C \
			 tt_isam_record.C \
			 tt_isam_record_utils.C
