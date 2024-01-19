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

#include "apeproperties.h"

#include "tdebug.h"
#include "apefile.h"
#include "apefooter.h"
#include "apetag.h"

using namespace TagLib;

class APE::Properties::PropertiesPrivate
{
public:
  int length { 0 };
  int bitrate { 0 };
  int sampleRate { 0 };
  int channels { 0 };
  int version { 0 };
  int bitsPerSample { 0 };
  unsigned int sampleFrames { 0 };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

APE::Properties::Properties(File *file, offset_t streamLength, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(file, streamLength);
}

APE::Properties::~Properties() = default;

int APE::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int APE::Properties::bitrate() const
{
  return d->bitrate;
}

int APE::Properties::sampleRate() const
{
  return d->sampleRate;
}

int APE::Properties::channels() const
{
  return d->channels;
}

int APE::Properties::version() const
{
  return d->version;
}

int APE::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

unsigned int APE::Properties::sampleFrames() const
{
  return d->sampleFrames;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

namespace
{
  int headerVersion(const ByteVector &header)
  {
    if(header.size() < 6 || !header.startsWith("MAC "))
      return -1;

    return header.toUShort(4, false);
  }
} // namespace

void APE::Properties::read(File *file, offset_t streamLength)
{
  // First, we assume that the file pointer is set at the first descriptor.
  offset_t offset = file->tell();
  int vers = headerVersion(file->readBlock(6));

  // Next, we look for the descriptor.
  if(vers < 0) {
    offset = file->find("MAC ", offset);
    file->seek(offset);
    vers = headerVersion(file->readBlock(6));
  }

  if(vers < 0) {
    debug("APE::Properties::read() -- APE descriptor not found");
    return;
  }

  d->version = vers;

  if(d->version >= 3980)
    analyzeCurrent(file);
  else
    analyzeOld(file);

  if(d->sampleFrames > 0 && d->sampleRate > 0) {
    const auto length = static_cast<double>(d->sampleFrames) * 1000.0 / d->sampleRate;
    d->length  = static_cast<int>(length + 0.5);
    d->bitrate = static_cast<int>(streamLength * 8.0 / length + 0.5);
  }
}

void APE::Properties::analyzeCurrent(File *file)
{
  // Read the descriptor
  file->seek(2, File::Current);
  const ByteVector descriptor = file->readBlock(44);
  if(descriptor.size() < 44) {
    debug("APE::Properties::analyzeCurrent() -- descriptor is too short.");
    return;
  }

  if(const unsigned int descriptorBytes = descriptor.toUInt(0, false);
     descriptorBytes - 52 > 0)
    file->seek(descriptorBytes - 52, File::Current);

  // Read the header
  const ByteVector header = file->readBlock(24);
  if(header.size() < 24) {
    debug("APE::Properties::analyzeCurrent() -- MAC header is too short.");
    return;
  }

  // Get the APE info
  d->channels      = header.toShort(18, false);
  d->sampleRate    = header.toUInt(20, false);
  d->bitsPerSample = header.toShort(16, false);

  const unsigned int totalFrames = header.toUInt(12, false);
  if(totalFrames == 0)
    return;

  const unsigned int blocksPerFrame   = header.toUInt(4, false);
  const unsigned int finalFrameBlocks = header.toUInt(8, false);
  d->sampleFrames = (totalFrames - 1) * blocksPerFrame + finalFrameBlocks;
}

void APE::Properties::analyzeOld(File *file)
{
  const ByteVector header = file->readBlock(26);
  if(header.size() < 26) {
    debug("APE::Properties::analyzeOld() -- MAC header is too short.");
    return;
  }

  const unsigned int totalFrames = header.toUInt(18, false);

  // Fail on 0 length APE files (catches non-finalized APE files)
  if(totalFrames == 0)
    return;

  const short compressionLevel = header.toShort(0, false);
  unsigned int blocksPerFrame;
  if(d->version >= 3950)
    blocksPerFrame = 73728 * 4;
  else if(d->version >= 3900 || (d->version >= 3800 && compressionLevel == 4000))
    blocksPerFrame = 73728;
  else
    blocksPerFrame = 9216;

  // Get the APE info
  d->channels   = header.toShort(4, false);
  d->sampleRate = header.toUInt(6, false);

  const unsigned int finalFrameBlocks = header.toUInt(22, false);
  d->sampleFrames = (totalFrames - 1) * blocksPerFrame + finalFrameBlocks;

  // Get the bit depth from the RIFF-fmt chunk.
  file->seek(16, File::Current);
  const ByteVector fmt = file->readBlock(28);
  if(fmt.size() < 28 || !fmt.startsWith("WAVEfmt ")) {
    debug("APE::Properties::analyzeOld() -- fmt header is too short.");
    return;
  }

  d->bitsPerSample = fmt.toShort(26, false);
}
