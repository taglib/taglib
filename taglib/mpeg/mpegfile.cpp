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

#include "mpegfile.h"
#include "mpegheader.h"
#include "tpropertymap.h"

using namespace TagLib;

namespace
{
  /*!
   * MPEG frames can be recognized by the bit pattern 11111111 111, so the
   * first byte is easy to check for, however checking to see if the second byte
   * starts with \e 111 is a bit more tricky, hence these functions.
   */

  inline bool firstSyncByte(uchar byte)
  {
    return (byte == 0xFF);
  }

  inline bool secondSynchByte(uchar byte)
  {
    return ((byte & 0xE0) == 0xE0);
  }
}

namespace
{
  enum { ID3v2Index = 0, APEIndex = 1, ID3v1Index = 2 };
}

class MPEG::File::FilePrivate
{
public:
  FilePrivate(ID3v2::FrameFactory *frameFactory = ID3v2::FrameFactory::instance()) :
    ID3v2FrameFactory(frameFactory),
    ID3v2Location(-1),
    ID3v2OriginalSize(0),
    APELocation(-1),
    APEFooterLocation(-1),
    APEOriginalSize(0),
    ID3v1Location(-1),
    hasID3v2(false),
    hasID3v1(false),
    hasAPE(false),
    properties(0)
  {
  }

  ~FilePrivate()
  {
    delete properties;
  }

  const ID3v2::FrameFactory *ID3v2FrameFactory;

  long ID3v2Location;
  uint ID3v2OriginalSize;

  long APELocation;
  long APEFooterLocation;
  uint APEOriginalSize;

  long ID3v1Location;

  TagUnion tag;

  // These indicate whether the file *on disk* has these tags, not if
  // this data structure does.  This is used in computing offsets.

  bool hasID3v2;
  bool hasID3v1;
  bool hasAPE;

  Properties *properties;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPEG::File::File(FileName file, bool readProperties, Properties::ReadStyle) :
  TagLib::File(file),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties);
}

MPEG::File::File(FileName file, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle) :
  TagLib::File(file),
  d(new FilePrivate(frameFactory))
{
  if(isOpen())
    read(readProperties);
}

MPEG::File::File(IOStream *stream, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle) :
  TagLib::File(stream),
  d(new FilePrivate(frameFactory))
{
  if(isOpen())
    read(readProperties);
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

      d->hasID3v2 = true;

      // v1 tag location has changed, update if it exists

      if(ID3v1Tag())
        d->ID3v1Location = findID3v1();

      // APE tag location has changed, update if it exists

      if(APETag())
        findAPE();
    }
    else if(stripOthers)
      success = strip(ID3v2, false) && success;
  }
  else if(d->hasID3v2 && stripOthers)
    success = strip(ID3v2) && success;

  if(ID3v1 & tags) {
    if(ID3v1Tag() && !ID3v1Tag()->isEmpty()) {
      int offset = d->hasID3v1 ? -128 : 0;
      seek(offset, End);
      writeBlock(ID3v1Tag()->render());
      d->hasID3v1 = true;
      d->ID3v1Location = findID3v1();
    }
    else if(stripOthers)
      success = strip(ID3v1) && success;
  }
  else if(d->hasID3v1 && stripOthers)
    success = strip(ID3v1, false) && success;

  // Dont save an APE-tag unless one has been created

  if((APE & tags) && APETag()) {
    if(d->hasAPE)
      insert(APETag()->render(), d->APELocation, d->APEOriginalSize);
    else {
      if(d->hasID3v1) {
        insert(APETag()->render(), d->ID3v1Location, 0);
        d->APEOriginalSize = APETag()->footer()->completeTagSize();
        d->hasAPE = true;
        d->APELocation = d->ID3v1Location;
        d->ID3v1Location += d->APEOriginalSize;
      }
      else {
        seek(0, End);
        d->APELocation = tell();
        APE::Tag *apeTag = d->tag.access<APE::Tag>(APEIndex, false);
        d->APEFooterLocation = d->APELocation
                               + apeTag->footer()->completeTagSize()
                               - APE::Footer::size();
        writeBlock(APETag()->render());
        d->APEOriginalSize = APETag()->footer()->completeTagSize();
        d->hasAPE = true;
      }
    }
  }
  else if(d->hasAPE && stripOthers)
    success = strip(APE, false) && success;

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
    d->ID3v2Location = -1;
    d->ID3v2OriginalSize = 0;
    d->hasID3v2 = false;

    if(freeMemory)
      d->tag.set(ID3v2Index, 0);

    // v1 tag location has changed, update if it exists

    if(ID3v1Tag())
      d->ID3v1Location = findID3v1();

    // APE tag location has changed, update if it exists

   if(APETag())
      findAPE();
  }

  if((tags & ID3v1) && d->hasID3v1) {
    truncate(d->ID3v1Location);
    d->ID3v1Location = -1;
    d->hasID3v1 = false;

    if(freeMemory)
      d->tag.set(ID3v1Index, 0);
  }

  if((tags & APE) && d->hasAPE) {
    removeBlock(d->APELocation, d->APEOriginalSize);
    d->APELocation = -1;
    d->APEFooterLocation = -1;
    d->hasAPE = false;
    if(d->hasID3v1) {
      if(d->ID3v1Location > d->APELocation)
        d->ID3v1Location -= d->APEOriginalSize;
    }

    if(freeMemory)
      d->tag.set(APEIndex, 0);
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
      if(firstSyncByte(buffer[i]) && secondSynchByte(buffer[i + 1]))
        return position + i;
    }

    foundLastSyncPattern = firstSyncByte(buffer[buffer.size() - 1]);
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

    if(foundFirstSyncPattern && firstSyncByte(buffer[buffer.size() - 1]))
      return position + buffer.size() - 1;

    for(int i = buffer.size() - 2; i >= 0; i--) {
      if(firstSyncByte(buffer[i]) && secondSynchByte(buffer[i + 1]))
        return position + i;
    }

    foundFirstSyncPattern = secondSynchByte(buffer[0]);
  }
  return -1;
}

