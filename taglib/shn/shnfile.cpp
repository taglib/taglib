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

#include <cmath>
#include <type_traits>

#include "shnfile.h"
#include "shnutils.h"

#include "tdebug.h"
#include "tutils.h"
#include "tagutils.h"
#include "tpropertymap.h"

using namespace TagLib;

namespace {

// MARK: Constants

  const auto MIN_SUPPORTED_VERSION  { 1 };
  const auto MAX_SUPPORTED_VERSION  { 3 };

  const auto DEFAULT_BLOCK_SIZE     { 256 };

  const auto CHANSIZE               { 0 };

  const auto FNSIZE                 { 2 };
  const auto FN_VERBATIM            { 9 };

  const auto VERBATIM_CKSIZE_SIZE   { 5 };
  const auto VERBATIM_BYTE_SIZE     { 8 };
  const auto VERBATIM_CHUNK_MAX     { 256 };

  const auto ULONGSIZE              { 2 };
  const auto NSKIPSIZE              { 1 };
  const auto LPCQSIZE               { 2 };
  const auto XBYTESIZE              { 7 };

  const auto TYPESIZE               { 4 };

  const auto MAX_CHANNELS           { 8 };
  const auto MAX_BLOCKSIZE          { 65535 };

  const auto CANONICAL_HEADER_SIZE  { 44 };

  const auto WAVE_FORMAT_PCM        { 0x0001 };

// MARK: Variable-Length Input

  /// Variable-length input using Golomb-Rice coding
  class vario_input {

  public:

    static constexpr uint32_t sMaskTable [] = {
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

    /// Creates a new `vario_input` object with an internal buffer of the specified size
    /// - warning: Sizes other than `512` will break seeking
    vario_input(File *file, size_t size = 512) noexcept
    : file_{file}, size_{size}
    {
      byte_buffer_ = new (std::nothrow) uint8_t [size_];
      byte_buffer_position_ = byte_buffer_;
    }

    ~vario_input()
    {
      delete [] byte_buffer_;
    }

    vario_input() = delete;
    vario_input(const vario_input&) = delete;
    vario_input(vario_input&&) = delete;
    vario_input& operator=(const vario_input&) = delete;
    vario_input& operator=(vario_input&&) = delete;

    explicit operator bool() const noexcept
    {
      return byte_buffer_ != nullptr;
    }

    /// Reads a single unsigned value from the specified bin
    bool uvar_get(int32_t& i32, size_t bin)
    {
      if(bits_available_ == 0) {
        if(!word_get(bit_buffer_))
          return false;
        bits_available_ = 32;
      }

      int32_t result;
      for(result = 0; !(bit_buffer_ & (1L << --bits_available_)); ++result) {
        if(bits_available_ == 0) {
          if(!word_get(bit_buffer_))
            return false;
          bits_available_ = 32;
        }
      }

      while(bin != 0) {
        if(bits_available_ >= bin) {
          result = (result << bin) | static_cast<int32_t>((bit_buffer_ >> (bits_available_ - bin)) & sMaskTable[bin]);
          bits_available_ -= bin;
          bin = 0;
        }
        else {
          result = (result << bits_available_) | static_cast<int32_t>(bit_buffer_ & sMaskTable[bits_available_]);
          bin -= bits_available_;
          if(!word_get(bit_buffer_))
            return false;
          bits_available_ = 32;
        }
      }

      i32 = result;
      return true;
    }

    /// Reads the unsigned Golomb-Rice code
    bool ulong_get(uint32_t& ui32)
    {
      int32_t bitcount;
      if(!uvar_get(bitcount, ULONGSIZE))
        return false;

      int32_t i32;
      if(!uvar_get(i32, static_cast<uint32_t>(bitcount)))
        return false;

      ui32 = static_cast<uint32_t>(i32);
      return true;
    }

    bool uint_get(uint32_t& ui32, int version, size_t bin)
    {
      if(version == 0) {
        int32_t i32;
        if(!uvar_get(i32, bin))
          return false;
        ui32 = static_cast<uint32_t>(i32);
        return true;
      }
      else
        return ulong_get(ui32);
    }

  private:

    /// Input stream
    File *file_ { nullptr };
    /// Size of `byte_buffer_` in bytes
    size_t size_ { 0 };
    /// Byte buffer
    uint8_t *byte_buffer_ { nullptr };
    /// Current position in `byte_buffer_`
    uint8_t *byte_buffer_position_ { nullptr };
    /// Bytes available in `byte_buffer_`
    size_t bytes_available_ { 0 };
    /// Bit buffer
    uint32_t bit_buffer_ { 0 };
    /// Bits available in `bit_buffer_`
    size_t bits_available_ { 0 };

    /// Reads a single `uint32_t` from the byte buffer, refilling if necessary
    bool word_get(uint32_t& ui32)
    {
      if(bytes_available_ < 4) {
        auto block = file_->readBlock(size_);
        if(block.size() < 4)
          return false;
        memcpy(byte_buffer_, block.data(), block.size());
        bytes_available_ += block.size();
        byte_buffer_position_ = byte_buffer_;
      }

      ui32 = static_cast<uint32_t>((static_cast<int32_t>(byte_buffer_position_[0]) << 24) | (static_cast<int32_t>(byte_buffer_position_[1]) << 16) | (static_cast<int32_t>(byte_buffer_position_[2]) << 8) | static_cast<int32_t>(byte_buffer_position_[3]));

      byte_buffer_position_ += 4;
      bytes_available_ -= 4;

      return true;
    }
  };

// MARK: Unsigned Integer Reading

