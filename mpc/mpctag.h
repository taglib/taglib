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

#ifndef TAGLIB_MPCTAG_H
#define TAGLIB_MPCTAG_H

////////////////////////////////////////////////////////////////////////////////
// Note that this header is not installed.
////////////////////////////////////////////////////////////////////////////////

#include <apetag.h>
#include <id3v1tag.h>

namespace TagLib {

  namespace MPC {

    /*!
     * A union of APE and ID3v1 tags.
     */
    class CombinedTag : public TagLib::Tag
    {
    public:
      CombinedTag(APE::Tag *ape = 0, ID3v1::Tag *id3v1 = 0) :
        TagLib::Tag(),
        ape(ape), id3v1(id3v1) {}

      virtual String title() const {
        if(ape && !ape->title().isEmpty())
          return ape->title();

        if(id3v1)
          return id3v1->title();

        return String::null;
      }

      virtual String artist() const {
        if(ape && !ape->artist().isEmpty())
          return ape->artist();

        if(id3v1)
          return id3v1->artist();

        return String::null;
      }

      virtual String album() const {
        if(ape && !ape->album().isEmpty())
          return ape->album();

        if(id3v1)
          return id3v1->album();

        return String::null;
      }

      virtual String comment() const {
        if(ape && !ape->comment().isEmpty())
          return ape->comment();

        if(id3v1)
          return id3v1->comment();

        return String::null;
      }

      virtual String genre() const {
        if(ape && !ape->genre().isEmpty())
          return ape->genre();

        if(id3v1)
          return id3v1->genre();

        return String::null;
      }

      virtual uint year() const {
        if(ape && ape->year() > 0)
          return ape->year();

        if(id3v1)
          return id3v1->year();

        return 0;
      }

      virtual uint track() const {
        if(ape && ape->track() > 0)
          return ape->track();

        if(id3v1)
          return id3v1->track();

        return 0;
      }

      virtual void setTitle(const String &s) {
        if(ape)
          ape->setTitle(s);
        if(id3v1)
          id3v1->setTitle(s);
      }

      virtual void setArtist(const String &s) {
        if(ape)
          ape->setArtist(s);
        if(id3v1)
          id3v1->setArtist(s);
      }

      virtual void setAlbum(const String &s) {
        if(ape)
          ape->setAlbum(s);
        if(id3v1)
          id3v1->setAlbum(s);
      }

      virtual void setComment(const String &s) {
        if(ape)
          ape->setComment(s);
        if(id3v1)
          id3v1->setComment(s);
      }

      virtual void setGenre(const String &s) {
        if(ape)
          ape->setGenre(s);
        if(id3v1)
          id3v1->setGenre(s);
      }

      virtual void setYear(uint i) {
        if(ape)
          ape->setYear(i);
        if(id3v1)
          id3v1->setYear(i);
      }

      virtual void setTrack(uint i) {
        if(ape)
          ape->setTrack(i);
        if(id3v1)
          id3v1->setTrack(i);
      }

    private:
      APE::Tag *ape;
      ID3v1::Tag *id3v1;
    };
  }
}

#endif
#endif
