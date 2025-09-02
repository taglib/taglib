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

#ifndef TAGLIB_MATROSKASEEKHEAD_H
#define TAGLIB_MATROSKASEEKHEAD_H
#ifndef DO_NOT_DOCUMENT

#include "matroskaelement.h"
#include "tbytevector.h"
#include "tlist.h"

namespace TagLib {
  class File;
  namespace Matroska {
    class SeekHead : public Element
    {
    public:
      explicit SeekHead(offset_t segmentDataOffset);
      ~SeekHead() override = default;
      bool isValid(TagLib::File &file) const;
      void addEntry(const Element &element);
      void addEntry(ID id, offset_t offset);
      void write(TagLib::File &file) override;
      void sort();
      bool sizeChanged(Element &caller, offset_t delta) override;

    private:
      ByteVector renderInternal() override;
      List<std::pair<unsigned int, offset_t>> entries;
      const offset_t segmentDataOffset;
    };
  }
}

#endif
#endif
