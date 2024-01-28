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

#ifdef _WIN32
# include <windows.h>
#else
# include <climits>
# include <cstdio>
# include <unistd.h>
#endif

#include "tstring.h"
#include "tdebug.h"

using namespace TagLib;

namespace
{
#ifdef _WIN32

  // Uses Win32 native API instead of POSIX API to reduce the resource consumption.

  using FileNameHandle = FileName;
  using FileHandle = HANDLE;

  const FileHandle InvalidFileHandle = INVALID_HANDLE_VALUE;

  FileHandle openFile(const FileName &path, bool readOnly)
  {
    const DWORD access = readOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);

#if defined (PLATFORM_WINRT)
    return CreateFile2(path.wstr().c_str(), access, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
#else
    constexpr wchar_t LongLocalPathPrefix[] = L"\\\\?\\";
    constexpr wchar_t UNCPathPrefix[] = L"\\\\";
    constexpr wchar_t LongUNCPathPrefix[] = L"\\\\?\\UNC\\";
    std::wstring pathWStr = path.wstr();
    if(pathWStr.length() > MAX_PATH &&
       pathWStr.compare(0, std::size(LongLocalPathPrefix) - 1, LongLocalPathPrefix) != 0 &&
       pathWStr.compare(0, std::size(LongUNCPathPrefix) - 1, LongUNCPathPrefix) != 0) {
      if(pathWStr.compare(0, std::size(UNCPathPrefix) - 1, UNCPathPrefix) == 0) {
        pathWStr = LongUNCPathPrefix + pathWStr.substr(2);
      }
      else {
        pathWStr = LongLocalPathPrefix + pathWStr;
      }
    }
    return CreateFileW(pathWStr.c_str(), access, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
#endif
  }

  FileHandle openFile([[maybe_unused]] const int fileDescriptor, [[maybe_unused]] bool readOnly)
  {
    return InvalidFileHandle;
  }

  void closeFile(FileHandle file)
  {
    CloseHandle(file);
  }

  size_t readFile(FileHandle file, ByteVector &buffer)
  {
    DWORD length;
    if(ReadFile(file, buffer.data(), static_cast<DWORD>(buffer.size()), &length, nullptr))
      return static_cast<size_t>(length);
    return 0;
  }

  size_t writeFile(FileHandle file, const ByteVector &buffer)
  {
    DWORD length;
    if(WriteFile(file, buffer.data(), static_cast<DWORD>(buffer.size()), &length, nullptr))
      return static_cast<size_t>(length);
    return 0;
  }

#else   // _WIN32

  struct FileNameHandle : public std::string
  {
    FileNameHandle(FileName name) : std::string(name) {}
    operator FileName () const { return c_str(); }
  };

  using FileHandle = FILE *;

  const FileHandle InvalidFileHandle = nullptr;

  FileHandle openFile(const FileName &path, bool readOnly)
  {
    return fopen(path, readOnly ? "rb" : "rb+");
  }

  FileHandle openFile(const int fileDescriptor, bool readOnly)
  {
    return fdopen(fileDescriptor, readOnly ? "rb" : "rb+");
  }

  void closeFile(FileHandle file)
  {
    fclose(file);
  }

  size_t readFile(FileHandle file, ByteVector &buffer)
  {
    return fread(buffer.data(), sizeof(char), buffer.size(), file);
  }

  size_t writeFile(FileHandle file, const ByteVector &buffer)
  {
    return fwrite(buffer.data(), sizeof(char), buffer.size(), file);
  }

#endif  // _WIN32
}  // namespace

class FileStream::FileStreamPrivate
{
public:
  FileStreamPrivate(const FileName &fileName) :
    name(fileName)
  {
  }

