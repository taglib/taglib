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

#ifndef TAGLIB_EBMLDEFERREDBINARYELEMENT_H
#define TAGLIB_EBMLDEFERREDBINARYELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include "ebmlbinaryelement.h"

namespace TagLib {
  class File;

  namespace EBML {
    /*!
     * A binary element whose data is not pulled into memory while the file is
     * read.  read() only registers the offset of the data in the file and
     * skips over it, so that the payload can be loaded later, when it is
     * really requested.  This keeps reading the metadata of a file cheap even
     * if it contains large attachments.
     *
     * Elements which are created to be rendered (i.e. not read from a file)
     * behave exactly like a BinaryElement: setValue() makes the data
     * available and isDeferred() stays false.
     */
    class DeferredBinaryElement : public BinaryElement
    {
    public:
      DeferredBinaryElement(Id id, int sizeLength, offset_t dataSize);
      DeferredBinaryElement(Id id, int sizeLength, offset_t dataSize, offset_t);
      explicit DeferredBinaryElement(Id id);

      /*!
       * Registers the offset of the data in \a file and skips the data.
       */
      bool read(File &file) override;

      /*!
       * Returns \c true if the data has not been read into memory, i.e. if it
       * has to be loaded from getDataOffset() to be available.
       */
      bool isDeferred() const;

      /*!
       * Returns the offset of the data inside the file, only valid if
       * isDeferred() is \c true.
       */
      offset_t getDataOffset() const;

    private:
      offset_t dataOffset = 0;
      bool deferred = false;
    };
  }
}

#endif
#endif
