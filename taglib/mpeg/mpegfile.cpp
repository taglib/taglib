/***************************************************************************
    copyright            : (C) 2002, 2003 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#include <id3v2tag.h>
#include <id3v2header.h>
#include <id3v1tag.h>
#include <apefooter.h>
#include <apetag.h>
#include <tdebug.h>

#include <bitset>

#include "mpegfile.h"
#include "mpegheader.h"

using namespace TagLib;

namespace TagLib {
  /*!
   * A union of the ID3v2 and ID3v1 tags.
   */
  class MPEGTag : public TagLib::Tag
  {
  public:
    MPEGTag(MPEG::File *f) : TagLib::Tag(), file(f) {}

    virtual String title() const
    {
      if(file->ID3v2Tag() && !file->ID3v2Tag()->title().isEmpty())
        return file->ID3v2Tag()->title();

      if(file->ID3v1Tag())
        return file->ID3v1Tag()->title();

      return String::null;
    }

    virtual String artist() const
    {
      if(file->ID3v2Tag() && !file->ID3v2Tag()->artist().isEmpty())
        return file->ID3v2Tag()->artist();

      if(file->ID3v1Tag())
        return file->ID3v1Tag()->artist();

      return String::null;
    }

    virtual String album() const
    {
      if(file->ID3v2Tag() && !file->ID3v2Tag()->album().isEmpty())
        return file->ID3v2Tag()->album();

      if(file->ID3v1Tag())
        return file->ID3v1Tag()->album();

      return String::null;
    }

    virtual String comment() const
    {
      if(file->ID3v2Tag() && !file->ID3v2Tag()->comment().isEmpty())
        return file->ID3v2Tag()->comment();

      if(file->ID3v1Tag())
        return file->ID3v1Tag()->comment();

      return String::null;
    }

    virtual String genre() const
    {
      if(file->ID3v2Tag() && !file->ID3v2Tag()->genre().isEmpty())
        return file->ID3v2Tag()->genre();

      if(file->ID3v1Tag())
        return file->ID3v1Tag()->genre();

      return String::null;
    }

    virtual uint year() const
    {
      if(file->ID3v2Tag() && file->ID3v2Tag()->year() > 0)
        return file->ID3v2Tag()->year();

      if(file->ID3v1Tag())
        return file->ID3v1Tag()->year();

      return 0;
    }

    virtual uint track() const
    {
      if(file->ID3v2Tag() && file->ID3v2Tag()->track() > 0)
        return file->ID3v2Tag()->track();

      if(file->ID3v1Tag())
        return file->ID3v1Tag()->track();

      return 0;
    }

    virtual void setTitle(const String &s)
    {
      file->ID3v2Tag(true)->setTitle(s);
      file->ID3v1Tag(true)->setTitle(s);
    }

    virtual void setArtist(const String &s)
    {
      file->ID3v2Tag(true)->setArtist(s);
      file->ID3v1Tag(true)->setArtist(s);
    }

    virtual void setAlbum(const String &s)
    {
      file->ID3v2Tag(true)->setAlbum(s);
      file->ID3v1Tag(true)->setAlbum(s);
    }

    virtual void setComment(const String &s)
    {
      file->ID3v2Tag(true)->setComment(s);
      file->ID3v1Tag(true)->setComment(s);
    }

    virtual void setGenre(const String &s)
    {
      file->ID3v2Tag(true)->setGenre(s);
      file->ID3v1Tag(true)->setGenre(s);
    }

    virtual void setYear(uint i)
    {
      file->ID3v2Tag(true)->setYear(i);
      file->ID3v1Tag(true)->setYear(i);
    }

    virtual void setTrack(uint i)
    {
      file->ID3v2Tag(true)->setTrack(i);
      file->ID3v1Tag(true)->setTrack(i);
    }

  private:
    MPEG::File *file;
  };
}

class MPEG::File::FilePrivate
{
public:
  FilePrivate(ID3v2::FrameFactory *frameFactory = ID3v2::FrameFactory::instance()) :
    ID3v2FrameFactory(frameFactory),
    ID3v2Tag(0),
    ID3v2Location(-1),
    ID3v2OriginalSize(0),
    APETag(0),
    APELocation(-1),
    APEOriginalSize(0),
    ID3v1Tag(0),
    ID3v1Location(-1),
    tag(0),
    hasID3v2(false),
    hasID3v1(false),
    hasAPE(false),
    properties(0) {}

