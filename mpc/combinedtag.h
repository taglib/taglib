/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#ifndef DO_NOT_DOCUMENT // Tell Doxygen not to document this header

#ifndef TAGLIB_COMBINEDTAG_H
#define TAGLIB_COMBINEDTAG_H

////////////////////////////////////////////////////////////////////////////////
// Note that this header is not installed.
////////////////////////////////////////////////////////////////////////////////

#include <tag.h>

namespace TagLib {

  /*!
   * A union of two TagLib::Tags.
   */
  class CombinedTag : public TagLib::Tag
  {
  public:
    CombinedTag(Tag *tag1 = 0, Tag *tag2 = 0)
               : TagLib::Tag(),
                 tag1(tag1), tag2(tag2) {}

    virtual String title() const {
      if(tag1 && !tag1->title().isEmpty())
          return tag1->title();

      if(tag2)
          return tag2->title();

      return String::null;
    }

    virtual String artist() const {
      if(tag1 && !tag1->artist().isEmpty())
          return tag1->artist();

      if(tag2)
          return tag2->artist();

      return String::null;
    }

    virtual String album() const {
      if(tag1 && !tag1->album().isEmpty())
          return tag1->album();

      if(tag2)
          return tag2->album();

      return String::null;
    }

    virtual String comment() const {
      if(tag1 && !tag1->comment().isEmpty())
          return tag1->comment();

      if(tag2)
          return tag2->comment();

      return String::null;
    }

    virtual String genre() const {
      if(tag1 && !tag1->genre().isEmpty())
          return tag1->genre();

      if(tag2)
          return tag2->genre();

      return String::null;
    }

    virtual uint year() const {
      if(tag1 && tag1->year() > 0)
          return tag1->year();

      if(tag2)
          return tag2->year();

      return 0;
    }

    virtual uint track() const {
      if(tag1 && tag1->track() > 0)
          return tag1->track();

      if(tag2)
          return tag2->track();

      return 0;
    }

    virtual void setTitle(const String &s) {
      if(tag1)
          tag1->setTitle(s);
      if(tag2)
          tag2->setTitle(s);
    }

    virtual void setArtist(const String &s) {
      if(tag1)
          tag1->setArtist(s);
      if(tag2)
          tag2->setArtist(s);
    }

    virtual void setAlbum(const String &s) {
      if(tag1)
          tag1->setAlbum(s);
      if(tag2)
          tag2->setAlbum(s);
    }

    virtual void setComment(const String &s) {
      if(tag1)
          tag1->setComment(s);
      if(tag2)
          tag2->setComment(s);
    }

    virtual void setGenre(const String &s) {
      if(tag1)
          tag1->setGenre(s);
      if(tag2)
          tag2->setGenre(s);
    }

    virtual void setYear(uint i) {
      if(tag1)
          tag1->setYear(i);
      if(tag2)
          tag2->setYear(i);
    }

    virtual void setTrack(uint i) {
      if(tag1)
          tag1->setTrack(i);
      if(tag2)
          tag2->setTrack(i);
    }

  private:
      Tag *tag1;
      Tag *tag2;
  };
}

#endif
#endif
