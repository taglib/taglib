/***************************************************************************
    copyright           :(C) 2011 by Mathias Panzenböck
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

#include "itproperties.h"

using namespace TagLib;
using namespace IT;

class IT::AudioProperties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    channels(0),
    lengthInPatterns(0),
    instrumentCount(0),
    sampleCount(0),
    patternCount(0),
    version(0),
    compatibleVersion(0),
    flags(0),
    special(0),
    globalVolume(0),
    mixVolume(0),
    tempo(0),
    bpmSpeed(0),
    panningSeparation(0),
    pitchWheelDepth(0) {}

  int    channels;
  ushort lengthInPatterns;
  ushort instrumentCount;
  ushort sampleCount;
  ushort patternCount;
  ushort version;
  ushort compatibleVersion;
  ushort flags;
  ushort special;
  uchar  globalVolume;
  uchar  mixVolume;
  uchar  tempo;
  uchar  bpmSpeed;
  uchar  panningSeparation;
  uchar  pitchWheelDepth;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

IT::AudioProperties::AudioProperties(AudioProperties::ReadStyle propertiesStyle) : 
  d(new PropertiesPrivate())
{
}

IT::AudioProperties::~AudioProperties()
{
  delete d;
}

int IT::AudioProperties::length() const
{
  return 0;
}

int IT::AudioProperties::bitrate() const
{
  return 0;
}

int IT::AudioProperties::sampleRate() const
{
  return 0;
}

int IT::AudioProperties::channels() const
{
  return d->channels;
}

TagLib::ushort IT::AudioProperties::lengthInPatterns() const
{
  return d->lengthInPatterns;
}

bool IT::AudioProperties::stereo() const
{
  return d->flags & Stereo;
}

TagLib::ushort IT::AudioProperties::instrumentCount() const
{
  return d->instrumentCount;
}

TagLib::ushort IT::AudioProperties::sampleCount() const
{
  return d->sampleCount;
}

TagLib::ushort IT::AudioProperties::patternCount() const
{
  return d->patternCount;
}

TagLib::ushort IT::AudioProperties::version() const
{
  return d->version;
}

TagLib::ushort IT::AudioProperties::compatibleVersion() const
{
  return d->compatibleVersion;
}

TagLib::ushort IT::AudioProperties::flags() const
{
  return d->flags;
}

TagLib::ushort IT::AudioProperties::special() const
{
  return d->special;
}

uchar IT::AudioProperties::globalVolume() const
{
  return d->globalVolume;
}

uchar IT::AudioProperties::mixVolume() const
{
  return d->mixVolume;
}

uchar IT::AudioProperties::tempo() const
{
  return d->tempo;
}

uchar IT::AudioProperties::bpmSpeed() const
{
  return d->bpmSpeed;
}

uchar IT::AudioProperties::panningSeparation() const
{
  return d->panningSeparation;
}

uchar IT::AudioProperties::pitchWheelDepth() const
{
  return d->pitchWheelDepth;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void IT::AudioProperties::setChannels(int channels)
{
  d->channels = channels;
}

void IT::AudioProperties::setLengthInPatterns(ushort lengthInPatterns)
{
  d->lengthInPatterns = lengthInPatterns;
}

void IT::AudioProperties::setInstrumentCount(ushort instrumentCount)
{
  d->instrumentCount = instrumentCount;
}

void IT::AudioProperties::setSampleCount(ushort sampleCount)
{
  d->sampleCount = sampleCount;
}

void IT::AudioProperties::setPatternCount(ushort patternCount)
{
  d->patternCount = patternCount;
}

void IT::AudioProperties::setFlags(ushort flags)
{
  d->flags = flags;
}

void IT::AudioProperties::setSpecial(ushort special)
{
  d->special = special;
}

void IT::AudioProperties::setCompatibleVersion(ushort compatibleVersion)
{
  d->compatibleVersion = compatibleVersion;
}

void IT::AudioProperties::setVersion(ushort version)
{
  d->version = version;
}

void IT::AudioProperties::setGlobalVolume(uchar globalVolume)
{
  d->globalVolume = globalVolume;
}

void IT::AudioProperties::setMixVolume(uchar mixVolume)
{
  d->mixVolume = mixVolume;
}

void IT::AudioProperties::setTempo(uchar tempo)
{
  d->tempo = tempo;
}

void IT::AudioProperties::setBpmSpeed(uchar bpmSpeed)
{
  d->bpmSpeed = bpmSpeed;
}

void IT::AudioProperties::setPanningSeparation(uchar panningSeparation)
{
  d->panningSeparation = panningSeparation;
}

void IT::AudioProperties::setPitchWheelDepth(uchar pitchWheelDepth)
{
  d->pitchWheelDepth = pitchWheelDepth;
}
