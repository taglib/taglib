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
  ChannelConfiguration channelConfiguration { Custom };
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

MPEG::Header::ChannelConfiguration MPEG::Header::channelConfiguration() const
{
  return d->channelConfiguration;
}

bool MPEG::Header::isADTS() const
{
  // See detection in parse().
  return d->layer == 0 && (d->version == Version2 || d->version == Version4);
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

  if(const int layerBits = (static_cast<unsigned char>(data[1]) >> 1) & 0x03;
     layerBits == 1)
    d->layer = 3;
  else if(layerBits == 2)
    d->layer = 2;
  else if(layerBits == 3)
    d->layer = 1;
  else {
    // layerBits == 0 is reserved in the
    // <a href="http://www.mp3-tech.org/programmer/frame_header.html">
    // MPEG Audio Layer I/II/III frame header</a>, for
    // <a href="https://wiki.multimedia.cx/index.php/ADTS">ADTS</a>
    // they are always set to 0.
    // Bit 1 of versionBits is bit 4 of the 2nd header word. For ADTS
    // it must be set to 1, therefore these three bits are used to detect
    // that this header is from an ADTS file.
    if(versionBits == 2) {
      d->version = Version4;
      d->layer = 0;
    }
    else if(versionBits == 3) {
      d->version = Version2;
      d->layer = 0;
    }
    else {
      return;
    }
  }

  d->protectionEnabled = static_cast<unsigned char>(data[1] & 0x01) == 0;

  if(isADTS()) {
    static constexpr std::array sampleRates {
      96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
      16000, 12000, 11025, 8000, 7350, 0, 0, 0
    };

    const int sampleRateIndex = (static_cast<unsigned char>(data[2]) >> 2) & 0x0F;
    d->sampleRate = sampleRates[sampleRateIndex];
    d->samplesPerFrame = 1024;

    d->channelConfiguration = static_cast<ChannelConfiguration>(
      ((static_cast<unsigned char>(data[3]) >> 6) & 0x03) |
      ((static_cast<unsigned char>(data[2]) << 2) & 0x04));
    d->channelMode = d->channelConfiguration == FrontCenter ? SingleChannel : Stereo;

    // TODO: Add mode extension for completeness

    d->isOriginal = (static_cast<unsigned char>(data[3]) & 0x20) != 0;
    d->isCopyrighted = (static_cast<unsigned char>(data[3]) & 0x04) != 0;

    // Calculate the frame length
    if(const ByteVector frameLengthData = file->readBlock(2);
       frameLengthData.size() >= 2) {
      d->frameLength = (static_cast<unsigned char>(data[3]) & 0x3) << 11 |
                       (static_cast<unsigned char>(frameLengthData[0]) << 3) |
                       (static_cast<unsigned char>(frameLengthData[1]) >> 5);

      d->bitrate = static_cast<int>(d->frameLength * d->sampleRate / 1024.0 + 0.5) * 8 / 1024;
    }
  }
  else {
    // Not ADTS

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

    const int versionIndex = d->version == Version1 ? 0 : 1;
    const int layerIndex   = d->layer > 0 ? d->layer - 1 : 0;

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

    d->isOriginal    = (static_cast<unsigned char>(data[3]) & 0x04) != 0;
    d->isCopyrighted = (static_cast<unsigned char>(data[3]) & 0x08) != 0;
    d->isPadded      = (static_cast<unsigned char>(data[2]) & 0x02) != 0;

    // Samples per frame

    static constexpr std::array samplesPerFrameForLayer {
      // MPEG1, 2/2.5
      std::pair(384, 384),   // Layer I
      std::pair(1152, 1152), // Layer II
      std::pair(1152, 576),  // Layer III
    };

    d->samplesPerFrame = versionIndex
      ? samplesPerFrameForLayer[layerIndex].second
      : samplesPerFrameForLayer[layerIndex].first;

    // Calculate the frame length

    static constexpr std::array paddingSize { 4, 1, 1 };

    d->frameLength = d->samplesPerFrame * d->bitrate * 125 / d->sampleRate;

    if(d->isPadded)
      d->frameLength += paddingSize[layerIndex];
  }

  if(checkLength) {

    // Check if the frame length has been calculated correctly, or the next frame
    // header is right next to the end of this frame.

    // The MPEG versions, layers and sample rates of the two frames should be
    // consistent. Otherwise, we assume that either or both of the frames are
    // broken.

    // A frame length of 0 is probably invalid and would pass the test below
    // because nextData would be the same as data.
    if(d->frameLength == 0)
      return;

    file->seek(offset + d->frameLength);
    const ByteVector nextData = file->readBlock(4);

    if(nextData.size() < 4)
      return;

    constexpr unsigned int HeaderMask = 0xfffe0c00;

    const unsigned int header     = data.toUInt(0, true)     & HeaderMask;

    if(const unsigned int nextHeader = nextData.toUInt(0, true) & HeaderMask;
       header != nextHeader)
      return;
  }

  // Now that we're done parsing, set this to be a valid frame.

  d->isValid = true;
}
