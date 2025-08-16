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

#ifndef TAGLIB_EBMLMKTRACKS_H
#define TAGLIB_EBMLMKTRACKS_H
#ifndef DO_NOT_DOCUMENT

#include "ebmlmasterelement.h"
#include "ebmlutils.h"
#include "taglib.h"

namespace TagLib {
  namespace Matroska {
    class Properties;
  }
  namespace EBML {
    class MkTracks : public MasterElement
    {
    public:
      MkTracks(int sizeLength, offset_t dataSize, offset_t offset) :
        MasterElement(ElementIDs::MkTracks, sizeLength, dataSize, offset)
      {
      }
      MkTracks() :
        MasterElement(ElementIDs::MkTracks, 0, 0, 0)
      {
      }
      void parse(Matroska::Properties *properties);
    };
  }
}

#endif
#endif
