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

#include "tstring.h"

#include <cerrno>
#include <climits>
#include <iostream>
#include <utf8.h>

#include "tdebug.h"
#include "tstringlist.h"
#include "tutils.h"

namespace
{
  using namespace TagLib;

  // Returns the native format of std::wstring.
  constexpr String::Type wcharByteOrder()
  {
    return Utils::systemByteOrder() == Utils::LittleEndian ? String::UTF16LE
                                                           : String::UTF16BE;
  }

  // Converts a Latin-1 string into UTF-16(without BOM/CPU byte order)
  // and copies it to the internal buffer.
  void copyFromLatin1(std::wstring &data, const char *s, size_t length)
  {
    data.resize(length);

    for(size_t i = 0; i < length; ++i)
      data[i] = static_cast<unsigned char>(s[i]);
  }

  // Converts a UTF-8 string into UTF-16(without BOM/CPU byte order)
  // and copies it to the internal buffer.
  void copyFromUTF8(std::wstring &data, const char *s, size_t length)
  {
    data.resize(length);

    try {
      const std::wstring::iterator dstEnd = utf8::utf8to16(s, s + length, data.begin());
      data.resize(dstEnd - data.begin());
    }
    catch(const utf8::exception &e) {
      const String message(e.what());
      debug("String::copyFromUTF8() - UTF8-CPP error: " + message);
      data.clear();
    }
  }

  // Helper functions to read a UTF-16 character from an array.
  template <typename T>
  unsigned short nextUTF16(const T **p);

  template <>
  unsigned short nextUTF16<wchar_t>(const wchar_t **p)
  {
    return static_cast<unsigned short>(*(*p)++);
  }

  template <>
  unsigned short nextUTF16<char>(const char **p)
  {
    union {
      unsigned short w;
      char c[2];
    } u;
    u.c[0] = *(*p)++;
    u.c[1] = *(*p)++;
    return u.w;
  }

  // Converts a UTF-16 (with BOM), UTF-16LE or UTF16-BE string into
  // UTF-16(without BOM/CPU byte order) and copies it to the internal buffer.
  template <typename T>
  void copyFromUTF16(std::wstring &data, const T *s, size_t length, String::Type t)
  {
    bool swap;
    if(t == String::UTF16) {
      if(length < 1) {
        debug("String::copyFromUTF16() - Invalid UTF16 string. Too short to have a BOM.");
        return;
      }

      if(const unsigned short bom = nextUTF16(&s); bom == 0xfeff)
        swap = false; // Same as CPU endian. No need to swap bytes.
      else if(bom == 0xfffe)
        swap = true;  // Not same as CPU endian. Need to swap bytes.
      else {
        debug("String::copyFromUTF16() - Invalid UTF16 string. BOM is broken.");
        return;
      }

      length--;
    }
    else {
      swap = t != wcharByteOrder();
    }

    data.resize(length);
    for(size_t i = 0; i < length; ++i) {
      const unsigned short c = nextUTF16(&s);
      if(swap)
        data[i] = Utils::byteSwap(c);
      else
        data[i] = c;
    }
  }
}  // namespace

namespace TagLib {

  class String::StringPrivate
  {
  public:
    /*!
     * Stores string in UTF-16. The byte order depends on the CPU endian.
     */
    std::wstring data;

