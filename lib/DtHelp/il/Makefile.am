MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libil.la

libil_la_CFLAGS = -I.. -DDTLIB -DCDE_INSTALLATION_TOP='"$(CDE_INSTALLATION_TOP)"' \
		  -DCDE_CONFIGURATION_TOP='"$(CDE_CONFIGURATION_TOP)"' \
		  @DT_INCDIR@ @DTI_INCDIR@

libil_la_SOURCES = ilX.c                ilbigray.c           ilcodec.c \
		   ilcontext.c          ilconvert.c          ilcrop.c \
		   ildata.c             ildecompg3.c         ildecompg4.c \
		   ildecompjpeg.c       ildecomplzw.c        ildecomppackbits.c \
		   ildecompress.c       ildither.c           ilformat.c \
		   ilgraybi.c           ilimage.c            ilinvert.c \
		   iljpgdecode.c        iljpgdecodejif.c     iljpgdedct.c \
		   iljpgdehuff.c        iljpgutil.c          ilmap.c \
		   ilobject.c           ilpalette.c          ilpipe.c \
		   ilrwimage.c          ilscale.c            ilscalesample.c \
		   iltiff.c             iltiffread.c         iltruegray.c \
		   ilupsample.c         ilutiljpeg.c         ilycbcr.c
