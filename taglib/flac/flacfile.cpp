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
#include <tsmartptr.h>

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

  typedef SHARED_PTR<FLAC::MetadataBlock> BlockPtr;
  typedef SHARED_PTR<FLAC::Picture>       PicturePtr;
}

namespace TagLib { namespace FLAC
{
  // Allows List<BlockPtr>::find() to take a Picture*.
  bool operator==(const BlockPtr &a, const Picture *b) { return (a.get() == b); }
}}

class FLAC::File::FilePrivate
{
public:
  FilePrivate() :
    ID3v2FrameFactory(ID3v2::FrameFactory::instance()),
    ID3v2Location(-1),
    ID3v2OriginalSize(0),
    ID3v1Location(-1),
    flacStart(0),
    streamStart(0),
    streamLength(0),
    scanned(false),
    hasXiphComment(false),
    hasID3v2(false),
    hasID3v1(false)
  {
  }

  const ID3v2::FrameFactory *ID3v2FrameFactory;
  offset_t ID3v2Location;
  uint ID3v2OriginalSize;

  offset_t ID3v1Location;

  SCOPED_PTR<TripleTagUnion>  tag;

  SCOPED_PTR<AudioProperties> properties;
  ByteVector streamInfoData;
  ByteVector xiphCommentData;
  List<BlockPtr> blocks;

  offset_t flacStart;
  offset_t streamStart;
  offset_t streamLength;
  bool scanned;

  bool hasXiphComment;
  bool hasID3v2;
  bool hasID3v1;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FLAC::File::File(FileName file, bool readProperties, 
                 AudioProperties::ReadStyle propertiesStyle,
                 ID3v2::FrameFactory *frameFactory) :
  TagLib::File(file),
  d(new FilePrivate())
{
  if(frameFactory)
    d->ID3v2FrameFactory = frameFactory;

  if(isOpen())
    read(readProperties, propertiesStyle);
}

FLAC::File::File(IOStream *stream, bool readProperties, 
                 AudioProperties::ReadStyle propertiesStyle,
                 ID3v2::FrameFactory *frameFactory) :
  TagLib::File(stream),
  d(new FilePrivate())
{
  if(frameFactory)
    d->ID3v2FrameFactory = frameFactory;

  if(isOpen())
    read(readProperties, propertiesStyle);
}

FLAC::File::~File()
{
  delete d;
}

TagLib::Tag *FLAC::File::tag() const
{
  return d->tag.get();
}

PropertyMap FLAC::File::setProperties(const PropertyMap &properties)
{
  return d->tag->access<Ogg::XiphComment>(FlacXiphIndex, true)->setProperties(properties);
}

FLAC::AudioProperties *FLAC::File::audioProperties() const
{
  return d->properties.get();
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

  Tag::duplicate(d->tag.get(), xiphComment(true), false);

  d->xiphCommentData = xiphComment()->render(false);

  // Replace metadata blocks

  bool foundVorbisCommentBlock = false;
  List<BlockPtr> newBlocks;
  for(List<BlockPtr>::ConstIterator it = d->blocks.begin(); it != d->blocks.end(); ++it) {
    BlockPtr block = *it;
    if(block->code() == MetadataBlock::VorbisComment) {
      // Set the new Vorbis Comment block
      block.reset(new UnknownMetadataBlock(MetadataBlock::VorbisComment, d->xiphCommentData));
      foundVorbisCommentBlock = true;
    }

    if(block->code() != MetadataBlock::Padding)
      newBlocks.append(block);
  }

  if(!foundVorbisCommentBlock) {
    newBlocks.append(BlockPtr(
      new UnknownMetadataBlock(MetadataBlock::VorbisComment, d->xiphCommentData)));
    foundVorbisCommentBlock = true;
  }
  d->blocks = newBlocks;

  // Render data for the metadata blocks

  ByteVector data;
  for(List<BlockPtr>::ConstIterator it = newBlocks.begin(); it != newBlocks.end(); ++it) {
    ByteVector blockData = (*it)->render();
    ByteVector blockHeader = ByteVector::fromUInt32BE(blockData.size());
    blockHeader[0] = (*it)->code();
    data.append(blockHeader);
    data.append(blockData);
  }

  // Adjust the padding block(s)

  uint originalLength = static_cast<uint>(d->streamStart - d->flacStart);
  int paddingLength = static_cast<int>(originalLength - data.size() - 4);
  if (paddingLength < 0)
    paddingLength = MinPaddingLength;

  ByteVector padding = ByteVector::fromUInt32BE(paddingLength);
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
    seek(-128, End);
    writeBlock(ID3v1Tag()->render());
  }

  return true;
}

ID3v2::Tag *FLAC::File::ID3v2Tag(bool create)
{
  return d->tag->access<ID3v2::Tag>(FlacID3v2Index, create);
}

ID3v1::Tag *FLAC::File::ID3v1Tag(bool create)
{
  return d->tag->access<ID3v1::Tag>(FlacID3v1Index, create);
}

