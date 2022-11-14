prefix=@prefix@
exec_prefix=@exec_prefix@
libdir=@libdir@
includedir=@includedir@

Name: TagLib
Description: Audio meta-data library
Requires:
Version: @TAGLIB_LIB_VERSION_STRING@
Libs: -L${libdir} -ltag @ZLIB_LIBRARIES_FLAGS@
Cflags: -I${includedir} -I${includedir}/taglib
