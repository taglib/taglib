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

#include "ebmlutils.h"
#include <random>
#include "tbytevector.h"
#include "matroskafile.h"
#include "tutils.h"
#include "tdebug.h"

using namespace TagLib;

std::unique_ptr<EBML::Element> EBML::findElement(File &file, Element::Id id, offset_t maxOffset)
{
  std::unique_ptr<Element> element;
  while(file.tell() < maxOffset) {
    element = Element::factory(file);
    if(!element || element->getId() == id)
      return element;
    element->skipData(file);
    element.reset();
  }
  return element;
}

std::unique_ptr<EBML::Element> EBML::findNextElement(File &file, offset_t maxOffset)
{
  return file.tell() < maxOffset ? Element::factory(file) : nullptr;
}

template <int maxSizeLength>
unsigned int EBML::VINTSizeLength(uint8_t firstByte)
{
  static_assert(maxSizeLength >= 1 && maxSizeLength <= 8);
  if(!firstByte) {
    debug("VINT with greater than 8 bytes not allowed");
    return 0;
  }
  uint8_t mask = 0b10000000;
  unsigned int numBytes = 1;
  while(!(mask & firstByte)) {
    numBytes++;
    mask >>= 1;
  }
  if(numBytes > maxSizeLength) {
    debug(Utils::formatString("VINT size length exceeds %i bytes", maxSizeLength));
    return 0;
  }
  return numBytes;
}

namespace TagLib::EBML {
  template unsigned int VINTSizeLength<4>(uint8_t firstByte);
  template unsigned int VINTSizeLength<8>(uint8_t firstByte);
}

std::pair<unsigned int, uint64_t> EBML::readVINT(File &file)
{
  auto buffer = file.readBlock(1);
  if(buffer.size() != 1) {
    debug("Failed to read VINT size");
    return {0, 0};
  }
  unsigned int numBytes = VINTSizeLength<8>(*buffer.begin());
  if(!numBytes)
    return {0, 0};

  if(numBytes > 1)
    buffer.append(file.readBlock(numBytes - 1));
  const int bitsToShift = static_cast<int>(sizeof(uint64_t) * 8) - 7 * numBytes;
  const uint64_t mask = 0xFFFFFFFFFFFFFFFF >> bitsToShift;
  return { numBytes, buffer.toULongLong(true) & mask };
}

std::pair<unsigned int, uint64_t> EBML::parseVINT(const ByteVector &buffer)
{
  if(buffer.isEmpty())
    return {0, 0};

  unsigned int numBytes = VINTSizeLength<8>(*buffer.begin());
  if(!numBytes)
    return {0, 0};

  const int bitsToShift = static_cast<int>(sizeof(uint64_t) * 8) - 7 * numBytes;
  const uint64_t mask = 0xFFFFFFFFFFFFFFFF >> bitsToShift;
  return { numBytes, buffer.toULongLong(true) & mask };
}

ByteVector EBML::renderVINT(uint64_t number, int minSizeLength)
{
  const int numBytes = std::max(minSizeLength, minSize(number));
  number |= 1ULL << (numBytes * 7);
  static const auto byteOrder = Utils::systemByteOrder();
  if(byteOrder == Utils::LittleEndian)
    number = Utils::byteSwap(number);
  return ByteVector(reinterpret_cast<char *>(&number) + (sizeof(number) - numBytes), numBytes);
}

unsigned long long EBML::randomUID()
{
  static std::random_device device;
  static std::mt19937 generator(device());
  static std::uniform_int_distribution<unsigned long long> distribution;
  return distribution(generator);
}
