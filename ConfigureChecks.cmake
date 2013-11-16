include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)
include(TestBigEndian)

# Check if the size of integral types are suitable.

check_type_size("short" SIZEOF_SHORT)
if(NOT ${SIZEOF_SHORT} EQUAL 2)
  MESSAGE(FATAL_ERROR "TagLib requires that short is 16-bit wide.")
endif()

check_type_size("int" SIZEOF_INT)
if(NOT ${SIZEOF_INT} EQUAL 4)
  MESSAGE(FATAL_ERROR "TagLib requires that int is 32-bit wide.")
endif()

check_type_size("long long" SIZEOF_LONGLONG)
if(NOT ${SIZEOF_LONGLONG} EQUAL 8)
  MESSAGE(FATAL_ERROR "TagLib requires that long long is 64-bit wide.")
endif()

check_type_size("wchar_t" SIZEOF_WCHAR_T)
if(${SIZEOF_WCHAR_T} LESS 2)
  MESSAGE(FATAL_ERROR "TagLib requires that wchar_t is sufficient to store a UTF-16 char.")
endif()

# Determine the CPU byte order.

test_big_endian(IS_BIG_ENDIAN)

if(NOT IS_BIG_ENDIAN)
  set(SYSTEM_BYTEORDER 1)
else()
  set(SYSTEM_BYTEORDER 2)
endif()

# Determine the size of floating point types.

check_type_size("float"       SIZEOF_FLOAT)
check_type_size("double"      SIZEOF_DOUBLE)
check_type_size("long double" SIZEOF_LONGDOUBLE)

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
  int main() { std::shared_ptr<int> x; return 0; }
" HAVE_STD_SHARED_PTR)

if(NOT HAVE_STD_SHARED_PTR)
  check_cxx_source_compiles("
    #include <tr1/memory>
    int main() { std::tr1::shared_ptr<int> x; return 0; }
  " HAVE_TR1_SHARED_PTR)

  if(NOT HAVE_TR1_SHARED_PTR)
    check_cxx_source_compiles("
      #include <boost/shared_ptr.hpp>
      int main() { boost::shared_ptr<int> x; return 0; }
    " HAVE_BOOST_SHARED_PTR)
  endif()
endif()

# Determine where unique_ptr<T> or scoped_ptr<T> is defined regardless of C++11 support.

check_cxx_source_compiles("
  #include <memory>
  int main() { std::unique_ptr<int> x; return 0; }
" HAVE_STD_UNIQUE_PTR)

if(NOT HAVE_STD_UNIQUE_PTR)
  check_cxx_source_compiles("
    #include <boost/scoped_ptr.hpp>
    int main() { boost::scoped_ptr<int> x; return 0; }
  " HAVE_BOOST_SCOPED_PTR)
endif()

# Determine which kind of atomic operations your compiler supports.

if(NOT HAVE_STD_SHARED_PTR AND NOT HAVE_TR1_SHARED_PTR AND NOT HAVE_BOOST_SHARED_PTR)
  check_cxx_source_compiles("
    int main() { 
      volatile int x;
      __sync_add_and_fetch(&x, 1);
      int y = __sync_sub_and_fetch(&x, 1);
      return 0; 
    }
  " HAVE_GCC_ATOMIC)

  if(NOT HAVE_GCC_ATOMIC)
    check_cxx_source_compiles("
      #include <libkern/OSAtomic.h>
      int main() { 
        volatile int32_t x;
        OSAtomicIncrement32Barrier(&x);
        int32_t y = OSAtomicDecrement32Barrier(&x);
        return 0; 
      }
    " HAVE_MAC_ATOMIC)

    if(NOT HAVE_MAC_ATOMIC)
      check_cxx_source_compiles("
        #include <windows.h>
        int main() { 
          volatile LONG x;
          InterlockedIncrement(&x);
          LONG y = InterlockedDecrement(&x);
          return 0; 
        }
      " HAVE_WIN_ATOMIC)

      if(NOT HAVE_WIN_ATOMIC)
        check_cxx_source_compiles("
          #include <ia64intrin.h>
          int main() { 
            volatile int x;
            __sync_add_and_fetch(&x, 1);
            int y = __sync_sub_and_fetch(&x, 1);
            return 0; 
          }
        " HAVE_IA64_ATOMIC)
      endif()
    endif()
  endif()
endif()

# Determine whether your compiler supports snpritf or sprintf_s.

check_cxx_source_compiles("
  #include <cstdio>
  int main() { char buf[20]; snprintf(buf, 20, \"%d\", 1); return 0; }
" HAVE_SNPRINTF)

if(NOT HAVE_SNPRINTF)
  check_cxx_source_compiles("
    #include <cstdio>
    int main() { char buf[20]; sprintf_s(buf, \"%d\", 1);  return 0; }
  " HAVE_SPRINTF_S)
endif()

# Check which your compiler supports ISO _strdup.

check_cxx_source_compiles("
  #include <cstring>
  int main() { 
    _strdup(0); 
    return 0; 
}
" HAVE_ISO_STRDUP)

# Determine whether your compiler supports codecvt.

check_cxx_source_compiles("
  #include <codecvt>
  int main() { 
    std::codecvt_utf8_utf16<wchar_t> x; 
    return 0; 
  }
" HAVE_STD_CODECVT)

# Determine whether zlib is installed.

find_package(ZLIB)
if(ZLIB_FOUND)
  set(HAVE_ZLIB 1)
else()
  set(HAVE_ZLIB 0)
endif()

# Determine whether CppUnit is installed.

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(CppUnit)
if(NOT CppUnit_FOUND AND BUILD_TESTS)
  message(STATUS "CppUnit not found, disabling tests.")
  set(BUILD_TESTS OFF)
endif()
