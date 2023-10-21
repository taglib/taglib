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

#include <cstdint>
#include "ebmlelement.h"
#include "ebmlutils.h"
#include "tbytevector.h"
#include "tstring.h"

namespace TagLib {
  class File;
  namespace EBML {
    template<String::Type t>
    class StringElement : public Element
    {
    public:
      StringElement(Id id, int sizeLength, offset_t dataSize)
      : Element(id, sizeLength, dataSize)
      {}
      StringElement(Id id)
      : Element(id, 0, 0)
      {}
      const String& getValue() const { return value; }
      void setValue(const String &value) { this->value = value; }
      bool read(File &file) override;
      ByteVector render() override;

    private:
      String value;
    };

    using UTF8StringElement = StringElement<String::UTF8>;
    using Latin1StringElement = StringElement<String::Latin1>;
  }
}

#endif
#endif