  template <typename T, typename = std::enable_if<std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>>>
  T read_uint(void *p, uintptr_t n, bool big) noexcept
  {
    T value = *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(p) + n);

    auto system_big = Utils::systemByteOrder() == Utils::BigEndian;
    if(big != system_big)
      value = Utils::byteSwap(value);

    return value;
  }

  uint64_t read_uint_big64(void *p, uintptr_t n) noexcept
  {
    return read_uint<uint64_t>(p, n, true);
  }

  uint32_t read_uint_big32(void *p, uintptr_t n) noexcept
  {
    return read_uint<uint32_t>(p, n, true);
  }

  uint32_t read_uint_little32(void *p, uintptr_t n) noexcept
  {
    return read_uint<uint32_t>(p, n, false);
  }

  uint16_t read_uint_big16(void *p, uintptr_t n) noexcept
  {
    return read_uint<uint16_t>(p, n, true);
  }

  uint16_t read_uint_little16(void *p, uintptr_t n) noexcept
  {
    return read_uint<uint16_t>(p, n, false);
  }
} // namespace

class SHN::File::FilePrivate
{
public:
  FilePrivate() = default;
  ~FilePrivate() = default;

  FilePrivate(const FilePrivate &) = delete;
  FilePrivate &operator=(const FilePrivate &) = delete;

  std::unique_ptr<Properties> properties;
  std::unique_ptr<Tag> tag;
};

bool SHN::File::isSupported(IOStream *stream)
{
  // A Shorten file has to start with "ajkg"
  const ByteVector id = Utils::readHeader(stream, 4, false);
  return id.startsWith("ajkg");
}

SHN::File::File(FileName file, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>())
{
  if(isOpen())
    read(propertiesStyle);
}

SHN::File::File(IOStream *stream, bool readProperties,
                AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>())
{
  if(isOpen())
    read(propertiesStyle);
}

SHN::File::~File() = default;

SHN::Tag *SHN::File::tag() const
{
  return d->tag.get();
}

PropertyMap SHN::File::properties() const
{
  return d->tag->properties();
}

PropertyMap SHN::File::setProperties(const PropertyMap &properties)
{
  return d->tag->setProperties(properties);
}

SHN::Properties *SHN::File::audioProperties() const
{
  return d->properties.get();
}

bool SHN::File::save()
{
  if(readOnly()) {
    debug("SHN::File::save() - Cannot save to a read only file.");
    return false;
  }

  debug("SHN::File::save() - Saving not supported.");
  return false;
}

