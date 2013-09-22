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

#include "dsffile.h"

using namespace TagLib;

// The DSF specification is located at http://dsd-guide.com/sites/default/files/white-papers/DSFFileFormatSpec_E.pdf

class DSF::File::FilePrivate
{
public:
  FilePrivate() :
    properties(0),
    tag(0)
  {

  }

  ~FilePrivate()
  {
    delete properties;
    delete tag;
  }

  TagLib::uint fileSize;
  TagLib::uint metadataOffset;
  Properties *properties;
  ID3v2::Tag *tag;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSF::File::File(FileName file, bool readProperties,
                       Properties::ReadStyle propertiesStyle) : TagLib::File(file)
{
  d = new FilePrivate;
  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSF::File::File(IOStream *stream, bool readProperties,
                       Properties::ReadStyle propertiesStyle) : TagLib::File(stream)
{
  d = new FilePrivate;
  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSF::File::~File()
{
  delete d;
}

ID3v2::Tag *DSF::File::tag() const
{
  return d->tag;
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
  return d->properties;
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

  // Two things must be updated: the file size and the tag data
  // The metadata offset doesn't change

  ByteVector tagData = d->tag->render();
  
  uint oldTagSize = d->fileSize - d->metadataOffset;
  uint newFileSize = d->metadataOffset + tagData.size();

  // Write the file size
  insert(ByteVector::fromUInt(newFileSize, false), 12, 4);

  // Delete the old tag and write the new one
  insert(tagData, d->metadataOffset, oldTagSize);
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DSF::File::read(bool readProperties, Properties::ReadStyle propertiesStyle)
{
  // A DSF file consists of four chunks: DSD chunk, format chunk, data chunk, and metadata chunk
  // The file format is not chunked in the sense of a RIFF File, though
  
  // DSD chunk
  ByteVector chunkName = readBlock(4);
  if(chunkName != "DSD ") {
    debug("DSF::File::read() -- Not a DSF file.");
    return;
  }

  TagLib::uint chunkSize = readBlock(8).toUInt(false);
  
  // Integrity check
  if(28 != chunkSize) {
    debug("DSF::File::read() -- File is corrupted.");
    return;
  }
  
  d->fileSize = readBlock(8).toUInt(false);
  d->metadataOffset = readBlock(8).toUInt(false);

  // File is malformed or corrupted
  if(d->metadataOffset > d->fileSize) {
    debug("DSF::File::read() -- Invalid metadata offset.");
    return;
  }

  // Format chunk
  chunkName = readBlock(4);
  if(chunkName != "fmt ") {
    debug("DSF::File::read() -- Missing 'fmt ' chunk.");
    return;
  }

  chunkSize = readBlock(8).toUInt(false);
  
  d->properties = new Properties(readBlock(chunkSize), propertiesStyle);
  
  // Skip the data chunk

  // TODO: Is it better to read the chunk size directly or trust the offset in the DSD chunk?  
  d->tag = new ID3v2::Tag(this, d->metadataOffset);  
}
