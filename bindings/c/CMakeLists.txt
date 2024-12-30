include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/toolkit
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/asf
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/mpeg
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/ogg
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/ogg/vorbis
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/ogg/flac
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/flac
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/mpc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/mp4
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/mpeg/id3v2
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/mpeg/id3v2/frames
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/wavpack
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/ogg/speex
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/trueaudio
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/riff
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/riff/aiff
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/riff/wav
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/ape
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/it
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/mod
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/s3m
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/xm
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/ogg/opus
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/dsf
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/dsdiff
  ${CMAKE_CURRENT_SOURCE_DIR}/../../taglib/shorten
)

set(tag_c_HDRS tag_c.h)

add_library(tag_c tag_c.cpp ${tag_c_HDRS})

target_include_directories(tag_c INTERFACE
  $<INSTALL_INTERFACE:include/taglib${TAGLIB_INSTALL_SUFFIX}>
)

target_link_libraries(tag_c PRIVATE tag)
set_target_properties(tag_c PROPERTIES
  PUBLIC_HEADER "${tag_c_HDRS}"
  DEFINE_SYMBOL MAKE_TAGLIB_LIB
)
if(VISIBILITY_HIDDEN)
  set_target_properties(tag_c PROPERTIES C_VISIBILITY_PRESET hidden)
endif()
if(BUILD_FRAMEWORK)
  set_target_properties(tag_c PROPERTIES FRAMEWORK TRUE)
endif()

# On Solaris we need to explicitly add the C++ standard and runtime
# libraries to the libs used by the C bindings, because those C bindings
# themselves won't pull in the C++ libs -- and if a C application is
# using the C bindings then we get link errors.
check_library_exists(Crun __RTTI___ "" HAVE_CRUN_LIB)
if(HAVE_CRUN_LIB)
  # Which libraries to link depends critically on which
  # STL version is going to be used by your application
  # and which runtime is in use. While Crun is pretty much
  # the only game in town, the three available STLs -- Cstd,
  # stlport4 and stdcxx -- make this a mess. The KDE-Solaris
  # team supports stdcxx (Apache RogueWave stdcxx 4.1.3).

  # According to http://bugs.kde.org/show_bug.cgi?id=215225 the library can have the following two names:
  find_library(ROGUEWAVE_STDCXX_LIBRARY NAMES stdcxx4 stdcxx)
  if(NOT ROGUEWAVE_STDCXX_LIBRARY)
    message(FATAL_ERROR "Did not find supported STL library (tried stdcxx4 and stdcxx)")
  endif()
  target_link_libraries(tag_c ${ROGUEWAVE_STDCXX_LIBRARY} Crun)
endif()

set_target_properties(tag_c PROPERTIES
  VERSION ${TAGLIB_SOVERSION_MAJOR}.${TAGLIB_SOVERSION_MINOR}.${TAGLIB_SOVERSION_PATCH}
  SOVERSION ${TAGLIB_SOVERSION_MAJOR}
  DEFINE_SYMBOL MAKE_TAGLIB_C_LIB
  INSTALL_NAME_DIR ${CMAKE_INSTALL_FULL_LIBDIR}
)

if(TAGLIB_INSTALL_SUFFIX)
  if(BUILD_SHARED_LIBS)
    set(TAGLIB_LIBRARY_SUFFIX "${TAGLIB_INSTALL_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  else()
    set(TAGLIB_LIBRARY_SUFFIX "${TAGLIB_INSTALL_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif()
  set_target_properties(tag_c PROPERTIES SUFFIX ${TAGLIB_LIBRARY_SUFFIX})
endif()

install(TARGETS tag_c
  EXPORT taglibTargets
  FRAMEWORK DESTINATION ${FRAMEWORK_INSTALL_DIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/taglib${TAGLIB_INSTALL_SUFFIX}
)

if(NOT BUILD_FRAMEWORK)
  if(IS_ABSOLUTE ${CMAKE_INSTALL_INCLUDEDIR})
    set(CMAKE_PC_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
  else()
    set(CMAKE_PC_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
  endif()
  if(IS_ABSOLUTE ${CMAKE_INSTALL_LIBDIR})
    set(CMAKE_PC_LIBDIR ${CMAKE_INSTALL_LIBDIR})
  else()
    set(CMAKE_PC_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
  endif()
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/taglib_c.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/taglib_c.pc @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/taglib_c.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
          RENAME taglib${TAGLIB_INSTALL_SUFFIX}_c.pc)
endif()