  FileHandle file { InvalidFileHandle };
  FileNameHandle name;
  bool readOnly { true };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FileStream::FileStream(FileName fileName, bool openReadOnly) :
  d(std::make_unique<FileStreamPrivate>(fileName))
{
  // First try with read / write mode, if that fails, fall back to read only.

  if(!openReadOnly)
    d->file = openFile(fileName, false);

  if(d->file != InvalidFileHandle)
    d->readOnly = false;
  else
    d->file = openFile(fileName, true);

  if(d->file == InvalidFileHandle)
# ifdef _WIN32
    debug("Could not open file " + fileName.toString());
# else
    debug("Could not open file " + String(static_cast<const char *>(d->name)));
# endif
}

FileStream::FileStream(int fileDescriptor, bool openReadOnly) :
  d(std::make_unique<FileStreamPrivate>(""))
{
  // First try with read / write mode, if that fails, fall back to read only.

  if(!openReadOnly)
    d->file = openFile(fileDescriptor, false);

  if(d->file != InvalidFileHandle)
    d->readOnly = false;
  else
    d->file = openFile(fileDescriptor, true);

  if(d->file == InvalidFileHandle)
    debug("Could not open file using file descriptor");
}

FileStream::~FileStream()
{
  if(isOpen())
    closeFile(d->file);
}

FileName FileStream::name() const
{
  return d->name;
}

ByteVector FileStream::readBlock(size_t length)
{
  if(!isOpen()) {
    debug("FileStream::readBlock() -- invalid file.");
    return ByteVector();
  }

  if(length == 0)
    return ByteVector();

  if(length > bufferSize()) {
    if(const auto streamLength = static_cast<size_t>(FileStream::length());
       length > streamLength) {
      length = streamLength;
    }
  }

  ByteVector buffer(static_cast<unsigned int>(length));

  const size_t count = readFile(d->file, buffer);
  buffer.resize(static_cast<unsigned int>(count));

  return buffer;
}

void FileStream::writeBlock(const ByteVector &data)
{
  if(!isOpen()) {
    debug("FileStream::writeBlock() -- invalid file.");
    return;
  }

  if(readOnly()) {
    debug("FileStream::writeBlock() -- read only file.");
    return;
  }

  writeFile(d->file, data);
}

void FileStream::insert(const ByteVector &data, offset_t start, size_t replace)
{
  if(!isOpen()) {
    debug("FileStream::insert() -- invalid file.");
    return;
  }

  if(readOnly()) {
    debug("FileStream::insert() -- read only file.");
    return;
  }

  if(data.size() == replace) {
    seek(start);
    writeBlock(data);
    return;
  }
  if(data.size() < replace) {
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
  // the *difference* in the tag sizes.  We want to avoid overwriting parts
  // that aren't yet in memory, so this is necessary.

  size_t bufferLength = bufferSize();

  while(data.size() - replace > bufferLength)
    bufferLength += bufferSize();

  // Set where to start the reading and writing.

  offset_t readPosition = start + replace;
  offset_t writePosition = start;

  ByteVector buffer = data;
  ByteVector aboutToOverwrite(static_cast<unsigned int>(bufferLength));

  while(true) {
    // Seek to the current read position and read the data that we're about
    // to overwrite.  Appropriately increment the readPosition.

    seek(readPosition);
    const auto bytesRead = static_cast<unsigned int>(readFile(d->file, aboutToOverwrite));
    aboutToOverwrite.resize(bytesRead);
    readPosition += bufferLength;

    // Check to see if we just read the last block.  We need to call clear()
    // if we did so that the last write succeeds.

    if(bytesRead < bufferLength)
      clear();

    // Seek to the write position and write our buffer.  Increment the
    // writePosition.

    seek(writePosition);
    writeBlock(buffer);

    // We hit the end of the file.

    if(bytesRead == 0)
      break;

    writePosition += buffer.size();

    // Make the current buffer the data that we read in the beginning.

    buffer = aboutToOverwrite;
  }
}

void FileStream::removeBlock(offset_t start, size_t length)
{
  if(!isOpen()) {
    debug("FileStream::removeBlock() -- invalid file.");
    return;
  }

  unsigned int bufferLength = bufferSize();

  offset_t readPosition = start + length;
  offset_t writePosition = start;

  ByteVector buffer(bufferLength);

  unsigned int bytesRead = UINT_MAX;
  while(bytesRead != 0) {
    seek(readPosition);
    bytesRead = static_cast<unsigned int>(readFile(d->file, buffer));
    readPosition += bytesRead;

    // Check to see if we just read the last block.  We need to call clear()
    // if we did so that the last write succeeds.

    if(bytesRead < buffer.size()) {
      clear();
      buffer.resize(bytesRead);
    }

    seek(writePosition);
    writeFile(d->file, buffer);

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
  return d->file != InvalidFileHandle;
}

void FileStream::seek(offset_t offset, Position p)
{
  if(!isOpen()) {
    debug("FileStream::seek() -- invalid file.");
    return;
  }

#ifdef _WIN32

  if(p != Beginning && p != Current && p != End) {
    debug("FileStream::seek() -- Invalid Position value.");
    return;
  }

  LARGE_INTEGER liOffset;
  liOffset.QuadPart = offset;

  if(!SetFilePointerEx(d->file, liOffset, nullptr, static_cast<DWORD>(p))) {
    debug("FileStream::seek() -- Failed to set the file pointer.");
  }

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

  fseek(d->file, offset, whence);

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

  const LARGE_INTEGER zero = {};
  LARGE_INTEGER position;

  if(SetFilePointerEx(d->file, zero, &position, FILE_CURRENT)) {
    return position.QuadPart;
  }

  debug("FileStream::tell() -- Failed to get the file pointer.");
  return 0;

#else

  return ftell(d->file);

#endif
}

offset_t FileStream::length()
{
  if(!isOpen()) {
    debug("FileStream::length() -- invalid file.");
    return 0;
  }

#ifdef _WIN32

  LARGE_INTEGER fileSize;

  if(GetFileSizeEx(d->file, &fileSize)) {
    return fileSize.QuadPart;
  }

  debug("FileStream::length() -- Failed to get the file size.");
  return 0;

#else

  const offset_t curpos = tell();

  seek(0, End);
  const offset_t endpos = tell();

  seek(curpos, Beginning);

  return endpos;

#endif
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void FileStream::truncate(offset_t length)
{
#ifdef _WIN32

  const offset_t currentPos = tell();

  seek(length);

  if(!SetEndOfFile(d->file)) {
    debug("FileStream::truncate() -- Failed to truncate the file.");
  }

  seek(currentPos);

#else

  fflush(d->file);
  if(const int error = ftruncate(fileno(d->file), length); error != 0)
    debug("FileStream::truncate() -- Couldn't truncate the file.");

#endif
}

unsigned int FileStream::bufferSize()
{
  return 1024;
}
