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
#include "xmfile.h"

#include <algorithm>

namespace TagLib {

	namespace XM {

File::File(FileName file, bool readProperties,
           AudioProperties::ReadStyle propertiesStyle) :
           Mod::File(file), m_tag(0), m_properties(0) {
	read(readProperties, propertiesStyle);
}

File::~File() {
	delete m_tag;
	delete m_properties;
}

XM::Tag *File::tag() const {
    return m_tag;
}

XM::Properties *File::audioProperties() const {
    return m_properties;
}

bool File::save() {
    return false;
}

void File::read(bool, AudioProperties::ReadStyle propertiesStyle) {
	delete m_tag;
	delete m_properties;

	m_tag        = new XM::Tag();
	m_properties = new XM::Properties(propertiesStyle);

	if (!isOpen())
		return;

	try {
		if (readBytes(17) != "Extended Module: ") {
			throw Mod::ReadError();
		}

		m_tag->setTitle(readString(20));

		if (readByte() != 0x1A) {
			throw Mod::ReadError();
		}

		m_tag->setTrackerName(readString(20));
		m_properties->setVersion(readU16L());
		uint32_t headerSize = readU32L();
		m_properties->setSampleLength(readU16L());
		m_properties->setRestartPosition(readU16L());
		m_properties->setChannels(readU16L());
		uint32_t patternCount = readU16L();
		m_properties->setPatternCount(patternCount);
		uint32_t instrumentCount = readU16L();
		m_properties->setInstrumentCount(instrumentCount);
		m_properties->setFlags(readU16L());
		m_properties->setTempo(readU16L());
		m_properties->setBpmSpeed(readU16L());

		seek(60 + headerSize);
		
		for (uint16_t i = 0; i < patternCount; ++ i) {
			uint32_t patternHeaderLength = readU32L();
			uint8_t  patternType = readByte();
			uint16_t rowCount = readU16L();
			uint16_t patternDataSize = readU16L();

			seek(patternHeaderLength - (4+1+2+2) + patternDataSize, Current);
		}
		
		StringList intrumentNames;
		StringList sampleNames;
		for (uint16_t i = 0; i < instrumentCount; ++ i) {
			long pos = tell();
			uint32_t instrumentSize = readU32L();

			String instrumentName;
			uint8_t instrumentType = 0;
			uint16_t sampleCount = 0;
			
			if (instrumentSize > 4) {
				instrumentName = readString(std::min((uint32_t)22,instrumentSize-4));

				if (instrumentSize >= (4+22+1)) {
					instrumentType = readByte();

					if (instrumentSize >= (4+22+1+2)) {
						sampleCount = readU16L();
					}
				}
			}

			uint32_t sampleHeaderSize = 0;
			uint32_t sumSampleLength = 0;
			if (sampleCount > 0) {
				sampleHeaderSize = readU32L();
				seek(pos + instrumentSize);

				long sampleheaderPos = tell();
				for (uint16_t j = 0; j < sampleCount; ++ j) {
					seek(sampleheaderPos + sampleHeaderSize * j);
					uint32_t length = readU32L();
					uint32_t loopStart = readU32L();
					uint32_t loopLength = readU32L();
					uint8_t  volume = readByte();
					uint8_t  finetune = readByte();
					uint8_t  sampleType = readByte();
					uint8_t  panning = readByte();
					uint8_t  noteNumber = readByte();
					uint8_t  compression = readByte();
					String sampleName = readString(22);
					
					sumSampleLength += length;
					sampleNames.append(sampleName);
				}
			}
			intrumentNames.append(instrumentName);
			seek(pos + instrumentSize + sampleHeaderSize * sampleCount + sumSampleLength);
		}

		m_tag->setComment(intrumentNames.toString("\n") + "\n" + sampleNames.toString("\n"));
	}
	catch (const Mod::ReadError&) {
		setValid(false);
	}
}

	}
}
