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

#include "tagunion.h"
#include "tstringlist.h"
#include "tpropertymap.h"

using namespace TagLib;

namespace
{
  typedef std::vector<TAGLIB_SHARED_PTR<Tag> > TagVector;
  typedef TagVector::iterator TagIterator;
  typedef TagVector::const_iterator TagConstIterator;
}

#define stringUnion(method)                                               \
  for(TagConstIterator it = d->tags.begin(); it != d->tags.end(); ++it) { \
    String val = (*it) ? (*it)->method() : String::null;                  \
    if(!val.isEmpty())                                                    \
      return val;                                                         \
  }                                                                       \
  return String::null;

#define numberUnion(method)                                               \
  for(TagConstIterator it = d->tags.begin(); it != d->tags.end(); ++it) { \
    uint val = (*it) ? (*it)->method() : 0;                               \
    if(val > 0)                                                           \
      return val;                                                         \
  }                                                                       \
  return 0;

#define setUnion(method, value)                                           \
  for(TagIterator it = d->tags.begin(); it != d->tags.end(); ++it) {      \
    if(*it)                                                               \
      (*it)->set##method(value);                                          \
  }

class TagUnion::TagUnionPrivate
{
public:
  TagUnionPrivate(size_t count) 
    : tags(count)
  {
  }

  std::vector<TAGLIB_SHARED_PTR<Tag> > tags;
};

TagUnion::TagUnion(size_t count)
  : d(new TagUnionPrivate(count))
{
}

TagUnion::~TagUnion()
{
}

Tag *TagUnion::operator[](size_t index) const
{
  return tag(index);
}

Tag *TagUnion::tag(size_t index) const
{
  return d->tags[index].get();
}

void TagUnion::set(size_t index, Tag *tag)
{
  d->tags[index].reset(tag);
}

PropertyMap TagUnion::properties() const
{
  for(TagConstIterator it = d->tags.begin(); it != d->tags.end(); ++it) {
    if(*it)
      return (*it)->properties();
  }

  return PropertyMap();
}

void TagUnion::removeUnsupportedProperties(const StringList &unsupported)
{
  for(TagIterator it = d->tags.begin(); it != d->tags.end(); ++it) {
    if(*it)
      (*it)->removeUnsupportedProperties(unsupported);
  }
}
  

String TagUnion::title() const
{
  stringUnion(title);
}

String TagUnion::artist() const
{
  stringUnion(artist);
}

String TagUnion::album() const
{
  stringUnion(album);
}

String TagUnion::comment() const
{
  stringUnion(comment);
}

String TagUnion::genre() const
{
  stringUnion(genre);
}

TagLib::uint TagUnion::year() const
{
  numberUnion(year);
}

TagLib::uint TagUnion::track() const
{
  numberUnion(track);
}

void TagUnion::setTitle(const String &s)
{
  setUnion(Title, s);
}

void TagUnion::setArtist(const String &s)
{
  setUnion(Artist, s);
}

void TagUnion::setAlbum(const String &s)
{
  setUnion(Album, s);
}

void TagUnion::setComment(const String &s)
{
  setUnion(Comment, s);
}

void TagUnion::setGenre(const String &s)
{
  setUnion(Genre, s);
}

void TagUnion::setYear(uint i)
{
  setUnion(Year, i);
}

void TagUnion::setTrack(uint i)
{
  setUnion(Track, i);
}

bool TagUnion::isEmpty() const
{
  for(TagIterator it = d->tags.begin(); it != d->tags.end(); ++it) {
    if(*it && !(*it)->isEmpty())
      return false;
  }

  return true;
}

