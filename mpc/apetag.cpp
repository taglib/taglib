/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.com
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#include <tdebug.h>
#include <tfile.h>
#include <tstring.h>
#include <tmap.h>

#include "apetag.h"

using namespace TagLib;
using namespace APE;

class APE::Tag::TagPrivate
{
public:
  TagPrivate() : file(0), tagOffset(-1), tagLength(0) {}

  File *file;
  long tagOffset;
  long tagLength;

  FieldListMap fieldListMap;
  Map<const String, ByteVector> binaries;
};

APE::Item::Item(const String& str) : readOnly(false), locator(false) {
  value.append(str);
}

APE::Item::Item(const StringList& values) : readOnly(false), locator(false) {
  value.append(values);
}

bool APE::Item::isEmpty() const
{
  return value.isEmpty();
}

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

APE::Tag::Tag() : TagLib::Tag()
{
  d = new TagPrivate;
}

APE::Tag::Tag(File *file, long tagOffset) : TagLib::Tag()
{
  d = new TagPrivate;
  d->file = file;
  d->tagOffset = tagOffset;

  read();
}

APE::Tag::~Tag()
{
  delete d;
}

using TagLib::uint;

static ByteVector _render_APEItem(String key, Item item)
{
  ByteVector data;
  uint flags = ((item.readOnly) ? 1 : 0) | ((item.locator) ? 2 : 0);
  ByteVector value;

  if (item.value.isEmpty()) return data;

  StringList::Iterator it = item.value.begin();
  value.append(it->data(String::UTF8));
  it++;
  while (it != item.value.end()) {
    value.append('\0');
    value.append(it->data(String::UTF8));
    it++;
  }

  data.append(ByteVector::fromUInt(value.size(), false));
  data.append(ByteVector::fromUInt(flags, false));
  data.append(key.data(String::UTF8));
  data.append(ByteVector('\0'));
  data.append(value);

  return data;
}

static ByteVector _render_APEFrame(bool isHeader, uint dataSize, uint itemCount) {
  ByteVector header;
  TagLib::uint tagSize = 32 + dataSize;
  // bit 31: Has a header
  // bit 29: Is the header
  TagLib::uint flags = (1U << 31) | ((isHeader) ? (1U << 29) : 0);

  header.append(APE::Tag::fileIdentifier());
  header.append(ByteVector::fromUInt(2, false));
  header.append(ByteVector::fromUInt(tagSize, false));
  header.append(ByteVector::fromUInt(itemCount, false));
  header.append(ByteVector::fromUInt(flags, false));
  header.append(ByteVector::fromLongLong(0, false));

  return header;
}

ByteVector APE::Tag::render() const
{
  ByteVector data;
  uint itemCount = 0;

  { Map<const String,Item>::Iterator i = d->fieldListMap.begin();
    while (i != d->fieldListMap.end()) {
      if (!i->second.value.isEmpty()) {
        data.append(_render_APEItem(i->first, i->second));
        itemCount++;
      }
      i++;
    }
  }

  { Map<String,ByteVector>::Iterator i = d->binaries.begin();
    while (i != d->binaries.end()) {
      if (!i->second.isEmpty()) {
          data.append(i->second);
          itemCount++;
      }
      i++;
    }
  }

  ByteVector tag;
  tag.append(_render_APEFrame(true, data.size(), itemCount));
  tag.append(data);
  tag.append(_render_APEFrame(false, data.size(), itemCount));

  return tag;
}

ByteVector APE::Tag::fileIdentifier()
{
  return ByteVector::fromCString("APETAGEX");
}

String APE::Tag::title() const
{
  if(d->fieldListMap["TITLE"].isEmpty())
    return String::null;
  return d->fieldListMap["TITLE"].value.front();
}

String APE::Tag::artist() const
{
  if(d->fieldListMap["ARTIST"].isEmpty())
    return String::null;
  return d->fieldListMap["ARTIST"].value.front();
}

String APE::Tag::album() const
{
  if(d->fieldListMap["ALBUM"].isEmpty())
    return String::null;
  return d->fieldListMap["ALBUM"].value.front();
}

String APE::Tag::comment() const
{
  if(d->fieldListMap["COMMENT"].isEmpty())
    return String::null;
  return d->fieldListMap["COMMENT"].value.front();
}

String APE::Tag::genre() const
{
  if(d->fieldListMap["GENRE"].isEmpty())
    return String::null;
  return d->fieldListMap["GENRE"].value.front();
}

