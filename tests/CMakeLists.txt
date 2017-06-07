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
  ${CMAKE_SOURCE_DIR}/3rdparty
)

set(test_runner_SRCS
  main.cpp
  test_list.cpp
  test_map.cpp
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
  test_vorbis.cpp
  test_oggflac.cpp
  test_flac.cpp
  test_flacmetadatablock.cpp
  test_ape.cpp
  test_apetag.cpp
  test_wav.cpp
  test_info.cpp
  test_wavpack.cpp
  test_mp4.cpp
  test_mp4item.cpp
  test_asf.cpp
  test_mod.cpp
  test_s3m.cpp
  test_it.cpp
  test_xm.cpp
  test_mpc.cpp
  test_opus.cpp
  test_speex.cpp
)

add_executable(test_runner ${test_runner_SRCS})
target_link_libraries(test_runner tag)

add_test(test_runner test_runner)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V DEPENDS test_runner)
