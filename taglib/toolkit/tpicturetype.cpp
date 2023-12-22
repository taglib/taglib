/***************************************************************************
    copyright            : (C) 2023 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

#include "tpicturetype.h"

#include "tstring.h"

using namespace TagLib;

namespace {

  constexpr const char *typeStrs[] = {
    "Other",
    "File Icon",
    "Other File Icon",
    "Front Cover",
    "Back Cover",
    "Leaflet Page",
    "Media",
    "Lead Artist",
    "Artist",
    "Conductor",
    "Band",
    "Composer",
    "Lyricist",
    "Recording Location",
    "During Recording",
    "During Performance",
    "Movie Screen Capture",
    "Coloured Fish",
    "Illustration",
    "Band Logo",
    "Publisher Logo",
  };

}  // namespace

String Utils::pictureTypeToString(int type)
{
  if(type >= 0 && type < static_cast<int>(std::size(typeStrs))) {
    return typeStrs[type];
  }
  return "";
}

int Utils::pictureTypeFromString(const String& str)
{
  for(int i = 0; i < static_cast<int>(std::size(typeStrs)); ++i) {
    if(str == typeStrs[i]) {
      return i;
    }
  }
  return 0;
}
