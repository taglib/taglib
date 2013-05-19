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
#include "audioproperties.h"

using namespace TagLib;

class File::FilePrivateBase
{
public:
  FilePrivateBase()
    : valid(true)
  {
  }

  virtual ~FilePrivateBase()
  {
  }

  virtual IOStream *stream() const = 0;

  bool valid;
};

// FilePrivate implementation which takes ownership of the stream.

class File::ManagedFilePrivate : public File::FilePrivateBase
{
public:
  ManagedFilePrivate(IOStream *stream)
    : p(stream)
  {
  }

  virtual IOStream *stream() const
  {
    return p.get();
  }

private:
  NonRefCountPtr<IOStream> p;
};

// FilePrivate implementation which doesn't take ownership of the stream.

class File::UnmanagedFilePrivate : public File::FilePrivateBase
{
public:
  UnmanagedFilePrivate(IOStream *stream)
    : p(stream)
  {
  }

  virtual IOStream *stream() const
  {
    return p;
  }

private:
  IOStream *p;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

File::~File()
{
}

FileName File::name() const
{
  return d->stream()->name();
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
  return d->stream()->readBlock(length);
}

void File::writeBlock(const ByteVector &data)
{
  d->stream()->writeBlock(data);
}

offset_t File::find(const ByteVector &pattern, offset_t fromOffset, const ByteVector &before)
{
  if(!d->stream() || pattern.size() > bufferSize())
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
      && bufferSize() > previousPartialMatch) 
    {
      const size_t patternOffset = (bufferSize() - previousPartialMatch);
      if(buffer.containsAt(pattern, 0, patternOffset)) {
        seek(originalPosition);
        return bufferOffset - bufferSize() + previousPartialMatch;
      }
    }

    if(!before.isNull() 
      && beforePreviousPartialMatch != ByteVector::npos 
      && bufferSize() > beforePreviousPartialMatch) 
    {
      const size_t beforeOffset = (bufferSize() - beforePreviousPartialMatch);
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

    bufferOffset += bufferSize();
  }

  // Since we hit the end of the file, reset the status before continuing.

  clear();

  seek(originalPosition);

  return -1;
}


offset_t File::rfind(const ByteVector &pattern, offset_t fromOffset, const ByteVector &before)
{
  if(!d->stream() || pattern.size() > bufferSize())
      return -1;

  // Save the location of the current read pointer.  We will restore the
  // position using seek() before all returns.

  offset_t originalPosition = tell();

  // Start the search at the offset.

  offset_t bufferOffset;
  if(fromOffset == 0) {
    seek(-1 * int(bufferSize()), End);
    bufferOffset = tell();
  }
  else {
    seek(fromOffset + -1 * int(bufferSize()), Beginning);
    bufferOffset = tell();
  }

  // See the notes in find() for an explanation of this algorithm.

  while(true)
  {
    ByteVector buffer = readBlock(bufferSize());
    if(buffer.isEmpty())
      break;

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

    bufferOffset -= bufferSize();
    seek(bufferOffset);
  }

  // Since we hit the end of the file, reset the status before continuing.

  clear();

  seek(originalPosition);

  return -1;
}

void File::insert(const ByteVector &data, offset_t start, size_t replace)
{
  d->stream()->insert(data, start, replace);
}

void File::removeBlock(offset_t start, size_t length)
{
  d->stream()->removeBlock(start, length);
}

bool File::readOnly() const
{
  return d->stream()->readOnly();
}

bool File::isOpen() const
{
  return d->stream()->isOpen();
}

bool File::isValid() const
{
  return isOpen() && d->valid;
}

void File::seek(offset_t offset, Position p)
{
  d->stream()->seek(offset, IOStream::Position(p));
}

void File::truncate(offset_t length)
{
  d->stream()->truncate(length);
}

void File::clear()
{
  d->stream()->clear();
}

offset_t File::tell() const
{
  return d->stream()->tell();
}

offset_t File::length()
{
  return d->stream()->length();
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

File::File(const FileName &fileName)
  : d(new ManagedFilePrivate(new FileStream(fileName)))
{
}

File::File(IOStream *stream)
  : d(new UnmanagedFilePrivate(stream))
{
}

size_t File::bufferSize()
{
  return FileStream::bufferSize();
}

void File::setValid(bool valid)
{
  d->valid = valid;
}

