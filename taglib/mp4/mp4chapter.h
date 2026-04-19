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

#ifndef TAGLIB_MP4CHAPTER_H
#define TAGLIB_MP4CHAPTER_H

#include <memory>
#include "taglib_export.h"
#include "tlist.h"

namespace TagLib {
  class String;
  namespace MP4 {

    /*!
     * A single Nero-style chapter marker.
     */
    class TAGLIB_EXPORT Chapter {
    public:
      /*!
       * Construct a chapter.
       */
      Chapter(const String &title, long long startTime);

      /*!
       * Construct a chapter as a copy of \a other.
       */
      Chapter(const Chapter &other);

      /*!
       * Construct a chapter moving from \a other.
       */
      Chapter(Chapter &&other) noexcept;

      /*!
       * Destroys this chapter.
       */
      ~Chapter();

      /*!
       * Copies the contents of \a other into this object.
       */
      Chapter &operator=(const Chapter &other);

      /*!
       * Moves the contents of \a other into this object.
       */
      Chapter &operator=(Chapter &&other) noexcept;

      /*!
       * Exchanges the content of the object with the content of \a other.
       */
      void swap(Chapter &other) noexcept;

      /*!
       * Returns the title representing the chapter.
       */
      const String &title() const;

      /*!
       * Returns the start time in milliseconds.
       */
      long long startTime() const;

    private:
      class ChapterPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<ChapterPrivate> d;
    };

    //! List of chapters.
    using ChapterList = List<Chapter>;

  }  // namespace MP4
}  // namespace TagLib

#endif
