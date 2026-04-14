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

#ifndef TAGLIB_MP4QTCHAPTERLIST_H
#define TAGLIB_MP4QTCHAPTERLIST_H

#include "mp4chapterlist.h"

namespace TagLib {
  namespace MP4 {

    /*!
     * Reads, writes, and removes QuickTime-style chapter tracks from MP4
     * files.  A QT chapter track is a disabled text track (\c hdlr type
     * \c "text") referenced by a \c chap track-reference in the audio
     * track's \c tref box.  This format is understood by QuickTime,
     * iTunes, Final Cut, Logic, DaVinci Resolve, Twisted Wave, and most
     * other Apple/macOS software.
     *
     * The existing \c MP4ChapterList class handles Nero-style \c chpl
     * atoms, which are a different (and less widely supported) chapter
     * format.
     *
     * Chapter times use the same 100-nanosecond unit convention as
     * \c MP4ChapterList so that existing \c Chapter / \c ChapterList
     * types can be shared.
     */
    class TAGLIB_EXPORT MP4QTChapterList
    {
    public:
      /*!
       * Reads chapter markers from the QuickTime chapter track in the
       * MP4 file at \a path.  Returns an empty list if the file has no
       * chapter track (i.e. no \c tref/chap reference to a text track).
       */
      static ChapterList read(const char *path);

      /*!
       * Reads chapter markers from the QuickTime chapter track in the
       * already-opened \a file.  Avoids a second open when the caller
       * already has the file open.
       * Returns an empty list if the file has no chapter track.
       */
      static ChapterList read(MP4::File *file);

      /*!
       * Writes chapter markers as a QuickTime chapter track to the MP4
       * file at \a path, replacing any existing chapter track.  The
       * file's duration is read internally from the movie header.
       * Returns \c true on success.
       */
      static bool write(const char *path, const ChapterList &chapters);

      /*!
       * Writes chapter markers as a QuickTime chapter track to the
       * already-opened \a file, replacing any existing chapter track.
       * Returns \c true on success.
       */
      static bool write(MP4::File *file, const ChapterList &chapters);

      /*!
       * Removes the QuickTime chapter track and its \c tref/chap
       * reference from the MP4 file at \a path.
       * Returns \c true on success, or if no chapter track exists.
       */
      static bool remove(const char *path);

      /*!
       * Removes the QuickTime chapter track and its \c tref/chap
       * reference from the already-opened \a file.
       * Returns \c true on success, or if no chapter track exists.
       */
      static bool remove(MP4::File *file);
    };

  }  // namespace MP4
}  // namespace TagLib

#endif
