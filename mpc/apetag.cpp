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

  Map<const String, String> items;
  Map<const String, ByteVector> unknowns;

};
/*
struct APE::Tag::Item
{
  Item(String key) : key(key), type(STRING), value.str(String::null), readOnly(false) {};
  const String key;
  enum Type{ STRING, BINARY, URL, RESERVED } type;
  union value{
    String str;
    ByteVector bin;
  }
  bool readOnly;
}*/


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

static ByteVector APEItem(String key, String value) {
  ByteVector data;
  uint flags = 0;

  data.append(ByteVector::fromUInt(value.size(),false));
  data.append(ByteVector::fromUInt(flags,false));
  data.append(key.data(String::UTF8));
  data.append(char(0));
  data.append(value.data(String::UTF8));

  return data;
}

static ByteVector APEFrame(bool isHeader, uint dataSize, uint itemCount) {
  ByteVector header;
  uint tagSize = 32 + dataSize;
  // bit 31: Has a header
  // bit 29: Is the header
  uint flags = (1U<<31) | ((isHeader) ? (1U<<29) : 0);

  header.append(APE::Tag::fileIdentifier());
  header.append(ByteVector::fromUInt(2,false));
  header.append(ByteVector::fromUInt(tagSize,false));
  header.append(ByteVector::fromUInt(itemCount,false));
  header.append(ByteVector::fromUInt(flags,false));
  header.append(ByteVector::fromLongLong(0,false));

  return header;
}

ByteVector APE::Tag::render() const
{
  ByteVector data;
  uint itemCount = 0;

  { Map<String,String>::Iterator i = d->items.begin();
    while (i != d->items.end()) {
      if (!i->second.isEmpty()) {
        data.append(APEItem(i->first, i->second));
        itemCount++;
      }
      i++;
    }
  }

  { Map<String,ByteVector>::Iterator i = d->unknowns.begin();
    while (i != d->unknown.end()) {
      if (!i->second.isEmpty()) {
          data.append(i->second);
          itemCount++;
      }
      i++;
    }
  }

  ByteVector tag;
  tag.append(APEFrame(true, data.size(), itemCount));
  tag.append(data);
  tag.append(APEFrame(false, data.size(), itemCount));

  return tag;
}

ByteVector APE::Tag::fileIdentifier()
{
  return ByteVector::fromCString("APETAGEX");
}

String APE::Tag::title() const
{
  if (d->items.contains("Title"))
    return d->items["Title"];
  else
    return String::null;
}

String APE::Tag::artist() const
{
  if (d->items.contains("Artist"))
    return d->items["Artist"];
  else
    return String::null;
}

String APE::Tag::album() const
{
  if (d->items.contains("Album"))
    return d->items["Album"];
  else
    return String::null;
}

String APE::Tag::comment() const
{
  if (d->items.contains("Comment"))
    return d->items["Comment"];
  else
    return String::null;
}

String APE::Tag::genre() const
{
  if (d->items.contains("Genre"))
    return d->items["Genre"];
  else
    return String::null;
}

TagLib::uint APE::Tag::year() const
{
  if (d->items.contains("Year"))
    return (d->items["Year"]).toInt();
  return 0;
}

TagLib::uint APE::Tag::track() const
{
  if (d->items.contains("Track"))
    return (d->items["Track"]).toInt();
  return 0;
}

void APE::Tag::setTitle(const String &s)
{
  d->items["Title"] = s;
}

void APE::Tag::setArtist(const String &s)
{
  d->items["Artist"] = s;
}

void APE::Tag::setAlbum(const String &s)
{
  d->items["Album"] = s;
}

void APE::Tag::setComment(const String &s)
{
  if(s.isEmpty() )
    removeComment("Comment");
  else
    d->items["Comment"] = s;
}

void APE::Tag::setGenre(const String &s)
{
  if(s.isEmpty())
    removeComment("Genre");
  else
    d->items["Genre"] = s;
}

void APE::Tag::setYear(uint i)
{
  if(i <=0 )
    removeComment("Year");
  else
    d->items["Year"] = String::number(i);
}

void APE::Tag::setTrack(uint i)
{
  if(i <=0 )
    removeComment("Track");
  else
    d->items["Track"] = String::number(i);
}

void APE::Tag::removeComment(const String &key) {
  Map<String,String>::Iterator it = d->items.find(key);
  if (it != d->items.end())
    d->items.erase(it);
}

void APE::Tag::addComment(const String &key, const String &value) {
  if (value.isEmpty())
    removeComment(key);
  else
    d->items[key] = value;
}

uint APE::Tag::tagSize(ByteVector footer) {

    // The reported length (excl. header)

    uint length = footer.mid(12,4).toUInt(false);

    // Flags (bit 31: tag contains a header)

    uint flags = footer.mid(20,4).toUInt(false);

    return length + (flags & (1U<<31) ? 32 : 0);

}

////////////////////////////////////////////////////////////////////////////////
// protected methods
////////////////////////////////////////////////////////////////////////////////

void APE::Tag::read()
{
  if(d->file && d->file->isValid()) {
    d->file->seek(d->tagOffset);
    // read the footer -- always 32 bytes
    ByteVector footer = d->file->readBlock(32);

    // parse footer and some initial sanity checking
    if(footer.size() == 32 && footer.mid(0, 8) == "APETAGEX") {
      uint length = footer.mid(12,4).toUInt(false);
      uint count = footer.mid(16,4).toUInt(false);
      d->tagLength = length;
      d->file->seek(d->tagOffset + 32 -length);
      ByteVector data = d->file->readBlock(length-32);
      parse(data,count);
    }
    else
      debug("APE tag is not valid or could not be read at the specified offset.");
  }
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

        if (flags == 0) {
          value = String(data.mid(pos+8+key.size()+1, vallen), String::UTF8);
          d->items.insert(key,value);
        } else {
          d->unknown.insert(data.mid(pos, 8+key.size()+1+vallen));
        }

        pos += 8+key.size()+1+vallen;
        count--;
    }
}
/*
void APE::Tag::parse(const ByteVector &data, uint count)
{
    uint pos = 0;
    uint vallen, flags;
    String key;
    while(count > 0) {
      vallen = data.mid(pos+0,4).toUInt(false);
      flags = data.mid(pos+4,4).toUInt(false);
      key = String(data.mid(pos+8), String::UTF8);
      Item item(key);

      ByteVector value = data.mid(pos+8+key.size()+1, vallen);

      switch ((flags >> 1) & 3) {
        case 0:
          item.value.str = String(value, String::UTF8);
          item.type = Item::STRING;
          break;
        case 1:
          item.value.bin = value;
          item.type = Item::BINARY;
          break;
        case 2:
          item.value.str = String(value, String::UTF8);
          item.type = Item::URL;
          break;
        case 3:
          item.value.bin = value;
          item.type = Item::RESERVED;
          break;
      }
      item.readOnly = (flags & 1);

      d->items.insert(key,item);

      pos += 8+key.size()+1+vallen;
      count--;
    }
}*/