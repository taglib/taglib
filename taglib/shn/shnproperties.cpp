/***************************************************************************
    copyright           : (C) 2020-2024 Stephen F. Booth
    email               : me@sbooth.org
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


#include "shnproperties.h"

#include "shnutils.h"

using namespace TagLib;

class SHN::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() = default;
  ~PropertiesPrivate() = default;

  PropertiesPrivate(const PropertiesPrivate &) = delete;
  PropertiesPrivate &operator=(const PropertiesPrivate &) = delete;

  int version { 0 };
  int internalFileType { 0 };
  int channelCount { 0 };
  unsigned long sampleFrames { 0 };
  int sampleRate { 0 };
  int bitsPerSample { 0 };

  // Computed
  int bitrate { 0 };
  int length { 0 };
};

SHN::Properties::Properties(const PropertyValues *values, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  if(values) {
    d->version = values->version;
    d->internalFileType = values->internal_file_type;
    d->channelCount = values->channel_count;
    d->sampleRate = values->sample_rate;
    d->bitsPerSample = values->bits_per_sample;
    d->sampleFrames = values->sample_frames;

    d->bitrate = static_cast<int>(d->sampleRate * d->bitsPerSample * d->channelCount / 1000.0 + 0.5);
    if(d->sampleRate > 0)
      d->length = static_cast<int>(d->sampleFrames * 1000.0 / d->sampleRate + 0.5);
  }
}

SHN::Properties::~Properties() = default;

int SHN::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int SHN::Properties::bitrate() const
{
  return d->bitrate;
}

int SHN::Properties::sampleRate() const
{
  return d->sampleRate;
}

int SHN::Properties::channels() const
{
  return d->channelCount;
}

int SHN::Properties::shortenVersion() const
{
  return d->version;
}

int SHN::Properties::internalFileType() const
{
  return d->internalFileType;
}

int SHN::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

unsigned long SHN::Properties::sampleFrames() const
{
  return d->sampleFrames;
}
