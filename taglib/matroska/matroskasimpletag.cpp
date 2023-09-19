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

#include "matroskasimpletag.h"
#include "matroskatag.h"
#include "tstring.h"
#include "tbytevector.h"

using namespace TagLib;

class Matroska::SimpleTag::SimpleTagPrivate
{
  public:
    SimpleTagPrivate() = default;
    Tag::TargetTypeValue targetTypeValue;
    String name;

};

class Matroska::SimpleTagString::SimpleTagStringPrivate
{
  public:
    SimpleTagStringPrivate() = default;
    String value;

};

class Matroska::SimpleTagBinary::SimpleTagBinaryPrivate
{
  public:
    SimpleTagBinaryPrivate() = default;
    ByteVector value;

};

Matroska::SimpleTag::SimpleTag(Tag::TargetTypeValue targetTypeValue)
: d(std::make_unique<SimpleTagPrivate>())
{
  d->targetTypeValue = targetTypeValue;
}
Matroska::SimpleTag::~SimpleTag() = default;


Matroska::Tag::TargetTypeValue Matroska::SimpleTag::targetTypeValue() const
{
  return d->targetTypeValue;
}

void Matroska::SimpleTag::setTargetTypeValue(Matroska::Tag::TargetTypeValue targetTypeValue)
{
  d->targetTypeValue = targetTypeValue;
}

const String& Matroska::SimpleTag::name() const
{
  return d->name;
}

void Matroska::SimpleTag::setName(const String &name)
{
  d->name = name;
}

Matroska::SimpleTagString::SimpleTagString(Tag::TargetTypeValue targetTypeValue)
: Matroska::SimpleTag(targetTypeValue),
  dd(std::make_unique<SimpleTagStringPrivate>())
{

}
Matroska::SimpleTagString::~SimpleTagString() = default;

const String& Matroska::SimpleTagString::value() const
{
  return dd->value;
}

void Matroska::SimpleTagString::setValue(const String &value)
{
  dd->value = value;
}

Matroska::SimpleTagBinary::SimpleTagBinary(Tag::TargetTypeValue targetTypeValue)
: Matroska::SimpleTag(targetTypeValue),
  dd(std::make_unique<SimpleTagBinaryPrivate>())
{

}
Matroska::SimpleTagBinary::~SimpleTagBinary() = default;

const ByteVector& Matroska::SimpleTagBinary::value() const
{
  return dd->value;
}

void Matroska::SimpleTagBinary::setValue(const ByteVector &value)
{
  dd->value = value;
}
