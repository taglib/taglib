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

#include "matroskacues.h"
#include "ebmlelement.h"
#include "ebmlmkcues.h"
#include "ebmlmasterelement.h"
#include "ebmluintelement.h"
#include "tlist.h"
#include "tdebug.h"
#include "tfile.h"

using namespace TagLib;

Matroska::Cues::Cues()
: Element(ElementIDs::MkCues)
{
  cuePoints.setAutoDelete(true);
}

ByteVector Matroska::Cues::renderInternal()
{
  EBML::MkCues cues;
  for (auto &cuePoint : cuePoints) {
    auto cuePointElement = new EBML::MasterElement(EBML::ElementIDs::MkCuePoint);
    auto timestamp = new EBML::UIntElement(EBML::ElementIDs::MkCueTime);
    timestamp->setValue(cuePoint->getTime());
    cuePointElement->appendElement(timestamp);

    auto trackList = cuePoint->cueTrackList();
    for (auto &cueTrack : trackList) {
      auto cueTrackElement = new EBML::MasterElement(EBML::ElementIDs::MkCueTrackPositions);
      
      // Track number
      auto trackNumber = new EBML::UIntElement(EBML::ElementIDs::MkCueTrack);
      trackNumber->setValue(cueTrack->getTrackNumber());
      cueTrackElement->appendElement(trackNumber);
      
      // Cluster position
      auto clusterPosition = new EBML::UIntElement(EBML::ElementIDs::MkCueClusterPosition);
      clusterPosition->setValue(cueTrack->getClusterPosition());
      cueTrackElement->appendElement(clusterPosition);

      // Todo - other elements


      // Reference times
      auto referenceTimes = cueTrack->referenceTimes();
      if (!referenceTimes.isEmpty()) {
        auto cueReference = new EBML::MasterElement(EBML::ElementIDs::MkCueReference);
        for (auto reference : referenceTimes) {
          auto refTime = new EBML::UIntElement(EBML::ElementIDs::MkCueRefTime);
          refTime->setValue(reference);
          cueReference->appendElement(refTime);
        }
        cueTrackElement->appendElement(cueReference);
      }
      cuePointElement->appendElement(cueTrackElement);
    }
  }
  return cues.render();
}

bool Matroska::Cues::render()
{
  if (!needsRender)
    return true;


  setData(cues.render());
  needsRender = false;
  return true;
}

bool Matroska::Cues::sizeChanged(Element &caller, offset_t delta)
{
  offset_t offset = caller.offset();
  for (auto cuePoint : cuePoints)
    needsRender |= cuePoint->adjustOffset(offset, delta);
  return true;
}

bool Matroska::Cues::isValid(TagLib::File &file, offset_t segmentDataOffset) const
{
  for (const auto cuePoint : cuePoints) {
    if (!cuePoint->isValid(file, segmentDataOffset))
      return false;
  }
  return true;
}

Matroska::CuePoint::CuePoint()
{
  cueTracks.setAutoDelete(true);
}

bool Matroska::CuePoint::isValid(TagLib::File &file, offset_t segmentDataOffset) const
{
  for (const auto track : cueTracks) {
    if (!track->isValid(file, segmentDataOffset))
      return false;
  }
  return true;
}

bool Matroska::CuePoint::adjustOffset(offset_t offset, offset_t delta)
{
  bool ret = false;
  for (auto cueTrack : cueTracks)
    ret |= cueTrack->adjustOffset(offset, delta);

  return ret;
}

bool Matroska::CueTrack::isValid(TagLib::File &file, offset_t segmentDataOffset) const
{
  if (!trackNumber) {
    debug("Cue track number not set");
    return false;
  }
  if (!clusterPosition) {
    debug("Cue track cluster position not set");
    return false;
  }
  file.seek(segmentDataOffset + clusterPosition);
  if (EBML::Element::readId(file) != EBML::ElementIDs::MkCluster) {
    debug("No cluster found at position");
    return false;
  }
  if (codecState) {
    file.seek(segmentDataOffset + codecState);
    if (EBML::Element::readId(file) != EBML::ElementIDs::MkCodecState) {
      debug("No codec state found at position");
      return false;
    }
  }
  return true;
}

bool Matroska::CueTrack::adjustOffset(offset_t offset, offset_t delta)
{
  return false;
}

