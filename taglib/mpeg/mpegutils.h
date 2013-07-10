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

#ifndef TAGLIB_MPEGUTILS_H
#define TAGLIB_MPEGUTILS_H

// THIS FILE IS NOT A PART OF THE TAGLIB API

#ifndef DO_NOT_DOCUMENT  // tell Doxygen not to document this header

#include <tbytevector.h>
#include "mpegheader.h"
#include "xingheader.h"

namespace TagLib
{
  using MPEG::Header;
  using MPEG::XingHeader;

  ////////////////////////////////////////////////////////////////////////////////
  // Private common functions for MPEG related classes.
  ////////////////////////////////////////////////////////////////////////////////
  
  inline bool isSecondSynchByte(char byte)
  {
    return ((byte & '\xe0') == '\xe0');
  }

  inline offset_t nextMpegFrameOffset(File *file, offset_t position, size_t bufferSize)
  {
    // Seek forward the bit pattern 11111111 111????? 

    while(true)
    {
      file->seek(position);

      const ByteVector buffer = file->readBlock(bufferSize);
      if(buffer.size() <= 1)
        return -1;

      const char *begin = buffer.data();
      const char *end   = buffer.data() + buffer.size() - 1;
      for(const char *p = begin; p < end; ++p) {
        if(p[0] == '\xff' && isSecondSynchByte(p[1]))
          return position + (p - begin);
      }

      position += buffer.size() - 1;
    }
  }

  inline offset_t previousMpegFrameOffset(File *file, offset_t position, size_t bufferSize)
  {
    // Seek backward the bit pattern 11111111 111????? 

    while(true)
    {
      offset_t newPosition = std::max<offset_t>(0, position - bufferSize);

      file->seek(newPosition);
      const ByteVector buffer 
        = file->readBlock(static_cast<size_t>(position - newPosition));

      const char *begin = buffer.data() + buffer.size() - 2;
      const char *end   = buffer.data();
      for(const char *p = begin; p >= end; --p) {
        if(p[0] == '\xff' && isSecondSynchByte(p[1]))
          return newPosition + (p - end);
      }

      if(newPosition == 0)
        return -1;

      position = newPosition + 1;
    }
  }

  template <typename TFile, typename TPropertiesPrivate>
  inline void readMpegAudioProperties(TFile *file, TPropertiesPrivate *d)
  {
    // Since we've likely just looked for the ID3v1 tag, start at the end of the
    // file where we're least likely to have to have to move the disk head.

    offset_t last = file->lastFrameOffset();

    if(last < 0) {
      debug("readMpegAudioProperties() -- Could not find a valid last MPEG frame in the stream.");
      return;
    }

    file->seek(last);
    Header lastHeader(file->readBlock(4));

    offset_t first = file->firstFrameOffset();

    if(first < 0) {
      debug("readMpegAudioProperties() -- Could not find a valid first MPEG frame in the stream.");
      return;
    }

    if(!lastHeader.isValid()) {

      offset_t pos = last;

      while(pos > first) {

        pos = file->previousFrameOffset(pos);

        if(pos < 0)
          break;

        file->seek(pos);
        Header header(file->readBlock(4));

        if(header.isValid()) {
          lastHeader = header;
          last = pos;
          break;
        }
      }
    }

    // Now jump back to the front of the file and read what we need from there.

    file->seek(first);
    Header firstHeader(file->readBlock(4));

    if(!firstHeader.isValid() || !lastHeader.isValid()) {
      debug("readMpegAudioProperties() -- Page headers were invalid.");
      return;
    }

    // Check for a Xing header that will help us in gathering information about a
    // VBR stream.

    int xingHeaderOffset = XingHeader::offset(firstHeader.version(), firstHeader.channelMode());

    file->seek(first + xingHeaderOffset);
    d->xingHeader.reset(new XingHeader(file->readBlock(16)));

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
    d->channels = firstHeader.channelMode() == Header::SingleChannel ? 1 : 2;
    d->version = firstHeader.version();
    d->layer = firstHeader.layer();
    d->protectionEnabled = firstHeader.protectionEnabled();
    d->channelMode = firstHeader.channelMode();
    d->isCopyrighted = firstHeader.isCopyrighted();
    d->isOriginal = firstHeader.isOriginal();
  }
}


#endif

#endif
