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

#include "flacfile.h"

#include <algorithm>
#include <utility>

#include "tdebug.h"
#include "tpropertymap.h"
#include "tagunion.h"
#include "tagutils.h"
#include "id3v2tag.h"
#include "id3v1tag.h"
#include "xiphcomment.h"
#include "flacpicture.h"
#include "flacmetadatablock.h"
#include "flacunknownmetadatablock.h"

using namespace TagLib;

namespace
{
  enum { FlacXiphIndex = 0, FlacID3v2Index = 1, FlacID3v1Index = 2 };

  constexpr long MinPaddingLength = 4096;
  constexpr long MaxPaddingLegnth = 1024 * 1024;

  constexpr char LastBlockFlag = '\x80';
}  // namespace

class FLAC::File::FilePrivate
{
public:
  FilePrivate(const ID3v2::FrameFactory *frameFactory = ID3v2::FrameFactory::instance()) :
    ID3v2FrameFactory(frameFactory)
  {
    blocks.setAutoDelete(true);
  }

  const ID3v2::FrameFactory *ID3v2FrameFactory;
  offset_t ID3v2Location { -1 };
  long ID3v2OriginalSize { 0 };

  offset_t ID3v1Location { -1 };

  TagUnion tag;

  std::unique_ptr<Properties> properties;
  ByteVector xiphCommentData;
  List<FLAC::MetadataBlock *> blocks;

  offset_t flacStart { 0 };
  offset_t streamStart { 0 };
  bool scanned { false };
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

bool FLAC::File::isSupported(IOStream *stream)
{
  // A FLAC file has an ID "fLaC" somewhere. An ID3v2 tag may precede.

  const ByteVector buffer = Utils::readHeader(stream, bufferSize(), true);
  return buffer.find("fLaC") >= 0;
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FLAC::File::File(FileName file, bool readProperties,
                 Properties::ReadStyle,
                 ID3v2::FrameFactory *frameFactory) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(
    frameFactory ? frameFactory : ID3v2::FrameFactory::instance()))
{
  if(isOpen())
    read(readProperties);
}

FLAC::File::File(FileName file, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties);
}

FLAC::File::File(IOStream *stream, bool readProperties,
                 Properties::ReadStyle,
                 ID3v2::FrameFactory *frameFactory) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(
    frameFactory ? frameFactory : ID3v2::FrameFactory::instance()))
{
  if(isOpen())
    read(readProperties);
}

FLAC::File::File(IOStream *stream, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties);
}

FLAC::File::~File() = default;

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

StringList FLAC::File::complexPropertyKeys() const
{
  StringList keys = TagLib::File::complexPropertyKeys();
  if(!keys.contains("PICTURE")) {
    if(std::any_of(d->blocks.cbegin(), d->blocks.cend(),
        [](MetadataBlock *block) {
      return dynamic_cast<Picture *>(block) != nullptr;
    })) {
      keys.append("PICTURE");
    }
  }
  return keys;
}

List<VariantMap> FLAC::File::complexProperties(const String &key) const
{
  if(const String uppercaseKey = key.upper(); uppercaseKey == "PICTURE") {
    List<VariantMap> props;
    for(const auto &block : std::as_const(d->blocks)) {
      if(auto picture = dynamic_cast<Picture *>(block)) {
        VariantMap property;
        property.insert("data", picture->data());
        property.insert("mimeType", picture->mimeType());
        property.insert("description", picture->description());
        property.insert("pictureType",
          FLAC::Picture::typeToString(picture->type()));
        property.insert("width", picture->width());
        property.insert("height", picture->height());
        property.insert("numColors", picture->numColors());
        property.insert("colorDepth", picture->colorDepth());
        props.append(property);
      }
    }
    return props;
  }
  return TagLib::File::complexProperties(key);
}

bool FLAC::File::setComplexProperties(const String &key, const List<VariantMap> &value)
{
  if(const String uppercaseKey = key.upper(); uppercaseKey == "PICTURE") {
    removePictures();

    for(const auto &property : value) {
      auto picture = new FLAC::Picture;
      picture->setData(property.value("data").value<ByteVector>());
      picture->setMimeType(property.value("mimeType").value<String>());
      picture->setDescription(property.value("description").value<String>());
      picture->setType(FLAC::Picture::typeFromString(
        property.value("pictureType").value<String>()));
      picture->setWidth(property.value("width").value<int>());
      picture->setHeight(property.value("height").value<int>());
      picture->setNumColors(property.value("numColors").value<int>());
      picture->setColorDepth(property.value("colorDepth").value<int>());
      addPicture(picture);
    }
  }
  else {
    return TagLib::File::setComplexProperties(key, value);
  }
  return true;
}

