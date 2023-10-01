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

#include "ebmlmksegment.h"
#include "ebmlmktags.h"
#include "ebmlutils.h"
#include "matroskafile.h"
#include "matroskatag.h"
#include "tutils.h"
#include "tbytevector.h"
#include "tdebug.h"

using namespace TagLib;

EBML::MkSegment::~MkSegment()
{
  delete tags;
}

bool EBML::MkSegment::read(File &file)
{
  offset_t maxOffset = file.tell() + dataSize;
  tags = static_cast<MkTags*>(findElement(file, ElementIDs::MkTags, maxOffset));
  if (tags) {
    offset_t tagsHeadSize = tags->headSize();
    tagsOffset = file.tell() - tagsHeadSize;
    tagsOriginalSize = tagsHeadSize + tags->getDataSize();
    if (!tags->read(file))
      return false;
  }
  return true;
}

std::tuple<Matroska::Tag*, offset_t, offset_t> EBML::MkSegment::parseTag()
{
  if (tags)
    return {tags->parse(), tagsOffset, tagsOriginalSize};
  else
    return {nullptr, 0, 0};
}
