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

#ifndef TAGLIB_EBMLMKTAGS_H
#define TAGLIB_EBMLMKTAGS_H
#ifndef DO_NOT_DOCUMENT

#include "ebmlmasterelement.h"
#include "ebmlutils.h"
#include "taglib.h"

namespace TagLib {
  namespace Matroska {
    class Tag;
  }
  //class Matroska::Tag;
  namespace EBML {
    class MkTags : public MasterElement
    {
    public:
      MkTags(int sizeLength, offset_t dataSize, offset_t offset) :
        MasterElement(Id::MkTags, sizeLength, dataSize, offset)
      {
      }
      MkTags(Id, int sizeLength, offset_t dataSize, offset_t offset) :
        MasterElement(Id::MkTags, sizeLength, dataSize, offset)
      {
      }
      MkTags() :
        MasterElement(Id::MkTags, 0, 0, 0)
      {
      }

      std::unique_ptr<Matroska::Tag> parse();
    };
  }
}

#endif
#endif
