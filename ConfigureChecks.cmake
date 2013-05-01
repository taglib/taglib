include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)

# Determine whether or not your compiler supports move semantics.
check_cxx_source_compiles("
  #ifdef __clang__
  # pragma clang diagnostic error \"-Wc++11-extensions\" 
  #endif
  #include <utility>
  int func(int &&x) { return x - 1; }
  int main() { return func(std::move(1)); }
" SUPPORT_MOVE_SEMANTICS)

# Determine where shared_ptr<T> is defined regardless of C++11 support.
check_cxx_source_compiles("
  #include <memory>
  int main() { std::tr1::shared_ptr<int> x; return 0; }
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

# Determine whether your compiler supports codecvt header.
check_cxx_source_compiles("
#include <codecvt>
int main() { std::codecvt_utf8_utf16<wchar_t> x; return 0; }
" HAVE_CODECVT)

# Determine whether your compiler supports some safer version of sprintf.
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

