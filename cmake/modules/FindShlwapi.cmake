#  *
#  * It is what it is, you can do with it as you please.
#  *
#  * Just don't blame me if it teaches your computer to smoke!
#  *
#  *  -Enjoy
#  *  fh :)_~
#  *
FIND_PATH(SHLWAPI_INCLUDE_DIR shlwapi.h)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SHLWAPI REQUIRED_VARS SHLWAPI_LIBRARY SHLWAPI_INCLUDE_DIR)
IF(SHLWAPI_FOUND)
  SET(SHLWAPI_LIBRARIES ${SHLWAPI_LIBRARY} )
ENDIF(SHLWAPI_FOUND)

