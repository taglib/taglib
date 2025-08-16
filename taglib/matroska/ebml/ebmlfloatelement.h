/***************************************************************************
    copyright            : (C) 2025 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

#ifndef TAGLIB_EBMLFLOATELEMENT_H
#define TAGLIB_EBMLFLOATELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include <variant>
#include "ebmlelement.h"

namespace TagLib {
  class File;

  namespace EBML {
    class FloatElement : public Element
    {
    public:
      using FloatVariantType = std::variant<std::monostate, float, double>;

      FloatElement(Id id, int sizeLength, offset_t dataSize) :
        Element(id, sizeLength, dataSize)
      {
      }

      explicit FloatElement(Id id) :
        FloatElement(id, 0, 0)
      {
      }
      FloatVariantType getValue() const { return value; }
      double getValueAsDouble(double defaultValue = 0.0) const;
      void setValue(FloatVariantType val) { value = val; }
      bool read(File &file) override;
      ByteVector render() override;

    private:
      FloatVariantType value;
    };
  }
}

#endif
#endif
