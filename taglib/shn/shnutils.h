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

#ifndef TAGLIB_SHNUTILS_H
#define TAGLIB_SHNUTILS_H

namespace TagLib {
  namespace SHN {

    /// Values shared with \c SHN::Properties by \c SHN::File
    struct PropertyValues
    {
      int version { 0 };
      int fileType { 0 };
      int channelCount { 0 };
      int sampleRate { 0 };
      int bitsPerSample { 0 };
      unsigned long sampleFrames { 0 };
    };
  } // namespace SHN
} // namespace TagLib

#endif
