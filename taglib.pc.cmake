prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=@CMAKE_PC_LIBDIR@
includedir=@CMAKE_PC_INCLUDEDIR@

Name: TagLib
Description: Audio meta-data library
Requires:
Version: @TAGLIB_LIB_VERSION_STRING@
Libs: -L${libdir} -ltag@TAGLIB_INSTALL_SUFFIX@ @ZLIB_LIBRARIES_FLAGS@
Cflags: -I${includedir} -I${includedir}/taglib@TAGLIB_INSTALL_SUFFIX@
