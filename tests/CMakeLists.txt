include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/toolkit
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ape
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/asf
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpeg/id3v1
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpeg/id3v2
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpeg/id3v2/frames
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpeg
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mpc
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mp4
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/riff
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/riff/aiff
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/riff/wav
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/trueaudio
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg/vorbis
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg/flac
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg/speex
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ogg/opus
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/flac
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/wavpack
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/mod
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/s3m
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/it
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/xm
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/dsf
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ebml
  ${CMAKE_CURRENT_SOURCE_DIR}/../taglib/ebml/matroska
)

set(test_runner_SRCS
  main.cpp
  test_list.cpp
  test_map.cpp
  test_smartptr.cpp
  test_mpeg.cpp
  test_synchdata.cpp
  test_trueaudio.cpp
  test_bytevector.cpp
  test_bytevectorlist.cpp
  test_bytevectorstream.cpp
  test_string.cpp
  test_propertymap.cpp
  test_file.cpp
  test_fileref.cpp
  test_id3v1.cpp
  test_id3v2.cpp
  test_xiphcomment.cpp
  test_aiff.cpp
  test_riff.cpp
  test_ogg.cpp
  test_oggflac.cpp
  test_flac.cpp
  test_flacpicture.cpp
  test_flacunknownmetadatablock.cpp
  test_ape.cpp
  test_apetag.cpp
  test_wav.cpp
  test_info.cpp
  test_wavpack.cpp
  test_mp4.cpp
  test_mp4item.cpp
  test_mp4coverart.cpp
  test_asf.cpp
  test_mod.cpp
  test_s3m.cpp
  test_it.cpp
  test_xm.cpp
  test_mpc.cpp
  test_opus.cpp
  test_speex.cpp
  test_dsf.cpp
  test_matroska.cpp
)

include_directories(${CPPUNIT_INCLUDE_DIR})

add_executable(test_runner ${test_runner_SRCS})
target_link_libraries(test_runner tag ${CPPUNIT_LIBRARIES})

if(WIN32 AND BUILD_SHARED_LIBS)
  get_target_property(dll_location tag LOCATION)
  get_filename_component(dll_location "${dll_location}" PATH)

#  file(TO_NATIVE_PATH ... ...)
#    leaves it with forward slashes on native windows with MinGW ?
# So we do it the old fashion way ...
  string(REPLACE "/" "\\" dll_location "${dll_location}")

  set(varname "PATH")
  set(pathvar "${dll_location};$ENV{PATH}")
  # Protect Windows semi-colon seperated path environment variable
  string(REPLACE ";" "\;" pathvar "${pathvar}")
  unset(dll_location)
endif()

add_test(test_runner test_runner)

if(WIN32 AND BUILD_SHARED_LIBS)
  set_tests_properties(test_runner PROPERTIES ENVIRONMENT "${varname}=${pathvar}")
endif()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V DEPENDS test_runner)

