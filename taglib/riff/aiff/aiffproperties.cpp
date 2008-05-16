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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <tstring.h>
#include <tdebug.h>
#include <bitset>

#include "aiffproperties.h"

using namespace TagLib;

class RIFF::AIFF::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0)
  {

  }

  int length;
  int bitrate;
  int sampleRate;
  int channels;
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


////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::AIFF::Properties::read(const ByteVector &data)
{

}