void SHN::File::read(AudioProperties::ReadStyle propertiesStyle)
{
  if(!isOpen())
    return;

  // Read magic number
  auto magic = readBlock(4);
  if(magic != "ajkg") {
    debug("SHN::File::read() -- Not a Shorten file.");
    setValid(false);
    return;
  }

  PropertyValues props{};

  // Read file version
  auto version = readBlock(1).toUInt();
  if(version < MIN_SUPPORTED_VERSION || version > MAX_SUPPORTED_VERSION) {
    debug("SHN::File::read() -- Unsupported version.");
    setValid(false);
    return;
  }
  props.version = version;

  // Set up variable length input
  vario_input input{this, 512};
  if(!input) {
    debug("SHN::File::read() -- Unable to allocate variable-length input.");
    setValid(false);
    return;
  }

  // Read internal file type
  uint32_t ftype;
  if(!input.uint_get(ftype, version, TYPESIZE)) {
    debug("SHN::File::read() -- Unable to read internal file type.");
    setValid(false);
    return;
  }
  props.internal_file_type = static_cast<int>(ftype);

  // Read number of channels
  uint32_t nchan = 0;
  if(!input.uint_get(nchan, version, CHANSIZE) || nchan == 0 || nchan > MAX_CHANNELS) {
    debug("SHN::File::read() -- Invalid or unsupported channel count.");
    setValid(false);
    return;
  }
  props.channel_count = static_cast<int>(nchan);

  // Read blocksize if version > 0
  if(version > 0) {
    uint32_t blocksize = 0;
    if(!input.uint_get(blocksize, version, static_cast<size_t>(std::log2(DEFAULT_BLOCK_SIZE))) || blocksize == 0 || blocksize > MAX_BLOCKSIZE) {
      debug("SHN::File::read() -- Invalid or unsupported block size.");
      setValid(false);
      return;
    }

    uint32_t maxnlpc = 0;
    if(!input.uint_get(maxnlpc, version, LPCQSIZE) /*|| maxnlpc > 1024*/) {
      debug("SHN::File::read() -- Invalid maxnlpc.");
      setValid(false);
      return;
    }

    uint32_t nmean = 0;
    if(!input.uint_get(nmean, version, 0) /*|| nmean > 32768*/) {
      debug("SHN::File::read() -- Invalid nmean.");
      setValid(false);
      return;
    }

    uint32_t nskip;
    if(!input.uint_get(nskip, version, NSKIPSIZE)) {
      setValid(false);
      return;
    }

    for(uint32_t i = 0; i < nskip; ++i) {
      uint32_t dummy;
      if(!input.uint_get(dummy, version, XBYTESIZE)) {
        setValid(false);
        return;
      }
    }
  }

  // Parse the WAVE or AIFF header in the verbatim section

  int32_t fn;
  if(!input.uvar_get(fn, FNSIZE) || fn != FN_VERBATIM) {
    debug("SHN::File::read() -- Missing initial verbatim section.");
    setValid(false);
    return;
  }

  int32_t header_size;
  if(!input.uvar_get(header_size, VERBATIM_CKSIZE_SIZE) || header_size < CANONICAL_HEADER_SIZE || header_size > VERBATIM_CHUNK_MAX) {
    debug("SHN::File::read() -- Incorrect header size.");
    setValid(false);
    return;
  }

  uint8_t header_bytes [header_size];
  for(int32_t i = 0; i < header_size; ++i) {
    int32_t byte;
    if(!input.uvar_get(byte, VERBATIM_BYTE_SIZE)) {
      debug("SHN::File::read() -- Unable to read header.");
      setValid(false);
      return;
    }

    header_bytes[i] = static_cast<uint8_t>(byte);
  }

  // header_bytes is at least CANONICAL_HEADER_SIZE (44) bytes in size

  auto chunkID = read_uint_big32(header_bytes, 0);
//  auto chunkSize = read_uint_big32(header_bytes, 4);

  const auto chunkData = header_bytes + 8;
  const uintptr_t size = header_size - 8;

  // WAVE
  if(chunkID == 'RIFF') {
    uintptr_t offset = 0;

    chunkID = read_uint_big32(chunkData, offset);
    offset += 4;
    if(chunkID != 'WAVE') {
      debug("SHN::File::read() -- Missing 'WAVE' in 'RIFF' chunk.");
      setValid(false);
      return;
    }

    auto sawFormatChunk = false;
    uint32_t dataChunkSize = 0;
    uint16_t blockAlign = 0;

    while(offset < size) {
      chunkID = read_uint_big32(chunkData, offset);
      offset += 4;

      auto chunkSize = read_uint_little32(chunkData, offset);
      offset += 4;

      switch(chunkID) {
        case 'fmt ':
        {
          if(chunkSize < 16) {
            debug("SHN::File::read() -- 'fmt ' chunk is too small.");
            setValid(false);
            return;
          }

          auto format_tag = read_uint_little16(chunkData, offset);
          offset += 2;
          if(format_tag != WAVE_FORMAT_PCM) {
            debug("SHN::File::read() -- Unsupported WAVE format tag.");
            setValid(false);
            return;
          }

          auto channels = read_uint_little16(chunkData, offset);
          offset += 2;
          if(props.channel_count != channels)
            debug("SHN::File::read() -- Channel count mismatch between Shorten and 'fmt ' chunk.");

          props.sample_rate = static_cast<int>(read_uint_little32(chunkData, offset));
          offset += 4;

          // Skip average bytes per second
          offset += 4;

          blockAlign = read_uint_little16(chunkData, offset);
          offset += 2;

          props.bits_per_sample = static_cast<int>(read_uint_little16(chunkData, offset));
          offset += 2;

          if(chunkSize > 16)
            debug("SHN::File::read() -- Extra bytes in 'fmt ' chunk not parsed.");

          sawFormatChunk = true;

          break;
        }

        case 'data':
          dataChunkSize = chunkSize;
          break;
      }
    }

    if(!sawFormatChunk) {
      debug("SHN::File::read() -- Missing 'fmt ' chunk.");
      setValid(false);
      return;
    }

    if(dataChunkSize && blockAlign)
      props.sample_frames = static_cast<unsigned long>(dataChunkSize / blockAlign);
  }
  // AIFF
  else if(chunkID == 'FORM') {
    uintptr_t offset = 0;

    auto chunkID = read_uint_big32(chunkData, offset);
    offset += 4;
    if(chunkID != 'AIFF' && chunkID != 'AIFC') {
      debug("SHN::File::read() -- Missing 'AIFF' or 'AIFC' in 'FORM' chunk.");
      setValid(false);
      return;
    }

//    if(chunkID == 'AIFC')
//      props.big_endian = true;

    auto sawCommonChunk = false;
    while(offset < size) {
      chunkID = read_uint_big32(chunkData, offset);
      offset += 4;

      auto chunkSize = read_uint_big32(chunkData, offset);
      offset += 4;

      // All chunks must have an even length but the pad byte is not included in ckSize
      chunkSize += (chunkSize & 1);

      switch(chunkID) {
        case 'COMM':
        {
          if(chunkSize < 18) {
            debug("SHN::File::read() -- 'COMM' chunk is too small.");
            setValid(false);
            return;
          }

          auto channels = read_uint_big16(chunkData, offset);
          offset += 2;
          if(props.channel_count != channels)
            debug("SHN::File::read() -- Channel count mismatch between Shorten and 'COMM' chunk.");

          props.sample_frames = static_cast<unsigned long>(read_uint_big32(chunkData, offset));
          offset += 4;

          props.bits_per_sample = static_cast<int>(read_uint_big16(chunkData, offset));
          offset += 2;

          // sample rate is IEEE 754 80-bit extended float (16-bit exponent, 1-bit integer part, 63-bit fraction)
          auto exp = static_cast<int16_t>(read_uint_big16(chunkData, offset)) - 16383 - 63;
          offset += 2;
          if(exp < -63 || exp > 63) {
            debug("SHN::File::read() -- exp out of range.");
            setValid(false);
            return;
          }

          auto frac = read_uint_big64(chunkData, offset);
          offset += 8;
          if(exp >= 0)
            props.sample_rate = static_cast<int>(frac << exp);
          else
            props.sample_rate = static_cast<int>((frac + (static_cast<uint64_t>(1) << (-exp - 1))) >> -exp);

          if(chunkSize > 18)
            debug("SHN::File::read() -- Extra bytes in 'COMM' chunk not parsed.");

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
      debug("SHN::File::read() -- Missing 'COMM' chunk");
      setValid(false);
      return;
    }
  }
  else {
    debug("SHN::File::read() -- Unsupported data format.");
    setValid(false);
    return;
  }

  d->tag = std::make_unique<Tag>();
  d->properties = std::make_unique<Properties>(&props, propertiesStyle);
}
