/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <algorithm>
#include <iostream>
#include <limits>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstddef>

#include <tstring.h>
#include <tdebug.h>
#include "trefcounter.h"
#include "tutils.h"

#include "tbytevector.h"

// This is a bit ugly to keep writing over and over again.

// A rather obscure feature of the C++ spec that I hadn't thought of that makes
// working with C libs much more efficient.  There's more here:
//
// http://www.informit.com/isapi/product_id~{9C84DAB4-FE6E-49C5-BB0A-FB50331233EA}/content/index.asp

#define DATA(x) (&(x->data->data[0]))

namespace TagLib {

/*!
  * A templatized straightforward find that works with the types
  * std::vector<char>::iterator and std::vector<char>::reverse_iterator.
  */
template <class TIterator>
int findChar(
  const TIterator dataBegin, const TIterator dataEnd,
  char c, uint offset, int byteAlign)
{
  const size_t dataSize = dataEnd - dataBegin;
  if(offset + 1 > dataSize)
    return -1;

  // n % 0 is invalid

  if(byteAlign == 0)
    return -1;

  for(TIterator it = dataBegin + offset; it < dataEnd; it += byteAlign) {
    if(*it == c)
      return (it - dataBegin);
  }

  return -1;
}

/*!
  * A templatized KMP find that works with the types
  * std::vector<char>::iterator and std::vector<char>::reverse_iterator.
  */
template <class TIterator>
int findVector(
  const TIterator dataBegin, const TIterator dataEnd,
  const TIterator patternBegin, const TIterator patternEnd,
  uint offset, int byteAlign)
{
  const size_t dataSize    = dataEnd    - dataBegin;
  const size_t patternSize = patternEnd - patternBegin;
  if(patternSize == 0 || offset + patternSize > dataSize)
    return -1;

  // n % 0 is invalid

  if(byteAlign == 0)
    return -1;

  // Special case that pattern contains just single char.

  if(patternSize == 1)
    return findChar(dataBegin, dataEnd, *patternBegin, offset, byteAlign);

  size_t lastOccurrence[256];

  for(size_t i = 0; i < 256; ++i)
    lastOccurrence[i] = patternSize;

  for(size_t i = 0; i < patternSize - 1; ++i)
    lastOccurrence[static_cast<uchar>(*(patternBegin + i))] = patternSize - i - 1;

  TIterator it = dataBegin + patternSize - 1 + offset;
  while(true) {
    TIterator itBuffer = it;
    TIterator itPattern = patternBegin + patternSize - 1;

    while(*itBuffer == *itPattern) {
      if(itPattern == patternBegin) {
        if((itBuffer - dataBegin - offset) % byteAlign == 0)
          return (itBuffer - dataBegin);
        else
          break;
      }

      --itBuffer;
      --itPattern;
    }

    const size_t step = lastOccurrence[static_cast<uchar>(*it)];
    if(dataEnd - step <= it)
      break;

    it += step;
  }

  return -1;
}

template <class T>
T toNumber(const ByteVector &v, size_t offset, size_t length, bool mostSignificantByteFirst)
{
  if(offset >= v.size()) {
    debug("toNumber<T>() -- No data to convert. Returning 0.");
    return 0;
  }

  length = std::min(length, v.size() - offset);

  T sum = 0;
  for(size_t i = 0; i < length; i++) {
    const size_t shift = (mostSignificantByteFirst ? length - 1 - i : i) * 8;
    sum |= static_cast<T>(static_cast<uchar>(v[offset + i])) << shift;
  }

  return sum;
}

template <class T>
T toNumber(const ByteVector &v, size_t offset, bool mostSignificantByteFirst)
{
  static const bool isBigEndian = (Utils::SystemByteOrder == Utils::BigEndian);
  const bool swap = (mostSignificantByteFirst != isBigEndian);

  if(offset + sizeof(T) > v.size())
    return toNumber<T>(v, offset, v.size() - offset, mostSignificantByteFirst);

  // Uses memcpy instead of reinterpret_cast to avoid an alignment exception.
  T tmp;
  ::memcpy(&tmp, v.data() + offset, sizeof(T));

  if(swap)
    return Utils::byteSwap(tmp);
  else
    return tmp;
}

template <class T>
ByteVector fromNumber(T value, bool mostSignificantByteFirst)
{
  static const bool isBigEndian = (Utils::SystemByteOrder == Utils::BigEndian);
  const bool swap = (mostSignificantByteFirst != isBigEndian);

  if(swap)
    value = Utils::byteSwap(value);

  return ByteVector(reinterpret_cast<const char *>(&value), sizeof(T));
}

template <typename TFloat, typename TInt, Utils::ByteOrder ENDIAN>
TFloat toFloat(const ByteVector &v, size_t offset)
{
  if(offset > v.size() - sizeof(TInt)) {
    debug("toFloat() - offset is out of range. Returning 0.");
    return 0.0;
  }

  union {
    TInt   i;
    TFloat f;
  } tmp;
  ::memcpy(&tmp, v.data() + offset, sizeof(TInt));

  if(ENDIAN != Utils::FloatByteOrder)
    tmp.i = Utils::byteSwap(tmp.i);

  return tmp.f;
}

template <typename TFloat, typename TInt, Utils::ByteOrder ENDIAN>
ByteVector fromFloat(TFloat value)
{
  union {
    TInt   i;
    TFloat f;
  } tmp;
  tmp.f = value;

  if(ENDIAN != Utils::FloatByteOrder)
    tmp.i = Utils::byteSwap(tmp.i);

  return ByteVector(reinterpret_cast<char *>(&tmp), sizeof(TInt));
}

template <Utils::ByteOrder ENDIAN>
long double toFloat80(const ByteVector &v, size_t offset)
{
  if(offset > v.size() - 10) {
    debug("toFloat80() - offset is out of range. Returning 0.");
    return 0.0;
  }

  uchar bytes[10];
  ::memcpy(bytes, v.data() + offset, 10);

  if(ENDIAN == Utils::LittleEndian) {
    std::swap(bytes[0], bytes[9]);
    std::swap(bytes[1], bytes[8]);
    std::swap(bytes[2], bytes[7]);
    std::swap(bytes[3], bytes[6]);
    std::swap(bytes[4], bytes[5]);
  }

  // 1-bit sign
  const bool negative = ((bytes[0] & 0x80) != 0);

  // 15-bit exponent
  const int exponent = ((bytes[0] & 0x7F) << 8) | bytes[1];

  // 64-bit fraction. Leading 1 is explicit.
  const ulonglong fraction
    = (static_cast<ulonglong>(bytes[2]) << 56)
    | (static_cast<ulonglong>(bytes[3]) << 48)
    | (static_cast<ulonglong>(bytes[4]) << 40)
    | (static_cast<ulonglong>(bytes[5]) << 32)
    | (static_cast<ulonglong>(bytes[6]) << 24)
    | (static_cast<ulonglong>(bytes[7]) << 16)
    | (static_cast<ulonglong>(bytes[8]) <<  8)
    | (static_cast<ulonglong>(bytes[9]));

  long double val;
  if(exponent == 0 && fraction == 0)
    val = 0;
  else {
    if(exponent == 0x7FFF) {
      debug("toFloat80() - can't handle the infinity or NaN. Returning 0.");
      return 0.0;
    }
    else
      val = ::ldexp(static_cast<long double>(fraction), exponent - 16383 - 63);
  }

  if(negative)
    return -val;
  else
    return val;
}

class DataPrivate : public RefCounter
{
public:
  DataPrivate()
  {
  }

