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
#include <memory>
#include "matroskaelement.h"
#include "tlist.h"
#include "tfile.h"
#include "tbytevector.h"

using namespace TagLib;

class Matroska::Element::ElementPrivate
{
public:
  ElementPrivate() {}
  ~ElementPrivate() = default;
  ElementPrivate(const ElementPrivate &) = delete;
  ElementPrivate &operator=(const ElementPrivate &) = delete;
  offset_t size = 0;
  offset_t offset = 0;
  ID id = 0;
  ByteVector data;
  List<Element*> sizeListeners;
  List<Element*> offsetListeners;

};

Matroska::Element::Element(ID id)
: e(std::make_unique<ElementPrivate>())
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

const ByteVector& Matroska::Element::data() const
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

void Matroska::Element::addSizeListeners(const List<Element*> &elements)
{
  e->sizeListeners.append(elements);
}

void Matroska::Element::addOffsetListener(Element *element)
{
  e->offsetListeners.append(element);
}

void Matroska::Element::addOffsetListeners(const List<Element*> &elements)
{
  e->offsetListeners.append(elements);
}

void Matroska::Element::setID(ID id)
{
  e->id = id;
}

bool Matroska::Element::emitSizeChanged(offset_t delta)
{
  for(auto element : e->sizeListeners) {
    if (!element->sizeChanged(*this, delta))
      return false;
  }
  return true;
}

bool Matroska::Element::emitOffsetChanged(offset_t delta)
{
  for(auto element : e->offsetListeners) {
    if(!element->offsetChanged(*this, delta))
      return false;
  }
  return true;
}

bool Matroska::Element::sizeChanged(Element &caller, offset_t delta)
{
  if (caller.offset() < e->offset) {
    e->offset += delta;
    //return emitOffsetChanged(delta);
  }
  return true;
}

bool Matroska::Element::offsetChanged(Element &, offset_t)
{
  // Most elements don't need to handle this
  return true;
}

void Matroska::Element::write(TagLib::File &file)
{
  file.insert(e->data, e->offset, e->size);
  e->size = e->data.size();
}
