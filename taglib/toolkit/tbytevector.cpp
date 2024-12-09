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

#include "tbytevector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>

#include "tdebug.h"
#include "tutils.h"

// This is a bit ugly to keep writing over and over again.

// A rather obscure feature of the C++ spec that I hadn't thought of that makes
// working with C libs much more efficient.  There's more here:
//
// http://www.informit.com/isapi/product_id~{9C84DAB4-FE6E-49C5-BB0A-FB50331233EA}/content/index.asp

namespace TagLib {

template <class TIterator>
int findChar(
  const TIterator dataBegin, const TIterator dataEnd,
  char c, unsigned int offset, int byteAlign)
{
  if(const size_t dataSize = dataEnd - dataBegin; offset + 1 > dataSize)
    return -1;

  // n % 0 is invalid

  if(byteAlign == 0)
    return -1;

  for(TIterator it = dataBegin + offset; it < dataEnd; it += byteAlign) {
    if(*it == c)
      return static_cast<int>(it - dataBegin);
  }

  return -1;
}

template <class TIterator>
int findVector(
  const TIterator dataBegin, const TIterator dataEnd,
  const TIterator patternBegin, const TIterator patternEnd,
  unsigned int offset, int byteAlign)
{
  const size_t dataSize    = dataEnd    - dataBegin;
  const size_t patternSize = patternEnd - patternBegin;
  if(patternSize == 0 || offset + patternSize > dataSize)
    return -1;

  // Special case that pattern contains just single char.

  if(patternSize == 1)
    return findChar(dataBegin, dataEnd, *patternBegin, offset, byteAlign);

  // n % 0 is invalid

  if(byteAlign == 0)
    return -1;

  // We don't use sophisticated algorithms like Knuth-Morris-Pratt here.

  // In the current implementation of TagLib, data and patterns are too small
  // for such algorithms to work effectively.

  for(TIterator it = dataBegin + offset; it < dataEnd - patternSize + 1; it += byteAlign) {

    TIterator itData    = it;
    TIterator itPattern = patternBegin;

    while(*itData == *itPattern) {
      ++itData;
      ++itPattern;

      if(itPattern == patternEnd)
        return static_cast<int>(it - dataBegin);
    }
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
    sum |= static_cast<T>(static_cast<unsigned char>(v[static_cast<int>(offset + i)])) << shift;
  }

  return sum;
}

template <class T>
T toNumber(const ByteVector &v, size_t offset, bool mostSignificantByteFirst)
{
  const bool isBigEndian = Utils::systemByteOrder() == Utils::BigEndian;
  const bool swap = mostSignificantByteFirst != isBigEndian;

  if(offset + sizeof(T) > v.size())
    return toNumber<T>(v, offset, v.size() - offset, mostSignificantByteFirst);

  // Uses memcpy instead of reinterpret_cast to avoid an alignment exception.
  T tmp;
  ::memcpy(&tmp, v.data() + offset, sizeof(T));

  if(swap)
    return Utils::byteSwap(tmp);
  return tmp;
}

template <class T>
ByteVector fromNumber(T value, bool mostSignificantByteFirst)
{
  const bool isBigEndian = Utils::systemByteOrder() == Utils::BigEndian;

  if(mostSignificantByteFirst != isBigEndian)
    value = Utils::byteSwap(value);

  return ByteVector(reinterpret_cast<const char *>(&value), sizeof(T));
}

template <typename TFloat, typename TInt, Utils::ByteOrder ENDIAN>
TFloat toFloat(const ByteVector &v, size_t offset)
{
  if(offset + sizeof(TInt) > v.size()) {
    debug("toFloat() - offset is out of range. Returning 0.");
    return 0.0;
  }

  union {
    TInt   i;
    TFloat f;
  } tmp;
  ::memcpy(&tmp, v.data() + offset, sizeof(TInt));

  if(ENDIAN != Utils::systemByteOrder())
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

  if(ENDIAN != Utils::systemByteOrder())
    tmp.i = Utils::byteSwap(tmp.i);

  return ByteVector(reinterpret_cast<char *>(&tmp), sizeof(TInt));
}

template <Utils::ByteOrder ENDIAN>
long double toFloat80(const ByteVector &v, size_t offset)
{
  using std::swap;

  if(offset + 10 > v.size()) {
    debug("toFloat80() - offset is out of range. Returning 0.");
    return 0.0;
  }

  unsigned char bytes[10];
  ::memcpy(bytes, v.data() + offset, 10);

  if constexpr(ENDIAN == Utils::LittleEndian) {
    swap(bytes[0], bytes[9]);
    swap(bytes[1], bytes[8]);
    swap(bytes[2], bytes[7]);
    swap(bytes[3], bytes[6]);
    swap(bytes[4], bytes[5]);
  }

  // 1-bit sign
  const bool negative = ((bytes[0] & 0x80) != 0);

  // 15-bit exponent
  const int exponent = ((bytes[0] & 0x7F) << 8) | bytes[1];

  // 64-bit fraction. Leading 1 is explicit.
  const unsigned long long fraction
    = (static_cast<unsigned long long>(bytes[2]) << 56)
    | (static_cast<unsigned long long>(bytes[3]) << 48)
    | (static_cast<unsigned long long>(bytes[4]) << 40)
    | (static_cast<unsigned long long>(bytes[5]) << 32)
    | (static_cast<unsigned long long>(bytes[6]) << 24)
    | (static_cast<unsigned long long>(bytes[7]) << 16)
    | (static_cast<unsigned long long>(bytes[8]) <<  8)
    | (static_cast<unsigned long long>(bytes[9]));

  long double val;
  if(exponent == 0 && fraction == 0)
    val = 0;
  else {
    if(exponent == 0x7FFF) {
      debug("toFloat80() - can't handle the infinity or NaN. Returning 0.");
      return 0.0;
    }
    val = ::ldexp(static_cast<long double>(fraction), exponent - 16383 - 63);
  }

  if(negative)
    return -val;
  return val;
}

class ByteVector::ByteVectorPrivate
{
public:
  ByteVectorPrivate(unsigned int l, char c) :
    data(std::make_shared<std::vector<char>>(l, c)),
    offset(0),
    length(l) { }

