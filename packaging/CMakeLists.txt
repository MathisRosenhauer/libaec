include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED libaec_INSTALL_CMAKEDIR)
  set(libaec_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/libaec"
    CACHE STRING "Path to libaec CMake files")
endif()

write_basic_package_version_file(
  libaec-config-version.cmake
  COMPATIBILITY SameMajorVersion)

install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/libaec-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/libaec-config-version.cmake"
  DESTINATION "${libaec_INSTALL_CMAKEDIR}")

set(TARGETS_EXPORT)
if(BUILD_SHARED_LIBS)
  set(TARGETS_EXPORT aec-shared-objects sz-shared-objects aec-shared sz-shared)

  install(TARGETS ${TARGETS_EXPORT}
    EXPORT libaec_shared_targets
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

  install(EXPORT libaec_shared_targets
    DESTINATION "${libaec_INSTALL_CMAKEDIR}"
    NAMESPACE libaec::
    FILE libaec_shared-targets.cmake)
endif()
if(BUILD_STATIC_LIBS)
  set(TARGETS_EXPORT aec-static-objects sz-static-objects aec-static sz-static)

  install(TARGETS ${TARGETS_EXPORT}
    EXPORT libaec_static_targets
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

  install(EXPORT libaec_static_targets
    DESTINATION "${libaec_INSTALL_CMAKEDIR}"
    NAMESPACE libaec::
    FILE libaec_static-targets.cmake)
endif()

# Cpack configuration mainly for Windows installer
set(CPACK_PACKAGE_NAME "libaec")
set(CPACK_PACKAGE_VENDOR "Deutsches Klimarechenzentrum GmbH")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "libaec - Adaptive Entropy Coding library")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "libaec")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.txt")

if(WIN32)
  set(CPACK_GENERATOR "WIX")
  set(CPACK_WIX_UPGRADE_GUID "E58A21F6-BB99-46B5-8AD8-6114E4CCA0A6")
endif()

set(CPACK_SOURCE_IGNORE_FILES /.git;.*~)

include(CPack)
