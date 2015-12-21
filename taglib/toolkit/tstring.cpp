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
# include <config.h>
#endif

#include <tstring.h>
#include <tdebug.h>
#include <tstringlist.h>
#include <tsmartptr.h>
#include <tutils.h>

#include <iostream>
#include <cstring>
#include <cwchar>
#include <cerrno>
#include <climits>

#ifdef _WIN32
# include <windows.h>
#else
# include "unicode.h"
#endif

namespace
{
  using namespace TagLib;

  inline size_t UTF16toUTF8(
    const wchar_t *src, size_t srcLength, char *dst, size_t dstLength)
  {
    size_t len = 0;

#ifdef _WIN32

    len = ::WideCharToMultiByte(
      CP_UTF8, 0, src, static_cast<int>(srcLength), dst, static_cast<int>(dstLength), NULL, NULL);

#else

    using namespace Unicode;

    const UTF16 *srcBegin = src;
    const UTF16 *srcEnd   = srcBegin + srcLength;

    UTF8 *dstBegin = reinterpret_cast<UTF8*>(dst);
    UTF8 *dstEnd   = dstBegin + dstLength;

    ConversionResult result = ConvertUTF16toUTF8(
      &srcBegin, srcEnd, &dstBegin, dstEnd, lenientConversion);

    if(result == conversionOK)
      len = dstBegin - reinterpret_cast<UTF8*>(dst);

#endif

    if(len == 0)
      debug("String::UTF16toUTF8() - Unicode conversion error.");

    return len;
  }

  inline size_t UTF8toUTF16(
    const char *src, size_t srcLength, wchar_t *dst, size_t dstLength)
  {
    size_t len = 0;

#ifdef _WIN32

    len = ::MultiByteToWideChar(
      CP_UTF8, 0, src, static_cast<int>(srcLength), dst, static_cast<int>(dstLength));

#else

    using namespace Unicode;

    const UTF8 *srcBegin = reinterpret_cast<const UTF8*>(src);
    const UTF8 *srcEnd   = srcBegin + srcLength;

    UTF16 *dstBegin = dst;
    UTF16 *dstEnd   = dstBegin + dstLength;

    ConversionResult result = ConvertUTF8toUTF16(
      &srcBegin, srcEnd, &dstBegin, dstEnd, lenientConversion);

    if(result == conversionOK)
      len = dstBegin - dst;

#endif

    if(len == 0)
      debug("String::UTF8toUTF16() - Unicode conversion error.");

    return len;
  }

  // Returns the native format of std::wstring.
  inline String::Type wcharByteOrder()
  {
    if(Utils::systemByteOrder() == LittleEndian)
      return String::UTF16LE;
    else
      return String::UTF16BE;
  }

  // Converts a Latin-1 string into UTF-16(without BOM/CPU byte order)
  // and copies it to the internal buffer.
  inline void copyFromLatin1(std::wstring &data, const char *s, size_t length)
  {
    data.resize(length);

    for(size_t i = 0; i < length; ++i)
      data[i] = static_cast<unsigned char>(s[i]);
  }

  // Converts a UTF-8 string into UTF-16(without BOM/CPU byte order)
  // and copies it to the internal buffer.
  inline void copyFromUTF8(std::wstring &data, const char *s, size_t length)
  {
    data.resize(length);

    if(length > 0) {
      const size_t len = UTF8toUTF16(s, length, &data[0], data.size());
      data.resize(len);
    }
  }

