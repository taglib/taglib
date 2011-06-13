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

#include "s3mfile.h"

#include "tstringlist.h"

namespace TagLib {

	namespace S3M {

File::File(FileName file, bool readProperties,
           AudioProperties::ReadStyle propertiesStyle) :
           Mod::File(file), m_tag(0), m_properties(0) {
	read(readProperties, propertiesStyle);
}

File::~File() {
	delete m_tag;
	delete m_properties;
}

Mod::Tag *File::tag() const {
    return m_tag;
}

S3M::Properties *File::audioProperties() const {
    return m_properties;
}

bool File::save() {
    return false;
}

void File::read(bool, AudioProperties::ReadStyle propertiesStyle) {
	delete m_tag;
	delete m_properties;

	m_tag        = new Mod::Tag();
	m_properties = new S3M::Properties(propertiesStyle);

	if (!isOpen())
		return;

	try {
		m_tag->setTitle(readString(28));

		uint8_t mark = readByte();
		uint8_t type = readByte();

		if (mark != 0x1A || type != 0x10) {
			throw Mod::ReadError();
		}

		seek(32);

		uint16_t length = readU16L();
		uint16_t sampleCount = readU16L();
		m_properties->setSampleLength(length);
		m_properties->setSampleCount(sampleCount);
		m_properties->setPatternCount(readU16L());
		m_properties->setFlags(readU16L());
		m_properties->setVersion(readU16L());
		m_properties->setSamplesType(readU16L());

		ByteVector mod_id(readBytes(4UL));

		if (mod_id != "SCRM") {
			throw Mod::ReadError();
		}

		m_properties->setBaseVolume(readByte() << 1);
		m_properties->setTempo(readByte());
		m_properties->setBpmSpeed(readByte());
		m_properties->setStereo((readByte() & 0x80) != 0);
		m_properties->setUltraClick(readByte());
		m_properties->setUsePanningValues(readByte() == 0xFC);

		seek(10, Current);

		int channels = 0;
		for (int i = 0; i < 32; ++ i) {
			if (readByte() != 0xff) ++ channels;
		}
		m_properties->setChannels(channels);

		seek(channels, Current);

		StringList comment;
		for (uint16_t i = 0; i < sampleCount; ++ i) {
			seek(96 + length + (i << 1));

			uint16_t instrumentOffset = readU16L();
			seek(instrumentOffset << 4);

			uint8_t sampleType = readByte();
			String dosFileName = readString(13);

			uint16_t sampleOffset = readU16L();
			uint32_t sampleLength = readU32L();
			uint32_t repeatStart  = readU32L();
			uint32_t repeatStop   = readU32L();
			uint8_t  sampleColume = readByte();

			seek(2, Current);

			uint8_t  sampleFlags   = readByte();
			uint32_t baseFrequency = readU32L();

			seek(12, Current);

			String sampleName = readString(28);

			comment.append(sampleName);
		}

		m_tag->setComment(comment.toString("\n"));
	}
	catch (const Mod::ReadError&) {
		setValid(false);
	}
}

	}
}
