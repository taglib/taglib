/***************************************************************************
    copyright            : (C) 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#include <tstring.h>
#include <tdebug.h>
#include "aiffproperties.h"

using namespace TagLib;

class RIFF::AIFF::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    sampleWidth(0),
    sampleFrames(0) {}

  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int sampleWidth;

  ByteVector compressionType;
  String compressionName;

  uint sampleFrames;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::AIFF::Properties::Properties(const ByteVector &data, ReadStyle style) : AudioProperties(style)
{
  d = new PropertiesPrivate;
  read(data);
}

RIFF::AIFF::Properties::~Properties()
{
  delete d;
}

int RIFF::AIFF::Properties::length() const
{
  return d->length;
}

int RIFF::AIFF::Properties::bitrate() const
{
  return d->bitrate;
}

int RIFF::AIFF::Properties::sampleRate() const
{
  return d->sampleRate;
}

int RIFF::AIFF::Properties::channels() const
{
  return d->channels;
}

int RIFF::AIFF::Properties::sampleWidth() const
{
  return d->sampleWidth;
}

TagLib::uint RIFF::AIFF::Properties::sampleFrames() const
{
  return d->sampleFrames;
}

bool RIFF::AIFF::Properties::isAiffC() const
{
  return (!d->compressionType.isEmpty());
}

ByteVector RIFF::AIFF::Properties::compressionType() const
{
  return d->compressionType;
}

String RIFF::AIFF::Properties::compressionName() const
{
  return d->compressionName;
}
////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::AIFF::Properties::read(const ByteVector &data)
{
  if(data.size() < 18) {
    debug("RIFF::AIFF::Properties::read() - \"COMM\" chunk is too short for AIFF.");
    return;
  }

  d->channels       = data.toShort(0U);
  d->sampleFrames   = data.toUInt(2U);
  d->sampleWidth    = data.toShort(6U);
  const long double sampleRate = data.toFloat80BE(8);
  d->sampleRate     = (int)sampleRate;
  d->bitrate        = (int)((sampleRate * d->sampleWidth * d->channels) / 1000.0);
  d->length         = d->sampleRate > 0 ? d->sampleFrames / d->sampleRate : 0;

  if(data.size() >= 23) {
    d->compressionType = data.mid(18, 4);
    d->compressionName = String(data.mid(23, static_cast<uchar>(data[22])));
  }
}
