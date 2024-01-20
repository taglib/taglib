/***************************************************************************
    copyright            : (C) 2023 by Urs Fleisch
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

#include "tvariant.h"

#include <variant>
#include <iomanip>

#include "tstring.h"
#include "tstringlist.h"
#include "tbytevector.h"
#include "tbytevectorlist.h"

using namespace TagLib;

namespace {

// The number and order of the template parameters must correspond to the
// enum values in Variant::Type!
using StdVariantType = std::variant<
  std::monostate,
  bool,
  int,
  unsigned int,
  long long,
  unsigned long long,
  double,
  TagLib::String,
  TagLib::StringList,
  TagLib::ByteVector,
  TagLib::ByteVectorList,
  List<TagLib::Variant>,
  Map<TagLib::String, TagLib::Variant>
>;

template<typename T>
T getVariantValue(StdVariantType *v, bool *ok)
{
  if(const auto valPtr = std::get_if<T>(v)) {
    if(ok) {
      *ok = true;
    }
    return *valPtr;
  }
  if(ok) {
    *ok = false;
  }
  return {};
}

void printStringToStream(std::ostream &s, const String &v)
{
  s << '"';
  for(char c : v.to8Bit(true)) {
    if(c == '"') {
      s << "\\\"";
    }
    else {
      s << c;
    }
  }
  s << '"';
}

void printByteVectorToStream(std::ostream &s, const String &v)
{
  s << '"';
  for(int c : v) {
    s << "\\x" << std::setfill('0') << std::setw(2) << std::right << std::hex
      << (c & 0xff);
  }
  s << std::dec << '"';
}

// Print a possibly recursive Variant to an ostream.
// The representation is JSON with hex strings for ByteVector.
void printVariantToStream(std::ostream &s, const StdVariantType &v)
{
  switch (v.index()) {
  case Variant::Void:
    s << "null";
    break;
  case Variant::Bool:
    if(const auto valPtr = std::get_if<Variant::Bool>(&v)) {
      s << (*valPtr ? "true" : "false");
    }
    break;
  case Variant::Int:
    if(const auto valPtr = std::get_if<Variant::Int>(&v)) {
      s << *valPtr;
    }
    break;
  case Variant::UInt:
    if(const auto valPtr = std::get_if<Variant::UInt>(&v)) {
      s << *valPtr;
    }
    break;
  case Variant::LongLong:
    if(const auto valPtr = std::get_if<Variant::LongLong>(&v)) {
      s << *valPtr;
    }
    break;
  case Variant::ULongLong:
    if(const auto valPtr = std::get_if<Variant::ULongLong>(&v)) {
      s << *valPtr;
    }
    break;
  case Variant::Double:
    if(const auto valPtr = std::get_if<Variant::Double>(&v)) {
      s << *valPtr;
    }
    break;
  case Variant::String:
    if(const auto valPtr = std::get_if<Variant::String>(&v)) {
      printStringToStream(s, *valPtr);
    }
    break;
  case Variant::StringList:
    if(const auto valPtr = std::get_if<Variant::StringList>(&v)) {
      s << '[';
      for(auto it = valPtr->cbegin(); it != valPtr->cend(); ++it) {
        if(it != valPtr->cbegin()) {
          s << ", ";
        }
        printStringToStream(s, *it);
      }
      s << ']';
    }
    break;
  case Variant::ByteVector:
    if(const auto valPtr = std::get_if<Variant::ByteVector>(&v)) {
      printByteVectorToStream(s, *valPtr);
    }
    break;
  case Variant::ByteVectorList:
    if(const auto valPtr = std::get_if<Variant::ByteVectorList>(&v)) {
      s << '[';
      for(auto it = valPtr->cbegin(); it != valPtr->cend(); ++it) {
        if(it != valPtr->cbegin()) {
          s << ", ";
        }
        printByteVectorToStream(s, *it);
      }
      s << ']';
    }
    break;
  case Variant::VariantList:
    if(const auto valPtr = std::get_if<Variant::VariantList>(&v)) {
      s << '[';
      for(auto it = valPtr->cbegin(); it != valPtr->cend(); ++it) {
        if(it != valPtr->cbegin()) {
          s << ", ";
        }
        s << *it;
      }
      s << ']';
    }
    break;
  case Variant::VariantMap:
    if(const auto valPtr = std::get_if<Variant::VariantMap>(&v)) {
      s << '{';
      for(auto it = valPtr->cbegin(); it != valPtr->cend(); ++it) {
        if(it != valPtr->cbegin()) {
          s << ", ";
        }
        printStringToStream(s, it->first);
        s << ": ";
        s << it->second;
      }
      s << '}';
    }
    break;
  }
}

} // namespace

class Variant::VariantPrivate
{
public:
  VariantPrivate() = default;
  VariantPrivate(StdVariantType v) : data(std::move(v)) {}
  StdVariantType data;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Variant::Variant() :
  d(std::make_shared<VariantPrivate>())
{
}

Variant::Variant(int val) :
  d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(unsigned int val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(long long val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(unsigned long long val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(bool val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(double val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const char *val) :
 d(std::make_shared<VariantPrivate>(TagLib::String(val)))
{
}

Variant::Variant(const TagLib::String &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::StringList &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::ByteVector &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::ByteVectorList &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::List<TagLib::Variant> &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::Map<TagLib::String, TagLib::Variant> &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const Variant &) = default;

////////////////////////////////////////////////////////////////////////////////

Variant::~Variant() = default;

Variant::Type Variant::type() const {
  return static_cast<Type>(d->data.index());
}

bool Variant::isEmpty() const
{
  return type() == Void;
}

template<typename T>
T Variant::value(bool *ok) const
{
  return getVariantValue<T>(&d->data, ok);
}

template bool Variant::value(bool *ok) const;
template int Variant::value(bool *ok) const;
template unsigned int Variant::value(bool *ok) const;
template long long Variant::value(bool *ok) const;
template unsigned long long Variant::value(bool *ok) const;
template double Variant::value(bool *ok) const;
template String Variant::value(bool *ok) const;
template StringList Variant::value(bool *ok) const;
template ByteVector Variant::value(bool *ok) const;
template ByteVectorList Variant::value(bool *ok) const;
template VariantList Variant::value(bool *ok) const;
template VariantMap Variant::value(bool *ok) const;

bool Variant::toBool(bool *ok) const
{
  return value<bool>(ok);
}

int Variant::toInt(bool *ok) const
{
  return value<int>(ok);
}

unsigned int Variant::toUInt(bool *ok) const
{
  return value<unsigned int>(ok);
}

long long Variant::toLongLong(bool *ok) const
{
  return value<long long>(ok);
}

unsigned long long Variant::toULongLong(bool *ok) const
{
  return value<unsigned long long>(ok);
}

double Variant::toDouble(bool *ok) const
{
  return value<double>(ok);
}

TagLib::String Variant::toString(bool *ok) const
{
  return value<TagLib::String>(ok);
}

TagLib::StringList Variant::toStringList(bool *ok) const
{
  return value<TagLib::StringList>(ok);
}

TagLib::ByteVector Variant::toByteVector(bool *ok) const
{
  return value<TagLib::ByteVector>(ok);
}

TagLib::ByteVectorList Variant::toByteVectorList(bool *ok) const
{
  return value<TagLib::ByteVectorList>(ok);
}

TagLib::List<TagLib::Variant> Variant::toList(bool *ok) const
{
  return value<TagLib::List<TagLib::Variant>>(ok);
}

TagLib::Map<TagLib::String, TagLib::Variant> Variant::toMap(bool *ok) const
{
  return value<TagLib::Map<TagLib::String, TagLib::Variant>>(ok);
}

bool Variant::operator==(const Variant &v) const
{
  return d == v.d || d->data == v.d->data;
}

bool Variant::operator!=(const Variant &v) const
{
  return !(*this == v);
}

Variant &Variant::operator=(const Variant &) = default;

////////////////////////////////////////////////////////////////////////////////
// related non-member functions
////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &s, const TagLib::Variant &v)
{
  printVariantToStream(s, v.d->data);
  return s;
}
