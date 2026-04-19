/**************************************************************************
    copyright            : (C) 2006 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

#ifndef TAGLIB_MP4CHAPTERHOLDER_H
#define TAGLIB_MP4CHAPTERHOLDER_H

#include "mp4chapter.h"

namespace TagLib {
  class File;
  namespace MP4 {
    /*!
     * Base class to hold chapters and store modified state.
     */
    class ChapterHolder {
    public:
      /*!
       * Get list of chapters.
       */
      ChapterList chapters() const;

      /*!
       * Set list of chapters.
       */
      void setChapters(const ChapterList &chapters);

      /*!
       * Returns \c true if the list of chapters has been modified.
       */
      bool isModified() const;

    protected:
      ChapterList chapterList;
      bool modified = false;
    };

    /*!
     * Lazily fetch list of chapters.
     * @tparam T class derived from ChapterHolder and implementing read(File *)
     * @param holder unique pointer to holder, initially null
     * @param file file with chapters
     * @return list of chapters, empty if no chapters found.
     */
    template <typename T>
    ChapterList getChaptersLazy(std::unique_ptr<T> &holder, TagLib::File *file)
    {
      if (!holder) {
        holder = std::make_unique<T>();
        holder->read(file);
      }
      return holder->chapters();
    }

    /*!
     * Lazily set a list of chapters.
     * @tparam T class derived from ChapterHolder
     * @param holder unique pointer to holder, initially null
     * @param chapters list of chapters to set
     */
    template <typename T>
    void setChaptersLazy(std::unique_ptr<T> &holder, const ChapterList& chapters)
    {
      if (!holder) {
        holder = std::make_unique<T>();
      }
      holder->setChapters(chapters);
    }

    /*!
     * Save a list of chapters if it has been modified.
     * @tparam T class derived from ChapterHolder and implementing write(File *)
     * @param holder unique pointer to holder, initially null
     * @param file file with chapters
     * @return true if write successful or not modified.
     */
    template <typename T>
    bool saveChaptersIfModified(std::unique_ptr<T> &holder, TagLib::File *file)
    {
      if(holder && holder->isModified()) {
        return holder->write(file);
      }
      return true;
    }

  }  // namespace MP4
}  // namespace TagLib

#endif
