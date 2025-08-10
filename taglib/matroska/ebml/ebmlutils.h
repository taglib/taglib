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

#ifndef TAGLIB_EBMLUTILS_H
#define TAGLIB_EBMLUTILS_H
#ifndef DO_NOT_DOCUMENT

#include <utility>
#include "taglib.h"
#include "tutils.h"
#include "tdebug.h"
#include "ebmlelement.h"
#include "tbytevector.h"

namespace TagLib {
  class File;
  class ByteVector;

  namespace EBML {
    template <int maxSizeLength>
    constexpr unsigned int VINTSizeLength(uint8_t firstByte)
    {
      static_assert(maxSizeLength >= 1 && maxSizeLength <= 8);
      if(!firstByte) {
        debug("VINT with greater than 8 bytes not allowed");
        return 0;
      }
      uint8_t mask = 0b10000000;
      unsigned int numBytes = 1;
      while(!(mask & firstByte)) {
        numBytes++;
        mask >>= 1;
      }
      if(numBytes > maxSizeLength) {
        debug(Utils::formatString("VINT size length exceeds %i bytes", maxSizeLength));
        return 0;
      }
      return numBytes;
    }

    template <typename T>
    std::pair<int, T> readVINT(File &file);
    template <typename T>
    std::pair<int, T> parseVINT(const ByteVector &buffer);
    Element *findElement(File &file, Element::Id id, offset_t maxOffset);
    Element *findNextElement(File &file, offset_t maxOffset);
    ByteVector renderVINT(uint64_t number, int minSizeLength);
    unsigned long long randomUID();

    constexpr int minSize(uint64_t data)
    {
      if(data <= 0x7Fu)
        return 1;
      else if(data <= 0x3FFFu)
        return 2;
      else if(data <= 0x1FFFFFu)
        return 3;
      else if(data <= 0xFFFFFFFu)
        return 4;
      else if(data <= 0x7FFFFFFFFu)
        return 5;
      else
        return 0;
    }

    constexpr int idSize(Element::Id id)
    {
      if(id <= 0xFF)
        return 1;
      else if(id <= 0xFFFF)
        return 2;
      else if(id <= 0xFFFFFF)
        return 3;
      else
        return 4;
    }
  }
}

#endif
#endif
