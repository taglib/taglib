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

#ifndef TAGLIB_CPUFEATURESCHECK_H
#define TAGLIB_CPUFEATURESCHECK_H

// checks if a CPU supports ALL of the below:
// * FMA3
// * MOVBE
// * XSAVE
// * OSXSAVE
// * AVX2
// * BMI1,2
// * LZCNT
// This list of features corresponds to Haswell or newer intel's CPU
bool check_4th_gen_intel_core_features() noexcept;

#endif
