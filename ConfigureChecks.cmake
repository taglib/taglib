include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)
include(TestBigEndian)

# Determine the CPU byte order.

test_big_endian(TAGLIB_BIG_ENDIAN)

if(NOT TAGLIB_BIG_ENDIAN)
  set(TAGLIB_LITTLE_ENDIAN 1)
endif()

# Determine whether or not your compiler supports move semantics.
check_cxx_source_compiles("
    #ifdef __clang__
    # pragma clang diagnostic error \"-Wc++11-extensions\" 
    #endif
    #include <utility>
    int func(int &&x) { return x - 1; }
    int main() { return func(std::move(1)); }
" SUPPORT_MOVE_SEMANTICS)

# Determine whether or not your compiler supports template alias.
check_cxx_source_compiles("
    #ifdef __clang__
    # pragma clang diagnostic error \"-Wc++11-extensions\" 
    #endif
    #include <vector>
    template <typename T> using myVector = std::vector<T>;
    int main() { return 0; }
" SUPPORT_TEMPLATE_ALIAS)

# Determine which kind of byte swap functions your compiler supports.

# GCC's __builtin_bswap* should be checked individually 
# because some of them can be missing depends on the GCC version.
check_cxx_source_compiles("
  int main() {
    __builtin_bswap16(0);
    return 0; 
  }
" HAVE_GCC_BYTESWAP_16)

check_cxx_source_compiles("
  int main() {
    __builtin_bswap32(0);
    return 0; 
  }
" HAVE_GCC_BYTESWAP_32)

check_cxx_source_compiles("
  int main() {
    __builtin_bswap64(0);
    return 0; 
  }
" HAVE_GCC_BYTESWAP_64)

if(NOT HAVE_GCC_BYTESWAP_16 OR NOT HAVE_GCC_BYTESWAP_32 OR NOT HAVE_GCC_BYTESWAP_64)
  check_cxx_source_compiles("
    #include <byteswap.h>
    int main() {
      __bswap_16(0);
      __bswap_32(0);
      __bswap_64(0);
      return 0; 
    }
  " HAVE_GLIBC_BYTESWAP)

  if(NOT HAVE_GLIBC_BYTESWAP)
    check_cxx_source_compiles("
      #include <stdlib.h>
      int main() {
        _byteswap_ushort(0);
        _byteswap_ulong(0);
        _byteswap_uint64(0);
        return 0; 
      }
    " HAVE_MSC_BYTESWAP)

    if(NOT HAVE_MSC_BYTESWAP)
      check_cxx_source_compiles("
        #include <libkern/OSByteOrder.h>
        int main() {
          OSSwapInt16(0);
          OSSwapInt32(0);
          OSSwapInt64(0);
          return 0; 
        }
      " HAVE_MAC_BYTESWAP)

      if(NOT HAVE_MAC_BYTESWAP)
        check_cxx_source_compiles("
          #include <sys/endian.h>
          int main() {
            swap16(0);
            swap32(0);
            swap64(0);
            return 0; 
          }
        " HAVE_OPENBSD_BYTESWAP)
      endif()
    endif()
  endif()
endif()

# Determine where shared_ptr<T> is defined regardless of C++11 support.
check_cxx_source_compiles("
    #include <memory>
    int main() { std::tr1::shared_ptr<int> x; return 0; }
" HAVE_STD_SHARED_PTR)

check_cxx_source_compiles("
    #include <tr1/memory>
    int main() { std::tr1::shared_ptr<int> x; return 0; }
" HAVE_TR1_SHARED_PTR)

check_cxx_source_compiles("
    #include <boost/shared_ptr.hpp>
    int main() { boost::shared_ptr<int> x; return 0; }
" HAVE_BOOST_SHARED_PTR)

# Determine whether your compiler supports codecvt header.
check_cxx_source_compiles("
  #include <codecvt>
  int main() { std::codecvt_utf8_utf16<wchar_t> x; return 0; }
" HAVE_CODECVT)

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

