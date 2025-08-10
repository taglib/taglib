/***************************************************************************
    copyright            : (C) 2025 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

#include "matroskaproperties.h"

using namespace TagLib;

class Matroska::Properties::PropertiesPrivate
{
public:
  int length { 0 };
  int bitrate { 0 };
  int sampleRate { 0 };
  int bitsPerSample { 0 };
  int channels { 0 };
};

Matroska::Properties::Properties(File *file, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(file);
}

Matroska::Properties::~Properties() = default;

int Matroska::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int Matroska::Properties::bitrate() const
{
  return d->bitrate;
}

int Matroska::Properties::sampleRate() const
{
  return d->sampleRate;
}

int Matroska::Properties::channels() const
{
  return d->channels;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void Matroska::Properties::read(File *file)
{
  // TODO implement.
}
