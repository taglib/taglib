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

#include "matroskafile.h"

using namespace TagLib;

class Matroska::Properties::PropertiesPrivate
{
public:
  explicit PropertiesPrivate(File *file) : file(file) {}
  ~PropertiesPrivate() = default;

  PropertiesPrivate(const PropertiesPrivate &) = delete;
  PropertiesPrivate &operator=(const PropertiesPrivate &) = delete;

  File *file;
  String codecName;
  String title;
  int length { 0 };
  int bitrate { -1 };
  int sampleRate { 0 };
  int channels { 0 };
  int bitsPerSample { 0 };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Matroska::Properties::Properties(File *file, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>(file))
{
}

Matroska::Properties::~Properties() = default;

int Matroska::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int Matroska::Properties::bitrate() const
{
  if (d->bitrate == -1) {
    d->bitrate = d->length != 0 ? static_cast<int>(d->file->length() * 8 / d->length) : 0;
  }
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

int Matroska::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

String Matroska::Properties::codecName() const
{
  return d->codecName;
}

String Matroska::Properties::title() const
{
  return d->title;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void Matroska::Properties::setLengthInMilliseconds(int length)
{
  d->length = length;
}

void Matroska::Properties::setBitrate(int bitrate)
{
  d->bitrate = bitrate;
}

void Matroska::Properties::setSampleRate(int sampleRate)
{
  d->sampleRate = sampleRate;
}

void Matroska::Properties::setChannels(int channels)
{
  d->channels = channels;
}

void Matroska::Properties::setBitsPerSample(int bitsPerSample)
{
  d->bitsPerSample = bitsPerSample;
}

void Matroska::Properties::setCodecName(const String &codecName)
{
  d->codecName = codecName;
}

void Matroska::Properties::setTitle(const String& title)
{
  d->title = title;
}
