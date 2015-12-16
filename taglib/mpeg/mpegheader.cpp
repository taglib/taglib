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

#include <tbytevector.h>
#include <tstring.h>
#include <tdebug.h>
#include <tsmartptr.h>

#include "mpegheader.h"

using namespace TagLib;

namespace
{
  struct HeaderData
  {
    bool isValid;
    MPEG::Header::Version version;
    int layer;
    bool protectionEnabled;
    int bitrate;
    int sampleRate;
    bool isPadded;
    MPEG::Header::ChannelMode channelMode;
    bool isCopyrighted;
    bool isOriginal;
    int frameLength;
    int samplesPerFrame;
  };
}
class MPEG::Header::HeaderPrivate
{
public:
  HeaderPrivate() :
    data(new HeaderData())
  {
    data->isValid           = false;
    data->layer             = 0;
    data->version           = Version1;
    data->protectionEnabled = false;
    data->sampleRate        = 0;
    data->isPadded          = false;
    data->channelMode       = Stereo;
    data->isCopyrighted     = false;
    data->isOriginal        = false;
    data->frameLength       = 0;
    data->samplesPerFrame   = 0;
  }

  SHARED_PTR<HeaderData> data;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPEG::Header::Header(const ByteVector &data) :
  d(new HeaderPrivate())
{
  parse(data);
}

MPEG::Header::Header(const Header &h) :
  d(new HeaderPrivate(*h.d))
{
}

MPEG::Header::~Header()
{
  delete d;
}

bool MPEG::Header::isValid() const
{
  return d->data->isValid;
}

MPEG::Header::Version MPEG::Header::version() const
{
  return d->data->version;
}

int MPEG::Header::layer() const
{
  return d->data->layer;
}

bool MPEG::Header::protectionEnabled() const
{
  return d->data->protectionEnabled;
}

int MPEG::Header::bitrate() const
{
  return d->data->bitrate;
}

int MPEG::Header::sampleRate() const
{
  return d->data->sampleRate;
}

bool MPEG::Header::isPadded() const
{
  return d->data->isPadded;
}

MPEG::Header::ChannelMode MPEG::Header::channelMode() const
{
  return d->data->channelMode;
}

bool MPEG::Header::isCopyrighted() const
{
  return d->data->isCopyrighted;
}

bool MPEG::Header::isOriginal() const
{
  return d->data->isOriginal;
}

int MPEG::Header::frameLength() const
{
  return d->data->frameLength;
}

int MPEG::Header::samplesPerFrame() const
{
  return d->data->samplesPerFrame;
}

MPEG::Header &MPEG::Header::operator=(const Header &h)
{
  *d = *h.d;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MPEG::Header::parse(const ByteVector &data)
{
  if(data.size() < 4) {
    debug("MPEG::Header::parse() -- data is too short for an MPEG frame header.");
    return;
  }

  // Check for the MPEG synch bytes.

  if(static_cast<unsigned char>(data[0]) != 0xFF) {
    debug("MPEG::Header::parse() -- First byte did not match MPEG synch.");
    return;
  }

  if((static_cast<unsigned char>(data[1]) & 0xE0) != 0xE0) {
    debug("MPEG::Header::parse() -- Second byte did not match MPEG synch.");
    return;
  }

  // Set the MPEG version

  const int versionBits = (static_cast<unsigned char>(data[1]) >> 3) & 0x03;

  if(versionBits == 0)
    d->data->version = Version2_5;
  else if(versionBits == 2)
    d->data->version = Version2;
  else if(versionBits == 3)
    d->data->version = Version1;
  else {
    debug("MPEG::Header::parse() -- Invalid MPEG version bits.");
    return;
  }

  // Set the MPEG layer

  const int layerBits = (static_cast<unsigned char>(data[1]) >> 1) & 0x03;

  if(layerBits == 1)
    d->data->layer = 3;
  else if(layerBits == 2)
    d->data->layer = 2;
  else if(layerBits == 3)
    d->data->layer = 1;
  else {
    debug("MPEG::Header::parse() -- Invalid MPEG layer bits.");
    return;
  }

  d->data->protectionEnabled = (static_cast<unsigned char>(data[1] & 0x01) == 0);

  // Set the bitrate

  static const int bitrates[2][3][16] = {
    { // Version 1
      { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 }, // layer 1
      { 0, 32, 48, 56, 64,  80,  96,  112, 128, 160, 192, 224, 256, 320, 384, 0 }, // layer 2
      { 0, 32, 40, 48, 56,  64,  80,  96,  112, 128, 160, 192, 224, 256, 320, 0 }  // layer 3
    },
    { // Version 2 or 2.5
      { 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }, // layer 1
      { 0, 8,  16, 24, 32, 40, 48, 56,  64,  80,  96,  112, 128, 144, 160, 0 }, // layer 2
      { 0, 8,  16, 24, 32, 40, 48, 56,  64,  80,  96,  112, 128, 144, 160, 0 }  // layer 3
    }
  };

  const int versionIndex = (d->data->version == Version1) ? 0 : 1;
  const int layerIndex   = (d->data->layer > 0) ? d->data->layer - 1 : 0;

  // The bitrate index is encoded as the first 4 bits of the 3rd byte,
  // i.e. 1111xxxx

  const int bitrateIndex = (static_cast<unsigned char>(data[2]) >> 4) & 0x0F;

  d->data->bitrate = bitrates[versionIndex][layerIndex][bitrateIndex];

  if(d->data->bitrate == 0) {
    debug("MPEG::Header::parse() -- Invalid bit rate.");
    return;
  }

  // Set the sample rate

  static const int sampleRates[3][4] = {
    { 44100, 48000, 32000, 0 }, // Version 1
    { 22050, 24000, 16000, 0 }, // Version 2
    { 11025, 12000, 8000,  0 }  // Version 2.5
  };

  // The sample rate index is encoded as two bits in the 3nd byte, i.e. xxxx11xx

  const int samplerateIndex = (static_cast<unsigned char>(data[2]) >> 2) & 0x03;

  d->data->sampleRate = sampleRates[d->data->version][samplerateIndex];

  if(d->data->sampleRate == 0) {
    debug("MPEG::Header::parse() -- Invalid sample rate.");
    return;
  }

  // The channel mode is encoded as a 2 bit value at the end of the 3nd byte,
  // i.e. xxxxxx11

  d->data->channelMode = static_cast<ChannelMode>((static_cast<unsigned char>(data[3]) >> 6) & 0x03);

  // TODO: Add mode extension for completeness

  d->data->isOriginal    = ((static_cast<unsigned char>(data[3]) & 0x04) != 0);
  d->data->isCopyrighted = ((static_cast<unsigned char>(data[3]) & 0x08) != 0);
  d->data->isPadded      = ((static_cast<unsigned char>(data[2]) & 0x02) != 0);

  // Samples per frame

  static const int samplesPerFrame[3][2] = {
    // MPEG1, 2/2.5
    {  384,   384 }, // Layer I
    { 1152,  1152 }, // Layer II
    { 1152,   576 }  // Layer III
  };

  d->data->samplesPerFrame = samplesPerFrame[layerIndex][versionIndex];

  // Calculate the frame length

  static const int paddingSize[3] = { 4, 1, 1 };

  d->data->frameLength = d->data->samplesPerFrame * d->data->bitrate * 125 / d->data->sampleRate;

  if(d->data->isPadded)
    d->data->frameLength += paddingSize[layerIndex];

  // Now that we're done parsing, set this to be a valid frame.

  d->data->isValid = true;
}
