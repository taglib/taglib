/***************************************************************************
    copyright            : (C) 2013 by Stephen F. Booth
    email                : me@sbooth.org
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
#include <id3v2tag.h>
#include <tstringlist.h>
#include <tpropertymap.h>
#include <tsmartptr.h>

#include "dsffile.h"

using namespace TagLib;

// The DSF specification is located at http://dsd-guide.com/sites/default/files/white-papers/DSFFileFormatSpec_E.pdf

class DSF::File::FilePrivate
{
public:
  FilePrivate() :
    fileSize(0),
    metadataOffset(0) {}

  long long fileSize;
  long long metadataOffset;

  SCOPED_PTR<AudioProperties> properties;
  SCOPED_PTR<ID3v2::Tag> tag;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSF::File::File(FileName file, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(file),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSF::File::File(IOStream *stream, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(stream),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSF::File::~File()
{
  delete d;
}

ID3v2::Tag *DSF::File::tag() const
{
  return d->tag.get();
}

DSF::AudioProperties *DSF::File::audioProperties() const
{
  return d->properties.get();
}

bool DSF::File::save()
{
  if(readOnly()) {
    debug("DSF::File::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    debug("DSF::File::save() -- Trying to save invalid file.");
    return false;
  }

  // Three things must be updated: the file size, the tag data, and the metadata offset

  if(d->tag->isEmpty()) {
    long long newFileSize = d->metadataOffset ? d->metadataOffset : d->fileSize;

    // Update the file size
    if(d->fileSize != newFileSize) {
      insert(ByteVector::fromUInt64LE(newFileSize), 12, 8);
      d->fileSize = newFileSize;
    }

    // Update the metadata offset to 0 since there is no longer a tag
    if(d->metadataOffset) {
      insert(ByteVector::fromUInt64LE(0ULL), 20, 8);
      d->metadataOffset = 0;
    }

    // Delete the old tag
    truncate(newFileSize);
  }
  else {
    ByteVector tagData = d->tag->render();

    long long newMetadataOffset = d->metadataOffset ? d->metadataOffset : d->fileSize;
    long long newFileSize = newMetadataOffset + tagData.size();
    long long oldTagSize = d->fileSize - newMetadataOffset;

    // Update the file size
    if(d->fileSize != newFileSize) {
      insert(ByteVector::fromUInt64LE(newFileSize), 12, 8);
      d->fileSize = newFileSize;
    }

    // Update the metadata offset
    if(d->metadataOffset != newMetadataOffset) {
      insert(ByteVector::fromUInt64LE(newMetadataOffset), 20, 8);
      d->metadataOffset = newMetadataOffset;
    }

    // Delete the old tag and write the new one
    insert(tagData, newMetadataOffset, static_cast<size_t>(oldTagSize));
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DSF::File::read(bool readProperties, AudioProperties::ReadStyle propertiesStyle)
{
  // A DSF file consists of four chunks: DSD chunk, format chunk, data chunk, and metadata chunk
  // The file format is not chunked in the sense of a RIFF File, though

  // DSD chunk
  ByteVector chunkName = readBlock(4);
  if(chunkName != "DSD ") {
    debug("DSF::File::read() -- Not a DSF file.");
    setValid(false);
    return;
  }

  long long chunkSize = readBlock(8).toInt64LE(0);

  // Integrity check
  if(28 != chunkSize) {
    debug("DSF::File::read() -- File is corrupted.");
    setValid(false);
    return;
  }

  d->fileSize = readBlock(8).toInt64LE(0);

  // File is malformed or corrupted
  if(d->fileSize != length()) {
    debug("DSF::File::read() -- File is corrupted.");
    setValid(false);
    return;
  }

  d->metadataOffset = readBlock(8).toInt64LE(0);

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

  chunkSize = readBlock(8).toInt64LE(0);

  d->properties.reset(
    new AudioProperties(readBlock(static_cast<size_t>(chunkSize)), propertiesStyle));

  // Skip the data chunk

  // A metadata offset of 0 indicates the absence of an ID3v2 tag
  if(0 == d->metadataOffset)
    d->tag.reset(new ID3v2::Tag());
  else
    d->tag.reset(new ID3v2::Tag(this, d->metadataOffset));
}

