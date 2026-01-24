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
#include "ebmlvoidelement.h"
#include "ebmlutils.h"
#include "tfile.h"

using namespace TagLib;

EBML::MasterElement::MasterElement(Id id, int sizeLength, offset_t dataSize, offset_t offset):
  Element(id, sizeLength, dataSize), offset(offset)
{
}

EBML::MasterElement::MasterElement(Id id):
  Element(id, 0, 0), offset(0)
{
}

EBML::MasterElement::~MasterElement() = default;

offset_t EBML::MasterElement::getOffset() const
{
  return offset;
}

void EBML::MasterElement::appendElement(std::unique_ptr<Element> &&element)
{
  elements.push_back(std::move(element));
}

std::list<std::unique_ptr<EBML::Element>>::iterator EBML::MasterElement::begin()
{
  return elements.begin();
}

std::list<std::unique_ptr<EBML::Element>>::iterator EBML::MasterElement::end()
{
  return elements.end();
}

std::list<std::unique_ptr<EBML::Element>>::const_iterator EBML::MasterElement::begin() const
{
  return elements.begin();
}

std::list<std::unique_ptr<EBML::Element>>::const_iterator EBML::MasterElement::end() const
{
  return elements.end();
}

std::list<std::unique_ptr<EBML::Element>>::const_iterator EBML::MasterElement::cbegin() const
{
  return elements.cbegin();
}

std::list<std::unique_ptr<EBML::Element>>::const_iterator EBML::MasterElement::cend() const
{
  return elements.cend();
}

offset_t EBML::MasterElement::getPadding() const
{
  return padding;
}

void EBML::MasterElement::setPadding(offset_t numBytes)
{
  padding = numBytes;
}

offset_t EBML::MasterElement::getMinRenderSize() const
{
  return minRenderSize;
}

void EBML::MasterElement::setMinRenderSize(offset_t minimumSize)
{
  minRenderSize = minimumSize;
}

bool EBML::MasterElement::read(File &file)
{
  const offset_t maxOffset = file.tell() + dataSize;
  std::unique_ptr<Element> element;
  while((element = findNextElement(file, maxOffset))) {
    if(!element->read(file))
      return false;
    elements.push_back(std::move(element));
  }
  return file.tell() == maxOffset;
}

ByteVector EBML::MasterElement::render()
{
  ByteVector buffer = renderId();
  ByteVector data;
  for(const auto &element : elements)
    data.append(element->render());
  dataSize = data.size();
  buffer.append(renderVINT(dataSize, 0));
  buffer.append(data);
  if(minRenderSize) {
    if(const auto bufferSize = buffer.size();
       minRenderSize >= bufferSize + MIN_VOID_ELEMENT_SIZE)
      buffer.append(VoidElement::renderSize(minRenderSize - bufferSize));
  }
  return buffer;
}
