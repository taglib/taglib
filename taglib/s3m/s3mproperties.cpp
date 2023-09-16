/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
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

#include "s3mproperties.h"

using namespace TagLib;
using namespace S3M;

class S3M::Properties::PropertiesPrivate
{
public:
  unsigned short lengthInPatterns { 0 };
  int channels { 0 };
  bool stereo { false };
  unsigned short sampleCount { 0 };
  unsigned short patternCount { 0 };
  unsigned short flags { 0 };
  unsigned short trackerVersion { 0 };
  unsigned short fileFormatVersion { 0 };
  unsigned char globalVolume { 0 };
  unsigned char masterVolume { 0 };
  unsigned char tempo { 0 };
  unsigned char bpmSpeed { 0 };
};

S3M::Properties::Properties(AudioProperties::ReadStyle propertiesStyle) :
  AudioProperties(propertiesStyle),
  d(std::make_unique<PropertiesPrivate>())
{
}

S3M::Properties::~Properties() = default;

int S3M::Properties::channels() const
{
  return d->channels;
}

unsigned short S3M::Properties::lengthInPatterns() const
{
  return d->lengthInPatterns;
}

bool S3M::Properties::stereo() const
{
  return d->stereo;
}

unsigned short S3M::Properties::sampleCount() const
{
  return d->sampleCount;
}

unsigned short S3M::Properties::patternCount() const
{
  return d->patternCount;
}

unsigned short S3M::Properties::flags() const
{
  return d->flags;
}

unsigned short S3M::Properties::trackerVersion() const
{
  return d->trackerVersion;
}

unsigned short S3M::Properties::fileFormatVersion() const
{
  return d->fileFormatVersion;
}

unsigned char S3M::Properties::globalVolume() const
{
  return d->globalVolume;
}

unsigned char S3M::Properties::masterVolume() const
{
  return d->masterVolume;
}

unsigned char S3M::Properties::tempo() const
{
  return d->tempo;
}

unsigned char S3M::Properties::bpmSpeed() const
{
  return d->bpmSpeed;
}

void S3M::Properties::setLengthInPatterns(unsigned short lengthInPatterns)
{
  d->lengthInPatterns = lengthInPatterns;
}

void S3M::Properties::setChannels(int channels)
{
  d->channels = channels;
}

void S3M::Properties::setStereo(bool stereo)
{
  d->stereo = stereo;
}

void S3M::Properties::setSampleCount(unsigned short sampleCount)
{
  d->sampleCount = sampleCount;
}

void S3M::Properties::setPatternCount(unsigned short patternCount)
{
  d->patternCount = patternCount;
}

void S3M::Properties::setFlags(unsigned short flags)
{
  d->flags = flags;
}

void S3M::Properties::setTrackerVersion(unsigned short trackerVersion)
{
  d->trackerVersion = trackerVersion;
}

void S3M::Properties::setFileFormatVersion(unsigned short fileFormatVersion)
{
  d->fileFormatVersion = fileFormatVersion;
}

void S3M::Properties::setGlobalVolume(unsigned char globalVolume)
{
  d->globalVolume = globalVolume;
}

void S3M::Properties::setMasterVolume(unsigned char masterVolume)
{
  d->masterVolume = masterVolume;
}

void S3M::Properties::setTempo(unsigned char tempo)
{
  d->tempo = tempo;
}

void S3M::Properties::setBpmSpeed(unsigned char bpmSpeed)
{
  d->bpmSpeed = bpmSpeed;
}
