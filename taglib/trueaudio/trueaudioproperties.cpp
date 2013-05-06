/***************************************************************************
    copyright            : (C) 2006 by Lukáš Lalinský
    email                : lalinsky@gmail.com

    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
                           (original MPC implementation)
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
#include <bitset>

#include "trueaudioproperties.h"
#include "trueaudiofile.h"

using namespace TagLib;

class TrueAudio::AudioProperties::PropertiesPrivate
{
public:
  PropertiesPrivate(const ByteVector &d, offset_t length, ReadStyle s) :
    data(d),
    streamLength(length),
    style(s),
    version(0),
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    bitsPerSample(0),
    sampleFrames(0) {}

  ByteVector data;
  offset_t streamLength;
  ReadStyle style;
  int version;
  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int bitsPerSample;
  uint sampleFrames;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

TrueAudio::AudioProperties::AudioProperties(
    const ByteVector &data, offset_t streamLength, ReadStyle style) 
  : TagLib::AudioProperties(style)
{
  d = new PropertiesPrivate(data, streamLength, style);
  read();
}

TrueAudio::AudioProperties::~AudioProperties()
{
  delete d;
}

int TrueAudio::AudioProperties::length() const
{
  return d->length;
}

int TrueAudio::AudioProperties::bitrate() const
{
  return d->bitrate;
}

int TrueAudio::AudioProperties::sampleRate() const
{
  return d->sampleRate;
}

int TrueAudio::AudioProperties::bitsPerSample() const
{
  return d->bitsPerSample;
}

int TrueAudio::AudioProperties::channels() const
{
  return d->channels;
}

TagLib::uint TrueAudio::AudioProperties::sampleFrames() const
{
  return d->sampleFrames;
}

int TrueAudio::AudioProperties::ttaVersion() const
{
  return d->version;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void TrueAudio::AudioProperties::read()
{
  if(!d->data.startsWith("TTA"))
    return;

  int pos = 3;

  d->version = d->data[pos] - '0';
  pos += 1;

  // According to http://en.true-audio.com/TTA_Lossless_Audio_Codec_-_Format_Description
  // TTA2 headers are in development, and have a different format
  if(1 == d->version) {
    // Skip the audio format
    pos += 2;

    d->channels = d->data.toInt16LE(pos);
    pos += 2;

    d->bitsPerSample = d->data.toInt16LE(pos);
    pos += 2;

    d->sampleRate = d->data.toUInt32LE(pos);
    pos += 4;

    d->sampleFrames = d->data.toUInt32LE(pos);
    d->length = d->sampleRate > 0 ? d->sampleFrames / d->sampleRate : 0;

    d->bitrate = d->length > 0 ? static_cast<int>(d->streamLength * 8L / d->length / 1000) : 0;
  }
}
