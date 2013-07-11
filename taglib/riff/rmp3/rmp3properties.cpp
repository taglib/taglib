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

const MPEG::XingHeader *RIFF::RMP3::AudioProperties::xingHeader() const
{
  return d->xingHeader.get();
}

MPEG::Header::Version RIFF::RMP3::AudioProperties::version() const
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

MPEG::Header::ChannelMode RIFF::RMP3::AudioProperties::channelMode() const
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
  // Since we've likely just looked for the ID3v1 tag, start at the end of the
  // file where we're least likely to have to have to move the disk head.

  offset_t last = file->lastFrameOffset();

  if(last < 0) {
    debug("RIFF::RMP3::AudioProperties::read() -- "
      "Could not find a valid last MPEG frame in the stream.");
    return;
  }

  file->seek(last);
  MPEG::Header lastHeader(file->readBlock(4));

  offset_t first = file->firstFrameOffset();

  if(first < 0) {
    debug("RIFF::RMP3::AudioProperties::read() -- "
      "Could not find a valid first MPEG frame in the stream.");
    return;
  }

  if(!lastHeader.isValid()) {

    offset_t pos = last;

    while(pos > first) {

      pos = file->previousFrameOffset(pos);

      if(pos < 0)
        break;

      file->seek(pos);
      MPEG::Header header(file->readBlock(4));

      if(header.isValid()) {
        lastHeader = header;
        last = pos;
        break;
      }
    }
  }

  // Now jump back to the front of the file and read what we need from there.

  file->seek(first);
  MPEG::Header firstHeader(file->readBlock(4));

  if(!firstHeader.isValid() || !lastHeader.isValid()) {
    debug("readMpegAudioProperties() -- Page headers were invalid.");
    return;
  }

  // Check for a Xing header that will help us in gathering information about a
  // VBR stream.

  int xingHeaderOffset = MPEG::XingHeader::offset(firstHeader.version(), firstHeader.channelMode());

  file->seek(first + xingHeaderOffset);
  d->xingHeader.reset(new MPEG::XingHeader(file->readBlock(16)));

  // Read the length and the bitrate from the Xing header.

  if(d->xingHeader->isValid() &&
    firstHeader.sampleRate() > 0 &&
    d->xingHeader->totalFrames() > 0)
  {
    double timePerFrame =
      double(firstHeader.samplesPerFrame()) / firstHeader.sampleRate();

    double length = timePerFrame * d->xingHeader->totalFrames();

    d->length = int(length);
    d->bitrate = d->length > 0 ? (int)(d->xingHeader->totalSize() * 8 / length / 1000) : 0;
  }
  else {
    // Since there was no valid Xing header found, we hope that we're in a constant
    // bitrate file.

    d->xingHeader.reset();

    // TODO: Make this more robust with audio property detection for VBR without a
    // Xing header.

    if(firstHeader.frameLength() > 0 && firstHeader.bitrate() > 0) {
      int frames = static_cast<int>((last - first) / firstHeader.frameLength() + 1);

      d->length = int(float(firstHeader.frameLength() * frames) /
        float(firstHeader.bitrate() * 125) + 0.5);
      d->bitrate = firstHeader.bitrate();
    }
  }

  d->sampleRate = firstHeader.sampleRate();
  d->channels = firstHeader.channelMode() == MPEG::Header::SingleChannel ? 1 : 2;
  d->version = firstHeader.version();
  d->layer = firstHeader.layer();
  d->protectionEnabled = firstHeader.protectionEnabled();
  d->channelMode = firstHeader.channelMode();
  d->isCopyrighted = firstHeader.isCopyrighted();
  d->isOriginal = firstHeader.isOriginal();
}
