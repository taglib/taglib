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

// This class assumes that std::basic_string<T> has a contiguous and null-terminated buffer.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "tstring.h"
#include "tdebug.h"
#include "tstringlist.h"
#include "tsmartptr.h"
#include "tutils.h"

#include <iostream>
#include <cstdio>
#include <cstring>

#ifdef HAVE_STD_CODECVT
# include <codecvt>
#else
# include "unicode.h"
#endif

namespace
{

  inline unsigned short combine(unsigned char c1, unsigned char c2)
  {
    return (c1 << 8) | c2;
  }

  void UTF16toUTF8(const wchar_t *src, size_t srcLength, char *dst, size_t dstLength)
  {
#ifdef HAVE_STD_CODECVT

    typedef std::codecvt_utf8_utf16<wchar_t> utf8_utf16_t;

    using namespace TagLib;

    const wchar_t *srcBegin = src;
    const wchar_t *srcEnd   = srcBegin + srcLength;

    char *dstBegin = dst;
    char *dstEnd   = dstBegin + dstLength;

    std::mbstate_t st;
    const wchar_t *source;
    char *target;
    memset(&st, 0, sizeof(st));
    std::codecvt_base::result result = utf8_utf16_t().out(
      st, srcBegin, srcEnd, source, dstBegin, dstEnd, target);

    if(result != utf8_utf16_t::ok) {
      debug("String::copyFromUTF8() - Unicode conversion error.");
    }

#else

    using namespace Unicode;
    using namespace TagLib;

    const Unicode::UTF16 *srcBegin = src;
    const Unicode::UTF16 *srcEnd   = srcBegin + srcLength;

    Unicode::UTF8 *dstBegin = reinterpret_cast<Unicode::UTF8*>(dst);
    Unicode::UTF8 *dstEnd   = dstBegin + dstLength;

    Unicode::ConversionResult result = Unicode::ConvertUTF16toUTF8(
      &srcBegin, srcEnd, &dstBegin, dstEnd, Unicode::lenientConversion);

    if(result != Unicode::conversionOK) {
      debug("String::to8Bit() - Unicode conversion error.");
    }

#endif
  }

  void UTF8toUTF16(const char *src, size_t srcLength, wchar_t *dst, size_t dstLength)
  {
#ifdef HAVE_STD_CODECVT

    typedef std::codecvt_utf8_utf16<wchar_t> utf8_utf16_t;

    using namespace TagLib;

    const char *srcBegin = src;
    const char *srcEnd   = srcBegin + srcLength;

    wchar_t *dstBegin = dst;
    wchar_t *dstEnd   = dstBegin + dstLength;

    std::mbstate_t st;
    const char *source;
    wchar_t *target;
    memset(&st, 0, sizeof(st));
    std::codecvt_base::result result = utf8_utf16_t().in(
      st, srcBegin, srcEnd, source, dstBegin, dstEnd, target);

    if(result != utf8_utf16_t::ok) {
      debug("String::copyFromUTF8() - Unicode conversion error.");
    }

#else

    using namespace Unicode;
    using namespace TagLib;

    const Unicode::UTF8 *srcBegin = reinterpret_cast<const Unicode::UTF8*>(src);
    const Unicode::UTF8 *srcEnd   = srcBegin + srcLength;

    Unicode::UTF16 *dstBegin = dst;
    Unicode::UTF16 *dstEnd   = dstBegin + dstLength;

    Unicode::ConversionResult result = Unicode::ConvertUTF8toUTF16(
      &srcBegin, srcEnd, &dstBegin, dstEnd, Unicode::lenientConversion);

    if(result != Unicode::conversionOK) {
      debug("String::copyFromUTF8() - Unicode conversion error.");
    }

#endif 
  }
}

namespace TagLib {

class String::StringPrivate 
{
public:
  StringPrivate()
    : data(new std::wstring())
  {
  }

  StringPrivate(const std::wstring &s) 
    : data(new std::wstring(s)) 
  {
  }

  StringPrivate(size_t n, wchar_t c) 
    : data(new std::wstring(n, c)) 
  {
  }

  /*!
   * Stores string in UTF-16. The byte order depends on the CPU endian. 
   */
  SHARED_PTR<std::wstring> data;

