/***************************************************************************
    copyright           : (C) 2018 inMusic brands, inc.
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

#include "adtsheader.h"

#include <array>

#include "tbytevector.h"
#include "tdebug.h"
#include "tfile.h"
#include "mpegutils.h"

using namespace TagLib;

class MPEG::ADTSHeader::HeaderPrivate
{
public:
  bool isValid { false };
  Header::Version version { Header::Version4 };
  bool protectionEnabled { false };
  int bitrate { 0 };
  int sampleRate { 0 };
  ChannelMode channelMode { Custom };
  bool isCopyrighted { false };
  bool isOriginal { false };
  int frameLength { 0 };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPEG::ADTSHeader::ADTSHeader(File *file, offset_t offset) :
  d(std::make_shared<HeaderPrivate>())
{
  parse(file, offset);
}

MPEG::ADTSHeader::ADTSHeader(const ADTSHeader &h) = default;
MPEG::ADTSHeader::~ADTSHeader() = default;

bool MPEG::ADTSHeader::isValid() const
{
  return d->isValid;
}

MPEG::Header::Version MPEG::ADTSHeader::version() const
{
  return d->version;
}

bool MPEG::ADTSHeader::protectionEnabled() const
{
  return d->protectionEnabled;
}

int MPEG::ADTSHeader::bitrate() const
{
  return d->bitrate;
}

int MPEG::ADTSHeader::sampleRate() const
{
  return d->sampleRate;
}

MPEG::ADTSHeader::ChannelMode MPEG::ADTSHeader::channelMode() const
{
  return d->channelMode;
}

bool MPEG::ADTSHeader::isCopyrighted() const
{
  return d->isCopyrighted;
}

bool MPEG::ADTSHeader::isOriginal() const
{
  return d->isOriginal;
}

int MPEG::ADTSHeader::frameLength() const
{
  return d->frameLength;
}

MPEG::ADTSHeader &MPEG::ADTSHeader::operator=(const ADTSHeader &h)
{
  if(&h == this)
    return *this;

  d = h.d;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MPEG::ADTSHeader::parse(File *file, offset_t offset)
{
  file->seek(offset);
  const ByteVector data = file->readBlock(6);

  if(data.size() < 6) {
    debug("MPEG::ADTSHeader::parse() -- data is too short for an ADTS frame header.");
    return;
  }

  // Check for the MPEG sync bytes.

  if(!isFrameSync(data)) {
    debug("MPEG::ADTSHeader::parse() -- MPEG header did not match MPEG synch.");
    return;
  }

  // Set the MPEG version

  if((static_cast<unsigned char>(data[1]) & 0x08) == 0x08)
    d->version = Header::Version2;
  else
    d->version = Header::Version4;

  // Set the MPEG layer

  d->protectionEnabled = (static_cast<unsigned char>(data[1] & 0x01) == 0);

  // Set the sample rate

  static constexpr std::array sampleRates {
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000, 7350, 0, 0, 0
  };

  const int samplerateIndex = (static_cast<unsigned char>(data[2]) >> 2) & 0x0F;

  d->sampleRate = sampleRates[samplerateIndex];

  if(d->sampleRate == 0)
    return;

  // The channel mode is encoded as a 3 bit value at the end of the 3rd byte,
  // i.e. xxxxx111

  d->channelMode = static_cast<ChannelMode>((static_cast<unsigned char>(data[3]) >> 6) & 0x07);

  // TODO: Add mode extension for completeness

  d->isOriginal    = ((static_cast<unsigned char>(data[3]) & 0x04) != 0);
  d->isCopyrighted = ((static_cast<unsigned char>(data[3]) & 0x08) != 0);

  // Calculate the frame length

  if(data.size() >= 6) {
    d->frameLength = (static_cast<unsigned char>(data[3]) & 0x3) << 11 |
                     (static_cast<unsigned char>(data[4]) << 3) |
                     (static_cast<unsigned char>(data[5]) >> 5);

    d->bitrate = static_cast<int>(d->frameLength * d->sampleRate / 1024.0 + 0.5) * 8 / 1024;
  }
  // Now that we're done parsing, set this to be a valid frame.

  d->isValid = true;
}
