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

#include <tagunion.h>
#include <id3v2tag.h>
#include <id3v2header.h>
#include <id3v1tag.h>
#include <apefooter.h>
#include <apetag.h>
#include <tdebug.h>

#include <bitset>

#include "mpegfile.h"
#include "mpegheader.h"
#include "tpropertymap.h"

using namespace TagLib;

namespace
{
  enum { ID3v2Index = 0, APEIndex = 1, ID3v1Index = 2 };

  /*!
   * MPEG frames can be recognized by the bit pattern 11111111 111, so the
   * first byte is easy to check for, however checking to see if the second byte
   * starts with \e 111 is a bit more tricky, hence this member function.
   */
  inline bool firstSynchByte(uchar byte)
  {
    return (byte == 0xFF);
  }

  inline bool secondSynchByte(uchar byte)
  {
    return ((byte & 0xE0) == 0xE0);
  }
}

class MPEG::File::FilePrivate
{
public:
  FilePrivate(ID3v2::FrameFactory *frameFactory = ID3v2::FrameFactory::instance()) :
    ID3v2FrameFactory(frameFactory),
    ID3v2Location(-1),
    ID3v2OriginalSize(0),
    APELocation(-1),
    APEOriginalSize(0),
    ID3v1Location(-1),
    properties(0),
    hasID3v2(false),
    hasID3v1(false),
    hasAPE(false) {}

  ~FilePrivate()
  {
    delete properties;
  }

  const ID3v2::FrameFactory *ID3v2FrameFactory;

  long ID3v2Location;
  uint ID3v2OriginalSize;

  long APELocation;
  uint APEOriginalSize;

  long ID3v1Location;

  TagUnion tag;

  Properties *properties;

  // These indicate whether the file *on disk* has these tags, not if
  // this data structure does.  This is used in computing offsets.

