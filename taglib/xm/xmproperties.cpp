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

#include "xmproperties.h"

using namespace TagLib;
using namespace XM;

class XM::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    lengthInPatterns(0),
    channels(0),
    version(0),
    restartPosition(0),
    patternCount(0),
    instrumentCount(0),
    sampleCount(0),
    flags(0),
    tempo(0),
    bpmSpeed(0)
  {
  }
  
  ushort lengthInPatterns;
  int    channels;
  ushort version;
  ushort restartPosition;
  ushort patternCount;
  ushort instrumentCount;
  uint   sampleCount;
  ushort flags;
  ushort tempo;
  ushort bpmSpeed;
};

XM::Properties::Properties(AudioProperties::ReadStyle propertiesStyle) :
  AudioProperties(propertiesStyle),
  d(new PropertiesPrivate)
{
}     

XM::Properties::~Properties()
{
  delete d;
}

int XM::Properties::length() const
{
  return 0;
}

int XM::Properties::bitrate() const
{
  return 0;
}

int XM::Properties::sampleRate() const
{
  return 0;
}

int XM::Properties::channels() const
{
  return d->channels;
}

ushort XM::Properties::lengthInPatterns() const
{
  return d->lengthInPatterns;
}

ushort XM::Properties::version() const
{
  return d->version;
}

ushort XM::Properties::restartPosition() const
{
  return d->restartPosition;
}

ushort XM::Properties::patternCount() const
{
  return d->patternCount;
}

ushort XM::Properties::instrumentCount() const
{
  return d->instrumentCount;
}

uint XM::Properties::sampleCount() const
{
  return d->sampleCount;
}

ushort XM::Properties::flags() const
{
  return d->flags;
}

ushort XM::Properties::tempo() const
{
  return d->tempo;
}

ushort XM::Properties::bpmSpeed() const
{
  return d->bpmSpeed;
}

void XM::Properties::setLengthInPatterns(ushort lengthInPatterns)
{
  d->lengthInPatterns = lengthInPatterns;
}

void XM::Properties::setChannels(int channels)
{
  d->channels = channels;
}

void XM::Properties::setVersion(ushort version)
{
  d->version = version;
}

void XM::Properties::setRestartPosition(ushort restartPosition)
{
  d->restartPosition = restartPosition;
}

void XM::Properties::setPatternCount(ushort patternCount)
{
  d->patternCount = patternCount;
}

void XM::Properties::setInstrumentCount(ushort instrumentCount)
{
  d->instrumentCount = instrumentCount;
}

void XM::Properties::setSampleCount(uint sampleCount)
{
  d->sampleCount = sampleCount;
}

void XM::Properties::setFlags(ushort flags)
{
  d->flags = flags;
}

void XM::Properties::setTempo(ushort tempo)
{
  d->tempo = tempo;
}

void XM::Properties::setBpmSpeed(ushort bpmSpeed)
{
  d->bpmSpeed = bpmSpeed;
}
