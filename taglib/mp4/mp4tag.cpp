/**************************************************************************
    copyright            : (C) 2007,2011 by Lukáš Lalinský
    email                : lalinsky@gmail.com
 **************************************************************************/

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

#include "mp4tag.h"

#include <array>
#include <utility>

#include "tdebug.h"
#include "tpropertymap.h"
#include "id3v1genres.h"
#include "mp4atom.h"
#include "mp4coverart.h"

using namespace TagLib;

class MP4::Tag::TagPrivate
{
public:
  TagLib::File *file { nullptr };
  Atoms *atoms { nullptr };
  ItemMap items;
};

MP4::Tag::Tag() :
  d(std::make_unique<TagPrivate>())
{
}

MP4::Tag::Tag(TagLib::File *file, MP4::Atoms *atoms) :
  d(std::make_unique<TagPrivate>())
{
  d->file = file;
  d->atoms = atoms;

  MP4::Atom *ilst = atoms->find("moov", "udta", "meta", "ilst");
  if(!ilst) {
    //debug("Atom moov.udta.meta.ilst not found.");
    return;
  }

  for(const auto &atom : std::as_const(ilst->children)) {
    file->seek(atom->offset + 8);
    if(atom->name == "----") {
      parseFreeForm(atom);
    }
    else if(atom->name == "trkn" || atom->name == "disk") {
      parseIntPair(atom);
    }
    else if(atom->name == "cpil" || atom->name == "pgap" || atom->name == "pcst" ||
            atom->name == "hdvd" || atom->name == "shwm") {
      parseBool(atom);
    }
    else if(atom->name == "tmpo" || atom->name == "\251mvi" || atom->name == "\251mvc") {
      parseInt(atom);
    }
    else if(atom->name == "rate") {
      AtomDataList data = parseData2(atom);
      if(!data.isEmpty()) {
        AtomData val = data[0];
        if (val.type == TypeUTF8) {
          addItem(atom->name, StringList(String(val.data, String::UTF8)));
        } else {
          addItem(atom->name, static_cast<int>(val.data.toShort()));
        }
      }
    }
    else if(atom->name == "tvsn" || atom->name == "tves" || atom->name == "cnID" ||
            atom->name == "sfID" || atom->name == "atID" || atom->name == "geID" ||
            atom->name == "cmID") {
      parseUInt(atom);
    }
    else if(atom->name == "plID") {
      parseLongLong(atom);
    }
    else if(atom->name == "stik" || atom->name == "rtng" || atom->name == "akID") {
      parseByte(atom);
    }
    else if(atom->name == "gnre") {
      parseGnre(atom);
    }
    else if(atom->name == "covr") {
      parseCovr(atom);
    }
    else if(atom->name == "purl" || atom->name == "egid") {
      parseText(atom, -1);
    }
    else {
      parseText(atom);
    }
  }
}

MP4::Tag::~Tag() = default;

MP4::AtomDataList
MP4::Tag::parseData2(const MP4::Atom *atom, int expectedFlags, bool freeForm)
{
  AtomDataList result;
  ByteVector data = d->file->readBlock(atom->length - 8);
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
      result.append(AtomData(static_cast<AtomDataType>(flags), data.mid(pos + 12, length - 12)));
    }
    else {
      if(name != "data") {
        debug("MP4: Unexpected atom \"" + name + "\", expecting \"data\"");
        return result;
      }
      if(expectedFlags == -1 || flags == expectedFlags) {
        result.append(AtomData(static_cast<AtomDataType>(flags), data.mid(pos + 16, length - 16)));
      }
    }
    pos += length;
    i++;
  }
  return result;
}

ByteVectorList
MP4::Tag::parseData(const MP4::Atom *atom, int expectedFlags, bool freeForm)
{
  const AtomDataList data = parseData2(atom, expectedFlags, freeForm);
  ByteVectorList result;
  for(const auto &atom : data) {
    result.append(atom.data);
  }
  return result;
}

void
MP4::Tag::parseInt(const MP4::Atom *atom)
{
  ByteVectorList data = parseData(atom);
  if(!data.isEmpty()) {
    addItem(atom->name, static_cast<int>(data[0].toShort()));
  }
}

