/***************************************************************************
    copyright            : (C) 2003 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
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

#include "flacproperties.h"

#include "tstring.h"
#include "tdebug.h"

using namespace TagLib;

class FLAC::Properties::PropertiesPrivate
{
public:
  int length { 0 };
  int bitrate { 0 };
  int sampleRate { 0 };
  int bitsPerSample { 0 };
  int channels { 0 };
  unsigned long long sampleFrames { 0 };
  ByteVector signature;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FLAC::Properties::Properties(const ByteVector &data, offset_t streamLength, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(data, streamLength);
}

FLAC::Properties::~Properties() = default;

int FLAC::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int FLAC::Properties::bitrate() const
{
  return d->bitrate;
}

int FLAC::Properties::sampleRate() const
{
  return d->sampleRate;
}

int FLAC::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

int FLAC::Properties::channels() const
{
  return d->channels;
}

unsigned long long FLAC::Properties::sampleFrames() const
{
  return d->sampleFrames;
}

ByteVector FLAC::Properties::signature() const
{
  return d->signature;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void FLAC::Properties::read(const ByteVector &data, offset_t streamLength)
{
  if(data.size() < 18) {
    debug("FLAC::Properties::read() - FLAC properties must contain at least 18 bytes.");
    return;
  }

  unsigned int pos = 0;

  // Minimum block size (in samples)
  pos += 2;

  // Maximum block size (in samples)
  pos += 2;

  // Minimum frame size (in bytes)
  pos += 3;

  // Maximum frame size (in bytes)
  pos += 3;

  const unsigned int flags = data.toUInt(pos, true);
  pos += 4;

  d->sampleRate    = flags >> 12;
  d->channels      = ((flags >> 9) &  7) + 1;
  d->bitsPerSample = ((flags >> 4) & 31) + 1;

  // The last 4 bits are the most significant 4 bits for the 36 bit
  // stream length in samples. (Audio files measured in days)

  const unsigned long long hi = flags & 0xf;
  const unsigned long long lo = data.toUInt(pos, true);
  pos += 4;

  d->sampleFrames = (hi << 32) | lo;

  if(d->sampleFrames > 0 && d->sampleRate > 0) {
    const auto length = static_cast<double>(d->sampleFrames) * 1000.0 / d->sampleRate;
    d->length  = static_cast<int>(length + 0.5);
    d->bitrate = static_cast<int>(streamLength * 8.0 / length + 0.5);
  }

  if(data.size() >= pos + 16)
    d->signature = data.mid(pos, 16);
}
