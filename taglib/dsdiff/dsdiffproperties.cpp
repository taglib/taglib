/***************************************************************************
    copyright            : (C) 2016 by Damien Plisson, Audirvana
    email                : damien78@audirvana.com
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

#include "dsdiffproperties.h"

#include "tstring.h"

using namespace TagLib;

class DSDIFF::Properties::PropertiesPrivate
{
public:
  int length { 0 };
  int bitrate { 0 };
  int sampleRate { 0 };
  int channels { 0 };
  int sampleWidth { 0 };
  unsigned long long sampleCount { 0 };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSDIFF::Properties::Properties(unsigned int sampleRate,
                               unsigned short channels,
                               unsigned long long samplesCount,
                               int bitrate,
                               ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  d->channels = channels;
  d->sampleCount = samplesCount;
  d->sampleWidth = 1;
  d->sampleRate = sampleRate;
  d->bitrate = bitrate;
  d->length = d->sampleRate > 0
    ? static_cast<int>(d->sampleCount * 1000.0 / d->sampleRate + 0.5)
    : 0;
}

DSDIFF::Properties::~Properties() = default;

int DSDIFF::Properties::lengthInSeconds() const
{
  return d->length / 1000;
}

int DSDIFF::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int DSDIFF::Properties::bitrate() const
{
  return d->bitrate;
}

int DSDIFF::Properties::sampleRate() const
{
  return d->sampleRate;
}

int DSDIFF::Properties::channels() const
{
  return d->channels;
}

int DSDIFF::Properties::bitsPerSample() const
{
  return d->sampleWidth;
}

long long DSDIFF::Properties::sampleCount() const
{
  return d->sampleCount;
}
