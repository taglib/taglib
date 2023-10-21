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
#include "tdebug.h"

using namespace TagLib;

bool EBML::UIntElement::read(TagLib::File &file)
{
  ByteVector buffer = file.readBlock(dataSize);
  if (buffer.size() != dataSize) {
    debug("Failed to read EBML Uint element");
    return false;
  }
  value = buffer.toLongLong(true);
  return true;
}

ByteVector EBML::UIntElement::render()
{
  ByteVector buffer = renderId();
  dataSize = minSize(value);
  buffer.append(renderVINT(dataSize, 0));
  uint64_t value = this->value;
  static const auto byteOrder = Utils::systemByteOrder(); 
  if (byteOrder == Utils::LittleEndian)
    value = Utils::byteSwap((unsigned long long) value);

  buffer.append(ByteVector((char*) &value + (sizeof(value) - dataSize), dataSize));
  return buffer;
}