  bool hasID3v2;
  bool hasID3v1;
  bool hasAPE;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPEG::File::File(FileName file, bool readProperties,
                 Properties::ReadStyle propertiesStyle) : TagLib::File(file)
{
  d = new FilePrivate;

  if(isOpen())
    read(readProperties, propertiesStyle);
}

MPEG::File::File(FileName file, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle propertiesStyle) :
  TagLib::File(file)
{
  d = new FilePrivate(frameFactory);

  if(isOpen())
    read(readProperties, propertiesStyle);
}

MPEG::File::File(IOStream *stream, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle propertiesStyle) :
  TagLib::File(stream)
{
  d = new FilePrivate(frameFactory);

  if(isOpen())
    read(readProperties, propertiesStyle);
}

MPEG::File::~File()
{
  delete d;
}

TagLib::Tag *MPEG::File::tag() const
{
  return &d->tag;
}

PropertyMap MPEG::File::properties() const
{
  // once Tag::properties() is virtual, this case distinction could actually be done
  // within TagUnion.
  if(d->hasID3v2)
    return d->tag.access<ID3v2::Tag>(ID3v2Index, false)->properties();
  if(d->hasAPE)
    return d->tag.access<APE::Tag>(APEIndex, false)->properties();
  if(d->hasID3v1)
    return d->tag.access<ID3v1::Tag>(ID3v1Index, false)->properties();
  return PropertyMap();
}

void MPEG::File::removeUnsupportedProperties(const StringList &properties)
{
  if(d->hasID3v2)
    d->tag.access<ID3v2::Tag>(ID3v2Index, false)->removeUnsupportedProperties(properties);
  else if(d->hasAPE)
    d->tag.access<APE::Tag>(APEIndex, false)->removeUnsupportedProperties(properties);
  else if(d->hasID3v1)
    d->tag.access<ID3v1::Tag>(ID3v1Index, false)->removeUnsupportedProperties(properties);
}

PropertyMap MPEG::File::setProperties(const PropertyMap &properties)
{
  if(d->hasID3v1)
    // update ID3v1 tag if it exists, but ignore the return value
    d->tag.access<ID3v1::Tag>(ID3v1Index, false)->setProperties(properties);
  return d->tag.access<ID3v2::Tag>(ID3v2Index, true)->setProperties(properties);
}

MPEG::Properties *MPEG::File::audioProperties() const
{
  return d->properties;
}

bool MPEG::File::save()
{
  return save(AllTags);
}

bool MPEG::File::save(int tags)
{
  return save(tags, true);
}

bool MPEG::File::save(int tags, bool stripOthers)
{
  return save(tags, stripOthers, 4);
}

bool MPEG::File::save(int tags, bool stripOthers, int id3v2Version)
{
  return save(tags, stripOthers, id3v2Version, true);
}

bool MPEG::File::save(int tags, bool stripOthers, int id3v2Version, bool duplicateTags)
{
  if(tags == NoTags && stripOthers)
    return strip(AllTags);

  if(!ID3v2Tag() && !ID3v1Tag() && !APETag()) {

    if((d->hasID3v1 || d->hasID3v2 || d->hasAPE) && stripOthers)
      return strip(AllTags);

    return true;
  }

  if(readOnly()) {
    debug("MPEG::File::save() -- File is read only.");
    return false;
  }

  // Create the tags if we've been asked to.

  if (duplicateTags) {

    // Copy the values from the tag that does exist into the new tag,
    // except if the existing tag is to be stripped.

    if((tags & ID3v2) && ID3v1Tag() && !(stripOthers && !(tags & ID3v1)))
      Tag::duplicate(ID3v1Tag(), ID3v2Tag(true), false);

    if((tags & ID3v1) && d->tag[ID3v2Index] && !(stripOthers && !(tags & ID3v2)))
      Tag::duplicate(ID3v2Tag(), ID3v1Tag(true), false);
  }

  bool success = true;

  if(ID3v2 & tags) {
    if(ID3v2Tag() && !ID3v2Tag()->isEmpty()) {
      if(!d->hasID3v2)
        d->ID3v2Location = 0;

      insert(ID3v2Tag()->render(id3v2Version), d->ID3v2Location, d->ID3v2OriginalSize);

      const long prevOriginalSize = d->ID3v2OriginalSize;
      d->ID3v2OriginalSize = ID3v2Tag()->header()->completeTagSize();
      d->hasID3v2 = true;

      // v1 tag location has changed, update if it exists

      if(d->ID3v1Location >= 0)
        d->ID3v1Location += (d->ID3v2OriginalSize - prevOriginalSize);

      // APE tag location has changed, update if it exists

      if(d->APELocation >= 0)
        d->APELocation += (d->ID3v2OriginalSize - prevOriginalSize);
    }
    else if(stripOthers) {
      success = strip(ID3v2, false) && success;
    }
  }
  else if(d->hasID3v2 && stripOthers) {
    success = strip(ID3v2) && success;
  }

  if(ID3v1 & tags) {
    if(ID3v1Tag() && !ID3v1Tag()->isEmpty()) {
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
    else if(stripOthers) {
      success = strip(ID3v1) && success;
    }
  }
  else if(d->hasID3v1 && stripOthers) {
    success = strip(ID3v1, false) && success;
  }

  // Dont save an APE-tag unless one has been created

  if((APE & tags) && APETag()) {
    if(!d->hasAPE) {
      if(d->hasID3v1) {
        d->APELocation = d->ID3v1Location;
      }
      else {
        seek(0, End);
        d->APELocation = tell();
      }
    }

    insert(APETag()->render(), d->APELocation, d->APEOriginalSize);

    const long prevOriginalSize = d->APEOriginalSize;
    d->APEOriginalSize = APETag()->footer()->completeTagSize();
    d->hasAPE = true;

    // v1 tag location has changed, update if it exists

    if(d->ID3v1Location >= 0)
      d->ID3v1Location += (d->APEOriginalSize - prevOriginalSize);
  }
  else if(d->hasAPE && stripOthers) {
    success = strip(APE, false) && success;
  }

  return success;
}

ID3v2::Tag *MPEG::File::ID3v2Tag(bool create)
{
  return d->tag.access<ID3v2::Tag>(ID3v2Index, create);
}

ID3v1::Tag *MPEG::File::ID3v1Tag(bool create)
{
  return d->tag.access<ID3v1::Tag>(ID3v1Index, create);
}

APE::Tag *MPEG::File::APETag(bool create)
{
  return d->tag.access<APE::Tag>(APEIndex, create);
}

bool MPEG::File::strip(int tags)
{
  return strip(tags, true);
}

bool MPEG::File::strip(int tags, bool freeMemory)
{
  if(readOnly()) {
    debug("MPEG::File::strip() - Cannot strip tags from a read only file.");
    return false;
  }

  if((tags & ID3v2) && d->hasID3v2) {
    removeBlock(d->ID3v2Location, d->ID3v2OriginalSize);

    const long removedSize = d->ID3v2OriginalSize;
    d->ID3v2Location = -1;
    d->ID3v2OriginalSize = 0;
    d->hasID3v2 = false;

    if(freeMemory)
      d->tag.set(ID3v2Index, 0);

    // v1 tag location has changed, update if it exists

    if(d->ID3v1Location >= 0)
      d->ID3v1Location -= removedSize;

    // APE tag location has changed, update if it exists

    if(d->APELocation >= 0)
      d->APELocation -= removedSize;
  }

  if((tags & ID3v1) && d->hasID3v1) {
    removeBlock(d->ID3v1Location, 128);
    d->ID3v1Location = -1;
    d->hasID3v1 = false;

    if(freeMemory)
      d->tag.set(ID3v1Index, 0);
  }

  if((tags & APE) && d->hasAPE) {
    removeBlock(d->APELocation, d->APEOriginalSize);

    const long removedSize = d->APEOriginalSize;
    d->APELocation = -1;
    d->APEOriginalSize = 0;
    d->hasAPE = false;

    if(freeMemory)
      d->tag.set(APEIndex, 0);

    // v1 tag location has changed, update if it exists

    if(d->ID3v1Location >= 0)
      d->ID3v1Location -= removedSize;
  }

  return true;
}

void MPEG::File::setID3v2FrameFactory(const ID3v2::FrameFactory *factory)
{
  d->ID3v2FrameFactory = factory;
}

long MPEG::File::nextFrameOffset(long position)
{
  bool foundLastSyncPattern = false;

  ByteVector buffer;

  while(true) {
    seek(position);
    buffer = readBlock(bufferSize());

    if(buffer.size() <= 0)
      return -1;

    if(foundLastSyncPattern && secondSynchByte(buffer[0]))
      return position - 1;

    for(uint i = 0; i < buffer.size() - 1; i++) {
      if(firstSynchByte(buffer[i]) && secondSynchByte(buffer[i + 1]))
        return position + i;
    }

    foundLastSyncPattern = firstSynchByte(buffer[buffer.size() - 1]);
    position += buffer.size();
  }
}

long MPEG::File::previousFrameOffset(long position)
{
  bool foundFirstSyncPattern = false;
  ByteVector buffer;

  while (position > 0) {
    long size = ulong(position) < bufferSize() ? position : bufferSize();
    position -= size;

    seek(position);
    buffer = readBlock(size);

    if(buffer.size() <= 0)
      break;

    if(foundFirstSyncPattern && firstSynchByte(buffer[buffer.size() - 1]))
      return position + buffer.size() - 1;

    for(int i = buffer.size() - 2; i >= 0; i--) {
      if(firstSynchByte(buffer[i]) && secondSynchByte(buffer[i + 1]))
        return position + i;
    }

    foundFirstSyncPattern = secondSynchByte(buffer[0]);
  }
  return -1;
}

long MPEG::File::firstFrameOffset()
{
  long position = 0;

  if(d->hasID3v2)
    position = d->ID3v2Location + d->ID3v2OriginalSize;

  return nextFrameOffset(position);
}

long MPEG::File::lastFrameOffset()
{
  long position;

  if(d->hasAPE)
    position = d->APELocation - 1;
  else if(d->hasID3v1)
    position = d->ID3v1Location - 1;
  else
    position = length();

  return previousFrameOffset(position);
}

bool MPEG::File::hasID3v1Tag() const
{
  return d->hasID3v1;
}

bool MPEG::File::hasID3v2Tag() const
{
  return d->hasID3v2;
}

bool MPEG::File::hasAPETag() const
{
  return d->hasAPE;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MPEG::File::read(bool readProperties, Properties::ReadStyle propertiesStyle)
{
  // Look for an ID3v2 tag

  d->ID3v2Location = findID3v2();

  if(d->ID3v2Location >= 0) {

    d->tag.set(ID3v2Index, new ID3v2::Tag(this, d->ID3v2Location, d->ID3v2FrameFactory));

    d->ID3v2OriginalSize = ID3v2Tag()->header()->completeTagSize();

    if(ID3v2Tag()->header()->tagSize() <= 0)
      d->tag.set(ID3v2Index, 0);
    else
      d->hasID3v2 = true;
  }

  // Look for an ID3v1 tag

  d->ID3v1Location = findID3v1();

  if(d->ID3v1Location >= 0) {
    d->tag.set(ID3v1Index, new ID3v1::Tag(this, d->ID3v1Location));
    d->hasID3v1 = true;
  }

  // Look for an APE tag

  d->APELocation = findAPE();

  if(d->APELocation >= 0) {

    d->tag.set(APEIndex, new APE::Tag(this, d->APELocation));

    d->APEOriginalSize = APETag()->footer()->completeTagSize();
    d->APELocation = d->APELocation + APETag()->footer()->size() - d->APEOriginalSize;
    d->hasAPE = true;
  }

  if(readProperties)
    d->properties = new Properties(this, propertiesStyle);

  // Make sure that we have our default tag types available.

  ID3v2Tag(true);
  ID3v1Tag(true);
}

long MPEG::File::findID3v2()
{
  if(!isValid())
    return -1;

  // Look for an ID3v2 tag located before the first MPEG frame.

  // An ID3v2 tag or an MPEG frame is most likely at the beginning of the file.
  // It's worth it to have a special check first.

  seek(0);

  const ByteVector buffer = readBlock(3);
  if(buffer.size() < 3)
    return -1;

  if(buffer == ID3v2::Header::fileIdentifier())
    return 0;

  if(firstSynchByte(buffer[0]) && secondSynchByte(buffer[1]))
    return -1;

  // Neither an ID3v2 tag nor MPEG frame at the beginning of the file.
  // It's a fall-back option, so have an easy check ignoring its inefficiency.

  const long tagOffset = find(ID3v2::Header::fileIdentifier());
  if(tagOffset < 0)
    return -1;

  const long frameOffset = nextFrameOffset(0);
  if(frameOffset < 0 || tagOffset < frameOffset)
    return tagOffset;

  return -1;
}

long MPEG::File::findID3v1()
{
  if(!isValid())
    return -1;

  seek(-128, End);
  const long location = tell();

  if(readBlock(3) == ID3v1::Tag::fileIdentifier())
    return location;
  else
    return -1;
}

long MPEG::File::findAPE()
{
  if(!isValid())
    return -1;

  if(d->hasID3v1)
    seek(-160, End);
  else
    seek(-32, End);

  const long location = tell();

  if(readBlock(8) == APE::Tag::fileIdentifier())
    return location;
  else
    return -1;
}