  DataPrivate(const std::vector<char> &v, uint offset, uint length)
    : data(v.begin() + offset, v.begin() + offset + length)
  {
  }

  // A char* can be an iterator.
  DataPrivate(const char *begin, const char *end)
    : data(begin, end)
  {
  }

  DataPrivate(uint len, char c)
    : data(len, c)
  {
  }

  std::vector<char> data;
};

class ByteVector::ByteVectorPrivate : public RefCounter
{
public:
  ByteVectorPrivate()
    : RefCounter()
    , data(new DataPrivate())
    , offset(0)
    , length(0)
  {
  }

  ByteVectorPrivate(ByteVectorPrivate *d, uint o, uint l)
    : RefCounter()
    , data(d->data)
    , offset(d->offset + o)
    , length(l)
  {
    data->ref();
  }

  ByteVectorPrivate(const std::vector<char> &v, uint o, uint l)
    : RefCounter()
    , data(new DataPrivate(v, o, l))
    , offset(0)
    , length(l)
  {
  }

  ByteVectorPrivate(uint l, char c)
    : RefCounter()
    , data(new DataPrivate(l, c))
    , offset(0)
    , length(l)
  {
  }

  ByteVectorPrivate(const char *s, uint l)
    : RefCounter()
    , data(new DataPrivate(s, s + l))
    , offset(0)
    , length(l)
  {
  }

  void detach()
  {
    if(data->count() > 1) {
      data->deref();
      data = new DataPrivate(data->data, offset, length);
      offset = 0;
    }
  }

