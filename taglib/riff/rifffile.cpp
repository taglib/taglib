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

#include <tbytevector.h>
#include <tdebug.h>
#include <tstring.h>

#include "rifffile.h"
#include <vector>

using namespace TagLib;

namespace
{
  struct Chunk
  {
    ByteVector name;
    offset_t offset;
    TagLib::uint size;
    char padding;
  };
}

class RIFF::File::FilePrivate
{
public:
  FilePrivate() :
    endianness(BigEndian),
    size(0)
  {

  }
  Endianness endianness;
  ByteVector type;
  TagLib::uint size;
  ByteVector format;

  std::vector<Chunk> chunks;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::File::~File()
{
  delete d;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

RIFF::File::File(FileName file, Endianness endianness) : TagLib::File(file)
{
  d = new FilePrivate;
  d->endianness = endianness;

  if(isOpen())
    read();
}

RIFF::File::File(IOStream *stream, Endianness endianness) : TagLib::File(stream)
{
  d = new FilePrivate;
  d->endianness = endianness;

  if(isOpen())
    read();
}

TagLib::uint RIFF::File::riffSize() const
{
  return d->size;
}

TagLib::uint RIFF::File::chunkCount() const
{
  return static_cast<TagLib::uint>(d->chunks.size());
}

TagLib::uint RIFF::File::chunkDataSize(uint i) const
{
  return d->chunks[i].size;
}

offset_t RIFF::File::chunkOffset(uint i) const
{
  return d->chunks[i].offset;
}

TagLib::uint RIFF::File::chunkPadding(uint i) const
{
  return d->chunks[i].padding;
}

ByteVector RIFF::File::chunkName(uint i) const
{
  if(i >= chunkCount())
    return ByteVector::null;

  return d->chunks[i].name;
}

ByteVector RIFF::File::chunkData(uint i)
{
  if(i >= chunkCount())
    return ByteVector::null;

  // Offset for the first subchunk's data

  long begin = 12 + 8;

  for(uint it = 0; it < i; it++)
    begin += 8 + d->chunks[it].size + d->chunks[it].padding;

  seek(begin);

  return readBlock(d->chunks[i].size);
}

void RIFF::File::setChunkData(uint i, const ByteVector &data)
{
  // First we update the global size

  d->size += ((data.size() + 1) & ~1) - (d->chunks[i].size + d->chunks[i].padding);
  if(d->endianness == BigEndian)
    insert(ByteVector::fromUInt32BE(d->size), 4, 4);
  else
    insert(ByteVector::fromUInt32LE(d->size), 4, 4);

  // Now update the specific chunk

  writeChunk(chunkName(i), data, d->chunks[i].offset - 8, d->chunks[i].size + d->chunks[i].padding + 8);

  d->chunks[i].size = static_cast<uint>(data.size());
  d->chunks[i].padding = (data.size() & 0x01) ? 1 : 0;

  // Now update the internal offsets

  for(i++; i < d->chunks.size(); i++)
    d->chunks[i].offset = d->chunks[i-1].offset + 8 + d->chunks[i-1].size + d->chunks[i-1].padding;
}

void RIFF::File::setChunkData(const ByteVector &name, const ByteVector &data)
{
  setChunkData(name, data, false);
}

void RIFF::File::setChunkData(const ByteVector &name, const ByteVector &data, bool alwaysCreate)
{
  if(d->chunks.size() == 0) {
    debug("RIFF::File::setChunkData - No valid chunks found.");
    return;
  }

  if(alwaysCreate && name != "LIST") {
    debug("RIFF::File::setChunkData - alwaysCreate should be used for only \"LIST\" chunks.");
    return;
  }

  if(!alwaysCreate) {
    for(uint i = 0; i < d->chunks.size(); i++) {
      if(d->chunks[i].name == name) {
        setChunkData(i, data);
        return;
      }
    }
  }

  // Couldn't find an existing chunk, so let's create a new one.

  offset_t offset = d->chunks.back().offset + d->chunks.back().size;

  // First we update the global size

  d->size += static_cast<uint>((offset & 1) + data.size() + 8);
  if(d->endianness == BigEndian)
    insert(ByteVector::fromUInt32BE(d->size), 4, 4);
  else
    insert(ByteVector::fromUInt32LE(d->size), 4, 4);

  // Now add the chunk to the file

  writeChunk(
    name, 
    data, 
    offset, 
    static_cast<uint>(std::max<offset_t>(0, length() - offset)), 
    static_cast<uint>(offset & 1));

  // And update our internal structure

  if(offset & 1) {
    d->chunks.back().padding = 1;
    offset++;
  }

  Chunk chunk;
  chunk.name = name;
  chunk.size = static_cast<uint>(data.size());
  chunk.offset = offset + 8;
  chunk.padding = static_cast<char>(data.size() & 1);

  d->chunks.push_back(chunk);
}

void RIFF::File::removeChunk(uint i)
{
  if(i >= d->chunks.size())
    return;
  
  removeBlock(d->chunks[i].offset - 8, d->chunks[i].size + 8);
  d->chunks.erase(d->chunks.begin() + i);
}

void RIFF::File::removeChunk(const ByteVector &name)
{
  std::vector<Chunk> newChunks;
  for(size_t i = 0; i < d->chunks.size(); ++i) {
    if(d->chunks[i].name == name)
      removeBlock(d->chunks[i].offset - 8, d->chunks[i].size + 8);
    else
      newChunks.push_back(d->chunks[i]);
  }

  d->chunks.swap(newChunks);
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

namespace
{
  bool isValidChunkID(const ByteVector &name)
  {
    if(name.size() != 4) {
      return false;
    }
    for(int i = 0; i < 4; i++) {
      if(name[i] < 32 || name[i] > 127) {
        return false;
      }
    }
    return true;
  }
}

void RIFF::File::read()
{
  d->type = readBlock(4);

  if(d->endianness == BigEndian)
    d->size = readBlock(4).toUInt32BE(0);
  else
    d->size = readBlock(4).toUInt32LE(0);

  d->format = readBlock(4);

  // + 8: chunk header at least, fix for additional junk bytes
  while(tell() + 8 <= length()) {
    ByteVector chunkName = readBlock(4);
    uint chunkSize;
    if(d->endianness == BigEndian)
      chunkSize = readBlock(4).toUInt32BE(0);
    else
      chunkSize = readBlock(4).toUInt32LE(0);

    if(!isValidChunkID(chunkName)) {
      debug("RIFF::File::read() -- Chunk '" + chunkName + "' has invalid ID");
      setValid(false);
      break;
    }

    if(tell() + chunkSize > uint(length())) {
      debug("RIFF::File::read() -- Chunk '" + chunkName + "' has invalid size (larger than the file size)");
      setValid(false);
      break;
    }

    Chunk chunk;
    chunk.name = chunkName;
    chunk.size = chunkSize;
    chunk.offset = tell();

    seek(chunk.size, Current);

    // check padding
    chunk.padding = 0;
    offset_t uPosNotPadded = tell();
    if(uPosNotPadded & 1) {
      ByteVector iByte = readBlock(1);
      if((iByte.size() != 1) || (iByte[0] != 0)) {
        // not well formed, re-seek
        seek(uPosNotPadded, Beginning);
      }
      else {
        chunk.padding = 1;
      }
    }
    d->chunks.push_back(chunk);

  }
}

void RIFF::File::writeChunk(const ByteVector &name, const ByteVector &data,
                            offset_t offset, TagLib::uint replace, TagLib::uint leadingPadding)
{
  ByteVector combined;
  if(leadingPadding) {
    combined.append(ByteVector(leadingPadding, '\x00'));
  }
  combined.append(name);

  if(d->endianness == BigEndian)
    combined.append(ByteVector::fromUInt32BE(data.size()));
  else
    combined.append(ByteVector::fromUInt32LE(data.size()));

  combined.append(data);
  if((data.size() & 0x01) != 0) {
    combined.append('\x00');
  }
  insert(combined, offset, replace);
}
