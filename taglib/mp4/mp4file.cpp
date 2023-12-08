/**************************************************************************
    copyright            : (C) 2007 by Lukáš Lalinský
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

#include "mp4file.h"

#include "tdebug.h"
#include "tpropertymap.h"
#include "tagutils.h"

#include "mp4itemfactory.h"

using namespace TagLib;

namespace
{
  bool checkValid(const MP4::AtomList &list)
  {
    return std::none_of(list.begin(), list.end(),
      [](const auto &a) { return a->length == 0 || !checkValid(a->children); });
  }

  bool checkRootLevelAtoms(MP4::AtomList &list)
  {
    bool moovValid = false;
    for(auto it = list.begin(); it != list.end(); ++it) {
      bool invalid = (*it)->length == 0 || !checkValid((*it)->children);
      if(!moovValid && !invalid && (*it)->name == "moov") {
        moovValid = true;
      }
      if(invalid) {
        if(moovValid && (*it)->name != "moof") {
          // Only the root level atoms "moov" and (if present) "moof" are
          // modified.  If they are valid, ignore following invalid root level
          // atoms as trailing garbage.
          while(it != list.end()) {
            delete *it;
            it = list.erase(it);
          }
          return true;
        }
        else {
          return false;
        }
      }
    }

    return true;
  }

}  // namespace

class MP4::File::FilePrivate
{
public:
  FilePrivate(MP4::ItemFactory *mp4ItemFactory)
        : itemFactory(mp4ItemFactory ? mp4ItemFactory
                                     : MP4::ItemFactory::instance())
  {
  }

  ~FilePrivate() = default;

  const ItemFactory *itemFactory;
  std::unique_ptr<MP4::Tag> tag;
  std::unique_ptr<MP4::Atoms> atoms;
  std::unique_ptr<MP4::Properties> properties;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

bool MP4::File::isSupported(IOStream *stream)
{
  // An MP4 file has to have an "ftyp" box first.

  const ByteVector id = Utils::readHeader(stream, 8, false);
  return id.containsAt("ftyp", 4);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MP4::File::File(FileName file, bool readProperties, AudioProperties::ReadStyle,
                ItemFactory *itemFactory) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(itemFactory))
{
  if(isOpen())
    read(readProperties);
}

MP4::File::File(IOStream *stream, bool readProperties, AudioProperties::ReadStyle,
                ItemFactory *itemFactory) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(itemFactory))
{
  if(isOpen())
    read(readProperties);
}

MP4::File::~File() = default;

MP4::Tag *MP4::File::tag() const
{
  return d->tag.get();
}

PropertyMap MP4::File::properties() const
{
  return d->tag->properties();
}

void MP4::File::removeUnsupportedProperties(const StringList &properties)
{
  d->tag->removeUnsupportedProperties(properties);
}

PropertyMap MP4::File::setProperties(const PropertyMap &properties)
{
  return d->tag->setProperties(properties);
}

MP4::Properties *MP4::File::audioProperties() const
{
  return d->properties.get();
}

void
MP4::File::read(bool readProperties)
{
  if(!isValid())
    return;

  d->atoms = std::make_unique<Atoms>(this);
  if(!checkRootLevelAtoms(d->atoms->atoms)) {
    setValid(false);
    return;
  }

  // must have a moov atom, otherwise consider it invalid
  if(!d->atoms->find("moov")) {
    setValid(false);
    return;
  }

  d->tag = std::make_unique<Tag>(this, d->atoms.get(), d->itemFactory);
  if(readProperties) {
    d->properties = std::make_unique<Properties>(this, d->atoms.get());
  }
}

bool
MP4::File::save()
{
  if(readOnly()) {
    debug("MP4::File::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    debug("MP4::File::save() -- Trying to save invalid file.");
    return false;
  }

  return d->tag->save();
}

bool
MP4::File::strip(int tags)
{
  if(readOnly()) {
    debug("MP4::File::strip() - Cannot strip tags from a read only file.");
    return false;
  }

  if(!isValid()) {
    debug("MP4::File::strip() -- Cannot strip tags from an invalid file.");
    return false;
  }

  if(tags & MP4) {
    return d->tag->strip();
  }

  return true;
}

bool
MP4::File::hasMP4Tag() const
{
  return (d->atoms->find("moov", "udta", "meta", "ilst") != nullptr);
}
