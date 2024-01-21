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

#include "mpegfile.h"

#include "id3v2framefactory.h"
#include "tdebug.h"
#include "tpropertymap.h"
#include "apefooter.h"
#include "apetag.h"
#include "id3v1tag.h"
#include "id3v2tag.h"
#include "tagunion.h"
#include "tagutils.h"
#include "mpegheader.h"
#include "mpegutils.h"

using namespace TagLib;

namespace
{
  enum { ID3v2Index = 0, APEIndex = 1, ID3v1Index = 2 };
} // namespace

class MPEG::File::FilePrivate
{
public:
  FilePrivate(const ID3v2::FrameFactory *frameFactory = ID3v2::FrameFactory::instance()) :
    ID3v2FrameFactory(frameFactory)
  {
  }

  const ID3v2::FrameFactory *ID3v2FrameFactory;

  offset_t ID3v2Location { -1 };
  long ID3v2OriginalSize { 0 };

  offset_t APELocation { -1 };
  long APEOriginalSize { 0 };

  offset_t ID3v1Location { -1 };

  TagUnion tag;

  std::unique_ptr<Properties> properties;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

namespace
{
  // Dummy file class to make a stream work with MPEG::Header.

  class AdapterFile : public TagLib::File
  {
  public:
    AdapterFile(IOStream *stream) : File(stream) {}

    Tag *tag() const override { return nullptr; }
    AudioProperties *audioProperties() const override { return nullptr; }
    bool save() override { return false; }
  };
}  // namespace

bool MPEG::File::isSupported(IOStream *stream)
{
  if(!stream || !stream->isOpen())
    return false;

  // An MPEG file has MPEG frame headers. An ID3v2 tag may precede.

  // MPEG frame headers are really confusing with irrelevant binary data.
  // So we check if a frame header is really valid.

  offset_t headerOffset;
  const ByteVector buffer = Utils::readHeader(stream, bufferSize(), true, &headerOffset);

  if(buffer.isEmpty())
      return false;

  const offset_t originalPosition = stream->tell();
  AdapterFile file(stream);

  for(unsigned int i = 0; i < buffer.size() - 1; ++i) {
    if(isFrameSync(buffer, i)) {
      if(const Header header(&file, headerOffset + i, true); header.isValid()) {
        stream->seek(originalPosition);
        return true;
      }
    }
  }

  stream->seek(originalPosition);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPEG::File::File(FileName file, bool readProperties,
                 Properties::ReadStyle readStyle,
                 ID3v2::FrameFactory *frameFactory) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(
    frameFactory ? frameFactory : ID3v2::FrameFactory::instance()))
{
  if(isOpen())
    read(readProperties, readStyle);
}

MPEG::File::File(FileName file, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle readStyle) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties, readStyle);
}

MPEG::File::File(IOStream *stream, bool readProperties,
                 Properties::ReadStyle readStyle,
                 ID3v2::FrameFactory *frameFactory) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(
    frameFactory ? frameFactory : ID3v2::FrameFactory::instance()))
{
  if(isOpen())
    read(readProperties, readStyle);
}

MPEG::File::File(IOStream *stream, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle readStyle) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties, readStyle);
}

MPEG::File::~File() = default;

TagLib::Tag *MPEG::File::tag() const
{
  return &d->tag;
}

PropertyMap MPEG::File::properties() const
{
  return d->tag.properties();
}

void MPEG::File::removeUnsupportedProperties(const StringList &properties)
{
  d->tag.removeUnsupportedProperties(properties);
}

PropertyMap MPEG::File::setProperties(const PropertyMap &properties)
{
  // update ID3v1 tag if it exists, but ignore the return value

  if(ID3v1Tag())
    ID3v1Tag()->setProperties(properties);

  return ID3v2Tag(true)->setProperties(properties);
}

MPEG::Properties *MPEG::File::audioProperties() const
{
  return d->properties.get();
}

bool MPEG::File::save()
{
  return save(AllTags);
}

