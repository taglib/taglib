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

#include "ebmldeferredbinaryelement.h"
#include "tfile.h"

using namespace TagLib;

EBML::DeferredBinaryElement::DeferredBinaryElement(Id id, int sizeLength, offset_t dataSize):
  BinaryElement(id, sizeLength, dataSize)
{
}

EBML::DeferredBinaryElement::DeferredBinaryElement(Id id, int sizeLength, offset_t dataSize, offset_t):
  BinaryElement(id, sizeLength, dataSize)
{
}

EBML::DeferredBinaryElement::DeferredBinaryElement(Id id):
  BinaryElement(id)
{
}

bool EBML::DeferredBinaryElement::read(File &file)
{
  // The file is positioned at the data of the element, which is all we have
  // to remember in order to be able to read it later.
  dataOffset = file.tell();
  deferred = true;
  skipData(file);
  return true;
}

bool EBML::DeferredBinaryElement::isDeferred() const
{
  return deferred;
}

offset_t EBML::DeferredBinaryElement::getDataOffset() const
{
  return dataOffset;
}
