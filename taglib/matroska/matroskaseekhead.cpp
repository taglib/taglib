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

#include "matroskaseekhead.h"
#include "ebmlmkseekhead.h"
#include "ebmlbinaryelement.h"
#include "ebmluintelement.h"
#include "ebmlmasterelement.h"
#include "tfile.h"
#include "tutils.h"
#include "tdebug.h"

using namespace TagLib;

Matroska::SeekHead::SeekHead(offset_t segmentDataOffset) :
  Element(static_cast<ID>(EBML::Element::Id::MkSeekHead)),
  segmentDataOffset(segmentDataOffset)
{
  setNeedsRender(false);
}

Matroska::SeekHead::~SeekHead() = default;

bool Matroska::SeekHead::isValid(TagLib::File &file) const
{
  bool result = true;
  for(const auto &[id, offset] : entries) {
    file.seek(segmentDataOffset + offset);
    if(EBML::Element::readId(file) != id) {
      debug(Utils::formatString("No ID %x found at seek position", id));
      result = false;
    }
  }
  return result;
}

void Matroska::SeekHead::addEntry(const Element &element)
{
  entries.append({element.id(), element.offset()});
  debug("adding to seekhead");
  setNeedsRender(true);
}

void Matroska::SeekHead::addEntry(ID id, offset_t offset)
{
  entries.append({id, offset});
  setNeedsRender(true);
}

ByteVector Matroska::SeekHead::renderInternal()
{
  const auto beforeSize = sizeRenderedOrWritten();
  EBML::MkSeekHead seekHead;
  seekHead.setMinRenderSize(beforeSize);
  for(const auto &[id, position] : entries) {
    auto seekElement = EBML::make_unique_element<EBML::Element::Id::MkSeek>();
    auto idElement = EBML::make_unique_element<EBML::Element::Id::MkSeekID>();
    idElement->setValue(ByteVector::fromUInt(id, true));
    seekElement->appendElement(std::move(idElement));

    auto positionElement = EBML::make_unique_element<EBML::Element::Id::MkSeekPosition>();
    positionElement->setValue(static_cast<unsigned long long>(position));
    seekElement->appendElement(std::move(positionElement));

    seekHead.appendElement(std::move(seekElement));
  }
  return seekHead.render();
}

void Matroska::SeekHead::write(File &file)
{
  if(!data().isEmpty())
    Element::write(file);
}

void Matroska::SeekHead::sort()
{
  entries.sort([](const auto &a, const auto &b) { return a.second < b.second; });
}

bool Matroska::SeekHead::sizeChanged(Element &caller, offset_t delta)
{
  ID callerID = caller.id();
  if(callerID == static_cast<ID>(EBML::Element::Id::MkSegment)) {
    adjustOffset(delta);
    return true;
  }
  // The equal case is needed when multiple new elements are added
  // (e.g. Attachments and Tags), they will start with the same offset
  // and are updated via size change handling.
  offset_t offset = caller.offset() - segmentDataOffset;
  auto it = entries.begin();
  while(it != entries.end()) {
    it = std::find_if(it,
      entries.end(),
      [offset, callerID](const auto &a) {
        return a.second >= offset && a.first != callerID;
      }
    );
    if(it != entries.end()) {
      it->second += delta;
      setNeedsRender(true);
      ++it;
    }
  }

  if(caller.data().isEmpty() && caller.size() + delta == 0) {
    // The caller element is removed, remove it from the seek head.
    it = std::find_if(entries.begin(), entries.end(),
      [callerID](const auto &a){ return a.first == callerID; });
    if(it != entries.end()) {
      entries.erase(it);
    }
  }
  return true;
}