bool MPEG::File::save(int tags, StripTags strip, ID3v2::Version version, DuplicateTags duplicate)
{
  if(readOnly()) {
    debug("MPEG::File::save() -- File is read only.");
    return false;
  }

  // Create the tags if we've been asked to.

  if(duplicate == Duplicate) {

    // Copy the values from the tag that does exist into the new tag,
    // except if the existing tag is to be stripped.

    if((tags & ID3v2) && ID3v1Tag() && (strip != StripOthers || (tags & ID3v1)))
      Tag::duplicate(ID3v1Tag(), ID3v2Tag(true), false);

    if((tags & ID3v1) && d->tag[ID3v2Index] && (strip != StripOthers || (tags & ID3v2)))
      Tag::duplicate(ID3v2Tag(), ID3v1Tag(true), false);
  }

  // Remove all the tags not going to be saved.

  if(strip == StripOthers)
    File::strip(~tags, false);

  if(ID3v2 & tags) {

    if(ID3v2Tag() && !ID3v2Tag()->isEmpty()) {

      // ID3v2 tag is not empty. Update the old one or create a new one.

      if(d->ID3v2Location < 0)
        d->ID3v2Location = 0;

      const ByteVector data = ID3v2Tag()->render(version);
      insert(data, d->ID3v2Location, d->ID3v2OriginalSize);

      if(d->APELocation >= 0)
        d->APELocation += static_cast<long>(data.size()) - d->ID3v2OriginalSize;

      if(d->ID3v1Location >= 0)
        d->ID3v1Location += static_cast<long>(data.size()) - d->ID3v2OriginalSize;

      d->ID3v2OriginalSize = data.size();
    }
    else {

      // ID3v2 tag is empty. Remove the old one.

      File::strip(ID3v2, false);
    }
  }

  if(ID3v1 & tags) {

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

      File::strip(ID3v1, false);
    }
  }

  if(APE & tags) {

    if(APETag() && !APETag()->isEmpty()) {

      // APE tag is not empty. Update the old one or create a new one.

      if(d->APELocation < 0) {
        if(d->ID3v1Location >= 0)
          d->APELocation = d->ID3v1Location;
        else
          d->APELocation = length();
      }

      const ByteVector data = APETag()->render();
      insert(data, d->APELocation, d->APEOriginalSize);

      if(d->ID3v1Location >= 0)
        d->ID3v1Location += static_cast<long>(data.size()) - d->APEOriginalSize;

      d->APEOriginalSize = data.size();
    }
    else {

      // APE tag is empty. Remove the old one.

      File::strip(APE, false);
    }
  }

  return true;
}

ID3v2::Tag *MPEG::File::ID3v2Tag(bool create)
{
  return d->tag.access<ID3v2::Tag>(ID3v2Index, create, d->ID3v2FrameFactory);
}

ID3v1::Tag *MPEG::File::ID3v1Tag(bool create)
{
  return d->tag.access<ID3v1::Tag>(ID3v1Index, create);
}

APE::Tag *MPEG::File::APETag(bool create)
{
  return d->tag.access<APE::Tag>(APEIndex, create);
}

bool MPEG::File::strip(int tags, bool freeMemory)
{
  if(readOnly()) {
    debug("MPEG::File::strip() - Cannot strip tags from a read only file.");
    return false;
  }

  if((tags & ID3v2) && d->ID3v2Location >= 0) {
    removeBlock(d->ID3v2Location, d->ID3v2OriginalSize);

    if(d->APELocation >= 0)
      d->APELocation -= d->ID3v2OriginalSize;

    if(d->ID3v1Location >= 0)
      d->ID3v1Location -= d->ID3v2OriginalSize;

    d->ID3v2Location = -1;
    d->ID3v2OriginalSize = 0;

    if(freeMemory)
      d->tag.set(ID3v2Index, nullptr);
  }

  if((tags & ID3v1) && d->ID3v1Location >= 0) {
    truncate(d->ID3v1Location);

    d->ID3v1Location = -1;

    if(freeMemory)
      d->tag.set(ID3v1Index, nullptr);
  }

  if((tags & APE) && d->APELocation >= 0) {
    removeBlock(d->APELocation, d->APEOriginalSize);

    if(d->ID3v1Location >= 0)
      d->ID3v1Location -= d->APEOriginalSize;

    d->APELocation = -1;
    d->APEOriginalSize = 0;

    if(freeMemory)
      d->tag.set(APEIndex, nullptr);
  }

  return true;
}

offset_t MPEG::File::nextFrameOffset(offset_t position)
{
  ByteVector frameSyncBytes(2, '\0');

  while(true) {
    seek(position);
    const ByteVector buffer = readBlock(bufferSize());
    if(buffer.isEmpty())
      return -1;

    for(unsigned int i = 0; i < buffer.size(); ++i) {
      frameSyncBytes[0] = frameSyncBytes[1];
      frameSyncBytes[1] = buffer[i];
      if(isFrameSync(frameSyncBytes)) {
        if(const Header header(this, position + i - 1, true); header.isValid())
          return position + i - 1;
      }
    }

    position += bufferSize();
  }
}

