/***************************************************************************
    copyright            : (C) 2006 by Lukáš Lalinský
    email                : lalinsky@gmail.com

    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
                           (original MPC implementation)
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

#include "trueaudiofile.h"

#include "tdebug.h"
#include "tpropertymap.h"
#include "tagunion.h"
#include "tagutils.h"
#include "id3v1tag.h"
#include "id3v2tag.h"

using namespace TagLib;

namespace
{
  enum { TrueAudioID3v2Index = 0, TrueAudioID3v1Index = 1 };
} // namespace

class TrueAudio::File::FilePrivate
{
public:
  FilePrivate(const ID3v2::FrameFactory *frameFactory = ID3v2::FrameFactory::instance()) :
    ID3v2FrameFactory(frameFactory)
  {
  }

  const ID3v2::FrameFactory *ID3v2FrameFactory;
  offset_t ID3v2Location { -1 };
  long ID3v2OriginalSize { 0 };

  offset_t ID3v1Location { -1 };

  TagUnion tag;

  std::unique_ptr<Properties> properties;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

bool TrueAudio::File::isSupported(IOStream *stream)
{
  // A TrueAudio file has to start with "TTA". An ID3v2 tag may precede.

  const ByteVector id = Utils::readHeader(stream, 3, true);
  return id == "TTA";
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

TrueAudio::File::File(FileName file, bool readProperties,
                      Properties::ReadStyle,
                      ID3v2::FrameFactory *frameFactory) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(
    frameFactory ? frameFactory : ID3v2::FrameFactory::instance()))
{
  if(isOpen())
    read(readProperties);
}

TrueAudio::File::File(FileName file, ID3v2::FrameFactory *frameFactory,
                      bool readProperties, Properties::ReadStyle) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties);
}

TrueAudio::File::File(IOStream *stream, bool readProperties,
                      Properties::ReadStyle,
                      ID3v2::FrameFactory *frameFactory) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(
    frameFactory ? frameFactory : ID3v2::FrameFactory::instance()))
{
  if(isOpen())
    read(readProperties);
}

TrueAudio::File::File(IOStream *stream, ID3v2::FrameFactory *frameFactory,
                      bool readProperties, Properties::ReadStyle) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties);
}

TrueAudio::File::~File() = default;

TagLib::Tag *TrueAudio::File::tag() const
{
  return &d->tag;
}

PropertyMap TrueAudio::File::properties() const
{
  return d->tag.properties();
}

void TrueAudio::File::removeUnsupportedProperties(const StringList &unsupported)
{
  d->tag.removeUnsupportedProperties(unsupported);
}

PropertyMap TrueAudio::File::setProperties(const PropertyMap &properties)
{
  if(ID3v1Tag())
    ID3v1Tag()->setProperties(properties);

  return ID3v2Tag(true)->setProperties(properties);
}

TrueAudio::Properties *TrueAudio::File::audioProperties() const
{
  return d->properties.get();
}

bool TrueAudio::File::save()
{
  if(readOnly()) {
    debug("TrueAudio::File::save() -- File is read only.");
    return false;
  }

  // Update ID3v2 tag

  if(ID3v2Tag() && !ID3v2Tag()->isEmpty()) {

    // ID3v2 tag is not empty. Update the old one or create a new one.

    if(d->ID3v2Location < 0)
      d->ID3v2Location = 0;

    const ByteVector data = ID3v2Tag()->render();
    insert(data, d->ID3v2Location, d->ID3v2OriginalSize);

    if(d->ID3v1Location >= 0)
      d->ID3v1Location += static_cast<long>(data.size()) - d->ID3v2OriginalSize;

    d->ID3v2OriginalSize = data.size();
  }
  else {

    // ID3v2 tag is empty. Remove the old one.

    if(d->ID3v2Location >= 0) {
      removeBlock(d->ID3v2Location, d->ID3v2OriginalSize);

      if(d->ID3v1Location >= 0)
        d->ID3v1Location -= d->ID3v2OriginalSize;

      d->ID3v2Location = -1;
      d->ID3v2OriginalSize = 0;
    }
  }

  // Update ID3v1 tag

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

ID3v1::Tag *TrueAudio::File::ID3v1Tag(bool create)
{
  return d->tag.access<ID3v1::Tag>(TrueAudioID3v1Index, create);
}

ID3v2::Tag *TrueAudio::File::ID3v2Tag(bool create)
{
  return d->tag.access<ID3v2::Tag>(TrueAudioID3v2Index, create,
                                   d->ID3v2FrameFactory);
}

void TrueAudio::File::strip(int tags)
{
  if(tags & ID3v1)
    d->tag.set(TrueAudioID3v1Index, nullptr);

  if(tags & ID3v2)
    d->tag.set(TrueAudioID3v2Index, nullptr);

  if(!ID3v1Tag())
    ID3v2Tag(true);
}

bool TrueAudio::File::hasID3v1Tag() const
{
  return d->ID3v1Location >= 0;
}

bool TrueAudio::File::hasID3v2Tag() const
{
  return d->ID3v2Location >= 0;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void TrueAudio::File::read(bool readProperties)
{
  // Look for an ID3v2 tag

  d->ID3v2Location = Utils::findID3v2(this);

  if(d->ID3v2Location >= 0) {
    d->tag.set(TrueAudioID3v2Index, new ID3v2::Tag(this, d->ID3v2Location, d->ID3v2FrameFactory));
    d->ID3v2OriginalSize = ID3v2Tag()->header()->completeTagSize();
  }

  // Look for an ID3v1 tag

  d->ID3v1Location = Utils::findID3v1(this);

  if(d->ID3v1Location >= 0)
    d->tag.set(TrueAudioID3v1Index, new ID3v1::Tag(this, d->ID3v1Location));

  if(d->ID3v1Location < 0)
    ID3v2Tag(true);

  // Look for TrueAudio metadata

  if(readProperties) {

    offset_t streamLength;

    if(d->ID3v1Location >= 0)
      streamLength = d->ID3v1Location;
    else
      streamLength = length();

    if(d->ID3v2Location >= 0) {
      seek(d->ID3v2Location + d->ID3v2OriginalSize);
      streamLength -= d->ID3v2Location + d->ID3v2OriginalSize;
    }
    else {
      seek(0);
    }

    d->properties = std::make_unique<Properties>(readBlock(TrueAudio::HeaderSize), streamLength);
  }
}
