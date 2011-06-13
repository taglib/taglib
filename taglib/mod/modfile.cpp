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

#ifdef HAVE_ENDIAN_H
#include <endian.h>
#endif

namespace TagLib {

	namespace Mod {

ByteVector File::readBytes(unsigned long size) {
	ByteVector data(readBlock(size));
	if (data.size() != size) throw ReadError();
	return data;
}

String File::readString(unsigned long size) {
	ByteVector data(readBytes(size));
	int index = data.find((char) 0);
	if (index > -1) {
		data.resize(index);
	}
	data.replace((char) 0xff, ' ');

	return String(data);
}

uint8_t File::readByte() {
	return readBytes(1)[0];
}

#ifdef HAVE_ENDIAN_H
uint16_t File::readU16B() {
	return be16toh(*(uint16_t*) readBytes(2).data());
}

uint16_t File::readU16L() {
	return le16toh(*(uint16_t*) readBytes(2).data());
}

uint32_t File::readU32B() {
	return be32toh(*(uint32_t*) readBytes(4).data());
}

uint32_t File::readU32L() {
	return le32toh(*(uint32_t*) readBytes(4).data());
}
#else
uint16_t File::readU16B() {
	return readBytes(2).toUShort(true);
}

uint16_t File::readU16L() {
	return readBytes(2).toUShort(false);
}

// XXX: who knows if this works if sizeof(int) > 4?
uint32_t File::readU32B() {
	return readBytes(4).toUInt(true);
}

uint32_t File::readU32L() {
	return readBytes(4).toUInt(false);
}
#endif

	}
}