  /*!
   * This is only used to hold the the most recent value of toCString().
   */
  SHARED_PTR<std::string> cstring;
};

const String String::null;

// Actual value is -1.
const size_t String::npos = std::wstring::npos;

////////////////////////////////////////////////////////////////////////////////

String::String() 
  : d(new StringPrivate())
{
}

String::String(const String &s) 
  : d(new StringPrivate(*s.d))
{
}

String::String(const std::string &s, Type t)
  : d(new StringPrivate())
{
  if(t == Latin1)
    copyFromLatin1(&s[0], s.length());
  else if(t == String::UTF8)
    copyFromUTF8(&s[0], s.length());
  else {
    debug("String::String() -- A std::string should not contain UTF16.");
  }
}

String::String(const std::wstring &s, Type t)
  : d(new StringPrivate())
{
  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(s.c_str(), s.length(), t);
  else {
    debug("String::String() -- A TagLib::std::wstring should not contain Latin1 or UTF-8.");
  }
}

String::String(const wchar_t *s, Type t)
  : d(new StringPrivate())
{
  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(s, ::wcslen(s), t);
  else {
    debug("String::String() -- A const wchar_t * should not contain Latin1 or UTF-8.");
  }
}

String::String(const char *s, Type t)
  : d(new StringPrivate())
{
  if(t == Latin1)
    copyFromLatin1(s, ::strlen(s));
  else if(t == String::UTF8)
    copyFromUTF8(s, ::strlen(s));
  else {
    debug("String::String() -- A const char * should not contain UTF16.");
  }
}

String::String(wchar_t c, Type t)
  : d(new StringPrivate())
{
  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(&c, 1, t);
  else {
    debug("String::String() -- A const wchar_t should not contain Latin1 or UTF-8.");
  }
}

String::String(char c, Type t)
  : d(new StringPrivate(1, static_cast<uchar>(c)))
{
  if(t != Latin1 && t != UTF8) {
    debug("String::String() -- A char should not contain UTF16.");
  }
}

String::String(const ByteVector &v, Type t)
  : d(new StringPrivate())
{
  if(v.isEmpty())
    return;

  if(t == Latin1) 
    copyFromLatin1(v.data(), v.size());
  else if(t == UTF8) 
    copyFromUTF8(v.data(), v.size());
  else 
    copyFromUTF16(v.data(), v.size(), t);
}

////////////////////////////////////////////////////////////////////////////////

String::~String()
{
  delete d;
}

std::string String::to8Bit(bool unicode) const
{
  std::string s;

  if(!unicode) {
    s.resize(d->data->size());

    std::string::iterator targetIt = s.begin();
    for(std::wstring::const_iterator it = d->data->begin(); it != d->data->end(); it++) {
      *targetIt = static_cast<char>(*it);
      ++targetIt;
    }
  }
  else {
    s.resize(d->data->size() * 4 + 1);

    UTF16toUTF8(&(*d->data)[0], d->data->size(), &s[0], s.size());
    s.resize(::strlen(s.c_str()));
  }

  return s;
}

const std::wstring &String::toWString() const
{
  return *d->data;
}

const char *String::toCString(bool unicode) const
{
  d->cstring.reset(new std::string(to8Bit(unicode)));
  return d->cstring->c_str();
}

const wchar_t *String::toCWString() const
{
  return d->data->c_str();
}

String::Iterator String::begin()
{
  return d->data->begin();
}

String::ConstIterator String::begin() const
{
  return d->data->begin();
}

String::Iterator String::end()
{
  return d->data->end();
}

String::ConstIterator String::end() const
{
  return d->data->end();
}

size_t String::find(const String &s, size_t offset) const
{
  return d->data->find(*s.d->data, offset);
}

size_t String::rfind(const String &s, size_t offset) const
{
  return d->data->rfind(*s.d->data, offset);
}

StringList String::split(const String &separator) const
{
  StringList list;
  for(size_t index = 0;;)
  {
    const size_t sep = find(separator, index);
    if(sep == npos)
    {
      list.append(substr(index, size() - index));
      break;
    }
    else
    {
      list.append(substr(index, sep - index));
      index = sep + separator.size();
    }
  }
  return list;
}

bool String::startsWith(const String &s) const
{
  if(s.length() > length())
    return false;

  return substr(0, s.length()) == s;
}

String String::substr(size_t position, size_t length) const
{
  return String(d->data->substr(position, length));
}

String &String::append(const String &s)
{
  detach();
  *d->data += *s.d->data;
  return *this;
}

String String::upper() const
{
  static const int shift = 'A' - 'a';

  String s(*this);
  for(Iterator it = s.begin(); it != s.end(); ++it) {
    if(*it >= 'a' && *it <= 'z')
      *it = *it + shift;
  }

  return s;
}

size_t String::size() const
{
  return d->data->size();
}

size_t String::length() const
{
  return size();
}

bool String::isEmpty() const
{
  return d->data->empty();
}

bool String::isNull() const
{
  return (d == null.d);
}

ByteVector String::data(Type t) const
{
  switch(t) 
  {
  case Latin1:
  {
    ByteVector v(size(), 0);
    char *p = v.data();

    for(std::wstring::const_iterator it = d->data->begin(); it != d->data->end(); it++)
      *p++ = static_cast<char>(*it);

    return v;
  }
  case UTF8:
  {
    ByteVector v(size() * 4 + 1, 0);

    UTF16toUTF8(&(*d->data)[0], d->data->size(), v.data(), v.size());
    v.resize(::strlen(v.data()));

    return v;
  }
  case UTF16:
  {
    ByteVector v(2 + size() * 2, 0);
    char *p = v.data();

    // Assume that if we're doing UTF16 and not UTF16BE that we want little
    // endian encoding.  (Byte Order Mark)

    *p++ = '\xff';
    *p++ = '\xfe';

    for(std::wstring::const_iterator it = d->data->begin(); it != d->data->end(); it++) {
      *p++ = static_cast<char>(*it & 0xff);
      *p++ = static_cast<char>(*it >> 8);
    }

    return v;
  }
  case UTF16BE:
  {
    ByteVector v(size() * 2, 0);
    char *p = v.data();

    for(std::wstring::const_iterator it = d->data->begin(); it != d->data->end(); it++) {
      *p++ = static_cast<char>(*it >> 8);
      *p++ = static_cast<char>(*it & 0xff);
    }

    return v;
  }
  case UTF16LE:
  {
    ByteVector v(size() * 2, 0);
    char *p = v.data();

    for(std::wstring::const_iterator it = d->data->begin(); it != d->data->end(); it++) {
      *p++ = static_cast<char>(*it & 0xff);
      *p++ = static_cast<char>(*it >> 8);
    }

    return v;
  }
  default:
  {
    debug("String::data() - Invalid Type value.");
    return ByteVector();
  }
  }
}

int String::toInt(bool *ok) const
{
  int value = 0;

  const size_t size = d->data->size();
  const bool negative = size > 0 && (*d->data)[0] == '-';
  const size_t start = negative ? 1 : 0;

  size_t i = start;
  for(; i < size && (*d->data)[i] >= '0' && (*d->data)[i] <= '9'; i++)
    value = value * 10 + ((*d->data)[i] - '0');

  if(negative)
    value = value * -1;

  if(ok)
    *ok = (size > start && i == size);

  return value;
}

String String::stripWhiteSpace() const
{
  std::wstring::const_iterator begin = d->data->begin();
  std::wstring::const_iterator end = d->data->end();

  while(begin != end &&
        (*begin == '\t' || *begin == '\n' || *begin == '\f' ||
         *begin == '\r' || *begin == ' '))
  {
    ++begin;
  }

  if(begin == end)
    return null;

  // There must be at least one non-whitespace character here for us to have
  // gotten this far, so we should be safe not doing bounds checking.

  do {
    --end;
  } while(*end == '\t' || *end == '\n' ||
          *end == '\f' || *end == '\r' || *end == ' ');

  return String(std::wstring(begin, end + 1));
}

bool String::isLatin1() const
{
  for(std::wstring::const_iterator it = d->data->begin(); it != d->data->end(); it++) {
    if(*it >= 256)
      return false;
  }
  return true;
}

bool String::isAscii() const
{
  for(std::wstring::const_iterator it = d->data->begin(); it != d->data->end(); it++) {
    if(*it >= 128)
      return false;
  }
  return true;
}

String String::number(int n) // static
{
  static const size_t BufferSize = 11; // Sufficient to store "-214748364".
  static const char *Format = "%d";

  char buffer[BufferSize];
  int length;

#if defined(HAVE_SNPRINTF)

  length = snprintf(buffer, BufferSize, Format, n);

#elif defined(HAVE_SPRINTF_S)

  length = sprintf_s(buffer, Format, n);

#else

  length = sprintf(buffer, Format, n);

#endif

  if(length > 0)
    return String(buffer);
  else
    return String::null;
}

TagLib::wchar &String::operator[](size_t i)
{
  detach();
  return (*d->data)[i];
}

const TagLib::wchar &String::operator[](size_t i) const
{
  return (*d->data)[i];
}

bool String::operator==(const String &s) const
{
  return (d->data == s.d->data || *d->data == *s.d->data);
}

bool String::operator==(const char *s) const
{
  for(std::wstring::const_iterator it = d->data->begin(); it != d->data->end(); it++) {
    if(*it != static_cast<uchar>(*s))
      return false;

    s++;
  }

  return true;
}

bool String::operator==(const wchar_t *s) const
{
  return (*d->data == s);
}

bool String::operator!=(const String &s) const
{
  return !operator==(s);
}

String &String::operator+=(const String &s)
{
  detach();

  *d->data += *s.d->data;
  return *this;
}

String &String::operator+=(const wchar_t *s)
{
  detach();

  *d->data += s;
  return *this;
}

String &String::operator+=(const char *s)
{
  detach();

  for(int i = 0; s[i] != 0; i++)
    *d->data += uchar(s[i]);
  return *this;
}

String &String::operator+=(wchar_t c)
{
  detach();

  *d->data += c;
  return *this;
}

String &String::operator+=(char c)
{
  detach();

  *d->data += uchar(c);
  return *this;
}

String &String::operator=(const String &s)
{
  *d = *s.d;
  return *this;
}

String &String::operator=(const std::string &s)
{
  d->data.reset(new std::wstring());
  copyFromLatin1(s.c_str(), s.length());

  return *this;
}

String &String::operator=(const std::wstring &s)
{
  d->data.reset(new std::wstring(s));
  return *this;
}

String &String::operator=(const wchar_t *s)
{
  d->data.reset(new std::wstring());
  copyFromUTF16(s, ::wcslen(s), WCharByteOrder);

  return *this;
}

String &String::operator=(char c)
{
  d->data.reset(new std::wstring(1, c));
  return *this;
}

String &String::operator=(wchar_t c)
{
  d->data.reset(new std::wstring(1, c));
  return *this;
}

String &String::operator=(const char *s)
{
  d->data.reset(new std::wstring());
  copyFromLatin1(s, ::strlen(s));

  return *this;
}

String &String::operator=(const ByteVector &v)
{
  d->data.reset(new std::wstring());
  copyFromLatin1(v.data(), v.size());

  // If we hit a null in the ByteVector, shrink the string again.
  d->data->resize(::wcslen(d->data->c_str()));

  return *this;
}

bool String::operator<(const String &s) const
{
  return *d->data < *s.d->data;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void String::detach()
{
  if(!d->data.unique())
    d->data.reset(new std::wstring(*d->data));
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void String::copyFromLatin1(const char *s, size_t length)
{
  d->data->resize(length);

  for(size_t i = 0; i < length; ++i)
    (*d->data)[i] = static_cast<uchar>(s[i]);
}

void String::copyFromUTF8(const char *s, size_t length)
{
  d->data->resize(length);

  UTF8toUTF16(s, length, &(*d->data)[0], d->data->size());
  d->data->resize(::wcslen(d->data->c_str()));
}

void String::copyFromUTF16(const wchar_t *s, size_t length, Type t)
{
  bool swap;
  if(t == UTF16) {
    if(length >= 1 && s[0] == 0xfeff) 
      swap = false; // Same as CPU endian. No need to swap bytes.
    else if(length >= 1 && s[0] == 0xfffe) 
      swap = true;  // Not same as CPU endian. Need to swap bytes.
    else {
      debug("String::copyFromUTF16() - Invalid UTF16 string.");
      return;
    }

    s++;
    length--;
  }
  else 
    swap = (t != WCharByteOrder);

  d->data->resize(length);
  memcpy(&(*d->data)[0], s, length * sizeof(wchar_t));

  if(swap) {
    for(size_t i = 0; i < length; ++i)
      (*d->data)[i] = Utils::byteSwap(static_cast<ushort>(s[i]));
  }
}

void String::copyFromUTF16(const char *s, size_t length, Type t)
{
  bool swap;
  if(t == UTF16) {
    if(length < 2) {
      debug("String::copyFromUTF16() - Invalid UTF16 string.");
      return;
    }

    // Uses memcpy instead of reinterpret_cast to avoid an alignment exception.
    ushort bom;
    ::memcpy(&bom, s, 2);

    if(bom == 0xfeff) 
      swap = false; // Same as CPU endian. No need to swap bytes.
    else if(bom == 0xfffe) 
      swap = true;  // Not same as CPU endian. Need to swap bytes.
    else {
      debug("String::copyFromUTF16() - Invalid UTF16 string.");
      return;
    }

    s += 2;
    length -= 2;
  }
  else 
    swap = (t != WCharByteOrder);

  d->data->resize(length / 2);
  for(size_t i = 0; i < length / 2; ++i) {
    (*d->data)[i] = swap ? combine(*s, *(s + 1)) : combine(*(s + 1), *s);
    s += 2;
  }
}

const String::Type String::WCharByteOrder 
  = (Utils::SystemByteOrder == BigEndian) ? String::UTF16BE : String::UTF16LE;

////////////////////////////////////////////////////////////////////////////////
// related functions
////////////////////////////////////////////////////////////////////////////////

const String operator+(const String &s1, const String &s2)
{
  TagLib::String s(s1);
  s.append(s2);
  return s;
}

const String operator+(const char *s1, const String &s2)
{
  TagLib::String s(s1);
  s.append(s2);
  return s;
}

const String operator+(const String &s1, const char *s2)
{
  TagLib::String s(s1);
  s.append(s2);
  return s;
}

std::ostream &operator<<(std::ostream &s, const TagLib::String &str)
{
  s << str.to8Bit();
  return s;
}
}
