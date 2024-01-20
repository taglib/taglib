/***************************************************************************
    copyright            : (C) 2016 by Damien Plisson, Audirvana
    email                : damien78@audirvana.com
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

#include "dsdifffile.h"

#include <array>
#include <memory>

#include "tbytevector.h"
#include "tstringlist.h"
#include "tpropertymap.h"
#include "tdebug.h"
#include "id3v2tag.h"
#include "tagutils.h"
#include "tagunion.h"

using namespace TagLib;

namespace
{
  struct Chunk64
  {
    ByteVector name;
    unsigned long long offset;
    unsigned long long size;
    char padding;
  };

  using ChunkList = std::vector<Chunk64>;

  int chunkIndex(const ChunkList &chunks, const ByteVector &id)
  {
    for(size_t i = 0; i < chunks.size(); i++) {
      if(chunks[i].name == id)
        return static_cast<int>(i);
    }

    return -1;
  }

  bool isValidChunkID(const ByteVector &name)
  {
    if(name.size() != 4)
      return false;

    for(int i = 0; i < 4; i++) {
      if(name[i] < 32 || name[i] > 126)
        return false;
    }

    return true;
  }

  enum {
    ID3v2Index = 0,
    DIINIndex = 1
  };
  enum {
    PROPChunk = 0,
    DIINChunk = 1
  };
} // namespace

class DSDIFF::File::FilePrivate
{
public:
  FilePrivate(const ID3v2::FrameFactory *frameFactory)
        : ID3v2FrameFactory(frameFactory ? frameFactory
                                         : ID3v2::FrameFactory::instance())
  {
  }

  ~FilePrivate() = default;

  const ID3v2::FrameFactory *ID3v2FrameFactory;
  Endianness endianness { BigEndian };
  ByteVector type;
  unsigned long long size { 0 };
  ByteVector format;
  ChunkList chunks;
  std::array<ChunkList, 2> childChunks;
  std::array<int, 2> childChunkIndex { -1, -1 };
  /*
   * Two possibilities can be found: ID3V2 chunk inside PROP chunk or at root level
   */
  bool isID3InPropChunk { false };
  /*
   * ID3 chunks are present. This is then the index of the one in PROP chunk that
   * will be removed upon next save to remove duplicates.
   */
  int duplicateID3V2chunkIndex { -1 };

  std::unique_ptr<Properties> properties;

  TagUnion tag;

  ByteVector id3v2TagChunkID { "ID3 " };

  bool hasID3v2 { false };
  bool hasDiin { false };
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

