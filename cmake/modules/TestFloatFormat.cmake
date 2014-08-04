# Returns 1 if IEEE754 little-endian, 2 if IEEE754 big-endian, otherwise 0.

MACRO(TEST_FLOAT_FORMAT FP_IEEE754)
  IF(NOT FP_IEEE754)
    TRY_COMPILE(HAVE_FLOAT_FORMAT_BIN
      "${CMAKE_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/cmake/TestFloatFormat.c"
    COPY_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestFloatFormat.bin")

    SET(FP_IEEE754 0)

    IF(HAVE_FLOAT_FORMAT_BIN)

      # dont match first/last letter because of string rounding errors :-)
      FILE(STRINGS "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestFloatFormat.bin"
        DOUBLE_IEEE754_LE LIMIT_COUNT 1 REGEX "TAGLIB")
      FILE(STRINGS "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestFloatFormat.bin"
        DOUBLE_IEEE754_BE LIMIT_COUNT 1 REGEX "BILGAT")
      FILE(STRINGS "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestFloatFormat.bin"
        FLOAT_IEEE754_LE LIMIT_COUNT 1 REGEX "TL")
      FILE(STRINGS "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestFloatFormat.bin"
        FLOAT_IEEE754_BE LIMIT_COUNT 1 REGEX "LT")

      IF(DOUBLE_IEEE754_LE AND FLOAT_IEEE754_LE)
        SET(FP_IEEE754_LE 1)
      ENDIF()

      IF(DOUBLE_IEEE754_BE AND FLOAT_IEEE754_BE)
        SET(FP_IEEE754_BE 1)
      ENDIF()

      # OS X Universal binaries will contain both strings, set it to the host
      IF(FP_IEEE754_LE AND FP_IEEE754_BE)
        IF(CMAKE_SYSTEM_PROCESSOR MATCHES powerpc)
            SET(FP_IEEE754_LE FALSE)
            SET(FP_IEEE754_BE TRUE)
        ELSE()
            SET(FP_IEEE754_LE TRUE)
            SET(FP_IEEE754_BE FALSE)
        ENDIF()
      ENDIF()

      IF(FP_IEEE754_LE)
        SET(FP_IEEE754 1)
      ELSEIF(FP_IEEE754_BE)
        SET(FP_IEEE754 2)
      ENDIF()
    ENDIF()

    # just some informational output for the user
    IF(FP_IEEE754_LE)
       MESSAGE(STATUS "Checking the floating point format - IEEE754 (LittleEndian)")
    ELSEIF(FP_IEEE754_BE)
       MESSAGE(STATUS "Checking the floating point format - IEEE754 (BigEndian)")
    ELSE()
       MESSAGE(STATUS "Checking the floating point format - Not IEEE754 or failed to detect.")
    ENDIF()

    SET(FP_IEEE754 "${${FP_IEEE754}}" CACHE INTERNAL "Result of TEST_FLOAT_FORMAT" FORCE)
  ENDIF()
ENDMACRO(TEST_FLOAT_FORMAT FP_IEEE754)

