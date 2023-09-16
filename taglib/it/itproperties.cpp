/***************************************************************************
    copyright           :(C) 2011 by Mathias Panzenböck
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

#include "itproperties.h"

using namespace TagLib;
using namespace IT;

class IT::Properties::PropertiesPrivate
{
public:
  int channels { 0 };
  unsigned short lengthInPatterns { 0 };
  unsigned short instrumentCount { 0 };
  unsigned short sampleCount { 0 };
  unsigned short patternCount { 0 };
  unsigned short version { 0 };
  unsigned short compatibleVersion { 0 };
  unsigned short flags { 0 };
  unsigned short special { 0 };
  unsigned char globalVolume { 0 };
  unsigned char mixVolume { 0 };
  unsigned char tempo { 0 };
  unsigned char bpmSpeed { 0 };
  unsigned char panningSeparation { 0 };
  unsigned char pitchWheelDepth { 0 };
};

IT::Properties::Properties(AudioProperties::ReadStyle propertiesStyle) :
  AudioProperties(propertiesStyle),
  d(std::make_unique<PropertiesPrivate>())
{
}

IT::Properties::~Properties() = default;

int IT::Properties::channels() const
{
  return d->channels;
}

unsigned short IT::Properties::lengthInPatterns() const
{
  return d->lengthInPatterns;
}

bool IT::Properties::stereo() const
{
  return d->flags & Stereo;
}

unsigned short IT::Properties::instrumentCount() const
{
  return d->instrumentCount;
}

unsigned short IT::Properties::sampleCount() const
{
  return d->sampleCount;
}

unsigned short IT::Properties::patternCount() const
{
  return d->patternCount;
}

unsigned short IT::Properties::version() const
{
  return d->version;
}

unsigned short IT::Properties::compatibleVersion() const
{
  return d->compatibleVersion;
}

unsigned short IT::Properties::flags() const
{
  return d->flags;
}

unsigned short IT::Properties::special() const
{
  return d->special;
}

unsigned char IT::Properties::globalVolume() const
{
  return d->globalVolume;
}

unsigned char IT::Properties::mixVolume() const
{
  return d->mixVolume;
}

unsigned char IT::Properties::tempo() const
{
  return d->tempo;
}

unsigned char IT::Properties::bpmSpeed() const
{
  return d->bpmSpeed;
}

unsigned char IT::Properties::panningSeparation() const
{
  return d->panningSeparation;
}

unsigned char IT::Properties::pitchWheelDepth() const
{
  return d->pitchWheelDepth;
}

void IT::Properties::setChannels(int channels)
{
  d->channels = channels;
}

void IT::Properties::setLengthInPatterns(unsigned short lengthInPatterns)
{
  d->lengthInPatterns = lengthInPatterns;
}

void IT::Properties::setInstrumentCount(unsigned short instrumentCount)
{
  d->instrumentCount = instrumentCount;
}

void IT::Properties::setSampleCount(unsigned short sampleCount)
{
  d->sampleCount = sampleCount;
}

void IT::Properties::setPatternCount(unsigned short patternCount)
{
  d->patternCount = patternCount;
}

void IT::Properties::setFlags(unsigned short flags)
{
  d->flags = flags;
}

void IT::Properties::setSpecial(unsigned short special)
{
  d->special = special;
}

void IT::Properties::setCompatibleVersion(unsigned short compatibleVersion)
{
  d->compatibleVersion = compatibleVersion;
}

void IT::Properties::setVersion(unsigned short version)
{
  d->version = version;
}

void IT::Properties::setGlobalVolume(unsigned char globalVolume)
{
  d->globalVolume = globalVolume;
}

void IT::Properties::setMixVolume(unsigned char mixVolume)
{
  d->mixVolume = mixVolume;
}

void IT::Properties::setTempo(unsigned char tempo)
{
  d->tempo = tempo;
}

void IT::Properties::setBpmSpeed(unsigned char bpmSpeed)
{
  d->bpmSpeed = bpmSpeed;
}

void IT::Properties::setPanningSeparation(unsigned char panningSeparation)
{
  d->panningSeparation = panningSeparation;
}

void IT::Properties::setPitchWheelDepth(unsigned char pitchWheelDepth)
{
  d->pitchWheelDepth = pitchWheelDepth;
}
