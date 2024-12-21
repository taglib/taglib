/***************************************************************************
    copyright           : (C) 2020-2024 Stephen F. Booth
    email               : me@sbooth.org
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

#include "shortenfile.h"

#include <cmath>

#include "shortenutils.h"

#include "tdebug.h"
#include "tutils.h"
#include "tagutils.h"
#include "tpropertymap.h"

using namespace TagLib;

namespace {

// MARK: Constants

  constexpr int minSupportedVersion         = 1;
  constexpr int maxSupportedVersion         = 3;

  // Possible values of k
  constexpr int32_t channelCountCodeSize    = 0;
  constexpr int32_t functionCodeSize        = 2;
  constexpr int32_t verbatimChunkSizeCodeSize   = 5;
  constexpr int32_t verbatimByteCodeSize    = 8;
  constexpr int32_t uInt32CodeSize          = 2;
  constexpr int32_t skipBytesCodeSize       = 1;
  constexpr int32_t lpcqCodeSize            = 2;
  constexpr int32_t extraByteCodeSize       = 7;
  constexpr int32_t fileTypeCodeSize        = 4;

  constexpr int32_t functionVerbatim        = 9;

  constexpr int32_t canonicalHeaderSize     = 44;
  constexpr int32_t verbatimChunkMaxSize    = 256;

  constexpr uint32_t maxChannelCount        = 8;

  constexpr uint32_t defaultBlockSize       = 256;
  constexpr uint32_t maxBlockSize           = 65535;

  constexpr int waveFormatPCMTag            = 0x0001;

// MARK: Variable-Length Input

  //! Variable-length input using Golomb-Rice coding.
  class VariableLengthInput {
  public:
    //! Creates a new \c VariableLengthInput object.
    VariableLengthInput(File *file) : file(file) {}

    ~VariableLengthInput() = default;

    VariableLengthInput() = delete;
    VariableLengthInput(const VariableLengthInput &) = delete;
    VariableLengthInput(VariableLengthInput &&) = delete;
    VariableLengthInput &operator=(const VariableLengthInput &) = delete;
    VariableLengthInput &operator=(VariableLengthInput &&) = delete;

    bool getRiceGolombCode(int32_t &i32, int k);
    bool getUInt(uint32_t &ui32, int version, int32_t k);

  private:
    //! Refills \c bitBuffer with a single \c uint32_t from \c buffer,
    //! refilling \c buffer if necessary.
    bool refillBitBuffer();

    //! Input stream
    File *file { nullptr };
    //! Byte buffer
    ByteVector buffer;
    //! Current position in buffer
    unsigned int bufferPosition { 0 };
    //! Bit buffer
    uint32_t bitBuffer { 0 };
    //! Bits available in \c bitBuffer, 0..32
    int bitsAvailable { 0 };
  };

  bool VariableLengthInput::getRiceGolombCode(int32_t &i32, int32_t k)
  {
    static constexpr uint32_t sMaskTable[] = {
      0x0,
      0x1,        0x3,        0x7,        0xf,
      0x1f,       0x3f,       0x7f,       0xff,
      0x1ff,      0x3ff,      0x7ff,      0xfff,
      0x1fff,     0x3fff,     0x7fff,     0xffff,
      0x1ffff,    0x3ffff,    0x7ffff,    0xfffff,
      0x1fffff,   0x3fffff,   0x7fffff,   0xffffff,
      0x1ffffff,  0x3ffffff,  0x7ffffff,  0xfffffff,
      0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
    };

    if(bitsAvailable == 0 && !refillBitBuffer())
      return false;

    int32_t result;
    for(result = 0; !(bitBuffer & (1L << --bitsAvailable)); ++result) {
      if(bitsAvailable == 0 && !refillBitBuffer())
        return false;
    }

    while(k != 0) {
      if(bitsAvailable >= k) {
        result = (result << k) | static_cast<int32_t>(
          (bitBuffer >> (bitsAvailable - k)) & sMaskTable[k]);
        bitsAvailable -= k;
        k = 0;
      }
      else {
        result = (result << bitsAvailable) | static_cast<int32_t>(
          bitBuffer & sMaskTable[bitsAvailable]);
        k -= bitsAvailable;
        if(!refillBitBuffer())
          return false;
      }
    }

    i32 = result;
    return true;
  }

  bool VariableLengthInput::getUInt(uint32_t &ui32, int version, int32_t k)
  {
    if(version > 0 && !getRiceGolombCode(k, uInt32CodeSize))
      return false;

    int32_t i32;
    if(!getRiceGolombCode(i32, k))
      return false;
    ui32 = static_cast<uint32_t>(i32);
    return true;
  }

  bool VariableLengthInput::refillBitBuffer()
  {
    if(buffer.size() - bufferPosition < 4) {
      static constexpr size_t bufferSize = 512;
      auto block = file->readBlock(bufferSize);
      if(block.size() < 4)
        return false;
      buffer = block;
      bufferPosition = 0;
    }

    bitBuffer = buffer.toUInt(bufferPosition, true);
    bufferPosition += 4;
    bitsAvailable = 32;

    return true;
  }
} // namespace

class Shorten::File::FilePrivate
{
public:
  FilePrivate() = default;
  ~FilePrivate() = default;

  FilePrivate(const FilePrivate &) = delete;
  FilePrivate &operator=(const FilePrivate &) = delete;

  std::unique_ptr<Properties> properties;
  std::unique_ptr<Tag> tag;
};

bool Shorten::File::isSupported(IOStream *stream)
{
  // A Shorten file has to start with "ajkg"
  const ByteVector id = Utils::readHeader(stream, 4, false);
  return id.startsWith("ajkg");
}

Shorten::File::File(FileName file, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>())
{
  if(isOpen())
    read(propertiesStyle);
}

Shorten::File::File(IOStream *stream, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>())
{
  if(isOpen())
    read(propertiesStyle);
}

Shorten::File::~File() = default;

Shorten::Tag *Shorten::File::tag() const
{
  return d->tag.get();
}

PropertyMap Shorten::File::properties() const
{
  return d->tag->properties();
}

PropertyMap Shorten::File::setProperties(const PropertyMap &properties)
{
  return d->tag->setProperties(properties);
}

Shorten::Properties *Shorten::File::audioProperties() const
{
  return d->properties.get();
}

bool Shorten::File::save()
{
  if(readOnly()) {
    debug("Shorten::File::save() - Cannot save to a read only file.");
    return false;
  }

  debug("Shorten::File::save() - Saving not supported.");
  return false;
}

void Shorten::File::read(AudioProperties::ReadStyle propertiesStyle)
{
  if(!isOpen())
    return;

  // Read magic number
  auto magic = readBlock(4);
  if(magic != "ajkg") {
    debug("Shorten::File::read() -- Not a Shorten file.");
    setValid(false);
    return;
  }

  PropertyValues props{};

  // Read file version
  int version = readBlock(1).toUInt();
  if(version < minSupportedVersion || version > maxSupportedVersion) {
    debug("Shorten::File::read() -- Unsupported version.");
    setValid(false);
    return;
  }
  props.version = version;

  // Set up variable length input
  VariableLengthInput input(this);

  // Read file type
  uint32_t fileType;
  if(!input.getUInt(fileType, version, fileTypeCodeSize)) {
    debug("Shorten::File::read() -- Unable to read file type.");
    setValid(false);
    return;
  }
  props.fileType = static_cast<int>(fileType);

  // Read number of channels
  uint32_t channelCount = 0;
  if(!input.getUInt(channelCount, version, channelCountCodeSize) ||
     channelCount == 0 || channelCount > maxChannelCount) {
    debug("Shorten::File::read() -- Invalid or unsupported channel count.");
    setValid(false);
    return;
  }
  props.channelCount = static_cast<int>(channelCount);

  // Read block size if version > 0
  if(version > 0) {
    uint32_t blockSize = 0;
    if(!input.getUInt(blockSize, version,
                      static_cast<int32_t>(std::log2(defaultBlockSize))) ||
       blockSize == 0 || blockSize > maxBlockSize) {
      debug("Shorten::File::read() -- Invalid or unsupported block size.");
      setValid(false);
      return;
    }

    uint32_t maxnlpc = 0;
    if(!input.getUInt(maxnlpc, version, lpcqCodeSize) /*|| maxnlpc > 1024*/) {
      debug("Shorten::File::read() -- Invalid maximum nlpc.");
      setValid(false);
      return;
    }

    uint32_t nmean = 0;
    if(!input.getUInt(nmean, version, 0) /*|| nmean > 32768*/) {
      debug("Shorten::File::read() -- Invalid nmean.");
      setValid(false);
      return;
    }

    uint32_t skipCount;
    if(!input.getUInt(skipCount, version, skipBytesCodeSize)) {
      setValid(false);
      return;
    }

    for(uint32_t i = 0; i < skipCount; ++i) {
      uint32_t dummy;
      if(!input.getUInt(dummy, version, extraByteCodeSize)) {
        setValid(false);
        return;
      }
    }
  }

  // Parse the WAVE or AIFF header in the verbatim section

  int32_t function;
  if(!input.getRiceGolombCode(function, functionCodeSize) ||
     function != functionVerbatim) {
    debug("Shorten::File::read() -- Missing initial verbatim section.");
    setValid(false);
    return;
  }

  int32_t header_size;
  if(!input.getRiceGolombCode(header_size, verbatimChunkSizeCodeSize) ||
     header_size < canonicalHeaderSize || header_size > verbatimChunkMaxSize) {
    debug("Shorten::File::read() -- Incorrect header size.");
    setValid(false);
    return;
  }

  ByteVector header(header_size, 0);

  auto it = header.begin();
  for(int32_t i = 0; i < header_size; ++i) {
    int32_t byte;
    if(!input.getRiceGolombCode(byte, verbatimByteCodeSize)) {
      debug("Shorten::File::read() -- Unable to read header.");
      setValid(false);
      return;
    }

    *it++ = static_cast<uint8_t>(byte);
  }

  // header is at least canonicalHeaderSize (44) bytes in size

  auto chunkID = header.toUInt(0, true);
