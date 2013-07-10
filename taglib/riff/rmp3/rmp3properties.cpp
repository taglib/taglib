/***************************************************************************
    copyright            : (C) 2013 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
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

#include <tdebug.h>
#include <tstring.h>
#include <tsmartptr.h>

#include "rmp3properties.h"
#include "rmp3file.h"
#include "mpegutils.h"

using namespace TagLib;

class RIFF::RMP3::AudioProperties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    layer(0),
    version(MPEG::Header::Version1),
    channelMode(MPEG::Header::Stereo),
    protectionEnabled(false),
    isCopyrighted(false),
    isOriginal(false) {}

  SCOPED_PTR<MPEG::XingHeader> xingHeader;
  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int layer;
  MPEG::Header::Version version;
  MPEG::Header::ChannelMode channelMode;
  bool protectionEnabled;
  bool isCopyrighted;
  bool isOriginal;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::RMP3::AudioProperties::AudioProperties(File *file, ReadStyle style) : 
  TagLib::AudioProperties(style),
  d(new PropertiesPrivate())
{
  read(file);
}

RIFF::RMP3::AudioProperties::~AudioProperties()
{
  delete d;
}

int RIFF::RMP3::AudioProperties::length() const
{
  return d->length;
}

int RIFF::RMP3::AudioProperties::bitrate() const
{
  return d->bitrate;
}

int RIFF::RMP3::AudioProperties::sampleRate() const
{
  return d->sampleRate;
}

int RIFF::RMP3::AudioProperties::channels() const
{
  return d->channels;
}

const XingHeader *RIFF::RMP3::AudioProperties::xingHeader() const
{
  return d->xingHeader.get();
}

Header::Version RIFF::RMP3::AudioProperties::version() const
{
  return d->version;
}

int RIFF::RMP3::AudioProperties::layer() const
{
  return d->layer;
}

bool RIFF::RMP3::AudioProperties::protectionEnabled() const
{
  return d->protectionEnabled;
}

Header::ChannelMode RIFF::RMP3::AudioProperties::channelMode() const
{
  return d->channelMode;
}

bool RIFF::RMP3::AudioProperties::isCopyrighted() const
{
  return d->isCopyrighted;
}

bool RIFF::RMP3::AudioProperties::isOriginal() const
{
  return d->isOriginal;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::RMP3::AudioProperties::read(File *file)
{
  readMpegAudioProperties(file, d);
}
