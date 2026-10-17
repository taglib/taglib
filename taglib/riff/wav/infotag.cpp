/***************************************************************************
    copyright            : (C) 2012 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
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

#include "infotag.h"

#include <memory>
#include <utility>
#include <vector>

#include "charset.h"
#include "tbytevector.h"
#include "tdebug.h"
#include "tpropertymap.h"
#include "riffutils.h"

using namespace TagLib;
using namespace RIFF::Info;

namespace
{
  const RIFF::Info::StringHandler defaultStringHandler;
  const RIFF::Info::StringHandler *stringHandler = &defaultStringHandler;

  // The code page RIFF::Info::Tag::setAssumedCodePage() states, used when a file
  // declares none. 0 is the RIFF "unspecified" value and keeps the ISO 8859-1 the
  // specification prescribes for it.
  unsigned int codePageAssumed = 0;

  // The charset a file's CSET code page selects. Code page 0 means the file
  // declares none, so the code page the caller assumed applies; so does the
  // RIFF default "Latin1" for a code page this library has no table for. The
  // number is a hint about how the text was written, and dropping every field
  // over a hint we cannot follow would throw the whole tag away; Latin-1 at
  // least keeps the ASCII part of it intact.
  Charset::Type charsetForCodePage(unsigned int codePage)
  {
    if(const auto charset = Charset::typeForCodePage(codePage))
      return *charset;

    if(codePage != 0) {
      debug("RIFF::Info::Tag::parse() - Unsupported CSET code page " +
            String::number(static_cast<int>(codePage)) +
            ", using the RIFF default ISO 8859-1.");
      return Charset::Type::Latin1;
    }

    // The file declares nothing. setAssumedCodePage() only ever stored 0 or a
    // code page it verified, so the only way to miss here is that 0, which
    // means the default this function returns anyway.
    if(const auto charset = Charset::typeForCodePage(codePageAssumed))
      return *charset;

    return Charset::Type::Latin1;
  }

  // Decodes according to the code page the file declares.
  class CodePageStringHandler : public RIFF::Info::StringHandler
  {
  public:
    explicit CodePageStringHandler(Charset::Type charset) :
      m_charset(charset)
    {
    }

    String parse(const ByteVector &data) const override
    {
      return Charset::decode(data, m_charset);
    }

  private:
    const Charset::Type m_charset;
  };

  const RIFF::Info::StringHandler *stringHandlerForCodePage(unsigned int codePage)
  {
    // 1 handler per code page. They are stateless apart from the type they
    // decode, so they are built once and then shared. Charset::TypeCount of
    // them covers every code page Charset knows.
    static const std::vector<std::unique_ptr<CodePageStringHandler>> handlers = [] {
      std::vector<std::unique_ptr<CodePageStringHandler>> v;
      v.reserve(Charset::TypeCount);
      for(unsigned int i = 0; i < Charset::TypeCount; ++i)
        v.push_back(std::make_unique<CodePageStringHandler>(static_cast<Charset::Type>(i)));
      return v;
    }();

    return handlers[static_cast<unsigned int>(charsetForCodePage(codePage))].get();
  }
} // namespace

class RIFF::Info::Tag::TagPrivate
{
public:
  FieldListMap fieldListMap;

  // The handler that decodes the code page this file declares, or null when the
  // file declares none. It is only a fallback: see effectiveHandler().
  const StringHandler *codePageHandler = nullptr;

  // A handler the user installed with setStringHandler() is an explicit
  // override, so it overrides whatever the file declares.
  const StringHandler *effectiveHandler() const
  {
    if(stringHandler != &defaultStringHandler)
      return stringHandler;
    return codePageHandler ? codePageHandler : stringHandler;
  }
};

class RIFF::Info::StringHandler::StringHandlerPrivate
{
};

////////////////////////////////////////////////////////////////////////////////
// StringHandler implementation
////////////////////////////////////////////////////////////////////////////////

StringHandler::StringHandler() = default;

StringHandler::~StringHandler() = default;

String RIFF::Info::StringHandler::parse(const ByteVector &data) const
{
  return Charset::decode(data, Charset::Type::Latin1);
}

ByteVector RIFF::Info::StringHandler::render(const String &s) const
{
  return Charset::encode(s);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::Info::Tag::Tag(const ByteVector &data) :
  Tag(data, 0)
{
}

RIFF::Info::Tag::Tag(const ByteVector &data, unsigned int codePage) :
  d(std::make_unique<TagPrivate>())
{
  d->codePageHandler = stringHandlerForCodePage(codePage);
  parse(data);
}

RIFF::Info::Tag::Tag() :
  d(std::make_unique<TagPrivate>())
{
}

RIFF::Info::Tag::~Tag() = default;

String RIFF::Info::Tag::title() const
{
  return fieldText("INAM");
}

String RIFF::Info::Tag::artist() const
{
  return fieldText("IART");
}

String RIFF::Info::Tag::album() const
{
  return fieldText("IPRD");
}

String RIFF::Info::Tag::comment() const
{
  return fieldText("ICMT");
}

String RIFF::Info::Tag::genre() const
{
  return fieldText("IGNR");
}

unsigned int RIFF::Info::Tag::year() const
{
  return fieldText("ICRD").substr(0, 4).toInt();
}

unsigned int RIFF::Info::Tag::track() const
{
  return fieldText("IPRT").toInt();
}

void RIFF::Info::Tag::setTitle(const String &s)
{
  setFieldText("INAM", s);
}

void RIFF::Info::Tag::setArtist(const String &s)
{
  setFieldText("IART", s);
}

void RIFF::Info::Tag::setAlbum(const String &s)
{
  setFieldText("IPRD", s);
}

void RIFF::Info::Tag::setComment(const String &s)
{
  setFieldText("ICMT", s);
}

void RIFF::Info::Tag::setGenre(const String &s)
{
  setFieldText("IGNR", s);
}

void RIFF::Info::Tag::setYear(unsigned int i)
{
  if(i != 0)
    setFieldText("ICRD", String::number(i));
  else
    d->fieldListMap.erase("ICRD");
}

void RIFF::Info::Tag::setTrack(unsigned int i)
{
  if(i != 0)
    setFieldText("IPRT", String::number(i));
  else
    d->fieldListMap.erase("IPRT");
}

bool RIFF::Info::Tag::isEmpty() const
{
  return d->fieldListMap.isEmpty();
}

namespace
{
  const Map<ByteVector, String> propertyKeyForId = {
    {"IPRD", "ALBUM"},
    {"IENG", "ARRANGER"},
    {"IART", "ARTIST"},
    {"IBSU", "ARTISTWEBPAGE"},
    {"IBPM", "BPM"},
    {"ICMT", "COMMENT"},
    {"IMUS", "COMPOSER"},
    {"ICOP", "COPYRIGHT"},
    {"ICRD", "DATE"},
    {"PRT1", "DISCSUBTITLE"},
    {"ITCH", "ENCODEDBY"},
    {"ISFT", "ENCODING"},
    {"IDIT", "ENCODINGTIME"},
    {"IGNR", "GENRE"},
    {"ISRC", "ISRC"},
    {"IPUB", "LABEL"},
    {"ILNG", "LANGUAGE"},
    {"IWRI", "LYRICIST"},
    {"IMED", "MEDIA"},
    {"ISTR", "PERFORMER"},
    {"ICNT", "RELEASECOUNTRY"},
    {"IEDT", "REMIXER"},
    {"INAM", "TITLE"},
    {"IPRT", "TRACKNUMBER"}
  };
}  // namespace

PropertyMap RIFF::Info::Tag::properties() const
{
  PropertyMap props;
  for(const auto &[id, val] : std::as_const(d->fieldListMap)) {
    if(String key = propertyKeyForId.value(id); !key.isEmpty()) {
      props[key].append(val);
    }
    else {
      props.addUnsupportedData(key);
    }
  }
  return props;
}

void RIFF::Info::Tag::removeUnsupportedProperties(const StringList &props)
{
  for(const auto &id : props)
    d->fieldListMap.erase(id.data(String::Latin1));
}

PropertyMap RIFF::Info::Tag::setProperties(const PropertyMap &props)
{
  static const Map<String, ByteVector> idForPropertyKey = [] {
    Map<String, ByteVector> map;
    for(const auto &[id, key] : propertyKeyForId) {
      map[key] = id;
    }
    return map;
  }();

  const PropertyMap origProps = properties();
  for(const auto &[key, _] : origProps) {
    if(!props.contains(key) || props.value(key).isEmpty()) {
      d->fieldListMap.erase(idForPropertyKey.value(key));
    }
  }

  PropertyMap ignoredProps;
  for(const auto &[key, val] : props) {
    if(ByteVector id = idForPropertyKey.value(key);
       !id.isEmpty() && !val.isEmpty()) {
      d->fieldListMap[id] = val.front();
    }
    else {
      ignoredProps.insert(key, val);
    }
  }
  return ignoredProps;
}

FieldListMap RIFF::Info::Tag::fieldListMap() const
{
  return d->fieldListMap;
}

String RIFF::Info::Tag::fieldText(const ByteVector &id) const
{
  if(d->fieldListMap.contains(id))
    return String(d->fieldListMap[id]);
  return String();
}

void RIFF::Info::Tag::setFieldText(const ByteVector &id, const String &s)
{
  // id must be a four-byte long pure ascii string.
  if(!isValidChunkName(id))
    return;

  if(!s.isEmpty())
    d->fieldListMap[id] = s;
  else
    removeField(id);
}

void RIFF::Info::Tag::removeField(const ByteVector &id)
{
  if(d->fieldListMap.contains(id))
    d->fieldListMap.erase(id);
}

ByteVector RIFF::Info::Tag::render() const
{
  const StringHandler *handler = d->effectiveHandler();
  ByteVector data("INFO");

  for(const auto &[field, list] : std::as_const(d->fieldListMap)) {
    ByteVector text = handler->render(list);
    if(text.isEmpty())
      continue;

    data.append(field);
    data.append(ByteVector::fromUInt(text.size() + 1, false));
    data.append(text);

    do {
      data.append('\0');
    } while(data.size() & 1);
  }

  if(data.size() == 4)
    return ByteVector();
  return data;
}

void RIFF::Info::Tag::setStringHandler(const StringHandler *handler)
{
  if(handler)
    stringHandler = handler;
  else
    stringHandler = &defaultStringHandler;
}

unsigned int RIFF::Info::Tag::assumedCodePage()
{
  return codePageAssumed;
}

void RIFF::Info::Tag::setAssumedCodePage(unsigned int codePage)
{
  if(codePage != 0 && !Charset::typeForCodePage(codePage)) {
    debug("RIFF::Info::Tag::setAssumedCodePage() - Unsupported code page " +
          String::number(static_cast<int>(codePage)) + ", ignored.");
    return;
  }

  codePageAssumed = codePage;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void RIFF::Info::Tag::parse(const ByteVector &data)
{
  const StringHandler *handler = d->effectiveHandler();
  unsigned int p = 4;
  // Needs a full 8-byte header; fewer bytes left would wrap the check below.
  while(p + 8 <= data.size()) {
    const unsigned int size = data.toUInt(p + 4, false);
    if(size > data.size() - p - 8)
      break;

    if(const ByteVector id = data.mid(p, 4); isValidChunkName(id)) {
      const String text = handler->parse(data.mid(p + 8, size));
      d->fieldListMap[id] = text;
    }

    p += ((size + 1) & ~1) + 8;
  }
}
