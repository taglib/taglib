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

# Check where to find the byteorder-related functions (htobe32 etc).
check_cxx_source_compiles("
    #include <endian.h>
    int main() {
        htobe64(0);
    }
" BYTEORDER_IN_ENDIAN_H)
check_cxx_source_compiles("
    #include <sys/endian.h>
    int main() {
        htobe64(0);
    }
" BYTEORDER_IN_SYS_ENDIAN_H)
check_cxx_source_compiles("
    #include <sys/types.h>
    int main() {
        htobe64(0);
    }
" BYTEORDER_IN_SYS_TYPES_H)
