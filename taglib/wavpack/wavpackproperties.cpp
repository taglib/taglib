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

#include "wavpackproperties.h"

#include <cstdint>
#include <array>

#include "tstring.h"
#include "tdebug.h"
#include "wavpackfile.h"

// Implementation of this class is based on the information at:
// http://www.wavpack.com/file_format.txt

using namespace TagLib;

class WavPack::Properties::PropertiesPrivate
{
public:
  int length { 0 };
  int bitrate { 0 };
  int sampleRate { 0 };
  int channels { 0 };
  int version { 0 };
  int bitsPerSample { 0 };
  bool lossless { false };
  unsigned int sampleFrames { 0 };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

WavPack::Properties::Properties(File *file, offset_t streamLength, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(file, streamLength);
}

WavPack::Properties::~Properties() = default;

int WavPack::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int WavPack::Properties::bitrate() const
{
  return d->bitrate;
}

int WavPack::Properties::sampleRate() const
{
  return d->sampleRate;
}

int WavPack::Properties::channels() const
{
  return d->channels;
}

int WavPack::Properties::version() const
{
  return d->version;
}

int WavPack::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

bool WavPack::Properties::isLossless() const
{
  return d->lossless;
}

unsigned int WavPack::Properties::sampleFrames() const
{
  return d->sampleFrames;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

#define BYTES_STORED    3
#define MONO_FLAG       4
#define HYBRID_FLAG     8
#define DSD_FLAG        0x80000000      // block is encoded DSD (1-bit PCM)

#define SHIFT_LSB       13
#define SHIFT_MASK      (0x1fL << SHIFT_LSB)

#define SRATE_LSB       23
#define SRATE_MASK      (0xfL << SRATE_LSB)

#define MIN_STREAM_VERS 0x402
#define MAX_STREAM_VERS 0x410

#define INITIAL_BLOCK   0x800
#define FINAL_BLOCK     0x1000

#define ID_DSD_BLOCK            0x0e
#define ID_OPTIONAL_DATA        0x20
#define ID_UNIQUE               0x3f
#define ID_ODD_SIZE             0x40
#define ID_LARGE                0x80
#define ID_SAMPLE_RATE          (ID_OPTIONAL_DATA | 0x7)

namespace
{
  constexpr std::array sampleRates {
    6000U, 8000U, 9600U, 11025U, 12000U, 16000U, 22050U, 24000U,
    32000U, 44100U, 48000U, 64000U, 88200U, 96000U, 192000U, 0U
  };

  /*!
   * Given a WavPack \a block (complete, but not including the 32-byte header),
   * parse the metadata blocks until an \a id block is found and return the
   * contained data, or zero if no such block is found.
   * Supported values for \a id are ID_SAMPLE_RATE and ID_DSD_BLOCK.
  */
  int getMetaDataChunk(const ByteVector &block, unsigned char id)
  {
    if(id != ID_SAMPLE_RATE && id != ID_DSD_BLOCK)
      return 0;

    const int blockSize = static_cast<int>(block.size());
    int index = 0;

    while(index + 1 < blockSize) {
      const auto metaId = static_cast<unsigned char>(block[index]);
      int metaBc = static_cast<unsigned char>(block[index + 1]) << 1;
      index += 2;

      if(metaId & ID_LARGE) {
        if(index + 2 > blockSize)
          return 0;

        metaBc += (static_cast<uint32_t>(static_cast<unsigned char>(block[index])) << 9)
                + (static_cast<uint32_t>(static_cast<unsigned char>(block[index + 1])) << 17);
        index += 2;
      }

      if(index + metaBc > blockSize)
        return 0;

      // if we got a sample rate, return it

      if(id == ID_SAMPLE_RATE && (metaId & ID_UNIQUE) == ID_SAMPLE_RATE && metaBc == 4) {
        int sampleRate = static_cast<int32_t>(static_cast<unsigned char>(block[index]));
        sampleRate |= static_cast<int32_t>(static_cast<unsigned char>(block[index + 1])) << 8;
        sampleRate |= static_cast<int32_t>(static_cast<unsigned char>(block[index + 2])) << 16;

        // only use 4th byte if it's really there

        if(!(metaId & ID_ODD_SIZE))
          sampleRate |= static_cast<int32_t>(static_cast<unsigned char>(block[index + 3]) & 0x7f) << 24;

        return sampleRate;
      }

      // if we got DSD block, return the specified rate shift amount

      if(id == ID_DSD_BLOCK && (metaId & ID_UNIQUE) == ID_DSD_BLOCK && metaBc > 0) {
        if(const auto rateShift = static_cast<unsigned char>(block[index]);
           rateShift <= 31)
          return rateShift;
      }

      index += metaBc;
    }

    return 0;
  }

  /*!
   * Given a WavPack block (complete, but not including the 32-byte header),
   * parse the metadata blocks until an ID_SAMPLE_RATE block is found and
   * return the non-standard sample rate contained there, or zero if no such
   * block is found.
   */
  int getNonStandardRate(const ByteVector &block)
  {
    return getMetaDataChunk(block, ID_SAMPLE_RATE);
  }

  /*!
   * Given a WavPack block (complete, but not including the 32-byte header),
   * parse the metadata blocks until a DSD audio data block is found and return
   * the sample-rate shift value contained there, or zero if no such block is
   * found. The nominal sample rate of DSD audio files (found in the header)
   * must be left-shifted by this amount to get the actual "byte" sample rate.
   * Note that 8-bit bytes are the "atoms" of the DSD audio coding (for
   * decoding, seeking, etc), so the shifted rate must be further multiplied by
   * 8 to get the actual DSD bit sample rate.
  */
  int getDsdRateShifter(const ByteVector &block)
  {
    return getMetaDataChunk(block, ID_DSD_BLOCK);
  }

}  // namespace

void WavPack::Properties::read(File *file, offset_t streamLength)
{
  offset_t offset = 0;

  while(true) {
    file->seek(offset);
    const ByteVector data = file->readBlock(32);

    if(data.size() < 32) {
      debug("WavPack::Properties::read() -- data is too short.");
      break;
    }

    if(!data.startsWith("wvpk")) {
      debug("WavPack::Properties::read() -- Block header not found.");
      break;
    }

    const unsigned int blockSize = data.toUInt(4, false);
    const unsigned int smplFrames  = data.toUInt(12, false);
    const unsigned int blockSamples = data.toUInt(20, false);
    const unsigned int flags = data.toUInt(24, false);
    unsigned int smplRate = sampleRates[(flags & SRATE_MASK) >> SRATE_LSB];

    if(!blockSamples) {        // ignore blocks with no samples
      offset += blockSize + 8;
      continue;
    }

    if(blockSize < 24 || blockSize > 1048576) {
      debug("WavPack::Properties::read() -- Invalid block header found.");
      break;
    }

    // For non-standard sample rates or DSD audio files, we must read and parse the block
    // to actually determine the sample rate.

    if(!smplRate || (flags & DSD_FLAG)) {
      const unsigned int adjustedBlockSize = blockSize - 24;
      const ByteVector block = file->readBlock(adjustedBlockSize);

      if(block.size() < adjustedBlockSize) {
        debug("WavPack::Properties::read() -- block is too short.");
        break;
      }

      if(!smplRate)
        smplRate = static_cast<unsigned int>(getNonStandardRate(block));

      if(smplRate && (flags & DSD_FLAG))
        smplRate <<= getDsdRateShifter(block);
    }

    if(flags & INITIAL_BLOCK) {
      d->version = data.toShort(8, false);
      if(d->version < MIN_STREAM_VERS || d->version > MAX_STREAM_VERS)
        break;

      d->bitsPerSample = ((flags & BYTES_STORED) + 1) * 8 - ((flags & SHIFT_MASK) >> SHIFT_LSB);
      d->sampleRate    = static_cast<int>(smplRate);
      d->lossless      = !(flags & HYBRID_FLAG);
      d->sampleFrames  = smplFrames;
    }

    d->channels += (flags & MONO_FLAG) ? 1 : 2;

    if(flags & FINAL_BLOCK)
      break;

    offset += blockSize + 8;
  }

  if(d->sampleFrames == ~0u)
    d->sampleFrames = seekFinalIndex(file, streamLength);

  if(d->sampleFrames > 0 && d->sampleRate > 0) {
    const auto length = static_cast<double>(d->sampleFrames) * 1000.0 / d->sampleRate;
    d->length  = static_cast<int>(length + 0.5);
    d->bitrate = static_cast<int>(streamLength * 8.0 / length + 0.5);
  }
}

unsigned int WavPack::Properties::seekFinalIndex(File *file, offset_t streamLength)
{
  offset_t offset = streamLength;

  while (offset >= 32) {
    offset = file->rfind("wvpk", offset - 4);

    if(offset == -1)
      return 0;

    file->seek(offset);
    const ByteVector data = file->readBlock(32);
    if(data.size() < 32)
      return 0;

    const unsigned int blockSize    = data.toUInt(4, false);
    const unsigned int blockIndex   = data.toUInt(16, false);
    const unsigned int blockSamples = data.toUInt(20, false);
    const unsigned int flags        = data.toUInt(24, false);
    const int vers                  = data.toShort(8, false);

    // try not to trigger on a spurious "wvpk" in WavPack binary block data

    if(vers < MIN_STREAM_VERS || vers > MAX_STREAM_VERS || (blockSize & 1) ||
      blockSize < 24 || blockSize >= 1048576 || blockSamples > 131072)
        continue;

    if (blockSamples && (flags & FINAL_BLOCK))
      return blockIndex + blockSamples;
  }

  return 0;
}
