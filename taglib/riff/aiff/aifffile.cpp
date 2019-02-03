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

#include <tbytevector.h>
#include <tdebug.h>
#include <id3v2tag.h>
#include <tstringlist.h>
#include <tpropertymap.h>
#include <tagutils.h>
#include <tsmartptr.h>

#include "aifffile.h"

using namespace TagLib;

class RIFF::AIFF::File::FilePrivate
{
public:
  FilePrivate() :
    hasID3v2(false) {}

  SCOPED_PTR<AudioProperties> properties;
  SCOPED_PTR<ID3v2::Tag> tag;

  bool hasID3v2;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

bool RIFF::AIFF::File::isSupported(IOStream *stream)
{
  // An AIFF file has to start with "FORM????AIFF" or "FORM????AIFC".

  const ByteVector id = Utils::readHeader(stream, 12, false);
  return (id.startsWith("FORM") && (id.containsAt("AIFF", 8) || id.containsAt("AIFC", 8)));
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::AIFF::File::File(FileName file, bool readProperties, AudioProperties::ReadStyle) :
  RIFF::File(file, BigEndian),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties);
}

RIFF::AIFF::File::File(IOStream *stream, bool readProperties, AudioProperties::ReadStyle) :
  RIFF::File(stream, BigEndian),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties);
}

RIFF::AIFF::File::~File()
{
  delete d;
}

ID3v2::Tag *RIFF::AIFF::File::tag() const
{
  return d->tag.get();
}

RIFF::AIFF::AudioProperties *RIFF::AIFF::File::audioProperties() const
{
  return d->properties.get();
}

bool RIFF::AIFF::File::save()
{
  if(readOnly()) {
    debug("RIFF::AIFF::File::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    debug("RIFF::AIFF::File::save() -- Trying to save invalid file.");
    return false;
  }

  if(d->hasID3v2) {
    removeChunk("ID3 ");
    removeChunk("id3 ");
    d->hasID3v2 = false;
  }

  if(tag() && !tag()->isEmpty()) {
    setChunkData("ID3 ", d->tag->render());
    d->hasID3v2 = true;
  }

  return true;
}

FileTypes RIFF::AIFF::File::getType()
{
  return FileTypes::RIFFAIFF;
}

bool RIFF::AIFF::File::hasID3v2Tag() const
{
  return d->hasID3v2;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::AIFF::File::read(bool readProperties)
{
  for(unsigned int i = 0; i < chunkCount(); ++i) {
    const ByteVector name = chunkName(i);
    if(name == "ID3 " || name == "id3 ") {
      if(!d->tag) {
        d->tag.reset(new ID3v2::Tag(this, chunkOffset(i)));
        d->hasID3v2 = true;
      }
      else {
        debug("RIFF::AIFF::File::read() - Duplicate ID3v2 tag found.");
      }
    }
  }

  if(!d->tag)
    d->tag.reset(new ID3v2::Tag());

  if(readProperties)
    d->properties.reset(new AudioProperties(this));
}
