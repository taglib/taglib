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

#ifndef TAGLIB_EBMLBINARYELEMENT_H
#define TAGLIB_EBMLBINARYELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include "ebmlelement.h"

namespace TagLib {
  class File;
  namespace EBML {
    class BinaryElement : public Element
    {
    public:
      BinaryElement(Id id, int sizeLength, offset_t dataSize) :
        Element(id, sizeLength, dataSize)
      {
      }
      BinaryElement(Id id, int sizeLength, offset_t dataSize, offset_t) :
        Element(id, sizeLength, dataSize)
      {
      }

      explicit BinaryElement(Id id) :
        Element(id, 0, 0)
      {
      }
      const ByteVector &getValue() const { return value; }
      void setValue(const ByteVector &value) { this->value = value; }
      bool read(File &file) override;
      ByteVector render() override;

    private:
      ByteVector value;
    };
  }
}

#endif
#endif
