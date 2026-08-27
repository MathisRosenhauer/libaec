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
# Components
# ----------
# The installed library flavors are reported as find_package
# components. This can be used to require a certain flavor to be
# present.
# Example:
#   find_package(libaec CONFIG REQUIRED COMPONENTS static)
#
# Recognized components are "shared" and "static". Requesting a
# component does not select it, use libaec_USE_STATIC_LIBS for that.
#
# This will define the following variables:
#
#   libaec_FOUND        - True if the system has the AEC library.
#   libaec_VERSION      - The version of the AEC library which was found.
#   libaec_shared_FOUND - True if the shared libraries were installed.
#   libaec_static_FOUND - True if the static libraries were installed.
#   SZIP_FOUND          - True if the system has the SZIP library.
#   SZIP_VERSION        - The version of the SZIP library which was found.
#   SZIP_LIBRARIES      - All the required libraries to make use of SZIP.
#   SZIP_INCLUDE_DIR    - SZIP include directory.
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

# A targets file is only installed for a flavor which has actually been
# built. Both includes are therefore optional.
include(${CMAKE_CURRENT_LIST_DIR}/libaec_shared-targets.cmake OPTIONAL)
include(${CMAKE_CURRENT_LIST_DIR}/libaec_static-targets.cmake OPTIONAL)

# Report the installed flavors as components. Callers can query these
# variables directly, they are also used for the component check below.
foreach(_flavor shared static)
  if(TARGET libaec::aec-${_flavor} AND TARGET libaec::sz-${_flavor})
    set(${CMAKE_FIND_PACKAGE_NAME}_${_flavor}_FOUND TRUE)
  else()
    set(${CMAKE_FIND_PACKAGE_NAME}_${_flavor}_FOUND FALSE)
  endif()
endforeach()
unset(_flavor)

# Honor components requested by the caller. Config files have to do
# this themselves, find_package() only provides the request.
foreach(_comp IN LISTS ${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_${_comp}_FOUND
      AND ${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED_${_comp})
    string(APPEND ${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
      "Required component ${_comp} not found. ")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
  endif()
endforeach()
unset(_comp)

# Alias static or shared targets depending on libaec_USE_STATIC_LIBS
if(libaec_USE_STATIC_LIBS)
  set(_libaec_flavor static)
else()
  set(_libaec_flavor shared)
endif()

if(${CMAKE_FIND_PACKAGE_NAME}_${_libaec_flavor}_FOUND)
  if(NOT TARGET libaec::aec AND NOT TARGET libaec::sz)
    add_library(libaec::aec ALIAS libaec::aec-${_libaec_flavor})
    add_library(libaec::sz ALIAS libaec::sz-${_libaec_flavor})
  endif()
else()
  string(TOUPPER ${_libaec_flavor} _libaec_flavor_uc)
  string(APPEND ${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
    "${_libaec_flavor_uc} versions of libaec::aec and libaec::sz not found.")
  set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
  unset(_libaec_flavor_uc)
endif()
unset(_libaec_flavor)

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
