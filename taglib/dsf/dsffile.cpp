/***************************************************************************
    copyright           : (C) 2013-2023 Stephen F. Booth
    email               : me@sbooth.org
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

#include "dsffile.h"

#include "tdebug.h"
#include "tpropertymap.h"
#include "tagutils.h"

using namespace TagLib;

class DSF::File::FilePrivate
{
public:
  FilePrivate(const ID3v2::FrameFactory *frameFactory)
        : ID3v2FrameFactory(frameFactory ? frameFactory
                                         : ID3v2::FrameFactory::instance())
  {
  }

  ~FilePrivate() = default;

  FilePrivate(const FilePrivate &) = delete;
  FilePrivate &operator=(const FilePrivate &) = delete;

  const ID3v2::FrameFactory *ID3v2FrameFactory;
  long long fileSize = 0;
  long long metadataOffset = 0;
  std::unique_ptr<Properties> properties;
  std::unique_ptr<ID3v2::Tag> tag;
};

bool DSF::File::isSupported(IOStream *stream)
{
  // A DSF file has to start with "DSD "
  const ByteVector id = Utils::readHeader(stream, 4, false);
  return id.startsWith("DSD ");
}

DSF::File::File(FileName file, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle,
                ID3v2::FrameFactory *frameFactory) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(propertiesStyle);
}

DSF::File::File(IOStream *stream, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle,
                ID3v2::FrameFactory *frameFactory) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(propertiesStyle);
}

DSF::File::~File() = default;

ID3v2::Tag *DSF::File::tag() const
{
  return d->tag.get();
}

PropertyMap DSF::File::properties() const
{
  return d->tag->properties();
}

PropertyMap DSF::File::setProperties(const PropertyMap &properties)
{
  return d->tag->setProperties(properties);
}

DSF::Properties *DSF::File::audioProperties() const
{
  return d->properties.get();
}

bool DSF::File::save()
{
  return save(ID3v2::v4);
}

bool DSF::File::save(ID3v2::Version version)
{
  if(readOnly()) {
    debug("DSF::File::save() - Cannot save to a read only file.");
    return false;
  }

  // Three things must be updated: the file size, the tag data, and the metadata offset

  if(d->tag->isEmpty()) {
    long long newFileSize = d->metadataOffset ? d->metadataOffset : d->fileSize;

    // Update the file size
    if(d->fileSize != newFileSize) {
      insert(ByteVector::fromLongLong(newFileSize, false), 12, 8);
      d->fileSize = newFileSize;
    }

    // Update the metadata offset to 0 since there is no longer a tag
    if(d->metadataOffset) {
      insert(ByteVector::fromLongLong(0ULL, false), 20, 8);
      d->metadataOffset = 0;
    }

    // Delete the old tag
    truncate(newFileSize);
  }
  else {
    ByteVector tagData = d->tag->render(version);

    long long newMetadataOffset = d->metadataOffset ? d->metadataOffset : d->fileSize;
    long long newFileSize = newMetadataOffset + tagData.size();
    long long oldTagSize = d->fileSize - newMetadataOffset;

    // Update the file size
    if(d->fileSize != newFileSize) {
      insert(ByteVector::fromLongLong(newFileSize, false), 12, 8);
      d->fileSize = newFileSize;
    }

    // Update the metadata offset
    if(d->metadataOffset != newMetadataOffset) {
      insert(ByteVector::fromLongLong(newMetadataOffset, false), 20, 8);
      d->metadataOffset = newMetadataOffset;
    }

    // Delete the old tag and write the new one
    insert(tagData, newMetadataOffset, static_cast<size_t>(oldTagSize));
  }

  return true;
}

void DSF::File::read(AudioProperties::ReadStyle propertiesStyle)
{
  if(!isOpen())
    return;

  // A DSF file consists of four chunks: DSD chunk, format chunk, data chunk, and metadata chunk
  // The file format is not chunked in the sense of a RIFF File, though

  // DSD chunk
  ByteVector chunkName = readBlock(4);
  if(chunkName != "DSD ") {
    debug("DSF::File::read() -- Not a DSF file.");
    setValid(false);
    return;
  }

  long long dsdHeaderSize = readBlock(8).toLongLong(false);

  // Integrity check
  if(dsdHeaderSize != 28) {
    debug("DSF::File::read() -- File is corrupted, wrong DSD header size");
    setValid(false);
    return;
  }

  d->fileSize = readBlock(8).toLongLong(false);

  // File is malformed or corrupted, allow trailing garbage
  if(d->fileSize > length()) {
    debug("DSF::File::read() -- File is corrupted wrong length");
    setValid(false);
    return;
  }

  d->metadataOffset = readBlock(8).toLongLong(false);

  // File is malformed or corrupted
  if(d->metadataOffset > d->fileSize) {
    debug("DSF::File::read() -- Invalid metadata offset.");
    setValid(false);
    return;
  }

  // Format chunk
  chunkName = readBlock(4);
  if(chunkName != "fmt ") {
    debug("DSF::File::read() -- Missing 'fmt ' chunk.");
    setValid(false);
    return;
  }

  long long fmtHeaderSize = readBlock(8).toLongLong(false);
  if(fmtHeaderSize != 52) {
    debug("DSF::File::read() -- File is corrupted, wrong FMT header size");
    setValid(false);
    return;
  }

  d->properties = std::make_unique<Properties>(readBlock(fmtHeaderSize), propertiesStyle);

  // Skip the data chunk

  // A metadata offset of 0 indicates the absence of an ID3v2 tag
  if(d->metadataOffset == 0)
    d->tag = std::make_unique<ID3v2::Tag>(nullptr, 0, d->ID3v2FrameFactory);
  else
    d->tag = std::make_unique<ID3v2::Tag>(this, d->metadataOffset,
                                          d->ID3v2FrameFactory);
}
