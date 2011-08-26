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
#include "tstringlist.h"
using namespace TagLib;

class Tag::TagPrivate
{

};

Tag::Tag()
{

}

Tag::~Tag()
{

}

bool Tag::isEmpty() const
{
  return (title().isEmpty() &&
          artist().isEmpty() &&
          album().isEmpty() &&
          comment().isEmpty() &&
          genre().isEmpty() &&
          year() == 0 &&
          track() == 0);
}

TagDict Tag::toDict() const
{
  TagDict dict;
  if (!(title() == String::null))
    dict["TITLE"].append(title());
  if (!(artist() == String::null))
    dict["ARTIST"].append(artist());
  if (!(album() == String::null))
    dict["ALBUM"].append(album());
  if (!(comment() == String::null))
    dict["COMMENT"].append(comment());
  if (!(genre() == String::null))
    dict["GENRE"].append(genre());
  if (!(year() == 0))
    dict["DATE"].append(String::number(year()));
  if (!(track() == 0))
    dict["TRACKNUMBER"].append(String::number(track()));
  return dict;
}

void Tag::fromDict(const TagDict &dict)
{
  if (dict.contains("TITLE") and dict["TITLE"].size() >= 1)
    setTitle(dict["TITLE"].front());
  else
    setTitle(String::null);

  if (dict.contains("ARTIST") and dict["ARTIST"].size() >= 1)
    setArtist(dict["ARTIST"].front());
  else
    setArtist(String::null);

  if (dict.contains("ALBUM") and dict["ALBUM"].size() >= 1)
      setAlbum(dict["ALBUM"].front());
    else
      setAlbum(String::null);

  if (dict.contains("COMMENT") and dict["COMMENT"].size() >= 1)
    setComment(dict["COMMENT"].front());
  else
    setComment(String::null);

  if (dict.contains("GENRE") and dict["GENRE"].size() >=1)
    setGenre(dict["GENRE"].front());
  else
    setGenre(String::null);

  if (dict.contains("DATE") and dict["DATE"].size() >= 1) {
    bool ok;
    int date = dict["DATE"].front().toInt(&ok);
    if (ok)
      setYear(date);
    else
      setYear(0);
  }
  else
    setYear(0);

  if (dict.contains("TRACKNUMBER") and dict["TRACKNUMBER"].size() >= 1) {
    bool ok;
    int track = dict["TRACKNUMBER"].front().toInt(&ok);
    if (ok)
      setTrack(track);
    else
      setTrack(0);
  }
  else
    setYear(0);
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
    if(target->year() <= 0)
      target->setYear(source->year());
    if(target->track() <= 0)
      target->setTrack(source->track());
  }
}
