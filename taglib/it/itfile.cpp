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

#include "tstringlist.h"
#include "itfile.h"

namespace TagLib {

	namespace IT {

uint8_t AUTOVIB_IT_TO_XM[] = {0, 3, 1, 4, 2, 0, 0, 0};

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

IT::Properties *File::audioProperties() const {
    return m_properties;
}

bool File::save() {
    return false;
}

void File::read(bool, AudioProperties::ReadStyle propertiesStyle) {
	delete m_tag;
	delete m_properties;

	m_tag        = new Mod::Tag();
	m_properties = new IT::Properties(propertiesStyle);

	if (!isOpen())
		return;

	try {
		ByteVector mod_id(readBytes(4UL));
		if (mod_id != "IMPM") {
			throw Mod::ReadError();
		}

		m_tag->setTitle(readString(26));
		seek(2, Current);

		uint16_t length = readU16L();
		uint16_t instrumentCount = readU16L();
		uint16_t sampleCount = readU16L();
		
		m_properties->setSampleLength(length);
		m_properties->setInstrumentCount(instrumentCount);
		m_properties->setSampleCount(sampleCount);
		m_properties->setPatternCount(readU16L());
		m_properties->setVersion(readU16L());
		m_properties->setCmwt(readU16L());
		m_properties->setFlags(readU16L());

		uint16_t special = readU16L();

		m_properties->setSpecial(special);
		m_properties->setBaseVolume(readU16L());

		seek(1, Current);

		m_properties->setTempo(readByte());
		m_properties->setBpmSpeed(readByte());

		StringList comment;

		for (uint16_t i = 0; i < instrumentCount; ++ i) {
			seek(192 + length + (i << 2));
			uint32_t instrumentOffset = readU32L();
			seek(instrumentOffset);

			ByteVector instrumentMagic = readBytes(4);
			if (instrumentMagic != "IMPS" && instrumentMagic != "IMPI") {
				throw Mod::ReadError();
			}

			String dosFileName = readString(13);

			seek(15, Current);

			String instrumentName = readString(26);

			comment.append(instrumentName);
		}
		
		for (uint16_t i = 0; i < sampleCount; ++ i) {
			seek(192 + length + (instrumentCount << 2) + (i << 2));
			uint32_t sampleOffset = readU32L();
			seek(sampleOffset);

			ByteVector sampleMagic = readBytes(4);
			if (sampleMagic != "IMPS" && sampleMagic != "IMPI") {
				throw Mod::ReadError();
			}

			String dosFileName = readString(13);
			uint8_t globalVolume = readByte();
			uint8_t sampleFlags  = readByte();
			uint8_t sampleValume = readByte();
			String sampleName = readString(26);
			uint8_t sampleCvt = readByte();
			uint8_t samplePanning = readByte();
			uint32_t sampleLength = readU32L();
			uint32_t repeatStart = readU32L();
			uint32_t repeatStop = readU32L();
			uint32_t c4speed = readU32L();
			uint32_t sustainLoopStart = readU32L();
			uint32_t sustainLoopEnd = readU32L();
			uint32_t sampleDataOffset = readU32L();
			uint8_t vibratoRate = readByte();
			uint8_t vibratoDepth = readByte();
			uint8_t vibratoSweep = readByte();
			uint8_t vibratoType = readByte();

			if (c4speed == 0) {
				c4speed = 8363;
			}
			else if (c4speed < 256) {
				c4speed = 256;
			}
			
			vibratoDepth = vibratoDepth & 0x7F;
			vibratoSweep = (vibratoSweep + 3) >> 2;
			vibratoType  = AUTOVIB_IT_TO_XM[vibratoType & 0x07];

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
