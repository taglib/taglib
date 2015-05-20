/***************************************************************************
    copyright            : (C) 2008 by Scott Wheeler
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

#include "wavproperties.h"

#include <tstring.h>
#include <tdebug.h>
#include <cmath>
#include <math.h>

using namespace TagLib;

class RIFF::WAV::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    format(0),
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    sampleWidth(0),
    sampleFrames(0) {}

  short format;
  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int sampleWidth;
  uint sampleFrames;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::WAV::Properties::Properties(const ByteVector & /*data*/, ReadStyle style) :
  AudioProperties(style),
  d(new PropertiesPrivate())
{
  debug("RIFF::WAV::Properties::Properties() -- This constructor is no longer used.");
}

RIFF::WAV::Properties::Properties(const ByteVector &data, uint streamLength, ReadStyle style) :
  AudioProperties(style),
  d(new PropertiesPrivate())
{
  read(data, streamLength);
}

RIFF::WAV::Properties::~Properties()
{
  delete d;
}

int RIFF::WAV::Properties::length() const
{
  return lengthInSeconds();
}

int RIFF::WAV::Properties::lengthInSeconds() const
{
  return d->length / 1000;
}

int RIFF::WAV::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int RIFF::WAV::Properties::bitrate() const
{
  return d->bitrate;
}

int RIFF::WAV::Properties::sampleRate() const
{
  return d->sampleRate;
}

int RIFF::WAV::Properties::channels() const
{
  return d->channels;
}

int RIFF::WAV::Properties::sampleWidth() const
{
  return d->sampleWidth;
}

TagLib::uint RIFF::WAV::Properties::sampleFrames() const
{
  return d->sampleFrames;
}

TagLib::uint RIFF::WAV::Properties::format() const
{
  return d->format;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::WAV::Properties::read(const ByteVector &data, uint streamLength)
{
  if(data.size() < 16) {
    debug("RIFF::WAV::Properties::read() - \"fmt \" chunk is too short for WAV.");
    return;
  }

  d->format      = data.toShort(0, false);
  d->channels    = data.toShort(2, false);
  d->sampleRate  = data.toUInt(4, false);
  d->sampleWidth = data.toShort(14, false);

  const uint byteRate = data.toUInt(8, false);
  d->bitrate = byteRate * 8 / 1000;

  if(streamLength > 0) {
    if(byteRate > 0)
      d->length = static_cast<int>(streamLength * 1000.0 / byteRate + 0.5);

    if(d->format == 1 && d->channels > 0 && d->sampleWidth > 0) {
      const int bytesPerSample = d->channels * ((d->sampleWidth + 7) / 8);
      d->sampleFrames = streamLength / bytesPerSample;
    }
  }
}
