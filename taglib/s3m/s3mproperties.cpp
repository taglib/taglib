/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#include "s3mproperties.h"

using namespace TagLib;
using namespace S3M;

class S3M::AudioProperties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    lengthInPatterns(0),
    channels(0),
    stereo(false),
    sampleCount(0),
    patternCount(0),
    flags(0),
    trackerVersion(0),
    fileFormatVersion(0),
    globalVolume(0),
    masterVolume(0),
    tempo(0),
    bpmSpeed(0) {}

  ushort lengthInPatterns;
  int    channels;
  bool   stereo;
  ushort sampleCount;
  ushort patternCount;
  ushort flags;
  ushort trackerVersion;
  ushort fileFormatVersion;
  uchar  globalVolume;
  uchar  masterVolume;
  uchar  tempo;
  uchar  bpmSpeed;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

S3M::AudioProperties::AudioProperties(AudioProperties::ReadStyle propertiesStyle) :
  d(new PropertiesPrivate())
{
}

S3M::AudioProperties::~AudioProperties()
{
  delete d;
}

int S3M::AudioProperties::length() const
{
  return 0;
}

int S3M::AudioProperties::bitrate() const
{
  return 0;
}

int S3M::AudioProperties::sampleRate() const
{
  return 0;
}

int S3M::AudioProperties::channels() const
{
  return d->channels;
}

TagLib::ushort S3M::AudioProperties::lengthInPatterns() const
{
  return d->lengthInPatterns;
}

bool S3M::AudioProperties::stereo() const
{
  return d->stereo;
}

TagLib::ushort S3M::AudioProperties::sampleCount() const
{
  return d->sampleCount;
}

TagLib::ushort S3M::AudioProperties::patternCount() const
{
  return d->patternCount;
}

TagLib::ushort S3M::AudioProperties::flags() const
{
  return d->flags;
}

TagLib::ushort S3M::AudioProperties::trackerVersion() const
{
  return d->trackerVersion;
}

TagLib::ushort S3M::AudioProperties::fileFormatVersion() const
{
  return d->fileFormatVersion;
}

uchar S3M::AudioProperties::globalVolume() const
{
  return d->globalVolume;
}

uchar S3M::AudioProperties::masterVolume() const
{
  return d->masterVolume;
}

uchar S3M::AudioProperties::tempo() const
{
  return d->tempo;
}

uchar S3M::AudioProperties::bpmSpeed() const
{
  return d->bpmSpeed;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void S3M::AudioProperties::setLengthInPatterns(ushort lengthInPatterns)
{
  d->lengthInPatterns = lengthInPatterns;
}

void S3M::AudioProperties::setChannels(int channels)
{
  d->channels = channels;
}

void S3M::AudioProperties::setStereo(bool stereo)
{
  d->stereo = stereo;
}

void S3M::AudioProperties::setSampleCount(ushort sampleCount)
{
  d->sampleCount = sampleCount;
}

void S3M::AudioProperties::setPatternCount(ushort patternCount)
{
  d->patternCount = patternCount;
}

void S3M::AudioProperties::setFlags(ushort flags)
{
  d->flags = flags;
}

void S3M::AudioProperties::setTrackerVersion(ushort trackerVersion)
{
  d->trackerVersion = trackerVersion;
}

void S3M::AudioProperties::setFileFormatVersion(ushort fileFormatVersion)
{
  d->fileFormatVersion = fileFormatVersion;
}

void S3M::AudioProperties::setGlobalVolume(uchar globalVolume)
{
  d->globalVolume = globalVolume;
}

void S3M::AudioProperties::setMasterVolume(uchar masterVolume)
{
  d->masterVolume = masterVolume;
}

void S3M::AudioProperties::setTempo(uchar tempo)
{
  d->tempo = tempo;
}

void S3M::AudioProperties::setBpmSpeed(uchar bpmSpeed)
{
  d->bpmSpeed = bpmSpeed;
}
