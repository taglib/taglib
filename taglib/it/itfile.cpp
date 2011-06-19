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

#include "tstringlist.h"
#include "itfile.h"
#include "tdebug.h"
#include "modfileprivate.h"

using namespace TagLib;
using namespace IT;

class IT::File::FilePrivate
{
public:
  FilePrivate(AudioProperties::ReadStyle propertiesStyle)
    : tag(), properties(propertiesStyle)
  {
  }

  Mod::Tag       tag;
  IT::Properties properties;
};

IT::File::File(FileName file, bool readProperties,
               AudioProperties::ReadStyle propertiesStyle) :
  Mod::FileBase(file),
  d(new FilePrivate(propertiesStyle))
{
  read(readProperties);
}

IT::File::File(IOStream *stream, bool readProperties,
               AudioProperties::ReadStyle propertiesStyle) :
  Mod::FileBase(stream),
  d(new FilePrivate(propertiesStyle))
{
  read(readProperties);
}

IT::File::~File()
{
  delete d;
}

Mod::Tag *IT::File::tag() const
{
  return &d->tag;
}

IT::Properties *IT::File::audioProperties() const
{
  return &d->properties;
}

bool IT::File::save()
{
  if(readOnly())
  {
    debug("IT::File::save() - Cannot save to a read only file.");
    return false;
  }
  seek(4);
  writeString(d->tag.title(), 26);

  seek(2, Current);

  ushort length = 0;
  ushort instrumentCount = 0;
  ushort sampleCount = 0;

  if(!readU16L(length) || !readU16L(instrumentCount) || !readU16L(sampleCount))
    return false;

  seek(15, Current);

  // write comment as instrument and sample names:
  StringList lines = d->tag.comment().split("\n");
  for(ushort i = 0; i < instrumentCount; ++ i)
  {
    seek(192L + length + ((long)i << 2));
    ulong instrumentOffset = 0;
    if(!readU32L(instrumentOffset))
      return false;

    seek(instrumentOffset + 32);

    if(i < lines.size())
      writeString(lines[i], 26);
    else
      writeString(String::null, 26);
  }

  for(ushort i = 0; i < sampleCount; ++ i)
  {
    seek(192L + length + ((long)instrumentCount << 2) + ((long)i << 2));
    ulong sampleOffset = 0;
    if(!readU32L(sampleOffset))
      return false;
    
    seek(sampleOffset + 20);

    if((i + instrumentCount) < lines.size())
      writeString(lines[i + instrumentCount], 26);
    else
      writeString(String::null, 26);
  }

  return true;
}

void IT::File::read(bool)
{
  if(!isOpen())
    return;

  seek(0);
  READ_ASSERT(readBlock(4) == "IMPM");
  READ_STRING(d->tag.setTitle, 26);

  seek(2, Current);

  READ_U16L_AS(length);
  READ_U16L_AS(instrumentCount);
  READ_U16L_AS(sampleCount);
  
  d->properties.setTableLength(length);
  d->properties.setInstrumentCount(instrumentCount);
  d->properties.setSampleCount(sampleCount);
  READ_U16L(d->properties.setPatternCount);
  READ_U16L(d->properties.setVersion);
  READ_U16L(d->properties.setCompatibleVersion);
  READ_U16L(d->properties.setFlags);
  READ_U16L_AS(special);
  d->properties.setSpecial(special);
  READ_U16L(d->properties.setGlobalVolume);
  READ_U16L(d->properties.setMixVolume);
  READ_BYTE(d->properties.setBpmSpeed);
  READ_BYTE(d->properties.setTempo);
  READ_BYTE(d->properties.setPanningSeparation);
  READ_BYTE(d->properties.setPitchWheelDepth);

  /*
   * While the message would be a sorta comment tag, I don't
   * see any IT files in the wild that use this or set the
   * offset/length to a correct value.
   *
   * In all files I found where the message bit was set the
   * offset was either 0 or a ridiculous high value and the
   * length wasn't much better.
   *
  if(special & 0x1)
  {
    READ_U16L_AS(messageLength);
    READ_U32L_AS(messageOffset);
    seek(messageOffset);
    READ_STRING_AS(message, messageLength);
    debug("Message: \""+message+"\"");
  }
  */

  StringList comment;
  for(ushort i = 0; i < instrumentCount; ++ i)
  {
    seek(192L + length + ((long)i << 2));
    READ_U32L_AS(instrumentOffset);
    seek(instrumentOffset);

    ByteVector instrumentMagic = readBlock(4);
    READ_ASSERT(instrumentMagic == "IMPI");

    READ_STRING_AS(dosFileName, 13);

    seek(15, Current);

    READ_STRING_AS(instrumentName, 26);
    comment.append(instrumentName);
  }
  
  for(ushort i = 0; i < sampleCount; ++ i)
  {
    seek(192L + length + ((long)instrumentCount << 2) + ((long)i << 2));
    READ_U32L_AS(sampleOffset);
    
    seek(sampleOffset);

    ByteVector sampleMagic = readBlock(4);
    READ_ASSERT(sampleMagic == "IMPS");

    READ_STRING_AS(dosFileName, 13);
    READ_BYTE_AS(globalVolume);
    READ_BYTE_AS(sampleFlags);
    READ_BYTE_AS(sampleVolume);
    READ_STRING_AS(sampleName, 26);
    READ_BYTE_AS(sampleCvt);
    READ_BYTE_AS(samplePanning);
    READ_U32L_AS(sampleLength);
    READ_U32L_AS(loopStart);
    READ_U32L_AS(loopStop);
    READ_U32L_AS(c5speed);
    READ_U32L_AS(sustainLoopStart);
    READ_U32L_AS(sustainLoopEnd);
    READ_U32L_AS(sampleDataOffset);
    READ_BYTE_AS(vibratoSpeed);
    READ_BYTE_AS(vibratoDepth);
    READ_BYTE_AS(vibratoRate);
    READ_BYTE_AS(vibratoType);
    
    comment.append(sampleName);
  }

  d->tag.setComment(comment.toString("\n"));
  d->tag.setTrackerName("Impulse Tracker");
}
