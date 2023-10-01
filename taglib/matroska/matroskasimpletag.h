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

#ifndef HAS_MATROSKASIMPLETAG_H
#define HAS_MATROSKASIMPLETAG_H

#include <memory>
#include "tag.h"
#include "matroskatag.h"

namespace TagLib {
  class String;
  class ByteVector;
  namespace Matroska {
    class TAGLIB_EXPORT SimpleTag
    {
    public:
      enum TargetTypeValue {
        None = 0,
        Shot = 10,
        Subtrack = 20,
        Track = 30,
        Part = 40,
        Album = 50,
        Edition = 60,
        Collection = 70
      };
      const String& name() const;
      TargetTypeValue targetTypeValue() const;
      void setTargetTypeValue(TargetTypeValue targetTypeValue);
      void setName(const String &name);
      virtual ~SimpleTag();

    private:
      class SimpleTagPrivate;
      std::unique_ptr<SimpleTagPrivate> d;

    protected:
      SimpleTag();
    };

    class TAGLIB_EXPORT SimpleTagString : public SimpleTag
    {
    public:
      SimpleTagString();
      ~SimpleTagString() override;
      const String& value() const;
      void setValue(const String &value);

    private:
      class SimpleTagStringPrivate;
      std::unique_ptr<SimpleTagStringPrivate> dd;
    };

    class TAGLIB_EXPORT SimpleTagBinary : public SimpleTag
    {
    public:
      SimpleTagBinary();
      ~SimpleTagBinary() override;
      const ByteVector& value() const;
      void setValue(const ByteVector &value);

    private:
      class SimpleTagBinaryPrivate;
      std::unique_ptr<SimpleTagBinaryPrivate> dd;
    };

  }
}

#endif
