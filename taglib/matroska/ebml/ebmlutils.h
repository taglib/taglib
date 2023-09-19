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

#ifndef TAGLIB_EBMLUTILS_H
#define TAGLIB_EBMLUTILS_H
#ifndef DO_NOT_DOCUMENT

#include <utility>
#include <cstdint>
#include "taglib.h"

#define EBML_ID_HEAD           0x1A45DFA3
#define EBML_ID_MK_SEGMENT     0x18538067 
#define EBML_ID_MK_TAGS        0x1254C367
#define EBML_ID_MK_TAG         0x7373
#define EBML_ID_MK_TAG_TARGETS 0x63C0
#define EBML_ID_MK_TARGET_TYPE_VALUE 0x68CA
#define EBML_ID_MK_SIMPLE_TAG  0x67C8
#define EBML_ID_MK_TAG_NAME 0x45A3
#define EBML_ID_MK_TAG_LANGUAGE 0x447A
#define EBML_ID_MK_TAG_STRING 0x4487

namespace TagLib {
  class File;
  class ByteVector;

  namespace EBML {
    class Element;
    using Id = unsigned int;

    Id readId(File &file);
    template<typename T>
    std::pair<int, T> readVINT(File &file);
    template<typename T>
    std::pair<int, T> parseVINT(const ByteVector &buffer);
    Element* findElement(File &file, EBML::Id id, offset_t maxLength);
    Element* findNextElement(File &file, offset_t maxOffset);

  }
}

#endif
#endif
