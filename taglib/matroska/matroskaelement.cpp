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

#include "matroskaelement.h"
#include <memory>
#include "tlist.h"
#include "tfile.h"
#include "tbytevector.h"
#include "ebmlvoidelement.h"

using namespace TagLib;

class Matroska::Element::ElementPrivate
{
public:
  ElementPrivate() = default;
  ~ElementPrivate() = default;
  ElementPrivate(const ElementPrivate &) = delete;
  ElementPrivate &operator=(const ElementPrivate &) = delete;
  offset_t size = 0;
  offset_t offset = 0;
  ID id = 0;
  ByteVector data;
  List<Element *> sizeListeners;
  // The default write() implementation will delete an unrendered element,
  // therefore rendering is required by default and needs to be explicitly set
  // using setNeedsRender(false) together with overriding the write() method.
  bool needsRender = true;
  WriteStyle writeStyle = WriteStyle::Compact;
  bool isLastElement = true;
  bool isTrailingInSegment = false;
  offset_t appendOffset = 0;
  // Populated during render() for AvoidInsert+grow+non-last: the offset and
  // original size of the slot that should be overwritten with a Void element.
  offset_t voidAtOffset = 0;
  offset_t voidAtSize = 0;
};

Matroska::Element::Element(ID id) :
  e(std::make_unique<ElementPrivate>())
{
  e->id = id;
}

Matroska::Element::~Element() = default;

offset_t Matroska::Element::size() const
{
  return e->size;
}

offset_t Matroska::Element::offset() const
{
  return e->offset;
}

void Matroska::Element::setData(const ByteVector &data)
{
  e->data = data;
}

const ByteVector &Matroska::Element::data() const
{
  return e->data;
}

void Matroska::Element::setOffset(offset_t offset)
{
  e->offset = offset;
}

void Matroska::Element::adjustOffset(offset_t delta)
{
  e->offset += delta;
}

void Matroska::Element::setSize(offset_t size)
{
  e->size = size;
}

Matroska::Element::ID Matroska::Element::id() const
{
  return e->id;
}

void Matroska::Element::addSizeListener(Element *element)
{
  e->sizeListeners.append(element);
}

void Matroska::Element::addSizeListeners(const List<Element *> &elements)
{
  e->sizeListeners.append(elements);
}

void Matroska::Element::setID(ID id)
{
  e->id = id;
}

bool Matroska::Element::render()
{
  if(!needsRender())
    return true;

  const auto beforeSize = sizeRenderedOrWritten();
  const auto data = renderInternal();
  setNeedsRender(false);
  if(const auto afterSize = data.size(); afterSize != beforeSize) {
    if(e->writeStyle == WriteStyle::AvoidInsert && !e->isLastElement
       && afterSize > beforeSize && beforeSize > 0) {
      // Record old slot for void-overwrite, move element to end of segment.
      e->voidAtOffset = e->offset;
      e->voidAtSize = beforeSize;
      e->offset = e->appendOffset;
      // Notify listeners that a new element of afterSize bytes appeared at
      // appendOffset (which is past all other elements, so no offset shifts).
      if(!emitSizeChanged(static_cast<offset_t>(afterSize))) {
        return false;
      }
      // Update appendOffset for any subsequent AvoidInsert-grow in this round.
      e->appendOffset += static_cast<offset_t>(afterSize);
    }
    else {
      if(!emitSizeChanged(afterSize - beforeSize)) {
        return false;
      }
    }
  }

  setData(data);
  return true;
}

void Matroska::Element::setNeedsRender(bool needsRender)
{
  e->needsRender = needsRender;
}

bool Matroska::Element::needsRender() const
{
  return e->needsRender;
}

bool Matroska::Element::emitSizeChanged(offset_t delta)
{
  for(const auto element : e->sizeListeners) {
    if(!element->sizeChanged(*this, delta))
      return false;
  }
  return true;
}

bool Matroska::Element::sizeChanged(Element &caller, offset_t delta)
{
  // The equal case is needed when multiple new elements are added
  // (e.g. Attachments and Tags), they will start with the same offset
  // are updated via size change handling.
  if(caller.offset() <= e->offset && caller.id() != e->id) {
    e->offset += delta;
  }
  return true;
}

offset_t Matroska::Element::sizeRenderedOrWritten() const
{
  const offset_t dataSize = e->data.size();
  return dataSize != 0 ? dataSize : e->size;
}

void Matroska::Element::setWriteStyle(WriteStyle style)
{
  e->writeStyle = style;
}

Matroska::WriteStyle Matroska::Element::writeStyle() const
{
  return e->writeStyle;
}

void Matroska::Element::setIsLastElement(bool isLast)
{
  e->isLastElement = isLast;
}

void Matroska::Element::setAppendOffset(offset_t appendOffset)
{
  e->appendOffset = appendOffset;
}

void Matroska::Element::setIsTrailingInSegment(bool isTrailing)
{
  e->isTrailingInSegment = isTrailing;
}

bool Matroska::Element::isTrailingInSegment() const
{
  return e->isTrailingInSegment;
}

bool Matroska::Element::wasMoved() const
{
  // voidAtSize is set when the element was moved during render().
  // After write() it is cleared, but the caller checks before write().
  return e->voidAtOffset != 0 || e->voidAtSize != 0;
}

void Matroska::Element::write(File &file)
{
  if(e->voidAtSize > 0) {
    // AvoidInsert: overwrite the old slot with a Void element.
    const auto voidData = EBML::VoidElement::renderSize(e->voidAtSize);
    file.insert(voidData, e->voidAtOffset, e->voidAtSize);
    e->voidAtOffset = 0;
    // The element was moved to a new position (end of segment),
    // so there are no existing bytes to replace at the new offset.
    e->size = 0;
    e->voidAtSize = 0;
  }
  file.insert(e->data, e->offset, e->size);
  e->size = e->data.size();
}
