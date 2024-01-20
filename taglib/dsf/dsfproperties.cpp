/***************************************************************************
    copyright           : (C) 2013-2023 Stephen F. Booth
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


#include "dsfproperties.h"

using namespace TagLib;

class DSF::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() = default;
  ~PropertiesPrivate() = default;

  PropertiesPrivate(const PropertiesPrivate &) = delete;
  PropertiesPrivate &operator=(const PropertiesPrivate &) = delete;

  // Nomenclature is from DSF file format specification
  unsigned int formatVersion = 0;
  unsigned int formatID = 0;
  unsigned int channelType = 0;
  unsigned int channelNum = 0;
  unsigned int samplingFrequency = 0;
  unsigned int bitsPerSample = 0;
  long long sampleCount = 0;
  unsigned int blockSizePerChannel = 0;

  // Computed
  unsigned int bitrate = 0;
  unsigned int length = 0;
};

DSF::Properties::Properties(const ByteVector &data, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(data);
}

DSF::Properties::~Properties() = default;

int DSF::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int DSF::Properties::bitrate() const
{
  return d->bitrate;
}

int DSF::Properties::sampleRate() const
{
  return d->samplingFrequency;
}

int DSF::Properties::channels() const
{
  return d->channelNum;
}

int DSF::Properties::formatVersion() const
{
  return d->formatVersion;
}

int DSF::Properties::formatID() const
{
  return d->formatID;
}

int DSF::Properties::channelType() const
{
  return d->channelType;
}

int DSF::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

long long DSF::Properties::sampleCount() const
{
  return d->sampleCount;
}

int DSF::Properties::blockSizePerChannel() const
{
  return d->blockSizePerChannel;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DSF::Properties::read(const ByteVector &data)
{
  d->formatVersion = data.toUInt(0U,false);
  d->formatID = data.toUInt(4U,false);
  d->channelType = data.toUInt(8U,false);
  d->channelNum = data.toUInt(12U,false);
  d->samplingFrequency = data.toUInt(16U,false);
  d->bitsPerSample = data.toUInt(20U,false);
  d->sampleCount = data.toLongLong(24U,false);
  d->blockSizePerChannel = data.toUInt(32U,false);

  d->bitrate = static_cast<unsigned int>(
      d->samplingFrequency * d->bitsPerSample * d->channelNum / 1000.0 + 0.5);
  d->length = d->samplingFrequency > 0
      ? static_cast<unsigned int>(d->sampleCount * 1000.0 / d->samplingFrequency + 0.5)
      : 0;
}
