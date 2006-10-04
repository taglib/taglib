if(BUILD_TESTS)
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/taglib ${CMAKE_SOURCE_DIR}/taglib/toolkit ${CMAKE_SOURCE_DIR}/taglib/mpeg/id3v2  )


########### next target ###############

SET(toolkit-test_SRCS
	toolkit-test.cpp
)

ADD_EXECUTABLE(toolkit-test ${toolkit-test_SRCS})

TARGET_LINK_LIBRARIES(toolkit-test  tag )
endif(BUILD_TESTS)
