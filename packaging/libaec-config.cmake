# libaec-config.cmake
# -------------------
#
# Finds the AEC library.
#
# Static vs. shared
# -----------------
# To make use of the static library instead of the shared one, one
# needs to set the variable libaec_USE_STATIC_LIBS to ON before
# calling find_package.
# Example:
#   set(libaec_USE_STATIC_LIBS ON)
#   find_package(libaec CONFIG)
#
# This will define the following variables:
#
#   libaec_FOUND     - True if the system has the AEC library.
#   libaec_VERSION   - The version of the AEC library which was found.
#   SZIP_FOUND       - True if the system has the SZIP library.
#   SZIP_VERSION     - The version of the SZIP library which was found.
#   SZIP_LIBRARIES   - All the required libraries to make use of SZIP.
#   SZIP_INCLUDE_DIR - SZIP include directory.
#
# and the following imported targets:
#
#   libaec::aec-shared  - The shared AEC library target (if it was built).
#   libaec::sz-shared   - The shared SZIP compatible version of the AEC library
#                         (if it was built).
#   libaec::aec-static  - The static AEC library target (if it was built).
#   libaec::sz-static   - The static SZIP compatible version of the AEC library
#                         (if it was built).
#   libaec::aec         - The (shared or static) AEC library target (according
#                         to the value of libaec_USE_STATIC_LIBS).
#   libaec::sz          - The (shared or static) SZIP compatible version of the
#                         AEC library (according to the value of
#                         libaec_USE_STATIC_LIBS).

include(${CMAKE_CURRENT_LIST_DIR}/libaec-targets.cmake)

# Alias static or shared targets depending on libaec_USE_STATIC_LIBS
if(libaec_USE_STATIC_LIBS)
  if(TARGET libaec::aec-static AND TARGET libaec::sz-static)
    add_library(libaec::aec ALIAS libaec::aec-static)
    add_library(libaec::sz ALIAS libaec::sz-static)
  else()
    set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
      "STATIC versions of libaec::aec and libaec::sz not found.")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
  endif()
else()
  if(TARGET libaec::aec-shared AND TARGET libaec::sz-shared)
    add_library(libaec::aec ALIAS libaec::aec-shared)
    add_library(libaec::sz ALIAS libaec::sz-shared)
  else()
    set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
      "SHARED versions of libaec::aec and libaec::sz not found.")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
  endif()
endif()

if(TARGET libaec::sz)
  get_target_property(SZIP_INCLUDE_DIR
    libaec::sz INTERFACE_INCLUDE_DIRECTORIES)
  if(SZIP_INCLUDE_DIR)
    # This might be a list. The first item is the directory with the
    # "main" header.
    list(GET SZIP_INCLUDE_DIR 0 SZIP_INCLUDE_DIR)
  endif()

  # Loop over configurations for libaec::sz and set SZIP_LIBRARIES to
  # the first configuration with an existing file for
  # IMPORTED_IMPLIB_<CONFIG> or, if that does not exist, a file for
  # IMPORTED_LOCATION_<CONFIG>.
  get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(_isMultiConfig)
    # For multi-configuration generators (e.g., Visual Studio), prefer
    # those configurations.
    string(TOUPPER "${CMAKE_CONFIGURATION_TYPES}" _build_types)
  else()
    # For single-configuration generators, prefer the current
    # configuration.
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _build_types)
  endif()
  get_target_property(_imported_configs libaec::sz IMPORTED_CONFIGURATIONS)
  list(APPEND _build_types ${_imported_configs})
  list(REMOVE_DUPLICATES _build_types)

  foreach(_config ${_build_types})
    get_target_property(_library_chk
      libaec::sz "IMPORTED_IMPLIB_${_config}")
    if(EXISTS "${_library_chk}")
      set(SZIP_LIBRARIES "${_library_chk}")
      break()
    else()
      get_target_property(_lib_check
        libaec::sz "IMPORTED_LOCATION_${_config}")
      if(EXISTS "${_lib_check}")
        set(SZIP_LIBRARIES "${_lib_check}")
        break()
      endif()
    endif()
  endforeach()

  set(SZIP_VERSION "2.0.1")
  set(SZIP_FOUND "TRUE")
endif()
