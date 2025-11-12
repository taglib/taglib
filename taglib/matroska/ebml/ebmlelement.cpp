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
#include "ebmlvoidelement.h"
#include "ebmlmasterelement.h"
#include "ebmlbinaryelement.h"
#include "ebmlfloatelement.h"
#include "ebmlmkseekhead.h"
#include "ebmlmksegment.h"
#include "ebmlmktags.h"
#include "ebmlmkattachments.h"
#include "ebmlmkchapters.h"
#include "ebmlmktracks.h"
#include "ebmlstringelement.h"
#include "ebmluintelement.h"
#include "ebmlutils.h"
#include "tfile.h"
#include "tdebug.h"
#include "tutils.h"

#include <cstdint>

using namespace TagLib;

#define RETURN_ELEMENT_FOR_CASE(eid) \
  case (eid): return make_unique_element<eid>(id, sizeLength, dataSize, offset)

std::unique_ptr<EBML::Element> EBML::Element::factory(File &file)
{
  // Get the element ID
  offset_t offset = file.tell();
  unsigned int uintId = readId(file);
  if(!uintId) {
    debug("Failed to parse EMBL ElementID");
    return nullptr;
  }

  // Get the size length and data length
  const auto &[sizeLength, dataSize] = readVINT<offset_t>(file);
  if(!sizeLength)
    return nullptr;

  // Return the subclass
  // The enum switch without default will give us a warning if an ID is missing
  auto id = static_cast<Id>(uintId);
  switch(id) {
    RETURN_ELEMENT_FOR_CASE(Id::EBMLHeader);
    RETURN_ELEMENT_FOR_CASE(Id::DocType);
    RETURN_ELEMENT_FOR_CASE(Id::DocTypeVersion);
    RETURN_ELEMENT_FOR_CASE(Id::MkSegment);
    RETURN_ELEMENT_FOR_CASE(Id::MkInfo);
    RETURN_ELEMENT_FOR_CASE(Id::MkTracks);
    RETURN_ELEMENT_FOR_CASE(Id::MkTags);
    RETURN_ELEMENT_FOR_CASE(Id::MkAttachments);
    RETURN_ELEMENT_FOR_CASE(Id::MkTag);
    RETURN_ELEMENT_FOR_CASE(Id::MkTagTargets);
    RETURN_ELEMENT_FOR_CASE(Id::MkSimpleTag);
    RETURN_ELEMENT_FOR_CASE(Id::MkAttachedFile);
    RETURN_ELEMENT_FOR_CASE(Id::MkSeek);
    RETURN_ELEMENT_FOR_CASE(Id::MkTrackEntry);
    RETURN_ELEMENT_FOR_CASE(Id::MkAudio);
    RETURN_ELEMENT_FOR_CASE(Id::MkTagName);
    RETURN_ELEMENT_FOR_CASE(Id::MkTagString);
    RETURN_ELEMENT_FOR_CASE(Id::MkAttachedFileName);
    RETURN_ELEMENT_FOR_CASE(Id::MkAttachedFileDescription);
    RETURN_ELEMENT_FOR_CASE(Id::MkTagLanguage);
    RETURN_ELEMENT_FOR_CASE(Id::MkAttachedFileMediaType);
    RETURN_ELEMENT_FOR_CASE(Id::MkCodecID);
    RETURN_ELEMENT_FOR_CASE(Id::MkTagTargetTypeValue);
    RETURN_ELEMENT_FOR_CASE(Id::MkTagTrackUID);
    RETURN_ELEMENT_FOR_CASE(Id::MkTagsLanguageDefault);
    RETURN_ELEMENT_FOR_CASE(Id::MkAttachedFileUID);
    RETURN_ELEMENT_FOR_CASE(Id::MkSeekPosition);
    RETURN_ELEMENT_FOR_CASE(Id::MkTimestampScale);
    RETURN_ELEMENT_FOR_CASE(Id::MkBitDepth);
    RETURN_ELEMENT_FOR_CASE(Id::MkChannels);
    RETURN_ELEMENT_FOR_CASE(Id::MkAttachedFileData);
    RETURN_ELEMENT_FOR_CASE(Id::MkSeekID);
    RETURN_ELEMENT_FOR_CASE(Id::MkDuration);
    RETURN_ELEMENT_FOR_CASE(Id::MkTitle);
    RETURN_ELEMENT_FOR_CASE(Id::MkSamplingFrequency);
    RETURN_ELEMENT_FOR_CASE(Id::MkSeekHead);
    RETURN_ELEMENT_FOR_CASE(Id::VoidElement);
    RETURN_ELEMENT_FOR_CASE(Id::MkCluster);
    RETURN_ELEMENT_FOR_CASE(Id::MkCodecState);
    RETURN_ELEMENT_FOR_CASE(Id::MkTagBinary);
    RETURN_ELEMENT_FOR_CASE(Id::MkCues);
    RETURN_ELEMENT_FOR_CASE(Id::MkCuePoint);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueTime);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueTrackPositions);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueTrack);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueClusterPosition);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueRelativePosition);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueDuration);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueBlockNumber);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueCodecState);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueReference);
    RETURN_ELEMENT_FOR_CASE(Id::MkCueRefTime);
    RETURN_ELEMENT_FOR_CASE(Id::MkChapters);
    RETURN_ELEMENT_FOR_CASE(Id::MkEditionEntry);
    RETURN_ELEMENT_FOR_CASE(Id::MkEditionUID);
    RETURN_ELEMENT_FOR_CASE(Id::MkEditionFlagDefault);
    RETURN_ELEMENT_FOR_CASE(Id::MkEditionFlagOrdered);
    RETURN_ELEMENT_FOR_CASE(Id::MkChapterAtom);
    RETURN_ELEMENT_FOR_CASE(Id::MkChapterUID);
    RETURN_ELEMENT_FOR_CASE(Id::MkChapterTimeStart);
    RETURN_ELEMENT_FOR_CASE(Id::MkChapterTimeEnd);
    RETURN_ELEMENT_FOR_CASE(Id::MkChapterFlagHidden);
    RETURN_ELEMENT_FOR_CASE(Id::MkChapterDisplay);
    RETURN_ELEMENT_FOR_CASE(Id::MkChapString);
    RETURN_ELEMENT_FOR_CASE(Id::MkChapLanguage);
  }
  return std::make_unique<Element>(id, sizeLength, dataSize);
}

unsigned int EBML::Element::readId(File &file)
{
  auto buffer = file.readBlock(1);
  if(buffer.size() != 1) {
    debug("Failed to read VINT size");
    return 0;
  }
  unsigned int nb_bytes = VINTSizeLength<4>(*buffer.begin());
  if(!nb_bytes)
    return 0;
  if(nb_bytes > 1)
    buffer.append(file.readBlock(nb_bytes - 1));
  if(buffer.size() != nb_bytes) {
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
  return idSize(id) + sizeLength;
}

ByteVector EBML::Element::render()
{
  ByteVector buffer = renderId();
  buffer.append(renderVINT(0, 0));
  return buffer;
}

ByteVector EBML::Element::renderId() const
{
  int numBytes = idSize(id);
  static const auto byteOrder = Utils::systemByteOrder();
  auto uintId = static_cast<uint32_t>(id);
  uint32_t data = byteOrder == Utils::LittleEndian ? Utils::byteSwap(uintId) : uintId;
  return ByteVector(reinterpret_cast<char *>(&data) + (4 - numBytes), numBytes);
}
