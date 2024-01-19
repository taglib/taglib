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

#include "tdebug.h"
#include "wavfile.h"

using namespace TagLib;

namespace
{
  // Quoted from RFC 2361.
  enum WaveFormat
  {
    FORMAT_UNKNOWN = 0x0000,
    FORMAT_PCM     = 0x0001,
    FORMAT_IEEE_FLOAT = 0x0003
  };
} // namespace

class RIFF::WAV::Properties::PropertiesPrivate
{
public:
  int format { 0 };
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

TagLib::RIFF::WAV::Properties::Properties(File *file, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(file);
}

RIFF::WAV::Properties::~Properties() = default;

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

int RIFF::WAV::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

unsigned int RIFF::WAV::Properties::sampleFrames() const
{
  return d->sampleFrames;
}

int RIFF::WAV::Properties::format() const
{
  return d->format;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::WAV::Properties::read(File *file)
{
  ByteVector data;
  unsigned int streamLength = 0;
  unsigned int totalSamples = 0;

  for(unsigned int i = 0; i < file->chunkCount(); ++i) {
    if(const ByteVector name = file->chunkName(i); name == "fmt ") {
      if(data.isEmpty())
        data = file->chunkData(i);
      else
        debug("RIFF::WAV::Properties::read() - Duplicate 'fmt ' chunk found.");
    }
    else if(name == "data") {
      if(streamLength == 0)
        streamLength = file->chunkDataSize(i) + file->chunkPadding(i);
      else
        debug("RIFF::WAV::Properties::read() - Duplicate 'data' chunk found.");
    }
    else if(name == "fact") {
      if(totalSamples == 0)
        totalSamples = file->chunkData(i).toUInt(0, false);
      else
        debug("RIFF::WAV::Properties::read() - Duplicate 'fact' chunk found.");
    }
  }

  if(data.size() < 16) {
    debug("RIFF::WAV::Properties::read() - 'fmt ' chunk not found or too short.");
    return;
  }

  if(streamLength == 0) {
    debug("RIFF::WAV::Properties::read() - 'data' chunk not found.");
    return;
  }

  d->format = data.toShort(0, false);
  if((d->format & 0xffff) == 0xfffe) {
    // if extensible then read the format from the subformat
    if(data.size() != 40) {
      debug("RIFF::WAV::Properties::read() - extensible size incorrect");
      return;
    }
    d->format = data.toShort(24, false);
  }
  if(d->format != FORMAT_PCM && d->format != FORMAT_IEEE_FLOAT && totalSamples == 0) {
    debug("RIFF::WAV::Properties::read() - Non-PCM format, but 'fact' chunk not found.");
    return;
  }

  d->channels      = data.toShort(2, false);
  d->sampleRate    = data.toUInt(4, false);
  d->bitsPerSample = data.toShort(14, false);

  if(d->format != FORMAT_PCM && (d->format != FORMAT_IEEE_FLOAT || totalSamples != 0))
    d->sampleFrames = totalSamples;
  else if(d->channels > 0 && d->bitsPerSample > 0)
    d->sampleFrames = streamLength / (d->channels * ((d->bitsPerSample + 7) / 8));

  if(d->sampleFrames > 0 && d->sampleRate > 0) {
    const auto length = static_cast<double>(d->sampleFrames) * 1000.0 / d->sampleRate;
    d->length  = static_cast<int>(length + 0.5);
    d->bitrate = static_cast<int>(streamLength * 8.0 / length + 0.5);
  }
  else {
    if(const unsigned int byteRate = data.toUInt(8, false); byteRate > 0) {
      d->length  = static_cast<int>(streamLength * 1000.0 / byteRate + 0.5);
      d->bitrate = static_cast<int>(byteRate * 8.0 / 1000.0 + 0.5);
    }
  }
}
