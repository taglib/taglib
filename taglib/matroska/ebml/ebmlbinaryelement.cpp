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

#include "ebmlbinaryelement.h"
#include "tfile.h"
#include "tbytevector.h"
#include "tdebug.h"
#include <string>

using namespace TagLib;

bool EBML::BinaryElement::read(TagLib::File &file)
{
  value = file.readBlock(dataSize);
  if(value.size() != dataSize) {
    debug("Failed to read binary element");
    return false;
  }
  return true;
}

ByteVector EBML::BinaryElement::render()
{
  ByteVector buffer = renderId();
  dataSize = value.size();
  buffer.append(renderVINT(dataSize, 0));
  buffer.append(value);
  return buffer;
}
