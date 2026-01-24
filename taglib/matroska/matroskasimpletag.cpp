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
#include "tbytevector.h"

using namespace TagLib;

class Matroska::SimpleTag::SimpleTagPrivate
{
public:
  SimpleTagPrivate(const String &name, const String &value,
    TargetTypeValue targetTypeValue, const String &language, bool defaultLanguage,
    unsigned long long trackUid) :
    value(value), name(name), language(language), trackUid(trackUid),
    targetTypeValue(targetTypeValue), defaultLanguageFlag(defaultLanguage) {}
  SimpleTagPrivate(const String &name, const ByteVector &value,
    TargetTypeValue targetTypeValue, const String &language, bool defaultLanguage,
    unsigned long long trackUid) :
    value(value), name(name), language(language), trackUid(trackUid),
    targetTypeValue(targetTypeValue), defaultLanguageFlag(defaultLanguage) {}

  const std::variant<String, ByteVector> value;
  const String name;
  const String language;
  const unsigned long long trackUid;
  const TargetTypeValue targetTypeValue;
  const bool defaultLanguageFlag;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Matroska::SimpleTag::SimpleTag(const String &name, const String &value,
                               TargetTypeValue targetTypeValue,
                               const String &language, bool defaultLanguage,
                               unsigned long long trackUid) :
  d(std::make_unique<SimpleTagPrivate>(name, value, targetTypeValue,
    language, defaultLanguage, trackUid))
{
}

Matroska::SimpleTag::SimpleTag(const String &name, const ByteVector &value,
                               TargetTypeValue targetTypeValue,
                               const String &language, bool defaultLanguage,
                               unsigned long long trackUid) :
  d(std::make_unique<SimpleTagPrivate>(name, value, targetTypeValue,
    language, defaultLanguage, trackUid))
{
}

Matroska::SimpleTag::SimpleTag(const SimpleTag &other) :
  d(std::make_unique<SimpleTagPrivate>(*other.d))
{
}

Matroska::SimpleTag::SimpleTag(SimpleTag &&other) noexcept = default;

Matroska::SimpleTag::~SimpleTag() = default;

Matroska::SimpleTag &Matroska::SimpleTag::operator=(SimpleTag &&other) noexcept = default;

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

unsigned long long Matroska::SimpleTag::trackUid() const
{
  return d->trackUid;
}

Matroska::SimpleTag::ValueType Matroska::SimpleTag::type() const
{
  return std::holds_alternative<ByteVector>(d->value) ? BinaryType : StringType;
}

String Matroska::SimpleTag::toString() const
{
  if(std::holds_alternative<String>(d->value)) {
    // get_if() used instead of get() to support restricted compilers
    return *std::get_if<String>(&d->value);
  }
  return {};
}

ByteVector Matroska::SimpleTag::toByteVector() const
{
  if(std::holds_alternative<ByteVector>(d->value)) {
    // get_if() used instead of get() to support restricted compilers
    return *std::get_if<ByteVector>(&d->value);
  }
  return {};
}
