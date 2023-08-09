/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.com
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

#if defined(__SUNPRO_CC) && (__SUNPRO_CC < 0x5130)
// Sun Studio finds multiple specializations of Map because
// it considers specializations with and without class types
// to be different; this define forces Map to use only the
// specialization with the class keyword.
#define WANT_CLASS_INSTANTIATION_OF_MAP (1)
#endif

#include "apetag.h"

#include "tfile.h"
#include "tstring.h"
#include "tmap.h"
#include "tpropertymap.h"
#include "tdebug.h"
#include "tutils.h"

#include "apefooter.h"
#include "apeitem.h"

#include <array>

using namespace TagLib;
using namespace APE;

namespace
{
  const unsigned int MinKeyLength = 2;
  const unsigned int MaxKeyLength = 255;

  bool isKeyValid(const ByteVector &key)
  {
    static constexpr std::array invalidKeys { "ID3", "TAG", "OGGS", "MP+" };

    // only allow printable ASCII including space (32..126)

    for(auto it = key.begin(); it != key.end(); ++it) {
      const int c = static_cast<unsigned char>(*it);
      if(c < 32 || c > 126)
        return false;
    }

    return std::none_of(invalidKeys.begin(), invalidKeys.end(), [upperKey = String(key).upper()](auto k) { return upperKey == k; });
  }
}  // namespace

class APE::Tag::TagPrivate
{
public:
  TagPrivate() :
    file(nullptr),
    footerLocation(0) {}

  File *file;
  offset_t footerLocation;

