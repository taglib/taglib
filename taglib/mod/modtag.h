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

#ifndef TAGLIB_MODTAG_H
#define TAGLIB_MODTAG_H

#include "tag.h"

namespace TagLib {
  namespace Mod {
    /*!
     * Tags for module files (mod, s3m, it, xm).
     *
     * Note that only the \a title is supported as such by most
     * module file formats.  Except for xm files the \a trackerName
     * is derived from the file format or the flavour of the file
     * format.  For xm files it is stored in the file.
     *
     * The \a comment tag is not strictly supported by module files,
     * but it is common practice to abuse instrument/sample/pattern
     * names as multiline comments.  TagLib does so as well, but
     * currently does not support writing them.
     */
    class TAGLIB_EXPORT Tag : public TagLib::Tag
    {
    public:
      Tag();
      virtual ~Tag();

      /*!
       * Returns the track name; if no track name is present in the tag
       * String::null will be returned.
       */
      String title() const;

      /*!
       * Not supported by module files.  Therefore always returns String::null.
       */
      String artist() const;

      /*!
       * Not supported by module files.  Therefore always returns String::null.
       */
      String album() const;

      /*!
       * Returns the track comment derived from the instrument/sample/pattern
       * names; if no comment is present in the tag String::null will be
       * returned.
       */
      String comment() const;

      /*!
       * Not supported by module files.  Therefore always returns String::null.
       */
      String genre() const;

      /*!
       * Not supported by module files.  Therefore always returns 0.
       */
      uint year() const;

      /*!
       * Not supported by module files.  Therefore always returns 0.
       */
      uint track() const;

      /*!
       * Returns the name of the tracker used to create/edit the module file.
       * Only xm files store this tag to the file as such, for other formats
       * (mod, s3m, it) this is derived from the file type or the flavour of
       * the file type.  Therefore only xm file might have an empty
       * (String::null) tracker name.
       */
      String trackerName() const;

      /*!
       * Sets the title to \a title.  If \a title is String::null then this
       * value will be cleared.
       */
      void setTitle(const String &title);

      /*!
       * Not supported by module files and therefore ignored.
       */
      void setArtist(const String &artist);

      /*!
       * Not supported by module files and therefore ignored.
       */
      void setAlbum(const String &album);

      /*!
       * Not yet supported.
       */
      void setComment(const String &comment);

      /*!
       * Not supported by module files and therefore ignored.
       */
      void setGenre(const String &genre);

      /*!
       * Not supported by module files and therefore ignored.
       */
      void setYear(uint year);

      /*!
       * Not supported by module files and therefore ignored.
       */
      void setTrack(uint track);

      /*!
       * Sets the tracker name to \a trackerName.  If \a trackerName is
       * String::null then this value will be cleared.
       * 
       * Note that only xm files support this tag.  Setting the
       * tracker name for other module file formats will be ignored.
       */
      void setTrackerName(const String &trackerName);

    private:
      Tag(const Tag &);
      Tag &operator=(const Tag &);

      class TagPrivate;
      TagPrivate *d;
    };
  }
}

#endif
