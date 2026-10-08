MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libRFC.a

libRFC_a_CXXFLAGS = -I../Common -I../../include -I../../include/utils \
		    -DTTLOCK_OFF

if SOLARIS
libRFC_a_CXXFLAGS += -DMMAP_NORESERVE
endif

libRFC_a_SOURCES = AliasExpand.C           MIMEBodyPart.C \
		    MIMEPartial.C           RFCBodyPart.C \
		    RFCEnvelope.C           RFCFormat.C \
		    RFCMIME.C               RFCMailBox.C \
		    RFCMailValues.C         RFCMessage.C \
		    RFCTransport.C          SunV3.C \
		    V3BodyPart.C
