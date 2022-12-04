prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${prefix}/@CMAKE_INSTALL_LIBDIR@
includedir=${prefix}/@CMAKE_INSTALL_INCLUDEDIR@

Name: TagLib
Description: Audio meta-data library
Requires:
Version: @TAGLIB_LIB_VERSION_STRING@
Libs: -L${libdir} -ltag @ZLIB_LIBRARIES_FLAGS@
Cflags: -I${includedir} -I${includedir}/taglib
