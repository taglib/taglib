/***************************************************************************
    copyright            : (C) 2013 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifndef TAGLIB_BYTESWAP_H
#define TAGLIB_BYTESWAP_H

#include "taglib.h"

#ifndef DO_NOT_DOCUMENT // Tell Doxygen to skip this class.
/*!
 * \internal
 * The functions in this file in only for internal use.
 *
 * \warning This <b>is not</b> part of the TagLib public API!
 */

// Determines CPU byte order at compile time rather than run time if possible.

#if (defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))) \
  || (defined(__GNUC__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) \
  || (defined(__clang__) && defined(__LITTLE_ENDIAN__))

# define TAGLIB_IS_LITTLE_ENDIAN true

#elif (defined(__GNUC__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) \
  || (defined(__clang__) && defined(__BIG_ENDIAN__))

# define TAGLIB_IS_LITTLE_ENDIAN false

#else

# define TAGLIB_IS_LITTLE_ENDIAN isLittleEndianSystem

#endif

// Determines if compiler intrinsic functions are available.

// MSVC: Intrinsic _byteswap_* functions.
#if defined(_MSC_VER) && _MSC_VER >= 1400
# include <stdlib.h>
# define TAGLIB_BYTESWAP_MSC

// GCC 4.8 or above: __builtin_bswap16(), 32 and 64.
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
# define TAGLIB_BYTESWAP_GCC 2

// GCC 4.3 or above: __builtin_bswap16 is missing.
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
# define TAGLIB_BYTESWAP_GCC 1

#endif

// Determines if platform or library specific functions are available.

#if defined(__APPLE__)
#   include <libkern/OSByteOrder.h>
#   define TAGLIB_BYTESWAP_MAC

#elif defined(__OpenBSD__)
#   include <sys/endian.h>
#   define TAGLIB_BYTESWAP_OPENBSD

#elif defined(__GLIBC__)
#   include <byteswap.h>
#   define TAGLIB_BYTESWAP_GLIBC

#endif

namespace TagLib 
{
  // Cross-platform byte order conversion functions.
  // In order to encourage inlining, these functions would better be 
  // in same translation unit as callers.

  /*!
   * Converts the byte order of \a x as a 16-bit unsigned integer.
   */
  inline ushort byteSwap16(ushort x)
  {
#if defined(TAGLIB_BYTESWAP_MSC)

    return _byteswap_ushort(x);

#elif defined(TAGLIB_BYTESWAP_GCC) && TAGLIB_BYTESWAP_GCC == 2

    return __builtin_bswap16(x);

#elif defined(TAGLIB_BYTESWAP_MAC)

    return OSSwapInt16(x);

#elif defined(TAGLIB_BYTESWAP_OPENBSD)

    return swap16(x);

#elif defined(TAGLIB_BYTESWAP_GLIBC)

    return __bswap_16(x);

#else

    return ((x >> 8) & 0xff) | ((x & 0xff) << 8);

#endif
  }

  /*!
   * Converts the byte order of \a x as a 32-bit unsigned integer.
   */
  inline uint byteSwap32(uint x)
  {
#if defined(TAGLIB_BYTESWAP_MSC)

    return _byteswap_ulong(x);

#elif defined(TAGLIB_BYTESWAP_GCC) 

    return __builtin_bswap32(x);

#elif defined(TAGLIB_BYTESWAP_MAC)

    return OSSwapInt32(x);

#elif defined(TAGLIB_BYTESWAP_OPENBSD)

    return swap32(x);

#elif defined(TAGLIB_BYTESWAP_GLIBC)

    return __bswap_32(x);

#else

    return ((x & 0xff000000) >> 24) 
      | ((x & 0x00ff0000) >>  8)
      | ((x & 0x0000ff00) <<  8) 
      | ((x & 0x000000ff) << 24);

#endif
  }

  /*!
   * Converts the byte order of \a x as a 64-bit unsigned integer.
   */
  inline ulonglong byteSwap64(ulonglong x)
  {
#if defined(TAGLIB_BYTESWAP_MSC)

    return _byteswap_uint64(x);

#elif defined(TAGLIB_BYTESWAP_GCC) 

    return __builtin_bswap64(x);

#elif defined(TAGLIB_BYTESWAP_MAC)

    return OSSwapInt64(x);

#elif defined(TAGLIB_BYTESWAP_OPENBSD)

    return swap64(x);

#elif defined(TAGLIB_BYTESWAP_GLIBC)

    return __bswap_64(x);

#else

    return ((x & 0xff00000000000000ull) >> 56)    
      | ((x & 0x00ff000000000000ull) >> 40)            
      | ((x & 0x0000ff0000000000ull) >> 24)            
      | ((x & 0x000000ff00000000ull) >>  8)             
      | ((x & 0x00000000ff000000ull) <<  8)             
      | ((x & 0x0000000000ff0000ull) << 24)            
      | ((x & 0x000000000000ff00ull) << 40)            
      | ((x & 0x00000000000000ffull) << 56);

#endif
  }

  /*!
   * Indicates the system byte order. \a true if little endian, \a false if big endian.
   *
   * \note Do not use this directly. Use \a TAGLIB_IS_LITTLE_ENDIAN macro instead.
   */
  extern const bool isLittleEndianSystem;
}

#endif
#endif
