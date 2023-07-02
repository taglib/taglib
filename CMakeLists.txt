cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(taglib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(CTest)
include(FeatureSummary)
include(GNUInstallDirs)

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
  # Disable warnings for internal invocations of API functions
  # that have been marked with TAGLIB_DEPRECATED
  # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4996
  # Also disable C4910: '__declspec(dllexport)' and 'extern' are incompatible on an explicit instantiation
  # and C4251: needs to have dll-interface to be used by clients.
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4996 /wd4910 /wd4251")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4910 /wd4251")
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
set(TAGLIB_SOVERSION_PATCH 0)

include(ConfigureChecks.cmake)

# Determine whether zlib is installed.
option(WITH_ZLIB "Build with ZLIB" ON)

if(WITH_ZLIB)
  if(NOT ZLIB_SOURCE)
    find_package(ZLIB)
    if(ZLIB_FOUND)
      set(HAVE_ZLIB 1)
      set(ZLIB_LIBRARIES_FLAGS -lz)
    else()
      set(HAVE_ZLIB 0)
    endif()
  endif()

  if(NOT HAVE_ZLIB AND ZLIB_SOURCE)
    set(HAVE_ZLIB 1)
    set(HAVE_ZLIB_SOURCE 1)
  endif()
else()
  set(HAVE_ZLIB 0)
endif()

if(NOT WIN32)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/taglib-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/taglib-config" @ONLY)
  install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/taglib-config" DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(WIN32)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/taglib-config.cmd.cmake" "${CMAKE_CURRENT_BINARY_DIR}/taglib-config.cmd")
  install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/taglib-config.cmd" DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(NOT BUILD_FRAMEWORK)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/taglib.pc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/taglib.pc" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/taglib.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(config.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/config.h")

option(TRACE_IN_RELEASE "Output debug messages even in release mode" OFF)
if(TRACE_IN_RELEASE)
  set(TRACE_IN_RELEASE TRUE)
endif()

configure_file(taglib/taglib_config.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/taglib_config.h")

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
