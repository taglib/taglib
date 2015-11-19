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

#include "tdebug.h"
#include "modfilebase.h"

using namespace TagLib;
using namespace Mod;

Mod::FileBase::FileBase(FileName file) : TagLib::File(file)
{
}

Mod::FileBase::FileBase(IOStream *stream) : TagLib::File(stream)
{
}

void Mod::FileBase::writeString(const String &s, uint size, char padding)
{
  ByteVector data(s.data(String::Latin1));
  data.resize(size, padding);
  writeBlock(data);
}

bool Mod::FileBase::readString(String &s, uint size)
{
  ByteVector data(readBlock(size));
  if(data.size() < size) return false;
  const size_t index = data.find((char) 0);
  if(index != ByteVector::npos()) {
    data.resize(index);
  }
  data.replace((char) 0xff, ' ');

  s = data;
  return true;
}

void Mod::FileBase::writeByte(uchar byte)
{
  ByteVector data(1, byte);
  writeBlock(data);
}

void Mod::FileBase::writeU16L(ushort number)
{
  writeBlock(ByteVector::fromUInt16LE(number));
}

void Mod::FileBase::writeU32L(uint number)
{
  writeBlock(ByteVector::fromUInt32LE(number));
}

void Mod::FileBase::writeU16B(ushort number)
{
  writeBlock(ByteVector::fromUInt16BE(number));
}

void Mod::FileBase::writeU32B(uint number)
{
  writeBlock(ByteVector::fromUInt32BE(number));
}

bool Mod::FileBase::readByte(uchar &byte)
{
  ByteVector data(readBlock(1));
  if(data.size() < 1) return false;
  byte = data[0];
  return true;
}

bool Mod::FileBase::readU16L(ushort &number)
{
  ByteVector data(readBlock(2));
  if(data.size() < 2) return false;
  number = data.toUInt16LE(0);
  return true;
}

bool Mod::FileBase::readU32L(uint &number) {
  ByteVector data(readBlock(4));
  if(data.size() < 4) return false;
  number = data.toUInt32LE(0);
  return true;
}

bool Mod::FileBase::readU16B(ushort &number)
{
  ByteVector data(readBlock(2));
  if(data.size() < 2) return false;
  number = data.toUInt16BE(0);
  return true;
}

bool Mod::FileBase::readU32B(uint &number) {
  ByteVector data(readBlock(4));
  if(data.size() < 4) return false;
  number = data.toUInt32BE(0);
  return true;
}
