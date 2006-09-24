project(taglib)

find_package(ZLIB)

#add some KDE specific stuff
set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(EXEC_INSTALL_PREFIX  ${CMAKE_INSTALL_PREFIX}       CACHE PATH  "Base directory for executables and libraries" FORCE)
#
## the following are directories where stuff will be installed to
set(BIN_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/bin"                  CACHE PATH "The kde info install dir (default prefix/info)" FORCE)
set(LIB_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}"     CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)
set(INCLUDE_INSTALL_DIR      "${CMAKE_INSTALL_PREFIX}/include"             CACHE PATH "The subdirectory to the header prefix" FORCE)

include(ConfigureChecks.cmake)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/toolkit ${CMAKE_SOURCE_DIR}//mpeg ${CMAKE_SOURCE_DIR}/ogg ${CMAKE_SOURCE_DIR}/flac ${CMAKE_SOURCE_DIR}/mpc ${CMAKE_SOURCE_DIR}/ogg/vorbis ${CMAKE_SOURCE_DIR}/mpeg/id3v2/ ${CMAKE_SOURCE_DIR}/mpeg/id3v1 ${CMAKE_SOURCE_DIR}/ ${CMAKE_SOURCE_DIR}/ape ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/taglib-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/taglib-config )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/taglib.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/taglib.pc )
configure_file(config-taglib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-taglib.h )

ADD_SUBDIRECTORY( toolkit ) 
ADD_SUBDIRECTORY( mpeg ) 
ADD_SUBDIRECTORY( ogg ) 
ADD_SUBDIRECTORY( flac ) 
ADD_SUBDIRECTORY( ape ) 
ADD_SUBDIRECTORY( mpc ) 



########### next target ###############

SET(mpeg_SRCS
mpeg/mpegfile.cpp
mpeg/mpegproperties.cpp
mpeg/mpegheader.cpp
mpeg/xingheader.cpp
)

SET(id3v1_SRCS
mpeg/id3v1/id3v1tag.cpp
mpeg/id3v1/id3v1genres.cpp
)


SET(id3v2_SRCS
mpeg/id3v2/id3v2framefactory.cpp
mpeg/id3v2/id3v2synchdata.cpp
mpeg/id3v2/id3v2tag.cpp
mpeg/id3v2/id3v2header.cpp
mpeg/id3v2/id3v2frame.cpp
mpeg/id3v2/id3v2footer.cpp
mpeg/id3v2/id3v2extendedheader.cpp
)


SET(frames_SRCS
mpeg/id3v2/frames/attachedpictureframe.cpp
mpeg/id3v2/frames/commentsframe.cpp
mpeg/id3v2/frames/generalencapsulatedobjectframe.cpp
mpeg/id3v2/frames/relativevolumeframe.cpp
mpeg/id3v2/frames/textidentificationframe.cpp
mpeg/id3v2/frames/uniquefileidentifierframe.cpp
mpeg/id3v2/frames/unknownframe.cpp
)

SET(ogg_SRCS
ogg/oggfile.cpp
ogg/oggpage.cpp
ogg/oggpageheader.cpp
ogg/xiphcomment.cpp
)

SET(vorbis_SRCS
ogg/vorbis/vorbisfile.cpp
ogg/vorbis/vorbisproperties.cpp
)

SET(oggflac_SRCS
ogg/flac/oggflacfile.cpp
)

SET(mpc_SRCS
mpc/mpcfile.cpp
mpc/mpcproperties.cpp
)

SET(ape_SRCS
ape/apetag.cpp
ape/apefooter.cpp
ape/apeitem.cpp
)

SET(toolkit_SRCS
toolkit/tstring.cpp
toolkit/tstringlist.cpp
toolkit/tbytevector.cpp
toolkit/tbytevectorlist.cpp
toolkit/tfile.cpp
toolkit/tdebug.cpp
toolkit/unicode.cpp
)

SET(tag_LIB_SRCS ${mpeg_SRCS} ${id3v1_SRCS} ${id3v2_SRCS} ${frames_SRCS} ${ogg_SRCS} 
		 ${vorbis_SRCS} ${oggflacs_SRCS} ${mpc_SRCS} ${ape_SRCS} ${toolkit_SRCS}
		 tag.cpp
		 fileref.cpp
		 audioproperties.cpp
)


ADD_LIBRARY(tag SHARED ${tag_LIB_SRCS})

TARGET_LINK_LIBRARIES(tag )
if(ZLIB_FOUND)
	TARGET_LINK_LIBRARIES(tag z)
endif(ZLIB_FOUND)

SET_TARGET_PROPERTIES(tag PROPERTIES VERSION 1.4.0 SOVERSION 1 )
INSTALL(TARGETS tag DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

INSTALL( FILES  ${CMAKE_CURRENT_BINARY_DIR}/taglib.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
INSTALL( FILES  ${CMAKE_CURRENT_BINARY_DIR}/taglib-config DESTINATION ${BIN_INSTALL_DIR})
INSTALL( FILES  tag.h fileref.h audioproperties.h DESTINATION ${INCLUDE_INSTALL_DIR}/taglib)


