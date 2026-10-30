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

#include "tcharset.h"

#include <string>
#include <limits>

#include "charsettables.h"
#include "tbytevector.h"
#include "tstring.h"

using namespace TagLib;

std::optional<TagLib::Charset::Type> TagLib::Charset::typeForCodePage(unsigned int codePage)
{
  if(codePage == 65001)
    return Type::UTF8;
  const auto *entry = detail::entryForCodePage(codePage);
  if(entry)
    return entry->type;

  return std::nullopt;
}

unsigned int TagLib::Charset::codePageForType(Type type)
{
  return static_cast<unsigned int>(type);
}

String TagLib::Charset::decode(const ByteVector &data, Type type)
{
  if(type == Type::UTF8)
    return String(data, String::UTF8);

  // Fast path for likely Latin-1 case
  if(type == Type::Latin1)
    return String(data, String::Latin1);

  const auto *entry = detail::entryForCodePage(static_cast<unsigned int>(type));
  if(!entry)
    return String();

  std::wstring text;
  text.reserve(data.size());
  for(const char c : data) {
    if(c == '\0')
      break;
    text.push_back(static_cast<wchar_t>(entry->toUnicode[static_cast<unsigned char>(c)]));
  }
  return String(text);
}

ByteVector TagLib::Charset::encode(const String &s, Type type)
{
  if(type == Type::UTF8)
    return s.data(String::UTF8);

  if(type == Type::Latin1) {
    // String::data(Latin1) casts every code point to char, keeping only the
    // low byte, so the '?' substitution the header promises is done here.
    ByteVector data(s.size(), '?');
    unsigned int i = 0;
    for(const wchar_t c : s) {
      if(c < 256)
        data[i] = static_cast<char>(c);
      ++i;
    }
    return data;
  }

  const auto *entry = detail::entryForCodePage(static_cast<unsigned int>(type));
  if(!entry)
    return {};

  const unsigned short *unicodes = entry->toUnicode;
  ByteVector data(s.size(), '?');
  int i = 0;
  for(auto it = s.begin(); it != s.end(); ++it, ++i) {
    const wchar_t wc = *it;
    if(wc > std::numeric_limits<unsigned short>::max())
      continue;
    for(int ch = 0; ch < 256; ++ch) {
      if(unicodes[ch] == wc) {
        data[i] = static_cast<char>(ch);
        break;
      }
    }
  }
  return data;
}
