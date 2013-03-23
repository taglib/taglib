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

#include "tfilestream.h"
#include "tstring.h"
#include "tdebug.h"

#include <string.h>
#include <sys/stat.h>

#ifndef _WIN32
# include <stdio.h>
# include <unistd.h>
#endif

#include <stdlib.h>
#include <limits>

using namespace TagLib;

namespace {
#ifdef _WIN32

  // For Windows 

  typedef FileName FileNameHandle;

  // Using native file handles instead of file descriptors for reducing the resource consumption.

  const HANDLE InvalidFile = INVALID_HANDLE_VALUE;

  HANDLE openFile(const FileName &path, bool readOnly)
  {
    DWORD access = readOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);

    if(!path.wstr().empty())
      return CreateFileW(path.wstr().c_str(), access, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    else
      return CreateFileA(path.str().c_str(), access, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  }

  size_t fread(void *ptr, size_t size, size_t nmemb, HANDLE stream)
  {
    DWORD readLen;
    ReadFile(stream, ptr, static_cast<DWORD>(size * nmemb), &readLen, NULL);

    return (readLen / size);
  }

  size_t fwrite(const void *ptr, size_t size, size_t nmemb, HANDLE stream)
  {
    DWORD writtenLen;
    WriteFile(stream, ptr, static_cast<DWORD>(size * nmemb), &writtenLen, NULL);

    return writtenLen;
  }

#else

  // For non-Windows 

  FILE *const InvalidFile = 0;

  struct FileNameHandle : public std::string
  {
    FileNameHandle(FileName name) : std::string(name) {}
    operator FileName () const { return c_str(); }
  };

  FILE *openFile(const FileName &path, bool readOnly)
  {
    return fopen(path, readOnly ? "rb" : "rb+");
  }

#endif
}

class FileStream::FileStreamPrivate
{
public:
  FileStreamPrivate(FileName fileName, bool openReadOnly);

#ifdef _WIN32

  HANDLE file;

#else

  FILE *file;

#endif

  FileNameHandle name;

  bool readOnly;
  offset_t size;

#ifdef _WIN32

  static const size_t bufferSize = 8196;

#else

