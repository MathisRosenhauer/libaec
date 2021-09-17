# Main library aec
add_library(aec OBJECT
  encode.c
  encode_accessors.c
  decode.c)

target_include_directories(aec
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>"
  "$<INSTALL_INTERFACE:include>")

# Create both static and shared aec library.
add_library(aec_static STATIC "$<TARGET_OBJECTS:aec>")
target_link_libraries(aec_static PUBLIC aec)
set_target_properties(aec_static
  PROPERTIES
  OUTPUT_NAME $<IF:$<BOOL:${MSVC}>,aec-static,aec>)

add_library(aec_shared SHARED "$<TARGET_OBJECTS:aec>")
target_link_libraries(aec_shared PUBLIC aec)
set_target_properties(aec_shared
  PROPERTIES
  VERSION 0.0.12
  SOVERSION 0
  OUTPUT_NAME aec
  PUBLIC_HEADER ../include/libaec.h)

# Wrapper for compatibility with szip
add_library(sz OBJECT sz_compat.c)
target_link_libraries(sz PUBLIC aec)

# Create both static and shared szip library.
add_library(sz_static STATIC "$<TARGET_OBJECTS:sz>" "$<TARGET_OBJECTS:aec>")
set_target_properties(sz_static
  PROPERTIES
  OUTPUT_NAME $<IF:$<BOOL:${MSVC}>,szip-static,sz>)
target_link_libraries(sz_static PUBLIC sz)

add_library(sz_shared SHARED "$<TARGET_OBJECTS:sz>" "$<TARGET_OBJECTS:aec>")
target_link_libraries(sz_shared PUBLIC sz)
set_target_properties(sz_shared
  PROPERTIES
  VERSION 2.0.1
  SOVERSION 2
  OUTPUT_NAME $<IF:$<BOOL:${MSVC}>,szip,sz>
  PUBLIC_HEADER ../include/szlib.h)

# Simple client for testing and benchmarking.
# Can also be used stand-alone
add_executable(aec_client aec.c)
set_target_properties(aec_client PROPERTIES OUTPUT_NAME aec)
target_link_libraries(aec_client PUBLIC aec)

include(GNUInstallDirs)
if(UNIX)
  # Handle visibility of symbols. Compatible with gnulib's gl_VISIBILITY
  include(CheckCCompilerFlag)
  check_c_compiler_flag(-fvisibility=hidden COMPILER_HAS_HIDDEN_VISIBILITY)
  set_target_properties(aec sz
    PROPERTIES
    COMPILE_DEFINITIONS
    "HAVE_VISIBILITY=$<BOOL:${COMPILER_HAS_HIDDEN_VISIBILITY}>;BUILDING_LIBAEC")

  # The shell scripts for benchmarking are supported on unix only
  add_executable(utime EXCLUDE_FROM_ALL utime.c)
  target_include_directories(utime PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/..")
  add_custom_target(bench
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/benc.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/typical.rz
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bdec.sh
    DEPENDS aec_client utime)
endif()

if(UNIX OR MINGW)
  # Install manpage
  install(
    FILES aec.1
    DESTINATION ${CMAKE_INSTALL_FULL_MANDIR}/man1
    COMPONENT doc)
endif()

install(TARGETS aec_static aec_shared sz_static sz_shared aec_client)
