SET(test_HDR_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/toolkit
  ${CMAKE_CURRENT_SOURCE_DIR}/../bindings/c
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpeg/id3v1
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpeg/id3v2
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpeg/id3v2/frames
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpeg
)
IF(WITH_APE)
  SET(test_HDR_DIRS ${test_HDR_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ape
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpc
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/wavpack
  )
ENDIF()
IF(WITH_ASF)
  SET(test_HDR_DIRS ${test_HDR_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/asf
  )
ENDIF()
IF(WITH_MP4)
  SET(test_HDR_DIRS ${test_HDR_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mp4
  )
ENDIF()
IF(WITH_RIFF)
  SET(test_HDR_DIRS ${test_HDR_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/riff
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/riff/aiff
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/riff/wav
  )
ENDIF()
IF(WITH_TRUEAUDIO)
  SET(test_HDR_DIRS ${test_HDR_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/trueaudio
  )
ENDIF()
IF(WITH_VORBIS)
  SET(test_HDR_DIRS ${test_HDR_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg/vorbis
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg/flac
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg/speex
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg/opus
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/flac
  )
ENDIF()
IF(WITH_MOD)
  SET(test_HDR_DIRS ${test_HDR_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mod
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/s3m
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/it
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/xm
  )
ENDIF()
IF(WITH_DSF)
  SET(test_HDR_DIRS ${test_HDR_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/dsf
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/dsdiff
  )
ENDIF()
IF(WITH_SHORTEN)
  SET(test_HDR_DIRS ${test_HDR_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/shorten
  )
ENDIF()
INCLUDE_DIRECTORIES(${test_HDR_DIRS})

SET(test_runner_SRCS
  main.cpp
  test_list.cpp
  test_map.cpp
  test_mpeg.cpp
  test_synchdata.cpp
  test_bytevector.cpp
  test_bytevectorlist.cpp
  test_bytevectorstream.cpp
  test_string.cpp
  test_propertymap.cpp
  test_variant.cpp
  test_complexproperties.cpp
  test_file.cpp
  test_fileref.cpp
  test_id3v1.cpp
  test_id3v2.cpp
  test_id3v2framefactory.cpp
  test_sizes.cpp
  test_versionnumber.cpp
)
IF(WITH_TRUEAUDIO)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_trueaudio.cpp
  )
ENDIF()
IF(WITH_VORBIS)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_xiphcomment.cpp
    test_ogg.cpp
    test_oggflac.cpp
    test_flac.cpp
    test_flacpicture.cpp
    test_flacunknownmetadatablock.cpp
    test_opus.cpp
    test_speex.cpp
  )
ENDIF()
IF(WITH_RIFF)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_aiff.cpp
    test_riff.cpp
    test_wav.cpp
    test_info.cpp
  )
ENDIF()
if(WITH_APE)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_ape.cpp
    test_apetag.cpp
    test_wavpack.cpp
    test_mpc.cpp
  )
ENDIF()
IF(WITH_MP4)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_mp4.cpp
    test_mp4item.cpp
    test_mp4coverart.cpp
  )
ENDIF()
IF(WITH_ASF)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_asf.cpp
  )
ENDIF()
IF(WITH_MOD)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_mod.cpp
    test_s3m.cpp
    test_it.cpp
    test_xm.cpp
  )
ENDIF()
IF(WITH_DSF)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_dsf.cpp
    test_dsdiff.cpp
  )
ENDIF()
IF(WITH_SHORTEN)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_shorten.cpp
  )
ENDIF()
IF(BUILD_BINDINGS)
  SET(test_runner_SRCS ${test_runner_SRCS}
    test_tag_c.cpp
  )
ENDIF()

INCLUDE_DIRECTORIES(${CPPUNIT_INCLUDE_DIR})

ADD_EXECUTABLE(test_runner ${test_runner_SRCS})
TARGET_LINK_LIBRARIES(test_runner tag ${CPPUNIT_LIBRARIES})
IF(BUILD_BINDINGS)
  TARGET_LINK_LIBRARIES(test_runner tag_c)
ENDIF()

ADD_TEST(test_runner test_runner)
ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND} -V
                  DEPENDS test_runner)
