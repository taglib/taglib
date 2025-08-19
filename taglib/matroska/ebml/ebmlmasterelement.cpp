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

EBML::MasterElement::~MasterElement() = default;

void EBML::MasterElement::appendElement(std::unique_ptr<Element>&& element)
{
  elements.push_back(std::move(element));
}

bool EBML::MasterElement::read(File &file)
{
  offset_t maxOffset = file.tell() + dataSize;
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
    auto bufferSize = buffer.size();
    if(minRenderSize >= bufferSize + MIN_VOID_ELEMENT_SIZE)
      buffer.append(VoidElement::renderSize(minRenderSize - bufferSize));
  }
  return buffer;
}
