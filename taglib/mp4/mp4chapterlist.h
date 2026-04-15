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

#include "tlist.h"
#include "tstring.h"
#include "taglib_export.h"
#include "mp4file.h"

namespace TagLib {
  namespace MP4 {

    /*!
     * A single Nero-style chapter marker.
     */
    struct TAGLIB_EXPORT Chapter {
      long long startTime;  //!< Start time in milliseconds
      String title;
    };

    using ChapterList = List<Chapter>;

    /*!
     * Reads, writes, and removes Nero-style chapter markers (chpl atom)
     * from MP4 files.  Operates independently of MP4::Tag -- the chpl atom
     * lives at moov/udta/chpl, a sibling of the metadata ilst path.
     */
    class TAGLIB_EXPORT MP4ChapterList
    {
    public:
      /*!
       * Reads chapter markers from the MP4 file at \a path.
       * Returns an empty list if the file has no chpl atom.
       */
      static ChapterList read(const char *path);

      /*!
       * Reads chapter markers from the already-opened \a file.
       * Avoids a second open when the caller already has the file open.
       * Returns an empty list if the file has no chpl atom.
       */
      static ChapterList read(MP4::File *file);

      /*!
       * Writes chapter markers to the MP4 file at \a path,
       * replacing any existing chpl atom.  The chapter count is
       * capped at 255 (Nero format limit).
       * Returns \c true on success.
       */
      static bool write(const char *path, const ChapterList &chapters);

      /*!
       * Writes chapter markers to the already-opened \a file,
       * replacing any existing chpl atom.
       * The chapter count is capped at 255 (Nero format limit).
       * Returns \c true on success.
       */
      static bool write(MP4::File *file, const ChapterList &chapters);

      /*!
       * Removes the chpl atom from the MP4 file at \a path.
       * Returns \c true on success, or if no chpl atom exists.
       */
      static bool remove(const char *path);

      /*!
       * Removes the chpl atom from the already-opened \a file.
       * Returns \c true on success, or if no chpl atom exists.
       */
      static bool remove(MP4::File *file);
    };

  }  // namespace MP4
}  // namespace TagLib

#endif
