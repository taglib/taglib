/**************************************************************************
    copyright            : (C) 2026 by Antoine Colombier
    email                : antoine@mixxx.org
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

#ifndef TAGLIB_MP4STEM_H
#define TAGLIB_MP4STEM_H

#include "tlist.h"
#include "tbytevector.h"
#include "taglib_export.h"
#include "mp4atom.h"

namespace TagLib {
  namespace MP4 {
    //! STEM
    class StemPrivate
    {
    public:
      ByteVector data;
    };

    class TAGLIB_EXPORT Stem
    {
    public:
      Stem();
      Stem(const ByteVector &data);
      ~Stem();

      Stem(const Stem &item);

      /*!
       * Copies the contents of \a item into this Stem.
       */
      Stem &operator=(const Stem &item);

      /*!
       * Exchanges the content of the Stem with the content of \a item.
       */
      void swap(Stem &item) noexcept;

      //! The image data
      ByteVector data() const;

      /*!
       * Returns \c true if the Stem and \a other are of the same format and
       * contain the same data.
       */
      bool operator==(const Stem &other) const;

      /*!
       * Returns \c true if the Stem and \a other  differ in format or data.
       */
      bool operator!=(const Stem &other) const;

    private:
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::shared_ptr<StemPrivate> d;
    };
  }  // namespace MP4
}  // namespace TagLib
#endif
