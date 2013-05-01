/***************************************************************************
    copyright            : (C) 2013 by Sebastian Rachuj
    email                : rachus@web.de
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

#ifndef TAGLIB_EBMLMATROSKACONSTANTS_H
#define TAGLIB_EBMLMATROSKACONSTANTS_H

#include "ebmlconstants.h"
#include "tstring.h"

namespace TagLib {

  namespace EBML {
    
    namespace Matroska {
        
      namespace Constants {
        
        //! ID of an Matroska segment.
        const ulli Segment = 0x18538067L;
        
        //! ID of the tags element.
        const ulli Tags = 0x1254c367L;
        
        //! ID of the tag element.
        const ulli Tag = 0x7373L;
        
        //! ID of the targets element.
        const ulli Targets = 0x63c0L;
        
        //! ID of the target type value element.
        const ulli TargetTypeValue = 0x68caL;
        
        //! ID of the target type element.
        const ulli TargetType = 0x63caL;
        
        //! ID of a simple tag element.
        const ulli SimpleTag = 0x67c8L;
        
        //! ID of the tag name.
        const ulli TagName = 0x45a3L;
        
        //! ID of the tag content.
        const ulli TagString = 0x4487L;
        
        //! The DocType of a matroska file.
        const String DocTypeMatroska = "matroska";
        
        //! The DocType of a WebM file.
        const String DocTypeWebM = "webm";
        
        
        
        //! The TITLE entry
        const String TITLE = "TITLE";
        
        //! The ARTIST entry
        const String ARTIST = "ARTIST";
        
        //! The COMMENT entry
        const String COMMENT = "COMMENT";
        
        //! The GENRE entry
        const String GENRE = "GENRE";
        
        //! The DATE_RELEASE entry
        const String DATE_RELEASE = "DATE_RELEASE";
        
        //! The PART_NUMBER entry
        const String PART_NUMBER = "PART_NUMBER";
        
        //! The TargetTypeValue of the most common grouping level (e.g. album)
        const ulli MostCommonGroupingValue = 50;
        
        //! The TargetTypeValue of the most common parts of a group (e.g. track)
        const ulli MostCommonPartValue = 30;
        
        //! Name of the TargetType of an album.
        const String ALBUM = "ALBUM";
        
        //! Name of the TargetType of a track.
        const String TRACK = "TRACK";
        
        
        
        // For AudioProperties
        
        //! ID of the Info block within the Segment.
        const ulli SegmentInfo = 0x1549a966L;
        
        //! ID of the duration element.
        const ulli Duration = 0x4489L;
        
        //! ID of TimecodeScale element.
        const ulli TimecodeScale = 0x2ad7b1L;
        
        //! ID of the Tracks container
        const ulli Tracks = 0x1654ae6bL;
        
        //! ID of a TrackEntry element.
        const ulli TrackEntry = 0xaeL;
        
        //! ID of the Audio container.
        const ulli Audio = 0xe1L;
        
        //! ID of the SamplingFrequency element.
        const ulli SamplingFrequency = 0xb5L;
        
        //! ID of the Channels element.
        const ulli Channels = 0x9fL;
        
        //! ID of the BitDepth element.
        const ulli BitDepth = 0x6264L;
      }
    }
  }
}

#endif
