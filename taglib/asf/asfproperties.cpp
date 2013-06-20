/**************************************************************************
    copyright            : (C) 2005-2007 by Lukáš Lalinský
    email                : lalinsky@gmail.com
 **************************************************************************/

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

#include <tdebug.h>
#include <tstring.h>
#include "asfproperties.h"

using namespace TagLib;

class ASF::AudioProperties::PropertiesPrivate
{
public:
  PropertiesPrivate(): length(0), bitrate(0), sampleRate(0), channels(0), encrypted(false) {}
  int length;
  int bitrate;
  int sampleRate;
  int channels;
  bool encrypted;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ASF::AudioProperties::AudioProperties() 
  : TagLib::AudioProperties(AudioProperties::Average)
{
  d = new PropertiesPrivate;
}

ASF::AudioProperties::~AudioProperties()
{
  if(d)
    delete d;
}

int ASF::AudioProperties::length() const
{
  return d->length;
}

int ASF::AudioProperties::bitrate() const
{
  return d->bitrate;
}

int ASF::AudioProperties::sampleRate() const
{
  return d->sampleRate;
}

int ASF::AudioProperties::channels() const
{
  return d->channels;
}

bool ASF::AudioProperties::isEncrypted() const
{
  return d->encrypted;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void ASF::AudioProperties::setLength(int length)
{
  d->length = length;
}

void ASF::AudioProperties::setBitrate(int length)
{
  d->bitrate = length;
}

void ASF::AudioProperties::setSampleRate(int length)
{
  d->sampleRate = length;
}

void ASF::AudioProperties::setChannels(int length)
{
  d->channels = length;
}

void ASF::AudioProperties::setEncrypted(bool encrypted)
{
  d->encrypted = encrypted;
}

