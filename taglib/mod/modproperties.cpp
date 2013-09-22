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

#include "modproperties.h"

using namespace TagLib;
using namespace Mod;

class Mod::AudioProperties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    channels(0),
    instrumentCount(0),
    lengthInPatterns(0) {}

  int   channels;
  uint  instrumentCount;
  uchar lengthInPatterns;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Mod::AudioProperties::AudioProperties(AudioProperties::ReadStyle propertiesStyle) :
  d(new PropertiesPrivate)
{
}

Mod::AudioProperties::~AudioProperties()
{
  delete d;
}

int Mod::AudioProperties::length() const
{
  return 0;
}

int Mod::AudioProperties::bitrate() const
{
  return 0;
}

int Mod::AudioProperties::sampleRate() const
{
  return 0;
}

int Mod::AudioProperties::channels() const
{
  return d->channels;
}

TagLib::uint Mod::AudioProperties::instrumentCount() const
{
  return d->instrumentCount;
}

uchar Mod::AudioProperties::lengthInPatterns() const
{
  return d->lengthInPatterns;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void Mod::AudioProperties::setChannels(int channels)
{
  d->channels = channels;
}

void Mod::AudioProperties::setInstrumentCount(uint instrumentCount)
{
  d->instrumentCount = instrumentCount;
}

void Mod::AudioProperties::setLengthInPatterns(uchar lengthInPatterns)
{
  d->lengthInPatterns = lengthInPatterns;
}