FLAC::Properties *FLAC::File::audioProperties() const
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
  if(!hasXiphComment())
    Tag::duplicate(&d->tag, xiphComment(true), false);

  d->xiphCommentData = xiphComment()->render(false);

  // Replace metadata blocks

  MetadataBlock *commentBlock =
      new UnknownMetadataBlock(MetadataBlock::VorbisComment, d->xiphCommentData);
  for(auto it = d->blocks.begin(); it != d->blocks.end();) {
    if((*it)->code() == MetadataBlock::VorbisComment) {
      // Remove the old Vorbis Comment block
      delete *it;
      it = d->blocks.erase(it);
      continue;
    }
    if(commentBlock && (*it)->code() == MetadataBlock::Picture) {
      // Set the new Vorbis Comment block before the first picture block
      d->blocks.insert(it, commentBlock);
      commentBlock = nullptr;
    }
    ++it;
  }
  if(commentBlock)
    d->blocks.append(commentBlock);

  // Render data for the metadata blocks

  ByteVector data;
  for(auto it = d->blocks.begin(); it != d->blocks.end();) {
    ByteVector blockData = (*it)->render();
    ByteVector blockHeader = ByteVector::fromUInt(blockData.size());
    if(blockHeader[0] != 0) {
      debug("FLAC::File::save() -- Removing too large block.");
      delete *it;
      it = d->blocks.erase(it);
      continue;
    }
    blockHeader[0] = (*it)->code();
    data.append(blockHeader);
    data.append(blockData);
    ++it;
  }

  // Compute the amount of padding, and append that to data.

  offset_t originalLength = d->streamStart - d->flacStart;
  offset_t paddingLength = originalLength - data.size() - 4;

  if(paddingLength <= 0) {
    paddingLength = MinPaddingLength;
  }
  else {
    // Padding won't increase beyond 1% of the file size or 1MB.

    offset_t threshold = length() / 100;
    threshold = std::max<offset_t>(threshold, MinPaddingLength);
    threshold = std::min<offset_t>(threshold, MaxPaddingLegnth);

    if(paddingLength > threshold)
      paddingLength = MinPaddingLength;
  }

  ByteVector paddingHeader = ByteVector::fromUInt(static_cast<unsigned int>(paddingLength));
  paddingHeader[0] = static_cast<char>(MetadataBlock::Padding | LastBlockFlag);
  data.append(paddingHeader);
  data.resize(static_cast<unsigned int>(data.size() + paddingLength));

  // Write the data to the file

  insert(data, d->flacStart, originalLength);

  d->streamStart += static_cast<long>(data.size()) - originalLength;

  if(d->ID3v1Location >= 0)
    d->ID3v1Location += static_cast<long>(data.size()) - originalLength;

  // Update ID3 tags

  if(ID3v2Tag() && !ID3v2Tag()->isEmpty()) {

    // ID3v2 tag is not empty. Update the old one or create a new one.

    if(d->ID3v2Location < 0)
      d->ID3v2Location = 0;

    data = ID3v2Tag()->render();
    insert(data, d->ID3v2Location, d->ID3v2OriginalSize);

    d->flacStart   += static_cast<long>(data.size()) - d->ID3v2OriginalSize;
    d->streamStart += static_cast<long>(data.size()) - d->ID3v2OriginalSize;

    if(d->ID3v1Location >= 0)
      d->ID3v1Location += static_cast<long>(data.size()) - d->ID3v2OriginalSize;

    d->ID3v2OriginalSize = data.size();
  }
  else {

    // ID3v2 tag is empty. Remove the old one.

    if(d->ID3v2Location >= 0) {
      removeBlock(d->ID3v2Location, d->ID3v2OriginalSize);

      d->flacStart   -= d->ID3v2OriginalSize;
      d->streamStart -= d->ID3v2OriginalSize;

      if(d->ID3v1Location >= 0)
        d->ID3v1Location -= d->ID3v2OriginalSize;

      d->ID3v2Location = -1;
      d->ID3v2OriginalSize = 0;
    }
  }

  if(ID3v1Tag() && !ID3v1Tag()->isEmpty()) {

    // ID3v1 tag is not empty. Update the old one or create a new one.

    if(d->ID3v1Location >= 0) {
      seek(d->ID3v1Location);
    }
    else {
      seek(0, End);
      d->ID3v1Location = tell();
    }

    writeBlock(ID3v1Tag()->render());
  }
  else {

    // ID3v1 tag is empty. Remove the old one.

    if(d->ID3v1Location >= 0) {
      truncate(d->ID3v1Location);
      d->ID3v1Location = -1;
    }
  }

  return true;
}

ID3v2::Tag *FLAC::File::ID3v2Tag(bool create)
{
  return d->tag.access<ID3v2::Tag>(FlacID3v2Index, create,
                                   d->ID3v2FrameFactory);
}

ID3v1::Tag *FLAC::File::ID3v1Tag(bool create)
{
  return d->tag.access<ID3v1::Tag>(FlacID3v1Index, create);
}

Ogg::XiphComment *FLAC::File::xiphComment(bool create)
{
  return d->tag.access<Ogg::XiphComment>(FlacXiphIndex, create);
}

