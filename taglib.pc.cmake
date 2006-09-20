prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: TagLib
Description: Audio meta-data library
Requires: 
Version: 1.4
Libs: -L${libdir} -ltag
Cflags: -I${includedir}/taglib 
