INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/taglib ${CMAKE_SOURCE_DIR}/taglib/toolkit ${CMAKE_SOURCE_DIR}/taglib/mpeg ${CMAKE_SOURCE_DIR}/taglib/ogg ${CMAKE_SOURCE_DIR}/taglib/ogg/vorbis ${CMAKE_SOURCE_DIR}/taglib/ogg/flac ${CMAKE_SOURCE_DIR}/taglib/flac ${CMAKE_SOURCE_DIR}/taglib/mpc ${CMAKE_SOURCE_DIR}/taglib/mpeg/id3v2  )


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/taglib_c.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/taglib_c.pc @ONLY)
########### next target ###############

SET(tag_c_LIB_SRCS
tag_c.cpp
)


ADD_LIBRARY(tag_c SHARED ${tag_c_LIB_SRCS})

TARGET_LINK_LIBRARIES(tag_c  tag )

SET_TARGET_PROPERTIES(tag_c PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS tag_c DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

INSTALL( FILES  taglib_c.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
INSTALL( FILES  tag_c.h DESTINATION ${INCLUDE_INSTALL_DIR}/taglib)



#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/taglib \
#	-I$(top_srcdir)/taglib/toolkit \
#	-I$(top_srcdir)/taglib/mpeg \
#	-I$(top_srcdir)/taglib/ogg \
#	-I$(top_srcdir)/taglib/ogg/vorbis \
#	-I$(top_srcdir)/taglib/ogg/flac \
#	-I$(top_srcdir)/taglib/flac \
#	-I$(top_srcdir)/taglib/mpc \
#	-I$(top_srcdir)/taglib/mpeg/id3v2 \
#	$(all_includes)
#
#lib_LTLIBRARIES = libtag_c.la
#
#libtag_c_la_SOURCES = tag_c.cpp
#taglib_include_HEADERS = tag_c.h
#taglib_includedir = $(includedir)/taglib
#
#libtag_c_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 0:0
#libtag_c_la_LIBADD = ../../libtag.la
#
#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = taglib_c.pc
#
#EXTRA_DIST = $(libtag_c_la_SOURCES) $(taglib_include_HEADERS)
