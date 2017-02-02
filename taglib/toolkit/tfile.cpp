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

#include "tfile.h"
#include "tfilestream.h"
#include "tstring.h"
#include "tdebug.h"
#include "tpropertymap.h"

#ifdef _WIN32
# include <windows.h>
# include <io.h>
#else
# include <stdio.h>
# include <unistd.h>
#endif

#ifndef R_OK
# define R_OK 4
#endif
#ifndef W_OK
# define W_OK 2
#endif

#include "asffile.h"
#include "mpegfile.h"
#include "vorbisfile.h"
#include "flacfile.h"
#include "oggflacfile.h"
#include "mpcfile.h"
#include "mp4file.h"
#include "wavpackfile.h"
#include "speexfile.h"
#include "opusfile.h"
#include "trueaudiofile.h"
#include "aifffile.h"
#include "wavfile.h"
#include "apefile.h"
#include "modfile.h"
#include "s3mfile.h"
#include "itfile.h"
#include "xmfile.h"
#include "mp4file.h"

using namespace TagLib;

class File::FilePrivate
{
public:
  FilePrivate(IOStream *stream, bool owner);

  IOStream *stream;
  bool streamOwner;
  bool valid;
};

File::FilePrivate::FilePrivate(IOStream *stream, bool owner) :
  stream(stream),
  streamOwner(owner),
  valid(true)
{
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

File::File(FileName fileName) :
  d(new FilePrivate(new FileStream(fileName), true))
{
}

File::File(IOStream *stream) :
  d(new FilePrivate(stream, false))
{
}

File::~File()
{
  if(d->stream && d->streamOwner)
    delete d->stream;
  delete d;
}

FileName File::name() const
{
  return d->stream->name();
}

PropertyMap File::properties() const
{
  // ugly workaround until this method is virtual
  if(dynamic_cast<const APE::File* >(this))
    return dynamic_cast<const APE::File* >(this)->properties();
  if(dynamic_cast<const FLAC::File* >(this))
    return dynamic_cast<const FLAC::File* >(this)->properties();
  if(dynamic_cast<const IT::File* >(this))
    return dynamic_cast<const IT::File* >(this)->properties();
  if(dynamic_cast<const Mod::File* >(this))
    return dynamic_cast<const Mod::File* >(this)->properties();
  if(dynamic_cast<const MPC::File* >(this))
    return dynamic_cast<const MPC::File* >(this)->properties();
  if(dynamic_cast<const MPEG::File* >(this))
    return dynamic_cast<const MPEG::File* >(this)->properties();
  if(dynamic_cast<const Ogg::FLAC::File* >(this))
    return dynamic_cast<const Ogg::FLAC::File* >(this)->properties();
  if(dynamic_cast<const Ogg::Speex::File* >(this))
    return dynamic_cast<const Ogg::Speex::File* >(this)->properties();
  if(dynamic_cast<const Ogg::Opus::File* >(this))
    return dynamic_cast<const Ogg::Opus::File* >(this)->properties();
  if(dynamic_cast<const Ogg::Vorbis::File* >(this))
    return dynamic_cast<const Ogg::Vorbis::File* >(this)->properties();
  if(dynamic_cast<const RIFF::AIFF::File* >(this))
    return dynamic_cast<const RIFF::AIFF::File* >(this)->properties();
  if(dynamic_cast<const RIFF::WAV::File* >(this))
    return dynamic_cast<const RIFF::WAV::File* >(this)->properties();
  if(dynamic_cast<const S3M::File* >(this))
    return dynamic_cast<const S3M::File* >(this)->properties();
  if(dynamic_cast<const TrueAudio::File* >(this))
    return dynamic_cast<const TrueAudio::File* >(this)->properties();
  if(dynamic_cast<const WavPack::File* >(this))
    return dynamic_cast<const WavPack::File* >(this)->properties();
  if(dynamic_cast<const XM::File* >(this))
    return dynamic_cast<const XM::File* >(this)->properties();
  if(dynamic_cast<const MP4::File* >(this))
    return dynamic_cast<const MP4::File* >(this)->properties();
  if(dynamic_cast<const ASF::File* >(this))
    return dynamic_cast<const ASF::File* >(this)->properties();
  return tag()->properties();
}

void File::removeUnsupportedProperties(const StringList &properties)
{
  // here we only consider those formats that could possibly contain
  // unsupported properties
  if(dynamic_cast<APE::File* >(this))
    dynamic_cast<APE::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<FLAC::File* >(this))
    dynamic_cast<FLAC::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<MPC::File* >(this))
    dynamic_cast<MPC::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<MPEG::File* >(this))
    dynamic_cast<MPEG::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<Ogg::Vorbis::File* >(this))
    dynamic_cast<Ogg::Vorbis::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<RIFF::AIFF::File* >(this))
    dynamic_cast<RIFF::AIFF::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<RIFF::WAV::File* >(this))
    dynamic_cast<RIFF::WAV::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<TrueAudio::File* >(this))
    dynamic_cast<TrueAudio::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<WavPack::File* >(this))
    dynamic_cast<WavPack::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<MP4::File* >(this))
    dynamic_cast<MP4::File* >(this)->removeUnsupportedProperties(properties);
  else if(dynamic_cast<ASF::File* >(this))
    dynamic_cast<ASF::File* >(this)->removeUnsupportedProperties(properties);
  else
    tag()->removeUnsupportedProperties(properties);
}

