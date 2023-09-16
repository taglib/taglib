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

#include "xmproperties.h"

using namespace TagLib;
using namespace XM;

class XM::Properties::PropertiesPrivate
{
public:
  unsigned short lengthInPatterns { 0 };
  int channels { 0 };
  unsigned short version { 0 };
  unsigned short restartPosition { 0 };
  unsigned short patternCount { 0 };
  unsigned short instrumentCount { 0 };
  unsigned int sampleCount { 0 };
  unsigned short flags { 0 };
  unsigned short tempo { 0 };
  unsigned short bpmSpeed { 0 };
};

XM::Properties::Properties(AudioProperties::ReadStyle propertiesStyle) :
  AudioProperties(propertiesStyle),
  d(std::make_unique<PropertiesPrivate>())
{
}

XM::Properties::~Properties() = default;

int XM::Properties::channels() const
{
  return d->channels;
}

unsigned short XM::Properties::lengthInPatterns() const
{
  return d->lengthInPatterns;
}

unsigned short XM::Properties::version() const
{
  return d->version;
}

unsigned short XM::Properties::restartPosition() const
{
  return d->restartPosition;
}

unsigned short XM::Properties::patternCount() const
{
  return d->patternCount;
}

unsigned short XM::Properties::instrumentCount() const
{
  return d->instrumentCount;
}

unsigned int XM::Properties::sampleCount() const
{
  return d->sampleCount;
}

unsigned short XM::Properties::flags() const
{
  return d->flags;
}

unsigned short XM::Properties::tempo() const
{
  return d->tempo;
}

unsigned short XM::Properties::bpmSpeed() const
{
  return d->bpmSpeed;
}

void XM::Properties::setLengthInPatterns(unsigned short lengthInPatterns)
{
  d->lengthInPatterns = lengthInPatterns;
}

void XM::Properties::setChannels(int channels)
{
  d->channels = channels;
}

void XM::Properties::setVersion(unsigned short version)
{
  d->version = version;
}

void XM::Properties::setRestartPosition(unsigned short restartPosition)
{
  d->restartPosition = restartPosition;
}

void XM::Properties::setPatternCount(unsigned short patternCount)
{
  d->patternCount = patternCount;
}

void XM::Properties::setInstrumentCount(unsigned short instrumentCount)
{
  d->instrumentCount = instrumentCount;
}

void XM::Properties::setSampleCount(unsigned int sampleCount)
{
  d->sampleCount = sampleCount;
}

void XM::Properties::setFlags(unsigned short flags)
{
  d->flags = flags;
}

void XM::Properties::setTempo(unsigned short tempo)
{
  d->tempo = tempo;
}

void XM::Properties::setBpmSpeed(unsigned short bpmSpeed)
{
  d->bpmSpeed = bpmSpeed;
}
