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

#include <utility>

#include "tdebug.h"
#include "tpropertymap.h"
#include "mp4itemfactory.h"
#include "mp4atom.h"
#include "mp4coverart.h"

using namespace TagLib;

class MP4::Tag::TagPrivate
{
public:
  TagPrivate(const ItemFactory *itemFactory) :
    factory(itemFactory ? itemFactory
                        : ItemFactory::instance())
  {
  }

  ~TagPrivate() = default;

  const ItemFactory *factory;
  TagLib::File *file { nullptr };
  Atoms *atoms { nullptr };
  ItemMap items;
};

MP4::Tag::Tag() :
  d(std::make_unique<TagPrivate>(ItemFactory::instance()))
{
}

MP4::Tag::Tag(TagLib::File *file, MP4::Atoms *atoms,
              const MP4::ItemFactory *factory) :
  d(std::make_unique<TagPrivate>(factory))
{
  d->file = file;
  d->atoms = atoms;

  const MP4::Atom *ilst = atoms->find("moov", "udta", "meta", "ilst");
  if(!ilst) {
    //debug("Atom moov.udta.meta.ilst not found.");
    return;
  }

  for(const auto &atom : ilst->children()) {
    file->seek(atom->offset() + 8);
    ByteVector data = d->file->readBlock(atom->length() - 8);
    if(const auto &[name, itm] = d->factory->parseItem(atom, data);
       itm.isValid()) {
      addItem(name, itm);
    }
  }
}

MP4::Tag::~Tag() = default;

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

bool
MP4::Tag::save()
{
  ByteVector data;
  for(const auto &[name, itm] : std::as_const(d->items)) {
    data.append(d->factory->renderItem(name, itm));
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
    d->file->seek((*it)->offset());
    long size = d->file->readBlock(4).toUInt();
    // 64-bit
    if (size == 1) {
      d->file->seek(4, File::Current); // Skip name
      long long longSize = d->file->readBlock(8).toLongLong();
      // Seek the offset of the 64-bit size
      d->file->seek((*it)->offset() + 8);
      d->file->writeBlock(ByteVector::fromLongLong(longSize + delta));
    }
    // 32-bit
    else {
      d->file->seek((*it)->offset());
      d->file->writeBlock(ByteVector::fromUInt(static_cast<unsigned int>(size + delta)));
    }
  }
}

void
MP4::Tag::updateOffsets(offset_t delta, offset_t offset)
{
  if(MP4::Atom *moov = d->atoms->find("moov")) {
    const MP4::AtomList stco = moov->findall("stco", true);
    for(const auto &atom : stco) {
      if(atom->offset() > offset) {
        atom->addToOffset(delta);
      }
      d->file->seek(atom->offset() + 12);
      ByteVector data = d->file->readBlock(atom->length() - 12);
      unsigned int count = data.toUInt();
      d->file->seek(atom->offset() + 16);
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
      if(atom->offset() > offset) {
        atom->addToOffset(delta);
      }
      d->file->seek(atom->offset() + 12);
      ByteVector data = d->file->readBlock(atom->length() - 12);
      unsigned int count = data.toUInt();
      d->file->seek(atom->offset() + 16);
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

  if(MP4::Atom *moof = d->atoms->find("moof")) {
    const MP4::AtomList tfhd = moof->findall("tfhd", true);
    for(const auto &atom : tfhd) {
      if(atom->offset() > offset) {
        atom->addToOffset(delta);
      }
      d->file->seek(atom->offset() + 9);
      ByteVector data = d->file->readBlock(atom->length() - 9);
      if(const unsigned int flags = data.toUInt(0, 3, true);
         flags & 1) {
        long long o = data.toLongLong(7U);
        if(o > offset) {
          o += delta;
        }
        d->file->seek(atom->offset() + 16);
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

  offset_t offset = path.back()->offset() + 8;
  d->file->insert(data, offset, 0);

  updateParents(path, data.size());
  updateOffsets(data.size(), offset);

  // Insert the newly created atoms into the tree to keep it up-to-date.

  d->file->seek(offset);
  path.back()->prependChild(new Atom(d->file));
}

void
MP4::Tag::saveExisting(ByteVector data, const AtomList &path)
{
  auto it = path.end();

  MP4::Atom *ilst = *(--it);
  offset_t offset = ilst->offset();
  offset_t length = ilst->length();

  MP4::Atom *meta = *(--it);
  auto index = meta->children().cfind(ilst);

  // check if there is an atom before 'ilst', and possibly use it as padding
  if(index != meta->children().cbegin()) {
    auto prevIndex = std::prev(index);
    if(const MP4::Atom *prev = *prevIndex; prev->name() == "free") {
      offset = prev->offset();
      length += prev->length();
    }
  }
  // check if there is an atom after 'ilst', and possibly use it as padding
  auto nextIndex = std::next(index);
  if(nextIndex != meta->children().cend()) {
    if(const MP4::Atom *next = *nextIndex; next->name() == "free") {
      length += next->length();
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
    if(MP4::Atom *udta = *std::prev(it); udta->removeChild(meta)) {
      offset = meta->offset();
      delta = - meta->length();
      d->file->removeBlock(meta->offset(), meta->length());
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

PropertyMap MP4::Tag::properties() const
{
  PropertyMap props;
  for(const auto &[k, t] : std::as_const(d->items)) {
    if(auto [key, val] = d->factory->itemToProperty(k.data(String::Latin1), t);
       !key.isEmpty()) {
      props[key] = val;
    }
    else {
      props.addUnsupportedData(k);
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
  const PropertyMap origProps = properties();
  for(const auto &[prop, _] : origProps) {
    if(!props.contains(prop) || props[prop].isEmpty()) {
      d->items.erase(d->factory->nameForPropertyKey(prop));
    }
  }

  PropertyMap ignoredProps;
  for(const auto &[prop, val] : props) {
    if(auto [name, itm] = d->factory->itemFromProperty(prop, val);
       itm.isValid()) {
      d->items[name] = itm;
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
  List<VariantMap> props;
  if(const String uppercaseKey = key.upper(); uppercaseKey == "PICTURE") {
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
      props.append(property);
    }
  }
  return props;
}

bool MP4::Tag::setComplexProperties(const String &key, const List<VariantMap> &value)
{
  if(const String uppercaseKey = key.upper(); uppercaseKey == "PICTURE") {
    CoverArtList pictures;
    for(const auto &property : value) {
      auto mimeType = property.value("mimeType").value<String>();
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