void
MP4::Tag::parseUInt(const MP4::Atom *atom)
{
  ByteVectorList data = parseData(atom);
  if(!data.isEmpty()) {
    addItem(atom->name, data[0].toUInt());
  }
}

void
MP4::Tag::parseLongLong(const MP4::Atom *atom)
{
  ByteVectorList data = parseData(atom);
  if(!data.isEmpty()) {
    addItem(atom->name, data[0].toLongLong());
  }
}

void
MP4::Tag::parseByte(const MP4::Atom *atom)
{
  ByteVectorList data = parseData(atom);
  if(!data.isEmpty()) {
    addItem(atom->name, static_cast<unsigned char>(data[0].at(0)));
  }
}

void
MP4::Tag::parseGnre(const MP4::Atom *atom)
{
  ByteVectorList data = parseData(atom);
  if(!data.isEmpty()) {
    int idx = static_cast<int>(data[0].toShort());
    if(idx > 0) {
      addItem("\251gen", StringList(ID3v1::genre(idx - 1)));
    }
  }
}

void
MP4::Tag::parseIntPair(const MP4::Atom *atom)
{
  ByteVectorList data = parseData(atom);
  if(!data.isEmpty()) {
    const int a = data[0].toShort(2U);
    const int b = data[0].toShort(4U);
    addItem(atom->name, MP4::Item(a, b));
  }
}

void
MP4::Tag::parseBool(const MP4::Atom *atom)
{
  ByteVectorList data = parseData(atom);
  if(!data.isEmpty()) {
    bool value = !data[0].isEmpty() && data[0][0] != '\0';
    addItem(atom->name, value);
  }
}

void
MP4::Tag::parseText(const MP4::Atom *atom, int expectedFlags)
{
  const ByteVectorList data = parseData(atom, expectedFlags);
  if(!data.isEmpty()) {
    StringList value;
    for(const auto &byte : data) {
      value.append(String(byte, String::UTF8));
    }
    addItem(atom->name, value);
  }
}

void
MP4::Tag::parseFreeForm(const MP4::Atom *atom)
{
  const AtomDataList data = parseData2(atom, -1, true);
  if(data.size() > 2) {
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
      addItem(name, item);
    }
    else {
      ByteVectorList value;
      for(auto it = itBegin; it != data.end(); ++it) {
        value.append(it->data);
      }
      Item item(value);
      item.setAtomDataType(type);
      addItem(name, item);
    }
  }
}

void
MP4::Tag::parseCovr(const MP4::Atom *atom)
{
  MP4::CoverArtList value;
  ByteVector data = d->file->readBlock(atom->length - 8);
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
  if(!value.isEmpty())
    addItem(atom->name, value);
}

ByteVector
MP4::Tag::padIlst(const ByteVector &data, int length) const
{
  if(length == -1) {
    length = ((data.size() + 1023) & ~1023) - data.size();
  }
  return renderAtom("free", ByteVector(length, '\1'));
}

ByteVector
MP4::Tag::renderAtom(const ByteVector &name, const ByteVector &data) const
{
  return ByteVector::fromUInt(data.size() + 8) + name + data;
}

ByteVector
MP4::Tag::renderData(const ByteVector &name, int flags, const ByteVectorList &data) const
{
  ByteVector result;
  for(const auto &byte : data) {
    result.append(renderAtom("data", ByteVector::fromUInt(flags) + ByteVector(4, '\0') + byte));
  }
  return renderAtom(name, result);
}

ByteVector
MP4::Tag::renderBool(const ByteVector &name, const MP4::Item &item) const
{
  ByteVectorList data;
  data.append(ByteVector(1, item.toBool() ? '\1' : '\0'));
  return renderData(name, TypeInteger, data);
}

ByteVector
MP4::Tag::renderInt(const ByteVector &name, const MP4::Item &item) const
{
  ByteVectorList data;
  data.append(ByteVector::fromShort(item.toInt()));
  return renderData(name, TypeInteger, data);
}

