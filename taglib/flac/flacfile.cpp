/***************************************************************************
    copyright            : (C) 2003-2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
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
#include <tstring.h>
#include <tlist.h>
#include <tdebug.h>
#include <tagunion.h>
#include <tpropertymap.h>
#include <tagutils.h>

#include <id3v2header.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include <xiphcomment.h>

#include "flacpicture.h"
#include "flacfile.h"
#include "flacmetadatablock.h"
#include "flacunknownmetadatablock.h"

using namespace TagLib;

namespace
{
  enum { FlacXiphIndex = 0, FlacID3v2Index = 1, FlacID3v1Index = 2 };
  enum { MinPaddingLength = 4096 };
  enum { LastBlockFlag = 0x80 };
}

class FLAC::File::FilePrivate
{
public:
  FilePrivate() :
    ID3v2FrameFactory(ID3v2::FrameFactory::instance()),
    ID3v2Location(-1),
    ID3v2OriginalSize(0),
    ID3v1Location(-1),
    properties(0),
    flacStart(0),
    streamStart(0),
    scanned(false),
    hasXiphComment(false),
    hasID3v2(false),
    hasID3v1(false)
  {
  }

  ~FilePrivate()
  {
    uint size = blocks.size();
    for(uint i = 0; i < size; i++) {
      delete blocks[i];
    }
    delete properties;
  }

  const ID3v2::FrameFactory *ID3v2FrameFactory;
  long ID3v2Location;
  uint ID3v2OriginalSize;

  long ID3v1Location;

  TagUnion tag;

  Properties *properties;
  ByteVector xiphCommentData;
  List<MetadataBlock *> blocks;

  long flacStart;
  long streamStart;
  bool scanned;

  bool hasXiphComment;
  bool hasID3v2;
  bool hasID3v1;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FLAC::File::File(FileName file, bool readProperties, Properties::ReadStyle) :
  TagLib::File(file),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties);
}

FLAC::File::File(FileName file, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle) :
  TagLib::File(file),
  d(new FilePrivate())
{
  d->ID3v2FrameFactory = frameFactory;
  if(isOpen())
    read(readProperties);
}

FLAC::File::File(IOStream *stream, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle) :
  TagLib::File(stream),
  d(new FilePrivate())
{
  d->ID3v2FrameFactory = frameFactory;
  if(isOpen())
    read(readProperties);
}

FLAC::File::~File()
{
  delete d;
}

TagLib::Tag *FLAC::File::tag() const
{
  return &d->tag;
}

PropertyMap FLAC::File::properties() const
{
  return d->tag.properties();
}

void FLAC::File::removeUnsupportedProperties(const StringList &unsupported)
{
  d->tag.removeUnsupportedProperties(unsupported);
}

PropertyMap FLAC::File::setProperties(const PropertyMap &properties)
{
  return xiphComment(true)->setProperties(properties);
}

FLAC::Properties *FLAC::File::audioProperties() const
{
  return d->properties;
}

bool FLAC::File::save()
{
  if(readOnly()) {
    debug("FLAC::File::save() - Cannot save to a read only file.");
    return false;
  }

  if(!isValid()) {
    debug("FLAC::File::save() -- Trying to save invalid file.");
    return false;
  }

  // Create new vorbis comments

  Tag::duplicate(&d->tag, xiphComment(true), false);

  d->xiphCommentData = xiphComment()->render(false);

  // Replace metadata blocks

  bool foundVorbisCommentBlock = false;
  List<MetadataBlock *> newBlocks;
  for(uint i = 0; i < d->blocks.size(); i++) {
    MetadataBlock *block = d->blocks[i];
    if(block->code() == MetadataBlock::VorbisComment) {
      // Set the new Vorbis Comment block
      delete block;
      block = new UnknownMetadataBlock(MetadataBlock::VorbisComment, d->xiphCommentData);
      foundVorbisCommentBlock = true;
    }
    if(block->code() == MetadataBlock::Padding) {
      delete block;
      continue;
    }
    newBlocks.append(block);
  }
  if(!foundVorbisCommentBlock) {
    newBlocks.append(new UnknownMetadataBlock(MetadataBlock::VorbisComment, d->xiphCommentData));
    foundVorbisCommentBlock = true;
  }
  d->blocks = newBlocks;

  // Render data for the metadata blocks

  ByteVector data;
  for(uint i = 0; i < newBlocks.size(); i++) {
    FLAC::MetadataBlock *block = newBlocks[i];
    ByteVector blockData = block->render();
    ByteVector blockHeader = ByteVector::fromUInt(blockData.size());
    blockHeader[0] = block->code();
    data.append(blockHeader);
    data.append(blockData);
  }

  // Adjust the padding block(s)

  long originalLength = d->streamStart - d->flacStart;
  int paddingLength = originalLength - data.size() - 4;
  if(paddingLength <= 0) {
    paddingLength = MinPaddingLength;
  }
  ByteVector padding = ByteVector::fromUInt(paddingLength);
  padding.resize(paddingLength + 4);
  padding[0] = (char)(FLAC::MetadataBlock::Padding | LastBlockFlag);
  data.append(padding);

  // Write the data to the file

  insert(data, d->flacStart, originalLength);
  d->hasXiphComment = true;

  // Update ID3 tags

  if(ID3v2Tag()) {
    if(d->hasID3v2) {
      if(d->ID3v2Location < d->flacStart)
        debug("FLAC::File::save() -- This can't be right -- an ID3v2 tag after the "
              "start of the FLAC bytestream?  Not writing the ID3v2 tag.");
      else
        insert(ID3v2Tag()->render(), d->ID3v2Location, d->ID3v2OriginalSize);
    }
    else
      insert(ID3v2Tag()->render(), 0, 0);
  }

  if(ID3v1Tag()) {
    if(d->hasID3v1) {
      seek(d->ID3v1Location);
    }
    else {
      seek(0, End);
      d->ID3v1Location = tell();
    }

    writeBlock(ID3v1Tag()->render());
    d->hasID3v1 = true;
  }

  return true;
}

ID3v2::Tag *FLAC::File::ID3v2Tag(bool create)
{
  if(!create || d->tag[FlacID3v2Index])
    return static_cast<ID3v2::Tag *>(d->tag[FlacID3v2Index]);

  d->tag.set(FlacID3v2Index, new ID3v2::Tag);
  return static_cast<ID3v2::Tag *>(d->tag[FlacID3v2Index]);
}

ID3v1::Tag *FLAC::File::ID3v1Tag(bool create)
{
  return d->tag.access<ID3v1::Tag>(FlacID3v1Index, create);
}

Ogg::XiphComment *FLAC::File::xiphComment(bool create)
{
  return d->tag.access<Ogg::XiphComment>(FlacXiphIndex, create);
}

void FLAC::File::setID3v2FrameFactory(const ID3v2::FrameFactory *factory)
{
  d->ID3v2FrameFactory = factory;
}

ByteVector FLAC::File::streamInfoData()
{
  debug("FLAC::File::streamInfoData() -- This function is obsolete. Returning an empty ByteVector.");
  return ByteVector();
}

long FLAC::File::streamLength()
{
  debug("FLAC::File::streamLength() -- This function is obsolete. Returning zero.");
  return 0;
}

List<FLAC::Picture *> FLAC::File::pictureList()
{
  List<Picture *> pictures;
  for(uint i = 0; i < d->blocks.size(); i++) {
    Picture *picture = dynamic_cast<Picture *>(d->blocks[i]);
    if(picture) {
      pictures.append(picture);
    }
  }
  return pictures;
}

void FLAC::File::addPicture(Picture *picture)
{
  d->blocks.append(picture);
}

void FLAC::File::removePicture(Picture *picture, bool del)
{
  MetadataBlock *block = picture;
  List<MetadataBlock *>::Iterator it = d->blocks.find(block);
  if(it != d->blocks.end())
    d->blocks.erase(it);

  if(del)
    delete picture;
}

void FLAC::File::removePictures()
{
  List<MetadataBlock *> newBlocks;
  for(uint i = 0; i < d->blocks.size(); i++) {
    Picture *picture = dynamic_cast<Picture *>(d->blocks[i]);
    if(picture) {
      delete picture;
    }
    else {
      newBlocks.append(d->blocks[i]);
    }
  }
  d->blocks = newBlocks;
}

bool FLAC::File::hasXiphComment() const
{
  return d->hasXiphComment;
}

bool FLAC::File::hasID3v1Tag() const
{
  return d->hasID3v1;
}

bool FLAC::File::hasID3v2Tag() const
{
  return d->hasID3v2;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void FLAC::File::read(bool readProperties)
{
  // Look for an ID3v2 tag

  d->ID3v2Location = Utils::findID3v2(this);

  if(d->ID3v2Location >= 0) {

    d->tag.set(FlacID3v2Index, new ID3v2::Tag(this, d->ID3v2Location, d->ID3v2FrameFactory));

    d->ID3v2OriginalSize = ID3v2Tag()->header()->completeTagSize();

    if(ID3v2Tag()->header()->tagSize() <= 0)
      d->tag.set(FlacID3v2Index, 0);
    else
      d->hasID3v2 = true;
  }

  // Look for an ID3v1 tag

  d->ID3v1Location = Utils::findID3v1(this);

  if(d->ID3v1Location >= 0) {
    d->tag.set(FlacID3v1Index, new ID3v1::Tag(this, d->ID3v1Location));
    d->hasID3v1 = true;
  }

  // Look for FLAC metadata, including vorbis comments

  scan();

  if(!isValid())
    return;

  if(d->hasXiphComment)
    d->tag.set(FlacXiphIndex, new Ogg::XiphComment(d->xiphCommentData));
  else
    d->tag.set(FlacXiphIndex, new Ogg::XiphComment);

  if(readProperties) {

    // First block should be the stream_info metadata

    const ByteVector infoData = d->blocks.front()->render();

    long streamLength;

    if(d->hasID3v1)
      streamLength = d->ID3v1Location - d->streamStart;
    else
      streamLength = File::length() - d->streamStart;

    d->properties = new Properties(infoData, streamLength);
  }
}

void FLAC::File::scan()
{
  // Scan the metadata pages

  if(d->scanned)
    return;

  if(!isValid())
    return;

  long nextBlockOffset;

  if(d->hasID3v2)
    nextBlockOffset = find("fLaC", d->ID3v2Location + d->ID3v2OriginalSize);
  else
    nextBlockOffset = find("fLaC");

  if(nextBlockOffset < 0) {
    debug("FLAC::File::scan() -- FLAC stream not found");
    setValid(false);
    return;
  }

  nextBlockOffset += 4;
  d->flacStart = nextBlockOffset;

  seek(nextBlockOffset);

  ByteVector header = readBlock(4);

  // Header format (from spec):
  // <1> Last-metadata-block flag
  // <7> BLOCK_TYPE
  //    0 : STREAMINFO
  //    1 : PADDING
  //    ..
  //    4 : VORBIS_COMMENT
  //    ..
  // <24> Length of metadata to follow

  char blockType = header[0] & 0x7f;
  bool isLastBlock = (header[0] & 0x80) != 0;
  uint length = header.toUInt(1U, 3U);

  // First block should be the stream_info metadata

  if(blockType != MetadataBlock::StreamInfo) {
    debug("FLAC::File::scan() -- invalid FLAC stream");
    setValid(false);
    return;
  }

  d->blocks.append(new UnknownMetadataBlock(blockType, readBlock(length)));
  nextBlockOffset += length + 4;

  int blockCount = 0;

  // Search through the remaining metadata
  while(!isLastBlock) {

    if (++blockCount > 1024) {
      debug("FLAC::File::scan() -- FLAC stream has more than 1024 metadata "
            "blocks, probably corrupt.");
      setValid(false);
      return;
    }

    header = readBlock(4);
    blockType = header[0] & 0x7f;
    isLastBlock = (header[0] & 0x80) != 0;
    length = header.toUInt(1U, 3U);

    if(length == 0 && blockType != MetadataBlock::Padding) {
      debug("FLAC::File::scan() -- Zero-sized metadata block found");
      setValid(false);
      return;
    }

    const ByteVector data = readBlock(length);
    if(data.size() != length) {
      debug("FLAC::File::scan() -- Failed to read a metadata block");
      setValid(false);
      return;
    }

    MetadataBlock *block = 0;

    // Found the vorbis-comment
    if(blockType == MetadataBlock::VorbisComment) {
      if(!d->hasXiphComment) {
        d->xiphCommentData = data;
        d->hasXiphComment = true;
      }
      else {
        debug("FLAC::File::scan() -- multiple Vorbis Comment blocks found, using the first one");
      }
    }
    else if(blockType == MetadataBlock::Picture) {
      FLAC::Picture *picture = new FLAC::Picture();
      if(picture->parse(data)) {
        block = picture;
      }
      else {
        debug("FLAC::File::scan() -- invalid picture found, discarding");
        delete picture;
      }
    }

    if(!block) {
      block = new UnknownMetadataBlock(blockType, data);
    }
    if(block->code() != MetadataBlock::Padding) {
      d->blocks.append(block);
    }
    else {
      delete block;
    }

    nextBlockOffset += length + 4;

    if(nextBlockOffset >= File::length()) {
      debug("FLAC::File::scan() -- FLAC stream corrupted");
      setValid(false);
      return;
    }
    seek(nextBlockOffset);
  }

  // End of metadata, now comes the datastream

  d->streamStart = nextBlockOffset;

  d->scanned = true;
}
