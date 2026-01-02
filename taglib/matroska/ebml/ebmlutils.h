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
#include "ebmlelement.h"

namespace TagLib {
  class File;
  class ByteVector;

  namespace EBML {
    std::unique_ptr<Element> findElement(File &file, Element::Id id, offset_t maxOffset);
    std::unique_ptr<Element> findNextElement(File &file, offset_t maxOffset);

    template <int maxSizeLength>
    unsigned int VINTSizeLength(uint8_t firstByte);

    std::pair<unsigned int, uint64_t> readVINT(File &file);

    std::pair<unsigned int, uint64_t> parseVINT(const ByteVector &buffer);

    ByteVector renderVINT(uint64_t number, int minSizeLength);

    unsigned long long randomUID();

    constexpr int minSize(uint64_t data)
    {
      if(data <= 0x7Fu)
        return 1;
      if(data <= 0x3FFFu)
        return 2;
      if(data <= 0x1FFFFFu)
        return 3;
      if(data <= 0xFFFFFFFu)
        return 4;
      if(data <= 0x7FFFFFFFFu)
        return 5;
      return 0;
    }

    constexpr int idSize(Element::Id id)
    {
      const auto uintId = static_cast<unsigned int>(id);
      if(uintId <= 0xFF)
        return 1;
      if(uintId <= 0xFFFF)
        return 2;
      if(uintId <= 0xFFFFFF)
        return 3;
      return 4;
    }
  }
}

#endif
#endif
