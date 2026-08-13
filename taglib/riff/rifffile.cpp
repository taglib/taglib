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

#include "rifffile.h"

#include <algorithm>
#include <vector>

#include "tdebug.h"
#include "riffutils.h"

using namespace TagLib;

namespace {

  constexpr int MAX_RIFF_CHUNK_COUNT = 50000;

}

struct Chunk
{
  ByteVector   name;
  offset_t offset;
  //! May exceed 32 bits for the "data" chunk of an RF64/BW64 file.
  offset_t size;
  unsigned int padding;
};

class RIFF::File::FilePrivate
{
public:
  FilePrivate(Endianness endianness) :
    endianness(endianness)
  {
  }

  Endianness endianness;

  unsigned int size { 0 };
  offset_t sizeOffset { 0 };

  //! An RF64 or BW64 file: the 32-bit size fields hold a 0xffffffff sentinel and
  //! the real sizes live in a leading "ds64" chunk.
  bool isLongForm { false };
  //! Offset of the "ds64" chunk data, or 0 if the file has none.
  offset_t ds64Offset { 0 };
  offset_t dataSize64 { 0 };

  std::vector<Chunk> chunks;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::File::~File() = default;

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

RIFF::File::File(FileName file, Endianness endianness) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(endianness))
{
  if(isOpen())
    read();
}

RIFF::File::File(IOStream *stream, Endianness endianness) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(endianness))
{
  if(isOpen())
    read();
}

unsigned int RIFF::File::riffSize() const
{
  return d->size;
}

unsigned int RIFF::File::chunkCount() const
{
  return static_cast<unsigned int>(d->chunks.size());
}

unsigned int RIFF::File::chunkDataSize(unsigned int i) const
{
  return static_cast<unsigned int>(std::min<offset_t>(chunkDataSize64(i), 0xffffffff));
}

offset_t RIFF::File::chunkDataSize64(unsigned int i) const
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkDataSize64() - Index out of range. Returning 0.");
    return 0;
  }

  return d->chunks[i].size;
}

offset_t RIFF::File::chunkOffset(unsigned int i) const
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkOffset() - Index out of range. Returning 0.");
    return 0;
  }

  return d->chunks[i].offset;
}

unsigned int RIFF::File::chunkPadding(unsigned int i) const
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkPadding() - Index out of range. Returning 0.");
    return 0;
  }

  return d->chunks[i].padding;
}

ByteVector RIFF::File::chunkName(unsigned int i) const
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkName() - Index out of range. Returning an empty vector.");
    return ByteVector();
  }

  return d->chunks[i].name;
}

ByteVector RIFF::File::chunkData(unsigned int i)
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::chunkData() - Index out of range. Returning an empty vector.");
    return ByteVector();
  }

  seek(d->chunks[i].offset);

  // A ByteVector is limited to 32 bits. The only chunk that can be larger is the
  // "data" chunk of an RF64 file, which no caller reads through this API.
  return readBlock(static_cast<size_t>(std::min<offset_t>(d->chunks[i].size, 0xffffffff)));
}

void RIFF::File::setChunkData(unsigned int i, const ByteVector &data)
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::setChunkData() - Index out of range.");
    return;
  }

  // Now update the specific chunk

  auto it = d->chunks.begin();
  std::advance(it, i);

  const long long originalSize = static_cast<long long>(it->size) + it->padding;

  writeChunk(it->name, data, it->offset - 8, it->size + it->padding + 8);

  it->size    = data.size();
  it->padding = data.size() % 2;

  const long long diff = static_cast<long long>(it->size) + it->padding - originalSize;

  // Now update the internal offsets

  it = std::next(it);
  while(it != d->chunks.end()) {
    it->offset += static_cast<int>(diff);
    ++it;
  }

  // Update the global size.

  updateGlobalSize();
}

void RIFF::File::setChunkData(const ByteVector &name, const ByteVector &data)
{
  setChunkData(name, data, false);
}

void RIFF::File::setChunkData(const ByteVector &name, const ByteVector &data, bool alwaysCreate)
{
  if(d->chunks.empty()) {
    debug("RIFF::File::setChunkData - No valid chunks found.");
    return;
  }

  if(alwaysCreate && name != "LIST") {
    debug("RIFF::File::setChunkData - alwaysCreate should be used for only \"LIST\" chunks.");
    return;
  }

  if(!alwaysCreate) {
    for(unsigned int i = 0; i < d->chunks.size(); i++) {
      if(d->chunks[i].name == name) {
        setChunkData(i, data);
        return;
      }
    }
  }

  // Couldn't find an existing chunk, so let's create a new one.

  // Adjust the padding of the last chunk to place the new chunk at even position.

  Chunk &last = d->chunks.back();

  offset_t offset = last.offset + last.size + last.padding;
  if(offset & 1) {
    if(last.padding == 1) {
      last.padding = 0; // This should not happen unless the file is corrupted.
      offset--;
      removeBlock(offset, 1);
    }
    else {
      insert(ByteVector("\0", 1), offset, 0);
      last.padding = 1;
      offset++;
    }
  }

  // Now add the chunk to the file.

  writeChunk(name, data, offset, 0);

  // And update our internal structure

  Chunk chunk;
  chunk.name    = name;
  chunk.size    = data.size();
  chunk.offset  = offset + 8;
  chunk.padding = data.size() % 2;

  d->chunks.push_back(std::move(chunk));

  // Update the global size.

  updateGlobalSize();
}

