AM_CFLAGS = $(CFLAG_VISIBILITY)
AM_CPPFLAGS = -I$(top_srcdir)/include -DBUILDING_LIBAEC
lib_LTLIBRARIES = libaec.la libsz.la
libaec_la_SOURCES = encode.c encode_accessors.c decode.c \
encode.h encode_accessors.h decode.h
libaec_la_LDFLAGS = -version-info 0:12:0 -no-undefined

libsz_la_SOURCES = sz_compat.c
libsz_la_LIBADD = libaec.la
libsz_la_LDFLAGS = -version-info 2:1:0 -no-undefined

include_HEADERS = $(top_srcdir)/include/libaec.h $(top_srcdir)/include/szlib.h

bin_PROGRAMS = aec
noinst_PROGRAMS = utime
utime_SOURCES = utime.c
aec_LDADD = libaec.la
aec_SOURCES = aec.c
dist_man_MANS = aec.1

EXTRA_DIST = CMakeLists.txt benc.sh bdec.sh
CLEANFILES = bench.dat bench.rz

bench-local: all benc bdec
benc-local: all
	$(srcdir)/benc.sh $(top_srcdir)/data/typical.rz
bdec-local: all
	top_srcdir=$(top_srcdir) $(srcdir)/bdec.sh
