/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#include "mpegheader.h"

#include <array>

#include "tbytevector.h"
#include "tdebug.h"
#include "tfile.h"
#include "mpegutils.h"

using namespace TagLib;

class MPEG::Header::HeaderPrivate
{
public:
  bool isValid { false };
  Version version { Version1 };
  int layer { 0 };
  bool protectionEnabled { false };
  int bitrate { 0 };
  int sampleRate { 0 };
  bool isPadded { false };
  ChannelMode channelMode { Stereo };
  bool isCopyrighted { false };
  bool isOriginal { false };
  int frameLength { 0 };
  int samplesPerFrame { 0 };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPEG::Header::Header(File *file, offset_t offset, bool checkLength) :
  d(std::make_shared<HeaderPrivate>())
{
  parse(file, offset, checkLength);
}

MPEG::Header::Header(const Header &) = default;
MPEG::Header::~Header() = default;

bool MPEG::Header::isValid() const
{
  return d->isValid;
}

MPEG::Header::Version MPEG::Header::version() const
{
  return d->version;
}

int MPEG::Header::layer() const
{
  return d->layer;
}

bool MPEG::Header::protectionEnabled() const
{
  return d->protectionEnabled;
}

int MPEG::Header::bitrate() const
{
  return d->bitrate;
}

int MPEG::Header::sampleRate() const
{
  return d->sampleRate;
}

bool MPEG::Header::isPadded() const
{
  return d->isPadded;
}

MPEG::Header::ChannelMode MPEG::Header::channelMode() const
{
  return d->channelMode;
}

bool MPEG::Header::isCopyrighted() const
{
  return d->isCopyrighted;
}

bool MPEG::Header::isOriginal() const
{
  return d->isOriginal;
}

int MPEG::Header::frameLength() const
{
  return d->frameLength;
}

int MPEG::Header::samplesPerFrame() const
{
  return d->samplesPerFrame;
}

MPEG::Header &MPEG::Header::operator=(const Header &h)
{
  if(&h == this)
    return *this;

  d = h.d;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MPEG::Header::parse(File *file, offset_t offset, bool checkLength)
{
  file->seek(offset);
  const ByteVector data = file->readBlock(4);

  if(data.size() < 4) {
    debug("MPEG::Header::parse() -- data is too short for an MPEG frame header.");
    return;
  }

  // Check for the MPEG synch bytes.

  if(!isFrameSync(data)) {
    debug("MPEG::Header::parse() -- MPEG header did not match MPEG synch.");
    return;
  }

  // Set the MPEG version

  const int versionBits = (static_cast<unsigned char>(data[1]) >> 3) & 0x03;

  if(versionBits == 0)
    d->version = Version2_5;
  else if(versionBits == 2)
    d->version = Version2;
  else if(versionBits == 3)
    d->version = Version1;
  else
    return;

  // Set the MPEG layer

  const int layerBits = (static_cast<unsigned char>(data[1]) >> 1) & 0x03;

  if(layerBits == 1)
    d->layer = 3;
  else if(layerBits == 2)
    d->layer = 2;
  else if(layerBits == 3)
    d->layer = 1;
  else
    return;

  d->protectionEnabled = (static_cast<unsigned char>(data[1] & 0x01) == 0);

  // Set the bitrate

  static constexpr std::array bitrates {
    std::array {
      // Version 1
      std::array { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 }, // layer 1
      std::array { 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0 },    // layer 2
      std::array { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0 }      // layer 3
    },
    std::array {
      // Version 2 or 2.5
      std::array { 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }, // layer 1
      std::array { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 },      // layer 2
      std::array { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 }       // layer 3
    },
  };

  const int versionIndex = (d->version == Version1) ? 0 : 1;
  const int layerIndex   = (d->layer > 0) ? d->layer - 1 : 0;

  // The bitrate index is encoded as the first 4 bits of the 3rd byte,
  // i.e. 1111xxxx

  const int bitrateIndex = (static_cast<unsigned char>(data[2]) >> 4) & 0x0F;

  d->bitrate = bitrates[versionIndex][layerIndex][bitrateIndex];

  if(d->bitrate == 0)
    return;

  // Set the sample rate

  static constexpr std::array sampleRates {
    std::array { 44100, 48000, 32000, 0 }, // Version 1
    std::array { 22050, 24000, 16000, 0 }, // Version 2
    std::array { 11025, 12000, 8000, 0 }   // Version 2.5
  };

  // The sample rate index is encoded as two bits in the 3rd byte, i.e. xxxx11xx

  const int samplerateIndex = (static_cast<unsigned char>(data[2]) >> 2) & 0x03;

  d->sampleRate = sampleRates[d->version][samplerateIndex];

  if(d->sampleRate == 0) {
    return;
  }

  // The channel mode is encoded as a 2 bit value at the end of the 3rd byte,
  // i.e. xxxxxx11

  d->channelMode = static_cast<ChannelMode>((static_cast<unsigned char>(data[3]) >> 6) & 0x03);

  // TODO: Add mode extension for completeness

  d->isOriginal    = ((static_cast<unsigned char>(data[3]) & 0x04) != 0);
  d->isCopyrighted = ((static_cast<unsigned char>(data[3]) & 0x08) != 0);
  d->isPadded      = ((static_cast<unsigned char>(data[2]) & 0x02) != 0);

  // Samples per frame

  static constexpr std::array samplesPerFrame {
    // MPEG1, 2/2.5
    std::pair(384, 384),   // Layer I
    std::pair(1152, 1152), // Layer II
    std::pair(1152, 576),  // Layer III
  };

  d->samplesPerFrame = versionIndex ? samplesPerFrame[layerIndex].second : samplesPerFrame[layerIndex].first;

  // Calculate the frame length

  static constexpr std::array paddingSize { 4, 1, 1 };

  d->frameLength = d->samplesPerFrame * d->bitrate * 125 / d->sampleRate;

  if(d->isPadded)
    d->frameLength += paddingSize[layerIndex];

  if(checkLength) {

    // Check if the frame length has been calculated correctly, or the next frame
    // header is right next to the end of this frame.

    // The MPEG versions, layers and sample rates of the two frames should be
    // consistent. Otherwise, we assume that either or both of the frames are
    // broken.

    file->seek(offset + d->frameLength);
    const ByteVector nextData = file->readBlock(4);

    if(nextData.size() < 4)
      return;

    const unsigned int HeaderMask = 0xfffe0c00;

    const unsigned int header     = data.toUInt(0, true)     & HeaderMask;
    const unsigned int nextHeader = nextData.toUInt(0, true) & HeaderMask;

    if(header != nextHeader)
      return;
  }

  // Now that we're done parsing, set this to be a valid frame.

  d->isValid = true;
}
