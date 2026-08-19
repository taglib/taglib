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
#include "tdebug.h"
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

bool EBML::MasterElement::read(File &file, int depth)
{
  unsigned int elementCount = 0;
  return read(file, depth, elementCount);
}

bool EBML::MasterElement::read(File &file, int depth, unsigned int &elementCount)
{
  static constexpr int MAX_EBML_DEPTH = 64;
  static constexpr int MAX_EBML_ELEMENT_COUNT = 50000;
  static constexpr int MAX_EBML_ELEMENT_COUNT_PER_LEVEL = 50000;
  if(depth > MAX_EBML_DEPTH) {
    debug("EBML: Maximum nesting depth exceeded");
    return false;
  }
  const offset_t maxOffset = file.tell() + dataSize;
  std::unique_ptr<Element> element;
  while((element = findNextElement(file, maxOffset))) {
    if(elementCount >= MAX_EBML_ELEMENT_COUNT ||
       elements.size() >= MAX_EBML_ELEMENT_COUNT_PER_LEVEL) {
      debug("EBML: Maximum element count exceeded");
      return false;
    }
    ++elementCount;
    if(auto master = dynamic_cast<MasterElement *>(element.get())) {
      if(!master->read(file, depth + 1, elementCount)) {
        debug("EBML: Invalid MasterElement");
        continue;
      }
    }
    else {
      if(!element->read(file)) {
        debug("EBML: Invalid Element");
        continue;
      }
    }
    elements.push_back(std::move(element));
  }
  if(file.tell() == maxOffset) {
    return true;
  }
  file.seek(maxOffset);
  return false;
}

bool EBML::MasterElement::read(File &file)
{
  return read(file, 0);
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