long MPEG::File::firstFrameOffset()
{
  long position = 0;

  if(hasID3v2Tag())
    position = d->ID3v2Location + ID3v2Tag()->header()->completeTagSize();

  return nextFrameOffset(position);
}

long MPEG::File::lastFrameOffset()
{
  long position;

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

void MPEG::File::read(bool readProperties)
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

  findAPE();

  if(d->APELocation >= 0) {

    d->tag.set(APEIndex, new APE::Tag(this, d->APEFooterLocation));
    d->APEOriginalSize = APETag()->footer()->completeTagSize();
    d->hasAPE = true;
  }

  if(readProperties)
    d->properties = new Properties(this);

  // Make sure that we have our default tag types available.

  ID3v2Tag(true);
  ID3v1Tag(true);
}

long MPEG::File::findID3v2()
{
  if(!isValid())
    return -1;

  // An ID3v2 tag or MPEG frame is most likely be at the beginning of the file.

  const ByteVector headerID = ID3v2::Header::fileIdentifier();

  seek(0);

  const ByteVector data = readBlock(headerID.size());
  if(data.size() < headerID.size())
    return -1;

  if(data == headerID)
    return 0;

  if(firstSyncByte(data[0]) && secondSynchByte(data[1]))
    return -1;

  // Look for the entire file, if neither an MEPG frame or ID3v2 tag was found
  // at the beginning of the file.
  // We don't care about the inefficiency of the code, since this is a seldom case.

  const long tagOffset = find(headerID);
  if(tagOffset < 0)
    return -1;

  const long frameOffset = firstFrameOffset();
  if(frameOffset < tagOffset)
    return -1;

  return tagOffset;
}

long MPEG::File::findID3v1()
{
  if(isValid()) {
    seek(-128, End);
    long p = tell();

    if(readBlock(3) == ID3v1::Tag::fileIdentifier())
      return p;
  }
  return -1;
}

void MPEG::File::findAPE()
{
  if(isValid()) {
    seek(d->hasID3v1 ? -160 : -32, End);

    long p = tell();

    if(readBlock(8) == APE::Tag::fileIdentifier()) {
      d->APEFooterLocation = p;
      seek(d->APEFooterLocation);
      APE::Footer footer(readBlock(APE::Footer::size()));
      d->APELocation = d->APEFooterLocation - footer.completeTagSize()
                       + APE::Footer::size();
      return;
    }
  }

  d->APELocation = -1;
  d->APEFooterLocation = -1;
}
