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

#include "ebmlmkseekhead.h"
#include "matroskaseekhead.h"
#include "ebmluintelement.h"
#include "ebmlbinaryelement.h"

using namespace TagLib;

Matroska::SeekHead* EBML::MkSeekHead::parse()
{
  auto seekHead = new Matroska::SeekHead();
  seekHead->setOffset(offset);
  seekHead->setSize(getSize() + padding);

  for(auto element : elements) {
    if(element->getId() != ElementIDs::MkSeek)
      continue;
    auto seekElement = static_cast<MasterElement*>(element);
    Matroska::Element::ID entryId = 0;
    offset_t offset = 0;
    for(auto seekElementChild : *seekElement) {
      Id id = seekElementChild->getId();
      if(id == ElementIDs::MkSeekID && !entryId) {
        auto data = static_cast<BinaryElement*>(seekElementChild)->getValue();
        if(data.size() == 4)
          entryId = data.toUInt(true);
      }
      else if(id == ElementIDs::MkSeekPosition && !offset)
        offset = static_cast<UIntElement*>(seekElementChild)->getValue();
    }
    if(entryId && offset)
      seekHead->addEntry(entryId, offset);
    else {
      delete seekHead;
      return nullptr;
    }
  }

  return seekHead;
}
