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

#include "ebmlmasterelement.h"
#include "taglib.h"

#ifndef TAGLIB_EBMLMKSEEKHEAD_H
#define TAGLIB_EBMLMKSEEKHEAD_H
#ifndef DO_NOT_DOCUMENT

namespace TagLib {
  namespace Matroska {
    class SeekHead;
  }
  namespace EBML {
    class MkSeekHead : public MasterElement
    {
    public:
      MkSeekHead(int sizeLength, offset_t dataSize, offset_t offset)
      : MasterElement(ElementIDs::MkSeekHead, sizeLength, dataSize, offset)
      {}
      MkSeekHead()
      : MasterElement(ElementIDs::MkSeekHead, 0, 0, 0)
      {}

      Matroska::SeekHead* parse();

    };
  }
}
#endif
#endif
