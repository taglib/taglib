/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#include "modfile.h"

using namespace TagLib;
using namespace Mod;

Mod::File::File(FileName file) : TagLib::File(file)
{
}

Mod::File::File(IOStream *stream) : TagLib::File(stream)
{
}

void Mod::File::writeString(const String &s, ulong size)
{
  ByteVector data(s.data(String::Latin1));
  data.resize(size, 0);
  writeBlock(data);
}

bool Mod::File::readString(String &s, ulong size)
{
  ByteVector data(readBlock(size));
  if(data.size() < size) return false;
  int index = data.find((char) 0);
  if(index > -1)
  {
    data.resize(index);
  }
  data.replace((char) 0xff, ' ');

  s = data;
  return true;
}

bool Mod::File::readByte(uchar &byte)
{
  ByteVector data(readBlock(1));
  if(data.size() < 1) return false;
  byte = data[0];
  return true;
}

bool Mod::File::readU16L(ushort &number)
{
  ByteVector data(readBlock(2));
  if(data.size() < 2) return false;
  number = data.toUShort(false);
  return true;
}

bool Mod::File::readU32L(ulong &number) {
  ByteVector data(readBlock(4));
  if(data.size() < 4) return false;
  number = data.toUInt(false);
  return true;
}
