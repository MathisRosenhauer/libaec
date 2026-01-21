# Shared and static versions of aec and sz. Also build graec tool for
# internal use.

if(NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
  message(FATAL_ERROR
    "At least one out of BUILD_SHARED_LIBS and BUILD_STATIC_LIBS must be selected")
endif()

# Sources for libaec
set(libaec_sources
  encode.c
  encode_accessors.c
  decode.c
  vector.c)

# Properties for all shared and static libraries
macro(aec_common_include aec_target aec_header)
  target_include_directories(${aec_target}
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>"
    "$<INSTALL_INTERFACE:include>")
  set_target_properties(${aec_target}
    PROPERTIES
    PUBLIC_HEADER "${aec_header}")
endmacro()

# Public header files
set(libaec_header "${CMAKE_CURRENT_BINARY_DIR}/../include/libaec.h")
set(szlib_header "${CMAKE_CURRENT_SOURCE_DIR}/../include/szlib.h")

# Optionally, create both static and shared aec library.
if(BUILD_SHARED_LIBS)
  # Shared libaec
  add_library(aec-shared-objects OBJECT ${libaec_sources})
  aec_common_include(aec-shared-objects "${libaec_header}")
  target_compile_definitions(aec-shared-objects
    PUBLIC "LIBAEC_SHARED"
    PRIVATE "LIBAEC_BUILD")
  add_library(aec-shared SHARED "$<TARGET_OBJECTS:aec-shared-objects>")
  target_link_libraries(aec-shared PUBLIC aec-shared-objects)
  set_target_properties(aec-shared
    PROPERTIES
    VERSION 0.1.4
    SOVERSION 0
    OUTPUT_NAME aec)
  if(MSVC OR ("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC"))
    set_target_properties(aec-shared PROPERTIES IMPORT_SUFFIX "_imp.lib")
  endif()

  # Shared libsz
  add_library(sz-shared-objects OBJECT sz_compat.c)
  aec_common_include(sz-shared-objects "${szlib_header}")
  target_compile_definitions(sz-shared-objects
    PUBLIC "LIBAEC_SHARED"
    PRIVATE "LIBAEC_BUILD")
  add_library(sz-shared SHARED
    "$<TARGET_OBJECTS:sz-shared-objects>")
  target_link_libraries(sz-shared PUBLIC aec-shared)
  set_target_properties(sz-shared
    PROPERTIES
    VERSION 2.0.1
    SOVERSION 2
    OUTPUT_NAME sz)
  if(MSVC OR ("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC"))
    set_target_properties(sz-shared PROPERTIES IMPORT_SUFFIX "_imp.lib")
  endif()
  # Add include directory directly to sz-shared so config can get it.
  target_include_directories(sz-shared
    PUBLIC "$<INSTALL_INTERFACE:include>")

  # Aliases for other tools. Prefer shared.
  add_library(libaec::aec ALIAS aec-shared)
  add_library(libaec::sz ALIAS sz-shared)
endif()

if(BUILD_STATIC_LIBS)
  # Static libaec
  add_library(aec-static-objects OBJECT ${libaec_sources})
  aec_common_include(aec-static-objects "${libaec_header}")
  add_library(aec-static STATIC "$<TARGET_OBJECTS:aec-static-objects>")
  target_link_libraries(aec-static PUBLIC aec-static-objects)
  set_target_properties(aec-static PROPERTIES OUTPUT_NAME aec)

  # Static libsz
  add_library(sz-static-objects OBJECT sz_compat.c)
  aec_common_include(sz-static-objects "${szlib_header}")
  add_library(sz-static STATIC
    "$<TARGET_OBJECTS:sz-static-objects>")
  target_link_libraries(sz-static PUBLIC aec-static)
  set_target_properties(sz-static PROPERTIES OUTPUT_NAME sz)
  # Add include directory directly to sz-static so config can get it.
  target_include_directories(sz-static
    PUBLIC "$<INSTALL_INTERFACE:include>")

  if(NOT BUILD_SHARED_LIBS)
    # Aliases if shared not built.
    add_library(libaec::aec ALIAS aec-static)
    add_library(libaec::sz ALIAS sz-static)
  endif()
endif()

# Simple executable for testing and benchmarking.
add_executable(graec graec.c)
target_link_libraries(graec libaec::aec)

if(UNIX)
  # The shell scripts for benchmarking are supported on unix only
  add_executable(utime EXCLUDE_FROM_ALL utime.c)
  target_include_directories(utime PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/..")
  add_custom_target(bench
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/benc.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/typical.rz
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bdec.sh
    DEPENDS graec utime)
endif()
