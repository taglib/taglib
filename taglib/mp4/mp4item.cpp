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

#include "config.h"

#include <cstdio>
#include <cstdarg>

#include "taglib.h"
#include "tdebug.h"
#include "tsmartptr.h"
#include "mp4item.h"

using namespace TagLib;

namespace
{
  String format(const char *fmt, ...)
  {
    va_list args;
    va_start(args, fmt);

    char buf[256];

#if defined(HAVE_SNPRINTF)

    vsnprintf(buf, sizeof(buf), fmt, args);

#elif defined(HAVE_SPRINTF_S)

    vsprintf_s(buf, fmt, args);

#else

    // Be careful. May cause a buffer overflow.  
    vsprintf(buf, fmt, args);

#endif

    va_end(args);

    return String(buf);
  }

  struct ItemData
  {
    bool valid;
    MP4::AtomDataType atomDataType;
    MP4::Item::ItemType type;
    union {
      bool m_bool;
      int m_int;
      MP4::Item::IntPair m_intPair;
      uchar m_byte;
      uint m_uint;
      long long m_longlong;
    };
    StringList m_stringList;
    ByteVectorList m_byteVectorList;
    MP4::CoverArtList m_coverArtList;
  };
}

class MP4::Item::ItemPrivate 
{
public:
  ItemPrivate() 
    : data(new ItemData())
  {
    data->valid        = true;
    data->atomDataType = MP4::TypeUndefined;
    data->type         = MP4::Item::TypeUndefined;
  }

  SHARED_PTR<ItemData> data;
};

MP4::Item::Item()
  : d(new ItemPrivate())
{
  d->data->valid = false;
}

MP4::Item::Item(const Item &item) 
  : d(new ItemPrivate(*item.d))
{
}

MP4::Item &
  MP4::Item::operator=(const Item &item)
{
  *d = *item.d;
  return *this;
}


MP4::Item::~Item()
{
  delete d;
}

MP4::Item::Item(bool value)
  : d(new ItemPrivate())
{
  d->data->m_bool = value;
  d->data->type = TypeBool;
}

MP4::Item::Item(int value)
  : d(new ItemPrivate())
{
  d->data->m_int = value;
  d->data->type = TypeInt;
}

MP4::Item::Item(uchar value)
  : d(new ItemPrivate())
{
  d->data->m_byte = value;
  d->data->type = TypeByte;
}

MP4::Item::Item(uint value)
  : d(new ItemPrivate())
{
  d->data->m_uint = value;
  d->data->type = TypeUInt;
}

MP4::Item::Item(long long value)
  : d(new ItemPrivate())
{
  d->data->m_longlong = value;
  d->data->type = TypeLongLong;
}

MP4::Item::Item(int value1, int value2)
  : d(new ItemPrivate())
{
  d->data->m_intPair.first = value1;
  d->data->m_intPair.second = value2;
  d->data->type = TypeIntPair;
}

MP4::Item::Item(const ByteVectorList &value)
  : d(new ItemPrivate())
{
  d->data->m_byteVectorList = value;
  d->data->type = TypeByteVectorList;
}

MP4::Item::Item(const StringList &value)
  : d(new ItemPrivate())
{
  d->data->m_stringList = value;
  d->data->type = TypeStringList;
}

MP4::Item::Item(const MP4::CoverArtList &value)
  : d(new ItemPrivate())
{
  d->data->m_coverArtList = value;
  d->data->type = TypeCoverArtList;
}

void MP4::Item::setAtomDataType(MP4::AtomDataType type)
{
  d->data->atomDataType = type;
}

MP4::AtomDataType MP4::Item::atomDataType() const
{
  return d->data->atomDataType;
}

bool
MP4::Item::toBool() const
{
  return d->data->m_bool;
}

int
MP4::Item::toInt() const
{
  return d->data->m_int;
}

uchar
MP4::Item::toByte() const
{
  return d->data->m_byte;
}

TagLib::uint
MP4::Item::toUInt() const
{
  return d->data->m_uint;
}

long long
MP4::Item::toLongLong() const
{
  return d->data->m_longlong;
}

MP4::Item::IntPair
MP4::Item::toIntPair() const
{
  return d->data->m_intPair;
}

StringList
MP4::Item::toStringList() const
{
  return d->data->m_stringList;
}

ByteVectorList
MP4::Item::toByteVectorList() const
{
  return d->data->m_byteVectorList;
}

MP4::CoverArtList
MP4::Item::toCoverArtList() const
{
  return d->data->m_coverArtList;
}

bool
MP4::Item::isValid() const
{
  return d->data->valid;
}

String
MP4::Item::toString() const
{
  StringList desc;
  switch (d->data->type) {
    case TypeBool:
      return d->data->m_bool ? "true" : "false";
    case TypeInt:
      return format("%d", d->data->m_int);
    case TypeIntPair:
      return format("%d/%d", d->data->m_intPair.first, d->data->m_intPair.second);
    case TypeByte:
      return format("%d", d->data->m_byte);
    case TypeUInt:
      return format("%u", d->data->m_uint);
    case TypeLongLong:
      return format("%lld", d->data->m_longlong);
    case TypeStringList:
      return d->data->m_stringList.toString(" / ");
    case TypeByteVectorList:
      for(TagLib::uint i = 0; i < d->data->m_byteVectorList.size(); i++) {
        desc.append(format(
          "[%d bytes of data]", static_cast<int>(d->data->m_byteVectorList[i].size())));
      }
      return desc.toString(", ");
    case TypeCoverArtList:
      for(TagLib::uint i = 0; i < d->data->m_coverArtList.size(); i++) {
        desc.append(format(
          "[%d bytes of data]", static_cast<int>(d->data->m_coverArtList[i].data().size())));
      }
      return desc.toString(", ");
    case TypeUndefined:
      return "[unknown]";
  }
  return String();
}

