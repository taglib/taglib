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

void EBML::MkTracks::parse(Matroska::Properties *properties)
{
  if(!properties)
    return;

  for(auto element : elements) {
    if(element->getId() != ElementIDs::MkTrackEntry)
      continue;

    String codecId;
    double samplingFrequency = 0.0;
    unsigned long long bitDepth = 0;
    unsigned long long channels = 0;
    auto trackEntry = static_cast<MasterElement *>(element);
    for(auto trackEntryChild : *trackEntry) {
      Id trackEntryChildId = trackEntryChild->getId();
      if(trackEntryChildId == ElementIDs::MkCodecID)
        codecId = static_cast<Latin1StringElement *>(trackEntryChild)->getValue();
      else if(trackEntryChildId == ElementIDs::MkAudio) {
        auto audio = static_cast<MasterElement *>(trackEntryChild);
        for(auto audioChild : *audio) {
          Id audioChildId = audioChild->getId();
          if(audioChildId == ElementIDs::MkSamplingFrequency)
            samplingFrequency = static_cast<FloatElement *>(audioChild)->getValueAsDouble();
          else if(audioChildId == ElementIDs::MkBitDepth)
            bitDepth = static_cast<UIntElement *>(audioChild)->getValue();
          else if(audioChildId == ElementIDs::MkChannels)
            channels = static_cast<UIntElement *>(audioChild)->getValue();
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
