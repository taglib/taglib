if(BUILD_TESTS)
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/taglib ${CMAKE_SOURCE_DIR}/taglib/toolkit ${CMAKE_SOURCE_DIR}/taglib/mpeg ${CMAKE_SOURCE_DIR}/taglib/mpeg/id3v1 ${CMAKE_SOURCE_DIR}/taglib/mpeg/id3v2 ${CMAKE_SOURCE_DIR}/bindings/c/  )

########### next target ###############

SET(tagreader_SRCS
tagreader.cpp
)


ADD_EXECUTABLE(tagreader ${tagreader_SRCS})

TARGET_LINK_LIBRARIES(tagreader  tag )

INSTALL(PROGRAMS tagreader DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

SET(tagreader_c_SRCS
tagreader_c.c
)


ADD_EXECUTABLE(tagreader_c ${tagreader_c_SRCS})

TARGET_LINK_LIBRARIES(tagreader_c  tag_c )

INSTALL(PROGRAMS tagreader_c DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

SET(tagwriter_SRCS
tagwriter.cpp
)


ADD_EXECUTABLE(tagwriter ${tagwriter_SRCS})

TARGET_LINK_LIBRARIES(tagwriter  tag )

INSTALL(PROGRAMS tagwriter DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

SET(framelist_SRCS
framelist.cpp
)


ADD_EXECUTABLE(framelist ${framelist_SRCS})

TARGET_LINK_LIBRARIES(framelist  tag )

INSTALL(PROGRAMS framelist DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

SET(strip-id3v1_SRCS
	strip-id3v1.cpp)

ADD_EXECUTABLE(strip-id3v1 ${strip-id3v1_SRCS})

TARGET_LINK_LIBRARIES(strip-id3v1  tag )

INSTALL(PROGRAMS strip-id3v1 DESTINATION ${BIN_INSTALL_DIR})

endif(BUILD_TESTS)

