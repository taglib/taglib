/***************************************************************************
 copyright            : (C) 2016 by Damien Plisson, Audirvana
 email                : damien78@audirvana.com
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

#ifndef TAGLIB_DSDIFFDIINTAG_H
#define TAGLIB_DSDIFFDIINTAG_H

#include "tag.h"

namespace TagLib {

  namespace DSDIFF {

    namespace DIIN {

      /*!
       * Tags from the Edited Master Chunk Info
       *
       * Only Title and Artist tags are supported
       */
      class TAGLIB_EXPORT Tag : public TagLib::Tag
      {
      public:
        Tag();
        virtual ~Tag();

        /*!
         * Returns the track name; if no track name is present in the tag
         * String() will be returned.
         */
        virtual String title() const;

        /*!
         * Returns the artist name; if no artist name is present in the tag
         * String() will be returned.
         */
        virtual String artist() const;

        /*!
         * Not supported.  Therefore always returns String().
         */
        virtual String album() const;

        /*!
         * Not supported.  Therefore always returns String().
         */
        virtual String comment() const;

        /*!
         * Not supported.  Therefore always returns String().
         */
        virtual String genre() const;

        /*!
         * Not supported.  Therefore always returns 0.
         */
        virtual unsigned int year() const;

        /*!
         * Not supported.  Therefore always returns 0.
         */
        virtual unsigned int track() const;

        /*!
         * Not supported.  Therefore always returns an empty list.
         */
        virtual PictureMap pictures() const;

        /*!
         * Sets the title to \a title.  If \a title is String() then this
         * value will be cleared.
         */
        virtual void setTitle(const String &title);

        /*!
         * Sets the artist to \a artist.  If \a artist is String() then this
         * value will be cleared.
         */
        virtual void setArtist(const String &artist);

        /*!
         * Not supported and therefore ignored.
         */
        virtual void setAlbum(const String &album);

        /*!
         * Not supported and therefore ignored.
         */
        virtual void setComment(const String &comment);

        /*!
         * Not supported and therefore ignored.
         */
        virtual void setGenre(const String &genre);

        /*!
         * Not supported and therefore ignored.
         */
        virtual void setYear(unsigned int year);

        /*!
         * Not supported and therefore ignored.
         */
        virtual void setTrack(unsigned int track);

        /*!
         * Not supported and therefore ignored.
         */
        virtual void setPictures( const PictureMap& l );

        /*!
         * Implements the unified property interface -- export function.
         * Since the DIIN tag is very limited, the exported map is as well.
         */
        PropertyMap properties() const;

        /*!
         * Implements the unified property interface -- import function.
         * Because of the limitations of the DIIN file tag, any tags besides
         * TITLE and ARTIST, will be
         * returned. Additionally, if the map contains tags with multiple values,
         * all but the first will be contained in the returned map of unsupported
         * properties.
         */
        PropertyMap setProperties(const PropertyMap &);

      private:
        Tag(const Tag &);
        Tag &operator=(const Tag &);

        class TagPrivate;
        TagPrivate *d;
      };
    }
  }
}

#endif

