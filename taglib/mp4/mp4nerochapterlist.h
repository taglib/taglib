/**************************************************************************
    copyright            : (C) 2026 by Ryan Francesconi
 **************************************************************************/

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

#ifndef TAGLIB_MP4CHAPTERLIST_H
#define TAGLIB_MP4CHAPTERLIST_H

#include "mp4chapterholder.h"

namespace TagLib {
  class File;
  namespace MP4 {

    /*!
     * Reads, writes, and removes Nero-style chapter markers (chpl atom)
     * from MP4 files.  Operates independently of MP4::Tag -- the chpl atom
     * lives at moov/udta/chpl, a sibling of the metadata ilst path.
     */
    class NeroChapterList : public ChapterHolder
    {
    public:
      /*!
       * Reads chapter markers from the already-opened \a file.
       * Returns \c false if the file has no chpl atom.
       */
      bool read(TagLib::File *file);

      /*!
       * Writes chapter markers to the already-opened \a file,
       * replacing any existing chpl atom.
       * The chapter count is capped at 255 (Nero format limit).
       * Returns \c true on success.
       */
      bool write(TagLib::File *file);

      /*!
       * Removes the chpl atom from the already-opened \a file.
       * Returns \c true on success, or if no chpl atom exists.
       */
      bool remove(TagLib::File *file);
    };

  }  // namespace MP4
}  // namespace TagLib

#endif