  static const size_t bufferSize = 1024;

#endif
};

FileStream::FileStreamPrivate::FileStreamPrivate(FileName fileName, bool openReadOnly) :
  file(InvalidFile),
  name(fileName),
  readOnly(true),
  size(0)
{
  // First try with read / write mode, if that fails, fall back to read only.

  if(!openReadOnly)
    file = openFile(name, false);

  if(file != InvalidFile)
    readOnly = false;
  else
    file = openFile(name, true);

  if(file == InvalidFile) {
#ifdef _WIN32
    String n;
    if(!name.wstr().empty())
      n = name.wstr();
    else
      n = name.str();
#else
    String n(name);
#endif // DEBUG

    debug("Could not open file " + n);
  }
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FileStream::FileStream(FileName file, bool openReadOnly)
{
  d = new FileStreamPrivate(file, openReadOnly);
}

FileStream::~FileStream()
{
#ifdef _WIN32

  if(d->file)
    CloseHandle(d->file);

#else

  if(d->file)
    fclose(d->file);

#endif

  delete d;
}

FileName FileStream::name() const
{
  return d->name;
}

ByteVector FileStream::readBlock(size_t length)
{
  if(!d->file) {
    debug("FileStream::readBlock() -- Invalid File");
    return ByteVector::null;
  }

  if(length == 0)
    return ByteVector::null;

  if(length > FileStreamPrivate::bufferSize && static_cast<offset_t>(length) > FileStream::length())
    length = static_cast<size_t>(FileStream::length());

  ByteVector v(length, 0);
  const size_t count = fread(v.data(), sizeof(char), length, d->file);
  v.resize(count);
  return v;
}

void FileStream::writeBlock(const ByteVector &data)
{
  if(!d->file)
    return;

  if(d->readOnly) {
    debug("File::writeBlock() -- attempted to write to a file that is not writable");
    return;
  }

  fwrite(data.data(), sizeof(char), data.size(), d->file);
}

void FileStream::insert(const ByteVector &data, offset_t start, size_t replace)
{
  if(!d->file)
    return;

  if(data.size() == replace) {
    seek(start);
    writeBlock(data);
    return;
  }
  else if(data.size() < replace) {
      seek(start);
      writeBlock(data);
      removeBlock(start + data.size(), replace - data.size());
      return;
  }

  // Woohoo!  Faster (about 20%) than id3lib at last.  I had to get hardcore
  // and avoid TagLib's high level API for rendering just copying parts of
  // the file that don't contain tag data.
  //
  // Now I'll explain the steps in this ugliness:

  // First, make sure that we're working with a buffer that is longer than
  // the *differnce* in the tag sizes.  We want to avoid overwriting parts
  // that aren't yet in memory, so this is necessary.

  size_t bufferLength = FileStreamPrivate::bufferSize;

  while(data.size() - replace > bufferLength)
    bufferLength += FileStreamPrivate::bufferSize;

  // Set where to start the reading and writing.

  offset_t readPosition  = start + replace;
  offset_t writePosition = start;

  ByteVector buffer;
  ByteVector aboutToOverwrite(bufferLength, 0);

  // This is basically a special case of the loop below.  Here we're just
  // doing the same steps as below, but since we aren't using the same buffer
  // size -- instead we're using the tag size -- this has to be handled as a
  // special case.  We're also using File::writeBlock() just for the tag.
  // That's a bit slower than using char *'s so, we're only doing it here.

  seek(readPosition);
  size_t bytesRead = fread(aboutToOverwrite.data(), sizeof(char), bufferLength, d->file);
  readPosition += bufferLength;

  seek(writePosition);
  writeBlock(data);
  writePosition += data.size();

  buffer = aboutToOverwrite;

  // In case we've already reached the end of file...

  buffer.resize(bytesRead);

  // Ok, here's the main loop.  We want to loop until the read fails, which
  // means that we hit the end of the file.

  while(!buffer.isEmpty()) {

    // Seek to the current read position and read the data that we're about
    // to overwrite.  Appropriately increment the readPosition.

    seek(readPosition);
    bytesRead = fread(aboutToOverwrite.data(), sizeof(char), bufferLength, d->file);
    aboutToOverwrite.resize(static_cast<TagLib::uint>(bytesRead));
    readPosition += bufferLength;

    // Check to see if we just read the last block.  We need to call clear()
    // if we did so that the last write succeeds.

    if(bytesRead < bufferLength)
      clear();

    // Seek to the write position and write our buffer.  Increment the
    // writePosition.

    seek(writePosition);
    fwrite(buffer.data(), sizeof(char), buffer.size(), d->file);
    writePosition += buffer.size();

    // Make the current buffer the data that we read in the beginning.

    buffer = aboutToOverwrite;

    // Again, we need this for the last write.  We don't want to write garbage
    // at the end of our file, so we need to set the buffer size to the amount
    // that we actually read.

    bufferLength = bytesRead;
  }

  // Clear the file size cache. 
  d->size = 0;
}

void FileStream::removeBlock(offset_t start, size_t length)
{
  if(!d->file)
    return;

  size_t bufferLength = FileStreamPrivate::bufferSize;

  offset_t readPosition  = start + length;
  offset_t writePosition = start;

  ByteVector buffer(bufferLength, 0);

  size_t bytesRead = 1;

  while(bytesRead != 0) {
    seek(readPosition);
    bytesRead = fread(buffer.data(), sizeof(char), bufferLength, d->file);
    readPosition += bytesRead;

    // Check to see if we just read the last block.  We need to call clear()
    // if we did so that the last write succeeds.

    if(bytesRead < bufferLength)
      clear();

    seek(writePosition);
    fwrite(buffer.data(), sizeof(char), bytesRead, d->file);
    writePosition += bytesRead;
  }
  truncate(writePosition);
}

bool FileStream::readOnly() const
{
  return d->readOnly;
}

bool FileStream::isOpen() const
{
  return (d->file != NULL);
}

void FileStream::seek(offset_t offset, Position p)
{
  if(!d->file) {
    debug("FileStream::seek() -- trying to seek in a file that isn't opened.");
    return;
  }

#ifdef _WIN32

  DWORD whence;
  switch(p) {
  case Beginning:
    whence = FILE_BEGIN;
    break;
  case Current:
    whence = FILE_CURRENT;
    break;
  case End:
    whence = FILE_END;
    break;
  default:
    debug("FileStream::seek() -- Invalid Position value.");
    return;
  }

  LARGE_INTEGER largeOffset = {};
  largeOffset.QuadPart = offset;
  SetFilePointerEx(d->file, largeOffset, nullptr, whence);

#else

  int whence;
  switch(p) {
  case Beginning:
    whence = SEEK_SET;
    break;
  case Current:
    whence = SEEK_CUR;
    break;
  case End:
    whence = SEEK_END;
    break;
  default:
    debug("FileStream::seek() -- Invalid Position value.");
    return;
  }

# ifdef _LARGEFILE_SOURCE

  fseeko(d->file, offset, whence);

# else

  fseek(d->file, static_cast<long>(offset), whence);

# endif 

#endif
}

void FileStream::clear()
{
#ifdef _WIN32

  // NOP

#else

  clearerr(d->file);

#endif
}

offset_t FileStream::tell() const
{
#ifdef _WIN32

  const LARGE_INTEGER largeOffset = {};
  LARGE_INTEGER newPointer;

  SetFilePointerEx(d->file, largeOffset, &newPointer, FILE_CURRENT);

  return newPointer.QuadPart;

#else

# ifdef _LARGEFILE_SOURCE

  return ftello(d->file);

# else

  return static_cast<offset_t>(ftell(d->file));

# endif 

#endif
}

offset_t FileStream::length()
{
  // Do some caching in case we do multiple calls.

  if(d->size > 0)
    return d->size;

  if(!d->file)
    return 0;

  const offset_t currentPosition = tell();

  seek(0, End);
  offset_t endPosition = tell();

  seek(currentPosition, Beginning);

  d->size = endPosition;
  return endPosition;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void FileStream::truncate(offset_t length)
{
#ifdef _WIN32

  const offset_t currentPosition = tell();

  seek(length);
  BOOL set = SetEndOfFile(d->file);
  if(!set) {
    debug("FileStream::truncate() -- Coundn't truncate the file.");
  }
  seek(currentPosition);

#else

  const int error = ftruncate(fileno(d->file), length);
  if(error != 0) {
    debug("FileStream::truncate() -- Coundn't truncate the file.");
  }

#endif
}