PropertyMap File::setProperties(const PropertyMap &properties)
{
  if(dynamic_cast<APE::File* >(this))
    return dynamic_cast<APE::File* >(this)->setProperties(properties);
  else if(dynamic_cast<FLAC::File* >(this))
    return dynamic_cast<FLAC::File* >(this)->setProperties(properties);
  else if(dynamic_cast<IT::File* >(this))
    return dynamic_cast<IT::File* >(this)->setProperties(properties);
  else if(dynamic_cast<Mod::File* >(this))
    return dynamic_cast<Mod::File* >(this)->setProperties(properties);
  else if(dynamic_cast<MPC::File* >(this))
    return dynamic_cast<MPC::File* >(this)->setProperties(properties);
  else if(dynamic_cast<MPEG::File* >(this))
    return dynamic_cast<MPEG::File* >(this)->setProperties(properties);
  else if(dynamic_cast<Ogg::FLAC::File* >(this))
    return dynamic_cast<Ogg::FLAC::File* >(this)->setProperties(properties);
  else if(dynamic_cast<Ogg::Speex::File* >(this))
    return dynamic_cast<Ogg::Speex::File* >(this)->setProperties(properties);
  else if(dynamic_cast<Ogg::Opus::File* >(this))
    return dynamic_cast<Ogg::Opus::File* >(this)->setProperties(properties);
  else if(dynamic_cast<Ogg::Vorbis::File* >(this))
    return dynamic_cast<Ogg::Vorbis::File* >(this)->setProperties(properties);
  else if(dynamic_cast<RIFF::AIFF::File* >(this))
    return dynamic_cast<RIFF::AIFF::File* >(this)->setProperties(properties);
  else if(dynamic_cast<RIFF::WAV::File* >(this))
    return dynamic_cast<RIFF::WAV::File* >(this)->setProperties(properties);
  else if(dynamic_cast<S3M::File* >(this))
    return dynamic_cast<S3M::File* >(this)->setProperties(properties);
  else if(dynamic_cast<TrueAudio::File* >(this))
    return dynamic_cast<TrueAudio::File* >(this)->setProperties(properties);
  else if(dynamic_cast<WavPack::File* >(this))
    return dynamic_cast<WavPack::File* >(this)->setProperties(properties);
  else if(dynamic_cast<XM::File* >(this))
    return dynamic_cast<XM::File* >(this)->setProperties(properties);
  else if(dynamic_cast<MP4::File* >(this))
    return dynamic_cast<MP4::File* >(this)->setProperties(properties);
  else if(dynamic_cast<ASF::File* >(this))
    return dynamic_cast<ASF::File* >(this)->setProperties(properties);
  else
    return tag()->setProperties(properties);
}

ByteVector File::readBlock(unsigned long length)
{
  return d->stream->readBlock(length);
}

void File::writeBlock(const ByteVector &data)
{
  d->stream->writeBlock(data);
}

