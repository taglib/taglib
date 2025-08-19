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

Matroska::Cues::Cues() :
  Element(static_cast<ID>(EBML::Element::Id::MkCues))
{
}

ByteVector Matroska::Cues::renderInternal()
{
  EBML::MkCues cues;
  for(const auto &cuePoint : cuePoints) {
    auto cuePointElement = EBML::make_unique_element<EBML::Element::Id::MkCuePoint>();
    auto timestamp = EBML::make_unique_element<EBML::Element::Id::MkCueTime>();
    timestamp->setValue(cuePoint->getTime());
    cuePointElement->appendElement(std::move(timestamp));

    const auto &trackList = cuePoint->cueTrackList();
    for(const auto &cueTrack : trackList) {
      auto cueTrackElement = EBML::make_unique_element<EBML::Element::Id::MkCueTrackPositions>();

      // Track number
      auto trackNumber = EBML::make_unique_element<EBML::Element::Id::MkCueTrack>();
      trackNumber->setValue(cueTrack->getTrackNumber());
      cueTrackElement->appendElement(std::move(trackNumber));

      // Cluster position
      auto clusterPosition = EBML::make_unique_element<EBML::Element::Id::MkCueClusterPosition>();
      clusterPosition->setValue(cueTrack->getClusterPosition());
      cueTrackElement->appendElement(std::move(clusterPosition));

      // Todo - other elements

      // Reference times
      auto referenceTimes = cueTrack->referenceTimes();
      if(!referenceTimes.isEmpty()) {
        auto cueReference = EBML::make_unique_element<EBML::Element::Id::MkCueReference>();
        for(auto reference : referenceTimes) {
          auto refTime = EBML::make_unique_element<EBML::Element::Id::MkCueRefTime>();
          refTime->setValue(reference);
          cueReference->appendElement(std::move(refTime));
        }
        cueTrackElement->appendElement(std::move(cueReference));
      }
      cuePointElement->appendElement(std::move(cueTrackElement));
    }
  }
  return cues.render();
}

bool Matroska::Cues::render()
{
  if(!needsRender)
    return true;

  setData(renderInternal());
  needsRender = false;
  return true;
}

bool Matroska::Cues::sizeChanged(Element &caller, offset_t delta)
{
  offset_t offset = caller.offset();
  for(auto &cuePoint : cuePoints)
    needsRender |= cuePoint->adjustOffset(offset, delta);
  return true;
}

bool Matroska::Cues::isValid(TagLib::File &file, offset_t segmentDataOffset) const
{
  for(const auto &cuePoint : cuePoints) {
    if(!cuePoint->isValid(file, segmentDataOffset))
      return false;
  }
  return true;
}

void Matroska::Cues::addCuePoint(std::unique_ptr<CuePoint> &&cuePoint)
{
  cuePoints.push_back(std::move(cuePoint));
}

Matroska::CuePoint::CuePoint()
{
}

bool Matroska::CuePoint::isValid(TagLib::File &file, offset_t segmentDataOffset) const
{
  for(const auto &track : cueTracks) {
    if(!track->isValid(file, segmentDataOffset))
      return false;
  }
  return true;
}

void Matroska::CuePoint::addCueTrack(std::unique_ptr<CueTrack> &&cueTrack)
{
  cueTracks.push_back(std::move(cueTrack));
}

bool Matroska::CuePoint::adjustOffset(offset_t offset, offset_t delta)
{
  bool ret = false;
  for(auto &cueTrack : cueTracks)
    ret |= cueTrack->adjustOffset(offset, delta);

  return ret;
}

bool Matroska::CueTrack::isValid(TagLib::File &file, offset_t segmentDataOffset) const
{
  if(!trackNumber) {
    debug("Cue track number not set");
    return false;
  }
  if(!clusterPosition) {
    debug("Cue track cluster position not set");
    return false;
  }
  file.seek(segmentDataOffset + clusterPosition);
  if(EBML::Element::readId(file) != static_cast<unsigned int>(EBML::Element::Id::MkCluster)) {
    debug("No cluster found at position");
    return false;
  }
  if(codecState) {
    file.seek(segmentDataOffset + codecState);
    if(EBML::Element::readId(file) != static_cast<unsigned int>(EBML::Element::Id::MkCodecState)) {
      debug("No codec state found at position");
      return false;
    }
  }
  return true;
}

bool Matroska::CueTrack::adjustOffset(offset_t, offset_t)
{
  // TODO implement
  return false;
}