  // Converts a UTF-16 (with BOM), UTF-16LE or UTF16-BE string into
  // UTF-16(without BOM/CPU byte order) and copies it to the internal buffer.
  inline void copyFromUTF16(std::wstring &data, const wchar_t *s, size_t length, String::Type t)
  {
    bool swap;
    if(t == String::UTF16) {
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
    else {
      swap = (t != wcharByteOrder());
    }

    data.resize(length);
    if(length > 0) {
      if(swap) {
        for(size_t i = 0; i < length; ++i)
          data[i] = Utils::byteSwap(static_cast<unsigned short>(s[i]));
      }
      else {
        ::wmemcpy(&data[0], s, length);
      }
    }
  }

  // Converts a UTF-16 (with BOM), UTF-16LE or UTF16-BE string into
  // UTF-16(without BOM/CPU byte order) and copies it to the internal buffer.
  inline void copyFromUTF16(std::wstring &data, const char *s, size_t length, String::Type t)
  {
    bool swap;
    if(t == String::UTF16) {
      if(length < 2) {
        debug("String::copyFromUTF16() - Invalid UTF16 string.");
        return;
      }

      // Uses memcpy instead of reinterpret_cast to avoid an alignment exception.
      unsigned short bom;
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
    else {
      swap = (t != wcharByteOrder());
    }

    data.resize(length / 2);
    for(size_t i = 0; i < length / 2; ++i) {
      unsigned short c;
      ::memcpy(&c, s, 2);
      if(swap)
        c = Utils::byteSwap(c);

      data[i] = static_cast<wchar_t>(c);
      s += 2;
    }
  }
}

namespace TagLib {

class String::StringPrivate
{
public:
  StringPrivate() :
    data(new std::wstring()) {}

  /*!
   * Stores string in UTF-16. The byte order depends on the CPU endian.
   */
  SHARED_PTR<std::wstring> data;

  /*!
   * This is only used to hold the the most recent value of toCString().
   */
  SHARED_PTR<std::string> cstring;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

size_t String::npos()
{
  return std::wstring::npos;
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

String::String() :
  d(new StringPrivate())
{
}

String::String(const String &s) :
  d(new StringPrivate(*s.d))
{
}

String::String(const std::string &s, Type t) :
  d(new StringPrivate())
{
  if(t == Latin1)
    copyFromLatin1(*d->data, s.c_str(), s.length());
  else if(t == String::UTF8)
    copyFromUTF8(*d->data, s.c_str(), s.length());
  else {
    debug("String::String() -- std::string should not contain UTF16.");
  }
}

String::String(const std::wstring &s, Type t) :
  d(new StringPrivate())
{
  if(t == UTF16Native)
    t = wcharByteOrder();

  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(*d->data, s.c_str(), s.length(), t);
  else {
    debug("String::String() -- std::wstring should not contain Latin1 or UTF-8.");
  }
}

String::String(const wchar_t *s, Type t) :
  d(new StringPrivate())
{
  if(t == UTF16Native)
    t = wcharByteOrder();

  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(*d->data, s, ::wcslen(s), t);
  else {
    debug("String::String() -- const wchar_t * should not contain Latin1 or UTF-8.");
  }
}

String::String(const char *s, Type t) :
  d(new StringPrivate())
{
  if(t == Latin1)
    copyFromLatin1(*d->data, s, ::strlen(s));
  else if(t == String::UTF8)
    copyFromUTF8(*d->data, s, ::strlen(s));
  else {
    debug("String::String() -- const char * should not contain UTF16.");
  }
}

String::String(wchar_t c, Type t) :
  d(new StringPrivate())
{
  if(t == UTF16Native)
    t = wcharByteOrder();

  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(*d->data, &c, 1, t);
  else {
    debug("String::String() -- wchar_t should not contain Latin1 or UTF-8.");
  }
}

String::String(char c, Type t) :
  d(new StringPrivate())
{
  if(t == Latin1)
    copyFromLatin1(*d->data, &c, 1);
  else if(t == String::UTF8)
    copyFromUTF8(*d->data, &c, 1);
  else {
    debug("String::String() -- char should not contain UTF16.");
  }
}

String::String(const ByteVector &v, Type t) :
  d(new StringPrivate())
{
  if(v.isEmpty())
    return;

  if(t == UTF16Native)
    t = wcharByteOrder();

  if(t == Latin1)
    copyFromLatin1(*d->data, v.data(), v.size());
  else if(t == UTF8)
    copyFromUTF8(*d->data, v.data(), v.size());
  else
    copyFromUTF16(*d->data, v.data(), v.size(), t);

  // If we hit a null in the ByteVector, shrink the string again.
  d->data->resize(::wcslen(d->data->c_str()));
}

////////////////////////////////////////////////////////////////////////////////

String::~String()
{
  delete d;
}

std::string String::to8Bit(bool unicode) const
{
  const ByteVector v = data(unicode ? UTF8 : Latin1);
  return std::string(v.data(), v.size());
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
  detach();
  return d->data->begin();
}

String::ConstIterator String::begin() const
{
  return d->data->begin();
}

String::Iterator String::end()
{
  detach();
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
  for(size_t index = 0;;) {
    const size_t sep = find(separator, index);
    if(sep == npos()) {
      list.append(substr(index, size() - index));
      break;
    }
    else {
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

String & String::clear()
{
  *this = String();
  return *this;
}

String String::upper() const
{
  static const int shift = 'A' - 'a';

  String s(*d->data);
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
    if(!d->data->empty())
    {
      ByteVector v(size() * 4 + 1, 0);

      const size_t len = UTF16toUTF8(
        d->data->c_str(), d->data->size(), v.data(), v.size());
      v.resize(len);

      return v;
    }
    else {
      return ByteVector();
    }
  case UTF16:
    {
      ByteVector v(2 + size() * 2, 0);
      char *p = v.data();

      // We use little-endian encoding here and need a BOM.

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
  const wchar_t *begin = d->data->c_str();
  wchar_t *end;
  errno = 0;
  const long value = ::wcstol(begin, &end, 10);

  // Has wcstol() consumed the entire string and not overflowed?
  if(ok) {
    *ok = (errno == 0 && end > begin && *end == L'\0');
    *ok = (*ok && value > INT_MIN && value < INT_MAX);
  }

  return static_cast<int>(value);
}

String String::stripWhiteSpace() const
{
  static const wchar_t *WhiteSpaceChars = L"\t\n\f\r ";

  const size_t pos1 = d->data->find_first_not_of(WhiteSpaceChars);
  if(pos1 == std::wstring::npos)
    return String();

  const size_t pos2 = d->data->find_last_not_of(WhiteSpaceChars);
  return substr(pos1, pos2 - pos1 + 1);
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
  return Utils::formatString("%d", n);
}

wchar_t &String::operator[](size_t i)
{
  detach();
  return (*d->data)[i];
}

const wchar_t &String::operator[](size_t i) const
{
  return (*d->data)[i];
}

bool String::operator==(const String &s) const
{
  return (d->data == s.d->data || *d->data == *s.d->data);
}

bool String::operator!=(const String &s) const
{
  return !(*this == s);
}

bool String::operator==(const char *s) const
{
  const wchar_t *p = toCWString();

  while(*p != L'\0' || *s != '\0') {
    if(*p++ != static_cast<unsigned char>(*s++))
      return false;
  }
  return true;
}

bool String::operator!=(const char *s) const
{
  return !(*this == s);
}

bool String::operator==(const wchar_t *s) const
{
  return (*d->data == s);
}

bool String::operator!=(const wchar_t *s) const
{
  return !(*this == s);
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
    *d->data += static_cast<unsigned char>(s[i]);

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

  *d->data += static_cast<unsigned char>(c);
  return *this;
}

String &String::operator=(const String &s)
{
  String(s).swap(*this);
  return *this;
}

String &String::operator=(const std::string &s)
{
  String(s).swap(*this);
  return *this;
}

String &String::operator=(const std::wstring &s)
{
  String(s).swap(*this);
  return *this;
}

String &String::operator=(const wchar_t *s)
{
  String(s).swap(*this);
  return *this;
}

String &String::operator=(char c)
{
  String(c).swap(*this);
  return *this;
}

String &String::operator=(wchar_t c)
{
  String(c).swap(*this);
  return *this;
}

String &String::operator=(const char *s)
{
  String(s).swap(*this);
  return *this;
}

String &String::operator=(const ByteVector &v)
{
  String(v).swap(*this);
  return *this;
}

void String::swap(String &s)
{
  using std::swap;

  swap(d, s.d);
}

bool String::operator<(const String &s) const
{
  return (*d->data < *s.d->data);
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void String::detach()
{
  if(!d->data.unique())
    String(d->data->c_str()).swap(*this);
}

////////////////////////////////////////////////////////////////////////////////
// related non-member functions
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