    /*!
     * This is only used to hold the most recent value of toCString().
     */
    std::string cstring;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

String::String() :
  d(std::make_shared<StringPrivate>())
{
}

String::String(const String &) = default;

String::String(const std::string &s, Type t) :
  d(std::make_shared<StringPrivate>())
{
  if(t == Latin1)
    copyFromLatin1(d->data, s.c_str(), s.length());
  else if(t == String::UTF8)
    copyFromUTF8(d->data, s.c_str(), s.length());
  else {
    debug("String::String() -- std::string should not contain UTF16.");
  }
}

String::String(const std::wstring &s) :
 String(s, wcharByteOrder())
{
}

String::String(const std::wstring &s, Type t) :
  d(std::make_shared<StringPrivate>())
{
  if(t == UTF16 || t == UTF16BE || t == UTF16LE) {
    copyFromUTF16(d->data, s.c_str(), s.length(), t);
  }
  else {
    debug("String::String() -- std::wstring should not contain Latin1 or UTF-8.");
  }
}

String::String(const wchar_t *s) :
  String(s, wcharByteOrder())
{
}

String::String(const wchar_t *s, Type t) :
  d(std::make_shared<StringPrivate>())
{
  if(s) {
    if(t == UTF16 || t == UTF16BE || t == UTF16LE) {
      copyFromUTF16(d->data, s, ::wcslen(s), t);
    }
    else {
      debug("String::String() -- const wchar_t * should not contain Latin1 or UTF-8.");
    }
  }
}

String::String(const char *s, Type t) :
  d(std::make_shared<StringPrivate>())
{
  if(s) {
    if(t == Latin1)
      copyFromLatin1(d->data, s, ::strlen(s));
    else if(t == String::UTF8)
      copyFromUTF8(d->data, s, ::strlen(s));
    else {
      debug("String::String() -- const char * should not contain UTF16.");
    }
  }
}

String::String(wchar_t c, Type t) :
  d(std::make_shared<StringPrivate>())
{
  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(d->data, &c, 1, t);
  else {
    debug("String::String() -- wchar_t should not contain Latin1 or UTF-8.");
  }
}

String::String(char c, Type t) :
  d(std::make_shared<StringPrivate>())
{
  if(t == Latin1)
    copyFromLatin1(d->data, &c, 1);
  else if(t == String::UTF8)
    copyFromUTF8(d->data, &c, 1);
  else {
    debug("String::String() -- char should not contain UTF16.");
  }
}

String::String(const ByteVector &v, Type t) :
  d(std::make_shared<StringPrivate>())
{
  if(v.isEmpty())
    return;

  if(t == Latin1)
    copyFromLatin1(d->data, v.data(), v.size());
  else if(t == UTF8)
    copyFromUTF8(d->data, v.data(), v.size());
  else
    copyFromUTF16(d->data, v.data(), v.size() / 2, t);

  // If we hit a null in the ByteVector, shrink the string again.
  d->data.resize(::wcslen(d->data.c_str()));
}

////////////////////////////////////////////////////////////////////////////////

String::~String() = default;

std::string String::to8Bit(bool unicode) const
{
  const ByteVector v = data(unicode ? UTF8 : Latin1);
  return std::string(v.data(), v.size());
}

std::wstring String::toWString() const
{
  return d->data;
}

const char *String::toCString(bool unicode) const
{
  d->cstring = to8Bit(unicode);
  return d->cstring.c_str();
}

const wchar_t *String::toCWString() const
{
  return d->data.c_str();
}

String::Iterator String::begin()
{
  detach();
  return d->data.begin();
}

String::ConstIterator String::begin() const
{
  return d->data.begin();
}

String::ConstIterator String::cbegin() const
{
  return d->data.cbegin();
}

String::Iterator String::end()
{
  detach();
  return d->data.end();
}

String::ConstIterator String::end() const
{
  return d->data.end();
}

String::ConstIterator String::cend() const
{
  return d->data.cend();
}

int String::find(const String &s, int offset) const
{
  return static_cast<int>(d->data.find(s.d->data, offset));
}

int String::rfind(const String &s, int offset) const
{
  return static_cast<int>(d->data.rfind(s.d->data, offset));
}

StringList String::split(const String &separator) const
{
  StringList list;
  for(int index = 0;;) {
    int sep = find(separator, index);
    if(sep < 0) {
      list.append(substr(index, size() - index));
      break;
    }
    list.append(substr(index, sep - index));
    index = sep + separator.size();
  }
  return list;
}

bool String::startsWith(const String &s) const
{
  if(s.length() > length())
    return false;

  return substr(0, s.length()) == s;
}

String String::substr(unsigned int position, unsigned int n) const
{
  if(position == 0 && n >= size())
    return *this;
  return String(d->data.substr(position, n));
}

String &String::append(const String &s)
{
  detach();
  d->data += s.d->data;
  return *this;
}

String & String::clear()
{
  *this = String();
  return *this;
}

String String::upper() const
{
  String s;
  s.d->data.reserve(size());

  for(wchar_t c : *this) {
    if(c >= 'a' && c <= 'z')
      s.d->data.push_back(c + 'A' - 'a');
    else
      s.d->data.push_back(c);
  }

  return s;
}

unsigned int String::size() const
{
  return static_cast<unsigned int>(d->data.size());
}

unsigned int String::length() const
{
  return size();
}

bool String::isEmpty() const
{
  return d->data.empty();
}

ByteVector String::data(Type t) const
{
  switch(t)
  {
  case Latin1:
    {
      ByteVector v(size(), 0);
      char *p = v.data();

      for(wchar_t c : *this) {
        *p++ = static_cast<char>(c);
      }

      return v;
    }
  case UTF8:
    {
      ByteVector v(size() * 4, 0);

      try {
        const auto dstEnd = utf8::utf16to8(begin(), end(), v.begin());
        v.resize(static_cast<unsigned int>(dstEnd - v.begin()));
      }
      catch(const utf8::exception &e) {
        const String message(e.what());
        debug("String::data() - UTF8-CPP error: " + message);
        v.clear();
      }

      return v;
    }
  case UTF16:
    {
      ByteVector v(2 + size() * 2, 0);
      char *p = v.data();

      // We use little-endian encoding here and need a BOM.

      *p++ = '\xff';
      *p++ = '\xfe';

      for(wchar_t c : *this) {
        *p++ = static_cast<char>(c & 0xff);
        *p++ = static_cast<char>(c >> 8);
      }

      return v;
    }
  case UTF16BE:
    {
      ByteVector v(size() * 2, 0);
      char *p = v.data();

      for(wchar_t c : *this) {
        *p++ = static_cast<char>(c >> 8);
        *p++ = static_cast<char>(c & 0xff);
      }

      return v;
    }
  case UTF16LE:
    {
      ByteVector v(size() * 2, 0);
      char *p = v.data();

      for(wchar_t c : *this) {
        *p++ = static_cast<char>(c & 0xff);
        *p++ = static_cast<char>(c >> 8);
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
  const wchar_t *beginPtr = d->data.c_str();
  wchar_t *endPtr;
  errno = 0;
  const long value = ::wcstol(beginPtr, &endPtr, 10);

  // Has wcstol() consumed the entire string and not overflowed?
  if(ok) {
    *ok = errno == 0 && endPtr > beginPtr && *endPtr == L'\0';
    *ok = *ok && value > INT_MIN && value < INT_MAX;
  }

  return static_cast<int>(value);
}

String String::stripWhiteSpace() const
{
  static const wchar_t *WhiteSpaceChars = L"\t\n\f\r ";

  const size_t pos1 = d->data.find_first_not_of(WhiteSpaceChars);
  if(pos1 == std::wstring::npos)
    return String();

  const size_t pos2 = d->data.find_last_not_of(WhiteSpaceChars);
  return substr(static_cast<unsigned int>(pos1), static_cast<unsigned int>(pos2 - pos1 + 1));
}

bool String::isLatin1() const
{
  return std::none_of(this->begin(), this->end(), [](auto c) { return c >= 256; });
}

bool String::isAscii() const
{
  return std::none_of(this->begin(), this->end(), [](auto c) { return c >= 128; });
}

String String::number(int n) // static
{
  return std::to_string(n);
}

String String::fromLongLong(long long n) // static
{
  return std::to_string(n);
}

wchar_t &String::operator[](int i)
{
  detach();
  return d->data[i];
}

const wchar_t &String::operator[](int i) const
{
  return d->data[i];
}

bool String::operator==(const String &s) const
{
  return d == s.d || d->data == s.d->data;
}

bool String::operator!=(const String &s) const
{
  return !(*this == s);
}

bool String::operator==(const char *s) const
{
  if(!s) {
    return isEmpty();
  }

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
  if(!s) {
    return isEmpty();
  }

  return d->data == s;
}

bool String::operator!=(const wchar_t *s) const
{
  return !(*this == s);
}

String &String::operator+=(const String &s)
{
  detach();

  d->data += s.d->data;
  return *this;
}

String &String::operator+=(const wchar_t *s)
{
  if(s) {
    detach();

    d->data += s;
  }
  return *this;
}

String &String::operator+=(const char *s)
{
  if(s) {
    detach();

    for(int i = 0; s[i] != 0; i++)
      d->data += static_cast<unsigned char>(s[i]);
  }
  return *this;
}

String &String::operator+=(wchar_t c)
{
  detach();

  d->data += c;
  return *this;
}

String &String::operator+=(char c)
{
  detach();

  d->data += static_cast<unsigned char>(c);
  return *this;
}

String &String::operator=(const String &) = default;

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
  String(c, wcharByteOrder()).swap(*this);
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

void String::swap(String &s) noexcept
{
  using std::swap;

  swap(d, s.d);
}

bool String::operator<(const String &s) const
{
  return d->data < s.d->data;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void String::detach()
{
  if(d.use_count() > 1)
    String(d->data.c_str()).swap(*this);
}

}  // namespace TagLib

////////////////////////////////////////////////////////////////////////////////
// related non-member functions
////////////////////////////////////////////////////////////////////////////////

TagLib::String operator+(const TagLib::String &s1, const TagLib::String &s2)
{
  TagLib::String s(s1);
  s.append(s2);
  return s;
}

TagLib::String operator+(const char *s1, const TagLib::String &s2)
{
  TagLib::String s(s1);
  s.append(s2);
  return s;
}

TagLib::String operator+(const TagLib::String &s1, const char *s2)
{
  TagLib::String s(s1);
  s.append(s2);
  return s;
}

std::ostream &operator<<(std::ostream &s, const TagLib::String &str)
{
  s << str.to8Bit(true);
  return s;
}
