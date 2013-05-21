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

#include "modfile.h"
#include "tstringlist.h"
#include "tdebug.h"
#include "modfileprivate.h"
#include "tpropertymap.h"

using namespace TagLib;
using namespace Mod;

class Mod::File::FilePrivate
{
public:
  Mod::Tag        tag;
  NonRefCountPtr<Mod::AudioProperties> audioProperties;
};

Mod::File::File(FileName file, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle)
  : Mod::FileBase(file)
  , d(new FilePrivate())
{
  // readProperties is ignored. 
  // Reading audio properties can't be separated from reading metadata.
  if(isOpen())
    read();
}

Mod::File::File(IOStream *stream, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle) 
  : Mod::FileBase(stream)
  , d(new FilePrivate())
{
  // readProperties is ignored. 
  // Reading audio properties can't be separated from reading metadata.
  if(isOpen())
    read();
}

Mod::File::~File()
{
  delete d;
}

Mod::Tag *Mod::File::tag() const
{
  return &d->tag;
}

Mod::AudioProperties *Mod::File::audioProperties() const
{
  return d->audioProperties.get();
}

bool Mod::File::save()
{
  if(readOnly()) {
    debug("Mod::File::save() - Cannot save to a read only file.");
    return false;
  }
  seek(0);
  writeString(d->tag.title(), 20);
  StringList lines = d->tag.comment().split("\n");
  uint n = std::min(static_cast<uint>(lines.size()), d->audioProperties->instrumentCount());
  for(uint i = 0; i < n; ++ i) {
    writeString(lines[i], 22);
    seek(8, Current);
  }

  for(uint i = n; i < d->audioProperties->instrumentCount(); ++ i) {
    writeString(String::null, 22);
    seek(8, Current);
  }
  return true;
}

void Mod::File::read()
{
  d->audioProperties.reset(new AudioProperties());

  seek(1080);
  ByteVector modId = readBlock(4);
  READ_ASSERT(modId.size() == 4);

  int  channels    =  4;
  uint instruments = 31;
  if(modId == "M.K." || modId == "M!K!" || modId == "M&K!" || modId == "N.T.") {
    d->tag.setTrackerName("ProTracker");
    channels = 4;
  }
  else if(modId.startsWith("FLT") || modId.startsWith("TDZ")) {
    d->tag.setTrackerName("StarTrekker");
    char digit = modId[3];
    READ_ASSERT(digit >= '0' && digit <= '9');
    channels = digit - '0';
  }
  else if(modId.endsWith("CHN")) {
    d->tag.setTrackerName("StarTrekker");
    char digit = modId[0];
    READ_ASSERT(digit >= '0' && digit <= '9');
    channels = digit - '0';
  }
  else if(modId == "CD81" || modId == "OKTA") {
    d->tag.setTrackerName("Atari Oktalyzer");
    channels = 8;
  }
  else if(modId.endsWith("CH") || modId.endsWith("CN")) {
    d->tag.setTrackerName("TakeTracker");
    char digit = modId[0];
    READ_ASSERT(digit >= '0' && digit <= '9');
    channels = (digit - '0') * 10;
    digit = modId[1];
    READ_ASSERT(digit >= '0' && digit <= '9');
    channels += digit - '0';
  }
  else {
    // Not sure if this is correct. I'd need a file
    // created with NoiseTracker to check this.
    d->tag.setTrackerName("NoiseTracker"); // probably
    channels    =  4;
    instruments = 15;
  }
  d->audioProperties->setChannels(channels);
  d->audioProperties->setInstrumentCount(instruments);

  seek(0);
  READ_STRING(d->tag.setTitle, 20);

  StringList comment;
  for(uint i = 0; i < instruments; ++ i) {
    READ_STRING_AS(instrumentName, 22);
    // value in words, * 2 (<< 1) for bytes:
    READ_U16B_AS(sampleLength);

    READ_BYTE_AS(fineTuneByte);
    int fineTune = fineTuneByte & 0xF;
    // > 7 means negative value
    if(fineTune > 7) fineTune -= 16;

    READ_BYTE_AS(volume);
    if(volume > 64) volume = 64;
    // volume in decibels: 20 * log10(volume / 64)

    // value in words, * 2 (<< 1) for bytes:
    READ_U16B_AS(repeatStart);
    // value in words, * 2 (<< 1) for bytes:
    READ_U16B_AS(repatLength);

    comment.append(instrumentName);
  }

  READ_BYTE(d->audioProperties->setLengthInPatterns);

  d->tag.setComment(comment.toString("\n"));
}
