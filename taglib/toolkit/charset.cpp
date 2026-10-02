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

#include "charset.h"
#include "tbytevector.h"
#include "tstring.h"

#include <string>

using namespace TagLib;

namespace
{
  const TagLib::Charset::detail::CodePageEntry *entryFor(TagLib::Charset::Type type)
  {
    unsigned int count = 0;
    const auto *entries = TagLib::Charset::detail::codePageEntries(&count);
    const auto index = static_cast<unsigned int>(type);
    return index < count ? &entries[index] : nullptr;
  }
}  // namespace

std::optional<TagLib::Charset::Type> TagLib::Charset::typeForCodePage(unsigned int codePage)
{
  unsigned int count = 0;
  const auto *entries = detail::codePageEntries(&count);
  for(unsigned int i = 0; i < count; ++i) {
    if(entries[i].codePage == codePage)
      return entries[i].type;
  }
  if(codePage == 65001)
    return Type::UTF8;

  return std::nullopt;
}

unsigned int TagLib::Charset::codePageForType(Type type)
{
  if(type == Type::UTF8)
    return 65001;
  if(const auto *entry = entryFor(type))
    return entry->codePage;

  return 0;
}

String TagLib::Charset::decode(const ByteVector &data, Type type)
{
  if(type == Type::UTF8)
    return String(data, String::UTF8);

  const auto *entry = entryFor(type);
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

ByteVector TagLib::Charset::encode(const String &s)
{
  return s.data(String::UTF8);
}
