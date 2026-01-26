cmake_minimum_required(VERSION 3.26...3.31)
project(libaec LANGUAGES C VERSION 1.1.5)

option(BUILD_SHARED_LIBS "OFF: do not build shared libraries.  ON (default): build shared libraries" ON)
option(BUILD_STATIC_LIBS "OFF: do not build static libraries.  ON (default): build static libraries" ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(CTest)
include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

# Check for __builtin_clzll for faster decoding
include(CheckCSourceCompiles)
check_c_source_compiles(
  "int main(void)\n{return __builtin_clzll(1LL);}"
  HAVE_DECL___BUILTIN_CLZLL)

if(NOT HAVE_DECL___BUILTIN_CLZLL)
  # With MSVC we can use _BitScanReverse64
  check_c_source_compiles(
    "int main(void){unsigned long foo; unsigned __int64 bar=1LL;
return _BitScanReverse64(&foo, bar);}"
    HAVE_BSR64)
endif()

include(CheckSymbolExists)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
if(NOT HAVE_SNPRINTF)
  check_symbol_exists(_snprintf "stdio.h" HAVE__SNPRINTF)
  check_symbol_exists(_snprintf_s "stdio.h" HAVE__SNPRINTF_S)
endif()

# Communicate findings to code. Has to be compatible with autoconf's config.h.
configure_file(
  "include/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/config.h")

# Create libaec.h with current version information
configure_file(
  "include/libaec.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/libaec.h")

add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

option(AEC_FUZZING "Enable build of fuzzing target" OFF)
if(AEC_FUZZING)
  enable_language(CXX)

  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "You need to build with Clang for fuzzing to work")
  endif()

  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0.0")
    message(FATAL_ERROR "You need Clang ≥ 6.0.0")
  endif()

  add_subdirectory(fuzzing)
  set(FUZZ_TARGET_SAN_FLAGS -fsanitize=fuzzer-no-link,address,undefined)
  target_compile_options(aec-shared-objects PUBLIC -g -O1 ${FUZZ_TARGET_SAN_FLAGS})
  target_link_options(aec-shared PUBLIC ${FUZZ_TARGET_SAN_FLAGS})
endif()

# Include the install rules if the user wanted them (included by
# default when top-level)

string(COMPARE
  EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)
option(libaec_INCLUDE_PACKAGING
  "Include packaging rules for libaec" "${is_top_level}")
if(libaec_INCLUDE_PACKAGING)
  add_subdirectory(packaging)
endif()
