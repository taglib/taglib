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

#include "tstring.h"
#include "tdebug.h"
#include "tsmartptr.h"
#include "tutils.h"

#include "tbytevector.h"

// This is a bit ugly to keep writing over and over again.

// A rather obscure feature of the C++ spec that I hadn't thought of that makes
// working with C libs much more efficient.  There's more here:
//
// http://www.informit.com/isapi/product_id~{9C84DAB4-FE6E-49C5-BB0A-FB50331233EA}/content/index.asp

#define DATA(x) (&(*(x->data))[0])

namespace TagLib {

static const char hexTable[17] = "0123456789abcdef";

static const uint crcTable[256] = {
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
  0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
  0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
  0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
  0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
  0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
  0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
  0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
  0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
  0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
  0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
  0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
  0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
  0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
  0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
  0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
  0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
  0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
  0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
  0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
  0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
  0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

/*!
  * A templatized straightforward find that works with the types 
  * std::vector<char>::iterator and std::vector<char>::reverse_iterator.
  */
template <class TIterator>
size_t findChar(
  const TIterator dataBegin, const TIterator dataEnd,
  char c, size_t offset, size_t byteAlign)
{
  const size_t dataSize = dataEnd - dataBegin;
  if(dataSize == 0 || offset > dataSize - 1)
    return ByteVector::npos;

  // n % 0 is invalid

  if(byteAlign == 0)
    return ByteVector::npos;

  for(TIterator it = dataBegin + offset; it < dataEnd; it += byteAlign) {
    if(*it == c)
      return (it - dataBegin);
  }

  return ByteVector::npos;
}

/*!
  * A templatized KMP find that works with the types 
  * std::vector<char>::iterator and std::vector<char>::reverse_iterator.
  */
template <class TIterator>
size_t findVector(
  const TIterator dataBegin, const TIterator dataEnd,
  const TIterator patternBegin, const TIterator patternEnd,
  size_t offset, size_t byteAlign)
{
  const size_t dataSize    = dataEnd    - dataBegin;
  const size_t patternSize = patternEnd - patternBegin;
  if(patternSize > dataSize || offset > dataSize - 1)
    return ByteVector::npos;

  // n % 0 is invalid

  if(byteAlign == 0)
    return ByteVector::npos;

  // Special case that the pattern contains just single char.

  if(patternSize == 1)
    return findChar(dataBegin, dataEnd, *patternBegin, offset, byteAlign);

  size_t lastOccurrence[256];

  for(size_t i = 0; i < 256; ++i)
    lastOccurrence[i] = patternSize;

  for(size_t i = 0; i < patternSize - 1; ++i)
    lastOccurrence[static_cast<uchar>(*(patternBegin + i))] = patternSize - i - 1;

  TIterator it = dataBegin + patternSize - 1 + offset;
  while(true)
  {
    TIterator itBuffer  = it;
    TIterator itPattern = patternBegin + patternSize - 1;

    while(*itBuffer == *itPattern)
    {
      if(itPattern == patternBegin)
      {
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

  return ByteVector::npos;
}

template <typename T, size_t LENGTH, ByteOrder ENDIAN>
inline T toNumber(const ByteVector &v, size_t offset)
{
  static const bool swap = (ENDIAN != Utils::SystemByteOrder);

  if(LENGTH >= sizeof(T) && offset + LENGTH <= v.size()) 
  {
    // Uses memcpy instead of reinterpret_cast to avoid an alignment exception.
    T tmp;
    ::memcpy(&tmp, v.data() + offset, sizeof(T));

    if(swap)
      return Utils::byteSwap(tmp);
    else
      return tmp;
  }
  else if(offset < v.size())
  {
    const size_t length = std::min(LENGTH, v.size() - offset);
    T sum = 0;
    for(size_t i = 0; i < length; i++) {
      const size_t shift = (swap ? length - 1 - i : i) * 8;
      sum |= static_cast<T>(static_cast<uchar>(v[offset + i])) << shift;
    }

    return sum;
  }
  else
  {
    debug("toNumber<T>() - offset is out of range. Returning 0.");
    return 0;
  }
}

template <typename T, ByteOrder ENDIAN>
inline ByteVector fromNumber(T value)
{
  static const bool swap = (ENDIAN != Utils::SystemByteOrder);

  if(swap)
    value = Utils::byteSwap(value);

  return ByteVector(reinterpret_cast<const char *>(&value), sizeof(T));
}

class ByteVector::ByteVectorPrivate 
{
public:
  ByteVectorPrivate() 
    : data(new std::vector<char>())
    , offset(0)
    , length(0) 
  {
  }

  ByteVectorPrivate(ByteVectorPrivate* d, size_t o, size_t l)
    : data(d->data)
    , offset(d->offset + o)
    , length(l)
  {
  }

  ByteVectorPrivate(size_t l, char c) 
    : data(new std::vector<char>(l, c))
    , offset(0)
    , length(l)
  {
  }

  ByteVectorPrivate(const char *s, size_t l) 
    : data(new std::vector<char>(s, s + l))
    , offset(0)
    , length(l)
  {
  }
  
  void detach()
  {
    if(!data.unique()) {
      data.reset(new std::vector<char>(data->begin() + offset, data->begin() + offset + length));
      offset = 0;
    }
  }

  ~ByteVectorPrivate()
  {
  }

  SHARED_PTR<std::vector<char> > data;
  size_t offset;
  size_t length;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

const ByteVector ByteVector::null;

const size_t ByteVector::npos = static_cast<size_t>(-1);

ByteVector ByteVector::fromCString(const char *s, size_t length)
{
  if(length == npos)
    return ByteVector(s);
  else
    return ByteVector(s, length);
}

ByteVector ByteVector::fromUInt16LE(size_t value)
{
  return fromNumber<ushort, LittleEndian>(static_cast<ushort>(value));
}

ByteVector ByteVector::fromUInt16BE(size_t value)
{
  return fromNumber<ushort, BigEndian>(static_cast<ushort>(value));
}

ByteVector ByteVector::fromUInt32LE(size_t value)
{
  return fromNumber<uint, LittleEndian>(static_cast<uint>(value));
}

ByteVector ByteVector::fromUInt32BE(size_t value)
{
  return fromNumber<uint, BigEndian>(static_cast<uint>(value));
}

ByteVector ByteVector::fromUInt64LE(ulonglong value)
{
  return fromNumber<ulonglong, LittleEndian>(value);
}

ByteVector ByteVector::fromUInt64BE(ulonglong value)
{
  return fromNumber<ulonglong, BigEndian>(value);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ByteVector::ByteVector()
  : d(new ByteVectorPrivate())
{
}

ByteVector::ByteVector(size_t size, char value)
  : d(new ByteVectorPrivate(size, value))
{
}

ByteVector::ByteVector(const ByteVector &v) 
  : d(new ByteVectorPrivate(*v.d))
{
}

ByteVector::ByteVector(const ByteVector &v, size_t offset, size_t length)
  : d(new ByteVectorPrivate(v.d, offset, length))
{
}

ByteVector::ByteVector(char c)
  : d(new ByteVectorPrivate(1, c))
{
}

ByteVector::ByteVector(const char *data, size_t length)
  : d(new ByteVectorPrivate(data, length))
{
}

ByteVector::ByteVector(const char *data)
  : d(new ByteVectorPrivate(data, ::strlen(data)))
{
}

ByteVector::~ByteVector()
{
  delete d;
}

ByteVector &ByteVector::setData(const char *data, size_t length)
{
  *this = ByteVector(data, length);
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
  return !isEmpty() ? (DATA(d) + d->offset) : 0;
}

const char *ByteVector::data() const
{
  return !isEmpty() ? (DATA(d) + d->offset) : 0;
}

ByteVector ByteVector::mid(size_t index, size_t length) const
{
  index  = std::min(index, size());
  length = std::min(length, size() - index);

  return ByteVector(*this, index, length);
}

char ByteVector::at(size_t index) const
{
  return index < size() ? DATA(d)[d->offset + index] : 0;
}

size_t ByteVector::find(const ByteVector &pattern, size_t offset, size_t byteAlign) const
{
  return findVector<ConstIterator>(
    begin(), end(), pattern.begin(), pattern.end(), offset, byteAlign);
}

size_t ByteVector::find(char c, size_t offset, size_t byteAlign) const
{
  return findChar<ConstIterator>(begin(), end(), c, offset, byteAlign);
}

size_t ByteVector::rfind(const ByteVector &pattern, size_t offset, size_t byteAlign) const
{
  if(offset > 0) {
    offset = size() - offset - pattern.size();
    if(offset >= size())
      offset = 0;
  }

  const size_t pos = findVector<ConstReverseIterator>(
    rbegin(), rend(), pattern.rbegin(), pattern.rend(), offset, byteAlign);

  if(pos == npos)
    return npos;
  else
    return size() - pos - pattern.size();
}

bool ByteVector::containsAt(
  const ByteVector &pattern, size_t offset, size_t patternOffset, size_t patternLength) const
{
  if(pattern.size() < patternLength)
    patternLength = pattern.size();

  // do some sanity checking -- all of these things are needed for the search to be valid
  const size_t compareLength = patternLength - patternOffset;
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

  const size_t withSize = with.size();
  const size_t patternSize = pattern.size();
  size_t offset = 0;

  if(withSize == patternSize) {
    // I think this case might be common enough to optimize it
    detach();
    offset = find(pattern);
    while(offset != npos) {
      ::memcpy(DATA(d) + offset, DATA(with.d), withSize);
      offset = find(pattern, offset + withSize);
    }
    return *this;
  }

  // calculate new size:
  size_t newSize = 0;
  for(;;) {
    const size_t next = find(pattern, offset);
    if(next == npos) {
      if(offset == 0)
        // pattern not found, do nothing:
        return *this;
      newSize += size() - offset;
      break;
    }
    newSize += (next - offset) + withSize;
    offset = next + patternSize;
  }

  // new private data of appropriate size:
  ByteVectorPrivate newData(newSize, '\0');
  char *target = &(*newData.data)[0];
  const char *source = DATA(d);

  // copy modified data into new private data:
  offset = 0;
  for(;;) {
    const size_t next = find(pattern, offset);
    if(next == npos) {
      ::memcpy(target, source + offset, size() - offset);
      break;
    }
    const size_t chunkSize = next - offset;
    ::memcpy(target, source + offset, chunkSize);
    target += chunkSize;
    ::memcpy(target, DATA(with.d), withSize);
    target += withSize;
    offset += chunkSize + patternSize;
  }

  // replace private data:
  *d = newData;

  return *this;
}

size_t ByteVector::endsWithPartialMatch(const ByteVector &pattern) const
{
  if(pattern.size() > size())
    return npos;

  const size_t startIndex = size() - pattern.size();

  // try to match the last n-1 bytes from the vector (where n is the pattern
  // size) -- continue trying to match n-2, n-3...1 bytes

  for(size_t i = 1; i < pattern.size(); i++) {
    if(containsAt(pattern, startIndex + i, 0, pattern.size() - i))
      return startIndex + i;
  }

  return npos;
}

ByteVector &ByteVector::append(const ByteVector &v)
{
  if(!v.isEmpty())
  {
    detach();

    const size_t originalSize = size();
    resize(originalSize + v.size());
    ::memcpy(data() + originalSize, v.data(), v.size());
  }

  return *this;
}

ByteVector &ByteVector::append(char c)
{
  resize(size() + 1, c);
  return *this;
}

ByteVector &ByteVector::clear()
{
  detach();
  *d = *ByteVector::null.d; 

  return *this;
}

size_t ByteVector::size() const
{
  return d->length;
}

ByteVector &ByteVector::resize(size_t size, char padding)
{
  if(size != d->length) {
    detach();
    d->data->resize(d->offset + size, padding);
    d->length = size;
  }

  return *this;
}

ByteVector::Iterator ByteVector::begin()
{
  return d->data->begin() + d->offset;
}

ByteVector::ConstIterator ByteVector::begin() const
{
  return d->data->begin() + d->offset;
}

ByteVector::Iterator ByteVector::end()
{
  return d->data->begin() + d->offset + d->length;
}

ByteVector::ConstIterator ByteVector::end() const
{
  return d->data->begin() + d->offset + d->length;
}

ByteVector::ReverseIterator ByteVector::rbegin()
{
  std::vector<char> &v = *(d->data);
  return v.rbegin() + (v.size() - (d->offset + d->length));
}

ByteVector::ConstReverseIterator ByteVector::rbegin() const
{
  std::vector<char> &v = *(d->data);
  return v.rbegin() + (v.size() - (d->offset + d->length));
}

ByteVector::ReverseIterator ByteVector::rend()
{
  std::vector<char> &v = *(d->data);
  return v.rbegin() + (v.size() - d->offset);
}

ByteVector::ConstReverseIterator ByteVector::rend() const
{
  std::vector<char> &v = *(d->data);
  return v.rbegin() + (v.size() - d->offset);
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
  uint sum = 0;
  for(ByteVector::ConstIterator it = begin(); it != end(); ++it)
    sum = (sum << 8) ^ crcTable[((sum >> 24) & 0xff) ^ uchar(*it)];
  return sum;
}

short ByteVector::toInt16LE(size_t offset) const
{
  return static_cast<short>(toNumber<ushort, 2, LittleEndian>(*this, offset));
}

short ByteVector::toInt16BE(size_t offset) const
{
  return static_cast<short>(toNumber<ushort, 2, BigEndian>(*this, offset));
}

ushort ByteVector::toUInt16LE(size_t offset) const
{
  return toNumber<ushort, 2, LittleEndian>(*this, offset);
}

ushort ByteVector::toUInt16BE(size_t offset) const
{
  return toNumber<ushort, 2, BigEndian>(*this, offset);
}

uint ByteVector::toUInt24LE(size_t offset) const
{
  return toNumber<uint, 3, LittleEndian>(*this, offset);
}

uint ByteVector::toUInt24BE(size_t offset) const
{
  return toNumber<uint, 3, BigEndian>(*this, offset);
}

uint ByteVector::toUInt32LE(size_t offset) const
{
  return toNumber<uint, 4, LittleEndian>(*this, offset);
}

uint ByteVector::toUInt32BE(size_t offset) const
{
  return toNumber<uint, 4, BigEndian>(*this, offset);
}

long long ByteVector::toInt64LE(size_t offset) const
{
  return static_cast<long long>(toNumber<ulonglong, 8, LittleEndian>(*this, offset));
}

long long ByteVector::toInt64BE(size_t offset) const
{
  return static_cast<long long>(toNumber<ulonglong, 8, BigEndian>(*this, offset));
}    

float ByteVector::toFloat32BE(size_t offset) const
{
  if(offset > size() - 4) {
    debug("ByteVector::toFloat32BE() - offset is out of range. Returning 0.");
    return 0.0;
  }

#if defined(SIZEOF_FLOAT) && SIZEOF_FLOAT == 4

  if(std::numeric_limits<float>::is_iec559) 
  {
    // float is 32-bit wide and IEEE754 compliant.

    union {
      uint  i;
      float f;
    } tmp;
    ::memcpy(&tmp, data() + offset, 4);

    if(Utils::SystemByteOrder == LittleEndian)
      tmp.i = Utils::byteSwap(tmp.i);

    return tmp.f;
  }

#endif

  const uchar *bytes = reinterpret_cast<const uchar*>(data() + offset);

  // 1-bit sign 
  const bool negative = ((bytes[0] & 0x80) != 0);

  // 8-bit exponent
  const int exponent = ((bytes[0] & 0x7F) << 1) | (bytes[1] >> 7);

  // 1-bit integer part (always 1) and 23-bit fraction.
  const uint fraction 
    = (1U << 23)
    | (static_cast<uint>(bytes[1] & 0x7f) << 16) 
    | (static_cast<uint>(bytes[2]) <<  8) 
    | (static_cast<uint>(bytes[3]));

  float val;
  if (exponent == 0 && fraction == 0)
    val = 0;
  else {
    if(exponent == 0xFF) {
      debug("ByteVector::toFloat32BE() - can't handle the infinity or NaN. Returning 0.");
      return 0.0;
    }
    else 
      val = ::ldexp(static_cast<float>(fraction), exponent - 127 - 23);
  }

  if(negative)
    return -val;
  else
    return val;
}

double ByteVector::toFloat64BE(size_t offset) const
{
  if(offset > size() - 8) {
    debug("ByteVector::toFloat64BE() - offset is out of range. Returning 0.");
    return 0.0;
  }

#if defined(SIZEOF_DOUBLE) && SIZEOF_DOUBLE == 8

  if(std::numeric_limits<double>::is_iec559) 
  {
    // double is 64-bit wide and IEEE754 compliant.

    union {
      ulonglong i;
      double    f;
    } tmp;
    ::memcpy(&tmp, data() + offset, 8);

    if(Utils::SystemByteOrder == LittleEndian)
      tmp.i = Utils::byteSwap(tmp.i);

    return tmp.f;
  }

#endif

  const uchar *bytes = reinterpret_cast<const uchar*>(data() + offset);

  // 1-bit sign 
  const bool negative = ((bytes[0] & 0x80) != 0);

  // 11-bit exponent
  const int exponent = ((bytes[0] & 0x7F) << 4) | (bytes[1] >> 4);

  // 1-bit integer part (always 1) and 52-bit fraction.
  const ulonglong fraction 
    = (1ULL << 52)
    | (static_cast<ulonglong>(bytes[1] & 0x0F) << 48) 
    | (static_cast<ulonglong>(bytes[2]) << 40) 
    | (static_cast<ulonglong>(bytes[3]) << 32) 
    | (static_cast<ulonglong>(bytes[4]) << 24)
    | (static_cast<ulonglong>(bytes[5]) << 16) 
    | (static_cast<ulonglong>(bytes[6]) <<  8) 
    | (static_cast<ulonglong>(bytes[7]));

  double val;
  if (exponent == 0 && fraction == 0)
    val = 0;
  else {
    if(exponent == 0x7FF) {
      debug("ByteVector::toFloat64BE() - can't handle the infinity or NaN. Returning 0.");
      return 0.0;
    }
    else 
      val = ::ldexp(1.0 + static_cast<double>(fraction), exponent - 1023 - 52);
  }

  if(negative)
    return -val;
  else
    return val;
}

long double ByteVector::toFloat80BE(size_t offset) const
{
  if(offset > size() - 10) {
    debug("ByteVector::toFloat80BE() - offset is out of range. Returning 0.");
    return 0.0;
  }

  const uchar *bytes = reinterpret_cast<const uchar*>(data() + offset);

  // 1-bit sign 
  const bool negative = ((bytes[0] & 0x80) != 0);

  // 15-bit exponent
  const int exponent = ((bytes[0] & 0x7F) << 8) | bytes[1];

  // 1-bit integer part and 63-bit fraction.
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
  if (exponent == 0 && fraction == 0)
    val = 0;
  else {
    if(exponent == 0x7FFF) {
      debug("ByteVector::toFloat80BE() - can't handle the infinity or NaN. Returning 0.");
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

const char &ByteVector::operator[](size_t index) const
{
  return DATA(d)[d->offset + index];
}

char &ByteVector::operator[](size_t index)
{
  detach();
  return DATA(d)[d->offset + index];
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
  *d = *v.d;

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
  d->detach();
}

////////////////////////////////////////////////////////////////////////////////
// related functions
////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &s, const ByteVector &v)
{
  for(size_t i = 0; i < v.size(); i++)
    s << v[i];
  return s;
}
}
