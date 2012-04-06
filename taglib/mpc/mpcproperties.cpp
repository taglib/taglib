/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
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
#include <bitset>

#include "mpcproperties.h"
#include "mpcfile.h"

using namespace TagLib;

class MPC::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate(const ByteVector &d, long length, ReadStyle s) :
    data(d),
    streamLength(length),
    style(s),
    version(0),
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    totalFrames(0),
    sampleFrames(0) {}

  ByteVector data;
  long streamLength;
  ReadStyle style;
  int version;
  int length;
  int bitrate;
  int sampleRate;
  int channels;
  uint totalFrames;
  uint sampleFrames;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPC::Properties::Properties(const ByteVector &data, long streamLength, ReadStyle style) : AudioProperties(style)
{
  d = new PropertiesPrivate(data, streamLength, style);
  read();
}

MPC::Properties::~Properties()
{
  delete d;
}

int MPC::Properties::length() const
{
  return d->length;
}

int MPC::Properties::bitrate() const
{
  return d->bitrate;
}

int MPC::Properties::sampleRate() const
{
  return d->sampleRate;
}

int MPC::Properties::channels() const
{
  return d->channels;
}

int MPC::Properties::mpcVersion() const
{
  return d->version;
}

uint MPC::Properties::totalFrames() const
{
  return d->totalFrames;
}

uint MPC::Properties::sampleFrames() const
{
  return d->sampleFrames;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

static const unsigned short sftable [4] = { 44100, 48000, 37800, 32000 };

void MPC::Properties::read()
{
  if(!d->data.startsWith("MP+"))
    return;

  d->version = d->data[3] & 15;

  if(d->version >= 7) {
    d->totalFrames = d->data.mid(4, 4).toUInt(false);

    std::bitset<32> flags(TAGLIB_CONSTRUCT_BITSET(d->data.mid(8, 4).toUInt(false)));
    d->sampleRate = sftable[flags[17] * 2 + flags[16]];
    d->channels = 2;

    uint gapless = d->data.mid(5, 4).toUInt(false);
    bool trueGapless = (gapless >> 31) & 0x0001;
    if(trueGapless) {
      uint lastFrameSamples = (gapless >> 20) & 0x07FF;
      d->sampleFrames = d->totalFrames * 1152 - lastFrameSamples;
    }
    else
      d->sampleFrames = d->totalFrames * 1152 - 576;
  }
  else {
    uint headerData = d->data.mid(0, 4).toUInt(false);

    d->bitrate = (headerData >> 23) & 0x01ff;
    d->version = (headerData >> 11) & 0x03ff;
    d->sampleRate = 44100;
    d->channels = 2;

    if(d->version >= 5)
      d->totalFrames = d->data.mid(4, 4).toUInt(false);
    else
      d->totalFrames = d->data.mid(6, 2).toUInt(false);

    d->sampleFrames = d->totalFrames * 1152 - 576;
  }

  d->length = d->sampleRate > 0 ? (d->sampleFrames + (d->sampleRate / 2)) / d->sampleRate : 0;

  if(!d->bitrate)
    d->bitrate = d->length > 0 ? ((d->streamLength * 8L) / d->length) / 1000 : 0;
}
