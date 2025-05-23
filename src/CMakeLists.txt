# Shared and static versions of aec and sz. Also build graec tool for
# internal use.

if (NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
  message(FATAL_ERROR
    "At least one out of BUILD_SHARED_LIBS and BUILD_STATIC_LIBS must be selected")
endif()

# We are building libaec so symbols should be exported.
set(libaec_COMPILE_DEFINITIONS "LIBAEC_BUILD")

# Handle visibility of symbols. Compatible with gnulib's gl_VISIBILITY
include(CheckCCompilerFlag)
check_c_compiler_flag(-fvisibility=hidden COMPILER_HAS_HIDDEN_VISIBILITY)
if (COMPILER_HAS_HIDDEN_VISIBILITY)
  set(libaec_COMPILE_DEFINITIONS
    "${libaec_COMPILE_DEFINITIONS};HAVE_VISIBILITY")
endif()

# Sources for libaec
set(libaec_sources
  encode.c
  encode_accessors.c
  decode.c
  vector.c)

# Properties for both shared and static libaec
macro(common_aec aec_target)
  target_include_directories(${aec_target}
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>"
    "$<INSTALL_INTERFACE:include>")
  set_target_properties(${aec_target}
    PROPERTIES
    PUBLIC_HEADER ${CMAKE_CURRENT_BINARY_DIR}/../include/libaec.h)
  target_compile_definitions(${aec_target}
    PRIVATE "${libaec_COMPILE_DEFINITIONS}")
endmacro()

# Properties for both shared and static libsz
macro(common_sz sz_target)
  set_target_properties(${sz_target}
    PROPERTIES
    PUBLIC_HEADER ../include/szlib.h)
  target_compile_definitions(${sz_target}
    PRIVATE "${libaec_COMPILE_DEFINITIONS}")
endmacro()

set(TARGETS_EXPORT)

# Optionally, create both static and shared aec library.
if (BUILD_SHARED_LIBS)
  # Shared libaec
  add_library(aec_shared SHARED ${libaec_sources})
  set_target_properties(aec_shared
    PROPERTIES
    VERSION 0.1.3
    SOVERSION 0
    OUTPUT_NAME aec)
  common_aec(aec_shared)
  target_compile_definitions(aec_shared PUBLIC "LIBAEC_SHARED")

  # Shared libsz
  add_library(sz_shared SHARED sz_compat.c)
  set_target_properties(sz_shared
    PROPERTIES
    VERSION 2.0.1
    SOVERSION 2
    OUTPUT_NAME $<IF:$<BOOL:${MSVC}>,szip,sz>)
  target_link_libraries(sz_shared PRIVATE aec_shared)
  common_sz(sz_shared)
  target_compile_definitions(sz_shared PUBLIC "LIBAEC_SHARED")

  # Aliases for other tools. Prefer shared.
  add_library(libaec::aec ALIAS aec_shared)
  add_library(libaec::sz ALIAS sz_shared)

  set(TARGETS_EXPORT aec_shared sz_shared)
endif()

if (BUILD_STATIC_LIBS)
  # Static libaec
  add_library(aec_static STATIC ${libaec_sources})
  set_target_properties(aec_static
    PROPERTIES
    OUTPUT_NAME $<IF:$<BOOL:${MSVC}>,aec-static,aec>)
  common_aec(aec_static)

  # Static libsz
  add_library(sz_static STATIC sz_compat.c)
  set_target_properties(sz_static
    PROPERTIES
    OUTPUT_NAME $<IF:$<BOOL:${MSVC}>,szip-static,sz>)
  target_link_libraries(sz_static PRIVATE aec_static)
  common_sz(sz_static)

  if (NOT BUILD_SHARED_LIBS)
    # Aliases for other tools if shared not built.
    add_library(libaec::aec ALIAS aec_static)
    add_library(libaec::sz ALIAS sz_static)
  endif()

  set(TARGETS_EXPORT ${TARGETS_EXPORT} aec_static sz_static)
endif()

install(TARGETS ${TARGETS_EXPORT}
  EXPORT ${PROJECT_NAME}-targets
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT ${PROJECT_NAME}-targets
  FILE ${PROJECT_NAME}-targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION cmake)

# Simple executable for testing and benchmarking.
add_executable(graec graec.c)
target_link_libraries(graec libaec::aec)

include(GNUInstallDirs)
if (UNIX)
  # The shell scripts for benchmarking are supported on unix only
  add_executable(utime EXCLUDE_FROM_ALL utime.c)
  target_include_directories(utime PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/..")
  add_custom_target(bench
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/benc.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/typical.rz
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bdec.sh
    DEPENDS graec utime)
endif()
