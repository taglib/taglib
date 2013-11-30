/***************************************************************************
    copyright            : (C) 2013 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
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
#include <tpropertymap.h>
#include <tdebug.h>
#include <tsmartptr.h>

#include "rmp3file.h"
#include "rmp3properties.h"

using namespace TagLib;
using namespace RIFF;

namespace
{
  inline bool isSynchBytes(const char *byte)
  {
    return (byte[0] == '\xff' && (byte[1] & '\xe0') == '\xe0');
  }
}

class RMP3::File::FilePrivate
{
public:
  FilePrivate() :
    hasInfoTag(false) {}

  SCOPED_PTR<Info::Tag> tag;
  SCOPED_PTR<RMP3::AudioProperties> properties;

  bool hasInfoTag;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RMP3::File::File(FileName file, bool readProperties,
                 AudioProperties::ReadStyle propertiesStyle) : 
  RIFF::File(file, LittleEndian),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties, propertiesStyle);
}

RMP3::File::File(IOStream *stream, bool readProperties,
                 AudioProperties::ReadStyle propertiesStyle) : 
  RIFF::File(stream, LittleEndian),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties, propertiesStyle);
}

RMP3::File::~File()
{
  delete d;
}

Info::Tag *RMP3::File::tag() const
{
  return d->tag.get();
}

PropertyMap RMP3::File::properties() const
{
  return tag()->properties();
}

void RMP3::File::removeUnsupportedProperties(const StringList &unsupported)
{
  tag()->removeUnsupportedProperties(unsupported);
}

PropertyMap RMP3::File::setProperties(const PropertyMap &properties)
{
  return tag()->setProperties(properties);
}

RIFF::RMP3::AudioProperties *RMP3::File::audioProperties() const
{
  return d->properties.get();
}

bool RMP3::File::save()
{
  if(readOnly()) {
    debug("RIFF::RMP3::File::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    debug("RIFF::RMP3::File::save() -- Trying to save invalid file.");
    return false;
  }

  // Always removes the LIST/INFO chunk to relocate it at the end of the file
  // to let the "IID3" field work as a normal ID3v1 tag.

  for(uint i = 0; i < chunkCount(); ++i) {
    if(chunkName(i) == "LIST" && chunkData(i).mid(0, 4) == "INFO") {
      removeChunk(i);
      break;
    }
  }

  ByteVector data;
  if(d->tag)
    data = d->tag->render(true);

  if(!data.isEmpty()) {
    setChunkData("LIST", data, true);
    d->hasInfoTag = true;
  }
  else {
    d->hasInfoTag = false;
  }

  return true;
}

bool RMP3::File::hasInfoTag() const
{
  return d->hasInfoTag;
}

offset_t RMP3::File::firstFrameOffset()
{
  // The "data" chunk stores the raw MP3 data.  
  // So starts from the beginning of it.

  for(uint i = 0; i < chunkCount(); ++i) {
    if(chunkName(i) == "data")
      return nextFrameOffset(chunkOffset(i));
  }

  return -1;
}

offset_t RMP3::File::nextFrameOffset(offset_t position)
{
  // Seek forward the bit pattern 11111111 111????? 

  while(true)
  {
    seek(position);

    const ByteVector buffer = readBlock(bufferSize());
    if(buffer.size() <= 1)
      return -1;

    const char *begin = buffer.data();
    const char *end   = buffer.data() + buffer.size() - 1;
    for(const char *p = begin; p < end; ++p) {
      if(isSynchBytes(p))
        return position + (p - begin);
    }

    position += buffer.size() - 1;
  }}

offset_t RMP3::File::previousFrameOffset(offset_t position)
{
  // Seek backward the bit pattern 11111111 111????? 

  while(true)
  {
    offset_t newPosition = std::max<offset_t>(0, position - bufferSize());

    seek(newPosition);
    const ByteVector buffer = readBlock(static_cast<size_t>(position - newPosition));

    const char *begin = buffer.data() + buffer.size() - 2;
    const char *end   = buffer.data();
    for(const char *p = begin; p >= end; --p) {
      if(isSynchBytes(p))
        return newPosition + (p - end);
    }

    if(newPosition == 0)
      return -1;

    position = newPosition + 1;
  }
}

offset_t RMP3::File::lastFrameOffset()
{
  // The "data" chunk stores the raw MP3 data.  
  // So starts from the end of it.

  for(uint i = 0; i < chunkCount(); ++i) {
    if(chunkName(i) == "data")
      return previousFrameOffset(chunkOffset(i) + 8 + chunkDataSize(i));
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RMP3::File::read(bool readProperties, AudioProperties::ReadStyle propertiesStyle)
{
  if(typeName() != "RIFF" || formatName() != "RMP3") {
    setValid(false);
    return;
  }

  for(uint i = 0; i < chunkCount(); ++i) {
    if(chunkName(i) == "LIST") {
      const ByteVector data = chunkData(i);
      if(data.size() > 12 && data.mid(0, 4) == "INFO") {
        d->tag.reset(new Info::Tag(data));
        d->hasInfoTag = true;
        break;
      }
    }
  }

  if(!d->tag)
    d->tag.reset(new Info::Tag());

  if(readProperties)
    d->properties.reset(new RMP3::AudioProperties(this, propertiesStyle));
}