//  auto chunkSize = header.toUInt(4, true);

  const auto chunkData = ByteVector(header, 8, header.size() - 8);

  // WAVE
  if(chunkID == 0x52494646 /*'RIFF'*/) {
    unsigned int offset = 0;

    chunkID = chunkData.toUInt(offset, true);
    offset += 4;
    if(chunkID != 0x57415645 /*'WAVE'*/) {
      debug("Shorten::File::read() -- Missing 'WAVE' in 'RIFF' chunk.");
      setValid(false);
      return;
    }

    auto sawFormatChunk = false;
    uint32_t dataChunkSize = 0;
    uint16_t blockAlign = 0;

    while(offset < chunkData.size()) {
      chunkID = chunkData.toUInt(offset, true);
      offset += 4;

      auto chunkSize = chunkData.toUInt(offset, false);
      offset += 4;

      switch(chunkID) {
        case 0x666d7420 /*'fmt '*/:
        {
          if(chunkSize < 16) {
            debug("Shorten::File::read() -- 'fmt ' chunk is too small.");
            setValid(false);
            return;
          }

          int formatTag = chunkData.toUShort(offset, false);
          offset += 2;
          if(formatTag != waveFormatPCMTag) {
            debug("Shorten::File::read() -- Unsupported WAVE format tag.");
            setValid(false);
            return;
          }

          int fmtChannelCount = chunkData.toUShort(offset, false);
          offset += 2;
          if(props.channelCount != fmtChannelCount)
            debug("Shorten::File::read() -- Channel count mismatch between Shorten and 'fmt ' chunk.");

          props.sampleRate = static_cast<int>(chunkData.toUInt(offset, false));
          offset += 4;

          // Skip average bytes per second
          offset += 4;

          blockAlign = chunkData.toUShort(offset, false);
          offset += 2;

          props.bitsPerSample = static_cast<int>(chunkData.toUShort(offset, false));
          offset += 2;

          sawFormatChunk = true;

          break;
        }

        case 0x64617461 /*'data'*/:
          dataChunkSize = chunkSize;
          break;
      }
    }

    if(!sawFormatChunk) {
      debug("Shorten::File::read() -- Missing 'fmt ' chunk.");
      setValid(false);
      return;
    }

    if(dataChunkSize && blockAlign)
      props.sampleFrames = static_cast<unsigned long>(dataChunkSize / blockAlign);
  }
  // AIFF
  else if(chunkID == 0x464f524d /*'FORM'*/) {
    unsigned int offset = 0;

    chunkID = chunkData.toUInt(offset, true);
    offset += 4;
    if(chunkID != 0x41494646 /*'AIFF'*/ && chunkID != 0x41494643 /*'AIFC'*/) {
      debug("Shorten::File::read() -- Missing 'AIFF' or 'AIFC' in 'FORM' chunk.");
      setValid(false);
      return;
    }

//    if(chunkID == 0x41494643 /*'AIFC'*/)
//      props.big_endian = true;

    auto sawCommonChunk = false;
    while(offset < chunkData.size()) {
      chunkID = chunkData.toUInt(offset, true);
      offset += 4;

      auto chunkSize = chunkData.toUInt(offset, true);
      offset += 4;

      // All chunks must have an even length but the pad byte is not included in chunkSize
      chunkSize += (chunkSize & 1);

      switch(chunkID) {
        case 0x434f4d4d /*'COMM'*/:
        {
          if(chunkSize < 18) {
            debug("Shorten::File::read() -- 'COMM' chunk is too small.");
            setValid(false);
            return;
          }

          int commChannelCount = chunkData.toUShort(offset, true);
          offset += 2;
          if(props.channelCount != commChannelCount)
            debug("Shorten::File::read() -- Channel count mismatch between Shorten and 'COMM' chunk.");

          props.sampleFrames = static_cast<unsigned long>(chunkData.toUInt(offset, true));
          offset += 4;

          props.bitsPerSample = static_cast<int>(chunkData.toUShort(offset, true));
          offset += 2;

          // sample rate is IEEE 754 80-bit extended float
          // (16-bit exponent, 1-bit integer part, 63-bit fraction)
          auto exp = static_cast<int16_t>(chunkData.toUShort(offset, true)) - 16383 - 63;
          offset += 2;
          if(exp < -63 || exp > 63) {
            debug("Shorten::File::read() -- exp out of range.");
            setValid(false);
            return;
          }

          auto frac = chunkData.toULongLong(offset, true);
          offset += 8;
          if(exp >= 0)
            props.sampleRate = static_cast<int>(frac << exp);
          else
            props.sampleRate = static_cast<int>(
              (frac + (static_cast<uint64_t>(1) << (-exp - 1))) >> -exp);

          sawCommonChunk = true;

          break;
        }

          // Skip all other chunks
        default:
          offset += chunkSize;
          break;
      }
    }

    if(!sawCommonChunk) {
      debug("Shorten::File::read() -- Missing 'COMM' chunk");
      setValid(false);
      return;
    }
  }
  else {
    debug("Shorten::File::read() -- Unsupported data format.");
    setValid(false);
    return;
  }

  d->tag = std::make_unique<Tag>();
  d->properties = std::make_unique<Properties>(&props, propertiesStyle);
}
