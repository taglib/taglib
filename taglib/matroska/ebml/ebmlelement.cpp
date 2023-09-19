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
    case EBML_ID_HEAD:
      return new Element(id, sizeLength, dataSize);
    
    case EBML_ID_MK_SEGMENT:
      return new MkSegment(sizeLength, dataSize);

    case EBML_ID_MK_TAGS:
      return new MkTags(sizeLength, dataSize);

    case EBML_ID_MK_TAG:
    case EBML_ID_MK_TAG_TARGETS:
    case EBML_ID_MK_SIMPLE_TAG:
      return new MasterElement(id, sizeLength, dataSize);

    case EBML_ID_MK_TAG_NAME:
    case EBML_ID_MK_TAG_STRING:
      return new UTF8StringElement(id, sizeLength, dataSize);

    case EBML_ID_MK_TAG_LANGUAGE:
      return new Latin1StringElement(id, sizeLength, dataSize);

    case EBML_ID_MK_TARGET_TYPE_VALUE:
      return new UIntElement(id, sizeLength, dataSize);

    default:
      return new Element(id, sizeLength, dataSize);
  }

  return nullptr;
}

void EBML::Element::skipData(File &file) 
{
  file.seek(dataSize, File::Position::Current);
}
