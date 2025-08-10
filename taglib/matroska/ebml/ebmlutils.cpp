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

#include <random>
#include "ebmlutils.h"
#include "ebmlelement.h"
#include "tbytevector.h"
#include "matroskafile.h"

#include "tdebug.h"
#include "tutils.h"
#include "taglib.h"

using namespace TagLib;

EBML::Element *EBML::findElement(File &file, Element::Id id, offset_t maxOffset)
{
  Element *element = nullptr;
  while(file.tell() < maxOffset) {
    element = Element::factory(file);
    if(!element || element->getId() == id)
      return element;
    element->skipData(file);
    delete element;
    element = nullptr;
  }
  return element;
}

EBML::Element *EBML::findNextElement(File &file, offset_t maxOffset)
{
  return file.tell() < maxOffset ? Element::factory(file) : nullptr;
}

template <typename T>
std::pair<int, T> EBML::readVINT(File &file)
{
  static_assert(sizeof(T) == 8);
  auto buffer = file.readBlock(1);
  if(buffer.size() != 1) {
    debug("Failed to read VINT size");
    return {0, 0};
  }
  unsigned int nb_bytes = VINTSizeLength<8>(*buffer.begin());
  if(!nb_bytes)
    return {0, 0};

  if(nb_bytes > 1)
    buffer.append(file.readBlock(nb_bytes - 1));
  int bits_to_shift = static_cast<int>(sizeof(T) * 8) - 7 * nb_bytes;
  offset_t mask = 0xFFFFFFFFFFFFFFFF >> bits_to_shift;
  return { nb_bytes, static_cast<T>(buffer.toLongLong(true)) & mask };
}
namespace TagLib::EBML {
  template std::pair<int, offset_t> readVINT<offset_t>(File &file);
  template std::pair<int, uint64_t> readVINT<uint64_t>(File &file);
}

template <typename T>
std::pair<int, T> EBML::parseVINT(const ByteVector &buffer)
{
  if(buffer.isEmpty())
    return {0, 0};

  unsigned int numBytes = VINTSizeLength<8>(*buffer.begin());
  if(!numBytes)
    return {0, 0};

  int bits_to_shift = static_cast<int>(sizeof(T) * 8) - 7 * numBytes;
  offset_t mask = 0xFFFFFFFFFFFFFFFF >> bits_to_shift;
  return { numBytes, static_cast<T>(buffer.toLongLong(true)) & mask };
}
namespace TagLib::EBML {
  template std::pair<int, offset_t> parseVINT<offset_t>(const ByteVector &buffer);
  template std::pair<int, uint64_t> parseVINT<uint64_t>(const ByteVector &buffer);
}

ByteVector EBML::renderVINT(uint64_t number, int minSizeLength)
{
  int numBytes = std::max(minSizeLength, minSize(number));
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
