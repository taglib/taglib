/***************************************************************************
    copyright            : (C) 2003 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
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

#include <tbytevector.h>
#include <tstring.h>
#include <tdebug.h>

#include <id3v2header.h>

#include "flacfile.h"
#include "flactag.h"

using namespace TagLib;

class FLAC::File::FilePrivate
{
public:
  FilePrivate() :
    ID3v2FrameFactory(ID3v2::FrameFactory::instance()),
    ID3v2Tag(0),
    ID3v2Location(-1),
    ID3v2OriginalSize(0),
    ID3v1Tag(0),
    ID3v1Location(-1),
    comment(0),
    properties(0),
    flacStart(0),
    streamStart(0),
    streamLength(0),
    scanned(false),
    hasXiphComment(false),
    hasID3v2(false),
    hasID3v1(false) {}

  ~FilePrivate()
  {
    delete ID3v2Tag;
    delete ID3v1Tag;
    delete comment;
    delete properties;
  }

  const ID3v2::FrameFactory *ID3v2FrameFactory;
  ID3v2::Tag *ID3v2Tag;
  long ID3v2Location;
  uint ID3v2OriginalSize;

  ID3v1::Tag *ID3v1Tag;
  long ID3v1Location;

  Ogg::XiphComment *comment;

  FLAC::Tag *tag;

  Properties *properties;
  ByteVector streamInfoData;
  ByteVector xiphCommentData;
  long flacStart;
  long streamStart;
  long streamLength;
  bool scanned;

  bool hasXiphComment;
  bool hasID3v2;
  bool hasID3v1;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FLAC::File::File(const char *file, bool readProperties,
                 Properties::ReadStyle propertiesStyle) : TagLib::File(file)
{
  d = new FilePrivate;
  read(readProperties, propertiesStyle);
}

FLAC::File::~File()
{
  delete d;
}

TagLib::Tag *FLAC::File::tag() const
{
  return d->tag;
}

FLAC::Properties *FLAC::File::audioProperties() const
{
  return d->properties;
}


void FLAC::File::save()
{
  // Create new vorbis comments

  if(!d->comment) {
    d->comment = new Ogg::XiphComment;
    if(d->tag)
      Tag::duplicate(d->tag, d->comment, true);
  }

  d->xiphCommentData = d->comment->render();

  ByteVector v = ByteVector::fromUInt(d->xiphCommentData.size());

  // Set the type of the comment to be a Xiph / Vorbis comment
  // (See scan() for comments on header-format)
  v[0] = 4;
  v.append(d->xiphCommentData);


   // If file already have comment => find and update it
   //                       if not => insert one 
   // TODO: Search for padding and use that

  if(d->hasXiphComment) {
    long nextPageOffset = d->flacStart;
    seek(nextPageOffset);
    ByteVector header = readBlock(4);
    uint length = header.mid(1, 3).toUInt();

    nextPageOffset += length + 4;

    // Search through the remaining metadata

    char blocktype = header[0] & 0x7f;
    bool lastblock = header[0] & 0x80;

    while(!lastblock) {
      seek(nextPageOffset);

      header = readBlock(4);
      blocktype = header[0] & 0x7f;
      lastblock = header[0] & 0x80;
      length = header.mid(1, 3).toUInt();

      // Type is vorbiscomment
      if( blocktype == 4 ) {
        v[0] = header[0];
        insert(v, nextPageOffset, length + 4);
        break;
      }

      nextPageOffset += length + 4;
    }
  }
  else {
    long nextPageOffset = d->flacStart;

    seek(nextPageOffset);

    ByteVector header = readBlock(4);
    // char blockType = header[0] & 0x7f;
    bool lastBlock = header[0] & 0x80;
    uint length = header.mid(1, 3).toUInt();

    // If last block was last, make this one last

    if(lastBlock) { 

      // Copy the bottom seven bits into the new value

      ByteVector h(static_cast<char>(header[0] & 0x7F));
      insert(h, nextPageOffset, 1);

      // Set the last bit
      v[0] |= 0x80;
    }

    insert(v, nextPageOffset + length + 4, 0);
    d->hasXiphComment = true;
  }

  // Update ID3 tag

  if(d->hasID3v2 && d->ID3v2Tag)
    insert(d->ID3v2Tag->render(), d->ID3v2Location, d->ID3v2OriginalSize);

  if(d->hasID3v1 && d->ID3v1Tag) {
    seek(-128, End);
    writeBlock(d->ID3v1Tag->render());
  }

}

void FLAC::File::setID3v2FrameFactory(const ID3v2::FrameFactory *factory)
{
  d->ID3v2FrameFactory = factory;
}


////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void FLAC::File::read(bool readProperties, Properties::ReadStyle propertiesStyle)
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

  // Look for FLAC metadata, including vorbis comments

  scan();

  if(d->hasXiphComment)
    d->comment = new Ogg::XiphComment(xiphCommentData());

  if(d->hasXiphComment || d->hasID3v2 || d->hasID3v1)
    d->tag = new FLAC::Tag(d->comment, d->ID3v2Tag, d->ID3v1Tag);
  else
    d->tag = new FLAC::Tag(new Ogg::XiphComment);

  if(readProperties)
    d->properties = new Properties(streamInfoData(), streamLength(), propertiesStyle);
}

ByteVector FLAC::File::streamInfoData()
{
  scan();
  return d->streamInfoData;
}

ByteVector FLAC::File::xiphCommentData()
{
  scan();
  return d->xiphCommentData;
}

long FLAC::File::streamLength()
{
  return d->streamLength;
}

void FLAC::File::scan()
{
  // Scan the metadata pages

  if(d->scanned)
    return;

  if(!isValid())
    return;

  // Optimization: Use ID3v2 size and only skip that

  long nextPageOffset = find("fLaC");

  if(nextPageOffset < 0) {
    debug("FLAC::File::scan() -- FLAC stream not found");
    return;
  }

  nextPageOffset += 4;
  d->flacStart = nextPageOffset;

  seek(nextPageOffset);

  ByteVector header = readBlock(4);

  // Header format (from spec):
  // <1> Last-metadata-block flag
  // <7> BLOCK_TYPE
  //	0 : STREAMINFO
  //    1 : PADDING
  //    ..
  //    4 : VORBIS_COMMENT
  //    ..
  // <24> Length of metadata to follow

  char blockType = header[0] & 0x7f;
  bool lastBlock = header[0] & 0x80;
  uint length = header.mid(1, 3).toUInt();

  // First block should be the stream_info metadata
  if(blockType != 0) { 
    debug("FLAC::File::scan() -- invalid FLAC stream");
    return;
  }

  d->streamInfoData = readBlock(length);
  nextPageOffset += length + 4;

  // Search through the remaining metadata

  while(!lastBlock) {

    header = readBlock(4);
    blockType = header[0] & 0x7f;
    lastBlock = header[0] & 0x80;
    length = header.mid(1, 3).toUInt();

    // Found the vorbis-comment
    if(blockType == 4) {
      d->xiphCommentData = readBlock(length);
      d->hasXiphComment = true;

    }

    nextPageOffset += length + 4;
    seek(nextPageOffset);
  }
  
  // End of metadata, now comes the datastream
  d->streamStart = nextPageOffset;
  d->streamLength = File::length() - d->streamStart;

  d->scanned = true;
}

long FLAC::File::findID3v1()
{
  if(!isValid())
    return -1;

  seek(-128, End);
  long p = tell();

  if(readBlock(3) == ID3v1::Tag::fileIdentifier())
    return p;

  return -1;
}

long FLAC::File::findID3v2()
{
  if(!isValid())
    return -1;

  seek(0);

  if(readBlock(3) == ID3v2::Header::fileIdentifier())
    return 0;

  return -1;
}
