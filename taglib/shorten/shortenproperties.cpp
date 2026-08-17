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


#include "shortenproperties.h"

#include <limits>

#include "shortenutils.h"

using namespace TagLib;

class Shorten::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() = default;
  ~PropertiesPrivate() = default;

  PropertiesPrivate(const PropertiesPrivate &) = delete;
  PropertiesPrivate &operator=(const PropertiesPrivate &) = delete;

  int version { 0 };
  int fileType { 0 };
  int channelCount { 0 };
  int sampleRate { 0 };
  int bitsPerSample { 0 };
  unsigned long sampleFrames { 0 };

  // Computed
  int bitrate { 0 };
  int length { 0 };
};

Shorten::Properties::Properties(const PropertyValues *values, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  if(values) {
    d->version = values->version;
    d->fileType = values->fileType;
    d->channelCount = values->channelCount;
    d->sampleRate = values->sampleRate;
    d->bitsPerSample = values->bitsPerSample;
    d->sampleFrames = values->sampleFrames;

    const auto maxInt = static_cast<double>(std::numeric_limits<int>::max());
    const double bitrate = static_cast<double>(d->sampleRate) * d->bitsPerSample * d->channelCount / 1000.0;
    if(bitrate >= 0.0 && bitrate + 0.5 <= maxInt)
      d->bitrate = static_cast<int>(bitrate + 0.5);

    if(d->sampleRate > 0) {
      const double length = static_cast<double>(d->sampleFrames) * 1000.0 / d->sampleRate;
      if(length > 0.0 && length + 0.5 <= maxInt)
        d->length = static_cast<int>(length + 0.5);
    }
  }
}

Shorten::Properties::~Properties() = default;

int Shorten::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int Shorten::Properties::bitrate() const
{
  return d->bitrate;
}

int Shorten::Properties::sampleRate() const
{
  return d->sampleRate;
}

int Shorten::Properties::channels() const
{
  return d->channelCount;
}

int Shorten::Properties::shortenVersion() const
{
  return d->version;
}

int Shorten::Properties::fileType() const
{
  return d->fileType;
}

int Shorten::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

unsigned long Shorten::Properties::sampleFrames() const
{
  return d->sampleFrames;
}
