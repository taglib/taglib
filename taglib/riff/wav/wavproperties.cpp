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

#include <tstring.h>
#include <tdebug.h>
#include "wavproperties.h"

using namespace TagLib;

class RIFF::WAV::AudioProperties::PropertiesPrivate
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

RIFF::WAV::AudioProperties::AudioProperties(const ByteVector &data, uint streamLength, 
                                            ReadStyle style) :
  d(new PropertiesPrivate())
{
  read(data, streamLength);
}

RIFF::WAV::AudioProperties::~AudioProperties()
{
  delete d;
}

int RIFF::WAV::AudioProperties::length() const
{
  return d->length;
}

int RIFF::WAV::AudioProperties::bitrate() const
{
  return d->bitrate;
}

int RIFF::WAV::AudioProperties::sampleRate() const
{
  return d->sampleRate;
}

int RIFF::WAV::AudioProperties::channels() const
{
  return d->channels;
}

int RIFF::WAV::AudioProperties::sampleWidth() const
{
  return d->sampleWidth;
}

TagLib::uint RIFF::WAV::AudioProperties::sampleFrames() const
{
  return d->sampleFrames;
}

TagLib::uint RIFF::WAV::AudioProperties::format() const
{
  return d->format;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::WAV::AudioProperties::read(const ByteVector &data, uint streamLength)
{
  if(data.size() < 16) {
    debug("RIFF::WAV::AudioProperties::read() - \"fmt \" chunk is too short.");
    return;
  }

  d->format      = data.toInt16LE(0);
  d->channels    = data.toInt16LE(2);
  d->sampleRate  = data.toUInt32LE(4);
  d->sampleWidth = data.toInt16LE(14);

  const uint byteRate = data.toUInt32LE(8);
  d->bitrate = byteRate * 8 / 1000;

  d->length = byteRate > 0 ? streamLength / byteRate : 0;

  // format ID 1 means uncompressed PCM.
  if(d->format == 1 && d->channels > 0 && d->sampleWidth > 0)
    d->sampleFrames = streamLength / (d->channels * ((d->sampleWidth + 7) / 8));
}
