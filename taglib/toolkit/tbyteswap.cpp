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

#include "tbyteswap.h"

// Detects CPU byte order at runtime if unable at compile time.

#ifndef TAGLIB_IS_LITTLE_ENDIAN

namespace {
  bool isLittleEndian()
  {
    TagLib::ushort x = 1;
    return (*reinterpret_cast<TagLib::uchar*>(&x) == 1);
  }
}

#endif

namespace TagLib 
{
#if defined(TAGLIB_IS_LITTLE_ENDIAN)

  const bool isLittleEndianSystem = TAGLIB_IS_LITTLE_ENDIAN;

#else

  const bool isLittleEndianSystem = isLittleEndian();

#endif
}
