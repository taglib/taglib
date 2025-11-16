/***************************************************************************
    copyright            : (C) 2025 by Urs Fleisch
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

#include "ebmlfloatelement.h"
#include "ebmlutils.h"
#include "tbytevector.h"
#include "tfile.h"
#include "tdebug.h"

using namespace TagLib;

double EBML::FloatElement::getValueAsDouble(double defaultValue) const
{
  if(std::holds_alternative<double>(value)) {
    return std::get<double>(value);
  }
  if(std::holds_alternative<float>(value)) {
    return std::get<float>(value);
  }
  return defaultValue;
}

bool EBML::FloatElement::read(File &file)
{
  const ByteVector buffer = file.readBlock(dataSize);
  if(buffer.size() != dataSize) {
    debug("Failed to read EBML Float element");
    return false;
  }

  if(dataSize == 0) {
    value = std::monostate();
  }
  else if(dataSize == 4) {
    value = buffer.toFloat32BE(0);
  }
  else if(dataSize == 8) {
    value = buffer.toFloat64BE(0);
  }
  else {
    debug("Invalid size for EBML Float element");
    return false;
  }
  return true;
}

ByteVector EBML::FloatElement::render()
{
  ByteVector data;
  if(std::holds_alternative<double>(value)) {
    data = ByteVector::fromFloat64BE(std::get<double>(value));
  }
  else if(std::holds_alternative<float>(value)) {
    data = ByteVector::fromFloat32BE(std::get<float>(value));
  }
  ByteVector buffer = renderId();
  buffer.append(renderVINT(data.size(), 0));
  buffer.append(data);
  return buffer;
}
