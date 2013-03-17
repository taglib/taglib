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
#include "tdebug.h"
#include "tstringlist.h"

#include <string.h>

// Determine if the compiler supports codecvt.

#if (defined(_MSC_VER) && _MSC_VER >= 1600)  // VC++2010 or later 
# define TAGLIB_USE_CODECVT
#endif

#ifdef TAGLIB_USE_CODECVT
# include <codecvt>
  typedef std::codecvt_utf8_utf16<wchar_t> utf8_utf16_t;
#else
# include "unicode.h"
#endif

using namespace TagLib;

namespace {

  inline unsigned short byteSwap(unsigned short x)
  {
#if defined(_MSC_VER) && (_MSC_VER >= 1400)  // VC++2005 or later

    return _byteswap_ushort(x);

#else

    return (((x) >> 8) & 0xff) | (((x) & 0xff) << 8);

#endif
  }

  inline unsigned short combine(unsigned char c1, unsigned char c2)
  {
    return (c1 << 8) | c2;
  }

  String::Type wcharByteOrder() 
  {
    // Detect CPU endian.
    union {
      TagLib::ushort w;
      char c;
    } x = { 0x1234 };

    if(x.c == 0x34)
      return String::UTF16LE;
    else
      return String::UTF16BE;
  }
}


class String::StringPrivate : public RefCounter
{
public:
  StringPrivate(const wstring &s) :
    RefCounter(),
    data(s) {}

  StringPrivate() :
    RefCounter() {}

  /*!
   * Stores string in UTF-16. The byte order depends on the CPU endian. 
   */
  TagLib::wstring data;

  /*!
   * This is only used to hold the the most recent value of toCString().
   */
  std::string cstring;
};

String String::null;

////////////////////////////////////////////////////////////////////////////////

String::String()
{
  d = new StringPrivate;
}

String::String(const String &s) : d(s.d)
{
  d->ref();
}

String::String(const std::string &s, Type t)
{
  d = new StringPrivate;

  if(t == Latin1)
    copyFromLatin1(&s[0], s.length());
  else if(t == String::UTF8)
    copyFromUTF8(&s[0], s.length());
  else {
    debug("String::String() -- A std::string should not contain UTF16.");
  }
}

String::String(const wstring &s, Type t)
{
  d = new StringPrivate;

  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(s.c_str(), s.length(), t);
  else {
    debug("String::String() -- A TagLib::wstring should not contain Latin1 or UTF-8.");
  }
}

String::String(const wchar_t *s, Type t)
{
  d = new StringPrivate;

  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(s, ::wcslen(s), t);
  else {
    debug("String::String() -- A const wchar_t * should not contain Latin1 or UTF-8.");
  }
}

String::String(const char *s, Type t)
{
  d = new StringPrivate;

  if(t == Latin1)
    copyFromLatin1(s, ::strlen(s));
  else if(t == String::UTF8)
    copyFromUTF8(s, ::strlen(s));
  else {
    debug("String::String() -- A const char * should not contain UTF16.");
  }
}

String::String(wchar_t c, Type t)
{
  d = new StringPrivate;
  
  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(&c, 1, t);
  else {
    debug("String::String() -- A const wchar_t should not contain Latin1 or UTF-8.");
  }
}

String::String(char c, Type t)
{
  d = new StringPrivate;

  if(t == Latin1 || t == UTF8) {
    d->data.resize(1);
    d->data[0] = static_cast<uchar>(c);
  }
  else {
    debug("String::String() -- A char  should not contain UTF16.");
  }
}

String::String(const ByteVector &v, Type t)
{
  d = new StringPrivate;

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
  if(d->deref())
    delete d;
}

