prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}
libdir=${CMAKE_INSTALL_FULL_LIBDIR}
includedir=${CMAKE_INSTALL_FULL_INCLUDEDIR}


Name: TagLib C Bindings
Description: Audio meta-data library (C bindings)
Requires: taglib
Version: ${TAGLIB_LIB_VERSION_STRING}
Libs: -L${CMAKE_INSTALL_FULL_LIBDIR} -ltag_c
Cflags: -I${CMAKE_INSTALL_FULL_INCLUDEDIR}/taglib
