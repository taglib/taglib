/***************************************************************************
 copyright            : (C) 2012 by Rupert Daniel
 email                : rupert@cancelmonday.com
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

#include "acidinfo.h"

#include <tstring.h>
#include <tbytevector.h>
#include <tdebug.h>

using namespace TagLib;

class RIFF::WAV::AcidInfo::AcidInfoPrivate
{
public:
  AcidInfoPrivate() :
    flags(0),
    rootNote(0),
    unknown1(0x8000u),
    unknown2(0.0),
    numberOfBeats(0),
    timeSignatureDenominator(4),
    timeSignatureNumerator(4),
    tempo(0.0)
  {

  }

  int flags;
  short rootNote;
  short unknown1;
  float unknown2;
  int numberOfBeats;
  short timeSignatureDenominator;
  short timeSignatureNumerator;
  float tempo;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::WAV::AcidInfo::AcidInfo()
{
  d = new AcidInfoPrivate();
}

RIFF::WAV::AcidInfo::AcidInfo(const ByteVector &data)
{
  d = new AcidInfoPrivate();
  read(data);
}

RIFF::WAV::AcidInfo::~AcidInfo()
{
  delete d;
}

int RIFF::WAV::AcidInfo::flags() const
{
  return d->flags;
}

void RIFF::WAV::AcidInfo::setFlags(int flags)
{
  d->flags = flags;
}

short RIFF::WAV::AcidInfo::rootNote() const
{
  return d->rootNote;
}

void RIFF::WAV::AcidInfo::setRootNote(short rootNote)
{
  d->rootNote = rootNote;
}

int RIFF::WAV::AcidInfo::numberOfBeats() const
{
  return d->numberOfBeats;
}

void RIFF::WAV::AcidInfo::setNumberOfBeats(int numberOfBeats)
{
  d->numberOfBeats = numberOfBeats;
}

short RIFF::WAV::AcidInfo::timeSignatureDenominator() const
{
  return d->timeSignatureDenominator;
}

void RIFF::WAV::AcidInfo::setTimeSignatureDenominator(short timeSignatureDenominator)
{
  d->timeSignatureDenominator = timeSignatureDenominator;
}

short RIFF::WAV::AcidInfo::timeSignatureNumerator() const
{
  return d->timeSignatureNumerator;
}

void RIFF::WAV::AcidInfo::setTimeSignatureNumerator(short timeSignatureNumerator)
{
  d->timeSignatureNumerator = timeSignatureNumerator;
}

float RIFF::WAV::AcidInfo::tempo() const
{
  return d->tempo;
}

void RIFF::WAV::AcidInfo::setTempo(float tempo)
{
  d->tempo = tempo;
}

bool RIFF::WAV::AcidInfo::isEmpty() const
{
  if(d->tempo >= 0.0f)
    return false;
  else
    return true;
}

ByteVector RIFF::WAV::AcidInfo::render() const
{
  ByteVector data;

  data.append(ByteVector::fromUInt(d->flags, false));

  if((d->flags & RootNote))
    data.append(ByteVector::fromShort(d->rootNote, false));
  else
    data.append(ByteVector::fromShort((d->rootNote + 12), false));

  data.append(ByteVector::fromShort(d->unknown1, false));
  data.append(ByteVector::fromFloat32LE(d->unknown2));
  data.append(ByteVector::fromUInt(d->numberOfBeats, false));
  data.append(ByteVector::fromShort(d->timeSignatureDenominator, false));
  data.append(ByteVector::fromShort(d->timeSignatureNumerator, false));
  data.append(ByteVector::fromFloat32LE(d->tempo));

  return data;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::WAV::AcidInfo::read(const ByteVector &data)
{
  d->flags = data.mid(0, 4).toUInt(false);

  short rootNote = data.mid(4, 2).toShort(false);
  d->rootNote = (d->flags & RootNote) ? rootNote : (rootNote - 12);

  d->unknown1 = data.mid(6, 2).toShort(false);
  d->unknown2 = data.toFloat32LE(8);
  d->numberOfBeats = data.mid(12, 4).toUInt(false);
  d->timeSignatureDenominator = data.mid(16, 2).toShort(false);
  d->timeSignatureNumerator = data.mid(18, 2).toShort(false);
  d->tempo = data.toFloat32LE(20);
}

