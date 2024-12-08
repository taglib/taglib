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

#include "shntag.h"

#include "tpropertymap.h"

using namespace TagLib;

class SHN::Tag::TagPrivate
{
};

SHN::Tag::Tag() :
d(std::make_unique<TagPrivate>())
{
}

SHN::Tag::~Tag() = default;

String SHN::Tag::title() const
{
  return String();
}

String SHN::Tag::artist() const
{
  return String();
}

String SHN::Tag::album() const
{
  return String();
}

String SHN::Tag::comment() const
{
  return String();
}

String SHN::Tag::genre() const
{
  return String();
}

unsigned int SHN::Tag::year() const
{
  return 0;
}

unsigned int SHN::Tag::track() const
{
  return 0;
}

void SHN::Tag::setTitle(const String &)
{
}

void SHN::Tag::setArtist(const String &)
{
}

void SHN::Tag::setAlbum(const String &)
{
}

void SHN::Tag::setComment(const String &)
{
}

void SHN::Tag::setGenre(const String &)
{
}

void SHN::Tag::setYear(unsigned int)
{
}

void SHN::Tag::setTrack(unsigned int)
{
}

PropertyMap SHN::Tag::properties() const
{
  return PropertyMap{};
}

PropertyMap SHN::Tag::setProperties(const PropertyMap &origProps)
{
  return PropertyMap{origProps};
}
