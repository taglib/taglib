project(taglib)

find_package(ZLIB)

OPTION(BUILD_TESTS "Create tests and examples"  OFF)

#add some KDE specific stuff
set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(EXEC_INSTALL_PREFIX  ${CMAKE_INSTALL_PREFIX}       CACHE PATH  "Base directory for executables and libraries" FORCE)
#
## the following are directories where stuff will be installed to
set(BIN_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/bin"                  CACHE PATH "The kde info install dir (default prefix/info)" FORCE)
set(LIB_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}"     CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)
set(INCLUDE_INSTALL_DIR      "${CMAKE_INSTALL_PREFIX}/include"             CACHE PATH "The subdirectory to the header prefix" FORCE)

if (CMAKE_COMPILER_IS_GNUCXX)
   if (CMAKE_SYSTEM_NAME MATCHES Linux)
     set ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -ansi -Wundef -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
     set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -fno-exceptions -fno-check-new -fno-common")
   endif (CMAKE_SYSTEM_NAME MATCHES Linux)
endif (CMAKE_COMPILER_IS_GNUCXX)

include(ConfigureChecks.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/taglib-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/taglib-config )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/taglib.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/taglib.pc )
#For the future rename it as config-taglib.h
configure_file(config-taglib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

ADD_SUBDIRECTORY( taglib ) 

ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(examples)

ADD_SUBDIRECTORY(bindings)

INSTALL( FILES  ${CMAKE_CURRENT_BINARY_DIR}/taglib.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
INSTALL( PROGRAMS  ${CMAKE_CURRENT_BINARY_DIR}/taglib-config DESTINATION ${BIN_INSTALL_DIR})