ByteVector
MP4::Tag::renderUInt(const ByteVector &name, const MP4::Item &item) const
{
  ByteVectorList data;
  data.append(ByteVector::fromUInt(item.toUInt()));
  return renderData(name, TypeInteger, data);
}

ByteVector
MP4::Tag::renderLongLong(const ByteVector &name, const MP4::Item &item) const
{
  ByteVectorList data;
  data.append(ByteVector::fromLongLong(item.toLongLong()));
  return renderData(name, TypeInteger, data);
}

ByteVector
MP4::Tag::renderByte(const ByteVector &name, const MP4::Item &item) const
{
  ByteVectorList data;
  data.append(ByteVector(1, item.toByte()));
  return renderData(name, TypeInteger, data);
}

ByteVector
MP4::Tag::renderIntPair(const ByteVector &name, const MP4::Item &item) const
{
  ByteVectorList data;
  data.append(ByteVector(2, '\0') +
              ByteVector::fromShort(item.toIntPair().first) +
              ByteVector::fromShort(item.toIntPair().second) +
              ByteVector(2, '\0'));
  return renderData(name, TypeImplicit, data);
}

ByteVector
MP4::Tag::renderIntPairNoTrailing(const ByteVector &name, const MP4::Item &item) const
{
  ByteVectorList data;
  data.append(ByteVector(2, '\0') +
              ByteVector::fromShort(item.toIntPair().first) +
              ByteVector::fromShort(item.toIntPair().second));
  return renderData(name, TypeImplicit, data);
}

ByteVector
MP4::Tag::renderText(const ByteVector &name, const MP4::Item &item, int flags) const
{
  ByteVectorList data;
  const StringList values = item.toStringList();
  for(const auto &value : values) {
    data.append(value.data(String::UTF8));
  }
  return renderData(name, flags, data);
}

ByteVector
MP4::Tag::renderCovr(const ByteVector &name, const MP4::Item &item) const
{
  ByteVector data;
  const MP4::CoverArtList values = item.toCoverArtList();
  for(const auto &value : values) {
    data.append(renderAtom("data", ByteVector::fromUInt(value.format()) +
                                   ByteVector(4, '\0') + value.data()));
  }
  return renderAtom(name, data);
}

ByteVector
MP4::Tag::renderFreeForm(const String &name, const MP4::Item &item) const
{
  StringList header = StringList::split(name, ":");
  if(header.size() != 3) {
    debug("MP4: Invalid free-form item name \"" + name + "\"");
    return ByteVector();
  }
  ByteVector data;
  data.append(renderAtom("mean", ByteVector::fromUInt(0) + header[1].data(String::UTF8)));
  data.append(renderAtom("name", ByteVector::fromUInt(0) + header[2].data(String::UTF8)));
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
                                     ByteVector(4, '\0') + value.data(String::UTF8)));
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

bool
MP4::Tag::save()
{
  ByteVector data;
  for(const auto &[name, item] : std::as_const(d->items)) {
    if(name.startsWith("----")) {
      data.append(renderFreeForm(name, item));
    }
    else if(name == "trkn") {
      data.append(renderIntPair(name.data(String::Latin1), item));
    }
    else if(name == "disk") {
      data.append(renderIntPairNoTrailing(name.data(String::Latin1), item));
    }
    else if(name == "cpil" || name == "pgap" || name == "pcst" || name == "hdvd" ||
            name == "shwm") {
      data.append(renderBool(name.data(String::Latin1), item));
    }
    else if(name == "tmpo" || name == "\251mvi" || name == "\251mvc") {
      data.append(renderInt(name.data(String::Latin1), item));
    }
    else if(name == "rate") {
      StringList value = item.toStringList();
      if (value.isEmpty()) {
        data.append(renderInt(name.data(String::Latin1), item));
      }
      else {
        data.append(renderText(name.data(String::Latin1), item));
      }
    }
    else if(name == "tvsn" || name == "tves" || name == "cnID" ||
            name == "sfID" || name == "atID" || name == "geID" ||
            name == "cmID") {
      data.append(renderUInt(name.data(String::Latin1), item));
    }
    else if(name == "plID") {
      data.append(renderLongLong(name.data(String::Latin1), item));
    }
    else if(name == "stik" || name == "rtng" || name == "akID") {
      data.append(renderByte(name.data(String::Latin1), item));
    }
    else if(name == "covr") {
      data.append(renderCovr(name.data(String::Latin1), item));
    }
    else if(name == "purl" || name == "egid") {
      data.append(renderText(name.data(String::Latin1), item, TypeImplicit));
    }
    else if(name.size() == 4){
      data.append(renderText(name.data(String::Latin1), item));
    }
    else {
      debug("MP4: Unknown item name \"" + name + "\"");
    }
  }
  data = renderAtom("ilst", data);

  AtomList path = d->atoms->path("moov", "udta", "meta", "ilst");
  if(path.size() == 4) {
    saveExisting(data, path);
  }
  else {
    saveNew(data);
  }

  return true;
}