List<FLAC::Picture *> FLAC::File::pictureList()
{
  List<Picture *> pictures;
  for(const auto &block : std::as_const(d->blocks)) {
    if(auto picture = dynamic_cast<Picture *>(block)) {
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
  auto it = d->blocks.find(picture);
  if(it != d->blocks.end())
    d->blocks.erase(it);

  if(del)
    delete picture;
}

void FLAC::File::removePictures()
{
  for(auto it = d->blocks.begin(); it != d->blocks.end(); ) {
    if(dynamic_cast<Picture *>(*it)) {
      delete *it;
      it = d->blocks.erase(it);
    }
    else {
      ++it;
    }
  }
}

void FLAC::File::strip(int tags)
{
  if(tags & ID3v1)
    d->tag.set(FlacID3v1Index, nullptr);

  if(tags & ID3v2)
    d->tag.set(FlacID3v2Index, nullptr);

  if(tags & XiphComment) {
    xiphComment()->removeAllFields();
    xiphComment()->removeAllPictures();
  }
}

bool FLAC::File::hasXiphComment() const
{
  return !d->xiphCommentData.isEmpty();
}

bool FLAC::File::hasID3v1Tag() const
{
  return d->ID3v1Location >= 0;
}

bool FLAC::File::hasID3v2Tag() const
{
  return d->ID3v2Location >= 0;
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
  }

  // Look for an ID3v1 tag

  d->ID3v1Location = Utils::findID3v1(this);

  if(d->ID3v1Location >= 0)
    d->tag.set(FlacID3v1Index, new ID3v1::Tag(this, d->ID3v1Location));

  // Look for FLAC metadata, including vorbis comments

  scan();

  if(!isValid())
    return;

  if(!d->xiphCommentData.isEmpty())
    d->tag.set(FlacXiphIndex, new Ogg::XiphComment(d->xiphCommentData));
  else
    d->tag.set(FlacXiphIndex, new Ogg::XiphComment());

  if(readProperties) {

    // First block should be the stream_info metadata

    const ByteVector infoData = d->blocks.front()->render();

    offset_t streamLength;

    if(d->ID3v1Location >= 0)
      streamLength = d->ID3v1Location - d->streamStart;
    else
      streamLength = length() - d->streamStart;

    d->properties = std::make_unique<Properties>(infoData, streamLength);
  }
}

void FLAC::File::scan()
{
  // Scan the metadata pages

  if(d->scanned)
    return;

  if(!isValid())
    return;

  offset_t nextBlockOffset;

  if(d->ID3v2Location >= 0)
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

  while(true) {

    seek(nextBlockOffset);
    const ByteVector header = readBlock(4);
    if(header.size() != 4) {
      debug("FLAC::File::scan() -- Failed to read a block header");
      setValid(false);
      return;
    }

    // Header format (from spec):
    // <1> Last-metadata-block flag
    // <7> BLOCK_TYPE
    //    0 : STREAMINFO
    //    1 : PADDING
    //    ..
    //    4 : VORBIS_COMMENT
    //    ..
    //    6 : PICTURE
    //    ..
    // <24> Length of metadata to follow

    const char blockType = header[0] & ~LastBlockFlag;
    const bool isLastBlock = (header[0] & LastBlockFlag) != 0;
    const unsigned int blockLength = header.toUInt(1U, 3U);

    // First block should be the stream_info metadata

    if(d->blocks.isEmpty() && blockType != MetadataBlock::StreamInfo) {
      debug("FLAC::File::scan() -- First block should be the stream_info metadata");
      setValid(false);
      return;
    }

    if(blockLength == 0
      && blockType != MetadataBlock::Padding && blockType != MetadataBlock::SeekTable)
    {
      debug("FLAC::File::scan() -- Zero-sized metadata block found");
      setValid(false);
      return;
    }

    const ByteVector data = readBlock(blockLength);
    if(data.size() != blockLength) {
      debug("FLAC::File::scan() -- Failed to read a metadata block");
      setValid(false);
      return;
    }

    MetadataBlock *block = nullptr;

    // Found the vorbis-comment
    if(blockType == MetadataBlock::VorbisComment) {
      if(d->xiphCommentData.isEmpty()) {
        d->xiphCommentData = data;
        block = new UnknownMetadataBlock(MetadataBlock::VorbisComment, data);
      }
      else {
        debug("FLAC::File::scan() -- multiple Vorbis Comment blocks found, discarding");
      }
    }
    else if(blockType == MetadataBlock::Picture) {
      auto picture = new FLAC::Picture();
      if(picture->parse(data)) {
        block = picture;
      }
      else {
        debug("FLAC::File::scan() -- invalid picture found, discarding");
        delete picture;
      }
    }
    else if(blockType == MetadataBlock::Padding) {
      // Skip all padding blocks.
    }
    else {
      block = new UnknownMetadataBlock(blockType, data);
    }

    if(block)
      d->blocks.append(block);

    nextBlockOffset += blockLength + 4;

    if(isLastBlock)
      break;
  }

  // End of metadata, now comes the datastream

  d->streamStart = nextBlockOffset;

  d->scanned = true;
}
