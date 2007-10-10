/***************************************************************************
    copyright            : (C) 2003 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifndef DO_NOT_DOCUMENT // Tell Doxygen not to document this header

#ifndef TAGLIB_FLACTAG_H
#define TAGLIB_FLACTAG_H

////////////////////////////////////////////////////////////////////////////////
// Note that this header is not installed.
////////////////////////////////////////////////////////////////////////////////

#include <xiphcomment.h>
#include <id3v2tag.h>
#include <id3v1tag.h>

namespace TagLib {

  namespace FLAC {

    /*!
     * A union of Xiph, ID3v2 and ID3v1 tags.
     */
    class Tag : public TagLib::Tag
    {
    public:
      Tag(Ogg::XiphComment *xiph, ID3v2::Tag *id3v2 = 0, ID3v1::Tag *id3v1 = 0) :
        TagLib::Tag(),
        xiph(xiph), id3v2(id3v2), id3v1(id3v1) {}

      virtual String title() const {
        if(xiph && !xiph->title().isEmpty())
          return xiph->title();

        if(id3v2 && !id3v2->title().isEmpty())
          return id3v2->title();

        if(id3v1)
          return id3v1->title();

        return String::null;
      }

      virtual String artist() const {
        if(xiph && !xiph->artist().isEmpty())
          return xiph->artist();

        if(id3v2 && !id3v2->artist().isEmpty())
          return id3v2->artist();

        if(id3v1)
          return id3v1->artist();

        return String::null;
      }

      virtual String album() const {
        if(xiph && !xiph->album().isEmpty())
          return xiph->album();

        if(id3v2 && !id3v2->album().isEmpty())
          return id3v2->album();

        if(id3v1)
          return id3v1->album();

        return String::null;
      }

      virtual String comment() const {
        if(xiph && !xiph->comment().isEmpty())
          return xiph->comment();

        if(id3v2 && !id3v2->comment().isEmpty())
          return id3v2->comment();

        if(id3v1)
          return id3v1->comment();

        return String::null;
      }

      virtual String genre() const {
        if(xiph && !xiph->genre().isEmpty())
          return xiph->genre();

        if(id3v2 && !id3v2->genre().isEmpty())
          return id3v2->genre();

        if(id3v1)
          return id3v1->genre();

        return String::null;
      }

      virtual uint year() const {
        if(xiph && xiph->year() > 0)
          return xiph->year();

        if(id3v2 && id3v2->year() > 0)
          return id3v2->year();

        if(id3v1)
          return id3v1->year();

        return 0;
      }

      virtual uint track() const {
        if(xiph && xiph->track() > 0)
          return xiph->track();

        if(id3v2 && id3v2->track() > 0)
          return id3v2->track();

        if(id3v1)
          return id3v1->track();

        return 0;
      }

      virtual void setTitle(const String &s) {
        if(xiph)
          xiph->setTitle(s);
        if(id3v2)
          id3v2->setTitle(s);
        if(id3v1)
          id3v1->setTitle(s);
      }

      virtual void setArtist(const String &s) {
        if(xiph)
          xiph->setArtist(s);
        if(id3v2)
          id3v2->setArtist(s);
        if(id3v1)
          id3v1->setArtist(s);
      }

      virtual void setAlbum(const String &s) {
        if(xiph)
          xiph->setAlbum(s);
        if(id3v2)
          id3v2->setAlbum(s);
        if(id3v1)
          id3v1->setAlbum(s);
      }

      virtual void setComment(const String &s) {
        if(xiph)
          xiph->setComment(s);
        if(id3v2)
          id3v2->setComment(s);
        if(id3v1)
          id3v1->setComment(s);
      }

      virtual void setGenre(const String &s) {
        if(xiph)
          xiph->setGenre(s);
        if(id3v2)
          id3v2->setGenre(s);
        if(id3v1)
          id3v1->setGenre(s);
      }

      virtual void setYear(uint i) {
        if(xiph)
          xiph->setYear(i);
        if(id3v2)
          id3v2->setYear(i);
        if(id3v1)
          id3v1->setYear(i);
      }

      virtual void setTrack(uint i) {
        if(xiph)
          xiph->setTrack(i);
        if(id3v2)
          id3v2->setTrack(i);
        if(id3v1)
          id3v1->setTrack(i);
      }

    private:
      Ogg::XiphComment *xiph;
      ID3v2::Tag *id3v2;
      ID3v1::Tag *id3v1;
    };
  }
}

#endif
#endif
