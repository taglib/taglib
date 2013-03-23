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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
# include <wchar.h>
# include <windows.h>
# include <io.h>
# define ftruncate _chsize
#else
# include <unistd.h>
#endif

#include <stdlib.h>

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

#ifdef _WIN32

  static const size_t bufferSize = 8192;

#else

  static const size_t bufferSize = 1024;

#endif
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

File::File(FileName fileName)
{
  IOStream *stream = new FileStream(fileName);
  d = new FilePrivate(stream, true);
}

File::File(IOStream *stream)
{
  d = new FilePrivate(stream, false);
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
  return tag()->properties();
}

void File::removeUnsupportedProperties(const StringList &properties)
{
  tag()->removeUnsupportedProperties(properties);
}

PropertyMap File::setProperties(const PropertyMap &properties)
{
  return tag()->setProperties(properties);
}

ByteVector File::readBlock(size_t length)
{
  return d->stream->readBlock(length);
}

void File::writeBlock(const ByteVector &data)
{
  d->stream->writeBlock(data);
}

offset_t File::find(const ByteVector &pattern, offset_t fromOffset, const ByteVector &before)
{
  if(!d->stream || pattern.size() > d->bufferSize)
      return -1;

  // The position in the file that the current buffer starts at.

  offset_t bufferOffset = fromOffset;

  // These variables are used to keep track of a partial match that happens at
  // the end of a buffer.

  size_t previousPartialMatch = ByteVector::npos;
  size_t beforePreviousPartialMatch = ByteVector::npos;

  // Save the location of the current read pointer.  We will restore the
  // position using seek() before all returns.

  offset_t originalPosition = tell();

  // Start the search at the offset.

  seek(fromOffset);

  // This loop is the crux of the find method.  There are three cases that we
  // want to account for:
  //
  // (1) The previously searched buffer contained a partial match of the search
  // pattern and we want to see if the next one starts with the remainder of
  // that pattern.
  //
  // (2) The search pattern is wholly contained within the current buffer.
  //
  // (3) The current buffer ends with a partial match of the pattern.  We will
  // note this for use in the next itteration, where we will check for the rest
  // of the pattern.
  //
  // All three of these are done in two steps.  First we check for the pattern
  // and do things appropriately if a match (or partial match) is found.  We
  // then check for "before".  The order is important because it gives priority
  // to "real" matches.

  while(true)
  {
    ByteVector buffer = readBlock(bufferSize());
    if(buffer.isEmpty())
      break;

    // (1) previous partial match

    if(previousPartialMatch != ByteVector::npos 
      && d->bufferSize > previousPartialMatch) 
    {
      const size_t patternOffset = (d->bufferSize - previousPartialMatch);
      if(buffer.containsAt(pattern, 0, patternOffset)) {
        seek(originalPosition);
        return bufferOffset - d->bufferSize + previousPartialMatch;
      }
    }

    if(!before.isNull() 
      && beforePreviousPartialMatch != ByteVector::npos 
      && d->bufferSize > beforePreviousPartialMatch) 
    {
      const size_t beforeOffset = (d->bufferSize - beforePreviousPartialMatch);
      if(buffer.containsAt(before, 0, beforeOffset)) {
        seek(originalPosition);
        return -1;
      }
    }

    // (2) pattern contained in current buffer

    size_t location = buffer.find(pattern);
    if(location != ByteVector::npos) {
      seek(originalPosition);
      return bufferOffset + location;
    }

    if(!before.isNull() && buffer.find(before) != ByteVector::npos) {
      seek(originalPosition);
      return -1;
    }

    // (3) partial match

    previousPartialMatch = buffer.endsWithPartialMatch(pattern);

    if(!before.isNull())
      beforePreviousPartialMatch = buffer.endsWithPartialMatch(before);

    bufferOffset += d->bufferSize;
  }

  // Since we hit the end of the file, reset the status before continuing.

  clear();

  seek(originalPosition);

  return -1;
}


offset_t File::rfind(const ByteVector &pattern, offset_t fromOffset, const ByteVector &before)
{
  if(!d->stream || pattern.size() > d->bufferSize)
      return -1;

  // The position in the file that the current buffer starts at.

  ByteVector buffer;

  // These variables are used to keep track of a partial match that happens at
  // the end of a buffer.

  /*
  int previousPartialMatch = -1;
  int beforePreviousPartialMatch = -1;
  */

  // Save the location of the current read pointer.  We will restore the
  // position using seek() before all returns.

  offset_t originalPosition = tell();

  // Start the search at the offset.

  offset_t bufferOffset;
  if(fromOffset == 0) {
    seek(-1 * int(d->bufferSize), End);
    bufferOffset = tell();
  }
  else {
    seek(fromOffset + -1 * int(d->bufferSize), Beginning);
    bufferOffset = tell();
  }

  // See the notes in find() for an explanation of this algorithm.

  for(buffer = readBlock(d->bufferSize); buffer.size() > 0; buffer = readBlock(d->bufferSize)) {

    // TODO: (1) previous partial match

    // (2) pattern contained in current buffer

    const size_t location = buffer.rfind(pattern);
    if(location != ByteVector::npos) {
      seek(originalPosition);
      return bufferOffset + location;
    }

    if(!before.isNull() && buffer.find(before) != ByteVector::npos) {
      seek(originalPosition);
      return -1;
    }

    // TODO: (3) partial match

    bufferOffset -= d->bufferSize;
    seek(bufferOffset);
  }

  // Since we hit the end of the file, reset the status before continuing.

  clear();

  seek(originalPosition);

  return -1;
}

void File::insert(const ByteVector &data, offset_t start, size_t replace)
{
  d->stream->insert(data, start, replace);
}

void File::removeBlock(offset_t start, size_t length)
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

void File::seek(offset_t offset, Position p)
{
  d->stream->seek(offset, IOStream::Position(p));
}

void File::truncate(offset_t length)
{
  d->stream->truncate(length);
}

void File::clear()
{
  d->stream->clear();
}

offset_t File::tell() const
{
  return d->stream->tell();
}

offset_t File::length()
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

String File::toString() const
{
  StringList desc;
  AudioProperties *properties = audioProperties();
  if(properties) {
    desc.append(properties->toString());
  }
  Tag *t = tag();
  if(t) {
    desc.append(t->toString());
  }
  return desc.toString("\n");
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

size_t File::bufferSize()
{
  return FilePrivate::bufferSize;
}

void File::setValid(bool valid)
{
  d->valid = valid;
}

