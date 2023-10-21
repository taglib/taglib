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

#include "ebmlelement.h"
#include "ebmlmasterelement.h"
#include "ebmlmksegment.h"
#include "ebmlmktags.h"
#include "ebmlstringelement.h"
#include "ebmluintelement.h"
#include "ebmlutils.h"
#include "tfile.h"
#include "tdebug.h"
#include "tutils.h"

#include <cstdint>

using namespace TagLib;

EBML::Element* EBML::Element::factory(File &file)
{
  // Get the element ID
  Id id = readId(file);
  if (!id) {
    debug("Failed to parse EMBL ElementID");
    return nullptr;
  }

  // Get the size length and data length
  const auto& [sizeLength, dataSize] = readVINT<offset_t>(file);
  if (!sizeLength)
    return nullptr;

  // Return the subclass
  switch(id) {
    case ElementIDs::EBMLHeader:
      return new Element(id, sizeLength, dataSize);
    
    case ElementIDs::MkSegment:
      return new MkSegment(sizeLength, dataSize);

    case ElementIDs::MkTags:
      return new MkTags(sizeLength, dataSize);

    case ElementIDs::MkTag:
    case ElementIDs::MkTagTargets:
    case ElementIDs::MkSimpleTag:
      return new MasterElement(id, sizeLength, dataSize);

    case ElementIDs::MkTagName:
    case ElementIDs::MkTagString:
      return new UTF8StringElement(id, sizeLength, dataSize);

    case ElementIDs::MkTagLanguage:
      return new Latin1StringElement(id, sizeLength, dataSize);

    case ElementIDs::MkTagTargetTypeValue:
      return new UIntElement(id, sizeLength, dataSize);

    default:
      return new Element(id, sizeLength, dataSize);
  }

  return nullptr;
}

EBML::Element::Id EBML::Element::readId(File &file)
{
  auto buffer = file.readBlock(1);
  if (buffer.size() != 1) {
    debug("Failed to read VINT size");
    return 0;
  }
  unsigned int nb_bytes = VINTSizeLength<4>(*buffer.begin());
  if (!nb_bytes)
    return 0;
  if (nb_bytes > 1)
    buffer.append(file.readBlock(nb_bytes - 1));
  if (buffer.size() != nb_bytes) {
    debug("Failed to read VINT data");
    return 0;
  }
  return buffer.toUInt(true);
}

void EBML::Element::skipData(File &file) 
{
  file.seek(dataSize, File::Position::Current);
}

offset_t EBML::Element::headSize() const
{ 
  return EBML::idSize(id) + sizeLength;
}

ByteVector EBML::Element::render()
{
  ByteVector buffer = renderId();
  buffer.append(renderVINT(0, 0));
  return buffer;
}

ByteVector EBML::Element::renderId()
{
  int numBytes = idSize(id);
  id = Utils::byteSwap(id);
  return ByteVector((char*) &id + (4 - numBytes), numBytes);
}
