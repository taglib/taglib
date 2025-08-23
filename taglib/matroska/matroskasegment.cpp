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

#include "matroskasegment.h"
#include "ebmlutils.h"

using namespace TagLib;

Matroska::Segment::Segment(offset_t sizeLength, offset_t dataSize, offset_t lengthOffset) :
  Element(static_cast<ID>(EBML::Element::Id::MkSegment)),
  sizeLength(sizeLength), dataSize(dataSize)
{
  setOffset(lengthOffset);
  setSize(sizeLength);
}

ByteVector Matroska::Segment::renderInternal()
{
  return EBML::renderVINT(dataSize, static_cast<int>(sizeLength));
}

bool Matroska::Segment::render()
{
  auto beforeSize = sizeLength;
  auto data = renderInternal();
  setNeedsRender(false);
  auto afterSize = data.size();
  if(afterSize != beforeSize) {
    sizeLength = 8;
    data = renderInternal();
    setNeedsRender(false);
    afterSize = data.size();
    if(!emitSizeChanged(afterSize - beforeSize)) {
      return false;
    }
  }

  setData(data);
  return true;
}

bool Matroska::Segment::sizeChanged(Element &, offset_t delta)
{
  dataSize += delta;
  setNeedsRender(true);
  return true;
}