  ByteVectorPrivate(const char *s, unsigned int l) :
    data(std::make_shared<std::vector<char>>(s, s + l)),
    offset(0),
    length(l) { }

  ByteVectorPrivate(const ByteVectorPrivate &d, unsigned int o, unsigned int l) :
    data(d.data),
    offset(d.offset + o),
    length(l)
  {
  }

  std::shared_ptr<std::vector<char>> data;
  unsigned int       offset;
  unsigned int       length;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

ByteVector ByteVector::fromCString(const char *s, unsigned int length)
{
  if(length == 0xffffffff)
    return ByteVector(s, static_cast<unsigned int>(::strlen(s)));
  return ByteVector(s, length);
}

ByteVector ByteVector::fromUInt(unsigned int value, bool mostSignificantByteFirst)
{
  return fromNumber<uint32_t>(value, mostSignificantByteFirst);
}

ByteVector ByteVector::fromShort(short value, bool mostSignificantByteFirst)
{
  return fromNumber<uint16_t>(value, mostSignificantByteFirst);
}

ByteVector ByteVector::fromUShort(unsigned short value, bool mostSignificantByteFirst)
{
  return fromNumber<uint16_t>(value, mostSignificantByteFirst);
}

ByteVector ByteVector::fromLongLong(long long value, bool mostSignificantByteFirst)
{
  return fromNumber<uint64_t>(value, mostSignificantByteFirst);
}

ByteVector ByteVector::fromULongLong(unsigned long long value, bool mostSignificantByteFirst)
{
  return fromNumber<uint64_t>(value, mostSignificantByteFirst);
}

ByteVector ByteVector::fromFloat32LE(float value)
{
  return fromFloat<float, uint32_t, Utils::LittleEndian>(value);
}

ByteVector ByteVector::fromFloat32BE(float value)
{
  return fromFloat<float, uint32_t, Utils::BigEndian>(value);
}

ByteVector ByteVector::fromFloat64LE(double value)
{
  return fromFloat<double, uint64_t, Utils::LittleEndian>(value);
}

ByteVector ByteVector::fromFloat64BE(double value)
{
  return fromFloat<double, uint64_t, Utils::BigEndian>(value);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ByteVector::ByteVector() :
  d(std::make_unique<ByteVectorPrivate>(0, '\0'))
{
}

ByteVector::ByteVector(unsigned int size, char value) :
  d(std::make_unique<ByteVectorPrivate>(size, value))
{
}

ByteVector::ByteVector(const ByteVector &v) :
  d(std::make_unique<ByteVectorPrivate>(*v.d, 0, v.d->length))
{
}

ByteVector::ByteVector(const ByteVector &v, unsigned int offset, unsigned int length) :
  d(std::make_unique<ByteVectorPrivate>(*v.d, offset, length))
{
}

ByteVector::ByteVector(char c) :
  d(std::make_unique<ByteVectorPrivate>(1, c))
{
}

ByteVector::ByteVector(const char *data, unsigned int length) :
  d(std::make_unique<ByteVectorPrivate>(data, length))
{
}

ByteVector::ByteVector(const char *data) :
  d(std::make_unique<ByteVectorPrivate>(data, data ? static_cast<unsigned int>(::strlen(data)) : 0))
{
}

ByteVector::~ByteVector() = default;

ByteVector &ByteVector::setData(const char *s, unsigned int length)
{
  ByteVector(s, length).swap(*this);
  return *this;
}

ByteVector &ByteVector::setData(const char *data)
{
  ByteVector(data).swap(*this);
  return *this;
}

char *ByteVector::data()
{
  detach();
  return !isEmpty() ? &(*d->data)[d->offset] : nullptr;
}

const char *ByteVector::data() const
{
  return !isEmpty() ? &(*d->data)[d->offset] : nullptr;
}

ByteVector ByteVector::mid(unsigned int index, unsigned int length) const
{
  index  = std::min(index, size());
  length = std::min(length, size() - index);

  return ByteVector(*this, index, length);
}

char ByteVector::at(unsigned int index) const
{
  return index < size() ? (*d->data)[d->offset + index] : 0;
}

int ByteVector::find(const ByteVector &pattern, unsigned int offset, int byteAlign) const
{
  return findVector<ConstIterator>(
    begin(), end(), pattern.begin(), pattern.end(), offset, byteAlign);
}

int ByteVector::find(char c, unsigned int offset, int byteAlign) const
{
  return findChar<ConstIterator>(begin(), end(), c, offset, byteAlign);
}

int ByteVector::rfind(const ByteVector &pattern, unsigned int offset, int byteAlign) const
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
  return size() - pos - pattern.size();
}

bool ByteVector::containsAt(const ByteVector &pattern, unsigned int offset, unsigned int patternOffset, unsigned int patternLength) const
{
  if(pattern.size() < patternLength)
    patternLength = pattern.size();

  // do some sanity checking -- all of these things are needed for the search to be valid
  const unsigned int compareLength = patternLength - patternOffset;
  if(offset + compareLength > size() || patternOffset >= pattern.size() || patternLength == 0)
    return false;

  return ::memcmp(data() + offset, pattern.data() + patternOffset, compareLength) == 0;
}

bool ByteVector::startsWith(const ByteVector &pattern) const
{
  return containsAt(pattern, 0);
}

bool ByteVector::endsWith(const ByteVector &pattern) const
{
  return containsAt(pattern, size() - pattern.size());
}

ByteVector &ByteVector::replace(char oldByte, char newByte)
{
  detach();

  std::replace(this->begin(), this->end(), oldByte, newByte);

  return *this;
}

ByteVector &ByteVector::replace(const ByteVector &pattern, const ByteVector &with)
{
  if(pattern.size() == 1 && with.size() == 1)
    return replace(pattern[0], with[0]);

  // Check if there is at least one occurrence of the pattern.

  int offset = find(pattern, 0);
  if(offset == -1)
    return *this;

  if(pattern.size() == with.size()) {

    // We think this case might be common enough to optimize it.

    detach();
    do
    {
      ::memcpy(data() + offset, with.data(), with.size());
      offset = find(pattern, offset + pattern.size());
    } while(offset != -1);
  }
  else {

    // Loop once to calculate the result size.

    unsigned int dstSize = size();
    do
    {
      dstSize += with.size() - pattern.size();
      offset = find(pattern, offset + pattern.size());
    } while(offset != -1);

    // Loop again to copy modified data to the new vector.

    ByteVector dst(dstSize);
    int dstOffset = 0;

    offset = 0;
    while(true) {
      const int next = find(pattern, offset);
      if(next == -1) {
        ::memcpy(dst.data() + dstOffset, data() + offset, size() - offset);
        break;
      }

      ::memcpy(dst.data() + dstOffset, data() + offset, next - offset);
      dstOffset += next - offset;

      ::memcpy(dst.data() + dstOffset, with.data(), with.size());
      dstOffset += with.size();

      offset = next + pattern.size();
    }

    swap(dst);
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

  for(unsigned int i = 1; i < pattern.size(); i++) {
    if(containsAt(pattern, startIndex + i, 0, pattern.size() - i))
      return startIndex + i;
  }

  return -1;
}

ByteVector &ByteVector::append(const ByteVector &v)
{
  if(v.isEmpty())
    return *this;

  detach();

  const unsigned int originalSize = size();
  const unsigned int appendSize = v.size();

  resize(originalSize + appendSize);
  ::memcpy(data() + originalSize, v.data(), appendSize);

  return *this;
}

ByteVector &ByteVector::append(char c)
{
  resize(size() + 1, c);
  return *this;
}

ByteVector &ByteVector::clear()
{
  ByteVector().swap(*this);
  return *this;
}

unsigned int ByteVector::size() const
{
  return d->length;
}

ByteVector &ByteVector::resize(unsigned int size, char padding)
{
  if(size != d->length) {
    detach();

    // Remove the excessive length of the internal buffer first to pad correctly.
    // This doesn't reallocate the buffer, since std::vector::resize() doesn't
    // reallocate the buffer when shrinking.

    d->data->resize(d->offset + d->length);
    d->data->resize(d->offset + size, padding);

    d->length = size;
  }

  return *this;
}

ByteVector::Iterator ByteVector::begin()
{
  detach();
  return d->data->begin() + d->offset;
}

ByteVector::ConstIterator ByteVector::begin() const
{
  return d->data->begin() + d->offset;
}

ByteVector::ConstIterator ByteVector::cbegin() const
{
  return d->data->cbegin() + d->offset;
}

ByteVector::Iterator ByteVector::end()
{
  detach();
  return d->data->begin() + d->offset + d->length;
}

ByteVector::ConstIterator ByteVector::end() const
{
  return d->data->begin() + d->offset + d->length;
}

ByteVector::ConstIterator ByteVector::cend() const
{
  return d->data->cbegin() + d->offset + d->length;
}

ByteVector::ReverseIterator ByteVector::rbegin()
{
  detach();
  return d->data->rbegin() + (d->data->size() - (d->offset + d->length));
}

ByteVector::ConstReverseIterator ByteVector::rbegin() const
{
  // Workaround for the Solaris Studio 12.4 compiler.
  // We need a const reference to the data vector so we can ensure the const version of rbegin() is called.
  const std::vector<char> &v = *d->data;
  return v.rbegin() + (v.size() - (d->offset + d->length));
}

ByteVector::ReverseIterator ByteVector::rend()
{
  detach();
  return d->data->rbegin() + (d->data->size() - d->offset);
}

ByteVector::ConstReverseIterator ByteVector::rend() const
{
  // Workaround for the Solaris Studio 12.4 compiler.
  // We need a const reference to the data vector so we can ensure the const version of rbegin() is called.
  const std::vector<char> &v = *d->data;
  return v.rbegin() + (v.size() - d->offset);
}

bool ByteVector::isEmpty() const
{
  return d->length == 0;
}

// Sanity checks
static_assert(sizeof(unsigned short) == sizeof(uint16_t), "unsigned short and uint16_t are different sizes");
static_assert(sizeof(unsigned int) == sizeof(uint32_t), "unsigned int and uint32_t are different sizes");
static_assert(sizeof(unsigned long long) == sizeof(uint64_t), "unsigned long long and uint64_t are different sizes");
static_assert(sizeof(float) == sizeof(uint32_t), "float and uint32_t are different sizes");
static_assert(sizeof(double) == sizeof(uint64_t), "double and uint64_t are different sizes");

unsigned int ByteVector::toUInt(bool mostSignificantByteFirst) const
{
  return toNumber<uint32_t>(*this, 0, mostSignificantByteFirst);
}

unsigned int ByteVector::toUInt(unsigned int offset, bool mostSignificantByteFirst) const
{
  return toNumber<uint32_t>(*this, offset, mostSignificantByteFirst);
}

unsigned int ByteVector::toUInt(unsigned int offset, unsigned int length, bool mostSignificantByteFirst) const
{
  return toNumber<uint32_t>(*this, offset, length, mostSignificantByteFirst);
}

short ByteVector::toShort(bool mostSignificantByteFirst) const
{
  return toNumber<uint16_t>(*this, 0, mostSignificantByteFirst);
}

short ByteVector::toShort(unsigned int offset, bool mostSignificantByteFirst) const
{
  return toNumber<uint16_t>(*this, offset, mostSignificantByteFirst);
}

unsigned short ByteVector::toUShort(bool mostSignificantByteFirst) const
{
  return toNumber<uint16_t>(*this, 0, mostSignificantByteFirst);
}

unsigned short ByteVector::toUShort(unsigned int offset, bool mostSignificantByteFirst) const
{
  return toNumber<uint16_t>(*this, offset, mostSignificantByteFirst);
}

long long ByteVector::toLongLong(bool mostSignificantByteFirst) const
{
  return toNumber<uint64_t>(*this, 0, mostSignificantByteFirst);
}

long long ByteVector::toLongLong(unsigned int offset, bool mostSignificantByteFirst) const
{
  return toNumber<uint64_t>(*this, offset, mostSignificantByteFirst);
}

unsigned long long ByteVector::toULongLong(bool mostSignificantByteFirst) const
{
  return toNumber<uint64_t>(*this, 0, mostSignificantByteFirst);
}

unsigned long long ByteVector::toULongLong(unsigned int offset, bool mostSignificantByteFirst) const
{
  return toNumber<uint64_t>(*this, offset, mostSignificantByteFirst);
}

float ByteVector::toFloat32LE(size_t offset) const
{
  return toFloat<float, uint32_t, Utils::LittleEndian>(*this, offset);
}

float ByteVector::toFloat32BE(size_t offset) const
{
  return toFloat<float, uint32_t, Utils::BigEndian>(*this, offset);
}

double ByteVector::toFloat64LE(size_t offset) const
{
  return toFloat<double, uint64_t, Utils::LittleEndian>(*this, offset);
}

double ByteVector::toFloat64BE(size_t offset) const
{
  return toFloat<double, uint64_t, Utils::BigEndian>(*this, offset);
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
  return (*d->data)[d->offset + index];
}

char &ByteVector::operator[](int index)
{
  detach();
  return (*d->data)[d->offset + index];
}

bool ByteVector::operator==(const ByteVector &v) const
{
  if(size() != v.size())
    return false;

  return ::memcmp(data(), v.data(), size()) == 0;
}

bool ByteVector::operator!=(const ByteVector &v) const
{
  return !(*this == v);
}

bool ByteVector::operator==(const char *s) const
{
  if(!s)
    return isEmpty();

  if(size() != ::strlen(s))
    return false;

  return ::memcmp(data(), s, size()) == 0;
}

bool ByteVector::operator!=(const char *s) const
{
  return !(*this == s);
}

bool ByteVector::operator<(const ByteVector &v) const
{
  if(const int result = ::memcmp(data(), v.data(), std::min(size(), v.size()));
     result != 0)
    return result < 0;
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
  ByteVector(v).swap(*this);
  return *this;
}

ByteVector &ByteVector::operator=(char c)
{
  ByteVector(c).swap(*this);
  return *this;
}

ByteVector &ByteVector::operator=(const char *data)
{
  ByteVector(data).swap(*this);
  return *this;
}

void ByteVector::swap(ByteVector &v) noexcept
{
  using std::swap;

  swap(d, v.d);
}

ByteVector ByteVector::toHex() const
{
  static constexpr char hexTable[17] = "0123456789abcdef";

  ByteVector encoded(size() * 2);
  char *p = encoded.data();

  for(unsigned int i = 0; i < size(); i++) {
    unsigned char c = data()[i];
    *p++ = hexTable[(c >> 4) & 0x0F];
    *p++ = hexTable[ c       & 0x0F];
  }

  return encoded;
}

ByteVector ByteVector::fromBase64(const ByteVector & input)
{
  static constexpr std::array<unsigned char, 256> base64 {
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3e, 0x80, 0x80, 0x80, 0x3f,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
  };

  unsigned int len = input.size();

  ByteVector output(len);

  auto src = reinterpret_cast<const unsigned char*>(input.data());
  auto dst = reinterpret_cast<unsigned char*>(output.data());

  while(4 <= len) {

    // Check invalid character
    if(base64[src[0]] == 0x80)
      break;

    // Check invalid character
    if(base64[src[1]] == 0x80)
      break;

    // Decode first byte
    *dst++ = ((base64[src[0]] << 2) & 0xfc) | ((base64[src[1]] >> 4) & 0x03);

    if(src[2] != '=') {

      // Check invalid character
      if(base64[src[2]] == 0x80)
        break;

      // Decode second byte
      *dst++ = ((base64[src[1]] & 0x0f) << 4) | ((base64[src[2]] >> 2) & 0x0f);

      if(src[3] != '=') {

        // Check invalid character
        if(base64[src[3]] == 0x80)
          break;

        // Decode third byte
        *dst++ = ((base64[src[2]] & 0x03) << 6) | (base64[src[3]] & 0x3f);
      }
      else {
        // assume end of data
        len -= 4;
        break;
      }
    }
    else {
      // assume end of data
      len -= 4;
      break;
    }
    src += 4;
    len -= 4;
  }

  // Only return output if we processed all bytes
  if(len == 0) {
    output.resize(static_cast<unsigned int>(dst - reinterpret_cast<unsigned char*>(output.data())));
    return output;
  }
  return ByteVector();
}

ByteVector ByteVector::toBase64() const
{
  static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  if(!isEmpty()) {
    unsigned int len = size();
    ByteVector output(4 * ((len - 1) / 3 + 1)); // note roundup

    const char * src = data();
    char * dst = output.data();
    while(3 <= len) {
      *dst++ = alphabet[(src[0] >> 2) & 0x3f];
      *dst++ = alphabet[((src[0] & 0x03) << 4) | ((src[1] >> 4) & 0x0f)];
      *dst++ = alphabet[((src[1] & 0x0f) << 2) | ((src[2] >> 6) & 0x03)];
      *dst++ = alphabet[src[2] & 0x3f];
      src += 3;
      len -= 3;
    }
    if(len) {
      *dst++ = alphabet[(src[0] >> 2) & 0x3f];
      if(len>1) {
        *dst++ = alphabet[((src[0] & 0x03) << 4) | ((src[1] >> 4) & 0x0f)];
        *dst++ = alphabet[((src[1] & 0x0f) << 2)];
      }
      else {
        *dst++ = alphabet[(src[0] & 0x03) << 4];
        *dst++ = '=';
      }
    *dst++ = '=';
    }
    return output;
  }
  return ByteVector();
}


////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void ByteVector::detach()
{
  if(d->data.use_count() > 1) {
    if(!isEmpty())
      ByteVector(&d->data->front() + d->offset, d->length).swap(*this);
    else
      ByteVector().swap(*this);
  }
}
}  // namespace TagLib

////////////////////////////////////////////////////////////////////////////////
// related functions
////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &s, const TagLib::ByteVector &v)
{
  for(const auto &byte : v) {
    s << byte;
  }
  return s;
}
