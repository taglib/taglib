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

#include "ebmlmkcues.h"
#include "ebmluintelement.h"
#include "matroskacues.h"

using namespace TagLib;

std::unique_ptr<Matroska::Cues> EBML::MkCues::parse(offset_t segmentDataOffset) const
{
  auto cues = std::make_unique<Matroska::Cues>(segmentDataOffset);
  cues->setOffset(offset);
  cues->setSize(getSize());
  cues->setID(static_cast<Matroska::Element::ID>(id));

  for(const auto &cuesChild : elements) {
    if(cuesChild->getId() != Id::MkCuePoint)
      continue;
    const auto cuePointElement = element_cast<Id::MkCuePoint>(cuesChild);
    auto cuePoint = std::make_unique<Matroska::CuePoint>();

    for(const auto &cuePointChild : *cuePointElement) {
      if(const Id id = cuePointChild->getId(); id == Id::MkCueTime)
        cuePoint->setTime(element_cast<Id::MkCueTime>(cuePointChild)->getValue());
      else if(id == Id::MkCueTrackPositions) {
        auto cueTrack = std::make_unique<Matroska::CueTrack>();
        const auto cueTrackElement = element_cast<Id::MkCueTrackPositions>(cuePointChild);
        for(const auto &cueTrackChild : *cueTrackElement) {
          if(const Id trackId = cueTrackChild->getId(); trackId == Id::MkCueTrack)
            cueTrack->setTrackNumber(element_cast<Id::MkCueTrack>(cueTrackChild)->getValue());
          else if(trackId == Id::MkCueClusterPosition)
            cueTrack->setClusterPosition(element_cast<Id::MkCueClusterPosition>(cueTrackChild)->getValue());
          else if(trackId == Id::MkCueRelativePosition)
            cueTrack->setRelativePosition(element_cast<Id::MkCueRelativePosition>(cueTrackChild)->getValue());
          else if(trackId == Id::MkCueDuration)
            cueTrack->setDuration(element_cast<Id::MkCueDuration>(cueTrackChild)->getValue());
          else if(trackId == Id::MkCueBlockNumber)
            cueTrack->setBlockNumber(element_cast<Id::MkCueBlockNumber>(cueTrackChild)->getValue());
          else if(trackId == Id::MkCueCodecState)
            cueTrack->setCodecState(element_cast<Id::MkCueCodecState>(cueTrackChild)->getValue());
          else if(trackId == Id::MkCueReference) {
            const auto cueReference = element_cast<Id::MkCueReference>(cueTrackChild);
            for(const auto &cueReferenceChild : *cueReference) {
              if(cueReferenceChild->getId() != Id::MkCueRefTime)
                continue;
              cueTrack->addReferenceTime(element_cast<Id::MkCueRefTime>(cueReferenceChild)->getValue());
            }
          }
        }
        cuePoint->addCueTrack(std::move(cueTrack));
      }
    }
    cues->addCuePoint(std::move(cuePoint));
  }
  return cues;
}
