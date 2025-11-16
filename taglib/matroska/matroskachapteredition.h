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

#ifndef TAGLIB_MATROSKACHAPTEREDITION_H
#define TAGLIB_MATROSKACHAPTEREDITION_H

#include "matroskachapter.h"

namespace TagLib {
  class String;
  class ByteVector;

  namespace Matroska {
    //! Edition of chapters.
    class TAGLIB_EXPORT ChapterEdition
    {
    public:
      //! Unique identifier.
      using UID = unsigned long long;

      /*!
       * Construct an edition.
       */
      ChapterEdition(const List<Chapter> &chapterList,
        bool isDefault, bool isOrdered = false, UID uid = 0);

      /*!
       * Construct an edition as a copy of \a other.
       */
      ChapterEdition(const ChapterEdition &other);

      /*!
       * Construct an edition moving from \a other.
       */
      ChapterEdition(ChapterEdition &&other) noexcept;

      /*!
       * Destroys this edition.
       */
      ~ChapterEdition();

      /*!
       * Copies the contents of \a other into this object.
       */
      ChapterEdition &operator=(const ChapterEdition &other);

      /*!
       * Moves the contents of \a other into this object.
       */
      ChapterEdition &operator=(ChapterEdition &&other) noexcept;

      /*!
       * Exchanges the content of the object with the content of \a other.
       */
      void swap(ChapterEdition &other) noexcept;

      /*!
       * Returns the UID of the edition.
       */
      UID uid() const;

      /*!
       * Check if this edition should be used as the default one.
       */
      bool isDefault() const;

      /*!
       * Check if the chapters can be defined multiple times and the order to
       * play them is enforced.
       */
      bool isOrdered() const;

      /*!
       * Get the list of all chapters.
       */
      const List<Chapter> &chapterList() const;

    private:
      class ChapterEditionPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<ChapterEditionPrivate> d;
    };
  }
}

#endif