TagLib::uint APE::Tag::year() const
{
  if(d->fieldListMap["YEAR"].isEmpty())
    return 0;
  return d->fieldListMap["YEAR"].value.front().toInt();
}

TagLib::uint APE::Tag::track() const
{
  if(d->fieldListMap["TRACK"].isEmpty())
    return 0;
  return d->fieldListMap["TRACK"].value.front().toInt();
}

void APE::Tag::setTitle(const String &s)
{
  addField("TITLE", s, true);
}

void APE::Tag::setArtist(const String &s)
{
  addField("ARTIST", s, true);
}

void APE::Tag::setAlbum(const String &s)
{
  addField("ALBUM", s, true);
}

void APE::Tag::setComment(const String &s)
{
  addField("COMMENT", s, true);
}

void APE::Tag::setGenre(const String &s)
{
  addField("GENRE", s, true);
}

void APE::Tag::setYear(uint i)
{
  if(i <=0 )
    removeField("YEAR");
  else
    addField("YEAR", String::number(i), true);
}

void APE::Tag::setTrack(uint i)
{
  if(i <=0 )
    removeField("TRACK");
  else
    addField("TRACK", String::number(i), true);
}

const APE::FieldListMap& APE::Tag::fieldListMap() const
{
  return d->fieldListMap;
}

void APE::Tag::removeField(const String &key) {
  Map<const String, Item>::Iterator it = d->fieldListMap.find(key.upper());
  if(it != d->fieldListMap.end())
    d->fieldListMap.erase(it);
}

void APE::Tag::addField(const String &key, const String &value, bool replace) {
  if(replace)
    removeField(key);
  if(!value.isEmpty()) {
    Map<const String, Item>::Iterator it = d->fieldListMap.find(key.upper());
    if (it != d->fieldListMap.end())
      d->fieldListMap[key].value.append(value);
    else
      addItem(key, Item(value));
  }
}

void APE::Tag::addField(const String &key, const StringList &values) {
  removeField(key);

  if(values.isEmpty()) return;
  else {
    addItem(key, Item(values));
  }
}

TagLib::uint APE::Tag::tagSize(const ByteVector &footer)
{
  // The reported length (excl. header)

  uint length = footer.mid(12, 4).toUInt(false);

  // Flags (bit 31: tag contains a header)

  uint flags = footer.mid(20, 4).toUInt(false);

  return length + ((flags & (1U << 31)) ? 32 : 0);
}

////////////////////////////////////////////////////////////////////////////////
// protected methods
////////////////////////////////////////////////////////////////////////////////

void APE::Tag::addItem(const String &key, const Item &item) {
  removeField(key);

  Map<const String, Item>::Iterator it = d->fieldListMap.find(key.upper());
  d->fieldListMap.insert(key, item);
}

void APE::Tag::read()
{
  if(d->file && d->file->isValid()) {
    d->file->seek(d->tagOffset);
    // read the footer -- always 32 bytes
    ByteVector footer = d->file->readBlock(32);

    // parse footer and some initial sanity checking
    if(footer.size() == 32 && footer.mid(0, 8) == "APETAGEX") {
      uint length = footer.mid(12, 4).toUInt(false);
      uint count = footer.mid(16, 4).toUInt(false);
      d->tagLength = length;
      d->file->seek(d->tagOffset + 32 - length);
      ByteVector data = d->file->readBlock(length - 32);
      parse(data, count);
    }
    else
      debug("APE tag is not valid or could not be read at the specified offset.");
  }
}

static StringList _parse_APEString(ByteVector val)
{
    StringList value;
    int pold = 0;
    int p = val.find('\0');
    while (p != -1) {
        value.append(String(val.mid(pold, p), String::UTF8));
        pold = p+1;
        p = val.find('\0', pold);
    };
    value.append(String(val.mid(pold), String::UTF8));

    return value;
}

void APE::Tag::parse(const ByteVector &data, uint count)
{
  uint pos = 0;
  uint vallen, flags;
  String key, value;
  while(count > 0) {
    vallen = data.mid(pos+0,4).toUInt(false);
    flags = data.mid(pos+4,4).toUInt(false);
    key = String(data.mid(pos+8), String::UTF8);
    key = key.upper();
    APE::Item item;

    if (flags < 4 ) {
      ByteVector val = data.mid(pos+8+key.size()+1, vallen);
      d->fieldListMap.insert(key, Item(_parse_APEString(val)));
    } else {
      d->binaries.insert(key,data.mid(pos, 8+key.size()+1+vallen));
    }

    pos += 8 + key.size() + 1 + vallen;
    count--;
  }
}
