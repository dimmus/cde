MAINTAINERCLEANFILES = Makefile.in

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

bin_PROGRAMS = ttdbck

ttdbck_LDADD = ../../slib/libstt.a ../../mini_isam/libisam.a @LIBTT@ \
	$(TIRPCLIB) $(XTOOLLIB)

ttdbck_CXXFLAGS = -I../../slib -I../../lib -I../../mini_isam \
		   $(TT_VERSION_DEFINE)

ttdbck_SOURCES = binkey.C        binkey_utils.C  common.C \
		 options.C       options_tt.C    prop.C \
		 prop_utils.C    spec.C          spec_repair.C \
		 spec_utils.C    ttdbck.C
