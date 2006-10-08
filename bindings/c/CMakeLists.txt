INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/toolkit
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/mpeg
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/ogg
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/ogg/vorbis
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/ogg/flac
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/flac
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/mpc
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/mpeg/id3v2  )


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

