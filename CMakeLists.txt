cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

project(taglib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(CTest)
include(FeatureSummary)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
if(APPLE)
  option(BUILD_FRAMEWORK "Build an OS X framework" OFF)
  if(BUILD_FRAMEWORK)
    set(BUILD_SHARED_LIBS ON)
    #set(CMAKE_MACOSX_RPATH 1)
    set(FRAMEWORK_INSTALL_DIR "/Library/Frameworks" CACHE STRING "Directory to install frameworks to.")
  endif()
endif()
if(NOT BUILD_SHARED_LIBS)
  add_definitions(-DTAGLIB_STATIC)
endif()
option(ENABLE_STATIC_RUNTIME "Visual Studio, link with runtime statically" OFF)

option(ENABLE_CCACHE "Use ccache when building libtag" OFF)
if(ENABLE_CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  endif()
endif()

option(VISIBILITY_HIDDEN "Build with -fvisibility=hidden" OFF)
option(BUILD_EXAMPLES "Build the examples" OFF)
option(BUILD_BINDINGS "Build the bindings" ON)

option(NO_ITUNES_HACKS "Disable workarounds for iTunes bugs" OFF)

option(PLATFORM_WINRT "Enable WinRT support" OFF)
if(PLATFORM_WINRT)
  add_definitions(-DPLATFORM_WINRT)
endif()

set(TAGLIB_INSTALL_SUFFIX "" CACHE STRING
  "Suffix added to installed files (include directory, libraries, .pc)")

add_definitions(-DHAVE_CONFIG_H)
set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/")

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if(MSVC)
  if(ENABLE_STATIC_RUNTIME)
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)
  endif()
endif()

# Read version information from file taglib/toolkit/taglib.h into variables
# TAGLIB_LIB_MAJOR_VERSION, TAGLIB_LIB_MINOR_VERSION, TAGLIB_LIB_PATCH_VERSION.
foreach(version_part MAJOR MINOR PATCH)
  set(version_var_name "TAGLIB_${version_part}_VERSION")
  file(STRINGS taglib/toolkit/taglib.h version_line
       REGEX "^#define +${version_var_name}")
  if(NOT version_line)
    message(FATAL_ERROR "${version_var_name} not found in taglib.h")
  endif()
  string(REGEX MATCH "${version_var_name} +([^ ]+)" result ${version_line})
  set(TAGLIB_LIB_${version_part}_VERSION ${CMAKE_MATCH_1})
endforeach(version_part)

# Only used to force cmake rerun when taglib.h changes.
configure_file(taglib/toolkit/taglib.h ${CMAKE_CURRENT_BINARY_DIR}/taglib.h.stamp)

if("${TAGLIB_LIB_PATCH_VERSION}" EQUAL "0")
  set(TAGLIB_LIB_VERSION_STRING "${TAGLIB_LIB_MAJOR_VERSION}.${TAGLIB_LIB_MINOR_VERSION}")
else()
  set(TAGLIB_LIB_VERSION_STRING "${TAGLIB_LIB_MAJOR_VERSION}.${TAGLIB_LIB_MINOR_VERSION}.${TAGLIB_LIB_PATCH_VERSION}")
endif()

# Major version: increase it if you break ABI compatibility.
# Minor version: increase it if you add ABI compatible features.
# Patch version: increase it for bug fix releases.
set(TAGLIB_SOVERSION_MAJOR 2)
set(TAGLIB_SOVERSION_MINOR 0)
set(TAGLIB_SOVERSION_PATCH 2)

include(ConfigureChecks.cmake)

# Determine whether zlib is installed.
option(WITH_ZLIB "Build with ZLIB" ON)

if(WITH_ZLIB)
  find_package("ZLIB")
  set(HAVE_ZLIB ${ZLIB_FOUND})
  if(ZLIB_FOUND)
    set(ZLIB_LIBRARIES_FLAGS -lz)
    if(NOT BUILD_SHARED_LIBS)
      # When linking TagLib statically, zlib has to be linked explicitly.
      set(ZLIB_INTERFACE_LINK_LIBRARIES ZLIB::ZLIB)
    endif()
  endif()
endif()

if(NOT WIN32)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/taglib-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/taglib-config" @ONLY)
  install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/taglib-config" DESTINATION "${CMAKE_INSTALL_BINDIR}"
          RENAME "taglib${TAGLIB_INSTALL_SUFFIX}-config")
endif()

if(WIN32)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/taglib-config.cmd.cmake" "${CMAKE_CURRENT_BINARY_DIR}/taglib-config.cmd")
  install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/taglib-config.cmd" DESTINATION "${CMAKE_INSTALL_BINDIR}"
          RENAME "taglib${TAGLIB_INSTALL_SUFFIX}-config.cmd")
endif()

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
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/taglib.pc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/taglib.pc" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/taglib.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
          RENAME "taglib${TAGLIB_INSTALL_SUFFIX}.pc")
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(config.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/config.h")

option(TRACE_IN_RELEASE "Output debug messages even in release mode" OFF)
if(TRACE_IN_RELEASE)
  set(TRACE_IN_RELEASE TRUE)
endif()

find_package(utf8cpp QUIET)
if(utf8cpp_FOUND)
  message(STATUS "Using utfcpp ${utf8cpp_VERSION} from ${utf8cpp_CONFIG}")
else()
  find_path(utf8cpp_INCLUDE_DIR NAMES utf8.h PATH_SUFFIXES utf8cpp
            DOC "utf8cpp include directory")
  mark_as_advanced(utf8cpp_INCLUDE_DIR)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(utf8cpp REQUIRED_VARS utf8cpp_INCLUDE_DIR)
  if(utf8cpp_FOUND)
    set(utf8cpp_INCLUDE_DIRS "${utf8cpp_INCLUDE_DIR}")
    if(NOT TARGET utf8::cpp)
      add_library(utf8::cpp INTERFACE IMPORTED)
      set_target_properties(utf8::cpp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${utf8cpp_INCLUDE_DIR}")
    endif()
    message(STATUS "Using utfcpp from ${utf8cpp_INCLUDE_DIR}")
  else()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/utfcpp/CMakeLists.txt)
      add_subdirectory("3rdparty/utfcpp")
      message(STATUS "Using utfcpp from ${utf8cpp_SOURCE_DIR}")
    else()
      message(FATAL_ERROR
        "utfcpp not found. Either install package (probably utfcpp, utf8cpp, or libutfcpp-dev) "
        "or fetch the git submodule using\n"
        "git submodule update --init")
    endif()
  endif()
endif()

add_subdirectory(taglib)

if(BUILD_BINDINGS)
  add_subdirectory(bindings)
endif()

if(BUILD_TESTING)
  find_package(CppUnit)
  if(CppUnit_FOUND)
    add_subdirectory(tests)
  else()
    message(WARNING "BUILD_TESTING requested, but CppUnit not found, skipping tests.")
  endif()
endif()

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
add_custom_target(docs doxygen)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

if(NOT TARGET uninstall)
  add_custom_target(uninstall COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif()

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
