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

#ifndef TAGLIB_MATROSKACHAPTERS_H
#define TAGLIB_MATROSKACHAPTERS_H

#include <memory>
#include "taglib_export.h"
#include "tlist.h"
#include "matroskaelement.h"

namespace TagLib {
  class File;

  namespace EBML {
    class MkChapters;
  }
  namespace Matroska {
    class ChapterEdition;
    class File;

    //! Collection of chapter editions.
    class TAGLIB_EXPORT Chapters
#ifndef DO_NOT_DOCUMENT
      : private Element
#endif
    {
    public:
      //! List of chapter editions.
      using ChapterEditionList = List<ChapterEdition>;

      //! Construct chapters.
      Chapters();

      //! Destroy chapters.
      virtual ~Chapters();

      //! Add a chapter edition.
      void addChapterEdition(const ChapterEdition &edition);

      //! Remove a chapter edition.
      void removeChapterEdition(unsigned long long uid);

      //! Remove all chapter editions.
      void clear();

      //! Get list of all chapter editions.
      const ChapterEditionList &chapterEditionList() const;

    private:
      friend class EBML::MkChapters;
      friend class File;
      class ChaptersPrivate;

      // private Element implementation
      ByteVector renderInternal() override;

      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<ChaptersPrivate> d;
    };
  }
}

#endif
