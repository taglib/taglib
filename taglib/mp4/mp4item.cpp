/**************************************************************************
    copyright            : (C) 2007 by Lukáš Lalinský
    email                : lalinsky@gmail.com
 **************************************************************************/

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

#include "mp4item.h"

using namespace TagLib;

class MP4::Item::ItemPrivate
{
public:
  Type type;
  bool valid { true };
  AtomDataType atomDataType { TypeUndefined };
  union {
    bool m_bool;
    int m_int;
    IntPair m_intPair;
    unsigned char m_byte;
    unsigned int m_uint;
    long long m_longlong;
  };
  StringList m_stringList;
  ByteVectorList m_byteVectorList;
  MP4::CoverArtList m_coverArtList;
};

MP4::Item::Item() :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::Void;
  d->valid = false;
}

MP4::Item::Item(const Item &) = default;
MP4::Item &MP4::Item::operator=(const Item &) = default;

void
MP4::Item::swap(Item &item) noexcept
{
  using std::swap;

  swap(d, item.d);
}

MP4::Item::~Item() = default;

MP4::Item::Item(bool value) :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::Bool;
  d->m_bool = value;
}

MP4::Item::Item(int value) :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::Int;
  d->m_int = value;
}

MP4::Item::Item(unsigned char value) :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::Byte;
  d->m_byte = value;
}

MP4::Item::Item(unsigned int value) :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::UInt;
  d->m_uint = value;
}

MP4::Item::Item(long long value) :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::LongLong;
  d->m_longlong = value;
}

MP4::Item::Item(int value1, int value2) :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::IntPair;
  d->m_intPair.first = value1;
  d->m_intPair.second = value2;
}

MP4::Item::Item(const ByteVectorList &value) :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::ByteVectorList;
  d->m_byteVectorList = value;
}

MP4::Item::Item(const StringList &value) :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::StringList;
  d->m_stringList = value;
}

MP4::Item::Item(const MP4::CoverArtList &value) :
  d(std::make_shared<ItemPrivate>())
{
  d->type = Type::CoverArtList;
  d->m_coverArtList = value;
}

void MP4::Item::setAtomDataType(MP4::AtomDataType type)
{
  d->atomDataType = type;
}

MP4::AtomDataType MP4::Item::atomDataType() const
{
  return d->atomDataType;
}

bool
MP4::Item::toBool() const
{
  return d->m_bool;
}

int
MP4::Item::toInt() const
{
  return d->m_int;
}

unsigned char
MP4::Item::toByte() const
{
  return d->m_byte;
}

unsigned int
MP4::Item::toUInt() const
{
  return d->m_uint;
}

long long
MP4::Item::toLongLong() const
{
  return d->m_longlong;
}

MP4::Item::IntPair
MP4::Item::toIntPair() const
{
  return d->m_intPair;
}

StringList
MP4::Item::toStringList() const
{
  return d->m_stringList;
}

ByteVectorList
MP4::Item::toByteVectorList() const
{
  return d->m_byteVectorList;
}

MP4::CoverArtList
MP4::Item::toCoverArtList() const
{
  return d->m_coverArtList;
}

bool
MP4::Item::isValid() const
{
  return d->valid;
}

MP4::Item::Type MP4::Item::type() const
{
  return d->type;
}

bool MP4::Item::operator==(const Item &other) const
{
  if(isValid() && other.isValid() &&
     type() == other.type() &&
     atomDataType() == other.atomDataType()) {
    switch(type()) {
    case Type::Void:
      return true;
    case Type::Bool:
      return toBool() == other.toBool();
    case Type::Int:
      return toInt() == other.toInt();
    case Type::IntPair: {
      const auto lhs = toIntPair();
      const auto rhs = other.toIntPair();
      return lhs.first == rhs.first && lhs.second == rhs.second;
    }
    case Type::Byte:
      return toByte() == other.toByte();
    case Type::UInt:
      return toUInt() == other.toUInt();
    case Type::LongLong:
      return toLongLong() == other.toLongLong();
    case Type::StringList:
      return toStringList() == other.toStringList();
    case Type::ByteVectorList:
      return toByteVectorList() == other.toByteVectorList();
    case Type::CoverArtList:
      return toCoverArtList() == other.toCoverArtList();
    }
  }
  return false;
}

bool MP4::Item::operator!=(const Item &other) const
{
  return !(*this == other);
}
