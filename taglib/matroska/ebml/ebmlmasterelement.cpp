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
#include "ebmlutils.h"
#include "matroskafile.h"

#include "tfile.h"
#include "tdebug.h"
#include "tutils.h"

using namespace TagLib;

EBML::MasterElement::~MasterElement()
{
  for (auto element : elements)
    delete element;
}

bool EBML::MasterElement::read(File &file)
{
  offset_t maxOffset = file.tell() + dataSize;
  Element *element = nullptr;
  while ((element = findNextElement(file, maxOffset))) {
    if (!element->read(file))
      return false;
    elements.append(element);
  }
  return file.tell() == maxOffset;
}

ByteVector EBML::MasterElement::render()
{
  ByteVector buffer = renderId();
  ByteVector data;
  for (auto element : elements)
    data.append(element->render());
  dataSize = data.size();
  buffer.append(renderVINT(dataSize, 0));
  buffer.append(data);
  return buffer;
}
