/***************************************************************************
 copyright           : (C) 2020-2024 Stephen F. Booth
 email               : me@sbooth.org
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

#include "shortentag.h"

#include "tpropertymap.h"

using namespace TagLib;

class Shorten::Tag::TagPrivate
{
};

Shorten::Tag::Tag() :
  d(std::make_unique<TagPrivate>())
{
}

Shorten::Tag::~Tag() = default;

String Shorten::Tag::title() const
{
  return String();
}

String Shorten::Tag::artist() const
{
  return String();
}

String Shorten::Tag::album() const
{
  return String();
}

String Shorten::Tag::comment() const
{
  return String();
}

String Shorten::Tag::genre() const
{
  return String();
}

unsigned int Shorten::Tag::year() const
{
  return 0;
}

unsigned int Shorten::Tag::track() const
{
  return 0;
}

void Shorten::Tag::setTitle(const String &)
{
}

void Shorten::Tag::setArtist(const String &)
{
}

void Shorten::Tag::setAlbum(const String &)
{
}

void Shorten::Tag::setComment(const String &)
{
}

void Shorten::Tag::setGenre(const String &)
{
}

void Shorten::Tag::setYear(unsigned int)
{
}

void Shorten::Tag::setTrack(unsigned int)
{
}

PropertyMap Shorten::Tag::properties() const
{
  return PropertyMap{};
}

PropertyMap Shorten::Tag::setProperties(const PropertyMap &origProps)
{
  return PropertyMap{origProps};
}
