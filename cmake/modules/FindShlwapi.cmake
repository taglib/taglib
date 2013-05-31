#  *
#  * It is what it is, you can do with it as you please. [with respect]
#  *
#  * Just don't blame me if it teaches your computer to smoke!
#  *
#  *  -Enjoy
#  *  fh :)_~
#  *
#
# Methods to provide locations for the shlwapi files.
# 1. Command line options.
# 2. Enviroment variables. (Used just like the System PATH)
#
# Command line options (i.e. cmake -G "Generator" -DSHLWAPI_PREFIX_PATH=C:\Libraries\Shlwapi)
#   SHLWAPI_LIBRARY       = Fully qualified library filename (i.e. -DSHLWAPI_LIBRARY=C:\Libraries\Shlwapi\Lib\libshlwapi.a)
#   SHLWAPI_INCLUDE_DIR   = Path to shlwapi.h (i.e. -DSHLWAPI_INCLUDE_DIR=C:\Libraries\Shlwapi\Include)
#   SHLWAPI_PREFIX_PATH   = Path to Shlwapi root. (i.e. -DSHLWAPI_PREFIX_PATH=C:\Libraries\Shlwapi)
#                           Assumes it's the parent directory of \Include and \Lib that'll be appended for search paths.
#
# Shlwapi files are searched for in the following locations in the following order.
#     ${SHLWAPI_PREFIX_PATH} = CMake variable. Command line option, Path to Shlwapi root directory.
#                              It's used twice, once with \lib appended, once with \include appended.
#     ${SHLWAPI_LIBRARY}     = CMake variable. Command line option.
#     ${SHLWAPI_INCLUDE_DIR} = CMake variable. Command line option.
#     ${CMAKE_INCLUDE_PATH}  = CMake variable.
#     ${CMAKE_LIBRARY_PATH}  = CMake variable.
#     INCLUDE                = Environment variable. Include file locations. 
#     LIBPATH                = Environment variable. Library file locations.
#     LIB                    = Environment variable. Library file locations.
#     LIB32                  = Environment variable. Library file locations.
#     LIB64                  = Environment variable. Library file locations.
#     LIBRARY_PATH           = Environment variable. Library file locations.
#     PATH                   = System PATH Environment variable.
#
# If found, the results are in ...
#   ${SHLWAPI_FOUND}         = Set true/yes
#   ${SHLWAPI_INCLUDE_DIRS}  = The fully qualified path to shlwapi.h
#   ${SHLWAPI_LIBRARIES}     = Libraries, fully qualified filenames. (can be a list seperated like the System PATH)
#

# Include names
SET(SHLWAPI_INCLUDE "shlwapi.h")
# Library  names
SET(SHLWAPI_NAMES libshlwapi.a shlwapi.lib libshlwapi shlwapi)
# Where to look for the above ...
SET(SHLWAPI_PATHS "${SHLWAPI_PREFIX_PATH}/lib"
                  "${SHLWAPI_PREFIX_PATH}/include"
                   ${SHLWAPI_LIBRARY}
                   ${SHLWAPI_INCLUDE_DIR}
                   ${CMAKE_INCLUDE_PATH}
                   ${CMAKE_LIBRARY_PATH}
                   $ENV{INCLUDE}
                   $ENV{LIBPATH}
                   $ENV{LIB}
                   $ENV{LIB32}
                   $ENV{LIB64}
                   $ENV{LIBRARY_PATH}
                   $ENV{PATH}
                   )

FIND_PATH(SHLWAPI_INCLUDE_DIR ${SHLWAPI_INCLUDE} PATHS ${SHLWAPI_PATHS})
FIND_LIBRARY(SHLWAPI_LIBRARY NAMES ${SHLWAPI_NAMES} PATHS ${SHLWAPI_PATHS})

MARK_AS_ADVANCED(SHLWAPI_LIBRARY SHLWAPI_INCLUDE_DIR)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SHLWAPI REQUIRED_VARS SHLWAPI_LIBRARY SHLWAPI_INCLUDE_DIR)

if(SHLWAPI_FOUND)
  SET(SHLWAPI_LIBRARIES ${SHLWAPI_LIBRARY})
  SET(SHLWAPI_INCLUDE_DIRS ${SHLWAPI_INCLUDE_DIR})
endif()