void RIFF::File::removeChunk(unsigned int i)
{
  if(i >= d->chunks.size()) {
    debug("RIFF::File::removeChunk() - Index out of range.");
    return;
  }

  auto it = d->chunks.begin();
  std::advance(it, i);

  const offset_t removeSize = it->size + it->padding + 8;
  removeBlock(it->offset - 8, static_cast<size_t>(removeSize));
  it = d->chunks.erase(it);

  while(it != d->chunks.end()) {
    it->offset -= removeSize;
    ++it;
  }

  // Update the global size.

  updateGlobalSize();
}

void RIFF::File::removeChunk(const ByteVector &name)
{
  for(int i = static_cast<int>(d->chunks.size()) - 1; i >= 0; --i) {
    if(d->chunks[i].name == name)
      removeChunk(i);
  }
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::File::read()
{
  const bool bigEndian = d->endianness == BigEndian;

  offset_t offset = tell();

  // RF64 and BW64 are the long forms of WAVE, used past 4 GB: the 32-bit size fields
  // hold a 0xffffffff sentinel and a leading "ds64" chunk carries the real sizes.
  // Both are little-endian, so AIFF never takes this path.
  if(!bigEndian) {
    seek(offset);
    const ByteVector magic = readBlock(4);
    d->isLongForm = magic == "RF64" || magic == "BW64";
  }

  offset += 4;
  d->sizeOffset = offset;

  seek(offset);
  d->size = readBlock(4).toUInt(bigEndian);

  offset += 8;

  // + 8: chunk header at least, fix for additional junk bytes
  while(offset + 8 <= length()) {

    if(d->chunks.size() >= MAX_RIFF_CHUNK_COUNT) {
      debug("RIFF::File::read() -- Maximum chunk count exceeded");
      setValid(false);
      return;
    }

    seek(offset);
    const ByteVector   chnkName = readBlock(4);
    const unsigned int declaredSize = readBlock(4).toUInt(bigEndian);

    if(!isValidChunkName(chnkName)) {
      debug("RIFF::File::read() -- Chunk '" + chnkName + "' has invalid ID");
      break;
    }

    // "ds64" is required to be the first chunk, so its sizes are known by the time
    // the "data" chunk is reached. Only the four fixed fields are read; the table of
    // additional oversized chunks that may follow them is not parsed, and any chunk
    // listed there stays on the clamping path below.
    if(d->isLongForm && chnkName == "ds64" && d->chunks.empty() && declaredSize >= 28) {
      seek(offset + 8);
      const ByteVector ds64 = readBlock(28);
      d->ds64Offset = offset + 8;
      d->dataSize64 = static_cast<offset_t>(ds64.toULongLong(8, bigEndian));
    }

    offset_t chunkSize = declaredSize;

    if(d->isLongForm && chnkName == "data" && declaredSize == 0xffffffff && d->dataSize64 > 0)
      chunkSize = d->dataSize64;

    if(offset + 8 + chunkSize > length()) {
      // Clamp to available bytes rather than rejecting the chunk outright.
      // Some encoders write a correct data chunk but with a slightly too-large
      // declared size, or place the data chunk outside the declared RIFF boundary.
      // Lenient parsers (ffmpeg, QuickTime) handle this by clamping; we do the same.
      debug("RIFF::File::read() -- Chunk '" + chnkName + "' is truncated; clamping size to available bytes.");
      chunkSize = length() - offset - 8;
    }

    Chunk chunk;
    chunk.name    = chnkName;
    chunk.size    = chunkSize;
    chunk.offset  = offset + 8;
    chunk.padding = 0;

    offset = chunk.offset + chunk.size;

    // Check padding

    if(offset & 1) {
      seek(offset);
      if(const ByteVector iByte = readBlock(1); iByte.size() == 1) {
        bool skipPadding = iByte[0] == '\0';
        if(!skipPadding) {
          // Padding byte is not zero, check if it is good to ignore it
          if(const ByteVector fourCcAfterPadding = readBlock(4);
             isValidChunkName(fourCcAfterPadding)) {
            // Use the padding, it is followed by a valid chunk name.
            skipPadding = true;
          }
        }
        if(skipPadding) {
          chunk.padding = 1;
          offset++;
        }
      }
    }

    d->chunks.push_back(std::move(chunk));
  }
}

void RIFF::File::writeChunk(const ByteVector &name, const ByteVector &data,
                            offset_t offset, unsigned long replace)
{
  ByteVector combined;

  combined.append(name);
  combined.append(ByteVector::fromUInt(data.size(), d->endianness == BigEndian));
  combined.append(data);

  if(data.size() & 1)
    combined.resize(combined.size() + 1, '\0');

  insert(combined, offset, replace);
}

void RIFF::File::updateGlobalSize()
{
  if(d->chunks.empty())
    return;

  const Chunk first = d->chunks.front();
  const Chunk last  = d->chunks.back();
  const offset_t totalSize = last.offset + last.size + last.padding - first.offset + 12;

  if(d->isLongForm) {
    // A long-form file always carries the sentinel here and its real size in "ds64"; any other
    // value is malformed. Writing it unconditionally also repairs a file whose sentinel an
    // older writer replaced with a real total, which past 4 GB is a truncated value that makes
    // readers stop consulting "ds64" and believe it instead.
    d->size = 0xffffffff;
    insert(ByteVector::fromUInt(d->size, d->endianness == BigEndian), d->sizeOffset, 4);

    // The "data" chunk's own size and "ds64"'s copy of it are left alone because no write path
    // here changes the audio.
    if(d->ds64Offset > 0)
      insert(ByteVector::fromULongLong(totalSize, d->endianness == BigEndian),
             d->ds64Offset, 8);

    return;
  }

  d->size = static_cast<unsigned int>(totalSize);

  const ByteVector data = ByteVector::fromUInt(d->size, d->endianness == BigEndian);
  insert(data, d->sizeOffset, 4);
}