bool
MP4::Tag::strip()
{
  d->items.clear();

  AtomList path = d->atoms->path("moov", "udta", "meta", "ilst");
  if(path.size() == 4) {
    saveExisting(ByteVector(), path);
  }

  return true;
}

void
MP4::Tag::updateParents(const AtomList &path, offset_t delta, int ignore)
{
  if(static_cast<int>(path.size()) <= ignore)
    return;

  auto itEnd = path.end();
  std::advance(itEnd, 0 - ignore);

  for(auto it = path.begin(); it != itEnd; ++it) {
    d->file->seek((*it)->offset);
    long size = d->file->readBlock(4).toUInt();
    // 64-bit
    if (size == 1) {
      d->file->seek(4, File::Current); // Skip name
      long long longSize = d->file->readBlock(8).toLongLong();
      // Seek the offset of the 64-bit size
      d->file->seek((*it)->offset + 8);
      d->file->writeBlock(ByteVector::fromLongLong(longSize + delta));
    }
    // 32-bit
    else {
      d->file->seek((*it)->offset);
      d->file->writeBlock(ByteVector::fromUInt(static_cast<unsigned int>(size + delta)));
    }
  }
}

void
MP4::Tag::updateOffsets(offset_t delta, offset_t offset)
{
  MP4::Atom *moov = d->atoms->find("moov");
  if(moov) {
    const MP4::AtomList stco = moov->findall("stco", true);
    for(const auto &atom : stco) {
      if(atom->offset > offset) {
        atom->offset += delta;
      }
      d->file->seek(atom->offset + 12);
      ByteVector data = d->file->readBlock(atom->length - 12);
      unsigned int count = data.toUInt();
      d->file->seek(atom->offset + 16);
      unsigned int pos = 4;
      while(count--) {
        auto o = static_cast<offset_t>(data.toUInt(pos));
        if(o > offset) {
          o += delta;
        }
        d->file->writeBlock(ByteVector::fromUInt(static_cast<unsigned int>(o)));
        pos += 4;
      }
    }

    const MP4::AtomList co64 = moov->findall("co64", true);
    for(const auto &atom : co64) {
      if(atom->offset > offset) {
        atom->offset += delta;
      }
      d->file->seek(atom->offset + 12);
      ByteVector data = d->file->readBlock(atom->length - 12);
      unsigned int count = data.toUInt();
      d->file->seek(atom->offset + 16);
      unsigned int pos = 4;
      while(count--) {
        long long o = data.toLongLong(pos);
        if(o > offset) {
          o += delta;
        }
        d->file->writeBlock(ByteVector::fromLongLong(o));
        pos += 8;
      }
    }
  }

  MP4::Atom *moof = d->atoms->find("moof");
  if(moof) {
    const MP4::AtomList tfhd = moof->findall("tfhd", true);
    for(const auto &atom : tfhd) {
      if(atom->offset > offset) {
        atom->offset += delta;
      }
      d->file->seek(atom->offset + 9);
      ByteVector data = d->file->readBlock(atom->length - 9);
      const unsigned int flags = data.toUInt(0, 3, true);
      if(flags & 1) {
        long long o = data.toLongLong(7U);
        if(o > offset) {
          o += delta;
        }
        d->file->seek(atom->offset + 16);
        d->file->writeBlock(ByteVector::fromLongLong(o));
      }
    }
  }
}

