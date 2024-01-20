/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
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

#include "mpegproperties.h"

#include "tdebug.h"
#include "mpegfile.h"
#include "xingheader.h"
#include "apetag.h"

using namespace TagLib;

class MPEG::Properties::PropertiesPrivate
{
public:
  std::unique_ptr<XingHeader> xingHeader;
  int length { 0 };
  int bitrate { 0 };
  int sampleRate { 0 };
  int channels { 0 };
  int layer { 0 };
  Header::Version version { Header::Version1 };
  Header::ChannelMode channelMode { Header::Stereo };
  Header::ChannelConfiguration channelConfiguration { Header::Custom };
  bool protectionEnabled { false };
  bool isCopyrighted { false };
  bool isOriginal { false };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPEG::Properties::Properties(File *file, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(file, style);
}

MPEG::Properties::~Properties() = default;

int MPEG::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int MPEG::Properties::bitrate() const
{
  return d->bitrate;
}

int MPEG::Properties::sampleRate() const
{
  return d->sampleRate;
}

int MPEG::Properties::channels() const
{
  return d->channels;
}

const MPEG::XingHeader *MPEG::Properties::xingHeader() const
{
  return d->xingHeader.get();
}

MPEG::Header::Version MPEG::Properties::version() const
{
  return d->version;
}

int MPEG::Properties::layer() const
{
  return d->layer;
}

bool MPEG::Properties::protectionEnabled() const
{
  return d->protectionEnabled;
}

MPEG::Header::ChannelMode MPEG::Properties::channelMode() const
{
  return d->channelMode;
}

MPEG::Header::ChannelConfiguration MPEG::Properties::channelConfiguration() const
{
  return d->channelConfiguration;
}

bool MPEG::Properties::isADTS() const
{
  return d->layer == 0 &&
    (d->version == Header::Version2 || d->version == Header::Version4);
}

bool MPEG::Properties::isCopyrighted() const
{
  return d->isCopyrighted;
}

bool MPEG::Properties::isOriginal() const
{
  return d->isOriginal;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MPEG::Properties::read(File *file, ReadStyle readStyle)
{
  // Only the first valid frame is required if we have a VBR header.

  const offset_t firstFrameOffset = file->firstFrameOffset();
  if(firstFrameOffset < 0) {
    debug("MPEG::Properties::read() -- Could not find an MPEG frame in the stream.");
    return;
  }

  const Header firstHeader(file, firstFrameOffset, false);

  // Check for a VBR header that will help us in gathering information about a
  // VBR stream.

  file->seek(firstFrameOffset);
  d->xingHeader = std::make_unique<XingHeader>(file->readBlock(firstHeader.frameLength()));
  if(!d->xingHeader->isValid()) {
    d->xingHeader = nullptr;
  }

  if(d->xingHeader && firstHeader.samplesPerFrame() > 0 && firstHeader.sampleRate() > 0) {

    // Read the length and the bitrate from the VBR header.

    const double timePerFrame = firstHeader.samplesPerFrame() * 1000.0 / firstHeader.sampleRate();
    const double length = timePerFrame * d->xingHeader->totalFrames();

    d->length  = static_cast<int>(length + 0.5);
    d->bitrate = static_cast<int>(d->xingHeader->totalSize() * 8.0 / length + 0.5);
  }
  else {
    int bitRate = firstHeader.bitrate();
    if(firstHeader.isADTS()) {
      // ADTS is probably VBR, so to get the real length, we would have to go
      // through all frames, count the frames in numFrames and sum their
      // header.frameLength() to totalFrameSize, and finally calculate
      // d->length = 1000LL * numFrames * firstHeader.samplesPerFrame() / firstHeader.sampleRate();
      // d->bitrate = d->length > 0 ? totalFrameSize * 8 / d->length : 0;
      //
      // With Fast read style, we do not try to estimate the length and just set
      // it and the bitrate to zero.
      // With Average read style, in order to come faster to an estimate which
      // is accurate enough, we stop when the average bytes/frame rate is stable
      // for 10 frames and then calculate the length from the estimated bitrate
      // and the stream length.
      if(readStyle == Fast) {
        bitRate = 0;
        d->length = 0;
      }
      else {
        Header header(firstHeader);
        unsigned long long totalFrameSize = header.frameLength();
        unsigned long long bytesPerFrame = 0;
        unsigned long long lastBytesPerFrame = 0;
        offset_t offset = firstFrameOffset;
        offset_t nextOffset;
        int numFrames = 1;
        int sameBytesPerFrameCount = 0;
        while((nextOffset = file->nextFrameOffset(offset + header.frameLength())) > offset) {
          offset = nextOffset;
          header = Header(file, offset, false);
          totalFrameSize += header.frameLength();
          ++numFrames;
          bytesPerFrame = totalFrameSize / numFrames;
          if(readStyle != Accurate) {
            if(bytesPerFrame == lastBytesPerFrame) {
              if(++sameBytesPerFrameCount >= 10) {
                break;
              }
            }
            else {
              sameBytesPerFrameCount = 0;
            }
            lastBytesPerFrame = bytesPerFrame;
          }
        }
        bitRate = firstHeader.samplesPerFrame() != 0
          ? static_cast<int>(bytesPerFrame * 8 * firstHeader.sampleRate()
                             / 1000 / firstHeader.samplesPerFrame())
          : 0;
      }
    }
    else if(firstHeader.bitrate() > 0) {
      // Since there was no valid VBR header found, we hope that we're in a constant
      // bitrate file.

      // TODO: Make this more robust with audio property detection for VBR without a
      // Xing header.
      bitRate = firstHeader.bitrate();
    }
    if(bitRate > 0) {
      d->bitrate = bitRate;

      // Look for the last MPEG audio frame to calculate the stream length.

      if(const offset_t lastFrameOffset = file->lastFrameOffset();
         lastFrameOffset < 0) {
        debug("MPEG::Properties::read() -- Could not find an MPEG frame in the stream.");
      }
      else
      {
        const Header lastHeader(file, lastFrameOffset, false);
        if(const offset_t streamLength = lastFrameOffset - firstFrameOffset + lastHeader.frameLength();
           streamLength > 0)
          d->length = static_cast<int>(streamLength * 8.0 / d->bitrate + 0.5);
      }
    }
  }

  d->sampleRate        = firstHeader.sampleRate();
  d->channelConfiguration = firstHeader.channelConfiguration();
  switch(d->channelConfiguration) {
  case Header::FrontCenter:
    d->channels = 1;
    break;
  case Header::FrontLeftRight:
    d->channels = 2;
    break;
  case Header::FrontCenterLeftRight:
    d->channels = 3;
    break;
  case Header::FrontCenterLeftRightBackCenter:
    d->channels = 4;
    break;
  case Header::FrontCenterLeftRightBackLeftRight:
    d->channels = 5;
    break;
  case Header::FrontCenterLeftRightBackLeftRightLFE:
    d->channels = 6;
    break;
  case Header::FrontCenterLeftRightSideLeftRightBackLeftRightLFE:
    d->channels = 8;
    break;
  case Header::Custom:
  default:
    d->channels = firstHeader.channelMode() == Header::SingleChannel ? 1 : 2;
  }
  d->version           = firstHeader.version();
  d->layer             = firstHeader.layer();
  d->protectionEnabled = firstHeader.protectionEnabled();
  d->channelMode       = firstHeader.channelMode();
  d->isCopyrighted     = firstHeader.isCopyrighted();
  d->isOriginal        = firstHeader.isOriginal();
}