  ~FilePrivate() {
    delete ID3v2Tag;
    delete ID3v1Tag;
    delete tag;
    delete properties;
  }

  const ID3v2::FrameFactory *ID3v2FrameFactory;
  ID3v2::Tag *ID3v2Tag;
  long ID3v2Location;
  uint ID3v2OriginalSize;

  APE::Tag *APETag;
  long APELocation;
  uint APEOriginalSize;

  ID3v1::Tag *ID3v1Tag;
  long ID3v1Location;

  MPEGTag *tag;

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

MPEG::File::File(const char *file, bool readProperties,
                 Properties::ReadStyle propertiesStyle) : TagLib::File(file)
{
  d = new FilePrivate;
  if(isOpen()) {
    d->tag = new MPEGTag(this);
    read(readProperties, propertiesStyle);
  }
}

MPEG::File::File(const char *file, ID3v2::FrameFactory *frameFactory,
                 bool readProperties, Properties::ReadStyle propertiesStyle) :
  TagLib::File(file)
{
  d = new FilePrivate(frameFactory);
  if(isOpen()) {
    d->tag = new MPEGTag(this);
    read(readProperties, propertiesStyle);
  }
}

MPEG::File::~File()
{
  delete d;
}

TagLib::Tag *MPEG::File::tag() const
{
  return d->tag;
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
  if(tags == NoTags && stripOthers)
    return strip(AllTags);

  if(!d->ID3v2Tag && !d->ID3v1Tag && !d->APETag) {

    if((d->hasID3v1 || d->hasID3v2 || d->hasAPE) && stripOthers)
      return strip(AllTags);

    return true;
  }

  if(readOnly()) {
    debug("MPEG::File::save() -- File is read only.");
    return false;
  }

  // Create the tags if we've been asked to.  Copy the values from the tag that
  // does exist into the new tag.

  if((tags & ID3v2) && d->ID3v1Tag)
    Tag::duplicate(d->ID3v1Tag, ID3v2Tag(true), false);

  if((tags & ID3v1) && d->ID3v2Tag)
    Tag::duplicate(d->ID3v2Tag, ID3v1Tag(true), false);

  bool success = true;

  if(ID3v2 & tags) {

    if(d->ID3v2Tag && !d->ID3v2Tag->isEmpty()) {

      if(!d->hasID3v2)
        d->ID3v2Location = 0;

      insert(d->ID3v2Tag->render(), d->ID3v2Location, d->ID3v2OriginalSize);
    }
    else if(stripOthers)
      success = strip(ID3v2, false) && success;
  }
  else if(d->hasID3v2 && stripOthers)
    success = strip(ID3v2) && success;

  if(ID3v1 & tags) {
    if(d->ID3v1Tag && !d->ID3v1Tag->isEmpty()) {
      int offset = d->hasID3v1 ? -128 : 0;
      seek(offset, End);
      writeBlock(d->ID3v1Tag->render());
    }
    else if(stripOthers)
      success = strip(ID3v1) && success;
  }
  else if(d->hasID3v1 && stripOthers)
    success = strip(ID3v1, false) && success;

  // Dont save an APE-tag unless one has been created

  if((APE & tags) && d->APETag) {
    if(d->hasAPE)
      insert(d->APETag->render(), d->APELocation, d->APEOriginalSize);
    else {
      if(d->hasID3v1)  {
        insert(d->APETag->render(), d->ID3v1Location, 0);
        d->APEOriginalSize = d->APETag->footer()->completeTagSize();
        d->hasAPE = true;
        d->APELocation = d->ID3v1Location;
        d->ID3v1Location += d->APEOriginalSize;
      }
      else {
        seek(0, End);
        d->APELocation = tell();
        writeBlock(d->APETag->render());
        d->APEOriginalSize = d->APETag->footer()->completeTagSize();
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
  if(!create || d->ID3v2Tag)
    return d->ID3v2Tag;

  // no ID3v2 tag exists and we've been asked to create one

  d->ID3v2Tag = new ID3v2::Tag;
  return d->ID3v2Tag;
}

ID3v1::Tag *MPEG::File::ID3v1Tag(bool create)
{
  if(!create || d->ID3v1Tag)
    return d->ID3v1Tag;

  // no ID3v1 tag exists and we've been asked to create one

  d->ID3v1Tag = new ID3v1::Tag;
  return d->ID3v1Tag;
}

APE::Tag *MPEG::File::APETag(bool create)
{
  if(!create || d->APETag)
    return d->APETag;

  // no APE tag exists and we've been asked to create one

  d->APETag = new APE::Tag;
  return d->APETag;
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
    if(freeMemory) {
      delete d->ID3v2Tag;
      d->ID3v2Tag = 0;
    }

    // v1 tag location has changed, update if it exists
    if(d->ID3v1Tag)
      d->ID3v1Location = findID3v1();
  }

  if((tags & ID3v1) && d->hasID3v1) {
    truncate(d->ID3v1Location);
    d->ID3v1Location = -1;
    d->hasID3v1 = false;
    if(freeMemory) {
      delete d->ID3v1Tag;
      d->ID3v1Tag = 0;
    }
  }

  if((tags & APE) && d->hasAPE) {
    removeBlock(d->APELocation, d->APEOriginalSize);
    d->APELocation = -1;
    d->hasAPE = false;
    if(d->hasID3v1) {
      if (d->ID3v1Location > d->APELocation)
        d->ID3v1Location -= d->APEOriginalSize;
    }
    if(freeMemory) {
      delete d->APETag;
      d->APETag = 0;
    }
  }

  return true;
}

void MPEG::File::setID3v2FrameFactory(const ID3v2::FrameFactory *factory)
{
  d->ID3v2FrameFactory = factory;
}

long MPEG::File::nextFrameOffset(long position)
{
  // TODO: This will miss syncs spanning buffer read boundaries.

  ByteVector buffer = readBlock(bufferSize());

  while(buffer.size() > 0) {
    seek(position);
    buffer = readBlock(bufferSize());

    for(uint i = 0; i < buffer.size() - 1; i++) {
      if(uchar(buffer[i]) == 0xff && secondSynchByte(buffer[i + 1]))
    	return position + i;
    }
    position += bufferSize();
  }

  return -1;
}

long MPEG::File::previousFrameOffset(long position)
{
  // TODO: This will miss syncs spanning buffer read boundaries.

  while(int(position - bufferSize()) > int(bufferSize())) {
    position -= bufferSize();
    seek(position);
    ByteVector buffer = readBlock(bufferSize());

    // If the amount of data is smaller than an MPEG header (4 bytes) there's no
    // chance of this being valid.

    if(buffer.size() < 4)
      return -1;

    for(int i = buffer.size() - 2; i >= 0; i--) {
      if(uchar(buffer[i]) == 0xff && secondSynchByte(buffer[i + 1]))
        return position + i;
    }
  }

  return -1;
}

long MPEG::File::firstFrameOffset()
{
  long position = 0;

  if(d->ID3v2Tag)
    position = d->ID3v2Location + d->ID3v2Tag->header()->completeTagSize();

  return nextFrameOffset(position);
}

long MPEG::File::lastFrameOffset()
{
  return previousFrameOffset(d->ID3v1Tag ? d->ID3v1Location - 1 : length());
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MPEG::File::read(bool readProperties, Properties::ReadStyle propertiesStyle)
{
  // Look for an ID3v2 tag

  d->ID3v2Location = findID3v2();

  if(d->ID3v2Location >= 0) {

    d->ID3v2Tag = new ID3v2::Tag(this, d->ID3v2Location, d->ID3v2FrameFactory);

    d->ID3v2OriginalSize = d->ID3v2Tag->header()->completeTagSize();

    if(d->ID3v2Tag->header()->tagSize() <= 0) {
      delete d->ID3v2Tag;
      d->ID3v2Tag = 0;
    }
    else
      d->hasID3v2 = true;
  }

  // Look for an ID3v1 tag

  d->ID3v1Location = findID3v1();

  if(d->ID3v1Location >= 0) {
    d->ID3v1Tag = new ID3v1::Tag(this, d->ID3v1Location);
    d->hasID3v1 = true;
  }

  // Look for an APE tag

  d->APELocation = findAPE();

  if(d->APELocation >= 0) {

    d->APETag = new APE::Tag(this, d->APELocation);

    d->APEOriginalSize = d->APETag->footer()->completeTagSize();

    d->APELocation = d->APELocation + d->APETag->footer()->size() - d->APEOriginalSize;

    d->hasAPE = true;
  }

  if(readProperties)
    d->properties = new Properties(this, propertiesStyle);
}

long MPEG::File::findID3v2()
{
  // This method is based on the contents of TagLib::File::find(), but because
  // of some subtlteies -- specifically the need to look for the bit pattern of
  // an MPEG sync, it has been modified for use here.

  if(isValid() && ID3v2::Header::fileIdentifier().size() <= bufferSize()) {

    // The position in the file that the current buffer starts at.

    long bufferOffset = 0;
    ByteVector buffer;

    // These variables are used to keep track of a partial match that happens at
    // the end of a buffer.

    int previousPartialMatch = -1;
    bool previousPartialSynchMatch = false;

    // Save the location of the current read pointer.  We will restore the
    // position using seek() before all returns.

    long originalPosition = tell();

    // Start the search at the beginning of the file.

    seek(0);

    // This loop is the crux of the find method.  There are three cases that we
    // want to account for:
    // (1) The previously searched buffer contained a partial match of the search
    // pattern and we want to see if the next one starts with the remainder of
    // that pattern.
    //
    // (2) The search pattern is wholly contained within the current buffer.
    //
    // (3) The current buffer ends with a partial match of the pattern.  We will
    // note this for use in the next itteration, where we will check for the rest
    // of the pattern.

    for(buffer = readBlock(bufferSize()); buffer.size() > 0; buffer = readBlock(bufferSize())) {

      // (1) previous partial match

      if(previousPartialSynchMatch && secondSynchByte(buffer[0]))
        return -1;

      if(previousPartialMatch >= 0 && int(bufferSize()) > previousPartialMatch) {
        const int patternOffset = (bufferSize() - previousPartialMatch);
        if(buffer.containsAt(ID3v2::Header::fileIdentifier(), 0, patternOffset)) {
          seek(originalPosition);
          return bufferOffset - bufferSize() + previousPartialMatch;
        }
      }

      // (2) pattern contained in current buffer

      long location = buffer.find(ID3v2::Header::fileIdentifier());
      if(location >= 0) {
        seek(originalPosition);
        return bufferOffset + location;
      }

      int firstSynchByte = buffer.find(char(uchar(255)));

      // Here we have to loop because there could be several of the first
      // (11111111) byte, and we want to check all such instances until we find
      // a full match (11111111 111) or hit the end of the buffer.

      while(firstSynchByte >= 0) {

        // if this *is not* at the end of the buffer

        if(firstSynchByte < int(buffer.size()) - 1) {
          if(secondSynchByte(buffer[firstSynchByte + 1])) {
            // We've found the frame synch pattern.
            seek(originalPosition);
            return -1;
          }
          else {

            // We found 11111111 at the end of the current buffer indicating a
            // partial match of the synch pattern.  The find() below should
            // return -1 and break out of the loop.

            previousPartialSynchMatch = true;
          }
        }

        // Check in the rest of the buffer.

        firstSynchByte = buffer.find(char(uchar(255)), firstSynchByte + 1);
      }

      // (3) partial match

      previousPartialMatch = buffer.endsWithPartialMatch(ID3v2::Header::fileIdentifier());

      bufferOffset += bufferSize();
    }

    // Since we hit the end of the file, reset the status before continuing.

    clear();

    seek(originalPosition);
  }

  return -1;
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

long MPEG::File::findAPE()
{
  if(isValid()) {
    if (d->hasID3v1)
        seek(-160, End);
    else
        seek(-32, End);

    long p = tell();

    if(readBlock(8) == APE::Tag::fileIdentifier())
      return p;
  }
  return -1;
}

bool MPEG::File::secondSynchByte(char byte)
{
  if(uchar(byte) == 0xff)
    return false;

  std::bitset<8> b(byte);

  // check to see if the byte matches 111xxxxx
  return b.test(7) && b.test(6) && b.test(5);
}
