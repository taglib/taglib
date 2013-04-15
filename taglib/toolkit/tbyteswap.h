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

namespace TagLib 
{
  // Cross-platform byte order conversion functions.

  /*!
   * Converts the byte order of \a x as a 16-bit unsigned integer.
   */
  ushort byteSwap16(ushort x);

  /*!
   * Converts the byte order of \a x as a 32-bit unsigned integer.
   */
  uint byteSwap32(uint x);

  /*!
   * Converts the byte order of \a x as a 64-bit unsigned integer.
   */
  ulonglong byteSwap64(ulonglong x);

  /*!
   * Indicates the system byte order.
   * \a true if little endian, \a false if big endian.
   */
  extern const bool isLittleEndianSystem;
}

#endif
