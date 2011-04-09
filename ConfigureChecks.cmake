include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)

# check for libz using the cmake supplied FindZLIB.cmake
find_package(ZLIB)
if(ZLIB_FOUND)
	set(HAVE_ZLIB 1)
else()
	set(HAVE_ZLIB 0)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
find_package(CppUnit)
if(NOT CppUnit_FOUND AND BUILD_TESTS)
	message(STATUS "CppUnit not found, disabling tests.")
	set(BUILD_TESTS OFF)
endif()

