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

#include <taglib/tag.h>

namespace TagLib {
	namespace Mod {
		class TAGLIB_EXPORT Tag : public TagLib::Tag {
			public:
				String title()   const { return m_title; }
				String artist()  const { return String::null; }
				String album()   const { return String::null; }
				String comment() const { return m_comment; }
				String genre()   const { return String::null; }
				uint   year()    const { return 0; }
				uint   track()   const { return 0; }

				void setTitle  (const String &title) { m_title = title; }
				void setArtist (const String &) {}
				void setAlbum  (const String &) {}
				void setComment(const String &comment) { m_comment = comment; }
				void setGenre  (const String &) {}
				void setYear (uint) {}
				void setTrack(uint) {}

			private:
				String m_title;
				String m_comment;
		};
	}
}

#endif
