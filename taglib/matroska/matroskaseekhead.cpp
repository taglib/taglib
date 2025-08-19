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
#include "tdebug.h"

using namespace TagLib;

Matroska::SeekHead::SeekHead() :
  Element(static_cast<ID>(EBML::Element::Id::MkSeekHead))
{
}

void Matroska::SeekHead::addEntry(const Element &element)
{
  entries.append({element.id(), element.offset()});
  debug("adding to seekhead");
  needsRender = true;
}

void Matroska::SeekHead::addEntry(ID id, offset_t offset)
{
  entries.append({id, offset});
  needsRender = true;
}

ByteVector Matroska::SeekHead::renderInternal()
{
  auto beforeSize = size();
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

bool Matroska::SeekHead::render()
{
  if(!needsRender)
    return true;

  auto beforeSize = size();
  auto data = renderInternal();
  needsRender = false;
  auto afterSize = data.size();
  if(afterSize != beforeSize) {
    return false;
    // TODO handle expansion of seek head
    if(!emitSizeChanged(afterSize - beforeSize))
      return false;
  }

  setData(data);
  return true;
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
  else {
    offset_t offset = caller.offset();
    auto it = entries.begin();
    while(it != entries.end()) {
      it = std::find_if(it,
        entries.end(),
        [offset](const auto a){ return a.second > offset; }
      );
      if(it != entries.end()) {
        it->second += delta;
        needsRender = true;
        ++it;
      }
    }
    return true;
  }
}
