/***************************************************************************
    copyright           : (C) 2020 inMusic brands, inc.
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

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)

#include <cpuFeaturesCheck.h>

#include <cstdint>
#include <cstring>
#include <immintrin.h>

#ifndef _WIN32
#define NOINLINE __attribute__((noinline))
#else
#include <intrin.h>
#define NOINLINE __declspec(noinline)
#endif

#define TRY_AVX

namespace
{
const auto isAvx2Supported = check_4th_gen_intel_core_features();

template <typename T>
unsigned get_first_bit_set(const T value) noexcept
{
#ifndef _WIN32
  return __builtin_ctz(value);
#else
  unsigned long trailing_zero = 0;
  _BitScanForward(&trailing_zero, value);
  return trailing_zero;
#endif
}

template <typename T>
T clear_leftmost_set(const T value) noexcept
{
  return value & (value - 1);
}

}
#endif

#include "fastsearch.h"
namespace TagLib
{
#ifndef TRY_AVX
int findCharFast(const char*, size_t, char, unsigned int&)
{
  return -1;
}
int findVectorFast(const char*, size_t, const char*, size_t, unsigned int&)
{
  return -1;
}
#else
NOINLINE int findCharAvx2(const char* data, size_t dataSize, char c, unsigned int& offset)
{
  const auto mask = _mm256_set1_epi8(c);
  auto it = data + offset;
  for (const auto end = data + dataSize - 31; it < end; it += 32)
  {
    if (const auto result = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it)))))
    {
      return it - data + get_first_bit_set(result);
    }
  }
  offset = it - data;
  return -1;
}
NOINLINE int findVectorAvx2(const char* data, size_t dataSize, const char* pattern, size_t patternSize, unsigned int& offset)
{
  const auto lastPatternPosition = patternSize - 1;
  const auto first = _mm256_set1_epi8(pattern[0]);
  const auto last = _mm256_set1_epi8(pattern[lastPatternPosition]);
  auto it = data + offset;

  for (const auto end = data + (dataSize - lastPatternPosition - 31); it < end; it += 32)
  {
    const auto block_first = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it));
    const auto block_last = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it + lastPatternPosition));

    const auto eq_first = _mm256_cmpeq_epi8(first, block_first);
    const auto eq_last = _mm256_cmpeq_epi8(last, block_last);

    for (uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last)); mask; mask = clear_leftmost_set(mask))
    {
      const auto bitpos = get_first_bit_set(mask);
      if (std::memcmp(it + bitpos + 1, pattern + 1, patternSize - 2) == 0)
      {
        return it - data + bitpos;
      }
    }
  }
  offset = it - data;
  return -1;
}

int findCharFast(const char* data, size_t dataSize, char c, unsigned int& offset)
{
  if(isAvx2Supported)
  {
    return findCharAvx2(data, dataSize, c, offset);
  }
  return -1;
}
int findVectorFast(const char* data, size_t dataSize, const char* pattern, size_t patternSize, unsigned int& offset)
{
  if(isAvx2Supported)
  {
    return findVectorAvx2(data, dataSize, pattern, patternSize, offset);
  }
  return -1;
}
#endif
}
