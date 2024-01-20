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

#include <utility>

#include "tbytevector.h"
#include "tpropertymap.h"
#include "riffutils.h"

using namespace TagLib;
using namespace RIFF::Info;

namespace
{
  const RIFF::Info::StringHandler defaultStringHandler;
  const RIFF::Info::StringHandler *stringHandler = &defaultStringHandler;
} // namespace

class RIFF::Info::Tag::TagPrivate
{
public:
  FieldListMap fieldListMap;
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
  return String(data, String::UTF8);
}

ByteVector RIFF::Info::StringHandler::render(const String &s) const
{
  return s.data(String::UTF8);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::Info::Tag::Tag(const ByteVector &data) :
  d(std::make_unique<TagPrivate>())
{
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
  static Map<String, ByteVector> idForPropertyKey;
  if(idForPropertyKey.isEmpty()) {
    for(const auto &[id, key] : propertyKeyForId) {
      idForPropertyKey[key] = id;
    }
  }

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
  ByteVector data("INFO");

  for(const auto &[field, list] : std::as_const(d->fieldListMap)) {
    ByteVector text = stringHandler->render(list);
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

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void RIFF::Info::Tag::parse(const ByteVector &data)
{
  unsigned int p = 4;
  while(p < data.size()) {
    const unsigned int size = data.toUInt(p + 4, false);
    if(size > data.size() - p - 8)
      break;

    if(const ByteVector id = data.mid(p, 4); isValidChunkName(id)) {
      const String text = stringHandler->parse(data.mid(p + 8, size));
      d->fieldListMap[id] = text;
    }

    p += ((size + 1) & ~1) + 8;
  }
}