std::string String::to8Bit(bool unicode) const
{
  std::string s;

  if(!unicode) {
    s.resize(d->data.size());

    std::string::iterator targetIt = s.begin();
    for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {
      *targetIt = static_cast<char>(*it);
      ++targetIt;
    }
  }
  else {
    s.resize(d->data.size() * 4 + 1);

#ifdef TAGLIB_USE_CODECVT

    std::mbstate_t st = 0;
    const wchar_t *source;
    char *target;
    std::codecvt_base::result result = utf8_utf16_t().out(
      st, &d->data[0], &d->data[d->data.size()], source, &s[0], &s[s.size()], target);

    if(result != utf8_utf16_t::ok) {
      debug("String::copyFromUTF8() - Unicode conversion error.");
    }

#else

    const Unicode::UTF16 *source = &d->data[0];
    Unicode::UTF8 *target = reinterpret_cast<Unicode::UTF8*>(&s[0]);

    Unicode::ConversionResult result = Unicode::ConvertUTF16toUTF8(
      &source, source + d->data.size(),
      &target, target + s.size(),
      Unicode::lenientConversion);

    if(result != Unicode::conversionOK) {
      debug("String::to8Bit() - Unicode conversion error.");
    }

#endif

    s.resize(::strlen(s.c_str()));
  }

  return s;
}

const TagLib::wstring &String::toWString() const
{
  return d->data;
}

const char *String::toCString(bool unicode) const
{
  d->cstring = to8Bit(unicode);
  return d->cstring.c_str();
}

String::Iterator String::begin()
{
  return d->data.begin();
}

String::ConstIterator String::begin() const
{
  return d->data.begin();
}

String::Iterator String::end()
{
  return d->data.end();
}

String::ConstIterator String::end() const
{
  return d->data.end();
}

int String::find(const String &s, int offset) const
{
  const size_t position 
    = d->data.find(s.d->data, offset == -1 ? wstring::npos : offset);

  if(position != wstring::npos)
    return static_cast<int>(position);
  else
    return -1;
}

int String::rfind(const String &s, int offset) const
{
  const size_t position =
    d->data.rfind(s.d->data, offset == -1 ? wstring::npos : offset);

  if(position != wstring::npos)
    return static_cast<int>(position);
  else
    return -1;
}

StringList String::split(const String &separator) const
{
  StringList list;
  for(int index = 0;;)
  {
    const int sep = find(separator, index);
    if(sep < 0)
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

String String::substr(uint position, uint n) const
{
  String s;
  s.d->data = d->data.substr(position, n);
  return s;
}

String &String::append(const String &s)
{
  detach();
  d->data += s.d->data;
  return *this;
}

String String::upper() const
{
  static const int shift = 'A' - 'a';

  String s;
  s.d->data.reserve(d->data.size());

  for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); ++it) {
    if(*it >= 'a' && *it <= 'z')
      s.d->data.push_back(*it + shift);
    else
      s.d->data.push_back(*it);
  }

  return s;
}

TagLib::uint String::size() const
{
  return static_cast<TagLib::uint>(d->data.size());
}

TagLib::uint String::length() const
{
  return size();
}

bool String::isEmpty() const
{
  return (d->data.size() == 0);
}

bool String::isNull() const
{
  return (d == null.d);
}

ByteVector String::data(Type t) const
{
  ByteVector v;

  switch(t) {

  case Latin1:
  {
    for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++)
      v.append(char(*it));
    break;
  }
  case UTF8:
  {
    v.resize(d->data.size() * 4 + 1);

#ifdef TAGLIB_USE_CODECVT

    std::mbstate_t st = 0;
    const wchar_t *source;
    char *target;
    std::codecvt_base::result result = utf8_utf16_t().out(
        st, &d->data[0], &d->data[d->data.size()], source, v.data(), v.data() + v.size(), target);

    if(result != utf8_utf16_t::ok) {
      debug("String::copyFromUTF8() - Unicode conversion error.");
    }

#else

    const Unicode::UTF16 *source = &d->data[0];
    Unicode::UTF8 *target = reinterpret_cast<Unicode::UTF8*>(v.data());

    Unicode::ConversionResult result = Unicode::ConvertUTF16toUTF8(
      &source, source + d->data.size(),
      &target, target + v.size(),
      Unicode::lenientConversion);

    if(result != Unicode::conversionOK) {
      debug("String::to8Bit() - Unicode conversion error.");
    }

#endif

    v.resize(::strlen(v.data()) + 1);

    break;
  }
  case UTF16:
  {
    // Assume that if we're doing UTF16 and not UTF16BE that we want little
    // endian encoding.  (Byte Order Mark)

    v.append(char(0xff));
    v.append(char(0xfe));

    for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {

      char c1 = *it & 0xff;
      char c2 = *it >> 8;

      v.append(c1);
      v.append(c2);
    }
    break;
  }
  case UTF16BE:
  {
    for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {

      char c1 = *it >> 8;
      char c2 = *it & 0xff;

      v.append(c1);
      v.append(c2);
    }
    break;
  }
  case UTF16LE:
  {
    for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {

      char c1 = *it & 0xff;
      char c2 = *it >> 8;

      v.append(c1);
      v.append(c2);
    }
    break;
  }
  }

  return v;
}

