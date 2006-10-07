INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/taglib/taglib
		     ${CMAKE_SOURCE_DIR}/taglib/taglib/toolkit
		     ${CMAKE_SOURCE_DIR}/taglib/taglib/mpeg
		     ${CMAKE_SOURCE_DIR}/taglib/taglib/ogg
		     ${CMAKE_SOURCE_DIR}/taglib/taglib/ogg/vorbis
		     ${CMAKE_SOURCE_DIR}/taglib/taglib/ogg/flac
		     ${CMAKE_SOURCE_DIR}/taglib/taglib/flac
		     ${CMAKE_SOURCE_DIR}/taglib/taglib/mpc
		     ${CMAKE_SOURCE_DIR}/taglib/taglib/mpeg/id3v2  )


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/taglib_c.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/taglib_c.pc )
########### next target ###############

SET(tag_c_LIB_SRCS
tag_c.cpp
)


ADD_LIBRARY(tag_c SHARED ${tag_c_LIB_SRCS})

TARGET_LINK_LIBRARIES(tag_c  tag )

SET_TARGET_PROPERTIES(tag_c PROPERTIES VERSION 0.0.0 SOVERSION 0 )
INSTALL(TARGETS tag_c DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

INSTALL( FILES  ${CMAKE_CURRENT_BINARY_DIR}/taglib_c.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
INSTALL( FILES  tag_c.h DESTINATION ${INCLUDE_INSTALL_DIR}/taglib)

