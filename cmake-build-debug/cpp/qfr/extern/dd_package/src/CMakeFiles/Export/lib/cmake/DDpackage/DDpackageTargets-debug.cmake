#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "JKQ::DDpackage" for configuration "Debug"
set_property(TARGET JKQ::DDpackage APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(JKQ::DDpackage PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libdd_package.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS JKQ::DDpackage )
list(APPEND _IMPORT_CHECK_FILES_FOR_JKQ::DDpackage "${_IMPORT_PREFIX}/lib/libdd_package.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