int String::toInt(bool *ok) const
{
  int value = 0;

  const size_t size = d->data.size();
  const bool negative = size > 0 && d->data[0] == '-';
  const size_t start = negative ? 1 : 0;

  size_t i = start;
  for(; i < size && d->data[i] >= '0' && d->data[i] <= '9'; i++)
    value = value * 10 + (d->data[i] - '0');

  if(negative)
    value = value * -1;

  if(ok)
    *ok = (size > start && i == size);

  return value;
}

String String::stripWhiteSpace() const
{
  wstring::const_iterator begin = d->data.begin();
  wstring::const_iterator end = d->data.end();

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

  return String(wstring(begin, end + 1));
}

bool String::isLatin1() const
{
  for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {
    if(*it >= 256)
      return false;
  }
  return true;
}

bool String::isAscii() const
{
  for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {
    if(*it >= 128)
      return false;
  }
  return true;
}

String String::number(int n) // static
{
  if(n == 0)
    return String("0");

  String charStack;

  bool negative = n < 0;

  if(negative) 
    n = n * -1;

  while(n > 0) {
    int remainder = n % 10;
    charStack += char(remainder + '0');
    n = (n - remainder) / 10;
  }

  String s;

  if(negative)
    s += '-';

  for(int i = charStack.d->data.size() - 1; i >= 0; i--)
    s += charStack.d->data[i];

  return s;
}

TagLib::wchar &String::operator[](size_t i)
{
  detach();

  return d->data[i];
}

const TagLib::wchar &String::operator[](size_t i) const
{
  return d->data[i];
}

bool String::operator==(const String &s) const
{
  return (d == s.d || d->data == s.d->data);
}

bool String::operator==(const char *s) const
{
  for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {
    if(*it != static_cast<uchar>(*s))
      return false;

    s++;
  }

  return true;
}

bool String::operator==(const wchar_t *s) const
{
  return (d->data == s);
}

bool String::operator!=(const String &s) const
{
  return !operator==(s);
}

String &String::operator+=(const String &s)
{
  detach();

  d->data += s.d->data;
  return *this;
}

String &String::operator+=(const wchar_t *s)
{
  detach();

  d->data += s;
  return *this;
}

String &String::operator+=(const char *s)
{
  detach();

  for(int i = 0; s[i] != 0; i++)
    d->data += uchar(s[i]);
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

  d->data += uchar(c);
  return *this;
}

String &String::operator=(const String &s)
{
  if(&s == this)
    return *this;

  if(d->deref())
    delete d;
  d = s.d;
  d->ref();
  return *this;
}

String &String::operator=(const std::string &s)
{
  if(d->deref())
    delete d;

  d = new StringPrivate;
  copyFromLatin1(s.c_str(), s.length());

  return *this;
}

String &String::operator=(const wstring &s)
{
  if(d->deref())
    delete d;
  
  d = new StringPrivate(s);

  return *this;
}

String &String::operator=(const wchar_t *s)
{
  if(d->deref())
    delete d;
  
  d = new StringPrivate;
  copyFromUTF16(s, ::wcslen(s), WCharByteOrder);

  return *this;
}

