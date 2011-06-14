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
    class TAGLIB_EXPORT Tag : public TagLib::Tag {
      public:
        Tag();
        virtual ~Tag();

        String title()   const;
        String artist()  const;
        String album()   const;
        String comment() const;
        String genre()   const;
        uint   year()    const;
        uint   track()   const;
		String trackerName() const;

        void setTitle  (const String &title);
        void setArtist (const String &artist);
        void setAlbum  (const String &album);
        void setComment(const String &comment);
        void setGenre  (const String &genre);
        void setYear (uint year);
        void setTrack(uint track);
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