offset_t MPEG::File::previousFrameOffset(offset_t position)
{
  ByteVector frameSyncBytes(2, '\0');

  while(position > 0) {
    const offset_t bufferLength = std::min<offset_t>(position, bufferSize());
    position -= bufferLength;

    seek(position);
    const ByteVector buffer = readBlock(bufferLength);

    for(int i = buffer.size() - 1; i >= 0; --i) {
      frameSyncBytes[1] = frameSyncBytes[0];
      frameSyncBytes[0] = buffer[i];
      if(isFrameSync(frameSyncBytes)) {
        if(const Header header(this, position + i, true); header.isValid())
          return position + i + header.frameLength();
      }
    }
  }

  return -1;
}

offset_t MPEG::File::firstFrameOffset()
{
  offset_t position = 0;

  if(hasID3v2Tag())
    position = d->ID3v2Location + ID3v2Tag()->header()->completeTagSize();

  return nextFrameOffset(position);
}

offset_t MPEG::File::lastFrameOffset()
{
  offset_t position;

  if(hasAPETag())
    position = d->APELocation - 1;
  else if(hasID3v1Tag())
    position = d->ID3v1Location - 1;
  else
    position = length();

  return previousFrameOffset(position);
}

bool MPEG::File::hasID3v1Tag() const
{
  return d->ID3v1Location >= 0;
}

bool MPEG::File::hasID3v2Tag() const
{
  return d->ID3v2Location >= 0;
}

bool MPEG::File::hasAPETag() const
{
  return d->APELocation >= 0;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MPEG::File::read(bool readProperties, Properties::ReadStyle readStyle)
{
  // Look for an ID3v2 tag

  d->ID3v2Location = findID3v2(readStyle);

  if(d->ID3v2Location >= 0) {
    d->tag.set(ID3v2Index, new ID3v2::Tag(this, d->ID3v2Location, d->ID3v2FrameFactory));
    d->ID3v2OriginalSize = ID3v2Tag()->header()->completeTagSize();
  }

  // Look for an ID3v1 tag

  d->ID3v1Location = Utils::findID3v1(this);

  if(d->ID3v1Location >= 0)
    d->tag.set(ID3v1Index, new ID3v1::Tag(this, d->ID3v1Location));

  // Look for an APE tag

  d->APELocation = Utils::findAPE(this, d->ID3v1Location);

  if(d->APELocation >= 0) {
    d->tag.set(APEIndex, new APE::Tag(this, d->APELocation));
    d->APEOriginalSize = APETag()->footer()->completeTagSize();
    d->APELocation = d->APELocation + APE::Footer::size() - d->APEOriginalSize;
  }

  if(readProperties)
    d->properties = std::make_unique<Properties>(this, readStyle);

  // Make sure that we have our default tag types available.

  ID3v2Tag(true);
  ID3v1Tag(true);
}

offset_t MPEG::File::findID3v2(Properties::ReadStyle readStyle)
{
  if(!isValid())
    return -1;

  // An ID3v2 tag or MPEG frame is most likely be at the beginning of the file.

  const ByteVector headerID = ID3v2::Header::fileIdentifier();

  seek(0);
  if(readBlock(headerID.size()) == headerID)
    return 0;

  if(readStyle == Properties::Fast)
    return -1;

  if(const Header firstHeader(this, 0, true); firstHeader.isValid())
    return -1;

  // Look for an ID3v2 tag until reaching the first valid MPEG frame.

  ByteVector frameSyncBytes(2, '\0');
  ByteVector tagHeaderBytes(3, '\0');
  long position = 0;

  while(true) {
    seek(position);
    const ByteVector buffer = readBlock(bufferSize());
    if(buffer.isEmpty())
      return -1;

    for(unsigned int i = 0; i < buffer.size(); ++i) {
      frameSyncBytes[0] = frameSyncBytes[1];
      frameSyncBytes[1] = buffer[i];
      if(isFrameSync(frameSyncBytes)) {
        if(const Header header(this, position + i - 1, true); header.isValid())
          return -1;
      }

      tagHeaderBytes[0] = tagHeaderBytes[1];
      tagHeaderBytes[1] = tagHeaderBytes[2];
      tagHeaderBytes[2] = buffer[i];
      if(tagHeaderBytes == headerID)
        return position + i - 2;
    }

    position += bufferSize();
  }
}