void
MP4::Tag::saveNew(ByteVector data)
{
  data = renderAtom("meta", ByteVector(4, '\0') +
                    renderAtom("hdlr", ByteVector(8, '\0') + ByteVector("mdirappl") +
                               ByteVector(9, '\0')) +
                    data + padIlst(data));

  AtomList path = d->atoms->path("moov", "udta");
  if(path.size() != 2) {
    path = d->atoms->path("moov");
    data = renderAtom("udta", data);
  }

  offset_t offset = path.back()->offset + 8;
  d->file->insert(data, offset, 0);

  updateParents(path, data.size());
  updateOffsets(data.size(), offset);

  // Insert the newly created atoms into the tree to keep it up-to-date.

  d->file->seek(offset);
  path.back()->children.prepend(new Atom(d->file));
}

void
MP4::Tag::saveExisting(ByteVector data, const AtomList &path)
{
  auto it = path.end();

  MP4::Atom *ilst = *(--it);
  offset_t offset = ilst->offset;
  offset_t length = ilst->length;

  MP4::Atom *meta = *(--it);
  auto index = meta->children.cfind(ilst);

  // check if there is an atom before 'ilst', and possibly use it as padding
  if(index != meta->children.cbegin()) {
    auto prevIndex = std::prev(index);
    MP4::Atom *prev = *prevIndex;
    if(prev->name == "free") {
      offset = prev->offset;
      length += prev->length;
    }
  }
  // check if there is an atom after 'ilst', and possibly use it as padding
  auto nextIndex = std::next(index);
  if(nextIndex != meta->children.cend()) {
    MP4::Atom *next = *nextIndex;
    if(next->name == "free") {
      length += next->length;
    }
  }

  offset_t delta = data.size() - length;
  if(!data.isEmpty()) {
    if(delta > 0 || (delta < 0 && delta > -8)) {
      data.append(padIlst(data));
      delta = data.size() - length;
    }
    else if(delta < 0) {
      data.append(padIlst(data, static_cast<int>(-delta - 8)));
      delta = 0;
    }

    d->file->insert(data, offset, length);

    if(delta) {
      updateParents(path, delta, 1);
      updateOffsets(delta, offset);
    }
  }
  else {
    // Strip meta if data is empty, only the case when called from strip().
    MP4::Atom *udta = *std::prev(it);
    AtomList &udtaChildren = udta->children;
    auto metaIt = udtaChildren.find(meta);
    if(metaIt != udtaChildren.end()) {
      offset = meta->offset;
      delta = - meta->length;
      udtaChildren.erase(metaIt);
      d->file->removeBlock(meta->offset, meta->length);
      delete meta;

      if(delta) {
        updateParents(path, delta, 2);
        updateOffsets(delta, offset);
      }
    }
  }
}

String
MP4::Tag::title() const
{
  if(d->items.contains("\251nam"))
    return d->items["\251nam"].toStringList().toString(", ");
  return String();
}

String
MP4::Tag::artist() const
{
  if(d->items.contains("\251ART"))
    return d->items["\251ART"].toStringList().toString(", ");
  return String();
}

String
MP4::Tag::album() const
{
  if(d->items.contains("\251alb"))
    return d->items["\251alb"].toStringList().toString(", ");
  return String();
}

String
MP4::Tag::comment() const
{
  if(d->items.contains("\251cmt"))
    return d->items["\251cmt"].toStringList().toString(", ");
  return String();
}

String
MP4::Tag::genre() const
{
  if(d->items.contains("\251gen"))
    return d->items["\251gen"].toStringList().toString(", ");
  return String();
}

unsigned int
MP4::Tag::year() const
{
  if(d->items.contains("\251day"))
    return d->items["\251day"].toStringList().toString().toInt();
  return 0;
}

unsigned int
MP4::Tag::track() const
{
  if(d->items.contains("trkn"))
    return d->items["trkn"].toIntPair().first;
  return 0;
}

void
MP4::Tag::setTitle(const String &value)
{
  setTextItem("\251nam", value);
}