String &String::operator=(char c)
{
  if(d->deref())
    delete d;
  
  d = new StringPrivate;
  d->data.resize(1);
  d->data[0] = static_cast<uchar>(c);

  return *this;
}

String &String::operator=(wchar_t c)
{
  if(d->deref())
    delete d;

  d = new StringPrivate;
  d->data.resize(1);
  d->data[0] = c;

  return *this;
}

String &String::operator=(const char *s)
{
  if(d->deref())
    delete d;

  d = new StringPrivate;
  copyFromLatin1(s, ::strlen(s));

  return *this;
}

String &String::operator=(const ByteVector &v)
{
  if(d->deref())
    delete d;

  d = new StringPrivate;
  copyFromLatin1(v.data(), v.size());

  // If we hit a null in the ByteVector, shrink the string again.
  d->data.resize(::wcslen(d->data.c_str()));

  return *this;
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
  if(d->count() > 1) {
    d->deref();
    d = new StringPrivate(d->data);
  }
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void String::copyFromLatin1(const char *s, size_t length)
{
  d->data.resize(length);

  for(size_t i = 0; i < length; ++i)
    d->data[i] = static_cast<uchar>(s[i]);
}

void String::copyFromUTF8(const char *s, size_t length)
{
  d->data.resize(length);

#ifdef TAGLIB_USE_CODECVT

  std::mbstate_t st = 0;
  const char *source;
  wchar_t *target;
  std::codecvt_base::result result = utf8_utf16_t().in(
    st, s, s + length, source, &d->data[0], &d->data[d->data.size()], target);

  if(result != utf8_utf16_t::ok) {
    debug("String::copyFromUTF8() - Unicode conversion error.");
  }

#else
  
  const Unicode::UTF8 *source = reinterpret_cast<const Unicode::UTF8 *>(s);
  Unicode::UTF16 *target = &d->data[0];

  Unicode::ConversionResult result = Unicode::ConvertUTF8toUTF16(
    &source, source + length,
    &target, target + length,
    Unicode::lenientConversion);

  if(result != Unicode::conversionOK) {
    debug("String::copyFromUTF8() - Unicode conversion error.");
  }

#endif

  d->data.resize(::wcslen(d->data.c_str()));
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

  d->data.resize(length);
  memcpy(&d->data[0], s, length * sizeof(wchar_t));

  if(swap) {
    for(size_t i = 0; i < length; ++i)
      d->data[i] = byteSwap(static_cast<unsigned short>(s[i]));
  }
}

void String::copyFromUTF16(const char *s, size_t length, Type t)
{
  if(sizeof(wchar_t) == 2) 
    copyFromUTF16(reinterpret_cast<const wchar_t*>(s), length / 2, t);
  else
  {
    bool swap;
    if(t == UTF16) {
      if(length >= 2 && *reinterpret_cast<const TagLib::ushort*>(s) == 0xfeff) 
        swap = false; // Same as CPU endian. No need to swap bytes.
      else if(length >= 2 && *reinterpret_cast<const TagLib::ushort*>(s) == 0xfffe) 
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

    d->data.resize(length / 2);
    for(size_t i = 0; i < length / 2; ++i) {
      d->data[i] = swap ? combine(*s, *(s + 1)) : combine(*(s + 1), *s);
      s += 2;
    }
  }
}

String::Type String::WCharByteOrder = wcharByteOrder();


////////////////////////////////////////////////////////////////////////////////
// related functions
////////////////////////////////////////////////////////////////////////////////

const TagLib::String TagLib::operator+(const TagLib::String &s1, const TagLib::String &s2)
{
  String s(s1);
  s.append(s2);
  return s;
}

const TagLib::String TagLib::operator+(const char *s1, const TagLib::String &s2)
{
  String s(s1);
  s.append(s2);
  return s;
}

const TagLib::String TagLib::operator+(const TagLib::String &s1, const char *s2)
{
  String s(s1);
  s.append(s2);
  return s;
}

std::ostream &TagLib::operator<<(std::ostream &s, const String &str)
{
  s << str.to8Bit();
  return s;
}
