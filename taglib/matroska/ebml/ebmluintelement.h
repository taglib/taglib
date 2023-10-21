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

#ifndef TAGLIB_EBMLUINTELEMENT_H
#define TAGLIB_EBMLUINTELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include <cstdint>
#include "ebmlelement.h"

namespace TagLib {
  class File;

  namespace EBML {
    class UIntElement : public Element
    {
    public:
      UIntElement(Id id, int sizeLength, offset_t dataSize)
      : Element(id, sizeLength, dataSize)
      {}
      UIntElement(Id id)
      : UIntElement(id, 0, 0)
      {}
      unsigned int getValue() const { return value; }
      void setValue(unsigned int value) { this->value = value; }
      bool read(File &file) override;
      ByteVector render() override;

    private:
      uint64_t value = 0;

    //protected:


    };
  }
}

#endif
#endif
