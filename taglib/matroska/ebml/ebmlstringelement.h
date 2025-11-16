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

#ifndef TAGLIB_EBMLSTRINGELEMENT_H
#define TAGLIB_EBMLSTRINGELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include "ebmlelement.h"
#include "tstring.h"

namespace TagLib {
  class File;
  class ByteVector;

  namespace EBML {
    class StringElement : public Element
    {
    public:
      StringElement(String::Type stringEncoding, Id id, int sizeLength, offset_t dataSize) :
        Element(id, sizeLength, dataSize), encoding(stringEncoding) {}

      const String &getValue() const { return value; }
      void setValue(const String &val) { value = val; }
      bool read(File &file) override;
      ByteVector render() override;

    private:
      String value;
      String::Type encoding;
    };

    class UTF8StringElement : public StringElement {
    public:
      UTF8StringElement(Id id, int sizeLength, offset_t dataSize) :
        StringElement(String::UTF8, id, sizeLength, dataSize) {}
      UTF8StringElement(Id id, int sizeLength, offset_t dataSize, offset_t) :
        UTF8StringElement(id, sizeLength, dataSize) {}
      explicit UTF8StringElement(Id id) :
        UTF8StringElement(id, 0, 0) {}
    };

    class Latin1StringElement : public StringElement {
    public:
      Latin1StringElement(Id id, int sizeLength, offset_t dataSize) :
        StringElement(String::Latin1, id, sizeLength, dataSize) {}
      Latin1StringElement(Id id, int sizeLength, offset_t dataSize, offset_t) :
        Latin1StringElement(id, sizeLength, dataSize) {}
      explicit Latin1StringElement(Id id) :
        Latin1StringElement(id, 0, 0) {}
    };
  }
}

#endif
#endif
