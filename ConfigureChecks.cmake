include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)

# Check if the size of numeric types are suitable.

check_type_size("short" SIZEOF_SHORT)
if(NOT ${SIZEOF_SHORT} EQUAL 2)
  message(FATAL_ERROR "TagLib requires that short is 16-bit wide.")
endif()

check_type_size("int" SIZEOF_INT)
if(NOT ${SIZEOF_INT} EQUAL 4)
  message(FATAL_ERROR "TagLib requires that int is 32-bit wide.")
endif()

check_type_size("long long" SIZEOF_LONGLONG)
if(NOT ${SIZEOF_LONGLONG} EQUAL 8)
  message(FATAL_ERROR "TagLib requires that long long is 64-bit wide.")
endif()

check_type_size("wchar_t" SIZEOF_WCHAR_T)
if(NOT ${SIZEOF_WCHAR_T} GREATER 1)
  message(FATAL_ERROR "TagLib requires that wchar_t is sufficient to store a UTF-16 char.")
endif()

check_type_size("float" SIZEOF_FLOAT)
if(NOT ${SIZEOF_FLOAT} EQUAL 4)
  message(FATAL_ERROR "TagLib requires that float is 32-bit wide.")
endif()

check_type_size("double" SIZEOF_DOUBLE)
if(NOT ${SIZEOF_DOUBLE} EQUAL 8)
  message(FATAL_ERROR "TagLib requires that double is 64-bit wide.")
endif()

# Determine which kind of byte swap functions your compiler supports.

check_cxx_source_compiles("
  int main() {
    __builtin_bswap16(0);
    __builtin_bswap32(0);
    __builtin_bswap64(0);
    return 0;
  }
" HAVE_GCC_BYTESWAP)

if(NOT HAVE_GCC_BYTESWAP)
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
      #include <cstdlib>
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

# Determine whether your compiler supports ISO _strdup.

check_cxx_source_compiles("
  #include <cstring>
  int main() {
    _strdup(0);
    return 0;
  }
" HAVE_ISO_STRDUP)

# Detect WinRT mode
if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  set(PLATFORM_WINRT 1)
endif()
