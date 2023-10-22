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

#include "ebmlvoidelement.h"
#include "ebmlutils.h"
#include "tbytevector.h"
#include "tdebug.h"

#include <algorithm>

using namespace TagLib;

ByteVector EBML::VoidElement::render()
{
  offset_t bytesNeeded = targetSize;
  ByteVector buffer = renderId();
  bytesNeeded -= buffer.size();
  sizeLength = std::min(bytesNeeded, static_cast<offset_t>(8));
  bytesNeeded -= sizeLength;
  dataSize = bytesNeeded;
  buffer.append(renderVINT(dataSize, sizeLength));
  if (dataSize)
    buffer.append(ByteVector(dataSize, 0));

  return buffer;
}

offset_t EBML::VoidElement::getTargetSize() const
{
  return targetSize;
}

void EBML::VoidElement::setTargetSize(offset_t targetSize)
{
  this->targetSize = std::max(targetSize, MIN_VOID_ELEMENT_SIZE);
}

ByteVector EBML::VoidElement::renderSize(offset_t targetSize)
{
  VoidElement element;
  element.setTargetSize(targetSize);
  return element.render();
}
