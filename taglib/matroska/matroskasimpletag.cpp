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
#include <variant>
#include "matroskatag.h"
#include "tstring.h"
#include "tbytevector.h"

using namespace TagLib;

class Matroska::SimpleTag::SimpleTagPrivate
{
public:
  explicit SimpleTagPrivate(const String &name, const String& value,
  TargetTypeValue targetTypeValue, const String &language, bool defaultLanguage) :
    value(value), name(name), language(language),
    targetTypeValue(targetTypeValue), defaultLanguageFlag(defaultLanguage) {}
  explicit SimpleTagPrivate(const String &name, const ByteVector& value,
    TargetTypeValue targetTypeValue, const String &language, bool defaultLanguage) :
    value(value), name(name), language(language),
    targetTypeValue(targetTypeValue), defaultLanguageFlag(defaultLanguage) {}
  const std::variant<String, ByteVector> value;
  const String name;
  const String language;
  const TargetTypeValue targetTypeValue;
  const bool defaultLanguageFlag;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Matroska::SimpleTag::SimpleTag(const String &name, const String &value,
                               TargetTypeValue targetTypeValue, const String &language, bool defaultLanguage) :
  d(std::make_unique<SimpleTagPrivate>(name, value, targetTypeValue,
    language, defaultLanguage))
{
}

Matroska::SimpleTag::SimpleTag(const String &name, const ByteVector &value,
  TargetTypeValue targetTypeValue, const String &language, bool defaultLanguage) :
  d(std::make_unique<SimpleTagPrivate>(name, value, targetTypeValue,
    language, defaultLanguage))
{
}

Matroska::SimpleTag::SimpleTag(const SimpleTag &other) :
  d(std::make_unique<SimpleTagPrivate>(*other.d))
{
}

Matroska::SimpleTag::SimpleTag(SimpleTag&& other) noexcept = default;

Matroska::SimpleTag::~SimpleTag() = default;

Matroska::SimpleTag &Matroska::SimpleTag::operator=(SimpleTag &&other) = default;

Matroska::SimpleTag &Matroska::SimpleTag::operator=(const SimpleTag &other)
{
  SimpleTag(other).swap(*this);
  return *this;
}

void Matroska::SimpleTag::swap(SimpleTag &other) noexcept
{
  using std::swap;

  swap(d, other.d);
}

Matroska::SimpleTag::TargetTypeValue Matroska::SimpleTag::targetTypeValue() const
{
  return d->targetTypeValue;
}

const String &Matroska::SimpleTag::name() const
{
  return d->name;
}

const String &Matroska::SimpleTag::language() const
{
  return d->language;
}

bool Matroska::SimpleTag::defaultLanguageFlag() const
{
  return d->defaultLanguageFlag;
}

Matroska::SimpleTag::ValueType Matroska::SimpleTag::type() const
{
  return std::holds_alternative<ByteVector>(d->value) ? BinaryType : StringType;
}

String Matroska::SimpleTag::toString() const
{
  if(std::holds_alternative<String>(d->value)) {
    return std::get<String>(d->value);
  }
  return {};
}

ByteVector Matroska::SimpleTag::toByteVector() const
{
  if(std::holds_alternative<ByteVector>(d->value)) {
    return std::get<ByteVector>(d->value);
  }
  return {};
}
