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

#ifndef TAGLIB_EBMLVOIDELEMENT_H
#define TAGLIB_EBMLVOIDELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include "ebmlelement.h"

namespace TagLib {
  class File;

  namespace EBML {
    inline constexpr offset_t MIN_VOID_ELEMENT_SIZE = 2;
    class VoidElement : public Element
    {
    public:
      VoidElement(int sizeLength, offset_t dataSize) :
        Element(Id::VoidElement, sizeLength, dataSize)
      {}
      VoidElement(Id, int sizeLength, offset_t dataSize, offset_t) :
        Element(Id::VoidElement, sizeLength, dataSize)
      {}
      VoidElement() :
        Element(Id::VoidElement, 0, 0)
      {}
      ByteVector render() override;
      offset_t getTargetSize() const;
      void setTargetSize(offset_t targetSize);
      static ByteVector renderSize(offset_t targetSize);

    private:
      offset_t targetSize = MIN_VOID_ELEMENT_SIZE;
    };
  }
}

#endif
#endif
