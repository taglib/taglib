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

std::unique_ptr<Matroska::SeekHead> EBML::MkSeekHead::parse(offset_t segmentDataOffset)
{
  auto seekHead = std::make_unique<Matroska::SeekHead>(segmentDataOffset);
  seekHead->setOffset(offset);
  seekHead->setSize(getSize() + padding);

  for(const auto &element : elements) {
    if(element->getId() != Id::MkSeek)
      continue;
    auto seekElement = element_cast<Id::MkSeek>(element);
    Matroska::Element::ID entryId = 0;
    offset_t offset = 0;
    for(const auto &seekElementChild : *seekElement) {
      Id id = seekElementChild->getId();
      if(id == Id::MkSeekID && !entryId) {
        auto data = element_cast<Id::MkSeekID>(seekElementChild)->getValue();
        if(data.size() == 4)
          entryId = data.toUInt(true);
      }
      else if(id == Id::MkSeekPosition && !offset)
        offset = element_cast<Id::MkSeekPosition>(seekElementChild)->getValue();
    }
    if(entryId && offset)
      seekHead->addEntry(entryId, offset);
    else {
      seekHead.reset();
      return nullptr;
    }
  }

  return seekHead;
}
