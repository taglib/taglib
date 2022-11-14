prefix=@prefix@
exec_prefix=@exec_prefix@
libdir=@libdir@
includedir=@includedir@


Name: TagLib C Bindings
Description: Audio meta-data library (C bindings)
Requires: taglib
Version: ${TAGLIB_LIB_VERSION_STRING}
Libs: -L${CMAKE_INSTALL_FULL_LIBDIR} -ltag_c
Cflags: -I${CMAKE_INSTALL_FULL_INCLUDEDIR}/taglib
