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
#include "apefooter.h"

using namespace TagLib;
using namespace APE;

class APE::Tag::TagPrivate
{
public:
  TagPrivate() : file(0), tagOffset(-1), tagLength(0) {}

  File *file;
  long tagOffset;
  long tagLength;

  Footer footer;

  ItemListMap itemListMap;
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

ByteVector APE::Tag::render() const
{
  ByteVector data;
  uint itemCount = 0;

  { Map<const String,Item>::Iterator i = d->itemListMap.begin();
    while (i != d->itemListMap.end()) {
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

  d->footer.setItemCount(itemCount);
  d->footer.setTagSize(data.size()+Footer::size());

  return d->footer.renderHeader() + data + d->footer.renderFooter();
}

ByteVector APE::Tag::fileIdentifier()
{
  return ByteVector::fromCString("APETAGEX");
}

String APE::Tag::title() const
{
  if(d->itemListMap["TITLE"].isEmpty())
    return String::null;
  return d->itemListMap["TITLE"].value.front();
}

String APE::Tag::artist() const
{
  if(d->itemListMap["ARTIST"].isEmpty())
    return String::null;
  return d->itemListMap["ARTIST"].value.front();
}

String APE::Tag::album() const
{
  if(d->itemListMap["ALBUM"].isEmpty())
    return String::null;
  return d->itemListMap["ALBUM"].value.front();
}

String APE::Tag::comment() const
{
  if(d->itemListMap["COMMENT"].isEmpty())
    return String::null;
  return d->itemListMap["COMMENT"].value.front();
}

String APE::Tag::genre() const
{
  if(d->itemListMap["GENRE"].isEmpty())
    return String::null;
  return d->itemListMap["GENRE"].value.front();
}

TagLib::uint APE::Tag::year() const
{
  if(d->itemListMap["YEAR"].isEmpty())
    return 0;
  return d->itemListMap["YEAR"].value.front().toInt();
}

TagLib::uint APE::Tag::track() const
{
  if(d->itemListMap["TRACK"].isEmpty())
    return 0;
  return d->itemListMap["TRACK"].value.front().toInt();
}

void APE::Tag::setTitle(const String &s)
{
  addValue("TITLE", s, true);
}

void APE::Tag::setArtist(const String &s)
{
  addValue("ARTIST", s, true);
}

void APE::Tag::setAlbum(const String &s)
{
  addValue("ALBUM", s, true);
}

void APE::Tag::setComment(const String &s)
{
  addValue("COMMENT", s, true);
}

void APE::Tag::setGenre(const String &s)
{
  addValue("GENRE", s, true);
}

void APE::Tag::setYear(uint i)
{
  if(i <=0 )
    removeItem("YEAR");
  else
    addValue("YEAR", String::number(i), true);
}

void APE::Tag::setTrack(uint i)
{
  if(i <=0 )
    removeItem("TRACK");
  else
    addValue("TRACK", String::number(i), true);
}

APE::Footer* APE::Tag::footer() const
{
  return &d->footer;
}

const APE::ItemListMap& APE::Tag::itemListMap() const
{
  return d->itemListMap;
}

void APE::Tag::removeItem(const String &key) {
  Map<const String, Item>::Iterator it = d->itemListMap.find(key.upper());
  if(it != d->itemListMap.end())
    d->itemListMap.erase(it);
}

void APE::Tag::addValue(const String &key, const String &value, bool replace)
{
  if(replace)
    removeItem(key);
  if(!value.isEmpty()) {
    Map<const String, Item>::Iterator it = d->itemListMap.find(key.upper());
    if (it != d->itemListMap.end())
      d->itemListMap[key].value.append(value);
    else
      setItem(key, Item(value));
  }
}

void APE::Tag::setItem(const String &key, const Item &item)
{
  d->itemListMap.insert(key, item);
}

////////////////////////////////////////////////////////////////////////////////
// protected methods
////////////////////////////////////////////////////////////////////////////////

void APE::Tag::read()
{
  if(d->file && d->file->isValid()) {

    d->file->seek(d->tagOffset);
    d->footer.setData(d->file->readBlock(Footer::size()));

    if(d->footer.tagSize() == 0 || d->footer.tagSize() > d->file->length())
      return;

    d->file->seek(d->tagOffset + Footer::size() - d->footer.tagSize());
    parse(d->file->readBlock(d->footer.tagSize() - Footer::size()), d->footer.itemCount());
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
      d->itemListMap.insert(key, Item(_parse_APEString(val)));
    } else {
      d->binaries.insert(key,data.mid(pos, 8+key.size()+1+vallen));
    }

    pos += 8 + key.size() + 1 + vallen;
    count--;
  }
}
