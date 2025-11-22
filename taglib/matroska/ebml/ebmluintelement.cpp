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

#include "ebmluintelement.h"
#include "ebmlutils.h"
#include "tbytevector.h"
#include "tfile.h"
#include "tutils.h"
#include "tdebug.h"

using namespace TagLib;

EBML::UIntElement::UIntElement(Id id, int sizeLength, offset_t dataSize):
  Element(id, sizeLength, dataSize)
{
}

EBML::UIntElement::UIntElement(Id id, int sizeLength, offset_t dataSize, offset_t):
  Element(id, sizeLength, dataSize)
{
}

EBML::UIntElement::UIntElement(Id id):
  UIntElement(id, 0, 0)
{
}

unsigned long long EBML::UIntElement::getValue() const
{
  return value;
}

void EBML::UIntElement::setValue(unsigned long long val)
{
  value = val;
}

bool EBML::UIntElement::read(File &file)
{
  const ByteVector buffer = file.readBlock(dataSize);
  if(buffer.size() != dataSize) {
    debug("Failed to read EBML Uint element");
    return false;
  }
  value = buffer.toULongLong(true);
  return true;
}

ByteVector EBML::UIntElement::render()
{
  int dataSize = 0;
  if(value <= 0xFFull)
    dataSize = 1;
  else if(value <= 0xFFFFull)
    dataSize = 2;
  else if(value <= 0xFFFFFFull)
    dataSize = 3;
  else if(value <= 0xFFFFFFFFull)
    dataSize = 4;
  else if(value <= 0xFFFFFFFFFFull)
    dataSize = 5;
  else if(value <= 0xFFFFFFFFFFFFull)
    dataSize = 6;
  else if(value <= 0xFFFFFFFFFFFFFFull)
    dataSize = 7;
  else if(value <= 0xFFFFFFFFFFFFFFFFull)
    dataSize = 8;

  ByteVector buffer = renderId();
  buffer.append(renderVINT(dataSize, 0));
  uint64_t val = value;
  static const auto byteOrder = Utils::systemByteOrder();
  if(byteOrder == Utils::LittleEndian)
    val = Utils::byteSwap(val);

  buffer.append(ByteVector(reinterpret_cast<char *>(&val) + (sizeof(val) - dataSize), dataSize));
  return buffer;
}
