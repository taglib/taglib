/***************************************************************************
    copyright            : (C) 2010 by Alex Novichkov
    email                : novichko@atnet.ru

    copyright            : (C) 2006 by Lukáš Lalinský
    email                : lalinsky@gmail.com
                           (original WavPack implementation)
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

#include <tstring.h>
#include <tdebug.h>
#include <bitset>
#include "id3v2tag.h"
#include "apeproperties.h"
#include "apefile.h"

using namespace TagLib;

class APE::AudioProperties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    version(0),
    bitsPerSample(0),
    sampleFrames(0) {}

  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int version;
  int bitsPerSample;
  uint sampleFrames;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

APE::AudioProperties::AudioProperties(File *file, ReadStyle /*style*/) 
  : d(new PropertiesPrivate())
{
  read(file);
}

APE::AudioProperties::~AudioProperties()
{
}

int APE::AudioProperties::length() const
{
  return d->length;
}

int APE::AudioProperties::bitrate() const
{
  return d->bitrate;
}

int APE::AudioProperties::sampleRate() const
{
  return d->sampleRate;
}

int APE::AudioProperties::channels() const
{
  return d->channels;
}

int APE::AudioProperties::version() const
{
  return d->version;
}

int APE::AudioProperties::bitsPerSample() const
{
  return d->bitsPerSample;
}

TagLib::uint APE::AudioProperties::sampleFrames() const
{
  return d->sampleFrames;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////


void APE::AudioProperties::read(File *file)
{
  // First we are searching the descriptor
  offset_t offset = findDescriptor(file);
  if(offset < 0)
    return;

  // Then we read the header common for all versions of APE
  file->seek(offset);
  ByteVector commonHeader = file->readBlock(6);
  if(!commonHeader.startsWith("MAC "))
    return;

  d->version = commonHeader.toUInt16LE(4);
  if(d->version >= 3980)
    analyzeCurrent(file);
  else 
    analyzeOld(file);
}

offset_t APE::AudioProperties::findDescriptor(File *file)
{
  offset_t ID3v2Location = findID3v2(file);
  long ID3v2OriginalSize = 0;
  bool hasID3v2 = false;
  if(ID3v2Location >= 0) {
    ID3v2::Tag tag(file, ID3v2Location);
    ID3v2OriginalSize = tag.header()->completeTagSize();
    if(tag.header()->tagSize() > 0)
      hasID3v2 = true;
  }

  offset_t offset = 0;
  if(hasID3v2)
    offset = file->find("MAC ", ID3v2Location + ID3v2OriginalSize);
  else
    offset = file->find("MAC ");

  if(offset < 0) {
    debug("APE::Properties::findDescriptor() -- APE descriptor not found");
    return -1;
  }

  return offset;
}

offset_t APE::AudioProperties::findID3v2(File *file)
{
  if(!file->isValid())
    return -1;

  file->seek(0);

  if(file->readBlock(3) == ID3v2::Header::fileIdentifier())
    return 0;

  return -1;
}

void APE::AudioProperties::analyzeCurrent(File *file)
{
  // Read the descriptor
  file->seek(2, File::Current);
  ByteVector descriptor = file->readBlock(44);
  const uint descriptorBytes = descriptor.toUInt32LE(0);

  if ((descriptorBytes - 52) > 0)
    file->seek(descriptorBytes - 52, File::Current);

  // Read the header
  ByteVector header = file->readBlock(24);

  // Get the APE info
  d->channels = header.toInt16LE(18);
  d->sampleRate = header.toUInt32LE(20);
  d->bitsPerSample = header.toInt16LE(16);
  //d->compressionLevel =

  const uint totalFrames = header.toUInt32LE(12);
  const uint blocksPerFrame = header.toUInt32LE(4);
  const uint finalFrameBlocks = header.toUInt32LE(8);

  d->sampleFrames = totalFrames > 0 ? (totalFrames -  1) * blocksPerFrame + finalFrameBlocks : 0;
  d->length = d->sampleRate > 0 ? d->sampleFrames / d->sampleRate : 0;
  d->bitrate = d->length > 0 ? static_cast<int>(file->length() * 8L / d->length / 1000) : 0;
}

void APE::AudioProperties::analyzeOld(File *file)
{
  ByteVector header = file->readBlock(26);
  const uint totalFrames = header.toUInt32LE(18);

  // Fail on 0 length APE files (catches non-finalized APE files)
  if(totalFrames == 0)
    return;

  const short compressionLevel = header.toUInt32LE(0);
  uint blocksPerFrame;
  if(d->version >= 3950)
    blocksPerFrame = 73728 * 4;
  else if(d->version >= 3900 || (d->version >= 3800 && compressionLevel == 4000))
    blocksPerFrame = 73728;
  else
    blocksPerFrame = 9216;

  d->channels = header.toUInt16LE(4);
  d->sampleRate = header.toUInt32LE(6);
  const uint finalFrameBlocks = header.toUInt32LE(22);
  const uint totalBlocks = totalFrames > 0 ? (totalFrames - 1) * blocksPerFrame + finalFrameBlocks : 0;
  d->length = totalBlocks / d->sampleRate;
  d->bitrate = d->length > 0 ? static_cast<int>(file->length() * 8L / d->length / 1000) : 0;
}

