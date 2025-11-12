/***************************************************************************
    copyright            : (C) 2025 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

#ifndef TAGLIB_MATROSKACHAPTER_H
#define TAGLIB_MATROSKACHAPTER_H

#include <memory>
#include "taglib_export.h"
#include "tlist.h"

namespace TagLib {
  namespace EBML {
    class MkChapters;
  }

  class String;
  class ByteVector;
  namespace Matroska {
    //! Matroska chapter.
    class TAGLIB_EXPORT Chapter
    {
    public:
      using UID = unsigned long long;
      using Time = unsigned long long;

      /*!
       * Contains all possible strings to use for the chapter display.
       */
      class TAGLIB_EXPORT Display
      {
      public:
        /*!
         * Construct a chapter display.
         */
        Display(const String &string, const String &language);

        /*!
         * Construct a chapter display as a copy of \a other.
         */
        Display(const Display &other);

        /*!
         * Construct a chapter display moving from \a other.
         */
        Display(Display &&other) noexcept;

        /*!
         * Destroys this chapter display.
         */
        ~Display();

        /*!
         * Copies the contents of \a other into this object.
         */
        Display &operator=(const Display &other);

        /*!
         * Moves the contents of \a other into this object.
         */
        Display &operator=(Display &&other) noexcept;

        /*!
         * Exchanges the content of the object with the content of \a other.
         */
        void swap(Display &other) noexcept;

        /*!
         * Returns string representing the chapter.
         */
        const String &string() const;

        /*!
         * Returns language corresponding to the string.
         */
        const String &language() const;

      private:
        class DisplayPrivate;
        TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
        std::unique_ptr<DisplayPrivate> d;
      };

      /*!
       * Construct a chapter.
       */
      Chapter(Time timeStart, Time timeEnd, const List<Display> &displayList,
        UID uid, bool hidden = false);

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
       * Returns the UID of the chapter.
       */
      UID uid() const;

      /*!
       * Returns the timestamp of the start of the chapter in nanoseconds.
       */
      Time timeStart() const;

      /*!
       * Returns the timestamp of the start of the chapter in nanoseconds.
       */
      Time timeEnd() const;

      /*!
       * Check if chapter is hidden.
       */
      bool isHidden() const;

      /*!
       * Returns strings with language.
       */
      const List<Display> &displayList() const;

    private:
      class ChapterPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<ChapterPrivate> d;
    };
  }
}

#endif
