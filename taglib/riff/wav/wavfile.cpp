/***************************************************************************
    copyright            : (C) 2008 by Scott Wheeler
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

#include "tbytevector.h"
#include "tdebug.h"
#include "tstringlist.h"
#include "tpropertymap.h"

#include "wavfile.h"
#include "id3v2tag.h"
#include "infotag.h"
#include "tagunion.h"

using namespace TagLib;

namespace
{
  enum { ID3v2Index = 0, InfoIndex = 1 };
}

class RIFF::WAV::File::FilePrivate
{
public:
  FilePrivate() :
    properties(0),
    tagChunkID("ID3 "),
    id3v2Count(0),
    infoCount(0) {}

  ~FilePrivate()
  {
    delete properties;
  }

  Properties *properties;
  TagUnion tag;

  ByteVector tagChunkID;
  int id3v2Count;
  int infoCount;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::WAV::File::File(FileName file, bool readProperties,
                       Properties::ReadStyle propertiesStyle) : RIFF::File(file, LittleEndian)
{
  d = new FilePrivate;
  if(isOpen())
    read(readProperties, propertiesStyle);
}

RIFF::WAV::File::File(IOStream *stream, bool readProperties,
                       Properties::ReadStyle propertiesStyle) : RIFF::File(stream, LittleEndian)
{
  d = new FilePrivate;
  if(isOpen())
    read(readProperties, propertiesStyle);
}

RIFF::WAV::File::~File()
{
  delete d;
}

ID3v2::Tag *RIFF::WAV::File::tag() const
{
  return ID3v2Tag();
}

ID3v2::Tag *RIFF::WAV::File::ID3v2Tag() const
{
  return d->tag.access<ID3v2::Tag>(ID3v2Index, false);
}

RIFF::Info::Tag *RIFF::WAV::File::InfoTag() const
{
  return d->tag.access<RIFF::Info::Tag>(InfoIndex, false);
}

PropertyMap RIFF::WAV::File::properties() const
{
  return tag()->properties();
}

void RIFF::WAV::File::removeUnsupportedProperties(const StringList &unsupported)
{
  tag()->removeUnsupportedProperties(unsupported);
}

PropertyMap RIFF::WAV::File::setProperties(const PropertyMap &properties)
{
  return tag()->setProperties(properties);
}

RIFF::WAV::Properties *RIFF::WAV::File::audioProperties() const
{
  return d->properties;
}

bool RIFF::WAV::File::save()
{
  return RIFF::WAV::File::save(AllTags);
}

bool RIFF::WAV::File::save(TagTypes tags, bool stripOthers, int id3v2Version)
{
  if(readOnly()) {
    debug("RIFF::WAV::File::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    debug("RIFF::WAV::File::save() -- Trying to save invalid file.");
    return false;
  }

  if(stripOthers)
    strip(static_cast<TagTypes>(AllTags & ~tags));

  if(tags & ID3v2) {
    const ID3v2::Tag *id3v2tag = d->tag.access<ID3v2::Tag>(ID3v2Index, false);
    if(d->id3v2Count >= 2 || id3v2tag->isEmpty()) {
      strip(ID3v2);
    }

    if(!id3v2tag->isEmpty()) {
      setChunkData(d->tagChunkID, id3v2tag->render(id3v2Version));
      d->id3v2Count = 1;
    }
  }

  if(tags & Info) {
    const Info::Tag *infotag = d->tag.access<Info::Tag>(InfoIndex, false);
    if(d->infoCount >= 2 || infotag->isEmpty()) {
      strip(Info);
    }

    if(!infotag->isEmpty()) {
      const ByteVector data = infotag->render();
      if(hasInfoTag())
        setChunkData(findInfoTagChunk(), data);
      else
        setChunkData("LIST", data, true);

      d->infoCount = 1;
    }
  }

  return true;
}

bool RIFF::WAV::File::hasID3v2Tag() const
{
  return (d->id3v2Count != 0);
}

bool RIFF::WAV::File::hasInfoTag() const
{
  return (d->infoCount != 0);
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::WAV::File::read(bool readProperties, Properties::ReadStyle propertiesStyle)
{
  ByteVector formatData;
  uint streamLength = 0;
  for(uint i = 0; i < chunkCount(); i++) {
    const ByteVector name = chunkName(i);
    if(name == "ID3 " || name == "id3 ") {
      d->id3v2Count++;
      if(!d->tag[ID3v2Index]) {
        d->tag.set(ID3v2Index, new ID3v2::Tag(this, chunkOffset(i)));
        d->tagChunkID = name;
      }
      else {
        debug("RIFF::WAV::File::read() - Duplicate ID3v2 tag found.");
      }
    }
    else if(name == "LIST") {
      const ByteVector data = chunkData(i);
      if(data.mid(0, 4) == "INFO") {
        d->infoCount++;
        if(!d->tag[InfoIndex]) {
          d->tag.set(InfoIndex, new RIFF::Info::Tag(data));
        }
        else {
          debug("RIFF::WAV::File::read() - Duplicate INFO tag found.");
        }
      }
    }
    else if(name == "fmt " && readProperties) {
      if(formatData.isEmpty()) {
        formatData = chunkData(i);
      }
      else {
        debug("RIFF::WAV::File::read() - Duplicate 'fmt ' chunk found.");
      }
    }
    else if(name == "data" && readProperties) {
      if(streamLength == 0) {
        streamLength = chunkDataSize(i);
      }
      else {
        debug("RIFF::WAV::File::read() - Duplicate 'data' chunk found.");
      }
    }
  }

  if(!d->tag[ID3v2Index])
    d->tag.set(ID3v2Index, new ID3v2::Tag);

  if(!d->tag[InfoIndex])
    d->tag.set(InfoIndex, new RIFF::Info::Tag);

  if(!formatData.isEmpty())
    d->properties = new Properties(formatData, streamLength, propertiesStyle);
}

void RIFF::WAV::File::strip(TagTypes tags)
{
  if(tags & ID3v2) {
    removeChunk("ID3 ");
    removeChunk("id3 ");
    d->id3v2Count = 0;
  }

  if(tags & Info) {
    removeInfoTagChunk();
    d->infoCount = 0;
  }
}

TagLib::uint RIFF::WAV::File::findInfoTagChunk()
{
  for(uint i = 0; i < chunkCount(); ++i) {
    if(chunkName(i) == "LIST" && chunkData(i).mid(0, 4) == "INFO") {
      return i;
    }
  }

  return TagLib::uint(-1);
}

void TagLib::RIFF::WAV::File::removeInfoTagChunk()
{
  TagLib::uint chunkId;
  while((chunkId = findInfoTagChunk()) != TagLib::uint(-1)) {
    removeChunk(chunkId);
  }
}
