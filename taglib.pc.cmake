prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: TagLib
Description: Audio meta-data library
Requires:
Version: @TAGLIB_LIB_VERSION_STRING@
Libs: -L${libdir} -ltag @ZLIB_LIBRARIES_FLAGS@
Cflags: -I${includedir} -I${includedir}/taglib