void
MP4::Tag::setArtist(const String &value)
{
  setTextItem("\251ART", value);
}

void
MP4::Tag::setAlbum(const String &value)
{
  setTextItem("\251alb", value);
}

void
MP4::Tag::setComment(const String &value)
{
  setTextItem("\251cmt", value);
}

void
MP4::Tag::setGenre(const String &value)
{
  setTextItem("\251gen", value);
}

void
MP4::Tag::setTextItem(const String &key, const String &value)
{
  if (!value.isEmpty()) {
    d->items[key] = StringList(value);
  } else {
    d->items.erase(key);
  }
}

void
MP4::Tag::setYear(unsigned int value)
{
  if (value == 0) {
    d->items.erase("\251day");
  }
  else {
    d->items["\251day"] = StringList(String::number(value));
  }
}

void
MP4::Tag::setTrack(unsigned int value)
{
  if (value == 0) {
    d->items.erase("trkn");
  }
  else {
    d->items["trkn"] = MP4::Item(value, 0);
  }
}

bool MP4::Tag::isEmpty() const
{
  return d->items.isEmpty();
}

const MP4::ItemMap &MP4::Tag::itemMap() const
{
  return d->items;
}

MP4::Item MP4::Tag::item(const String &key) const
{
  return d->items[key];
}

void MP4::Tag::setItem(const String &key, const Item &value)
{
  d->items[key] = value;
}

void MP4::Tag::removeItem(const String &key)
{
  d->items.erase(key);
}

bool MP4::Tag::contains(const String &key) const
{
  return d->items.contains(key);
}

