bin_PROGRAMS = dtudcexch

FONTEDT_PATH1 = ../dtudcfonted

dtudcexch_CFLAGS = -I$(FONTEDT_PATH1) -I$(FONTEDT_PATH1)/include \
		   -I$(FONTEDT_PATH1)/libfal -I$(FONTEDT_PATH1)/dtgpftobdf \
		   -DFONTC='"'$(BDFTOPCF)'"'

dtudcexch_LDADD = $(LIBSVC) $(LIBTT) $(XTOOLLIB) ${X_LIBS} \
                $(FONTEDT_PATH1)/libfuty/liboakfuty.a \
                $(FONTEDT_PATH1)/libfal/libfal.a

if SOLARIS
dtudcexch_LDADD += -lgen
endif

if FREEBSD
dtudcexch_LDADD += -liconv
endif

dtudcexch_SOURCES = udcexc.c udcexp.c udcimp.c getbdffn.c excutil.c \
                    selectx.c exportbdf.c importbdf.c