bool DSDIFF::File::isSupported(IOStream *stream)
{
  // A DSDIFF file has to start with "FRM8????????DSD ".
  const ByteVector id = Utils::readHeader(stream, 16, false);
  return id.startsWith("FRM8") && id.containsAt("DSD ", 12);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSDIFF::File::File(FileName file, bool readProperties,
                   Properties::ReadStyle propertiesStyle,
                   ID3v2::FrameFactory *frameFactory) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSDIFF::File::File(IOStream *stream, bool readProperties,
                   Properties::ReadStyle propertiesStyle,
                   ID3v2::FrameFactory *frameFactory) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSDIFF::File::~File() = default;

TagLib::Tag *DSDIFF::File::tag() const
{
  return &d->tag;
}

ID3v2::Tag *DSDIFF::File::ID3v2Tag(bool create) const
{
  return d->tag.access<ID3v2::Tag>(ID3v2Index, create, d->ID3v2FrameFactory);
}

bool DSDIFF::File::hasID3v2Tag() const
{
  return d->hasID3v2;
}

DSDIFF::DIIN::Tag *DSDIFF::File::DIINTag(bool create) const
{
  return d->tag.access<DSDIFF::DIIN::Tag>(DIINIndex, create);
}

bool DSDIFF::File::hasDIINTag() const
{
  return d->hasDiin;
}

PropertyMap DSDIFF::File::properties() const
{
  return d->tag.properties();
}

void DSDIFF::File::removeUnsupportedProperties(const StringList &properties)
{
  d->tag.removeUnsupportedProperties(properties);
}

PropertyMap DSDIFF::File::setProperties(const PropertyMap &properties)
{
  return ID3v2Tag(true)->setProperties(properties);
}

DSDIFF::Properties *DSDIFF::File::audioProperties() const
{
  return d->properties.get();
}

bool DSDIFF::File::save()
{
  return save(AllTags);
}

bool DSDIFF::File::save(int tags, StripTags strip, ID3v2::Version version)
{
  if(readOnly()) {
    debug("DSDIFF::File::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    debug("DSDIFF::File::save() -- Trying to save invalid file.");
    return false;
  }

  if(strip == StripOthers)
    File::strip(~tags);

  // First: save ID3V2 chunk

  if(const ID3v2::Tag *id3v2Tag = ID3v2Tag(); (tags & ID3v2) && id3v2Tag) {
    if(d->isID3InPropChunk) {
      if(!id3v2Tag->isEmpty()) {
        setChildChunkData(d->id3v2TagChunkID, id3v2Tag->render(version), PROPChunk);
        d->hasID3v2 = true;
      }
      else {
        // Empty tag: remove it
        setChildChunkData(d->id3v2TagChunkID, ByteVector(), PROPChunk);
        d->hasID3v2 = false;
      }
    }
    else {
      if(!id3v2Tag->isEmpty()) {
        setRootChunkData(d->id3v2TagChunkID, id3v2Tag->render(version));
        d->hasID3v2 = true;
      }
      else {
        // Empty tag: remove it
        setRootChunkData(d->id3v2TagChunkID, ByteVector());
        d->hasID3v2 = false;
      }
    }
  }

  // Second: save the DIIN chunk

  if(const DSDIFF::DIIN::Tag *diinTag = DIINTag(); (tags & DIIN) && diinTag) {
    if(!diinTag->title().isEmpty()) {
      ByteVector diinTitle;
      diinTitle.append(ByteVector::fromUInt(diinTag->title().size(), d->endianness == BigEndian));
      diinTitle.append(ByteVector::fromCString(diinTag->title().toCString()));
      setChildChunkData("DITI", diinTitle, DIINChunk);
    }
    else
      setChildChunkData("DITI", ByteVector(), DIINChunk);

    if(!diinTag->artist().isEmpty()) {
      ByteVector diinArtist;
      diinArtist.append(ByteVector::fromUInt(diinTag->artist().size(), d->endianness == BigEndian));
      diinArtist.append(ByteVector::fromCString(diinTag->artist().toCString()));
      setChildChunkData("DIAR", diinArtist, DIINChunk);
    }
    else
      setChildChunkData("DIAR", ByteVector(), DIINChunk);
  }

  // Third: remove the duplicate ID3V2 chunk (inside PROP chunk) if any
  if(d->duplicateID3V2chunkIndex >= 0) {
    setChildChunkData(d->duplicateID3V2chunkIndex, ByteVector(), PROPChunk);
    d->duplicateID3V2chunkIndex = -1;
  }

  return true;
}

void DSDIFF::File::strip(int tags)
{
  if(tags & ID3v2) {
    removeRootChunk("ID3 ");
    removeRootChunk("id3 ");
    removeChildChunk("ID3 ", PROPChunk);
    removeChildChunk("id3 ", PROPChunk);
    d->hasID3v2 = false;
    d->tag.set(ID3v2Index, new ID3v2::Tag(nullptr, 0, d->ID3v2FrameFactory));
    d->duplicateID3V2chunkIndex = -1;
    d->isID3InPropChunk = false;
    d->id3v2TagChunkID.setData("ID3 ");
  }
  if(tags & DIIN) {
    removeChildChunk("DITI", DIINChunk);
    removeChildChunk("DIAR", DIINChunk);
    if(d->childChunks[DIINIndex].empty()) {
      removeRootChunk("DIIN");
    }

    d->hasDiin = false;
    d->tag.set(DIINIndex, new DIIN::Tag);
  }
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DSDIFF::File::removeRootChunk(unsigned int i)
{
    unsigned long long chunkSize = d->chunks[i].size + d->chunks[i].padding + 12;

    d->size -= chunkSize;
    insert(ByteVector::fromLongLong(d->size, d->endianness == BigEndian), 4, 8);

    removeBlock(d->chunks[i].offset - 12, chunkSize);

    // Update the internal offsets

    d->chunks.erase(d->chunks.begin() + i);
    for(int &cci : d->childChunkIndex) {
      if(cci > static_cast<int>(i)) {
        --cci;
      }
    }
    updateRootChunksStructure(i);
}

void DSDIFF::File::removeRootChunk(const ByteVector &id)
{
  int i = chunkIndex(d->chunks, id);

  if(i >= 0)
    removeRootChunk(i);
}

void DSDIFF::File::setRootChunkData(unsigned int i, const ByteVector &data)
{
  if(data.isEmpty()) {
    removeRootChunk(i);
    return;
  }

  // Non null data: update chunk
  // First we update the global size

  d->size += ((data.size() + 1) & ~1) - (d->chunks[i].size + d->chunks[i].padding);
  insert(ByteVector::fromLongLong(d->size, d->endianness == BigEndian), 4, 8);

  // Now update the specific chunk

  writeChunk(d->chunks[i].name,
             data,
             d->chunks[i].offset - 12,
             static_cast<unsigned long>(d->chunks[i].size + d->chunks[i].padding + 12));

  d->chunks[i].size = data.size();
  d->chunks[i].padding = (data.size() & 0x01) ? 1 : 0;

  // Finally update the internal offsets

  updateRootChunksStructure(i + 1);
}

void DSDIFF::File::setRootChunkData(const ByteVector &name, const ByteVector &data)
{
  if(d->chunks.empty()) {
    debug("DSDIFF::File::setRootChunkData('" + name + "') - No valid chunks found.");
    return;
  }

  int i = chunkIndex(d->chunks, name);

  if(i >= 0) {
    setRootChunkData(i, data);
    return;
  }

  // Couldn't find an existing chunk, so let's create a new one.
  i = static_cast<int>(d->chunks.size()) - 1;
  unsigned long long offset = d->chunks[i].offset + d->chunks[i].size + d->chunks[i].padding;

  // First we update the global size
  d->size += (offset & 1) + ((data.size() + 1) & ~1) + 12;
  insert(ByteVector::fromLongLong(d->size, d->endianness == BigEndian), 4, 8);

  // Now add the chunk to the file
  const unsigned long long fileLength = length();
  writeChunk(name,
             data,
             offset,
             static_cast<unsigned long>(fileLength > offset ? fileLength - offset : 0),
             (offset & 1) ? 1 : 0);

  Chunk64 chunk;
  chunk.name = name;
  chunk.size = data.size();
  chunk.offset = offset + 12;
  chunk.padding = (data.size() & 0x01) ? 1 : 0;

  d->chunks.push_back(chunk);
}

void DSDIFF::File::removeChildChunk(unsigned int i, unsigned int childChunkNum)
{
    ChunkList &childChunks = d->childChunks[childChunkNum];

    // Update global size

    unsigned long long removedChunkTotalSize = childChunks[i].size + childChunks[i].padding + 12;
    d->size -= removedChunkTotalSize;
    insert(ByteVector::fromLongLong(d->size, d->endianness == BigEndian), 4, 8);

    // Update child chunk size

    d->chunks[d->childChunkIndex[childChunkNum]].size -= removedChunkTotalSize;
    insert(ByteVector::fromLongLong(d->chunks[d->childChunkIndex[childChunkNum]].size,
                                    d->endianness == BigEndian),
           d->chunks[d->childChunkIndex[childChunkNum]].offset - 8, 8);
    // Remove the chunk

    removeBlock(childChunks[i].offset - 12, removedChunkTotalSize);

    // Update the internal offsets
    // For child chunks

    if(i + 1 < childChunks.size()) {
      childChunks[i + 1].offset = childChunks[i].offset;
      for(unsigned int c = i + 2; c < childChunks.size(); ++c)
        childChunks[c].offset = childChunks[c - 1].offset + 12
          + childChunks[c - 1].size + childChunks[c - 1].padding;
    }

    // And for root chunks

    childChunks.erase(childChunks.begin() + i);
    updateRootChunksStructure(d->childChunkIndex[childChunkNum] + 1);
}

void DSDIFF::File::removeChildChunk(const ByteVector &id, unsigned int childChunkNum)
{
  int i = chunkIndex(d->childChunks[childChunkNum], id);

  if(i >= 0)
    removeChildChunk(i, childChunkNum);
}

void DSDIFF::File::setChildChunkData(unsigned int i,
                                     const ByteVector &data,
                                     unsigned int childChunkNum)
{
  ChunkList &childChunks = d->childChunks[childChunkNum];

  if(data.isEmpty()) {
    removeChildChunk(i, childChunkNum);
    return;
  }

  // Non null data: update chunk
  // First we update the global size

  d->size += ((data.size() + 1) & ~1) - (childChunks[i].size + childChunks[i].padding);

  insert(ByteVector::fromLongLong(d->size, d->endianness == BigEndian), 4, 8);

  // And the PROP chunk size

  d->chunks[d->childChunkIndex[childChunkNum]].size +=
    ((data.size() + 1) & ~1) - (childChunks[i].size + childChunks[i].padding);
  insert(ByteVector::fromLongLong(d->chunks[d->childChunkIndex[childChunkNum]].size,
                                  d->endianness == BigEndian),
         d->chunks[d->childChunkIndex[childChunkNum]].offset - 8, 8);

  // Now update the specific chunk

  writeChunk(childChunks[i].name,
             data,
             childChunks[i].offset - 12,
             static_cast<unsigned long>(childChunks[i].size + childChunks[i].padding + 12));

  childChunks[i].size = data.size();
  childChunks[i].padding = (data.size() & 0x01) ? 1 : 0;

  // Now update the internal offsets
  // For child Chunks
  for(i++; i < childChunks.size(); i++)
    childChunks[i].offset = childChunks[i - 1].offset + 12
                            + childChunks[i - 1].size + childChunks[i - 1].padding;

  // And for root chunks
  updateRootChunksStructure(d->childChunkIndex[childChunkNum] + 1);
}

void DSDIFF::File::setChildChunkData(const ByteVector &name,
                                     const ByteVector &data,
                                     unsigned int childChunkNum)
{
  ChunkList &childChunks = d->childChunks[childChunkNum];

  if(int i = chunkIndex(childChunks, name); i >= 0) {
    setChildChunkData(i, data, childChunkNum);
    return;
  }

  // Do not attempt to remove a non existing chunk

  if(data.isEmpty())
    return;

  // Couldn't find an existing chunk, so let's create a new one.

  unsigned long long offset = 0;
  if(!childChunks.empty()) {
    size_t i = childChunks.size() - 1;
    offset = childChunks[i].offset + childChunks[i].size + childChunks[i].padding;
  }
  else if(childChunkNum == DIINChunk) {
    int i = d->childChunkIndex[DIINChunk];
    if(i < 0) {
      setRootChunkData("DIIN", ByteVector());
      if(const int lastChunkIndex = static_cast<int>(d->chunks.size()) - 1;
         lastChunkIndex >= 0 && d->chunks[lastChunkIndex].name == "DIIN") {
        i = lastChunkIndex;
        d->childChunkIndex[DIINChunk] = lastChunkIndex;
        d->hasDiin = true;
      }
    }
    if(i >= 0) {
      offset = d->chunks[i].offset; // 12 is already added in setRootChunkData()
    }
  }
  if(offset == 0) {
    debug("DSDIFF::File::setChildChunkData - No valid chunks found.");
    return;
  }

  // First we update the global size

  d->size += (offset & 1) + ((data.size() + 1) & ~1) + 12;
  insert(ByteVector::fromLongLong(d->size, d->endianness == BigEndian), 4, 8);

  // And the child chunk size

  d->chunks[d->childChunkIndex[childChunkNum]].size += (offset & 1)
    + ((data.size() + 1) & ~1) + 12;
  insert(ByteVector::fromLongLong(d->chunks[d->childChunkIndex[childChunkNum]].size,
                                  d->endianness == BigEndian),
         d->chunks[d->childChunkIndex[childChunkNum]].offset - 8, 8);

  // Now add the chunk to the file

  unsigned long long nextRootChunkIdx = length();
  if(d->childChunkIndex[childChunkNum] + 1 < static_cast<int>(d->chunks.size()))
    nextRootChunkIdx = d->chunks[d->childChunkIndex[childChunkNum] + 1].offset - 12;

  writeChunk(name, data, offset,
             static_cast<unsigned long>(
                nextRootChunkIdx > offset ? nextRootChunkIdx - offset : 0),
             offset & 1 ? 1 : 0);

  // For root chunks

  updateRootChunksStructure(d->childChunkIndex[childChunkNum] + 1);

  Chunk64 chunk;
  chunk.name = name;
  chunk.size = data.size();
  chunk.offset = offset + 12;
  chunk.padding = data.size() & 0x01 ? 1 : 0;

  childChunks.push_back(chunk);
}

void DSDIFF::File::updateRootChunksStructure(unsigned int startingChunk)
{
  for(unsigned int i = startingChunk; i < d->chunks.size(); i++)
    d->chunks[i].offset = d->chunks[i - 1].offset + 12
      + d->chunks[i - 1].size + d->chunks[i - 1].padding;

  // Update child chunks structure as well

  if(d->childChunkIndex[PROPChunk] >= static_cast<int>(startingChunk)) {
    if(ChunkList &childChunksToUpdate = d->childChunks[PROPChunk];
       !childChunksToUpdate.empty()) {
      childChunksToUpdate[0].offset = d->chunks[d->childChunkIndex[PROPChunk]].offset + 12;
      for(unsigned int i = 1; i < childChunksToUpdate.size(); i++)
        childChunksToUpdate[i].offset = childChunksToUpdate[i - 1].offset + 12
          + childChunksToUpdate[i - 1].size + childChunksToUpdate[i - 1].padding;
    }

  }
  if(d->childChunkIndex[DIINChunk] >= static_cast<int>(startingChunk)) {
    if(ChunkList &childChunksToUpdate = d->childChunks[DIINChunk];
       !childChunksToUpdate.empty()) {
      childChunksToUpdate[0].offset = d->chunks[d->childChunkIndex[DIINChunk]].offset + 12;
      for(unsigned int i = 1; i < childChunksToUpdate.size(); i++)
        childChunksToUpdate[i].offset = childChunksToUpdate[i - 1].offset + 12
          + childChunksToUpdate[i - 1].size + childChunksToUpdate[i - 1].padding;
    }
  }
}

void DSDIFF::File::read(bool readProperties, Properties::ReadStyle propertiesStyle)
{
  bool bigEndian = d->endianness == BigEndian;

  d->type = readBlock(4);
  d->size = readBlock(8).toLongLong(bigEndian);
  d->format = readBlock(4);

  // + 12: chunk header at least, fix for additional junk bytes

  while(tell() + 12 <= length()) {
    ByteVector chunkName = readBlock(4);
    unsigned long long chunkSize = readBlock(8).toLongLong(bigEndian);

    if(!isValidChunkID(chunkName)) {
      debug("DSDIFF::File::read() -- Chunk '" + chunkName + "' has invalid ID");
      setValid(false);
      break;
    }

    if(static_cast<unsigned long long>(tell()) + chunkSize >
       static_cast<unsigned long long>(length())) {
      debug("DSDIFF::File::read() -- Chunk '" + chunkName
            + "' has invalid size (larger than the file size)");
      setValid(false);
      break;
    }

    Chunk64 chunk;
    chunk.name = chunkName;
    chunk.size = chunkSize;
    chunk.offset = tell();

    seek(chunk.size, Current);

    // Check padding

    chunk.padding = 0;
    if(offset_t uPosNotPadded = tell(); (uPosNotPadded & 0x01) != 0) {
      if(ByteVector iByte = readBlock(1);
         iByte.size() != 1 || iByte[0] != 0)
        // Not well formed, re-seek
        seek(uPosNotPadded, Beginning);
      else
        chunk.padding = 1;
    }
    d->chunks.push_back(chunk);
  }

  // For DSD uncompressed
  unsigned long long lengthDSDSamplesTimeChannels = 0;
  // For computing bitrate
  unsigned long long audioDataSizeinBytes = 0;
  // For DST compressed frames
  unsigned long dstNumFrames = 0;
  // For DST compressed frames
  unsigned short dstFrameRate = 0;

  for(unsigned int i = 0; i < d->chunks.size(); i++) {
    if(d->chunks[i].name == "DSD ") {
      lengthDSDSamplesTimeChannels = d->chunks[i].size * 8;
      audioDataSizeinBytes = d->chunks[i].size;
    }
    else if(d->chunks[i].name == "DST ") {
      // Now decode the chunks inside the DST chunk to read the DST Frame Information one
      long long dstChunkEnd = d->chunks[i].offset + d->chunks[i].size;
      seek(d->chunks[i].offset);

      audioDataSizeinBytes = d->chunks[i].size;

      while(tell() + 12 <= dstChunkEnd) {
        ByteVector dstChunkName = readBlock(4);
        long long dstChunkSize = readBlock(8).toLongLong(bigEndian);

        if(!isValidChunkID(dstChunkName)) {
          debug("DSDIFF::File::read() -- DST Chunk '" + dstChunkName + "' has invalid ID");
          setValid(false);
          break;
        }

        if(static_cast<long long>(tell()) + dstChunkSize > dstChunkEnd) {
          debug("DSDIFF::File::read() -- DST Chunk '" + dstChunkName
                + "' has invalid size (larger than the DST chunk)");
          setValid(false);
          break;
        }

        if(dstChunkName == "FRTE") {
          // Found the DST frame information chunk
          dstNumFrames = readBlock(4).toUInt(bigEndian);
          dstFrameRate = readBlock(2).toUShort(bigEndian);
          // Found the wanted one, no need to look at the others
          break;
        }

        seek(dstChunkSize, Current);

        // Check padding
        if(offset_t uPosNotPadded = tell(); (uPosNotPadded & 0x01) != 0) {
          if(ByteVector iByte = readBlock(1);
             iByte.size() != 1 || iByte[0] != 0)
            // Not well formed, re-seek
            seek(uPosNotPadded, Beginning);
        }
      }
    }
    else if(d->chunks[i].name == "PROP") {
      d->childChunkIndex[PROPChunk] = i;
      // Now decodes the chunks inside the PROP chunk
      long long propChunkEnd = d->chunks[i].offset + d->chunks[i].size;
      // +4 to remove the 'SND ' marker at beginning of 'PROP' chunk
      seek(d->chunks[i].offset + 4);
      while(tell() + 12 <= propChunkEnd) {
        ByteVector propChunkName = readBlock(4);
        long long propChunkSize = readBlock(8).toLongLong(bigEndian);

        if(!isValidChunkID(propChunkName)) {
          debug("DSDIFF::File::read() -- PROP Chunk '" + propChunkName + "' has invalid ID");
          setValid(false);
          break;
        }

        if(static_cast<long long>(tell()) + propChunkSize > propChunkEnd) {
          debug("DSDIFF::File::read() -- PROP Chunk '" + propChunkName
                + "' has invalid size (larger than the PROP chunk)");
          setValid(false);
          break;
        }

        Chunk64 chunk;
        chunk.name = propChunkName;
        chunk.size = propChunkSize;
        chunk.offset = tell();

        seek(chunk.size, Current);

        // Check padding
        chunk.padding = 0;
        if(offset_t uPosNotPadded = tell(); (uPosNotPadded & 0x01) != 0) {
          if(ByteVector iByte = readBlock(1);
             iByte.size() != 1 || iByte[0] != 0)
            // Not well formed, re-seek
            seek(uPosNotPadded, Beginning);
          else
            chunk.padding = 1;
        }
        d->childChunks[PROPChunk].push_back(chunk);
      }
    }
    else if(d->chunks[i].name == "DIIN") {
      d->childChunkIndex[DIINChunk] = i;
      d->hasDiin = true;

      // Now decode the chunks inside the DIIN chunk

      long long diinChunkEnd = d->chunks[i].offset + d->chunks[i].size;
      seek(d->chunks[i].offset);

      while(tell() + 12 <= diinChunkEnd) {
        ByteVector diinChunkName = readBlock(4);
        long long diinChunkSize = readBlock(8).toLongLong(bigEndian);

        if(!isValidChunkID(diinChunkName)) {
          debug("DSDIFF::File::read() -- DIIN Chunk '" + diinChunkName + "' has invalid ID");
          setValid(false);
          break;
        }

        if(static_cast<long long>(tell()) + diinChunkSize > diinChunkEnd) {
          debug("DSDIFF::File::read() -- DIIN Chunk '" + diinChunkName
                + "' has invalid size (larger than the DIIN chunk)");
          setValid(false);
          break;
        }

        Chunk64 chunk;
        chunk.name = diinChunkName;
        chunk.size = diinChunkSize;
        chunk.offset = tell();

        seek(chunk.size, Current);

        // Check padding

        chunk.padding = 0;

        if(offset_t uPosNotPadded = tell(); (uPosNotPadded & 0x01) != 0) {
          if(ByteVector iByte = readBlock(1);
             iByte.size() != 1 || iByte[0] != 0)
            // Not well formed, re-seek
            seek(uPosNotPadded, Beginning);
          else
            chunk.padding = 1;
        }
        d->childChunks[DIINChunk].push_back(chunk);
      }
    }
    else if(d->chunks[i].name == "ID3 " || d->chunks[i].name == "id3 ") {
      d->id3v2TagChunkID = d->chunks[i].name;
      d->tag.set(ID3v2Index, new ID3v2::Tag(this, d->chunks[i].offset,
                                            d->ID3v2FrameFactory));
      d->isID3InPropChunk = false;
      d->hasID3v2 = true;
    }
  }

  if(!isValid())
    return;

  if(d->childChunkIndex[PROPChunk] < 0) {
    debug("DSDIFF::File::read() -- no PROP chunk found");
    setValid(false);
    return;
  }

  // Read properties

  unsigned int sampleRate = 0;
  unsigned short channels = 0;

  for(unsigned int i = 0; i < d->childChunks[PROPChunk].size(); i++) {
    if(d->childChunks[PROPChunk][i].name == "ID3 " ||
       d->childChunks[PROPChunk][i].name == "id3 ") {
      if(d->hasID3v2) {
        d->duplicateID3V2chunkIndex = i;
        // ID3V2 tag has already been found at root level
        continue;
      }
      d->id3v2TagChunkID = d->childChunks[PROPChunk][i].name;
      d->tag.set(ID3v2Index, new ID3v2::Tag(this, d->childChunks[PROPChunk][i].offset,
                                            d->ID3v2FrameFactory));
      d->isID3InPropChunk = true;
      d->hasID3v2 = true;
    }
    else if(d->childChunks[PROPChunk][i].name == "FS  ") {
      // Sample rate
      seek(d->childChunks[PROPChunk][i].offset);
      sampleRate = readBlock(4).toUInt(0, 4, bigEndian);
    }
    else if(d->childChunks[PROPChunk][i].name == "CHNL") {
      // Channels
      seek(d->childChunks[PROPChunk][i].offset);
      channels = readBlock(2).toShort(0, bigEndian);
    }
  }

  // Read title & artist from DIIN chunk

  d->tag.access<DSDIFF::DIIN::Tag>(DIINIndex, true);

  if(d->hasDiin) {
    for(unsigned int i = 0; i < d->childChunks[DIINChunk].size(); i++) {
      if(d->childChunks[DIINChunk][i].name == "DITI") {
        seek(d->childChunks[DIINChunk][i].offset);
        if(unsigned int titleStrLength = readBlock(4).toUInt(0, 4, bigEndian);
           titleStrLength <= d->childChunks[DIINChunk][i].size) {
          ByteVector titleStr = readBlock(titleStrLength);
          d->tag.access<DSDIFF::DIIN::Tag>(DIINIndex, false)->setTitle(titleStr);
        }
      }
      else if(d->childChunks[DIINChunk][i].name == "DIAR") {
        seek(d->childChunks[DIINChunk][i].offset);
        if(unsigned int artistStrLength = readBlock(4).toUInt(0, 4, bigEndian);
           artistStrLength <= d->childChunks[DIINChunk][i].size) {
          ByteVector artistStr = readBlock(artistStrLength);
          d->tag.access<DSDIFF::DIIN::Tag>(DIINIndex, false)->setArtist(artistStr);
        }
      }
    }
  }

  if(readProperties) {
    if(lengthDSDSamplesTimeChannels == 0) {
      // DST compressed signal : need to compute length of DSD uncompressed frames
      if(dstFrameRate > 0)
        lengthDSDSamplesTimeChannels = static_cast<unsigned long long>(dstNumFrames) *
                                       static_cast<unsigned long long>(sampleRate) /
                                       static_cast<unsigned long long>(dstFrameRate);
      else
        lengthDSDSamplesTimeChannels = 0;
    }
    else {
      // In DSD uncompressed files, the read number of samples is the total for each channel
      if(channels > 0)
        lengthDSDSamplesTimeChannels /= channels;
    }
    int bitrate = 0;
    if(lengthDSDSamplesTimeChannels > 0)
      bitrate = static_cast<int>(
        audioDataSizeinBytes * 8 * sampleRate / lengthDSDSamplesTimeChannels / 1000);

    d->properties = std::make_unique<Properties>(sampleRate, channels,
      lengthDSDSamplesTimeChannels, bitrate, propertiesStyle);
  }

  if(!ID3v2Tag()) {
    d->tag.access<ID3v2::Tag>(ID3v2Index, true, d->ID3v2FrameFactory);
    // By default, ID3 chunk is at root level
    d->isID3InPropChunk = false;
    d->hasID3v2 = false;
  }
}

void DSDIFF::File::writeChunk(const ByteVector &name, const ByteVector &data,
                              unsigned long long offset, unsigned long replace,
                              unsigned int leadingPadding)
{
  ByteVector combined;
  if(leadingPadding)
    combined.append(ByteVector(leadingPadding, '\x00'));

  combined.append(name);
  combined.append(ByteVector::fromLongLong(data.size(), d->endianness == BigEndian));
  combined.append(data);
  if((data.size() & 0x01) != 0)
    combined.append('\x00');

  insert(combined, offset, replace);
}
