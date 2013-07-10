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

#include <tdebug.h>
#include <tfile.h>

#include "infotag.h"
#include "id3v1tag.h"

using namespace TagLib;
using namespace RIFF::Info;

namespace 
{
  const RIFF::Info::StringHandler defaultStringHandler;
  const TagLib::StringHandler *stringHandler = &defaultStringHandler;

  bool isValidChunkID(const ByteVector &id)
  {
    // id should consist of 4 ASCII chars.

    if(id.size() != 4)
      return false;

    for(size_t i = 0; i < 4; i++) {
      const uchar c = static_cast<uchar>(id[i]);
      if(c < 32 || 127 < c)
        return false;
    }

    return true;
  }
}

class RIFF::Info::Tag::TagPrivate
{
public:
  FieldMap fields;
};

////////////////////////////////////////////////////////////////////////////////
// StringHandler implementation
////////////////////////////////////////////////////////////////////////////////

RIFF::Info::StringHandler::StringHandler()
{
}

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

RIFF::Info::Tag::Tag(const ByteVector &data) 
  : TagLib::Tag()
  , d(new TagPrivate())
{
  parse(data);
}

RIFF::Info::Tag::Tag() 
  : TagLib::Tag()
  , d(new TagPrivate())
{
}

RIFF::Info::Tag::~Tag()
{
  delete d;
}

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

TagLib::uint RIFF::Info::Tag::year() const
{
  return fieldText("ICRD").substr(0, 4).toInt();
}

TagLib::uint RIFF::Info::Tag::track() const
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

void RIFF::Info::Tag::setYear(uint i)
{
  if(i != 0) {
    String year = "000" + String::number(i);
    year = year.substr(year.length() - 4, 4);

    String date = fieldText("ICRD");
    if(date.length() >= 4) {
      date[0] = year[0];
      date[1] = year[1];
      date[2] = year[2];
      date[3] = year[3];
    }
    else {
      date = year;
    }

    setFieldText("ICRD", date);
  }
  else {
    removeField("ICRD");
  }
}

void RIFF::Info::Tag::setTrack(uint i)
{
  if(i != 0)
    setFieldText("IPRT", String::number(i));
  else
    removeField("IPRT");
}

bool RIFF::Info::Tag::isEmpty() const
{
  return d->fields.isEmpty();
}

RIFF::Info::FieldMap RIFF::Info::Tag::fieldMap() const
{
  return d->fields;
}

String RIFF::Info::Tag::fieldText(const ByteVector &id) const
{
  if(d->fields.contains(id))
    return d->fields[id];
  else
    return String::null;
}

void RIFF::Info::Tag::setFieldText(const ByteVector &id, const String &text)
{
  if(!isValidChunkID(id)) {
    debug("RIFF::Info::Tag::setFieldText() - Invalid field ID '" + String(id) + "'.");
    return;
  }

  if(!text.isEmpty())
    d->fields[id] = text;
  else
    removeField(id);
}

void RIFF::Info::Tag::removeField(const ByteVector &id)
{
  if(d->fields.contains(id))
    d->fields.erase(id);
}

ByteVector RIFF::Info::Tag::render(bool createIID3Field) const
{
  if(d->fields.isEmpty())
    return ByteVector::null;

  ByteVector data("INFO");
  data.reserve(1024);

  for(FieldMap::ConstIterator it = d->fields.begin(); it != d->fields.end(); ++it) {
    ByteVector text = stringHandler->render(it->second);
    if(text.isEmpty())
      continue;

    data.append(it->first);
    data.append(ByteVector::fromUInt32LE(text.size() + 1));
    data.append(text);
    
    do {
      data.append('\0');
    } while(data.size() & 1);
  }

  if(createIID3Field) {
    ID3v1::Tag v1tag;
    v1tag.setTitle(title());
    v1tag.setArtist(artist());
    v1tag.setAlbum(album());
    v1tag.setYear(year());
    v1tag.setComment(comment());
    v1tag.setTrack(track());
    v1tag.setGenre(genre());

    data.append("IID3");
    data.append(ByteVector::fromUInt32LE(128));
    data.append(v1tag.render());
  }

  if(data.size() == 4)
    return ByteVector::null;
  else
    return data;
}

void RIFF::Info::Tag::setStringHandler(const TagLib::StringHandler *handler)
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
  size_t p = 4;
  while(p < data.size() - 8) 
  {
    const uint size = data.toUInt32LE(p + 4);
    const ByteVector id = data.mid(p, 4);
    if(isValidChunkID(id)) {
      // "IID3" field is ignored here, but will be restored when the tag is saved. 
      if(id != "IID3")
        d->fields[id] = stringHandler->parse(data.mid(p + 8, size));
    }
    else { 
      debug("RIFF::Info::Tag::parse() - Skipped an invalid field '" + String(id) + "'.");
    }

    p += 8 + ((size + 1) & ~1);
  }
}