  Footer footer;
  ItemListMap itemListMap;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

APE::Tag::Tag() :
  d(std::make_unique<TagPrivate>())
{
}

APE::Tag::Tag(TagLib::File *file, offset_t footerLocation) :
  d(std::make_unique<TagPrivate>())
{
  d->file = file;
  d->footerLocation = footerLocation;

  read();
}

APE::Tag::~Tag() = default;

ByteVector APE::Tag::fileIdentifier()
{
  return ByteVector::fromCString("APETAGEX");
}

String APE::Tag::title() const
{
  Item value = d->itemListMap.value("TITLE");
  return value.isEmpty() ? String() : value.values().toString();
}

String APE::Tag::artist() const
{
  Item value = d->itemListMap.value("ARTIST");
  return value.isEmpty() ? String() : value.values().toString();
}

String APE::Tag::album() const
{
  Item value = d->itemListMap.value("ALBUM");
  return value.isEmpty() ? String() : value.values().toString();
}

String APE::Tag::comment() const
{
  Item value = d->itemListMap.value("COMMENT");
  return value.isEmpty() ? String() : value.values().toString();
}

String APE::Tag::genre() const
{
  Item value = d->itemListMap.value("GENRE");
  return value.isEmpty() ? String() : value.values().toString();
}

unsigned int APE::Tag::year() const
{
  Item value = d->itemListMap.value("YEAR");
  return value.isEmpty() ? 0 : value.toString().toInt();
}

unsigned int APE::Tag::track() const
{
  Item value = d->itemListMap.value("TRACK");
  return value.isEmpty() ? 0 : value.toString().toInt();
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

void APE::Tag::setYear(unsigned int i)
{
  if(i == 0)
    removeItem("YEAR");
  else
    addValue("YEAR", String::number(i), true);
}

void APE::Tag::setTrack(unsigned int i)
{
  if(i == 0)
    removeItem("TRACK");
  else
    addValue("TRACK", String::number(i), true);
}

namespace
{
  // conversions of tag keys between what we use in PropertyMap and what's usual
  // for APE tags
  //                usual,         APE
  constexpr std::array keyConversions {
    std::pair("TRACKNUMBER", "TRACK"),
    std::pair("DATE", "YEAR"),
    std::pair("ALBUMARTIST", "ALBUM ARTIST"),
    std::pair("DISCNUMBER", "DISC"),
    std::pair("REMIXER", "MIXARTIST"),
    std::pair("RELEASESTATUS", "MUSICBRAINZ_ALBUMSTATUS"),
    std::pair("RELEASETYPE", "MUSICBRAINZ_ALBUMTYPE"),
  };
}  // namespace

PropertyMap APE::Tag::properties() const
{
  PropertyMap properties;
  const auto &items = itemListMap();
  for(auto it = items.begin(); it != items.end(); ++it) {
    String tagName = it->first.upper();
    // if the item is Binary or Locator, or if the key is an invalid string,
    // add to unsupportedData
    if(it->second.type() != Item::Text || tagName.isEmpty()) {
      properties.unsupportedData().append(it->first);
    }
    else {
      // Some tags need to be handled specially
      for(const auto &[k, t] : keyConversions) {
        if(tagName == t)
          tagName = k;
      }
      properties[tagName].append(it->second.toStringList());
    }
  }
  return properties;
}

void APE::Tag::removeUnsupportedProperties(const StringList &properties)
{
  for(auto it = properties.begin(); it != properties.end(); ++it)
    removeItem(*it);
}

PropertyMap APE::Tag::setProperties(const PropertyMap &origProps)
{
  PropertyMap properties(origProps); // make a local copy that can be modified

  // see comment in properties()
  for(const auto &[k, t] : keyConversions)
    if(properties.contains(k)) {
      properties.insert(t, properties[k]);
      properties.erase(k);
    }

  // first check if tags need to be removed completely
  StringList toRemove;
  const auto &items = itemListMap();
  for(auto remIt = items.begin(); remIt != items.end(); ++remIt) {
    String key = remIt->first.upper();
    // only remove if a) key is valid, b) type is text, c) key not contained in new properties
    if(!key.isEmpty() && remIt->second.type() == APE::Item::Text && !properties.contains(key))
      toRemove.append(remIt->first);
  }

  for(auto removeIt = toRemove.cbegin(); removeIt != toRemove.cend(); removeIt++)
    removeItem(*removeIt);

  // now sync in the "forward direction"
  PropertyMap invalid;
  for(auto it = properties.begin(); it != properties.cend(); ++it) {
    const String &tagName = it->first;
    if(!checkKey(tagName))
      invalid.insert(it->first, it->second);
    else if(!(itemListMap().contains(tagName)) || !(itemListMap()[tagName].values() == it->second)) {
      if(it->second.isEmpty())
        removeItem(tagName);
      else {
        auto valueIt = it->second.begin();
        addValue(tagName, *valueIt, true);
        ++valueIt;
        for(; valueIt != it->second.end(); ++valueIt)
          addValue(tagName, *valueIt, false);
      }
    }
  }
  return invalid;
}

bool APE::Tag::checkKey(const String &key)
{
  if(key.size() < MinKeyLength || key.size() > MaxKeyLength)
    return false;

  return isKeyValid(key.data(String::UTF8));
}

APE::Footer *APE::Tag::footer() const
{
  return &d->footer;
}

const APE::ItemListMap& APE::Tag::itemListMap() const
{
  return d->itemListMap;
}

void APE::Tag::removeItem(const String &key)
{
  d->itemListMap.erase(key.upper());
}

void APE::Tag::addValue(const String &key, const String &value, bool replace)
{
  if(replace)
    removeItem(key);

  if(value.isEmpty())
    return;

  // Text items may contain more than one value.
  // Binary or locator items may have only one value, hence always replaced.

  auto it = d->itemListMap.find(key.upper());

  if(it != d->itemListMap.end() && it->second.type() == Item::Text)
    it->second.appendValue(value);
  else
    setItem(key, Item(key, value));
}

void APE::Tag::setData(const String &key, const ByteVector &value)
{
  removeItem(key);

  if(value.isEmpty())
    return;

  setItem(key, Item(key, value, true));
}

void APE::Tag::setItem(const String &key, const Item &item)
{
  if(!checkKey(key)) {
    debug("APE::Tag::setItem() - Couldn't set an item due to an invalid key.");
    return;
  }

  d->itemListMap[key.upper()] = item;
}

bool APE::Tag::isEmpty() const
{
  return d->itemListMap.isEmpty();
}

////////////////////////////////////////////////////////////////////////////////
// protected methods
////////////////////////////////////////////////////////////////////////////////

void APE::Tag::read()
{
  if(d->file && d->file->isValid()) {

    d->file->seek(d->footerLocation);
    d->footer.setData(d->file->readBlock(Footer::size()));

    if(d->footer.tagSize() <= Footer::size() ||
       d->footer.tagSize() > static_cast<unsigned long>(d->file->length()))
      return;

    d->file->seek(d->footerLocation + Footer::size() - d->footer.tagSize());
    parse(d->file->readBlock(d->footer.tagSize() - Footer::size()));
  }
}

ByteVector APE::Tag::render() const
{
  ByteVector data;
  unsigned int itemCount = 0;

  for(auto it = d->itemListMap.cbegin(); it != d->itemListMap.cend(); ++it) {
    data.append(it->second.render());
    itemCount++;
  }

  d->footer.setItemCount(itemCount);
  d->footer.setTagSize(data.size() + Footer::size());
  d->footer.setHeaderPresent(true);

  return d->footer.renderHeader() + data + d->footer.renderFooter();
}

void APE::Tag::parse(const ByteVector &data)
{
  // 11 bytes is the minimum size for an APE item

  if(data.size() < 11)
    return;

  unsigned int pos = 0;

  for(unsigned int i = 0; i < d->footer.itemCount() && pos <= data.size() - 11; i++) {

    const int nullPos = data.find('\0', pos + 8);
    if(nullPos < 0) {
      debug("APE::Tag::parse() - Couldn't find a key/value separator. Stopped parsing.");
      return;
    }

    const unsigned int keyLength = nullPos - pos - 8;
    const unsigned int valLength = data.toUInt(pos, false);

    if(valLength >= data.size() || pos > data.size() - valLength) {
      debug("APE::Tag::parse() - Invalid val length. Stopped parsing.");
      return;
    }

    if(keyLength >= MinKeyLength
      && keyLength <= MaxKeyLength
      && isKeyValid(data.mid(pos + 8, keyLength)))
    {
      APE::Item item;
      item.parse(data.mid(pos));

      d->itemListMap.insert(item.key().upper(), item);
    }
    else {
      debug("APE::Tag::parse() - Skipped an item due to an invalid key.");
    }

    pos += keyLength + valLength + 9;
  }
}
