prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}


Name: TagLib C Bindings
Description: Audio meta-data library (C bindings)
Requires: taglib
Version: 1.4
Libs: -L${libdir} -ltag_c
Cflags: -I${includedir}/taglib 
