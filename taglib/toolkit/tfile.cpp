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
#include "tpropertymap.h"
#include "tstring.h"

#ifdef _WIN32
# include <windows.h>
# include <io.h>
#else
#  include <unistd.h>
#endif

#ifndef R_OK
# define R_OK 4
#endif
#ifndef W_OK
# define W_OK 2
#endif

using namespace TagLib;

class File::FilePrivate
{
public:
  FilePrivate(IOStream *stream, bool owner) :
    stream(stream),
    streamOwner(owner)
  {
  }

  ~FilePrivate()
  {
    if(streamOwner)
      delete stream;
  }

  FilePrivate(const FilePrivate &) = delete;
  FilePrivate &operator=(const FilePrivate &) = delete;

  IOStream *stream;
  bool streamOwner;
  bool valid { true };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

File::File(FileName fileName) :
  d(std::make_unique<FilePrivate>(new FileStream(fileName), true))
{
}

File::File(IOStream *stream) :
  d(std::make_unique<FilePrivate>(stream, false))
{
}

File::~File() = default;

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

StringList File::complexPropertyKeys() const
{
  return tag()->complexPropertyKeys();
}

List<VariantMap> File::complexProperties(const String &key) const
{
  return tag()->complexProperties(key);
}

bool File::setComplexProperties(const String &key, const List<VariantMap> &value)
{
  return tag()->setComplexProperties(key, value);
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
  if(!d->stream || pattern.size() > bufferSize())
      return -1;

  // The position in the file that the current buffer starts at.

  offset_t bufferOffset = fromOffset;

  // These variables are used to keep track of a partial match that happens at
  // the end of a buffer.

  int previousPartialMatch = -1;
  int beforePreviousPartialMatch = -1;

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
  // note this for use in the next iteration, where we will check for the rest
  // of the pattern.
  //
  // All three of these are done in two steps.  First we check for the pattern
  // and do things appropriately if a match (or partial match) is found.  We
  // then check for "before".  The order is important because it gives priority
  // to "real" matches.

  for(auto buffer = readBlock(bufferSize()); !buffer.isEmpty(); buffer = readBlock(bufferSize())) {

    // (1) previous partial match

    if(previousPartialMatch >= 0 && static_cast<int>(bufferSize()) > previousPartialMatch) {
      if(const int patternOffset = bufferSize() - previousPartialMatch;
         buffer.containsAt(pattern, 0, patternOffset)) {
        seek(originalPosition);
        return bufferOffset - bufferSize() + previousPartialMatch;
      }
    }

    if(!before.isEmpty() && beforePreviousPartialMatch >= 0 && static_cast<int>(bufferSize()) > beforePreviousPartialMatch) {
      if(const int beforeOffset = bufferSize() - beforePreviousPartialMatch;
         buffer.containsAt(before, 0, beforeOffset)) {
        seek(originalPosition);
        return -1;
      }
    }

    // (2) pattern contained in current buffer

    long location = buffer.find(pattern);
    if(location >= 0) {
      seek(originalPosition);
      return bufferOffset + location;
    }

    if(!before.isEmpty() && buffer.find(before) >= 0) {
      seek(originalPosition);
      return -1;
    }

    // (3) partial match

    previousPartialMatch = buffer.endsWithPartialMatch(pattern);

    if(!before.isEmpty())
      beforePreviousPartialMatch = buffer.endsWithPartialMatch(before);

    bufferOffset += bufferSize();
  }

  // Since we hit the end of the file, reset the status before continuing.

  clear();

  seek(originalPosition);

  return -1;
}


offset_t File::rfind(const ByteVector &pattern, offset_t fromOffset, const ByteVector &before)
{
  if(!d->stream || pattern.size() > bufferSize())
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

  if(fromOffset == 0)
    fromOffset = length();

  offset_t bufferLength = bufferSize();
  offset_t bufferOffset = fromOffset + pattern.size();

  // See the notes in find() for an explanation of this algorithm.

  while(true) {

    if(bufferOffset > bufferLength) {
      bufferOffset -= bufferLength;
    }
    else {
      bufferLength = bufferOffset;
      bufferOffset = 0;
    }
    seek(bufferOffset);

    buffer = readBlock(bufferLength);
    if(buffer.isEmpty())
      break;

    // TODO: (1) previous partial match

    // (2) pattern contained in current buffer

    if(const long location = buffer.rfind(pattern); location >= 0) {
      seek(originalPosition);
      return bufferOffset + location;
    }

    if(!before.isEmpty() && buffer.find(before) >= 0) {
      seek(originalPosition);
      return -1;
    }

    // TODO: (3) partial match
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
  d->stream->seek(offset, static_cast<IOStream::Position>(p));
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
