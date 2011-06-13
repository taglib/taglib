/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#include "s3mfile.h"
#include "tstringlist.h"
#include "modfileprivate.h"

using namespace TagLib;
using namespace S3M;

class S3M::File::FilePrivate
{
public:
  FilePrivate(AudioProperties::ReadStyle propertiesStyle)
    : properties(propertiesStyle)
  {
  }

  Mod::Tag        tag;
  S3M::Properties properties;
};

S3M::File::File(FileName file, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle) :
  Mod::File(file),
  d(new FilePrivate(propertiesStyle))
{
  read(readProperties);
}

S3M::File::File(IOStream *stream, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle) :
  Mod::File(stream),
  d(new FilePrivate(propertiesStyle))
{
  read(readProperties);
}

S3M::File::~File()
{
  delete d;
}

Mod::Tag *S3M::File::tag() const
{
  return &d->tag;
}

S3M::Properties *S3M::File::audioProperties() const
{
  return &d->properties;
}

bool S3M::File::save()
{
  // note: if title starts with "Extended Module: "
  // the file would look like an .xm file
  seek(0);
  writeString(d->tag.title(), 28);
  // TODO: write comment as sample names
  return true;
}

void S3M::File::read(bool)
{
  if(!isOpen())
    return;

  READ_STRING(d->tag.setTitle, 28);
  READ_BYTE_AS(mark);
  READ_BYTE_AS(type);

  READ_ASSERT(mark == 0x1A && type == 0x10);

  seek(32);

  READ_U16L_AS(length);
  READ_U16L_AS(sampleCount);

  d->properties.setSampleLength(length);
  d->properties.setSampleCount(sampleCount);

  READ_U16L(d->properties.setPatternCount);
  READ_U16L(d->properties.setFlags);
  READ_U16L(d->properties.setVersion);
  READ_U16L(d->properties.setSamplesType);

  READ_ASSERT(readBlock(4) == "SCRM");

  READ_BYTE_AS(baseVolume);
  d->properties.setBaseVolume(baseVolume << 1);

  READ_BYTE(d->properties.setTempo);
  READ_BYTE(d->properties.setBpmSpeed);

  READ_BYTE_AS(stereo);
  d->properties.setStereo((stereo & 0x80) != 0);
  READ_BYTE(d->properties.setUltraClick);

  READ_BYTE_AS(usePanningValues);
  d->properties.setUsePanningValues(usePanningValues == 0xFC);

  seek(10, Current);

  int channels = 0;
  for(int i = 0; i < 32; ++ i)
  {
    READ_BYTE_AS(terminator);
    if(terminator != 0xff) ++ channels;
  }
  d->properties.setChannels(channels);

  seek(channels, Current);

  StringList comment;
  for(ushort i = 0; i < sampleCount; ++ i)
  {
    seek(96 + length + (i << 1));

    READ_U16L_AS(instrumentOffset);
    seek(instrumentOffset << 4);

    READ_BYTE_AS(sampleType);
    READ_STRING_AS(dosFileName, 13);
    READ_U16L_AS(sampleOffset);
    READ_U32L_AS(sampleLength);
    READ_U32L_AS(repeatStart);
    READ_U32L_AS(repeatStop);
    READ_BYTE_AS(sampleVolume);

    seek(2, Current);

    READ_BYTE_AS(sampleFlags);
    READ_U32L_AS(baseFrequency);

    seek(12, Current);

    READ_STRING_AS(sampleName, 28);
    comment.append(sampleName);
  }

  d->tag.setComment(comment.toString("\n"));
}
