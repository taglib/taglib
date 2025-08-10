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
#include "tfile.h"
#include "tstring.h"
#include "tbytevector.h"
#include "tdebug.h"
#include <string>

using namespace TagLib;

template<String::Type t>
bool EBML::StringElement<t>::read(TagLib::File &file)
{
  ByteVector buffer = file.readBlock(dataSize);
  if(buffer.size() != dataSize) {
    debug("Failed to read string");
    return false;
  }

  // The EBML strings aren't supposed to be null-terminated,
  // but we'll check for it and strip the null terminator if found
  int nullByte = buffer.find('\0');
  if(nullByte >= 0)
    buffer = ByteVector(buffer.data(), nullByte);
  value = String(buffer, t);
  return true;
}
template bool EBML::StringElement<String::UTF8>::read(TagLib::File &file);
template bool EBML::StringElement<String::Latin1>::read(TagLib::File &file);

template<String::Type t>
ByteVector EBML::StringElement<t>::render()
{
  ByteVector buffer = renderId();
  std::string string = value.to8Bit(t == String::UTF8);
  dataSize = string.size();
  buffer.append(renderVINT(dataSize, 0));
  buffer.append(ByteVector(string.data(), static_cast<unsigned int>(dataSize)));
  return buffer;
}
template ByteVector EBML::StringElement<String::UTF8>::render();
template ByteVector EBML::StringElement<String::Latin1>::render();
