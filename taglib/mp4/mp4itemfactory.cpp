/***************************************************************************
    copyright            : (C) 2023 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

#include "mp4itemfactory.h"

#include <utility>

#include "tbytevector.h"
#include "tdebug.h"

#include "id3v1genres.h"

using namespace TagLib;
using namespace MP4;

namespace {

constexpr char freeFormPrefix[] = "----:com.apple.iTunes:";

}  // namespace

class ItemFactory::ItemFactoryPrivate
{
public:
  NameHandlerMap handlerTypeForName;
  Map<ByteVector, String> propertyKeyForName;
  Map<String, ByteVector> nameForPropertyKey;
};

ItemFactory ItemFactory::factory;

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ItemFactory *ItemFactory::instance()
{
  return &factory;
}

std::pair<String, Item> ItemFactory::parseItem(
  const Atom *atom, const ByteVector &data) const
{
  switch(handlerTypeForName(atom->name())) {
  case ItemHandlerType::Unknown:
    break;
  case ItemHandlerType::FreeForm:
    return parseFreeForm(atom, data);
  case ItemHandlerType::IntPair:
  case ItemHandlerType::IntPairNoTrailing:
    return parseIntPair(atom, data);
  case ItemHandlerType::Bool:
    return parseBool(atom, data);
  case ItemHandlerType::Int:
    return parseInt(atom, data);
  case ItemHandlerType::TextOrInt:
    return parseTextOrInt(atom, data);
  case ItemHandlerType::UInt:
    return parseUInt(atom, data);
  case ItemHandlerType::LongLong:
    return parseLongLong(atom, data);
  case ItemHandlerType::Byte:
    return parseByte(atom, data);
  case ItemHandlerType::Gnre:
    return parseGnre(atom, data);
  case ItemHandlerType::Covr:
    return parseCovr(atom, data);
  case ItemHandlerType::TextImplicit:
    return parseText(atom, data, -1);
  case ItemHandlerType::Text:
    return parseText(atom, data);
  }
  return {atom->name(), Item()};
}

ByteVector ItemFactory::renderItem(
  const String &itemName, const Item &item) const
{
  if(itemName.startsWith("----")) {
    return renderFreeForm(itemName, item);
  }
  const ByteVector name = itemName.data(String::Latin1);
  switch(handlerTypeForName(name)) {
  case ItemHandlerType::Unknown:
    debug("MP4: Unknown item name \"" + name + "\"");
    break;
  case ItemHandlerType::FreeForm:
    return renderFreeForm(name, item);
  case ItemHandlerType::IntPair:
    return renderIntPair(name, item);
  case ItemHandlerType::IntPairNoTrailing:
    return renderIntPairNoTrailing(name, item);
  case ItemHandlerType::Bool:
    return renderBool(name, item);
  case ItemHandlerType::Int:
    return renderInt(name, item);
  case ItemHandlerType::TextOrInt:
    return renderTextOrInt(name, item);
  case ItemHandlerType::UInt:
    return renderUInt(name, item);
  case ItemHandlerType::LongLong:
    return renderLongLong(name, item);
  case ItemHandlerType::Byte:
    return renderByte(name, item);
  case ItemHandlerType::Gnre:
    return renderInt(name, item);
  case ItemHandlerType::Covr:
    return renderCovr(name, item);
  case ItemHandlerType::TextImplicit:
    return renderText(name, item, TypeImplicit);
  case ItemHandlerType::Text:
    return renderText(name, item);
  }
  return ByteVector();
}

std::pair<ByteVector, Item> ItemFactory::itemFromProperty(
  const String &key, const StringList &values) const
{
  ByteVector name = nameForPropertyKey(key);
  if(!name.isEmpty()) {
    if(values.isEmpty()) {
      return {name, values};
    }
    switch(name.startsWith("----")
           ? ItemHandlerType::FreeForm
           : handlerTypeForName(name)) {
    case ItemHandlerType::IntPair:
    case ItemHandlerType::IntPairNoTrailing:
      if(StringList parts = StringList::split(values.front(), "/");
         !parts.isEmpty()) {
        int first = parts[0].toInt();
        int second = 0;
        if(parts.size() > 1) {
          second = parts[1].toInt();
        }
        return {name, Item(first, second)};
      }
      break;
    case ItemHandlerType::Int:
    case ItemHandlerType::Gnre:
      return {name, Item(values.front().toInt())};
    case ItemHandlerType::UInt:
      return {name, Item(static_cast<unsigned int>(values.front().toInt()))};
    case ItemHandlerType::LongLong:
      return {name, Item(static_cast<long long>(values.front().toInt()))};
    case ItemHandlerType::Byte:
      return {name, Item(static_cast<unsigned char>(values.front().toInt()))};
    case ItemHandlerType::Bool:
      return {name, Item(values.front().toInt() != 0)};
    case ItemHandlerType::FreeForm:
    case ItemHandlerType::TextOrInt:
    case ItemHandlerType::TextImplicit:
    case ItemHandlerType::Text:
      return {name, values};

    case ItemHandlerType::Covr:
      debug("MP4: Invalid item \"" + name + "\" for property");
      break;
    case ItemHandlerType::Unknown:
      debug("MP4: Unknown item name \"" + name + "\" for property");
      break;
    }
  }
  return {name, Item()};
}

std::pair<String, StringList> ItemFactory::itemToProperty(
  const ByteVector &itemName, const Item &item) const
{
  if(const String key = propertyKeyForName(itemName); !key.isEmpty()) {
    switch(itemName.startsWith("----")
           ? ItemHandlerType::FreeForm
           : handlerTypeForName(itemName)) {
    case ItemHandlerType::IntPair:
    case ItemHandlerType::IntPairNoTrailing:
    {
      auto [vn, tn] = item.toIntPair();
      String value  = String::number(vn);
      if(tn) {
        value += "/" + String::number(tn);
      }
      return {key, value};
    }
    case ItemHandlerType::Int:
    case ItemHandlerType::Gnre:
      return {key, String::number(item.toInt())};
    case ItemHandlerType::UInt:
      return {key, String::number(item.toUInt())};
    case ItemHandlerType::LongLong:
      return {key, String::fromLongLong(item.toLongLong())};
    case ItemHandlerType::Byte:
      return {key, String::number(item.toByte())};
    case ItemHandlerType::Bool:
      return {key, String::number(item.toBool())};
    case ItemHandlerType::FreeForm:
    case ItemHandlerType::TextOrInt:
    case ItemHandlerType::TextImplicit:
    case ItemHandlerType::Text:
      return {key, item.toStringList()};

    case ItemHandlerType::Covr:
      debug("MP4: Invalid item \"" + itemName + "\" for property");
      break;
    case ItemHandlerType::Unknown:
      debug("MP4: Unknown item name \"" + itemName + "\" for property");
      break;
    }
  }
  return {String(), StringList()};
}

String ItemFactory::propertyKeyForName(const ByteVector &name) const
{
  if(d->propertyKeyForName.isEmpty()) {
    d->propertyKeyForName = namePropertyMap();
  }
  String key = d->propertyKeyForName.value(name);
  if(key.isEmpty() && name.startsWith(freeFormPrefix)) {
    key = name.mid(std::size(freeFormPrefix) - 1);
  }
  return key;
}

ByteVector ItemFactory::nameForPropertyKey(const String &key) const
{
  if(d->nameForPropertyKey.isEmpty()) {
    if(d->propertyKeyForName.isEmpty()) {
      d->propertyKeyForName = namePropertyMap();
    }
    for(const auto &[k, t] : std::as_const(d->propertyKeyForName)) {
      d->nameForPropertyKey[t] = k;
    }
  }
  ByteVector name = d->nameForPropertyKey.value(key);
  if(name.isEmpty() && !key.isEmpty()) {
    const auto &firstChar = key[0];
    if(firstChar >= 'A' && firstChar <= 'Z') {
      name = (freeFormPrefix + key).data(String::UTF8);
    }
  }
  return name;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

ItemFactory::ItemFactory() :
  d(std::make_unique<ItemFactoryPrivate>())
{
}

ItemFactory::~ItemFactory() = default;

ItemFactory::NameHandlerMap ItemFactory::nameHandlerMap() const
{
  return {
    {"----", ItemHandlerType::FreeForm},
    {"trkn", ItemHandlerType::IntPair},
    {"disk", ItemHandlerType::IntPairNoTrailing},
    {"cpil", ItemHandlerType::Bool},
    {"pgap", ItemHandlerType::Bool},
    {"pcst", ItemHandlerType::Bool},
    {"shwm", ItemHandlerType::Bool},
    {"tmpo", ItemHandlerType::Int},
    {"\251mvi", ItemHandlerType::Int},
    {"\251mvc", ItemHandlerType::Int},
    {"hdvd", ItemHandlerType::Int},
    {"rate", ItemHandlerType::TextOrInt},
    {"tvsn", ItemHandlerType::UInt},
    {"tves", ItemHandlerType::UInt},
    {"cnID", ItemHandlerType::UInt},
    {"sfID", ItemHandlerType::UInt},
    {"atID", ItemHandlerType::UInt},
    {"geID", ItemHandlerType::UInt},
    {"cmID", ItemHandlerType::UInt},
    {"plID", ItemHandlerType::LongLong},
    {"stik", ItemHandlerType::Byte},
    {"rtng", ItemHandlerType::Byte},
    {"akID", ItemHandlerType::Byte},
    {"gnre", ItemHandlerType::Gnre},
    {"covr", ItemHandlerType::Covr},
    {"purl", ItemHandlerType::TextImplicit},
    {"egid", ItemHandlerType::TextImplicit},
  };
}

ItemFactory::ItemHandlerType ItemFactory::handlerTypeForName(
  const ByteVector &name) const
{
  if(d->handlerTypeForName.isEmpty()) {
    d->handlerTypeForName = nameHandlerMap();
  }
  auto type = d->handlerTypeForName.value(name, ItemHandlerType::Unknown);
  if (type == ItemHandlerType::Unknown && name.size() == 4) {
    type = ItemHandlerType::Text;
  }
  return type;
}

Map<ByteVector, String> ItemFactory::namePropertyMap() const
{
  return {
    {"\251nam", "TITLE"},
    {"\251ART", "ARTIST"},
    {"\251alb", "ALBUM"},
    {"\251cmt", "COMMENT"},
    {"\251gen", "GENRE"},
    {"\251day", "DATE"},
    {"\251wrt", "COMPOSER"},
    {"\251grp", "GROUPING"},
    {"aART", "ALBUMARTIST"},
    {"trkn", "TRACKNUMBER"},
    {"disk", "DISCNUMBER"},
    {"cpil", "COMPILATION"},
    {"tmpo", "BPM"},
    {"cprt", "COPYRIGHT"},
    {"\251lyr", "LYRICS"},
    {"\251too", "ENCODING"},
    {"\251enc", "ENCODEDBY"},
    {"soal", "ALBUMSORT"},
    {"soaa", "ALBUMARTISTSORT"},
    {"soar", "ARTISTSORT"},
    {"sonm", "TITLESORT"},
    {"soco", "COMPOSERSORT"},
    {"sosn", "SHOWSORT"},
    {"shwm", "SHOWWORKMOVEMENT"},
    {"pgap", "GAPLESSPLAYBACK"},
    {"pcst", "PODCAST"},
    {"catg", "PODCASTCATEGORY"},
    {"desc", "PODCASTDESC"},
    {"egid", "PODCASTID"},
    {"purl", "PODCASTURL"},
    {"tves", "TVEPISODE"},
    {"tven", "TVEPISODEID"},
    {"tvnn", "TVNETWORK"},
    {"tvsn", "TVSEASON"},
    {"tvsh", "TVSHOW"},
    {"\251wrk", "WORK"},
    {"\251mvn", "MOVEMENTNAME"},
    {"\251mvi", "MOVEMENTNUMBER"},
    {"\251mvc", "MOVEMENTCOUNT"},
    {"ownr", "OWNER"},
    {"----:com.apple.iTunes:MusicBrainz Track Id", "MUSICBRAINZ_TRACKID"},
    {"----:com.apple.iTunes:MusicBrainz Artist Id", "MUSICBRAINZ_ARTISTID"},
    {"----:com.apple.iTunes:MusicBrainz Album Id", "MUSICBRAINZ_ALBUMID"},
    {"----:com.apple.iTunes:MusicBrainz Album Artist Id", "MUSICBRAINZ_ALBUMARTISTID"},
    {"----:com.apple.iTunes:MusicBrainz Release Group Id", "MUSICBRAINZ_RELEASEGROUPID"},
    {"----:com.apple.iTunes:MusicBrainz Release Track Id", "MUSICBRAINZ_RELEASETRACKID"},
    {"----:com.apple.iTunes:MusicBrainz Work Id", "MUSICBRAINZ_WORKID"},
    {"----:com.apple.iTunes:MusicBrainz Album Release Country", "RELEASECOUNTRY"},
    {"----:com.apple.iTunes:MusicBrainz Album Status", "RELEASESTATUS"},
    {"----:com.apple.iTunes:MusicBrainz Album Type", "RELEASETYPE"},
    {"----:com.apple.iTunes:ARTISTS", "ARTISTS"},
    {"----:com.apple.iTunes:ORIGINALDATE", "ORIGINALDATE"},
    {"----:com.apple.iTunes:RELEASEDATE", "RELEASEDATE"},
    {"----:com.apple.iTunes:ASIN", "ASIN"},
    {"----:com.apple.iTunes:LABEL", "LABEL"},
    {"----:com.apple.iTunes:LYRICIST", "LYRICIST"},
    {"----:com.apple.iTunes:CONDUCTOR", "CONDUCTOR"},
    {"----:com.apple.iTunes:REMIXER", "REMIXER"},
    {"----:com.apple.iTunes:ENGINEER", "ENGINEER"},
    {"----:com.apple.iTunes:PRODUCER", "PRODUCER"},
    {"----:com.apple.iTunes:DJMIXER", "DJMIXER"},
    {"----:com.apple.iTunes:MIXER", "MIXER"},
    {"----:com.apple.iTunes:SUBTITLE", "SUBTITLE"},
    {"----:com.apple.iTunes:DISCSUBTITLE", "DISCSUBTITLE"},
    {"----:com.apple.iTunes:MOOD", "MOOD"},
    {"----:com.apple.iTunes:ISRC", "ISRC"},
    {"----:com.apple.iTunes:CATALOGNUMBER", "CATALOGNUMBER"},
    {"----:com.apple.iTunes:BARCODE", "BARCODE"},
    {"----:com.apple.iTunes:SCRIPT", "SCRIPT"},
    {"----:com.apple.iTunes:LANGUAGE", "LANGUAGE"},
    {"----:com.apple.iTunes:LICENSE", "LICENSE"},
    {"----:com.apple.iTunes:MEDIA", "MEDIA"}
  };
}

MP4::AtomDataList ItemFactory::parseData2(
  const MP4::Atom *atom, const ByteVector &data, int expectedFlags,
  bool freeForm)
{
  AtomDataList result;
  int i = 0;
  unsigned int pos = 0;
  while(pos < data.size()) {
    const auto length = static_cast<int>(data.toUInt(pos));
    if(length < 12) {
      debug("MP4: Too short atom");
      return result;
    }

    const ByteVector name = data.mid(pos + 4, 4);
    const auto flags = static_cast<int>(data.toUInt(pos + 8));
    if(freeForm && i < 2) {
      if(i == 0 && name != "mean") {
        debug("MP4: Unexpected atom \"" + name + "\", expecting \"mean\"");
        return result;
      }
      if(i == 1 && name != "name") {
        debug("MP4: Unexpected atom \"" + name + "\", expecting \"name\"");
        return result;
      }
      result.append(AtomData(static_cast<AtomDataType>(flags),
                    data.mid(pos + 12, length - 12)));
    }
    else {
      if(name != "data") {
        debug("MP4: Unexpected atom \"" + name + "\", expecting \"data\"");
        return result;
      }
      if(expectedFlags == -1 || flags == expectedFlags) {
        result.append(AtomData(static_cast<AtomDataType>(flags),
                      data.mid(pos + 16, length - 16)));
      }
    }
    pos += length;
    i++;
  }
  return result;
}

ByteVectorList ItemFactory::parseData(
  const MP4::Atom *atom, const ByteVector &bytes, int expectedFlags,
  bool freeForm)
{
  const AtomDataList data = parseData2(atom, bytes, expectedFlags, freeForm);
  ByteVectorList result;
  for(const auto &atom : data) {
    result.append(atom.data);
  }
  return result;
}

std::pair<String, Item> ItemFactory::parseInt(
  const MP4::Atom *atom, const ByteVector &bytes)
{
  ByteVectorList data = parseData(atom, bytes);
  return {
    atom->name(),
    !data.isEmpty() ? Item(static_cast<int>(data[0].toShort())) : Item()
  };
}

std::pair<String, Item> ItemFactory::parseTextOrInt(
  const MP4::Atom *atom, const ByteVector &bytes)
{
  if(AtomDataList data = parseData2(atom, bytes); !data.isEmpty()) {
    AtomData val = data[0];
    return {
      atom->name(),
       val.type == TypeUTF8 ? Item(StringList(String(val.data, String::UTF8)))
                            : Item(static_cast<int>(val.data.toShort()))
    };
  }
  return {atom->name(), Item()};
}

std::pair<String, Item> ItemFactory::parseUInt(
  const MP4::Atom *atom, const ByteVector &bytes)
{
  ByteVectorList data = parseData(atom, bytes);
  return {
    atom->name(),
    !data.isEmpty() ? Item(data[0].toUInt()) : Item()
  };
}

std::pair<String, Item> ItemFactory::parseLongLong(
  const MP4::Atom *atom, const ByteVector &bytes)
{
  ByteVectorList data = parseData(atom, bytes);
  return {
    atom->name(),
    !data.isEmpty() ?Item (data[0].toLongLong()) : Item()
  };
}

std::pair<String, Item> ItemFactory::parseByte(
  const MP4::Atom *atom, const ByteVector &bytes)
{
  ByteVectorList data = parseData(atom, bytes);
  return {
    atom->name(),
    !data.isEmpty() ? Item(static_cast<unsigned char>(data[0].at(0))) : Item()
  };
}

std::pair<String, Item> ItemFactory::parseGnre(
  const MP4::Atom *atom, const ByteVector &bytes)
{
  if(ByteVectorList data = parseData(atom, bytes); !data.isEmpty()) {
    int idx = static_cast<int>(data[0].toShort());
    if(idx > 0) {
      return {
        "\251gen",
        Item(StringList(ID3v1::genre(idx - 1)))
      };
    }
  }
  return {"\251gen", Item()};
}

std::pair<String, Item> ItemFactory::parseIntPair(
  const MP4::Atom *atom, const ByteVector &bytes)
{
  if(ByteVectorList data = parseData(atom, bytes); !data.isEmpty()) {
    const int a = data[0].toShort(2U);
    const int b = data[0].toShort(4U);
    return {atom->name(), Item(a, b)};
  }
  return {atom->name(), Item()};
}

std::pair<String, Item> ItemFactory::parseBool(
  const MP4::Atom *atom, const ByteVector &bytes)
{
  if(ByteVectorList data = parseData(atom, bytes); !data.isEmpty()) {
    bool value = !data[0].isEmpty() && data[0][0] != '\0';
    return {atom->name(), Item(value)};
  }
  return {atom->name(), Item()};
}

std::pair<String, Item> ItemFactory::parseText(
  const MP4::Atom *atom, const ByteVector &bytes, int expectedFlags)
{
  if(const ByteVectorList data = parseData(atom, bytes, expectedFlags);
     !data.isEmpty()) {
    StringList value;
    for(const auto &byte : data) {
      value.append(String(byte, String::UTF8));
    }
    return {atom->name(), Item(value)};
  }
  return {atom->name(), Item()};
}

std::pair<String, Item> ItemFactory::parseFreeForm(
  const MP4::Atom *atom, const ByteVector &bytes)
{
  if(const AtomDataList data = parseData2(atom, bytes, -1, true);
     data.size() > 2) {
    auto itBegin = data.begin();

    String name = "----:";
    name += String((itBegin++)->data, String::UTF8);  // data[0].data
    name += ':';
    name += String((itBegin++)->data, String::UTF8);  // data[1].data

    AtomDataType type = itBegin->type; // data[2].type

    for(auto it = itBegin; it != data.end(); ++it) {
      if(it->type != type) {
        debug("MP4: We currently don't support values with multiple types");
        break;
      }
    }
    if(type == TypeUTF8) {
      StringList value;
      for(auto it = itBegin; it != data.end(); ++it) {
        value.append(String(it->data, String::UTF8));
      }
      Item item(value);
      item.setAtomDataType(type);
      return {name, item};
    }
    ByteVectorList value;
    for(auto it = itBegin; it != data.end(); ++it) {
      value.append(it->data);
    }
    Item item(value);
    item.setAtomDataType(type);
    return {name, item};
  }
  return {atom->name(), Item()};
}

std::pair<String, Item> ItemFactory::parseCovr(
  const MP4::Atom *atom, const ByteVector &data)
{
  MP4::CoverArtList value;
  unsigned int pos = 0;
  while(pos < data.size()) {
    const int length = static_cast<int>(data.toUInt(pos));
    if(length < 12) {
      debug("MP4: Too short atom");
      break;
    }

    const ByteVector name = data.mid(pos + 4, 4);
    const int flags = static_cast<int>(data.toUInt(pos + 8));
    if(name != "data") {
      debug("MP4: Unexpected atom \"" + name + "\", expecting \"data\"");
      break;
    }
    if(flags == TypeJPEG || flags == TypePNG || flags == TypeBMP ||
       flags == TypeGIF || flags == TypeImplicit) {
      value.append(MP4::CoverArt(static_cast<MP4::CoverArt::Format>(flags),
                                 data.mid(pos + 16, length - 16)));
    }
    else {
      debug("MP4: Unknown covr format " + String::number(flags));
    }
    pos += length;
  }
  return {
    atom->name(),
    !value.isEmpty() ? Item(value) : Item()
  };
}


ByteVector ItemFactory::renderAtom(
  const ByteVector &name, const ByteVector &data)
{
  return ByteVector::fromUInt(data.size() + 8) + name + data;
}

ByteVector ItemFactory::renderData(
  const ByteVector &name, int flags, const ByteVectorList &data)
{
  ByteVector result;
  for(const auto &byte : data) {
    result.append(renderAtom("data", ByteVector::fromUInt(flags) +
                                     ByteVector(4, '\0') + byte));
  }
  return renderAtom(name, result);
}

ByteVector ItemFactory::renderBool(
  const ByteVector &name, const MP4::Item &item)
{
  ByteVectorList data;
  data.append(ByteVector(1, item.toBool() ? '\1' : '\0'));
  return renderData(name, TypeInteger, data);
}

ByteVector ItemFactory::renderInt(
  const ByteVector &name, const MP4::Item &item)
{
  ByteVectorList data;
  data.append(ByteVector::fromShort(item.toInt()));
  return renderData(name, TypeInteger, data);
}

ByteVector ItemFactory::renderTextOrInt(
  const ByteVector &name, const MP4::Item &item)
{
  StringList value = item.toStringList();
  return value.isEmpty() ? renderInt(name, item) : renderText(name, item);
}

ByteVector ItemFactory::renderUInt(
  const ByteVector &name, const MP4::Item &item)
{
  ByteVectorList data;
  data.append(ByteVector::fromUInt(item.toUInt()));
  return renderData(name, TypeInteger, data);
}

ByteVector ItemFactory::renderLongLong(
  const ByteVector &name, const MP4::Item &item)
{
  ByteVectorList data;
  data.append(ByteVector::fromLongLong(item.toLongLong()));
  return renderData(name, TypeInteger, data);
}

ByteVector ItemFactory::renderByte(
  const ByteVector &name, const MP4::Item &item)
{
  ByteVectorList data;
  data.append(ByteVector(1, item.toByte()));
  return renderData(name, TypeInteger, data);
}

ByteVector ItemFactory::renderIntPair(
  const ByteVector &name, const MP4::Item &item)
{
  ByteVectorList data;
  data.append(ByteVector(2, '\0') +
              ByteVector::fromShort(item.toIntPair().first) +
              ByteVector::fromShort(item.toIntPair().second) +
              ByteVector(2, '\0'));
  return renderData(name, TypeImplicit, data);
}

ByteVector ItemFactory::renderIntPairNoTrailing(
  const ByteVector &name, const MP4::Item &item)
{
  ByteVectorList data;
  data.append(ByteVector(2, '\0') +
              ByteVector::fromShort(item.toIntPair().first) +
              ByteVector::fromShort(item.toIntPair().second));
  return renderData(name, TypeImplicit, data);
}

ByteVector ItemFactory::renderText(
  const ByteVector &name, const MP4::Item &item, int flags)
{
  ByteVectorList data;
  const StringList values = item.toStringList();
  for(const auto &value : values) {
    data.append(value.data(String::UTF8));
  }
  return renderData(name, flags, data);
}

ByteVector ItemFactory::renderCovr(
  const ByteVector &name, const MP4::Item &item)
{
  ByteVector data;
  const MP4::CoverArtList values = item.toCoverArtList();
  for(const auto &value : values) {
    data.append(renderAtom("data", ByteVector::fromUInt(value.format()) +
                                   ByteVector(4, '\0') + value.data()));
  }
  return renderAtom(name, data);
}

ByteVector ItemFactory::renderFreeForm(
  const String &name, const MP4::Item &item)
{
  StringList header = StringList::split(name, ":");
  if(header.size() != 3) {
    debug("MP4: Invalid free-form item name \"" + name + "\"");
    return ByteVector();
  }
  ByteVector data;
  data.append(renderAtom("mean", ByteVector::fromUInt(0) +
                                 header[1].data(String::UTF8)));
  data.append(renderAtom("name", ByteVector::fromUInt(0) +
                                 header[2].data(String::UTF8)));
  AtomDataType type = item.atomDataType();
  if(type == TypeUndefined) {
    if(!item.toStringList().isEmpty()) {
      type = TypeUTF8;
    }
    else {
      type = TypeImplicit;
    }
  }
  if(type == TypeUTF8) {
    const StringList values = item.toStringList();
    for(const auto &value : values) {
      data.append(renderAtom("data", ByteVector::fromUInt(type) +
                                     ByteVector(4, '\0') +
                                     value.data(String::UTF8)));
    }
  }
  else {
    const ByteVectorList values = item.toByteVectorList();
    for(const auto &value : values) {
      data.append(renderAtom("data", ByteVector::fromUInt(type) +
                                     ByteVector(4, '\0') + value));
    }
  }
  return renderAtom("----", data);
}
