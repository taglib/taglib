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

Matroska::Cues *EBML::MkCues::parse()
{
  auto cues = new Matroska::Cues();
  cues->setOffset(offset);
  cues->setSize(getSize());
  cues->setID(id);

  for(auto cuesChild : elements) {
    if(cuesChild->getId() != ElementIDs::MkCuePoint)
      continue;
    auto cuePointElement = static_cast<MasterElement *>(cuesChild);
    auto cuePoint = new Matroska::CuePoint();

    for(auto cuePointChild : *cuePointElement) {
      Id id = cuePointChild->getId();
      if(id == ElementIDs::MkCueTime)
        cuePoint->setTime(static_cast<UIntElement *>(cuePointChild)->getValue());
      else if(id == ElementIDs::MkCueTrackPositions) {
        auto cueTrack = new Matroska::CueTrack();
        auto cueTrackElement = static_cast<MasterElement *>(cuePointChild);
        for(auto cueTrackChild : *cueTrackElement) {
          Id trackId = cueTrackChild->getId();
          if(trackId == ElementIDs::MkCueTrack)
            cueTrack->setTrackNumber(static_cast<UIntElement *>(cueTrackChild)->getValue());
          else if(trackId == ElementIDs::MkCueClusterPosition)
            cueTrack->setClusterPosition(static_cast<UIntElement *>(cueTrackChild)->getValue());
          else if(trackId == ElementIDs::MkCueRelativePosition)
            cueTrack->setRelativePosition(static_cast<UIntElement *>(cueTrackChild)->getValue());
          else if(trackId == ElementIDs::MkCueDuration)
            cueTrack->setDuration(static_cast<UIntElement *>(cueTrackChild)->getValue());
          else if(trackId == ElementIDs::MkCueBlockNumber)
            cueTrack->setBlockNumber(static_cast<UIntElement *>(cueTrackChild)->getValue());
          else if(trackId == ElementIDs::MkCueCodecState)
            cueTrack->setCodecState(static_cast<UIntElement *>(cueTrackChild)->getValue());
          else if(trackId == ElementIDs::MkCueReference) {
            auto cueReference = static_cast<MasterElement *>(cueTrackChild);
            for(auto cueReferenceChild : *cueReference) {
              if(cueReferenceChild->getId() != ElementIDs::MkCueReference)
                continue;
              cueTrack->addReferenceTime(static_cast<UIntElement *>(cueReferenceChild)->getValue());
            }
          }
        }
        cuePoint->addCueTrack(cueTrack);
      }
    }
    cues->addCuePoint(cuePoint);
  }
  return cues;
}
