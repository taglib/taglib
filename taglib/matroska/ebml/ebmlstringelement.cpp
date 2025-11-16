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

#include "ebmlstringelement.h"
#include <string>
#include "tfile.h"
#include "tbytevector.h"
#include "tdebug.h"
#include "ebmlutils.h"

using namespace TagLib;

bool EBML::StringElement::read(File &file)
{
  ByteVector buffer = file.readBlock(dataSize);
  if(buffer.size() != dataSize) {
    debug("Failed to read string");
    return false;
  }

  // The EBML strings aren't supposed to be null-terminated,
  // but we'll check for it and strip the null terminator if found
  if(const int nullByte = buffer.find('\0'); nullByte >= 0)
    buffer = ByteVector(buffer.data(), nullByte);
  value = String(buffer, encoding);
  return true;
}

ByteVector EBML::StringElement::render()
{
  ByteVector buffer = renderId();
  const std::string string = value.to8Bit(encoding == String::UTF8);
  dataSize = string.size();
  buffer.append(renderVINT(dataSize, 0));
  buffer.append(ByteVector(string.data(), static_cast<unsigned int>(dataSize)));
  return buffer;
}