Ogg::XiphComment *FLAC::File::xiphComment(bool create)
{
  return d->tag->access<Ogg::XiphComment>(FlacXiphIndex, create);
}

void FLAC::File::setID3v2FrameFactory(const ID3v2::FrameFactory *factory)
{
  d->ID3v2FrameFactory = factory;
}


////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void FLAC::File::read(bool readProperties, AudioProperties::ReadStyle propertiesStyle)
{
  d->tag.reset(new TripleTagUnion());

  // Look for an ID3v2 tag

  d->ID3v2Location = findID3v2();

  if(d->ID3v2Location >= 0) {

    d->tag->set(FlacID3v2Index, new ID3v2::Tag(this, d->ID3v2Location, d->ID3v2FrameFactory));

    d->ID3v2OriginalSize = ID3v2Tag()->header()->completeTagSize();

    if(ID3v2Tag()->header()->tagSize() <= 0)
      d->tag->set(FlacID3v2Index, 0);
    else
      d->hasID3v2 = true;
  }

  // Look for an ID3v1 tag

  d->ID3v1Location = findID3v1();

  if(d->ID3v1Location >= 0) {
    d->tag->set(FlacID3v1Index, new ID3v1::Tag(this, d->ID3v1Location));
    d->hasID3v1 = true;
  }

  // Look for FLAC metadata, including vorbis comments

  scan();

  if(!isValid())
    return;

  if(d->hasXiphComment)
    d->tag->set(FlacXiphIndex, new Ogg::XiphComment(xiphCommentData()));
  else
    d->tag->set(FlacXiphIndex, new Ogg::XiphComment);

  if(readProperties)
    d->properties.reset(new AudioProperties(d->streamInfoData, d->streamLength, propertiesStyle));
}

ByteVector FLAC::File::xiphCommentData() const
{
  return (isValid() && d->hasXiphComment) ? d->xiphCommentData : ByteVector();
}

void FLAC::File::scan()
{
  // Scan the metadata pages

  if(d->scanned)
    return;

  if(!isValid())
    return;

  offset_t nextBlockOffset;

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
  uint length = header.toUInt24BE(1);

  // First block should be the stream_info metadata

  if(blockType != MetadataBlock::StreamInfo) {
    debug("FLAC::File::scan() -- invalid FLAC stream");
    setValid(false);
    return;
  }

  d->streamInfoData = readBlock(length);
  d->blocks.append(BlockPtr(new UnknownMetadataBlock(blockType, d->streamInfoData)));
  nextBlockOffset += length + 4;

  // Search through the remaining metadata
  while(!isLastBlock) {

    header = readBlock(4);
    blockType = header[0] & 0x7f;
    isLastBlock = (header[0] & 0x80) != 0;
    length = header.toUInt24BE(1);

    ByteVector data = readBlock(length);
    if(data.size() != length || length == 0) {
      debug("FLAC::File::scan() -- FLAC stream corrupted");
      setValid(false);
      return;
    }

    BlockPtr block;

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
      PicturePtr picture(new FLAC::Picture());
      if(picture->parse(data))
        block = picture;
      else
        debug("FLAC::File::scan() -- invalid picture found, discarding");
    }

    if(!block)
      block.reset(new UnknownMetadataBlock(blockType, data));

    if(block->code() != MetadataBlock::Padding)
      d->blocks.append(block);

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
  d->streamLength = File::length() - d->streamStart;

  if(d->hasID3v1)
    d->streamLength -= 128;

  d->scanned = true;
}

offset_t FLAC::File::findID3v1()
{
  if(!isValid())
    return -1;

  seek(-128, End);
  offset_t p = tell();

  if(readBlock(3) == ID3v1::Tag::fileIdentifier())
    return p;

  return -1;
}

offset_t FLAC::File::findID3v2()
{
  if(!isValid())
    return -1;

  seek(0);

  if(readBlock(3) == ID3v2::Header::fileIdentifier())
    return 0;

  return -1;
}

List<FLAC::Picture *> FLAC::File::pictureList()
{
  List<Picture *> pictures;
  for(List<BlockPtr>::ConstIterator it = d->blocks.begin(); it != d->blocks.end(); ++it) {
    Picture *picture = dynamic_cast<Picture *>(it->get());
    if(picture)
      pictures.append(picture);
  }

  return pictures;
}

FLAC::Picture *FLAC::File::addPicture(const ByteVector &data)
{
  PicturePtr picture;
  if(!data.isEmpty())
    picture.reset(new Picture(data));
  else
    picture.reset(new Picture());

  d->blocks.append(picture);

  return picture.get();
}

void FLAC::File::removePicture(Picture *picture)
{
  List<BlockPtr>::Iterator it = d->blocks.find(picture);
  if(it != d->blocks.end())
    d->blocks.erase(it);
}

void FLAC::File::removePictures()
{
  List<BlockPtr> newBlocks;
  for(List<BlockPtr>::ConstIterator it = d->blocks.begin(); it != d->blocks.end(); ++it) {
    if(!dynamic_cast<Picture *>(it->get()))
      newBlocks.append(*it);
  }

  d->blocks = newBlocks;
}

