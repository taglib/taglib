/***************************************************************************
    copyright            : (C) 2025 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
 ***************************************************************************/

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

#include "ebmlmktracks.h"
#include "ebmlstringelement.h"
#include "ebmluintelement.h"
#include "ebmlfloatelement.h"
#include "matroskaproperties.h"

using namespace TagLib;

void EBML::MkTracks::parse(Matroska::Properties *properties) const
{
  if(!properties)
    return;

  for(const auto &element : elements) {
    if(element->getId() != Id::MkTrackEntry)
      continue;

    String codecId;
    double samplingFrequency = 0.0;
    unsigned long long bitDepth = 0;
    unsigned long long channels = 0;
    const auto trackEntry = element_cast<Id::MkTrackEntry>(element);
    for(const auto &trackEntryChild : *trackEntry) {
      if(const Id trackEntryChildId = trackEntryChild->getId(); trackEntryChildId == Id::MkCodecID)
        codecId = element_cast<Id::MkCodecID>(trackEntryChild)->getValue();
      else if(trackEntryChildId == Id::MkAudio) {
        const auto audio = element_cast<Id::MkAudio>(trackEntryChild);
        for(const auto &audioChild : *audio) {
          if(const Id audioChildId = audioChild->getId(); audioChildId == Id::MkSamplingFrequency)
            samplingFrequency = element_cast<Id::MkSamplingFrequency>(audioChild)->getValueAsDouble();
          else if(audioChildId == Id::MkBitDepth)
            bitDepth = element_cast<Id::MkBitDepth>(audioChild)->getValue();
          else if(audioChildId == Id::MkChannels)
            channels = element_cast<Id::MkChannels>(audioChild)->getValue();
        }
      }
    }
    if(bitDepth || channels) {
      properties->setSampleRate(static_cast<int>(samplingFrequency));
      properties->setBitsPerSample(static_cast<int>(bitDepth));
      properties->setChannels(static_cast<int>(channels));
      properties->setCodecName(codecId);
      return;
    }
  }
}