long File::find(const ByteVector &pattern, long fromOffset, const ByteVector &before)
{
  if(!d->stream || pattern.isEmpty())
      return -1;

  // The position in the file that the current buffer starts at.

  long bufferOffset = fromOffset;

  // Save the location of the current read pointer.  We will restore the
  // position using seek() before all returns.

  const long originalPosition = tell();

  // Loop until we find either 'pattern' or 'before'.

  unsigned int patternIndex = 0;
  unsigned int beforeIndex  = 0;

  while(true) {
    seek(bufferOffset);
    const ByteVector buffer = readBlock(bufferSize());
    if(buffer.isEmpty())
      break;

    // Search the buffer for either 'pattern' or 'before' simultaneously.

    for(unsigned int i = 0; i < buffer.size(); ++i) {
      if(buffer[i] == pattern[patternIndex])
        patternIndex++;
      else
        patternIndex = 0;

      if(patternIndex == pattern.size()) {
        seek(originalPosition);
        return bufferOffset + i + 1 - pattern.size();
      }

      if(!before.isEmpty()) {
        if(buffer[i] == before[beforeIndex])
          beforeIndex++;
        else
          beforeIndex = 0;

        if(beforeIndex == before.size()) {
          seek(originalPosition);
          return -1;
        }
      }
    }

    bufferOffset += buffer.size();
  }

  // Since we hit the end of the file, reset the status before continuing.

  clear();
  seek(originalPosition);
  return -1;
}

long File::rfind(const ByteVector &pattern, long fromOffset, const ByteVector &before)
{
  if(!d->stream || pattern.isEmpty())
      return -1;

  if(fromOffset == 0)
    fromOffset = length();

  // The position in the file that the current buffer starts at.

  long bufferOffset = std::min<long>(fromOffset + pattern.size(), length());

  // Save the location of the current read pointer.  We will restore the
  // position using seek() before all returns.

  const long originalPosition = tell();

  // Loop until we find either 'pattern' or 'before'.

  int patternIndex = pattern.size() - 1;
  int beforeIndex  = before.size()  - 1;

  while(bufferOffset > 0) {
    const long bufferLength = std::min<long>(bufferOffset, bufferSize());
    bufferOffset -= bufferLength;

    seek(bufferOffset);
    const ByteVector buffer = readBlock(bufferLength);

    // Search the buffer for either 'pattern' or 'before' simultaneously.

    for(int i = buffer.size() - 1; i >= 0; --i) {
      if(buffer[i] == pattern[patternIndex])
        patternIndex--;
      else
        patternIndex = pattern.size() - 1;

      if(patternIndex == -1) {
        seek(originalPosition);
        return bufferOffset + i;
      }

      if(!before.isEmpty()) {
        if(buffer[i] == before[beforeIndex])
          beforeIndex--;
        else
          beforeIndex = before.size() - 1;

        if(beforeIndex == -1) {
          seek(originalPosition);
          return -1;
        }
      }
    }
  }

  seek(originalPosition);
  return -1;
}

void File::insert(const ByteVector &data, unsigned long start, unsigned long replace)
{
  d->stream->insert(data, start, replace);
}

void File::removeBlock(unsigned long start, unsigned long length)
{
  d->stream->removeBlock(start, length);
}

bool File::readOnly() const
{
  return d->stream->readOnly();
}

bool File::isOpen() const
{
  return d->stream->isOpen();
}

bool File::isValid() const
{
  return isOpen() && d->valid;
}

void File::seek(long offset, Position p)
{
  d->stream->seek(offset, IOStream::Position(p));
}

void File::truncate(long length)
{
  d->stream->truncate(length);
}

void File::clear()
{
  d->stream->clear();
}

long File::tell() const
{
  return d->stream->tell();
}

long File::length()
{
  return d->stream->length();
}

bool File::isReadable(const char *file)
{

#if defined(_MSC_VER) && (_MSC_VER >= 1400)  // VC++2005 or later

  return _access_s(file, R_OK) == 0;

#else

  return access(file, R_OK) == 0;

#endif

}

bool File::isWritable(const char *file)
{

#if defined(_MSC_VER) && (_MSC_VER >= 1400)  // VC++2005 or later

  return _access_s(file, W_OK) == 0;

#else

  return access(file, W_OK) == 0;

#endif

}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

unsigned int File::bufferSize()
{
  return 1024;
}

void File::setValid(bool valid)
{
  d->valid = valid;
}