namespace
{
  constexpr std::array keyTranslation {
    std::pair("\251nam", "TITLE"),
    std::pair("\251ART", "ARTIST"),
    std::pair("\251alb", "ALBUM"),
    std::pair("\251cmt", "COMMENT"),
    std::pair("\251gen", "GENRE"),
    std::pair("\251day", "DATE"),
    std::pair("\251wrt", "COMPOSER"),
    std::pair("\251grp", "GROUPING"),
    std::pair("aART", "ALBUMARTIST"),
    std::pair("trkn", "TRACKNUMBER"),
    std::pair("disk", "DISCNUMBER"),
    std::pair("cpil", "COMPILATION"),
    std::pair("tmpo", "BPM"),
    std::pair("cprt", "COPYRIGHT"),
    std::pair("\251lyr", "LYRICS"),
    std::pair("\251too", "ENCODEDBY"),
    std::pair("soal", "ALBUMSORT"),
    std::pair("soaa", "ALBUMARTISTSORT"),
    std::pair("soar", "ARTISTSORT"),
    std::pair("sonm", "TITLESORT"),
    std::pair("soco", "COMPOSERSORT"),
    std::pair("sosn", "SHOWSORT"),
    std::pair("shwm", "SHOWWORKMOVEMENT"),
    std::pair("pgap", "GAPLESSPLAYBACK"),
    std::pair("pcst", "PODCAST"),
    std::pair("catg", "PODCASTCATEGORY"),
    std::pair("desc", "PODCASTDESC"),
    std::pair("egid", "PODCASTID"),
    std::pair("purl", "PODCASTURL"),
    std::pair("tves", "TVEPISODE"),
    std::pair("tven", "TVEPISODEID"),
    std::pair("tvnn", "TVNETWORK"),
    std::pair("tvsn", "TVSEASON"),
    std::pair("tvsh", "TVSHOW"),
    std::pair("\251wrk", "WORK"),
    std::pair("\251mvn", "MOVEMENTNAME"),
    std::pair("\251mvi", "MOVEMENTNUMBER"),
    std::pair("\251mvc", "MOVEMENTCOUNT"),
    std::pair("----:com.apple.iTunes:MusicBrainz Track Id", "MUSICBRAINZ_TRACKID"),
    std::pair("----:com.apple.iTunes:MusicBrainz Artist Id", "MUSICBRAINZ_ARTISTID"),
    std::pair("----:com.apple.iTunes:MusicBrainz Album Id", "MUSICBRAINZ_ALBUMID"),
    std::pair("----:com.apple.iTunes:MusicBrainz Album Artist Id", "MUSICBRAINZ_ALBUMARTISTID"),
    std::pair("----:com.apple.iTunes:MusicBrainz Release Group Id", "MUSICBRAINZ_RELEASEGROUPID"),
    std::pair("----:com.apple.iTunes:MusicBrainz Release Track Id", "MUSICBRAINZ_RELEASETRACKID"),
    std::pair("----:com.apple.iTunes:MusicBrainz Work Id", "MUSICBRAINZ_WORKID"),
    std::pair("----:com.apple.iTunes:MusicBrainz Album Release Country", "RELEASECOUNTRY"),
    std::pair("----:com.apple.iTunes:MusicBrainz Album Status", "RELEASESTATUS"),
    std::pair("----:com.apple.iTunes:MusicBrainz Album Type", "RELEASETYPE"),
    std::pair("----:com.apple.iTunes:ARTISTS", "ARTISTS"),
    std::pair("----:com.apple.iTunes:originaldate", "ORIGINALDATE"),
    std::pair("----:com.apple.iTunes:ASIN", "ASIN"),
    std::pair("----:com.apple.iTunes:LABEL", "LABEL"),
    std::pair("----:com.apple.iTunes:LYRICIST", "LYRICIST"),
    std::pair("----:com.apple.iTunes:CONDUCTOR", "CONDUCTOR"),
    std::pair("----:com.apple.iTunes:REMIXER", "REMIXER"),
    std::pair("----:com.apple.iTunes:ENGINEER", "ENGINEER"),
    std::pair("----:com.apple.iTunes:PRODUCER", "PRODUCER"),
    std::pair("----:com.apple.iTunes:DJMIXER", "DJMIXER"),
    std::pair("----:com.apple.iTunes:MIXER", "MIXER"),
    std::pair("----:com.apple.iTunes:SUBTITLE", "SUBTITLE"),
    std::pair("----:com.apple.iTunes:DISCSUBTITLE", "DISCSUBTITLE"),
    std::pair("----:com.apple.iTunes:MOOD", "MOOD"),
    std::pair("----:com.apple.iTunes:ISRC", "ISRC"),
    std::pair("----:com.apple.iTunes:CATALOGNUMBER", "CATALOGNUMBER"),
    std::pair("----:com.apple.iTunes:BARCODE", "BARCODE"),
    std::pair("----:com.apple.iTunes:SCRIPT", "SCRIPT"),
    std::pair("----:com.apple.iTunes:LANGUAGE", "LANGUAGE"),
    std::pair("----:com.apple.iTunes:LICENSE", "LICENSE"),
    std::pair("----:com.apple.iTunes:MEDIA", "MEDIA"),
  };

  String translateKey(const String &key)
  {
    for(const auto &[k, t] : keyTranslation) {
      if(key == k)
        return t;
    }

    return String();
  }
}  // namespace

PropertyMap MP4::Tag::properties() const
{
  PropertyMap props;
  for(const auto &[k, t] : std::as_const(d->items)) {
    const String key = translateKey(k);
    if(!key.isEmpty()) {
      if(key == "TRACKNUMBER" || key == "DISCNUMBER") {
        auto [vn, tn] = t.toIntPair();
        String value  = String::number(vn);
        if(tn) {
          value += "/" + String::number(tn);
        }
        props[key] = value;
      }
      else if(key == "BPM" || key == "MOVEMENTNUMBER" || key == "MOVEMENTCOUNT" ||
              key == "TVEPISODE" || key == "TVSEASON") {
        props[key] = String::number(t.toInt());
      }
      else if(key == "COMPILATION" || key == "SHOWWORKMOVEMENT" ||
              key == "GAPLESSPLAYBACK" || key == "PODCAST") {
        props[key] = String::number(t.toBool());
      }
      else {
        props[key] = t.toStringList();
      }
    }
    else {
      props.unsupportedData().append(k);
    }
  }
  return props;
}

void MP4::Tag::removeUnsupportedProperties(const StringList &props)
{
  for(const auto &prop : props)
    d->items.erase(prop);
}

