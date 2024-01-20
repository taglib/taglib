/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#include "tag.h"

#include <utility>

#include "tstringlist.h"
#include "tpropertymap.h"

using namespace TagLib;

class Tag::TagPrivate
{
};

Tag::Tag() = default;

Tag::~Tag() = default;

bool Tag::isEmpty() const
{
  return title().isEmpty() &&
         artist().isEmpty() &&
         album().isEmpty() &&
         comment().isEmpty() &&
         genre().isEmpty() &&
         year() == 0 &&
         track() == 0;
}

PropertyMap Tag::properties() const
{
  PropertyMap map;
  if(!title().isEmpty())
    map["TITLE"].append(title());
  if(!artist().isEmpty())
    map["ARTIST"].append(artist());
  if(!album().isEmpty())
    map["ALBUM"].append(album());
  if(!comment().isEmpty())
    map["COMMENT"].append(comment());
  if(!genre().isEmpty())
    map["GENRE"].append(genre());
  if(year() != 0)
    map["DATE"].append(String::number(year()));
  if(track() != 0)
    map["TRACKNUMBER"].append(String::number(track()));
  return map;
}

void Tag::removeUnsupportedProperties(const StringList&)
{
}

PropertyMap Tag::setProperties(const PropertyMap &origProps)
{
  PropertyMap props(origProps);
  props.removeEmpty();
  StringList oneValueSet;
  // can this be simplified by using some preprocessor defines / function pointers?
  if(props.contains("TITLE")) {
    setTitle(props["TITLE"].front());
    oneValueSet.append("TITLE");
  } else
    setTitle(String());

  if(props.contains("ARTIST")) {
    setArtist(props["ARTIST"].front());
    oneValueSet.append("ARTIST");
  } else
    setArtist(String());

  if(props.contains("ALBUM")) {
    setAlbum(props["ALBUM"].front());
    oneValueSet.append("ALBUM");
  } else
    setAlbum(String());

  if(props.contains("COMMENT")) {
    setComment(props["COMMENT"].front());
    oneValueSet.append("COMMENT");
  } else
    setComment(String());

  if(props.contains("GENRE")) {
    setGenre(props["GENRE"].front());
    oneValueSet.append("GENRE");
  } else
    setGenre(String());

  if(props.contains("DATE")) {
    bool ok;
    int date = props["DATE"].front().toInt(&ok);
    if(ok) {
      setYear(date);
      oneValueSet.append("DATE");
    } else
      setYear(0);
  }
  else
    setYear(0);

  if(props.contains("TRACKNUMBER")) {
    bool ok;
    int trackNumber = props["TRACKNUMBER"].front().toInt(&ok);
    if(ok) {
      setTrack(trackNumber);
      oneValueSet.append("TRACKNUMBER");
    } else
      setTrack(0);
  }
  else
    setTrack(0);

  // for each tag that has been set above, remove the first entry in the corresponding
  // value list. The others will be returned as unsupported by this format.
  for(const auto &entry : std::as_const(oneValueSet)) {
    if(props[entry].size() == 1)
      props.erase(entry);
    else
      props[entry].erase(props[entry].begin());
  }
  return props;
}

StringList Tag::complexPropertyKeys() const
{
  return StringList();
}

List<VariantMap> Tag::complexProperties(const String &) const
{
  return {};
}

bool Tag::setComplexProperties(const String &, const List<VariantMap> &)
{
  return false;
}

void Tag::duplicate(const Tag *source, Tag *target, bool overwrite) // static
{
  if(overwrite) {
    target->setTitle(source->title());
    target->setArtist(source->artist());
    target->setAlbum(source->album());
    target->setComment(source->comment());
    target->setGenre(source->genre());
    target->setYear(source->year());
    target->setTrack(source->track());
  }
  else {
    if(target->title().isEmpty())
      target->setTitle(source->title());
    if(target->artist().isEmpty())
      target->setArtist(source->artist());
    if(target->album().isEmpty())
      target->setAlbum(source->album());
    if(target->comment().isEmpty())
      target->setComment(source->comment());
    if(target->genre().isEmpty())
      target->setGenre(source->genre());
    if(target->year() == 0)
      target->setYear(source->year());
    if(target->track() == 0)
      target->setTrack(source->track());
  }
}

String Tag::joinTagValues(const StringList &values)
{
  return values.toString(" / ");
}
