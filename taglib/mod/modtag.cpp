/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#include "modtag.h"

using namespace TagLib;
using namespace Mod;

class Mod::Tag::TagPrivate
{
public:
  TagPrivate()
  {
  }

  String title;
  String comment;
  String trackerName;
};

Mod::Tag::Tag() : TagLib::Tag()
{
  d = new TagPrivate;
}

Mod::Tag::~Tag()
{
  delete d;
}

String Mod::Tag::title() const
{
  return d->title;
}

String Mod::Tag::artist() const
{
  return String::null;
}

String Mod::Tag::album() const
{
  return String::null;
}

String Mod::Tag::comment() const
{
  return d->comment;
}

String Mod::Tag::genre() const
{
  return String::null;
}

uint Mod::Tag::year() const
{
  return 0;
}

uint Mod::Tag::track() const
{
  return 0;
}

String Mod::Tag::trackerName() const
{
  return d->trackerName;
}

void Mod::Tag::setTitle(const String &title)
{
  d->title = title;
}

void Mod::Tag::setArtist(const String &)
{
}

void Mod::Tag::setAlbum(const String &)
{
}

void Mod::Tag::setComment(const String &comment)
{
  d->comment = comment;
}

void Mod::Tag::setGenre(const String &)
{
}

void Mod::Tag::setYear(uint)
{
}

void Mod::Tag::setTrack(uint)
{
}

void Mod::Tag::setTrackerName(const String &trackerName)
{
  d->trackerName = trackerName;
}