PropertyMap MP4::Tag::setProperties(const PropertyMap &props)
{
  static Map<String, String> reverseKeyMap;
  if(reverseKeyMap.isEmpty()) {
    for(const auto &[k, t] : keyTranslation) {
      reverseKeyMap[t] = k;
    }
  }

  const PropertyMap origProps = properties();
  for(const auto &[prop, _] : origProps) {
    if(!props.contains(prop) || props[prop].isEmpty()) {
      d->items.erase(reverseKeyMap[prop]);
    }
  }

  PropertyMap ignoredProps;
  for(const auto &[prop, val] : props) {
    if(reverseKeyMap.contains(prop)) {
      String name = reverseKeyMap[prop];
      if((prop == "TRACKNUMBER" || prop == "DISCNUMBER") && !val.isEmpty()) {
        StringList parts = StringList::split(val.front(), "/");
        if(!parts.isEmpty()) {
          int first = parts[0].toInt();
          int second = 0;
          if(parts.size() > 1) {
            second = parts[1].toInt();
          }
          d->items[name] = MP4::Item(first, second);
        }
      }
      else if((prop == "BPM" || prop == "MOVEMENTNUMBER" ||
               prop == "MOVEMENTCOUNT" || prop == "TVEPISODE" ||
               prop == "TVSEASON") && !val.isEmpty()) {
        int value = val.front().toInt();
        d->items[name] = MP4::Item(value);
      }
      else if((prop == "COMPILATION" || prop == "SHOWWORKMOVEMENT" ||
               prop == "GAPLESSPLAYBACK" || prop == "PODCAST") &&
              !val.isEmpty()) {
        bool value = (val.front().toInt() != 0);
        d->items[name] = MP4::Item(value);
      }
      else {
        d->items[name] = val;
      }
    }
    else {
      ignoredProps.insert(prop, val);
    }
  }

  return ignoredProps;
}

StringList MP4::Tag::complexPropertyKeys() const
{
  StringList keys;
  if(d->items.contains("covr")) {
    keys.append("PICTURE");
  }
  return keys;
}

List<VariantMap> MP4::Tag::complexProperties(const String &key) const
{
  List<VariantMap> properties;
  const String uppercaseKey = key.upper();
  if(uppercaseKey == "PICTURE") {
    const CoverArtList pictures = d->items.value("covr").toCoverArtList();
    for(const CoverArt &picture : pictures) {
      String mimeType = "image/";
      switch(picture.format()) {
      case CoverArt::BMP:
        mimeType.append("bmp");
        break;
      case CoverArt::JPEG:
        mimeType.append("jpeg");
        break;
      case CoverArt::GIF:
        mimeType.append("gif");
        break;
      case CoverArt::PNG:
        mimeType.append("png");
        break;
      case CoverArt::Unknown:
        break;
      }

      VariantMap property;
      property.insert("data", picture.data());
      property.insert("mimeType", mimeType);
      properties.append(property);
    }
  }
  return properties;
}

bool MP4::Tag::setComplexProperties(const String &key, const List<VariantMap> &value)
{
  const String uppercaseKey = key.upper();
  if(uppercaseKey == "PICTURE") {
    CoverArtList pictures;
    for(auto property : value) {
      String mimeType = property.value("mimeType").value<String>();
      CoverArt::Format format;
      if(mimeType == "image/bmp") {
        format = CoverArt::BMP;
      } else if(mimeType == "image/png") {
        format = CoverArt::PNG;
      } else if(mimeType == "image/gif") {
        format = CoverArt::GIF;
      } else if(mimeType == "image/jpeg") {
        format = CoverArt::JPEG;
      } else {
        format = CoverArt::Unknown;
      }
      pictures.append(CoverArt(format, property.value("data").value<ByteVector>()));
    }
    d->items["covr"] = pictures;
  }
  else {
    return false;
  }
  return true;
}

void MP4::Tag::addItem(const String &name, const Item &value)
{
  if(!d->items.contains(name)) {
    d->items.insert(name, value);
  }
  else {
    debug("MP4: Ignoring duplicate atom \"" + name + "\"");
  }
}
