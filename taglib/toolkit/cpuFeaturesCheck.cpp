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

#include <cstdint>

#ifndef _WIN32
#include <cpuid.h>
#define _XCR_XFEATURE_ENABLED_MASK 0
#define NOINLINE __attribute__((noinline))
#else
#include <intrin.h>
#endif

namespace
{
void cpuid(int info[4], int InfoType) noexcept
{
#ifdef _WIN32
	__cpuidex(info, InfoType, 0);
#else
	__cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
#endif
}

bool check_xcr0_ymm() noexcept
{
	uint32_t xcr0;
#if defined(_MSC_VER)
	xcr0 = (uint32_t)_xgetbv(0);
#else
	__asm__ __volatile__ ("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx");
#endif
	// checking if xmm and ymm state are enabled in XCR0
	return (xcr0 & 6) == 6;
}
}

bool check_4th_gen_intel_core_features() noexcept
{
	// See original article
	// https://software.intel.com/en-us/articles/how-to-detect-new-instruction-support-in-the-4th-generation-intel-core-processor-family
	int cpuInfo[4] = {};

	cpuid(cpuInfo, 1);
	//    CPUID.(EAX=01H, ECX=0H):ECX.FMA[bit 12]==1
	// && CPUID.(EAX=01H, ECX=0H):ECX.MOVBE[bit 22]==1
	// && CPUID.(EAX=01H, ECX=0H):ECX.XSAVE[bit 26]==1
	// && CPUID.(EAX=01H, ECX=0H):ECX.OSXSAVE[bit 27]==1
	constexpr uint32_t fma_movbe_osxsave_mask = ((1 << 12) | (1 << 22) | (1 << 26) | (1 << 27));
	if((cpuInfo[2] & fma_movbe_osxsave_mask) != fma_movbe_osxsave_mask)
		return false;

	if(!check_xcr0_ymm())
		return false;

	cpuid(cpuInfo, 7);
	//    CPUID.(EAX=07H, ECX=0H):EBX.AVX2[bit 5]==1
	// && CPUID.(EAX=07H, ECX=0H):EBX.BMI1[bit 3]==1
	// && CPUID.(EAX=07H, ECX=0H):EBX.BMI2[bit 8]==1
	constexpr uint32_t avx2_bmi12_mask = (1 << 5) | (1 << 3) | (1 << 8);
	if((cpuInfo[1] & avx2_bmi12_mask) != avx2_bmi12_mask)
		return false;

	cpuid(cpuInfo, 0x80000001);
	// CPUID.(EAX=80000001H):ECX.LZCNT[bit 5]==1
	if((cpuInfo[2] & (1 << 5)) == 0)
		return false;

	return true;
}

#else

bool check_4th_gen_intel_core_features() noexcept
{
	return false;
}

#endif
