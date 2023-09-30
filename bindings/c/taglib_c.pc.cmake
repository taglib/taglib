prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=@CMAKE_PC_LIBDIR@
includedir=@CMAKE_PC_INCLUDEDIR@

Name: TagLib C Bindings
Description: Audio meta-data library (C bindings)
Requires: taglib
Version: @TAGLIB_LIB_VERSION_STRING@
Libs: -L${libdir} -ltag_c
Cflags: -I${includedir}/taglib
