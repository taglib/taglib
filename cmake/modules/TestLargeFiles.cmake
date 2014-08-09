# - Define macro to check large file support
#
#  TEST_LARGE_FILES(VARIABLE)
#
#  VARIABLE will be set to true if off_t is 64 bits, and fseeko/ftello present.
#  This macro will also set defines necessary enable large file support, for instance
#  _LARGE_FILES
#  _LARGEFILE_SOURCE
#  _FILE_OFFSET_BITS 64
#  HAVE_FSEEKO
#
#  However, it is YOUR job to make sure these defines are set in a cmakedefine so they
#  end up in a config.h file that is included in your source if necessary!

#  This macro skips the Windows specific checks. Because TagLib uses Win32 API.

MACRO(TEST_LARGE_FILES VARIABLE)
  IF(NOT DEFINED ${VARIABLE})

    # On most platforms it is probably overkill to first test the flags for 64-bit off_t,
    # and then separately fseeko. However, in the future we might have 128-bit filesystems
    # (ZFS), so it might be dangerous to indiscriminately set e.g. _FILE_OFFSET_BITS=64.

    MESSAGE(STATUS "Checking for 64-bit off_t")

    # First check without any special flags
    TRY_COMPILE(FILE64_OK "${PROJECT_BINARY_DIR}"
      "${CMAKE_SOURCE_DIR}/cmake/TestFileOffsetBits.c")
    if(FILE64_OK)
      MESSAGE(STATUS "Checking for 64-bit off_t - present")
    endif(FILE64_OK)

    if(NOT FILE64_OK)
      # Test with _FILE_OFFSET_BITS=64
      TRY_COMPILE(FILE64_OK "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/cmake/TestFileOffsetBits.c"
        COMPILE_DEFINITIONS "-D_FILE_OFFSET_BITS=64" )
      if(FILE64_OK)
        MESSAGE(STATUS "Checking for 64-bit off_t - present with _FILE_OFFSET_BITS=64")
        set(_FILE_OFFSET_BITS 64)
      endif(FILE64_OK)
    endif(NOT FILE64_OK)

    if(NOT FILE64_OK)
      # Test with _LARGE_FILES
      TRY_COMPILE(FILE64_OK "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/cmake/TestFileOffsetBits.c"
        COMPILE_DEFINITIONS "-D_LARGE_FILES" )
      if(FILE64_OK)
        MESSAGE(STATUS "Checking for 64-bit off_t - present with _LARGE_FILES")
        set(_LARGE_FILES 1)
      endif(FILE64_OK)
    endif(NOT FILE64_OK)

    if(NOT FILE64_OK)
      # Test with _LARGEFILE_SOURCE
      TRY_COMPILE(FILE64_OK "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/cmake/TestFileOffsetBits.c"
        COMPILE_DEFINITIONS "-D_LARGEFILE_SOURCE" )
      if(FILE64_OK)
        MESSAGE(STATUS "Checking for 64-bit off_t - present with _LARGEFILE_SOURCE")
        set(_LARGEFILE_SOURCE 1)
      endif(FILE64_OK)
    endif(NOT FILE64_OK)

    if(NOT FILE64_OK)
      MESSAGE(STATUS "Checking for 64-bit off_t - not present")
    else(NOT FILE64_OK)

      # Set the flags we might have determined to be required above
      configure_file("${CMAKE_SOURCE_DIR}/cmake/TestLargeFiles.c.cmakein"
                     "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestLargeFiles.c")

      MESSAGE(STATUS "Checking for fseeko/ftello")
      # Test if ftello/fseeko are available
      TRY_COMPILE(FSEEKO_COMPILE_OK "${PROJECT_BINARY_DIR}"
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestLargeFiles.c")
      if(FSEEKO_COMPILE_OK)
        MESSAGE(STATUS "Checking for fseeko/ftello - present")
      endif(FSEEKO_COMPILE_OK)

      if(NOT FSEEKO_COMPILE_OK)
        # glibc 2.2 neds _LARGEFILE_SOURCE for fseeko (but not 64-bit off_t...)
        TRY_COMPILE(FSEEKO_COMPILE_OK "${PROJECT_BINARY_DIR}"
          "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestLargeFiles.c"
          COMPILE_DEFINITIONS "-D_LARGEFILE_SOURCE" )
        if(FSEEKO_COMPILE_OK)
          MESSAGE(STATUS "Checking for fseeko/ftello - present with _LARGEFILE_SOURCE")
          set(_LARGEFILE_SOURCE 1)
        endif(FSEEKO_COMPILE_OK)
      endif(NOT FSEEKO_COMPILE_OK)

    endif(NOT FILE64_OK)

    if(FSEEKO_COMPILE_OK)
      SET(${VARIABLE} 1 CACHE INTERNAL "Result of test for large file support" FORCE)
      set(HAVE_FSEEKO 1)
    else(FSEEKO_COMPILE_OK)
      MESSAGE(STATUS "Checking for fseeko/ftello - not found")
      SET(${VARIABLE} 0 CACHE INTERNAL "Result of test for large file support" FORCE)
    endif(FSEEKO_COMPILE_OK)

  ENDIF(NOT DEFINED ${VARIABLE})
ENDMACRO(TEST_LARGE_FILES VARIABLE)
