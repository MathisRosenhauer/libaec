FUZZING_FLAGS = -g -fsanitize=address \
-fsanitize-coverage=trace-pc-guard,indirect-calls,edge
noinst_LIBRARIES = libaec_fuzz.a
libaec_fuzz_a_SOURCES = $(top_srcdir)/src/encode.c \
$(top_srcdir)/src/encode_accessors.c \
$(top_srcdir)/src/decode.c \
$(top_srcdir)/src/encode.h \
$(top_srcdir)/src/encode_accessors.h \
$(top_srcdir)/src/decode.h
libaec_fuzz_a_CFLAGS = $(FUZZING_FLAGS)
aec_fuzz_SOURCES = fuzz_target.cc
aec_fuzz_CXXFLAGS = $(FUZZING_FLAGS)
aec_fuzz_LDFLAGS = -fsanitize=address -lFuzzer
aec_fuzz_LDADD = libaec_fuzz.a
noinst_PROGRAMS = aec_fuzz
EXTRA_DIST = CMakeLists.txt