  ~ByteVectorPrivate()
  {
    if(data->deref())
      delete data;
  }

  ByteVectorPrivate &operator=(const ByteVectorPrivate &x)
  {
    if(&x != this)
    {
      if(data->deref())
        delete data;

      data = x.data;
      data->ref();
    }

    return *this;
  }

  DataPrivate *data;
  uint offset;
  uint length;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

ByteVector ByteVector::null;

ByteVector ByteVector::fromCString(const char *s, uint length)
{
  if(length == 0xffffffff)
    return ByteVector(s, ::strlen(s));
  else
    return ByteVector(s, length);
}

ByteVector ByteVector::fromUInt(uint value, bool mostSignificantByteFirst)
{
  return fromNumber<uint>(value, mostSignificantByteFirst);
}

ByteVector ByteVector::fromShort(short value, bool mostSignificantByteFirst)
{
  return fromNumber<ushort>(value, mostSignificantByteFirst);
}

ByteVector ByteVector::fromLongLong(long long value, bool mostSignificantByteFirst)
{
  return fromNumber<unsigned long long>(value, mostSignificantByteFirst);
}

ByteVector ByteVector::fromFloat32LE(float value)
{
  return fromFloat<float, uint, Utils::LittleEndian>(value);
}

ByteVector ByteVector::fromFloat32BE(float value)
{
  return fromFloat<float, uint, Utils::BigEndian>(value);
}

ByteVector ByteVector::fromFloat64LE(double value)
{
  return fromFloat<double, ulonglong, Utils::LittleEndian>(value);
}

ByteVector ByteVector::fromFloat64BE(double value)
{
  return fromFloat<double, ulonglong, Utils::BigEndian>(value);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ByteVector::ByteVector()
  : d(new ByteVectorPrivate())
{
}

ByteVector::ByteVector(uint size, char value)
  : d(new ByteVectorPrivate(size, value))
{
}

ByteVector::ByteVector(const ByteVector &v)
  : d(v.d)
{
  d->ref();
}

ByteVector::ByteVector(const ByteVector &v, uint offset, uint length)
  : d(new ByteVectorPrivate(v.d, offset, length))
{
}

ByteVector::ByteVector(char c)
  : d(new ByteVectorPrivate(1, c))
{
}

ByteVector::ByteVector(const char *data, uint length)
  : d(new ByteVectorPrivate(data, length))
{
}

ByteVector::ByteVector(const char *data)
  : d(new ByteVectorPrivate(data, ::strlen(data)))
{
}

ByteVector::~ByteVector()
{
  if(d->deref())
    delete d;
}

ByteVector &ByteVector::setData(const char *s, uint length)
{
  *this = ByteVector(s, length);
  return *this;
}

ByteVector &ByteVector::setData(const char *data)
{
  *this = ByteVector(data);
  return *this;
}

char *ByteVector::data()
{
  detach();
  return size() > 0 ? (DATA(d) + d->offset) : 0;
}

const char *ByteVector::data() const
{
  return size() > 0 ? (DATA(d) + d->offset) : 0;
}

ByteVector ByteVector::mid(uint index, uint length) const
{
  index  = std::min(index, size());
  length = std::min(length, size() - index);

  return ByteVector(*this, index, length);
}

char ByteVector::at(uint index) const
{
  return index < size() ? DATA(d)[d->offset + index] : 0;
}

int ByteVector::find(const ByteVector &pattern, uint offset, int byteAlign) const
{
  return findVector<ConstIterator>(
    begin(), end(), pattern.begin(), pattern.end(), offset, byteAlign);
}

int ByteVector::find(char c, uint offset, int byteAlign) const
{
  return findChar<ConstIterator>(begin(), end(), c, offset, byteAlign);
}

int ByteVector::rfind(const ByteVector &pattern, uint offset, int byteAlign) const
{
  if(offset > 0) {
    offset = size() - offset - pattern.size();
    if(offset >= size())
      offset = 0;
  }

  const int pos = findVector<ConstReverseIterator>(
    rbegin(), rend(), pattern.rbegin(), pattern.rend(), offset, byteAlign);

  if(pos == -1)
    return -1;
  else
    return size() - pos - pattern.size();
}

bool ByteVector::containsAt(const ByteVector &pattern, uint offset, uint patternOffset, uint patternLength) const
{
  if(pattern.size() < patternLength)
    patternLength = pattern.size();

  // do some sanity checking -- all of these things are needed for the search to be valid
  const uint compareLength = patternLength - patternOffset;
  if(offset + compareLength > size() || patternOffset >= pattern.size() || patternLength == 0)
    return false;

  return (::memcmp(data() + offset, pattern.data() + patternOffset, compareLength) == 0);
}

bool ByteVector::startsWith(const ByteVector &pattern) const
{
  return containsAt(pattern, 0);
}

bool ByteVector::endsWith(const ByteVector &pattern) const
{
  return containsAt(pattern, size() - pattern.size());
}

ByteVector &ByteVector::replace(const ByteVector &pattern, const ByteVector &with)
{
  if(pattern.size() == 0 || pattern.size() > size())
    return *this;

  const size_t withSize    = with.size();
  const size_t patternSize = pattern.size();
  const ptrdiff_t diff = withSize - patternSize;

  size_t offset = 0;
  while (true)
  {
    offset = find(pattern, offset);
    if(offset == static_cast<size_t>(-1)) // Use npos in taglib2.
      break;

    detach();

    if(diff < 0) {
      ::memmove(
        data() + offset + withSize,
        data() + offset + patternSize,
        size() - offset - patternSize);
      resize(size() + diff);
    }
    else if(diff > 0) {
      resize(size() + diff);
      ::memmove(
        data() + offset + withSize,
        data() + offset + patternSize,
        size() - diff - offset - patternSize);
    }

    ::memcpy(data() + offset, with.data(), with.size());

    offset += withSize;
    if(offset > size() - patternSize)
      break;
  }

  return *this;
}

int ByteVector::endsWithPartialMatch(const ByteVector &pattern) const
{
  if(pattern.size() > size())
    return -1;

  const int startIndex = size() - pattern.size();

  // try to match the last n-1 bytes from the vector (where n is the pattern
  // size) -- continue trying to match n-2, n-3...1 bytes

  for(uint i = 1; i < pattern.size(); i++) {
    if(containsAt(pattern, startIndex + i, 0, pattern.size() - i))
      return startIndex + i;
  }

  return -1;
}

ByteVector &ByteVector::append(const ByteVector &v)
{
  if(v.d->length != 0)
  {
    detach();

    uint originalSize = size();
    resize(originalSize + v.size());
    ::memcpy(data() + originalSize, v.data(), v.size());
  }

  return *this;
}

ByteVector &ByteVector::clear()
{
  *this = ByteVector();
  return *this;
}

TagLib::uint ByteVector::size() const
{
  return d->length;
}

ByteVector &ByteVector::resize(uint size, char padding)
{
  if(size != d->length) {
    detach();

    // Remove the excessive length of the internal buffer first to pad correctly.
    // This doesn't reallocate the buffer, since std::vector::resize() doesn't
    // reallocate the buffer when shrinking.

    d->data->data.resize(d->offset + d->length);
    d->data->data.resize(d->offset + size, padding);

    d->length = size;
  }

  return *this;
}

ByteVector::Iterator ByteVector::begin()
{
  detach();
  return d->data->data.begin() + d->offset;
}

ByteVector::ConstIterator ByteVector::begin() const
{
  return d->data->data.begin() + d->offset;
}

ByteVector::Iterator ByteVector::end()
{
  detach();
  return d->data->data.begin() + d->offset + d->length;
}

ByteVector::ConstIterator ByteVector::end() const
{
  return d->data->data.begin() + d->offset + d->length;
}

ByteVector::ReverseIterator ByteVector::rbegin()
{
  detach();
  return d->data->data.rbegin() + (d->data->data.size() - (d->offset + d->length));
}

ByteVector::ConstReverseIterator ByteVector::rbegin() const
{
  return d->data->data.rbegin() + (d->data->data.size() - (d->offset + d->length));
}

ByteVector::ReverseIterator ByteVector::rend()
{
  detach();
  return d->data->data.rbegin() + (d->data->data.size() - d->offset);
}

ByteVector::ConstReverseIterator ByteVector::rend() const
{
  return d->data->data.rbegin() + (d->data->data.size() - d->offset);
}

bool ByteVector::isNull() const
{
  return (d == null.d);
}

bool ByteVector::isEmpty() const
{
  return (d->length == 0);
}

TagLib::uint ByteVector::checksum() const
{
  return 0;
}

TagLib::uint ByteVector::toUInt(bool mostSignificantByteFirst) const
{
  return toNumber<uint>(*this, 0, mostSignificantByteFirst);
}

TagLib::uint ByteVector::toUInt(uint offset, bool mostSignificantByteFirst) const
{
  return toNumber<uint>(*this, offset, mostSignificantByteFirst);
}

TagLib::uint ByteVector::toUInt(uint offset, uint length, bool mostSignificantByteFirst) const
{
  return toNumber<uint>(*this, offset, length, mostSignificantByteFirst);
}

short ByteVector::toShort(bool mostSignificantByteFirst) const
{
  return toNumber<unsigned short>(*this, 0, mostSignificantByteFirst);
}

short ByteVector::toShort(uint offset, bool mostSignificantByteFirst) const
{
  return toNumber<unsigned short>(*this, offset, mostSignificantByteFirst);
}

unsigned short ByteVector::toUShort(bool mostSignificantByteFirst) const
{
  return toNumber<unsigned short>(*this, 0, mostSignificantByteFirst);
}

unsigned short ByteVector::toUShort(uint offset, bool mostSignificantByteFirst) const
{
  return toNumber<unsigned short>(*this, offset, mostSignificantByteFirst);
}

long long ByteVector::toLongLong(bool mostSignificantByteFirst) const
{
  return toNumber<unsigned long long>(*this, 0, mostSignificantByteFirst);
}

long long ByteVector::toLongLong(uint offset, bool mostSignificantByteFirst) const
{
  return toNumber<unsigned long long>(*this, offset, mostSignificantByteFirst);
}

float ByteVector::toFloat32LE(size_t offset) const
{
  return toFloat<float, uint, Utils::LittleEndian>(*this, offset);
}

float ByteVector::toFloat32BE(size_t offset) const
{
  return toFloat<float, uint, Utils::BigEndian>(*this, offset);
}

double ByteVector::toFloat64LE(size_t offset) const
{
  return toFloat<double, ulonglong, Utils::LittleEndian>(*this, offset);
}

double ByteVector::toFloat64BE(size_t offset) const
{
  return toFloat<double, ulonglong, Utils::BigEndian>(*this, offset);
}

long double ByteVector::toFloat80LE(size_t offset) const
{
  return toFloat80<Utils::LittleEndian>(*this, offset);
}

long double ByteVector::toFloat80BE(size_t offset) const
{
  return toFloat80<Utils::BigEndian>(*this, offset);
}

const char &ByteVector::operator[](int index) const
{
  return d->data->data[d->offset + index];
}

char &ByteVector::operator[](int index)
{
  detach();
  return d->data->data[d->offset + index];
}

bool ByteVector::operator==(const ByteVector &v) const
{
  if(size() != v.size())
    return false;

  return (::memcmp(data(), v.data(), size()) == 0);
}

bool ByteVector::operator!=(const ByteVector &v) const
{
  return !operator==(v);
}

bool ByteVector::operator==(const char *s) const
{
  if(size() != ::strlen(s))
    return false;

  return (::memcmp(data(), s, size()) == 0);
}

bool ByteVector::operator!=(const char *s) const
{
  return !operator==(s);
}

bool ByteVector::operator<(const ByteVector &v) const
{
  const int result = ::memcmp(data(), v.data(), std::min(size(), v.size()));
  if(result != 0)
    return result < 0;
  else
    return size() < v.size();
}

bool ByteVector::operator>(const ByteVector &v) const
{
  return v < *this;
}

ByteVector ByteVector::operator+(const ByteVector &v) const
{
  ByteVector sum(*this);
  sum.append(v);
  return sum;
}

ByteVector &ByteVector::operator=(const ByteVector &v)
{
  if(&v == this)
    return *this;

  if(d->deref())
    delete d;

  d = v.d;
  d->ref();
  return *this;
}

ByteVector &ByteVector::operator=(char c)
{
  *this = ByteVector(c);
  return *this;
}

ByteVector &ByteVector::operator=(const char *data)
{
  *this = ByteVector(data);
  return *this;
}

ByteVector ByteVector::toHex() const
{
  static const char hexTable[17] = "0123456789abcdef";

  ByteVector encoded(size() * 2);
  char *p = encoded.data();

  for(uint i = 0; i < size(); i++) {
    unsigned char c = data()[i];
    *p++ = hexTable[(c >> 4) & 0x0F];
    *p++ = hexTable[(c     ) & 0x0F];
  }

  return encoded;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void ByteVector::detach()
{
  if(d->data->count() > 1) {
    d->data->deref();
    d->data = new DataPrivate(d->data->data, d->offset, d->length);
    d->offset = 0;
  }

  if(d->count() > 1) {
    d->deref();
    d = new ByteVectorPrivate(d->data->data, d->offset, d->length);
  }
}
}

////////////////////////////////////////////////////////////////////////////////
// related functions
////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &s, const TagLib::ByteVector &v)
{
  for(TagLib::uint i = 0; i < v.size(); i++)
    s << v[i];
  return s;
}
