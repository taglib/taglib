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

#include "trueaudioproperties.h"

#include "tdebug.h"
#include "tstring.h"

using namespace TagLib;

class TrueAudio::Properties::PropertiesPrivate
{
public:
  int version { 0 };
  int length { 0 };
  int bitrate { 0 };
  int sampleRate { 0 };
  int channels { 0 };
  int bitsPerSample { 0 };
  unsigned int sampleFrames { 0 };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

TrueAudio::Properties::Properties(const ByteVector &data, offset_t streamLength, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(data, streamLength);
}

TrueAudio::Properties::~Properties() = default;

int TrueAudio::Properties::lengthInSeconds() const
{
  return d->length / 1000;
}

int TrueAudio::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int TrueAudio::Properties::bitrate() const
{
  return d->bitrate;
}

int TrueAudio::Properties::sampleRate() const
{
  return d->sampleRate;
}

int TrueAudio::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

int TrueAudio::Properties::channels() const
{
  return d->channels;
}

unsigned int TrueAudio::Properties::sampleFrames() const
{
  return d->sampleFrames;
}

int TrueAudio::Properties::ttaVersion() const
{
  return d->version;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void TrueAudio::Properties::read(const ByteVector &data, offset_t streamLength)
{
  if(data.size() < 4) {
    debug("TrueAudio::Properties::read() -- data is too short.");
    return;
  }

  if(!data.startsWith("TTA")) {
    debug("TrueAudio::Properties::read() -- invalid header signature.");
    return;
  }

  unsigned int pos = 3;

  d->version = data[pos] - '0';
  pos += 1;

  // According to http://en.true-audio.com/TTA_Lossless_Audio_Codec_-_Format_Description
  // TTA2 headers are in development, and have a different format
  if(1 == d->version) {
    if(data.size() < 18) {
      debug("TrueAudio::Properties::read() -- data is too short.");
      return;
    }

    // Skip the audio format
    pos += 2;

    d->channels = data.toShort(pos, false);
    pos += 2;

    d->bitsPerSample = data.toShort(pos, false);
    pos += 2;

    d->sampleRate = data.toUInt(pos, false);
    pos += 4;

    d->sampleFrames = data.toUInt(pos, false);

    if(d->sampleFrames > 0 && d->sampleRate > 0) {
      const auto length = static_cast<double>(d->sampleFrames) * 1000.0 / d->sampleRate;
      d->length  = static_cast<int>(length + 0.5);
      d->bitrate = static_cast<int>(streamLength * 8.0 / length + 0.5);
    }
  }
}
