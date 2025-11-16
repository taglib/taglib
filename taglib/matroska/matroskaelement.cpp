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
    if(!emitSizeChanged(afterSize - beforeSize)) {
      return false;
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

void Matroska::Element::write(File &file)
{
  file.insert(e->data, e->offset, e->size);
  e->size = e->data.size();
}
