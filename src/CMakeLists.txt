# Main library aec
add_library(aec
  encode.c
  encode_accessors.c
  decode.c)
add_library(libaec::aec ALIAS aec)

target_include_directories(aec
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>"
  "$<INSTALL_INTERFACE:include>")

set_target_properties(aec
  PROPERTIES
  VERSION 0.0.10
  SOVERSION 0
  PUBLIC_HEADER ../include/libaec.h)

# Wrapper for compatibility with szip
add_library(sz sz_compat.c)
add_library(libaec::sz ALIAS sz)

set_target_properties(sz
  PROPERTIES
  VERSION 2.0.1
  SOVERSION 2
  OUTPUT_NAME $<IF:$<BOOL:${WIN32}>,szip,sz>
  PUBLIC_HEADER ../include/szlib.h)
target_link_libraries(sz PUBLIC aec)

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

  # Install manpage
  install(
    FILES aec.1
    DESTINATION ${CMAKE_INSTALL_FULL_MANDIR}/man1
    COMPONENT doc)
endif()

install(TARGETS aec sz aec_client
  EXPORT libaecTargets)

install(EXPORT libaecTargets
  FILE libaecTargets.cmake
  NAMESPACE libaec::
  DESTINATION lib/cmake)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/../Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/libaecConfig.cmake"
  INSTALL_DESTINATION lib/cmake)

# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/libaecConfigVersion.cmake"
  COMPATIBILITY AnyNewerVersion)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/libaecConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/libaecConfigVersion.cmake"
  DESTINATION lib/cmake)
