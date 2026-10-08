MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libstt.a

libstt_a_CXXFLAGS = $(TIRPC_DEFINES) -I../lib

libstt_a_SOURCES = mp_ce_attrs.C mp_observer.C mp_observer_utils.C  \
mp_otype.C              mp_otype_utils.C           mp_ptype.C           \
mp_ptype_utils.C        mp_rpc_implement.C         mp_rpc_server.C      \
mp_rpc_server_utils.C                                                   \
mp_s_file.C             mp_s_file_utils.C                               \
mp_s_message.C          mp_s_message_utils.C       mp_s_mp.C            \
mp_s_pattern.C          mp_s_pattern_utils.C       mp_s_procid.C        \
mp_s_procid_utils.C     mp_s_msg_context.C         mp_s_pat_context.C   \
mp_s_session.C          mp_s_session_prop.C        mp_s_session_utils.C \
mp_s_xdr_functions.C    mp_self_procid.C                                \
mp_signature.C          mp_signature_utils.C       mp_typedb.C          \
mp_typedb_utils.C       tt_isstrerror.C
