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

#ifndef TAGLIB_EBMLELEMENT_H
#define TAGLIB_EBMLELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include "tfile.h"
#include "ebmlutils.h"
#include "taglib.h"

namespace TagLib {
  namespace EBML {
    class Element
    {
    public:
      Element(Id id, int sizeLength, offset_t dataSize)
      : id(id), sizeLength(sizeLength), dataSize(dataSize)
      {}
      virtual ~Element() = default;
      virtual bool isMaster() const { return false; }
      virtual bool read(File &file) {
        skipData(file);
        return true;
      }
      void skipData(File &file);
      Id getId() const { return id; }
      int getSizeLength() const { return sizeLength; }
      int64_t getDataSize() const { return dataSize; }
      static Element* factory(File &file);

    protected:
      Id id;
      int sizeLength;
      offset_t dataSize;
    };
  }
}


#endif
#endif
